#ifndef PHASECODER_HPP
#define PHASECODER_HPP

#include "WaveCoder.hpp"
#include "Wave.hpp"
#include <vector>
#include <complex>

class PhaseCoder : public WaveCoder {
    Wave *wave_file;
    int block_size;
    std::string output_file;
    std::string data_file;

    std::vector<short> encode_channel(std::vector<short>&, std::vector<bool>&, int&);
    void decompose_signal(std::vector<std::vector<short>>&, std::vector<std::vector<std::complex<double>>>&, std::vector<std::vector<double>>&);
    std::vector<std::vector<short>> divide_blocks(std::vector<short>&);
    std::vector<bool> decode_channel(std::vector<short>&);
    int skip_silence(std::vector<std::vector<std::complex<double>>>&);

public:
    PhaseCoder(Wave *wave_file, int block_size, std::string output_file, std::string data_file) : wave_file(wave_file), block_size(block_size), output_file(output_file), data_file(data_file) {
    }

    void encode();
    void decode();

    ~PhaseCoder();
};

#endif
