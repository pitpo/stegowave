#ifndef WAVE_HPP
#define WAVE_HPP

#include <vector>
#include <string>
#include <memory>

class Wave {
    char *header;
    char *subchunk1;
    char *subchunk2;
    int channels;
    size_t footerSize;
    char *footer;
    std::vector<short> data;

    void read_wav(std::string pathToFile);

public:

    Wave(std::string pathToFile);
    ~Wave();
    std::tuple<std::vector<short>, std::vector<short>> split_channels();
    void write_wav(std::string pathToFile);
    void set_data(std::vector<short> data);
};

#endif
