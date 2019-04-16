#include "PhaseCoder.hpp"
#include "Utils.hpp"
#include <fftw3.h>
#include <tuple>
#include <complex>
#include <iostream>

std::vector<short> PhaseCoder::encode2(std::vector<short> &left, std::vector<short> &right) {
    std::vector<std::vector<short>> left_blocks;
    for (int i = 0; i < left.size() / block_size; i++) {
        std::vector<short> block;
        for (int j = 0; j < block_size; j++) {
            block.push_back(left[i*block_size+j]);
        }
        left_blocks.push_back(block);
    }

    double* in_d = (double*)fftw_malloc(sizeof(double) * block_size);
    fftw_complex* out_c = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * block_size);
    fftw_complex* in_c = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * block_size);
    fftw_plan pf = fftw_plan_dft_r2c_1d(block_size, in_d, out_c, FFTW_PATIENT);
    fftw_plan pb_d = fftw_plan_dft_c2r_1d(block_size, out_c, in_d, FFTW_PATIENT);
    fftw_plan pb_c = fftw_plan_dft_1d(block_size, out_c, in_c, FFTW_BACKWARD, FFTW_PATIENT);

    std::vector<std::vector<std::complex<double>>> magnitudes;
    std::vector<std::vector<double>> phases;
    std::vector<double> left_out;

    for (int i = 0; i < left_blocks.size(); i++) {
        for (int j = 0; j < block_size; j++) {
            in_d[j] = left_blocks[i][j];
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

    // TODO: actual algorithm

    double max = 0;
    for (int i = 0; i < left_blocks.size(); i++) {
        for (int j = 0; j <= block_size / 2 + 1; j++) {
            std::complex<double> cstd = magnitudes[i][j] * std::exp(std::complex<double>(0, 1) * phases[i][j]);
            out_c[j][0] = cstd.real();
            out_c[j][1] = cstd.imag();
        }
        // fftw_execute(pb_c);
        // for (int j = 0; j < block_size; j++) {
        //     if (std::abs(in_c[j][0]) > max) {
        //         max = std::abs(in_c[j][0]);
        //     }
        //     left_out.push_back(in_c[j][0]);
        // }
        fftw_execute(pb_d);
        for (int j = 0; j < block_size; j++) {
            if (std::abs(in_d[j] > max)) {
                max = std::abs(in_d[j]);
            }
            left_out.push_back(in_d[j]);
        }
    }
    std::cout << max << std::endl;

    std::vector<short> out;
    for (int i = 0; i < left_out.size(); i++) {
        out.push_back(double_to_pcm(left_out[i]/max));
        out.push_back(right[i]);
    }
    for (int i = left_out.size(); i < left.size(); i++) {
        out.push_back(left[i]);
        out.push_back(right[i]);
    }
    return out;
}

void PhaseCoder::encode() {
    auto channels = (*wave_file).split_channels();
    auto data = encode2(std::get<0>(channels), std::get<1>(channels));
    (*wave_file).set_data(data);
    (*wave_file).write_wav("dupa.wav");
}