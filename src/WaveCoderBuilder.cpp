#include "WaveCoderBuilder.hpp"
#include "Wave.hpp"
#include "EchoCoder.hpp"
#include "PhaseCoder.hpp"
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <memory>

WaveCoderBuilder& WaveCoderBuilder::setInputFile(std::string input_file) {
    std::ifstream f(input_file);

    if (!f.good()) {
        throw std::invalid_argument("Given input file does not exist");
    } else {
        f.close();
    }
    this->input_file = input_file;
    return *this;
}

WaveCoderBuilder& WaveCoderBuilder::setOutputFile(std::string output_file) {
    std::ifstream f(output_file);

    if (f.good()) {
        std::cout << "WARNING: Given output file already exists, it will be overwritten" << std::endl;
        f.close();
    }
    this->output_file = output_file;
    return *this;
}

WaveCoderBuilder& WaveCoderBuilder::setDataFile(std::string data_file) {
    std::ifstream f(data_file);

    if (!f.good()) {
        throw std::invalid_argument("Given data file does not exist");
        f.close();
    }
    this->data_file = data_file;
    return *this;
}

WaveCoderBuilder& WaveCoderBuilder::setBlockSize(int block_size) {
    int check = block_size;

    while (check % 2 == 0 && check != 2) {
        check /= 2;
    }
    if (check == 2) {
        this->block_size = block_size;
    } else {
        std::cout << "WARNING: Invalid block size, using default (2048)" << std::endl;
    }
    return *this;
}

WaveCoderBuilder& WaveCoderBuilder::setZeroBitEcho(int d0) {
    if (d0 > block_size / 2) {
        throw std::invalid_argument("Echo value is too big");
    }
    this->d0 = d0;
    return *this;
}

WaveCoderBuilder& WaveCoderBuilder::setOneBitEcho(int d1) {
    if (d1 > block_size / 2) {
        throw std::invalid_argument("Echo value is too big");
    }
    this->d1 = d1;
    return *this;
}

WaveCoderBuilder& WaveCoderBuilder::setEchoAmplitude(double echo_amplitude) {
    if (echo_amplitude <= 0.0 || echo_amplitude > 1.0) {
        throw std::invalid_argument("Invalid echo amplitude value, it has to be in (0.0, 1.0] range");
    }
    this->echo_amplitude = echo_amplitude;
    return *this;
}

WaveCoderBuilder& WaveCoderBuilder::setWaveOffset(int offset) {
    this->offset = offset;
    return *this;
}

WaveCoderBuilder& WaveCoderBuilder::setDataSizeHint(int hint) {
    this->hint = hint;
    return *this;
}

WaveCoderBuilder& WaveCoderBuilder::setECCMode(bool ecc) {
    this->ecc = ecc;
    return *this;
}

WaveCoderBuilder& WaveCoderBuilder::setEchoFadeLength(int echo_fade) {
    this->echo_fade = echo_fade;
    return *this;
}

WaveCoderBuilder& WaveCoderBuilder::setEchoCoderStrategy(StrategyType st) {
    this->ecs = this->ecs | st;
    return *this;
}

WaveCoderBuilder& WaveCoderBuilder::setPhaseThreshold(double pt) {
    this->pt = pt;
    return *this;
}

WaveCoderBuilder& WaveCoderBuilder::setCoderType(CODER_TYPE coder) {
    this->coder = coder;
    return *this;
}

std::unique_ptr<WaveCoder> WaveCoderBuilder::build_echo() {
    if (input_file.empty() || d0 == -1 || d1 == -1) {
        throw std::runtime_error("Insufficient arguments");
    }
    Wave *wave_file = new Wave(input_file);
    std::cout << "EchoCoderStrategy = " << ecs << std::endl;
    EchoCoderStrategy *echo_coder_strategy = new EchoCoderStrategy(ecs);
    return std::make_unique<EchoCoder>(
        wave_file,
        echo_coder_strategy,
        output_file,
        data_file,
        block_size,
        d0,
        d1,
        echo_amplitude,
        offset,
        hint,
        ecc,
        echo_fade
        );
}

std::unique_ptr<WaveCoder> WaveCoderBuilder::build_phase() {
    Wave *wave_file = new Wave(input_file);

    return std::make_unique<PhaseCoder>(
        wave_file,
        block_size,
        output_file,
        data_file,
        pt
        );
}

std::unique_ptr<WaveCoder> WaveCoderBuilder::build() {
    switch (coder) {
      case ECHO:
          return build_echo();

          break;

      case PHASE:
          return build_phase();

          break;

      case NONE:
      default:
          throw std::runtime_error("No coder specified");
          break;
    }
}
