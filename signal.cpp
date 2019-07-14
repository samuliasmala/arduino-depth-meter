#include "signal.h"

Signal::Signal() {
  this->depth_no_signal(); // Show no signal until first measurement is taken
  this->prev_depth_from_depth();
}


Signal::~Signal() {
}


void Signal::check_signal() {
  // Check if the signal is correct, if not then reread before updating the screen
  if(this->check_signal_correctness()) {
    this->unsuccessful_signal_reads = 0;

    // Check whether the depth change was too quick (usually indicating incorrect decimal point bit)
    // If the depth change was checked for the previous depth value then skip because then the change
    // may be real
    if(this->last_time_too_fast_depth_change == true || this->too_fast_depth_change() == false) {
      // Depth change is not too fast, reset boolean indicator
      this->last_time_too_fast_depth_change = false;
    } else {
      // The depth change was too fast
      // Rise the flag and use previous depth value
      this->last_time_too_fast_depth_change = true;
      this->depth_from_prev_depth();
    }
  } else {
    this->unsuccessful_signal_reads++;
    
    if(this->unsuccessful_signal_reads > max_signal_read_retries) {
      // Signal has been reread max number of times and still not correct --> display dashes
      this->depth_no_signal();
    } else {
      // Signal not reread max number of times yet, keep previous depth value and try rereading
      this->depth_from_prev_depth();
    }
  }  
}
  


// Check if depth has changed too fast.  Often caused by missing decimal character
// because sometimes decimal character is not transmitted correctly
bool Signal::too_fast_depth_change() {
  if(atof(this->depth) > 5*atof(this->prev_depth) || 5*atof(this->depth) < atof(this->prev_depth))
    return true;
  else
    return false;
}


// Convert binary signal to depth string
void Signal::convert_binary_signal_to_depth(char* input_signal_print, unsigned int bits_read) {
  // Proper signal has 96 bits. If bits_read is different display no-signal string
  if(bits_read != 96) {
    strncpy(this->depth, no_signal, 5);
    return;
  }
  
  // Read digits (left to right) from input signal
  byte pos = 0;
  // If decimal point is not present add space to beginning to keep length of 4 chars
  if(input_signal_print[bits_read-23] != '1') {
    this->depth[pos++] = ' ';
  }
  this->depth[pos++] = this->convert_7bits_to_char(&input_signal_print[bits_read-7]);
  this->depth[pos++] = this->convert_7bits_to_char(&input_signal_print[bits_read-14]);
  // Check if decimal point is present
  if(input_signal_print[bits_read-23] == '1') {
    this->depth[pos++] = '.';
  }
  this->depth[pos++] = this->convert_7bits_to_char(&input_signal_print[bits_read-21]);
  this->depth[pos++] = '\0';
}

void Signal::depth_no_signal() {
  strncpy(this->depth, no_signal, 5);
}

void Signal::depth_from_prev_depth() {
  strncpy(this->depth, this->prev_depth, 5);
}

void Signal::prev_depth_from_depth() {
  strncpy(this->prev_depth, this->depth, 5);
}

char* Signal::get_depth() {
  return depth;
}


// Convert 7 bits first to a byte and then to a corresponding char
char Signal::convert_7bits_to_char(char* bits) {
  byte digit = 0;

  for(int i = 0; i < 7; i++) {
    if(bits[i] == '1') {
       digit += (1 << (6-i));
    }
  }
  return this->convert_byte_to_char(digit);
}


// Convert byte to a corresponding char
char Signal::convert_byte_to_char(byte depth) {
  if(depth == B1100000) {
    return '1';
  } else if(depth == B0110111) {
    return '2';
  } else if(depth == B1110101) {
    return '3';
  } else if(depth == B1101100) {
    return '4';
  } else if(depth == B1011101) {
    return '5';
  } else if(depth == B1011111) {
    return '6';
  } else if(depth == B1110000) {
    return '7';
  } else if(depth == B1111111) {
    return '8';
  } else if(depth == B1111101) {
    return '9';
  } else if(depth == B1111011) {
    return '0';
  } else if(depth == B0000000) {
    return ' ';
  } else {
    return '-';
  }
}

// Check depth value correctness
bool Signal::check_signal_correctness() {
  if(strcmp(this->depth, "    ") == 0)
    return false;

  if(this->depth[0] == '-' || this->depth[1] == '-'  || this->depth[2] == '-'  || this->depth[3] == '-' )
    return false;

  return true;
}
