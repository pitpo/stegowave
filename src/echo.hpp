#ifndef ECHO_HPP
#define ECHO_HPP

#include <vector>
#include "utils.hpp"

void echo(MODE mode, std::string inputFile, std::string outputFile, std::string dataFile, int block_size, int d0, int d1, double echo_amplitude, int offset);
std::vector<short> echo_encode(std::vector<short> left, std::vector<short> right, std::vector<bool> data_bits, int block_size, int d0, int d1, double echo_amplitude, int offset);
std::vector<bool> echo_decode(std::vector<short> left, std::vector<short> right, int block_size, int d0, int d1, int offset);

#endif