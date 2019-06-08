#include "EchoCoderStrategy.hpp"
#include "Utils.hpp"
#include <stdexcept>

short EchoCoderStrategy::get_applied_single_echo(std::vector<short>& channel, std::vector<short>& channel_d0, std::vector<short>& channel_d1, std::vector<double>& mixer, double echo_amplitude, int channel_offset, int echo_offset) {
    return double_to_pcm(
        pcm_to_double(channel[channel_offset]) +
        echo_amplitude * pcm_to_double(channel_d1[echo_offset]) * mixer[echo_offset] +
        echo_amplitude * pcm_to_double(channel_d0[echo_offset]) * (1.0 - mixer[echo_offset])
        );
}

short EchoCoderStrategy::get_applied_bipolar_echo(std::vector<short>& channel, std::vector<short>& channel_d0, std::vector<short>& channel_d1, std::vector<double>& mixer, double echo_amplitude, int channel_offset, int echo_offset) {
    return double_to_pcm(
        pcm_to_double(channel[channel_offset]) +
        (echo_amplitude / 2) * mixer[echo_offset] * (pcm_to_double(channel_d1[echo_offset]) - pcm_to_double(channel_d0[echo_offset])) +
        (echo_amplitude / 2) * (1.0 - mixer[echo_offset]) * (pcm_to_double(channel_d0[echo_offset]) - pcm_to_double(channel_d1[echo_offset]))
        );
}

short EchoCoderStrategy::get_applied_bf_echo(std::vector<short>& channel, std::vector<short>& channel_d0, std::vector<short>& channel_d1, std::vector<double>& mixer, double echo_amplitude, int channel_offset, int echo_offset, int d0, int d1) {
    double fd1, fd2;
    fd1 = fd2 = 0;
    if (echo_offset+(d1*2) < channel_d1.size()) {
        fd1 = pcm_to_double(channel_d1[echo_offset+(d1*2)]);
    }
    if (echo_offset+(d0*2) < channel_d0.size()) {
        fd2 = pcm_to_double(channel_d0[echo_offset+(d0*2)]);
    }
    return double_to_pcm(
        pcm_to_double(channel[channel_offset]) +
        (echo_amplitude / 2) * mixer[echo_offset] * (pcm_to_double(channel_d1[echo_offset]) + fd1) +
        (echo_amplitude / 2) * (1.0 - mixer[echo_offset]) * (pcm_to_double(channel_d0[echo_offset]) + fd2)
    );
}

short EchoCoderStrategy::get_applied_bipolar_bf_echo(std::vector<short>& channel, std::vector<short>& channel_d0, std::vector<short>& channel_d1, std::vector<double>& mixer, double echo_amplitude, int channel_offset, int echo_offset, int d0, int d1) {
    double fd1, fd2;
    fd1 = fd2 = 0;
    if (echo_offset+(d1*2) < channel_d1.size()) {
        fd1 = pcm_to_double(channel_d1[echo_offset+(d1*2)]);
    }
    if (echo_offset+(d0*2) < channel_d0.size()) {
        fd2 = pcm_to_double(channel_d0[echo_offset+(d0*2)]);
    }
    return double_to_pcm(
        pcm_to_double(channel[channel_offset]) +
        (echo_amplitude / 4) * mixer[echo_offset] * (pcm_to_double(channel_d1[echo_offset]) + fd1 - pcm_to_double(channel_d0[echo_offset]) - fd2) +
        (echo_amplitude / 4) * (1.0 - mixer[echo_offset]) * (pcm_to_double(channel_d0[echo_offset]) + fd2 - pcm_to_double(channel_d1[echo_offset]) - fd1)
    );
}

short EchoCoderStrategy::get_applied_echo(std::vector<short>& channel, std::vector<short>& channel_d0, std::vector<short>& channel_d1, std::vector<double>& mixer, double echo_amplitude, int channel_offset, int echo_offset, int d0, int d1) {
    switch (st) {
      case Single_Echo:
          return get_applied_single_echo(channel, channel_d0, channel_d1, mixer, echo_amplitude, channel_offset, echo_offset);

      case Bipolar_Echo:
          return get_applied_bipolar_echo(channel, channel_d0, channel_d1, mixer, echo_amplitude, channel_offset, echo_offset);

      case Backwards_Forwards_Echo:
          return get_applied_bf_echo(channel, channel_d0, channel_d1, mixer, echo_amplitude, channel_offset, echo_offset, d0, d1);

      case Bipolar_Backwards_Forwards_Echo:
          return get_applied_bipolar_bf_echo(channel, channel_d0, channel_d1, mixer, echo_amplitude, channel_offset, echo_offset, d0, d1);

      default:
          throw std::invalid_argument("StrategyType contains unknown value");
    }
}
