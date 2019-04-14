OBJS = Utils.o Wave.o EchoCoder.o WaveCoderBuilder.o
PROGRAM = $(OBJS) src/main.cpp
CC = g++
DEBUG = -g
CFLAGS = -c -std=c++17
LFLAGS = -Wall -std=c++17 -lfftw3 -lm

stegowave : $(PROGRAM)
	$(CC) $(PROGRAM) $(LFLAGS) -o stegowave

Utils.o : src/Utils.cpp src/Utils.hpp
	$(CC) $(CFLAGS) -fpermissive src/Utils.cpp

Wave.o : src/Wave.cpp src/Wave.hpp
	$(CC) $(CFLAGS) src/Wave.cpp

EchoCoder.o : src/EchoCoder.cpp src/EchoCoder.hpp src/Wave.hpp src/Utils.hpp src/WaveCoder.hpp
	$(CC) $(CFLAGS) src/EchoCoder.cpp

WaveCoderBuilder.o : src/WaveCoderBuilder.cpp src/WaveCoderBuilder.hpp src/WaveCoder.hpp src/EchoCoder.hpp src/Wave.hpp src/Utils.hpp
	$(CC) $(CFLAGS) src/WaveCoderBuilder.cpp