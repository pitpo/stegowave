OBJS = utils.o wav.o echo.o
PROGRAM = $(OBJS) src/main.cpp
CC = g++
DEBUG = -g
CFLAGS = -c -std=c++11
LFLAGS = -Wall -std=c++11 -lfftw3 -lm

stegowave : $(PROGRAM)
	$(CC) $(PROGRAM) $(LFLAGS) -o stegowave

utils.o : src/utils.cpp src/utils.hpp
	$(CC) $(CFLAGS) src/utils.cpp

wav.o : src/wav.cpp src/wav.hpp
	$(CC) $(CFLAGS) src/wav.cpp

echo.o : src/echo.cpp src/echo.hpp src/wav.hpp src/utils.hpp
	$(CC) $(CFLAGS) src/echo.cpp