#ifndef WAVECODER_HPP
#define WAVECODER_HPP

class WaveCoder {
public:
    virtual void encode() = 0;
    virtual void decode() = 0;
};

#endif