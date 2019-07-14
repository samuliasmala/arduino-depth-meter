#ifndef SIGNAL_H
#define SIGNAL_H

#include <Arduino.h>

// Import constants
extern const bool use_serial_for_debugging;
extern const int max_signal_read_retries;
extern const char no_signal[];
extern const int max_pulse_bits;


class Signal {
public:
  Signal();
  ~Signal();
  void check_signal();
  void convert_binary_signal_to_depth(char* input_signal_print, unsigned int bits_read);
  void depth_no_signal();
  void depth_from_prev_depth();
  void prev_depth_from_depth();
  char* get_depth();
  
private:
  bool too_fast_depth_change();
  char convert_7bits_to_char(char* bits);
  char convert_byte_to_char(byte depth);
  bool check_signal_correctness();
  
  int unsuccessful_signal_reads = 0;
  bool last_time_too_fast_depth_change = false;
  char depth[5];
  char prev_depth[5];
};

#endif
