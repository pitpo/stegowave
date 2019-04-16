#ifndef PHASECODER_HPP
#define PHASECODER_HPP

#include "WaveCoder.hpp"
#include "Wave.hpp"
#include <vector>

class PhaseCoder : public WaveCoder {
    Wave *wave_file;
    int block_size;

    std::vector<short> encode2(std::vector<short> &left, std::vector<short> &right);
public:
    PhaseCoder(Wave* wave_file, int block_size) : wave_file(wave_file), block_size(block_size) { } 
    void encode();
    void decode() {}
    ~PhaseCoder() {
        delete wave_file;
    }
};

#endif 