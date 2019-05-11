#ifndef ECHOCODER_HPP
#define ECHOCODER_HPP

#include "WaveCoder.hpp"
#include "Wave.hpp"
#include <string>
#include <vector>
#include <memory>
#include <fftw3.h>

class EchoCoder : public WaveCoder {
    Wave *wave_file;
    std::string output_file;
    std::string data_file;
    int block_size;
    int d0;
    int d1;
    double echo_amplitude;
    int offset;
    int hint;
    bool ecc;
    int echo_fade;

    std::vector<short> get_raw_echo(std::vector<short>&, std::vector<bool>&, bool);
    std::vector<double> get_mixer(std::vector<bool>&);
    std::vector<short> apply_echo(std::vector<short>&, std::vector<short>&, std::vector<bool>&);
    void calculate_cepstrum(double *, fftw_complex *, fftw_plan&, fftw_plan&, std::vector<short>&, int);
    std::vector<bool> extract_echo(std::vector<short>&, std::vector<short>&);

public:
    EchoCoder(Wave *wave_file, std::string output_file, std::string data_file, int block_size, int d0, int d1, double echo_amplitude, int offset, int hint, bool ecc, int echo_fade) : wave_file(wave_file), output_file(output_file), data_file(data_file), block_size(block_size), d0(d0), d1(d1), echo_amplitude(echo_amplitude), offset(offset), hint(hint), ecc(ecc), echo_fade(echo_fade) {
    }

    void encode() override;
    void decode() override;

    ~EchoCoder();
};

#endif
