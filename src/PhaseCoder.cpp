#include "PhaseCoder.hpp"
#include "Utils.hpp"
#include <fftw3.h>
#include <tuple>
#include <iostream>

#define PI    std::acos(-1.0)

void PhaseCoder::encode() {
    std::cout << "Encoding using phase coding method" << std::endl;
    std::cout << "Preparing wave file..." << std::endl;
    auto channels  = (*wave_file).split_channels();
    auto data_bits = read_bits(data_file);
    int	 blocks	   = 0;

    analyze_silence(std::get<0>(channels), blocks);
    analyze_silence(std::get<1>(channels), blocks);
    if (data_bits.size() >= writable_blocks.size() * block_size / 4) {
        throw std::runtime_error("Too much data");
    }
    std::cout << "Bits to write: " << data_bits.size() << std::endl;
    std::cout << "Blocks to write: " << writable_blocks.size() << std::endl;
    std::cout << "Bits per " << block_size << " sample long block: " << data_bits.size() / writable_blocks.size() << std::endl;
    std::cout << "Encoding in progress..." << std::endl;
    int	 bits_encoded	= 0;
    int	 blocks_encoded = 0;
    auto encoded_left	= encode_channel(std::get<0>(channels), data_bits, bits_encoded, blocks_encoded, 1);
    auto encoded_right	= encode_channel(std::get<1>(channels), data_bits, bits_encoded, blocks_encoded, 2);
    std::vector<short> out;
    for (int i = 0; i < encoded_left.size(); i++) {
        out.push_back(encoded_left[i]);
        out.push_back(encoded_right[i]);
    }
    std::cout << "Saving new wave file as \"" << output_file << "\"" << std::endl;
    (*wave_file).set_data(out);
    (*wave_file).write_wav(output_file);
    std::cout << "Done" << std::endl;
}

void PhaseCoder::decode() {
    std::cout << "Decoding using phase coding method" << std::endl;
    std::cout << "Preparing wave file..." << std::endl;
    int	 blocks	  = 0;
    auto channels = (*wave_file).split_channels();

    analyze_silence(std::get<0>(channels), blocks);
    analyze_silence(std::get<1>(channels), blocks);
    std::cout << "Blocks to read: " << writable_blocks.size() << std::endl;
    std::cout << "Decoding in progress..." << std::endl;
    int blocks_decoded = 0;
    auto left_bits	= decode_channel(std::get<0>(channels), blocks_decoded, 1);
    auto right_bits = decode_channel(std::get<1>(channels), blocks_decoded, 2);
    std::vector<bool> data;
    for (int i = 0; i < left_bits.size(); i++) {
        data.push_back(left_bits[i]);
    }
    for (int i = 0; i < right_bits.size(); i++) {
        data.push_back(right_bits[i]);
    }
    std::cout << "Writing output into \"" << output_file << "\" file" << std::endl;
    save_bits(output_file, data);
}

int extract_bit(double angle, double next_angle) {
    double threshold = 0.25;

    if (std::abs(PI / 2 - std::abs(angle)) > threshold && std::abs(PI / 2 - std::abs(next_angle)) > threshold) {
        return 2;
    }
    if (std::copysign(1, angle) == 1) {
        return 1;
    } else {
        return 0;
    }
}

std::vector<bool> PhaseCoder::decode_channel(std::vector<short>& channel, int& blocks_decoded, int channel_num) {
    auto blocks = divide_blocks(channel);
    std::vector<std::vector<std::complex<double>>> magnitudes;
    std::vector<std::vector<double>> phases;

    decompose_signal(blocks, magnitudes, phases);

    std::vector<bool> out;
    for (; blocks_decoded < writable_blocks.size() && writable_blocks[blocks_decoded] < blocks.size() * channel_num; blocks_decoded++) {
        std::cout << "decoding block " << writable_blocks[blocks_decoded] << std::endl;
        int j	= block_size / 2 - 1;
        int bit = extract_bit(phases[writable_blocks[blocks_decoded] % blocks.size()][j], phases[writable_blocks[blocks_decoded] % blocks.size()][j - 1]);
        while (bit != 2) {
            j--;
            out.push_back(bit);
            bit = extract_bit(phases[writable_blocks[blocks_decoded] % blocks.size()][j], phases[writable_blocks[blocks_decoded] % blocks.size()][j - 1]);
        }
    }
    return out;
}

