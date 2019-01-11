#ifndef WAV_HPP
#define WAV_HPP

#include <vector>
#include <string>

class Wave {
    char *header;
    char *subchunk1;
    char *subchunk2;
    int channels;
    size_t footerSize;
    char *footer;

    void read_wav(std::string pathToFile);

public:
    std::vector<short> data;

    Wave(std::string pathToFile);
    ~Wave();
    std::tuple<std::vector<short>, std::vector<short> > split_channels();
    void write_wav(std::string pathToFile);
};

#endif
