// SCREEN TYPES
// 1 = Starter Kit LCD 
// 2 = Waveshare eInk 2.9"
#define SCREEN_TYPE 2

#if SCREEN_TYPE == 1
  #include "screen-lcd.h"
  Screen_lcd screen;
#endif

#if SCREEN_TYPE == 2
  #include "screen-eink.h"
  Screen screen;
#endif

// Constants
const int screen_update_interval_s = 60; // How often update the screen even if the content has not changed
const int max_partial_updates = 20;      // How many partial updates before full refresh
const int pulse_length_ms = 10;          // Pulse length 10 ms (pulses come every 350 ms)
const int max_wait_for_pulse_ms = 1000;  // How long to wait for a pulse before displaying dashes
const int max_pulse_bits = 100;          // Maximum amount of bits allowed in the pulse
const int channel_1 = 2;                 // Channel for peak positions (black)
const int channel_2 = 3;                 // Channel for bit information at peak positions (brown)
const bool use_serial_for_debugging = false;
const int max_signal_read_retries = 3;
const char no_signal[] = " ---";

// Volatile variables used in the interrupt
volatile unsigned int bits_read;
volatile char input_signal[max_pulse_bits + 1]; // +1 to allow space for \0
volatile unsigned long last_interrupt_time;

// Global variables (arrays defined here to speed up the code)
int unsuccessful_signal_reads = 0;
bool last_time_too_fast_depth_change = false;
char depth[5];
char prev_depth[5];

// Setup code, to run once in the beginning
void setup()
{
  if(use_serial_for_debugging)
    Serial.begin(9600);

  pinMode(channel_1, INPUT);
  pinMode(channel_2, INPUT);
  attachInterrupt(digitalPinToInterrupt(channel_1), read_pulse, RISING);

  screen.init_screen();

  strncpy(prev_depth, no_signal, 5);

  // Reset variables for the next pulse
  bits_read = 0;
  last_interrupt_time = millis();
}


// Main loop, to run repeatedly
void loop()
{
  // Wait until a single pulse is read, i.e. more than pulse_length_ms since
  // last interrupt
  while( bits_read < 96 && millis() - last_interrupt_time < max_wait_for_pulse_ms) {
    delay(pulse_length_ms);
  }

  // Disable interrupts while reading depth from input_signal
  noInterrupts();
 
  // The input signal read in the interrupt is stored in input_signal variable
  // Add end-of-string character
  input_signal[bits_read] = '\0';
  
  // Convert binary interrupt signal to depth string stored in depth variable
  convert_binary_signal_to_depth((char*)input_signal);

  // Activate interrupts now depth is read
  interrupts();

  print_debugging_information();
  bits_read = 0;

  // Check if the signal is correct, if not then reread before updating the screen
  if(check_signal()) {
    unsuccessful_signal_reads = 0;

    // Check whether the depth change was too quick (usually indicating incorrect decimal point bit)
    // If the depth change was checked for the previous depth value then skip because then the change
    // may be real
    if(last_time_too_fast_depth_change == true || too_fast_depth_change() == false) {
      // Depth change is not too fast, reset boolean indicator
      last_time_too_fast_depth_change = false;
    } else {
      // The depth change was too fast
      // Rise the flag and use previous depth value
      last_time_too_fast_depth_change = true;
      strncpy(depth, prev_depth, 5);
    }
  } else {
    unsuccessful_signal_reads++;
    
    if(unsuccessful_signal_reads > max_signal_read_retries) {
      // Signal has been reread max number of times and still not correct --> display dashes
      strncpy(depth, no_signal, 5);
    } else {
      // Signal not reread max number of times yet, keep previous depth value and try rereading
      strncpy(depth, prev_depth, 5);
    }
  }

  // Update lcd with the value stored in depth variable
  screen.update_screen(depth);
  
  // Save depth variable so in case of unsuccessful signal read previous value can be displayed
  strncpy(prev_depth, depth, 5);
}


// Function used in the interrupt to convert signals to bits
void read_pulse()
{
  // Check if start of a new signal - if yes, reset bits_read
  if(millis() - last_interrupt_time > pulse_length_ms || bits_read > max_pulse_bits)
    bits_read = 0;
    
  if(digitalRead(channel_2) == HIGH) {
    input_signal[bits_read] = '1';
  } else {
    input_signal[bits_read] = '0';
  }

  bits_read++;
  last_interrupt_time = millis();
}


void print_debugging_information() {
  // Print debugging information only if enabled
  if(!use_serial_for_debugging)
    return;

  Serial.print("Number of bits in the pulse: ");
  Serial.println(bits_read);
  Serial.print("Bits: ");
  Serial.println((char*)input_signal);
  Serial.print("Depth: '");
  Serial.print(depth);
  Serial.print("'");
  Serial.println();
  Serial.println();
}


// Check depth value correctness
bool check_signal() {
  if(strcmp(depth, "    ") == 0)
    return false;

  if(depth[0] == '-' || depth[1] == '-'  || depth[2] == '-'  || depth[3] == '-' )
    return false;

  return true;
}


// Check if depth has changed too fast.  Often caused by missing decimal character
// because sometimes decimal character is not transmitted correctly
bool too_fast_depth_change() {
  if(atof(depth) > 5*atof(prev_depth) || 5*atof(depth) < atof(prev_depth))
    return true;
  else
    return false;
}




// Convert binary signal to depth string
void convert_binary_signal_to_depth(char* input_signal_print) {
  // Proper signal has 96 bits. If bits_read is different display no-signal string
  if(bits_read != 96) {
    strncpy(depth, no_signal, 5);
    return;
  }
  
  // Read digits (left to right) from input signal
  byte pos = 0;
  // If decimal point is not present add space to beginning to keep length of 4 chars
  if(input_signal_print[bits_read-23] != '1') {
    depth[pos++] = ' ';
  }
  depth[pos++] = convert_7bits_to_char(&input_signal_print[bits_read-7]);
  depth[pos++] = convert_7bits_to_char(&input_signal_print[bits_read-14]);
  // Check if decimal point is present
  if(input_signal_print[bits_read-23] == '1') {
    depth[pos++] = '.';
  }
  depth[pos++] = convert_7bits_to_char(&input_signal_print[bits_read-21]);
  depth[pos++] = '\0';
}


// Convert 7 bits first to a byte and then to a corresponding char
char convert_7bits_to_char(char* bits) {
  byte digit = 0;

  for(int i = 0; i < 7; i++) {
    if(bits[i] == '1') {
       digit += (1 << (6-i));
    }
  }
  return convert_byte_to_char(digit);
}


// Convert byte to a corresponding char
char convert_byte_to_char(byte depth) {
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
