#include <iostream>
#include <getopt.h>
#include "utils.hpp"
#include "echo.hpp"

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
              << "\t-offset [offset]\t\tSpecify the offset from which encoding/decoding should start\n"
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
    int block_size = 1024;
    int offset = 0;
    int d0, d1, hint;
    d0 = d1 = hint = -1;
    double amplitude = -2;
    std::string method, input, data, output;
    method = input = data = "";
    output = "output";

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
                    method = std::string(optarg);
                } else {
                    std::cerr << "Error: no method specified" << std::endl;
                    exit(-1);
                }
                break;
            case 'b':
                if (optarg) {
                    int val = std::stoi(optarg);
                    int check = val;
                    while (check % 2 == 0 && check != 2) {
                        check /= 2;
                    }
                    if (check == 2) {
                        block_size = val;
                    } else {
                        std::cerr << "Invalid block size, using default (1024)" << std::endl;
                        block_size = val;
                    }
                } else {
                    std::cerr << "No block size specified, using default (1024)" << std::endl;
                }
                break;
            case 'i':
                if (optarg) {
                    data = std::string(optarg);
                }
                break;
            case 'o':
                if (optarg) {
                    output = std::string(optarg);
                }
                break;
            case 'f':
                if (optarg) {
                    offset = std::stoi(optarg);
                }
                break;
            case '0':
                if (optarg) {
                    d0 = std::stoi(optarg);
                }
                break;
            case '1':
                if (optarg) {
                    d1 = std::stoi(optarg);
                }
                break;
            case 'a':
                if (optarg) {
                    amplitude = std::stod(optarg);
                }
                break;
            case 's':
                if (optarg) {
                    hint = std::stoi(optarg);
                }
            default:
                break;
        }
    }

    if (optind < argc) {
        input = argv[optind++];
    } else {
        help(argv[0]);
        return 0;
    }

    if (method.compare("echo") == 0) {
        try {
            echo(mode_flag, input, output, data, block_size, d0, d1, amplitude, offset, hint);
        } catch (const char *msg) {
            std::cerr << msg << std::endl;
        }
    }
    // printf("Mode: %d, method: %s, block_size: %d, data: %s, output %s\n", mode_flag, method.c_str(), block_size, data.c_str(), output.c_str());
}
