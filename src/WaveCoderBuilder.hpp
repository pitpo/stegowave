#ifndef WAVECODERBUILDER_HPP
#define WAVECODERBUILDER_HPP

#include "WaveCoder.hpp"
#include "EchoCoderStrategy.hpp"
#include "Utils.hpp"
#include <string>
#include <memory>

class WaveCoderBuilder {
    std::string input_file	= "";
    std::string output_file = "stegout";
    std::string data_file	= "data";
    int block_size			= 2048;
    int d0 = -1;
    int d1 = -1;
    double echo_amplitude = 0.25;
    int offset			  = 0;
    int hint		 = -1;
    bool ecc		 = 0;
    int echo_fade	 = 100;
    double pt		 = 0.35;
    CODER_TYPE coder = NONE;
    StrategyType ecs = Single_Echo;

    std::unique_ptr<WaveCoder> build_echo();
    std::unique_ptr<WaveCoder> build_phase();

public:
    WaveCoderBuilder() {
    }

    WaveCoderBuilder& setInputFile(std::string);
    WaveCoderBuilder& setOutputFile(std::string);
    WaveCoderBuilder& setDataFile(std::string);
    WaveCoderBuilder& setBlockSize(int);
    WaveCoderBuilder& setZeroBitEcho(int);
    WaveCoderBuilder& setOneBitEcho(int);
    WaveCoderBuilder& setEchoAmplitude(double);
    WaveCoderBuilder& setWaveOffset(int);
    WaveCoderBuilder& setDataSizeHint(int);
    WaveCoderBuilder& setECCMode(bool);
    WaveCoderBuilder& setEchoFadeLength(int);

    WaveCoderBuilder& setEchoCoderStrategy(StrategyType);
    WaveCoderBuilder& setPhaseThreshold(double);

    WaveCoderBuilder& setCoderType(CODER_TYPE);
    std::unique_ptr<WaveCoder> build();
};

#endif
