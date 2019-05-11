#include "PhaseCoder.hpp"
#include "Utils.hpp"
#include <fftw3.h>
#include <tuple>
#include <iostream>

#define PI    std::acos(-1.0)

void PhaseCoder::encode() {
    std::cout << "Encoding using echo hiding method" << std::endl;
    std::cout << "Preparing wave file..." << std::endl;
    auto channels  = (*wave_file).split_channels();
    auto data_bits = read_bits(data_file);

    if (data_bits.size() >= block_size) {
        throw std::runtime_error("Too much data");
    }
    std::cout << "Bits to write: " << data_bits.size() << std::endl;
    std::cout << "Encoding in progress..." << std::endl;
    int	 bits_encoded  = 0;
    auto encoded_left  = encode_channel(std::get<0>(channels), data_bits, bits_encoded);
    auto encoded_right = encode_channel(std::get<1>(channels), data_bits, bits_encoded);
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
    auto channels	= (*wave_file).split_channels();
    auto left_bits	= decode_channel(std::get<0>(channels));
    auto right_bits = decode_channel(std::get<1>(channels));
    std::vector<bool> data;

    for (int i = 0; i < left_bits.size(); i++) {
        data.push_back(left_bits[i]);
    }
    for (int i = 0; i < right_bits.size(); i++) {
        data.push_back(right_bits[i]);
    }
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

std::vector<bool> PhaseCoder::decode_channel(std::vector<short>& channel) {
    auto blocks = divide_blocks(channel);
    std::vector<std::vector<std::complex<double>>> magnitudes;
    std::vector<std::vector<double>> phases;

    decompose_signal(blocks, magnitudes, phases);

    std::vector<bool> out;
    int block_after_silence = skip_silence(magnitudes);
    int i	= block_size / 2 - 1;
    int bit = extract_bit(phases[block_after_silence][i], phases[block_after_silence][i - 1]);
    while (bit != 2) {
        i--;
        out.push_back(bit);
        bit = extract_bit(phases[block_after_silence][i], phases[block_after_silence][i - 1]);
    }
    return out;
}

std::vector<short> PhaseCoder::encode_channel(std::vector<short>& channel, std::vector<bool>& data_bits, int& bits_encoded) {
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
    }

    int block_after_silence = skip_silence(magnitudes);

    for (int i = 1; i <= data_bits.size() / 2 && bits_encoded < data_bits.size(); i++, bits_encoded++) {
        phases[block_after_silence][block_size / 2 - i] = data_bits[bits_encoded] ? PI / 2 : -PI / 2;
    }

    for (int i = block_after_silence; i < phase_differences.size(); i++) {
        for (int j = 0; j < phases[i].size(); j++) {
            phases[i + 1][j] = phases[i][j] + phase_differences[i][j];
        }
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
        // std::cout << "new block" << std::endl;
        for (int j = 0; j <= block_size / 2 + 1; j++) {
            std::complex<double> c(out_c[j][0], out_c[j][1]);
            magnitude.push_back(std::abs(c));
            phase.push_back(std::log(c).imag());
            // std::cout << magnitude[j] << " " << phase[j] << std::endl;
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

PhaseCoder::~PhaseCoder() {
    delete wave_file;
}
