#include <vector>
#include <fstream>
#include "utils.hpp"

std::vector<bool> read_bits(std::string pathToFile) {
    std::ifstream file(pathToFile, std::ios::binary | std::ios::in);
    std::vector<bool> output;
    char c;
    while (file.get(c)) {
        for (int i = 7; i >= 0; i--) {
            output.push_back((c >> i) & 1);
        }
    }
    file.close();
    return output;
}
 
double pcm_to_double(short signal) {
    double d = (double) signal / (double)32768;
    if (d > 1) {
        return 1;
    } else if (d < -1) {
        return -1;
    } else {
        return d;
    }
}

short double_to_pcm(double signal) {
    double d = signal * 32768;
    if (d > 32767) {
        d = 32767;
    } else if (d < -32768) {
        d = -32768;
    }
    return (short)d;
}