std::vector<short> PhaseCoder::encode_channel(std::vector<short>& channel, std::vector<bool>& data_bits, int& bits_encoded, int& blocks_encoded, int channel_num) {
    auto blocks = divide_blocks(channel);
    std::vector<std::vector<std::complex<double>>> magnitudes;
    std::vector<std::vector<double>> phases;

    decompose_signal(blocks, magnitudes, phases);

    std::vector<std::vector<double>> phase_differences;
    for (int i = 1; i < phases.size(); i++) {
        std::vector<double> phase_difference;
        for (int j = 0; j < phases[i].size(); j++) {
            phase_difference.push_back(phases[i][j] - phases[i - 1][j]);
        }
        phase_differences.push_back(phase_difference);
    }

    // TODO: find a decent name for these variables
    bool cond, cond2;
    for (; blocks_encoded < writable_blocks.size() && writable_blocks[blocks_encoded] < blocks.size() * channel_num; blocks_encoded++) {
        std::cout << "encoding block " << writable_blocks[blocks_encoded] << std::endl;
        int block_to_write = writable_blocks[blocks_encoded] % blocks.size();
        for (int i = 1; i <= data_bits.size() / writable_blocks.size() && bits_encoded < data_bits.size(); i++, bits_encoded++) {
            phases[block_to_write][block_size / 2 - i] = data_bits[bits_encoded] ? PI / 2 : -PI / 2;
        }
        int i = block_to_write;
        do {
            for (int j = 0; j < phases[i].size(); j++) {
                phases[i + 1][j] = phases[i][j] + phase_differences[i][j];
            }
            i++;
            cond  = i < phase_differences.size() && i + 1 < writable_blocks.size() && i < writable_blocks[blocks_encoded + 1] % blocks.size();
            cond2 = i < phase_differences.size() && i + 1 >= writable_blocks.size();
        } while (cond || cond2);
    }

    double				*in_d  = (double *)fftw_malloc(sizeof(double) * block_size);
    fftw_complex		*out_c = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * block_size);
    fftw_plan			pb	   = fftw_plan_dft_c2r_1d(block_size, out_c, in_d, FFTW_PATIENT);
    std::vector<double> encoded;

    double max = 0;
    for (int i = 0; i < blocks.size(); i++) {
        for (int j = 0; j <= block_size / 2 + 1; j++) {
            std::complex<double> cstd = magnitudes[i][j] * std::exp(std::complex<double>(0, 1) * phases[i][j]);
            out_c[j][0] = cstd.real();
            out_c[j][1] = cstd.imag();
        }
        fftw_execute(pb);
        for (int j = 0; j < block_size; j++) {
            if (std::abs(in_d[j] > max)) {
                max = std::abs(in_d[j]);
            }
            encoded.push_back(in_d[j]);
        }
    }

    std::vector<short> out;
    for (int i = 0; i < encoded.size(); i++) {
        out.push_back(double_to_pcm(encoded[i] / max));
    }
    for (int i = encoded.size(); i < channel.size(); i++) {
        out.push_back(channel[i]);
    }
    fftw_destroy_plan(pb);
    fftw_free(out_c);
    fftw_free(in_d);

    return out;
}

void PhaseCoder::decompose_signal(std::vector<std::vector<short>>& blocks, std::vector<std::vector<std::complex<double>>>& magnitudes, std::vector<std::vector<double>>& phases) {
    double		 *in_d	= (double *)fftw_malloc(sizeof(double) * block_size);
    fftw_complex *out_c = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * block_size);
    fftw_plan	 pf		= fftw_plan_dft_r2c_1d(block_size, in_d, out_c, FFTW_MEASURE);

    for (int i = 0; i < blocks.size(); i++) {
        for (int j = 0; j < block_size; j++) {
            in_d[j] = blocks[i][j];
        }
        fftw_execute(pf);
        std::vector<std::complex<double>> magnitude;
        std::vector<double> phase;
        for (int j = 0; j <= block_size / 2 + 1; j++) {
            std::complex<double> c(out_c[j][0], out_c[j][1]);
            magnitude.push_back(std::abs(c));
            phase.push_back(std::log(c).imag());
        }
        magnitudes.push_back(magnitude);
        phases.push_back(phase);
    }

    fftw_destroy_plan(pf);
    fftw_free(out_c);
    fftw_free(in_d);
}

std::vector<std::vector<short>> PhaseCoder::divide_blocks(std::vector<short>& channel) {
    std::vector<std::vector<short>> blocks;

    for (int i = 0; i < channel.size() / block_size; i++) {
        std::vector<short> block;
        for (int j = 0; j < block_size; j++) {
            block.push_back(channel[i * block_size + j]);
        }
        blocks.push_back(block);
    }
    return blocks;
}

int PhaseCoder::skip_silence(std::vector<std::vector<std::complex<double>>>& magnitudes) {
    for (int i = 0; i < magnitudes.size(); i++) {
        for (int j = 0; j < block_size / 4; j++) {
            if (magnitudes[i][j].real() > 1000) {
                return i;
            }
        }
    }
    throw std::runtime_error("ERROR: Signal is silent");
}

void PhaseCoder::analyze_silence(std::vector<short>& channel, int& blocks_encountered) {
    auto blocks = divide_blocks(channel);
    std::vector<std::vector<std::complex<double>>> magnitudes;
    std::vector<std::vector<double>> phases;

    decompose_signal(blocks, magnitudes, phases);

    bool prev_is_silent = false;
    for (int i = 0; i < magnitudes.size(); i++, blocks_encountered++) {
        bool is_silent = true;
        for (int j = 0; j < block_size / 4; j++) {
            if (magnitudes[i][j].real() > 1000) {
                is_silent = false;
                if (prev_is_silent || i == 0) {
                    writable_blocks.push_back(blocks_encountered);
                    prev_is_silent = false;
                    break;
                }
            }
        }
        prev_is_silent = is_silent;
    }
}

PhaseCoder::~PhaseCoder() {
    delete wave_file;
}
