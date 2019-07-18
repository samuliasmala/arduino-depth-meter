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
  const sFONT eink_font = DroidSansMono88;
#endif

#include "signal.h"

// Constants
const int screen_update_interval_s = 60; // How often update the screen even if the content has not changed
const int max_partial_updates = 40;      // How many partial updates before full refresh
const int pulse_length_ms = 10;          // Pulse length 10 ms (pulses come every 350 ms)
const int max_wait_for_pulse_ms = 1000;  // How long to wait for a pulse before displaying dashes
const int max_pulse_bits = 100;          // Maximum amount of bits allowed in the pulse
const int input_pin_for_bit_position = 2;// Input pin for peak positions (black wire)
const int input_pin_for_bit_value = 3;   // Input pin for bit information at peak positions (brown wire)
const bool use_serial_for_debugging = false;
const bool display_debugging_on_screen = false;
const int max_signal_read_retries = 5;
const char no_signal[] = " ---";

// Volatile variables used in the interrupt
volatile unsigned int bits_read;
volatile char input_signal[max_pulse_bits + 1]; // +1 to allow space for \0
volatile unsigned long last_interrupt_time;

// Global variables
Signal pulse;

// Setup code, to run once in the beginning
void setup()
{
  if(use_serial_for_debugging)
    Serial.begin(9600);

  pinMode(input_pin_for_bit_position, INPUT);
  pinMode(input_pin_for_bit_value, INPUT);
  attachInterrupt(digitalPinToInterrupt(input_pin_for_bit_position), read_pulse, RISING);

  screen.init_screen();

  // Reset variables for the first pulse
  bits_read = 0;
  last_interrupt_time = millis();
}


// Main loop, to run repeatedly
void loop()
{
  // Wait until a single pulse is read, i.e. more than pulse_length_ms since last interrupt
  while( bits_read < 96 && millis() - last_interrupt_time < max_wait_for_pulse_ms) {
    delay(pulse_length_ms);
  }

  // Disable interrupts while reading depth from input_signal table
  noInterrupts();
 
  // The input signal read in the interrupt is stored in input_signal variable
  // Add end-of-string character
  input_signal[bits_read] = '\0';
  
  // Convert binary interrupt signal to depth string
  pulse.convert_binary_signal_to_depth((char*)input_signal, bits_read);

  // Activate interrupts now depth is read
  interrupts();

  print_debugging_information();
  bits_read = 0;

  // Check signal for correctness. If signal is not correct use previous depth value and try to reread
  pulse.check_signal();

  // Update lcd with the value stored in depth variable
  screen.update_screen(pulse.get_depth());
  
  // Save depth variable so in case of unsuccessful signal read previous value can be displayed
  pulse.prev_depth_from_depth();
}


// Function used in the interrupt to convert signals to bits
void read_pulse()
{
  // Check if start of a new signal - if yes, reset bits_read
  if(millis() - last_interrupt_time > pulse_length_ms || bits_read > max_pulse_bits)
    bits_read = 0;
    
  if(digitalRead(input_pin_for_bit_value) == HIGH) {
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
  Serial.print(pulse.get_depth());
  Serial.print("'");
  Serial.println();
  Serial.println();
}
