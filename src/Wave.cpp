#include <vector>
#include <tuple>
#include <fstream>
#include <iostream>
#include <string.h>
#include <iterator>
#include "Wave.hpp"

Wave::Wave(std::string pathToFile) {
    read_wav(pathToFile);
}

Wave::~Wave() {
    delete[] header;
    delete[] subchunk1;
    delete[] subchunk2;
    delete[] footer;
}

std::tuple<std::vector<short>, std::vector<short> > Wave::split_channels() {
    std::vector<short> left, right;
    left.reserve(data.size()/2);
    right.reserve(data.size()/2);

    for (int i = 0; i < data.size(); i++) {
        if (i % 2 == 0) {
            left.push_back(data[i]);
        } else {
            right.push_back(data[i]);
        }
    }
    return std::make_tuple(left, right);
}

void Wave::write_wav(std::string pathToFile) {
    std::ofstream file(pathToFile, std::ios::binary | std::ios::out);
    file.write(header, 12);
    file.write(subchunk1, 24);
    file.write(subchunk2, 8);
    for (unsigned long i = 0; i < data.size(); i++) {
        file.put(data[i]);
        file.put(data[i] >> 8);
    }
    file.write(footer, footerSize);
    file.close();
}

void Wave::read_wav(std::string pathToFile) {
    std::ifstream file(pathToFile, std::ios::binary | std::ios::in);

    file.seekg(0, file.end);
    unsigned long filelength = file.tellg();
    file.seekg(0, file.beg);
    char *headerBuffer = new char[12];
    file.read(headerBuffer, 12);
    if (strncmp(headerBuffer, "RIFF", 4) != 0) {
        delete[] headerBuffer;
        file.close();
        throw "Not a wave file";
    }

    header = headerBuffer;

    char *subchunk1Buffer = new char[24];
    file.read(subchunk1Buffer, 24);
    // buffer[4] contains subchunk size, buffer[8] contains audio format
    if (subchunk1Buffer[4] != 16 && subchunk1Buffer[8] != 1) {
        delete[] headerBuffer;
        delete[] subchunk1Buffer;
        file.close();
        throw "Incompatible wave file";
    }
    subchunk1 = subchunk1Buffer;
    channels  = subchunk1Buffer[12];

    char *subchunk2Buffer = new char[8];
    file.read(subchunk2Buffer, 8);
    subchunk2 = subchunk2Buffer;

    // LITTLE ENDIAN FORMAT
    size_t dataSize	   = (((unsigned char)subchunk2Buffer[7]) << 24) | (((unsigned char)subchunk2Buffer[6]) << 16) | (((unsigned char)subchunk2Buffer[5]) << 8) | (unsigned char)subchunk2Buffer[4];
    char   *dataBuffer = new char[dataSize];
    file.read(dataBuffer, dataSize);
    for (size_t i = 0; i < dataSize; i += 2) {
        // ONCE AGAIN, LITTLE ENDIAN FORMAT
        short buf = (((unsigned char)dataBuffer[i + 1]) << 8) | ((unsigned char)dataBuffer[i]);
        data.push_back(buf);
    }

    size_t footerSize	 = filelength - file.tellg();
    char   *footerBuffer = new char[footerSize];
    file.read(footerBuffer, footerSize);
    this->footerSize = footerSize;
    footer	   = footerBuffer;

    file.close();
}

void Wave::set_data(std::vector<short> data) {
    this->data = data;
}