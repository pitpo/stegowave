#include "echo.hpp"
#include "wav.hpp"
#include <tuple>
#include <iostream>
#include <fftw3.h>
#include <complex>
#include <cstdio>

void echo(MODE mode, std::string inputFile, std::string outputFile, std::string dataFile, int block_size, int d0, int d1, double echo_amplitude, int offset, int hint) {
    if (mode == ENCODE) {
        if (outputFile.compare("") == 0 || dataFile.compare("") == 0) {
            throw "output file or data file not specified";
        }
        if (d0 == -1 || d1 == -1 || std::abs(echo_amplitude) > 1) {
            throw "insufficient or invalid parameters, please include -0 -1 -a with proper values";
        }
        Wave input		  = Wave(inputFile);
        auto channels	  = input.split_channels();
        auto data_bits	  = read_bits(dataFile);
        auto encoded_data = echo_encode(std::get<0>(channels), std::get<1>(channels), data_bits, block_size, d0, d1, echo_amplitude, offset);
        input.data = encoded_data;
        input.write_wav(outputFile);
    } else if (mode == DECODE) {
        if (d0 == -1 || d1 == -1) {
            throw "insufficient or invalid parameters, please include -0 -1 -a with proper values";
        }
        Wave input		  = Wave(inputFile);
        auto channels	  = input.split_channels();
        auto decoded_data = echo_decode(std::get<0>(channels), std::get<1>(channels), block_size, d0, d1, offset, hint);
        for (int i = 0; i < decoded_data.size(); i++) {
            if (i % 8 == 0) {
                std::cout << " ";
            }
            std::cout << decoded_data[i];
        }
        std::cout << std::endl;
        save_bits(outputFile, decoded_data);
    }
}

std::vector<short> echo_encode(std::vector<short> left, std::vector<short> right, std::vector<bool> data_bits, int block_size, int d0, int d1, double echo_amplitude, int offset) {
    std::vector<short>	left_d0;
    std::vector<short>	left_d1;
    std::vector<short>	right_d0;
    std::vector<short>	right_d1;
    std::vector<double> mixer;
    std::cout << "samples per channel: " << left.size() << std::endl;

    for (int i = 0; i < data_bits.size(); i++) {
        int current_offset = offset / 2 + i * block_size;
        for (int j = current_offset, k = 0; j < current_offset + block_size; j++, k++) {
            if (i == 0 && j < d0) {
                left_d0.push_back(0);
                right_d0.push_back(0);
            } else {
                left_d0.push_back(left[j - d0]);
                right_d0.push_back(right[j - d0]);
            }
            if (i == 0 && j < d1) {
                left_d1.push_back(0);
                right_d1.push_back(0);
            } else {
                left_d1.push_back(left[j - d1]);
                right_d1.push_back(right[j - d1]);
            }
            if (data_bits[i] == 1) {
                if (k < 100 && i > 0 && data_bits[i-1] == 0) {
                    mixer.push_back(mixer_translate(k));
                } else if (k >= block_size - 100 && i < data_bits.size() && data_bits[i+1] == 0) {
                    mixer.push_back(mixer_translate(block_size - k));
                } else {
                    mixer.push_back(data_bits[i]);
                }
            } else {
                if (k < 100 && i > 0 && data_bits[i-1] == 1) {
                    mixer.push_back(mixer_translate(-k));
                } else if (k >= block_size - 100 && i < data_bits.size() && data_bits[i+1] == 1) {
                    mixer.push_back(mixer_translate(k - block_size));
                } else {
                    mixer.push_back(data_bits[i]);
                }
            }
        }
    }
    std::cout << "data samples: " << left_d0.size() << std::endl;

    for (int i = 0, j = offset / 2; i < mixer.size(); i++, j++) {
        left[j]	 = double_to_pcm(pcm_to_double(left[j]) + echo_amplitude * pcm_to_double(left_d1[i]) * mixer[i] + echo_amplitude * pcm_to_double(left_d0[i]) * (1.0 - mixer[i]));
        right[j] = double_to_pcm(pcm_to_double(right[j]) + echo_amplitude * pcm_to_double(right_d1[i]) * mixer[i] + echo_amplitude * pcm_to_double(right_d0[i]) * (1.0 - mixer[i]));
    }

    std::vector<short> out;
    for (int i = 0; i < left.size(); i++) {
        out.push_back(left[i]);
        out.push_back(right[i]);
    }
    std::cout << "done" << std::endl;
    return out;
}

std::vector<bool> echo_decode(std::vector<short> left, std::vector<short> right, int block_size, int d0, int d1, int offset, int hint) {
    double			  *dv;
    fftw_complex	  *cv;
    fftw_plan		  pf, pb;
    std::vector<bool> result;

    if (hint == -1) {
        return result;
    }

    dv = (double *)fftw_malloc(sizeof(double) * block_size);
    cv = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * block_size);
    pf = fftw_plan_dft_r2c_1d(block_size, dv, cv, FFTW_MEASURE);
    pb = fftw_plan_dft_c2r_1d(block_size, cv, dv, FFTW_MEASURE);

    for (int i = 0; i < hint * 8; i++) {
        int current_offset = i * block_size + offset / 2;
        for (int j = current_offset, k = 0; k < block_size; j++, k++) {
            dv[k] = pcm_to_double(left[j]);
        }
        fftw_execute(pf);
        for (int j = 0; j < block_size; j++) {
            std::complex<double> c(cv[j][0], cv[j][1]);
            c		 = std::log(std::abs(c));
            cv[j][0] = std::real(c);
            cv[j][1] = std::imag(c);
        }
        fftw_execute(pb);
        // for (int j = 0; j < block_size; j++) {
        //     printf("%d %.2f\n", j, dv[j]);
        // }

        double l0 = dv[d0];
        double l1 = dv[d1];

        for (int j = current_offset, k = 0; k < block_size; j++, k++) {
            dv[k] = pcm_to_double(right[j]);
        }
        fftw_execute(pf);
        for (int j = 0; j < block_size; j++) {
            std::complex<double> c(cv[j][0], cv[j][1]);
            c		 = std::log(std::abs(c));
            cv[j][0] = std::real(c);
            cv[j][1] = std::imag(c);
        }
        fftw_execute(pf);

        double r0 = dv[d0];
        double r1 = dv[d1];

        if (l0 - l1 > r0 - r1) {
            result.push_back(0);
        } else {
            result.push_back(1);
        }
    }

    fftw_destroy_plan(pf);
    fftw_destroy_plan(pb);
    fftw_free(dv);
    fftw_free(cv);
    return result;
}
