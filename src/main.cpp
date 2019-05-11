#include <iostream>
#include <getopt.h>
#include "Utils.hpp"
#include "EchoCoder.hpp"
#include "WaveCoder.hpp"
#include "WaveCoderBuilder.hpp"

#ifndef __MINGW64__
#include "stdafx.h"
#endif

void help(std::string name) {
    std::cerr << "Usage: " << name << " <option> <parameters> SOURCE\n"
              << "Options:\n"
              << "\t-h,--help\t\t\t\tShow this help message\n"
              << "\t-e,--encode (echo [-0 -1 -a]/phase)\tEncode data into wave file using (echo hiding/phase coding) method\n"
              << "\t-d,--decode (echo [-0 -1 -s]/phase)\tDecode data from wave file using (echo hiding/phase coding) method\n\n"
              << "Parameters:\n"
              << "\t-0 [shift]\t\t\tSpecify the echo shift for bit 0\n"
              << "\t-1 [shift]\t\t\tSpecify the echo shift for bit 1\n"
              << "\t-a [amplitude]\t\t\tSpecify the max amplitude of applied echo (0.0 to 1.0)\n"
              << "\t-s [size]\t\t\tHint the size of expected output (in bytes)\n"
              << "\t--offset [offset]\t\tSpecify the offset from which encoding/decoding should start\n"
              << "\t--ecc\t\t\t\tToggle error correction code (writes additional 4 or 6 bits for each byte)\n"
              << "\t-b,--block-size [power-of-2]\tSpecify the amount of samples in block (default is 1024)\n"
              << "\t--data [path-to-file]\t\tSpecify the data file to be encoded in file (while using phase coding, it's size must be lower or equal to block size [in bits])\n"
              << "\t--output [path-to-file]\t\tSpecify the file that output should be written to (default is [original-name.dat])" << std::endl;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        help(argv[0]);
        return 0;
    }
    MODE mode_flag = UNSET;
    std::string input;
    WaveCoderBuilder wave_coder_builder;
    while (true) {
        int option_index = 0;
        static struct option long_options[] = {
            { "help",      no_argument,       0, 'h' },
            { "encode",	   required_argument, 0, 'e' },
            { "decode",	   required_argument, 0, 'd' },
            { "block-size",required_argument, 0, 'b' },
            { "data",	   required_argument, 0, 'i' },
            { "output",	   required_argument, 0, 'o' },
            { "offset",    required_argument, 0, 'f' },
            { "ecc",       no_argument      , 0, 'c' },
            { 0, 0, 0, 0}
        };

        int c = getopt_long(argc, argv, "he:d:b:f:s:0:1:a:", long_options, &option_index);
        if (c == -1) {
            break;
        }

        switch (c) {
            case 'h':
                help(argv[0]);
                exit(0);
                break;
            case 'e':
            case 'd':
                if (c == 'e') {
                    mode_flag = ENCODE;
                } else {
                    mode_flag = DECODE;
                }
                if (optarg) {
                    std::string method = std::string(optarg);
                    if (method.compare("echo") == 0) {
                        wave_coder_builder.setCoderType(ECHO);
                    } else if (method.compare("phase") == 0) {
                        wave_coder_builder.setCoderType(PHASE);
                    }
                } else {
                    std::cerr << "Error: no method specified" << std::endl;
                    exit(-1);
                }
                break;
            case 'b':
                if (optarg) {
                    wave_coder_builder.setBlockSize(std::stoi(optarg));
                } else {
                    std::cout << "No block size specified, using default (2048)" << std::endl;
                }
                break;
            case 'i':
                if (optarg) {
                    wave_coder_builder.setDataFile(std::string(optarg));
                }
                break;
            case 'o':
                if (optarg) {
                    wave_coder_builder.setOutputFile(std::string(optarg));
                }
                break;
            case 'f':
                if (optarg) {
                    wave_coder_builder.setWaveOffset(std::stoi(optarg));
                }
                break;
            case '0':
                if (optarg) {
                    wave_coder_builder.setZeroBitEcho(std::stoi(optarg));
                }
                break;
            case '1':
                if (optarg) {
                    wave_coder_builder.setOneBitEcho(std::stoi(optarg));
                }
                break;
            case 'a':
                if (optarg) {
                    wave_coder_builder.setEchoAmplitude(std::stod(optarg));
                }
                break;
            case 's':
                if (optarg) {
                    wave_coder_builder.setDataSizeHint(std::stoi(optarg));
                }
                break;
            case 'c':
                wave_coder_builder.setECCMode(true);
                break;
            default:
                break;
        }
    }
    if (optind < argc) {
        input = argv[optind++];
        wave_coder_builder.setInputFile(input);
    } else {
        help(argv[0]);
        return 0;
    }

    try {
        auto coder = wave_coder_builder.build();
        if (mode_flag == ENCODE) {
            (*coder.get()).encode();
        } else if (mode_flag == DECODE) {
            (*coder.get()).decode();
        }
        coder.reset();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}
