#include "echo.hpp"
#include "wav.hpp"
#include <tuple>
#include <iostream>
#include <fftw3.h>
#include <complex>

#define PI 3.14159265358979323846

void echo(MODE mode, std::string inputFile, std::string outputFile, std::string dataFile, int block_size, int d0, int d1, double echo_amplitude, int offset) {
    if (mode == ENCODE) {
        if (outputFile.compare("") == 0 || dataFile.compare("") == 0) {
            throw "output file or data file not specified";
        }
        if (d0 == -1 || d1 == -1 || std::abs(echo_amplitude) > 1) {
            throw "insufficient or invalid parameters, please include -0 -1 -a with proper values";
        }
        Wave input	   = Wave(inputFile);
        auto channels  = input.split_channels();
        auto data_bits = read_bits(dataFile);
        auto encoded_data = echo_encode(std::get<0>(channels), std::get<1>(channels), data_bits, block_size, d0, d1, echo_amplitude, offset);
        input.data = encoded_data;
        input.write_wav(outputFile);
    } else if (mode == DECODE) {
        if (d0 == -1 || d1 == -1 || std::abs(echo_amplitude) > 1) {
            throw "insufficient or invalid parameters, please include -0 -1 -a with proper values";
        }
        Wave input = Wave(inputFile);
        auto channels = input.split_channels();
        auto decoded_data = echo_decode(std::get<0>(channels), std::get<1>(channels), block_size, d0, d1, offset);
        for (int i = 0; i < decoded_data.size(); i++) {
            if (i % 8 == 0) {
                std::cout << " ";
            }
            std::cout << decoded_data[i];
        }
        std::cout << std::endl;
    }
}

std::vector<short> echo_encode(std::vector<short> left, std::vector<short> right, std::vector<bool> data_bits, int block_size, int d0, int d1, double echo_amplitude, int offset) {
    std::vector<short> left_out;
    std::vector<short> right_out;

    for (int i = 0; i < data_bits.size(); i++) {
        int current_offset = i * block_size + offset;
        for (int j = current_offset; j < current_offset + block_size; j++) {
            if (data_bits[i] == 1) {
                double original_signal = pcm_to_double(left[j]);
                double echoed_signal   = pcm_to_double(left[j - d1]);
                int	   result		   = double_to_pcm(original_signal + echo_amplitude * echoed_signal);
                left_out.push_back(double_to_pcm(original_signal + echo_amplitude * echoed_signal));
            } else {
                double original_signal = pcm_to_double(right[j]);
                double echoed_signal   = pcm_to_double(right[j - d1]);
                right_out.push_back(double_to_pcm(original_signal + echo_amplitude * echoed_signal));
            }
        }
        for (int j = current_offset; j < current_offset + block_size; j++) {
            if (data_bits[i] == 1) {
                double original_signal = pcm_to_double(right[j]);
                double echoed_signal   = pcm_to_double(right[j - d0]);
                right_out.push_back(double_to_pcm(original_signal + echo_amplitude * echoed_signal));
            } else {
                double original_signal = pcm_to_double(left[j]);
                double echoed_signal   = pcm_to_double(left[j - d0]);
                left_out.push_back(double_to_pcm(original_signal + echo_amplitude * echoed_signal));
            }
        }
    }
    int i = 0;
    std::vector<short> out;
    for (; i < offset; i++) {
        out.push_back(left[i]);
        out.push_back(right[i]);
    }
    for (int j = 0; j < left_out.size(); i++, j++) {
        out.push_back(left_out[j]);
        out.push_back(right_out[j]);
    }
    for (; i < left.size(); i++) {
        out.push_back(left[i]);
        out.push_back(right[i]);
    }
    return out;
}



std::vector<bool> echo_decode(std::vector<short> left, std::vector<short> right, int block_size, int d0, int d1, int offset) {
    fftw_complex *in, *out;
    fftw_plan pf, pb;
    std::vector<bool> result;

    in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * block_size);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * block_size);
    pf = fftw_plan_dft_1d(block_size, in, out, FFTW_FORWARD, FFTW_MEASURE);
    pb = fftw_plan_dft_1d(block_size, in, out, FFTW_BACKWARD, FFTW_MEASURE);

    for (int i = 0; i < 32; i++) {
        int current_offset = i * block_size + offset;
        for (int j = current_offset, k = 0; j < current_offset + block_size; j++, k++) {
            in[k][0] = std::pow(std::sin((PI * pcm_to_double(left[j]))/(block_size-1)), 2);
            in[k][1] = 0;
        }
        fftw_execute(pf);
        for (int j = 0; j < block_size; j++) {
            std::complex<double> c(out[j][0], out[j][1]);
            c = std::log(c);
            in[j][0] = std::real(c);
            in[j][1] = std::imag(c);
        }
        fftw_execute(pb);
        double l0 = out[d0][0];
        double l1 = out[d1][0];
        for (int j = current_offset, k = 0; j < current_offset + block_size; j++, k++) {
            in[k][0] = pcm_to_double(right[j]);
            in[k][1] = 0;
        }
        fftw_execute(pf);
        for (int j = 0; j < block_size; j++) {
            std::complex<double> c(out[j][0], out[j][1]);
            c = std::log(c);
            in[j][0] = std::real(c);
            in[j][1] = std::imag(c);
        }
        fftw_execute(pb);
        double r0 = out[d0][0];
        double r1 = out[d1][0];
        if (l0 - l1 > r0 - r1) { // since i don't really understand what i'm doing, the results are obviously VERY wrong
            result.push_back(0);
        } else {
            result.push_back(1);
        }
    }

    fftw_destroy_plan(pf);
    fftw_destroy_plan(pb);
    fftw_free(in);
    fftw_free(out);
    return result;
}
