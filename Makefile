# Compiler flags
CXXFLAGS = -Wall -Wno-sign-compare -c -std=c++17
LFLAGS = -Wall -std=c++17 -lfftw3 -lm

# Project variables
OBJS = Utils.o Wave.o EchoCoder.o PhaseCoder.o WaveCoderBuilder.o
EXE = stegowave

# Debug build
DBGDIR = debug
DBGEXE = $(DBGDIR)/$(EXE)
DBGOBJS = $(addprefix $(DBGDIR)/, $(OBJS))
DBGCXXFLAGS = -g -O0

# Release build
RELDIR = release
RELEXE = $(RELDIR)/$(EXE)
RELOBJS = $(addprefix $(RELDIR)/, $(OBJS))
RELCXXFLAGS = -O3

# default rules

all: prep release

prep:
	@mkdir -p $(DBGDIR) $(RELDIR)

# debug rules

debug: $(DBGEXE)
$(DBGEXE): $(DBGOBJS)
	$(CXX) $(DBGOBJS) -o $@ $(DBGCXXFLAGS) src/main.cpp $(LFLAGS)

$(DBGDIR)/Utils.o: src/Utils.cpp src/Utils.hpp
	$(CXX) $(CXXFLAGS) $(DBGCXXFLAGS) -fpermissive -o $@ $<

$(DBGDIR)/Wave.o: src/Wave.cpp src/Wave.hpp
	$(CXX) $(CXXFLAGS) $(DBGCXXFLAGS) -o $@ $<

$(DBGDIR)/EchoCoder.o: src/EchoCoder.cpp src/EchoCoder.hpp src/Wave.hpp src/Utils.hpp src/WaveCoder.hpp
	$(CXX) $(CXXFLAGS) $(DBGCXXFLAGS) -o $@ $<

$(DBGDIR)/PhaseCoder.o: src/PhaseCoder.cpp src/PhaseCoder.hpp src/Wave.hpp src/Utils.hpp src/WaveCoder.hpp
	$(CXX) $(CXXFLAGS) $(DBGCXXFLAGS) -o $@ $<

$(DBGDIR)/WaveCoderBuilder.o: src/WaveCoderBuilder.cpp src/WaveCoderBuilder.hpp src/WaveCoder.hpp src/EchoCoder.hpp src/Wave.hpp src/Utils.hpp
	$(CXX) $(CXXFLAGS) $(DBGCXXFLAGS) -o $@ $<

# release rules

release: $(RELEXE)
$(RELEXE): $(RELOBJS)
	$(CXX) $(RELCXXFLAGS) -o $@ $(RELOBJS) src/main.cpp $(LFLAGS)

$(RELDIR)/Utils.o: src/Utils.cpp src/Utils.hpp
	$(CXX) $(CXXFLAGS) $(RELCXXFLAGS) -fpermissive -o $@ $<

$(RELDIR)/Wave.o: src/Wave.cpp src/Wave.hpp
	$(CXX) $(CXXFLAGS) $(RELCXXFLAGS) -o $@ $<

$(RELDIR)/EchoCoder.o: src/EchoCoder.cpp src/EchoCoder.hpp src/Wave.hpp src/Utils.hpp src/WaveCoder.hpp
	$(CXX) $(CXXFLAGS) $(RELCXXFLAGS) -o $@ $<

$(RELDIR)/PhaseCoder.o: src/PhaseCoder.cpp src/PhaseCoder.hpp src/Wave.hpp src/Utils.hpp src/WaveCoder.hpp
	$(CXX) $(CXXFLAGS) $(RELCXXFLAGS) -o $@ $<

$(RELDIR)/WaveCoderBuilder.o: src/WaveCoderBuilder.cpp src/WaveCoderBuilder.hpp src/WaveCoder.hpp src/EchoCoder.hpp src/Wave.hpp src/Utils.hpp
	$(CXX) $(CXXFLAGS) $(RELCXXFLAGS) -o $@ $<