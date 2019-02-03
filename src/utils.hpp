#ifndef UTILS_HPP
#define UTILS_HPP

#include <vector>
#include <string>

enum MODE { UNSET = -1, ENCODE = 0, DECODE = 1 };

std::vector<bool> read_bits(std::string pathToFile);
void save_bits(std::string pathToFile, std::vector<bool> bits);
double pcm_to_double(short signal);
short double_to_pcm(double signal);
double mixer_translate(int i);

#endif