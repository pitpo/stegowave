#include <vector>
#include <fstream>
#include "Utils.hpp"
#include <cmath>

#define M_PI    3.14159265358979323846          /* pi */

std::vector<bool> read_bits(std::string pathToFile) {
    std::ifstream	  file(pathToFile, std::ios::binary | std::ios::in);
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

std::vector<bool> read_bits_with_ecc_8bit(std::string pathToFile) {
    std::ifstream	  file(pathToFile, std::ios::binary | std::ios::in);
    std::vector<bool> output;
    char c;

    while (file.get(c)) {
        bool byte[8];
        for (int i = 7; i >= 0; i--) {
            byte[i] = (c >> i) & 1; // lsb on pos 0, etc...
        }
        // [ 1,  2, 3,  4, 5, 6, 7,  8, 9, 10, 11, 12]
        // [r1, r2, 0, r3, 1, 2, 3, r4, 4,  5,  6,  7]
        bool r1 = byte[0] ^ byte[1] ^ byte[3] ^ byte[4] ^ byte[6];
        bool r2 = byte[0] ^ byte[2] ^ byte[3] ^ byte[5] ^ byte[6];
        bool r3 = byte[1] ^ byte[2] ^ byte[3] ^ byte[7];
        bool r4 = byte[4] ^ byte[5] ^ byte[6] ^ byte[7];
        output.push_back(r1);
        output.push_back(r2);
        output.push_back(byte[0]);
        output.push_back(r3);
        for (int i = 1; i < 4; i++) {
            output.push_back(byte[i]);
        }
        output.push_back(r4);
        for (int i = 4; i < 8; i++) {
            output.push_back(byte[i]);
        }
    }
    file.close();
    return output;
}

std::vector<bool> read_bits_with_ecc_6bit(std::string pathToFile) {
    std::ifstream	  file(pathToFile, std::ios::binary | std::ios::in);
    std::vector<bool> output;
    char c;

    while (file.get(c)) {
        bool byte[8];
        for (int i = 7; i >= 0; i--) {
            byte[i] = (c >> i) & 1;
        }
        for (int i = 0; i < 2; i++) {
            bool r1 = byte[i*4]^byte[i*4+1]^byte[i*4+3];
            bool r2 = byte[i*4]^byte[i*4+2]^byte[i*4+3];
            bool r3 = byte[i*4+1]^byte[i*4+2]^byte[i*4+3];
            output.push_back(r1);
            output.push_back(r2);
            output.push_back(byte[i*4]);
            output.push_back(r3);
            for (int j = 1; j < 4; j++) {
                output.push_back(byte[i*4+j]);
            }
        }
    }
    file.close();
    return output;
}

void decode_ecc_8bit(std::vector<bool>&result, bool *data, int& errors_recovered, int& errors_in_ecc) {
    bool c1 = data[0] ^ data[2] ^ data[4] ^ data[6] ^ data[8] ^ data[10];
    bool c2 = data[1] ^ data[2] ^ data[5] ^ data[6] ^ data[9] ^ data[10];
    bool c3 = data[3] ^ data[4] ^ data[5] ^ data[6] ^ data[11];
    bool c4 = data[7] ^ data[8] ^ data[9] ^ data[10] ^ data[11];
    int	 c	= 8 * c4 + 4 * c3 + 2 * c2 + c1;

    if (c != 0) {
        data[c - 1] = !data[c - 1];
        errors_recovered++;
        if (c == 1 || c == 2 || c == 4 || c == 8) {
            errors_in_ecc++;
        }
    }
    for (int i = 11; i >= 8; i--) {
        result.push_back(data[i]);
    }
    for (int i = 6; i >= 4; i--) {
        result.push_back(data[i]);
    }
    result.push_back(data[2]);
}

void decode_ecc_6bit(std::vector<bool>&result, bool *data, int& errors_recovered, int& errors_in_ecc) {
    for (int j = 1; j >= 0; j--) {
        bool c1 = data[j * 7] ^ data[j * 7 + 2] ^ data[j * 7 + 4] ^ data[j * 7 + 6];
        bool c2 = data[j * 7 + 1] ^ data[j * 7 + 2] ^ data[j * 7 + 5] ^ data[j * 7 + 6];
        bool c3 = data[j * 7 + 3] ^ data[j * 7 + 4] ^ data[j * 7 + 5] ^ data[j * 7 + 6];
        int	 c	= 4 * c3 + 2 * c2 + c1;

        if (c != 0) {
            data[j * 7 + c - 1] = !data[j * 7 + c - 1];
            errors_recovered++;
            if (c == 1 || c == 2 || c == 4) {
                errors_in_ecc++;
            }
        }
        for (int k = j * 7 + 6; k >= j * 7 + 4; k--) {
            result.push_back(data[k]);
        }
        result.push_back(data[j * 7 + 2]);
    }
}

void save_bits(std::string pathToFile, std::vector<bool> bits) {
    std::ofstream file(pathToFile, std::ios::binary | std::ios::out);
    char		  c = 0;

    for (int i = 0; i < bits.size(); i++) {
        c <<= 1;
        c  |= bits[i] & 1;
        if (i % 8 == 7) {
            file.put(c);
        }
    }
    file.close();
}

double pcm_to_double(short signal) {
    double d = (double)signal / (double)32768;

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
    return (std::cos(((double)(i + (fade * 3)) / (fade * 2)) * M_PI) + 1.0) / 2;
}
