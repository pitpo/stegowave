#ifndef ECHOCODERSTRATEGY_HPP
#define ECHOCODERSTRATEGY_HPP

#include <vector>

enum StrategyType {
    Single_Echo						= 0,
    Bipolar_Echo					= 1,
    Backwards_Forwards_Echo			= 2,
    Bipolar_Backwards_Forwards_Echo = 3
};

inline StrategyType operator|(StrategyType a, StrategyType b) {
    return static_cast<StrategyType>(static_cast<int>(a) | static_cast<int>(b));
}

class EchoCoderStrategy {
    short get_applied_single_echo(std::vector<short>& channel, std::vector<short>& channel_d0, std::vector<short>& channel_d1, std::vector<double>& mixer, double echo_amplitude, int channel_offset, int echo_offset);
    short get_applied_bipolar_echo(std::vector<short>& channel, std::vector<short>& channel_d0, std::vector<short>& channel_d1, std::vector<double>& mixer, double echo_amplitude, int channel_offset, int echo_offset);
    short get_applied_bf_echo(std::vector<short>& channel, std::vector<short>& channel_d0, std::vector<short>& channel_d1, std::vector<double>& mixer, double echo_amplitude, int channel_offset, int echo_offset, int d0, int d1);
    short get_applied_bipolar_bf_echo(std::vector<short>& channel, std::vector<short>& channel_d0, std::vector<short>& channel_d1, std::vector<double>& mixer, double echo_amplitude, int channel_offset, int echo_offset, int d0, int d1);

    StrategyType st;
public:
    EchoCoderStrategy(StrategyType st) : st(st) {
    }

    short get_applied_echo(std::vector<short>& channel, std::vector<short>& channel_d0, std::vector<short>& channel_d1, std::vector<double>& mixer, double echo_amplitude, int channel_offset, int echo_offset, int d0, int d1);
};

#endif
