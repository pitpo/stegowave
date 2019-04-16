#include "EchoCoder.hpp"
#include "Utils.hpp"
#include <tuple>
#include <stdexcept>
#include <complex>
#include <iostream>
#include <algorithm>

void EchoCoder::encode() {
    std::cout << "Encoding using echo hiding method" << std::endl;
    std::cout << "Preparing wave file..." << std::endl;
    auto channels = (*wave_file).split_channels();
    std::vector<bool> data_bits;
    if (ecc) {
        std::cout << "WARNING: Injecting recovery data" << std::endl;
        if (block_size > 2048) {
            data_bits = read_bits_with_ecc_8bit(data_file);
        } else {
            data_bits = read_bits_with_ecc_6bit(data_file);
        }
    } else {
        data_bits = read_bits(data_file);
    }
    std::cout << "Data capacity: " << std::get<0>(channels).size() / block_size << " bits" << std::endl;
    std::cout << "Bits to write: " << data_bits.size() << std::endl;
    std::cout << "Encoding in progress..." << std::endl;
    auto encoded_data = apply_echo(std::get<0>(channels), std::get<1>(channels), data_bits);

    (*wave_file).set_data(encoded_data);
    std::cout << "Saving new wave file as \"" << output_file << "\"" << std::endl;
    (*wave_file).write_wav(output_file);
    std::cout << "Done" << std::endl;
}

void EchoCoder::decode() {
    std::cout << "Decoding using echo hiding method" << std::endl;
    std::cout << "Preparing wave file..." << std::endl;
    auto channels	  = (*wave_file).split_channels();
    std::cout << "Decoding in progress..." << std::endl;
    auto decoded_data = extract_echo(std::get<0>(channels), std::get<1>(channels));

    std::cout << "Writing output into \"" << output_file << "\" file" << std::endl;
    save_bits(output_file, decoded_data);
}

std::vector<short> EchoCoder::get_raw_echo(std::vector<short>& channel, std::vector<bool>& data_bits, bool bit) {
    std::vector<short> echo;
    echo.reserve(data_bits.size() * block_size);
    int d = bit == 0 ? d0 : d1;

    for (int i = 0; i < data_bits.size(); i++) {
        int current_offset = offset / 2 + i * block_size;
        for (int j = current_offset; j < current_offset + block_size; j++) {
            if (i == 0 && j < d) {
                echo.push_back(0);
            } else {
                echo.push_back(channel[j - d]);
            }
        }
    }
    return echo;
}

std::vector<double> EchoCoder::get_mixer(std::vector<bool>& data_bits) {
    std::vector<double> mixer;
    mixer.reserve(data_bits.size() * block_size);

    for (int i = 0; i < data_bits.size(); i++) {
        int sign = data_bits[i] == 1 ? 1 : -1;
        for (int j = 0; j < block_size; j++) {
            if (j < echo_fade && (i > 0 && data_bits[i - 1] != data_bits[i]) || i == 0) {
                mixer.push_back(mixer_translate(j * sign, echo_fade));
            } else if (j >= block_size - echo_fade && (i + 1 < data_bits.size() && data_bits[i + 1] != data_bits[i]) || i == data_bits.size() - 1) {
                mixer.push_back(mixer_translate(sign * block_size - sign * j, echo_fade));
            } else {
                mixer.push_back(data_bits[i]);
            }
        }
    }
    return mixer;
}

short get_applied_echo(std::vector<short>& channel, std::vector<short>& channel_d0, std::vector<short>& channel_d1, std::vector<double>& mixer, double echo_amplitude, int offset, int i) {
    return double_to_pcm(
        pcm_to_double(channel[offset]) +
        echo_amplitude * pcm_to_double(channel_d1[i]) * mixer[i] +
        echo_amplitude * pcm_to_double(channel_d0[i]) * (1.0 - mixer[i])
    );
}

std::vector<short> EchoCoder::apply_echo(std::vector<short>&left, std::vector<short>&right, std::vector<bool>&data_bits) {
    auto left_d0  = get_raw_echo(left, data_bits, 0);
    auto left_d1  = get_raw_echo(left, data_bits, 1);
    auto right_d0 = get_raw_echo(right, data_bits, 1);
    auto right_d1 = get_raw_echo(right, data_bits, 0);
    auto mixer	  = get_mixer(data_bits);
    std::vector<short> out;
    out.reserve(left.size() * 2);
    int i = 0;
    for (int j = offset / 2; i < mixer.size(); i++, j++) {
        out.push_back(get_applied_echo(left, left_d0, left_d1, mixer, echo_amplitude, j, i));
        out.push_back(get_applied_echo(right, right_d0, right_d1, mixer, echo_amplitude, j, i));
    }

    for (; i < left.size(); i++) {
        out.push_back(left[i]);
        out.push_back(right[i]);
    }
    return out;
}

void EchoCoder::calculate_cepstrum(double *dv, fftw_complex *cv, fftw_plan&pf, fftw_plan&pb, std::vector<short>& channel, int current_offset) {
    for (int i = current_offset, j = 0; j < block_size; i++, j++) {
        dv[j] = pcm_to_double(channel[i]);
    }
    fftw_execute(pf);
    for (int i = 0; i <= block_size / 2; i++) {
        std::complex<double> c(cv[i][0], cv[i][1]);
        c		 = std::log(std::abs(c));
        cv[i][0] = std::real(c);
        cv[i][1] = std::imag(c);
    }
    fftw_execute(pb);
}

std::vector<bool> EchoCoder::extract_echo(std::vector<short>&left, std::vector<short>&right) {
    double			  *dv;
    fftw_complex	  *cv;
    fftw_plan		  pf, pb;
    std::vector<bool> result;

    if (hint == -1) {
        throw std::invalid_argument("Automatic data size detection is not implemented yet, please specify output data size");
    }

    dv = (double *)fftw_malloc(sizeof(double) * block_size);
    cv = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * block_size);
    pf = fftw_plan_dft_r2c_1d(block_size, dv, cv, FFTW_MEASURE);
    pb = fftw_plan_dft_c2r_1d(block_size, cv, dv, FFTW_MEASURE);

    int byte_size = 8;
    if (ecc) {
        byte_size = (block_size > 2048) ? 12 : 14;
    }
    int errors_recovered = 0;
    int errors_in_ecc = 0;

    bool byte[byte_size];
    for (int i = 0; i < hint * byte_size; i++) {
        int current_offset = i * block_size + offset / 2;

        calculate_cepstrum(dv, cv, pf, pb, left, current_offset);
        double l0 = dv[d0];
        double l1 = dv[d1];

        calculate_cepstrum(dv, cv, pf, pb, right, current_offset);
        double r0 = dv[d0];
        double r1 = dv[d1];

        byte[i % byte_size] = (l0 - l1 > r0 - r1) ? 0 : 1;

        if (ecc && i % byte_size == byte_size - 1) {
            if (block_size > 2048) {
                decode_ecc_8bit(result, byte, errors_recovered, errors_in_ecc);
            } else {
                decode_ecc_6bit(result, byte, errors_recovered, errors_in_ecc);
            }
        } else if (!ecc) {
            result.push_back(byte[i % byte_size]);
        }
    }

    if (ecc)
        std::cout << errors_recovered << " bits had errors, " << errors_in_ecc << " of them were in recovery data" << std::endl;

    fftw_destroy_plan(pf);
    fftw_destroy_plan(pb);
    fftw_free(dv);
    fftw_free(cv);
    return result;
}

EchoCoder::~EchoCoder() {
    delete wave_file;
}
