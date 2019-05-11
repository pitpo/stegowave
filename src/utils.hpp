#ifndef UTILS_HPP
#define UTILS_HPP

#include <vector>
#include <string>

enum MODE { UNSET = -1, ENCODE = 0, DECODE = 1 };
enum CODER_TYPE { NONE = -1, ECHO = 0, PHASE = 1 };

std::vector<bool> read_bits(std::string pathToFile);
std::vector<bool> read_bits_with_ecc_8bit(std::string pathToFile);
std::vector<bool> read_bits_with_ecc_6bit(std::string pathToFile);
void decode_ecc_8bit(std::vector<bool>&result, bool *data, int& errors_recovered, int& errors_in_ecc);
void decode_ecc_6bit(std::vector<bool>&result, bool *data, int& errors_recovered, int& errors_in_ecc);
void save_bits(std::string pathToFile, std::vector<bool> bits);
double pcm_to_double(short signal);
short double_to_pcm(double signal);
double mixer_translate(int i, int fade = 100);

#endif
