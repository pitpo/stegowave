#include <vector>
#include <fstream>
#include "Utils.hpp"
#include <cmath>

# define M_PI           3.14159265358979323846  /* pi */

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

std::vector<bool> read_bits_with_ecc(std::string pathToFile) {
    std::ifstream file(pathToFile, std::ios::binary | std::ios::in);
    std::vector<bool> output;
    char c;
    while (file.get(c)) {
        bool byte[12];
        // TODO: reimplement ecc
    }
    file.close();
    return output;
}

void save_bits(std::string pathToFile, std::vector<bool> bits) {
    std::ofstream file(pathToFile, std::ios::binary | std::ios::out);
    char c = 0;
    for (int i = 0; i < bits.size(); i++) {
        c <<= 1;
        c |= bits[i] & 1;
        if (i % 8 == 7) {
            file.put(c);
        }
    }
    file.close();
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

double mixer_translate(int i, int fade = 100) {
    return (std::cos(((double)(i+(fade*3))/(fade*2)) * M_PI) + 1.0)/2;
}