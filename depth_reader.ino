
// include the library code:
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 7, 6, 5, 4);

// char arrays
char lcd_line_1[20];
char lcd_line_1_prev[20];


int channel_1 = 2; // Channel for peak positions (black)
int channel_2 = 3; // Channel for bit information at peak positions (brown)
volatile unsigned int pulses;
volatile unsigned int counter = 0;
volatile char input_signal[100];
char depth[5];
char prev_depth[5] = "--.-";
char* input_signal_print;
bool signal_reread = false;
volatile unsigned long last_interrupt_time;
const int pulse_length_millis = 10+40; // Actual pulse length 10 ms, 40 ms for safety margin (pulses come every 350 ms)
 
void setup() 
{
  Serial.begin(9600); 
   
  pinMode(channel_1, INPUT);
  pinMode(channel_2, INPUT);
  attachInterrupt(digitalPinToInterrupt(channel_1), count_pulse, RISING); 

  // set up the number of columns and rows on the LCD
  lcd.begin(16, 2);
}
 
void loop() 
{ 
  pulses = 0;
  counter = 0;
  last_interrupt_time = millis();
  interrupts(); 
  delay(500); 
  noInterrupts();

  input_signal[counter] = '\0';
  input_signal_print = input_signal;
  convert_binary_signal_to_depth();
   
  Serial.print("Pulses per second: "); 
  Serial.println(pulses);
  Serial.print("Number of bits in the first full pulse: "); 
  Serial.println(counter);
  Serial.print("Bits: "); 
  Serial.println(input_signal_print);
  Serial.print("Depth: "); 
  Serial.println(depth);
  Serial.println();

  
  if(check_signal() == false) {
    if(signal_reread == true) {
      strncpy(depth, "--.-", 5);
    } else {
      strncpy(depth, prev_depth, 5);
    }
    signal_reread = !signal_reread;    
  } else {
    signal_reread = false;
  }
  update_lcd();
  strncpy(prev_depth, depth, 5);
}

bool check_signal() {
  if(strcmp(depth, "   ") == 0)
    return false;

  if(depth[0] == '-' || depth[1] == '-'  || depth[2] == '-'  || depth[3] == '-' )
    return false;

  if(depth[2] == '.' && prev_depth[2] != '.')
    return false;

  if(depth[2] != '.' && prev_depth[2] == '.')
    return false;

  return true;
}
 
void count_pulse() 
{
  // Check if new pulse is coming
  if(millis() - last_interrupt_time > pulse_length_millis) {    
    pulses++;
  }
  // Measure first full pulse
  if(pulses == 1) {
    if(digitalRead(channel_2) == HIGH) {
      input_signal[counter] = '1';
    } else {
      input_signal[counter] = '0';
    }
     
    counter++;
  }
  
  last_interrupt_time = millis();
}


void update_lcd() {
  snprintf(lcd_line_1, 20, "Depth: %s m", depth);

  // Update screen only if there has been a change
  if(strcmp(lcd_line_1, lcd_line_1_prev) != 0) {
    // clean up the screen before printing a new reply
    lcd.clear();
    // set the cursor to column 0, line 0
    lcd.setCursor(0, 0);
    // print line 1
    lcd.print(lcd_line_1); 
  }
  strncpy(lcd_line_1_prev, lcd_line_1, 20);
}


void convert_binary_signal_to_depth() {
  // Read digits (left to right) from input signal
  byte pos = 0;
  depth[pos++] = convert_7bits_to_char(&input_signal_print[counter-7]);
  depth[pos++] = convert_7bits_to_char(&input_signal_print[counter-14]);
  // Check if decimal point is present
  if(input_signal_print[counter-23] == '1') {
    depth[pos++] = '.';
  }  
  depth[pos++] = convert_7bits_to_char(&input_signal_print[counter-21]);
  depth[pos++] = '\0';
}


char convert_7bits_to_char(char* bits) {
  byte digit = 0;

  for(int i = 0; i < 7; i++) {
    if(bits[i] == '1') {
       digit += (1 << (6-i));
    }
  }
  return convert_byte_to_char(digit);
}


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
