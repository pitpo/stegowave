#include <iostream>
#include <getopt.h>

#ifndef __MINGW64__
#include "stdafx.h"
#endif

void help(std::string name) {
    std::cerr << "Usage: " << name << " <option> <parameters> SOURCE\n"
              << "Options:\n"
              << "\t-h,--help\t\t\tShow this help message\n"
              << "\t-e,--encode (echo/phase)\tEncode data into wave file using (echo hiding/phase coding) method\n"
              << "\t-d,--decode (echo/phase)\tDecode data from wave file using (echo hiding/phase coding) method\n\n"
              << "Parameters:\n"
              << "\t-b,--block-size [power-of-2]\tSpecify the amount of samples in block (default is 1024)\n"
              << "\t--data [path-to-file]\t\tSpecify the data file to be encoded in file (while using phase coding, it's size must be lower or equal to block size [in bits])\n"
              << "\t--output [path-to-file]\t\tSpecify the file that output should be written to (default is [original-name.dat])" << std::endl;
}

enum MODE { UNSET = -1, ENCODE = 0, DECODE = 1 };

int main(int argc, char *argv[]) {
    if (argc < 2) {
        help(argv[0]);
    }
    int mode_flag = UNSET;
    int block_size = 1024;
    std::string method, data, output;
    method = data = output = "";

    while (true) {
        int option_index = 0;
        static struct option long_options[] = {
            { "help",      no_argument,       0, 'h' },
            { "encode",	   required_argument, 0, 'e' },
            { "decode",	   required_argument, 0, 'd' },
            { "block-size",required_argument, 0, 'b' },
            { "data",	   required_argument, 0, 'i' },
            { "output",	   required_argument, 0, 'o' },
            { 0, 0, 0, 0}
        };

        int c = getopt_long(argc, argv, "he:d:b:", long_options, &option_index);
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
            default:
                break;
        }
    }
    // printf("Mode: %d, method: %s, block_size: %d, data: %s, output %s\n", mode_flag, method.c_str(), block_size, data.c_str(), output.c_str());
}
