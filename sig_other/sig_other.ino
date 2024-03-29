#include <stdio.h>
#include <EEPROM.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <LiquidCrystal.h>
#include <Wire.h>

// This project started with code from the K3NG cw keyer (version 2012101701) by Anthony Good, K3NG. Large chunks of that code have
// been/will be deleted, with consequent loss of that functionality. The current K3NG code can be found at a repository at 
// SourceForge: http://sourceforge.net/projects/k3ngarduinocwke

// All trademarks referred to in source code and documentation are copyright their respective owners.

/*
    This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// If you offer a hardware kit using this software, show your appreciation by sending the author a note and then forward a
// complimentary kit or a bottle of bourbon to K3NG ;-)

#define CODE_VERSION "1"

//Major changes in functionality versus the K3NG CW Keyer:
// Features removed entirely:
// * Delete PS2 keyboard
// * Delete Hellscreiber
// * Disable ability to not have a potentiometer - The potentiometer is an integral part of SO and the only way to set speed
// * Disable multiple transmitters - a single keyer output remains
// * Disable all PTT lines
// * Delete Memory Macros
// * Removed Winkey functions
// * Removed call sign practice
// * Removed the DL2SBA bankswitch and reverse button options
// * Removed memory repeat with button + dit hit
// * Removed memory repeat command (if you want a memory repeated, just hit the button again)
//
// SO incorporates a 16 x 2  HD4480-based LCD: 16x2 display with LED backlight (Adafruit #181) with I2C address 0h



// Command Line Interface ("CLI") (USB Port) (Note: turn on carriage return if using Arduino Serial Monitor program)
//    CW Keyboard: type what you want the keyer to send (all commands are preceded with a backslash ( \ )
//    \?     Help                                      (requires FEATURE_SERIAL_HELP)
//    \#     Play memory #                             (requires FEATURES_MEMORIES; can only play memories 1 - 9)
//    \a     Iambic A mode
//    \b     Iambic B mode
//    \d     Ultimatic mode
//    \e#### Set serial number to ####
//    \f#### Set sidetone frequency to #### hertz
//    \g     Bug modehg co
//    \i     Transmit enable/disable
//    \j###  Dah to dit ratio (300 = 3.00)
//    \l##   Set weighting (50 = normal)
//    \m###  Set Farnsworth speed
//    \n     Toggle paddle reverse
//    \o     Toggle sidetone on/off
//    \p#    Program memory #
//    \q##   Switch to QRSS mode, dit length ## seconds
//    \r     Switch to regular speed mode\\
//    \s     Status
//    \t     Tune mode
//    \y#    Change wordspace to # elements (# = 1 to 9)
//    \z     Autospace on/off
//    \+     Create prosign
//    \*     Toggle paddle echo
//    \^     Toggle wait for carriage return to send CW / send CW immediately
//    \~     Reset unit

// Buttons
//    button 0: command mode / command mode exit
//    button 0 + left paddle:  increase cw speed
//    button 0 + right paddle: decrease cw speed

// Command Mode (press button0 to enter command mode and press again to exit)
//    A  Switch to Iambic A mode
//    B  Switch to Iambic B mode
//    D  Switch to Ultimatic mode
//    F  Adjust sidetone frequency
//    G  Switch to bug mode
//    I  TX enable / disable
//    J  Dah to dit ratio adjust
//    N  Toggle paddle reverse
//    O  Toggle sidetone on / off
//    P#(#) Program a memory
//    T  Tune mode
//    W  Change speed
//    X  Exit command mode (you can also press the command button (button0) to exit)
//    Z  Autospace On/Off
//    #  Play a memory without transmitting

// Useful Stuff
//    Reset to defaults: squeeze both paddles at power up (good to use if you dorked up the speed and don't have the CLI)
//    Press the right paddle to enter straight key mode at power up
//    Press the left  paddle at power up to enter and stay forever in beacon mode

// compile time features and options - comment or uncomment to add or delete features
// FEATURES add more bytes to the compiled binary, OPTIONS change code behavior
#define FEATURE_SERIAL
#define FEATURE_COMMAND_LINE_INTERFACE        // this requires FEATURE_SERIAL
#define FEATURE_SAY_HI
#define FEATURE_MEMORIES
//#define FEATURE_BEACON
//#define FEATURE_SERIAL_HELP
//#define FEATURE_DEAD_OP_WATCHDOG
//#define FEATURE_AUTOSPACE
//#define FEATURE_FARNSWORTH
//#define OPTION_NON_ENGLISH_EXTENSIONS  // add support for additional CW characters (i.e. √Ä, √Ö, √û, etc.)


//#define OPTION_SUPPRESS_SERIAL_BOOT_MSG
#define OPTION_PROG_MEM_TRIM_TRAILING_SPACES         // trim trailing spaces from memory when programming in command mode
//#define OPTION_WATCHDOG_TIMER                        // this enables a four second ATmega48/88/168/328 watchdog timer; use for unattended/remote operation

// don't touch these unless you know what the hell you are doing
//#define DEBUG_STARTUP
//#define DEBUG_LOOP
//#define DEBUG_EEPROM
//#define DEBUG_MEMORIES
//#define DEBUG_PLAY_MEMORY
//#define DEBUG_SEND_CHAR
//#define DEBUG_MEMORY_WRITE
//#define DEBUG_MEMORYCHECK
//#define DEBUG_CAPTURE_COM_PORT
//#define DEBUG_CHECK_SERIAL
//#define FEATURE_FREQ_COUNTER                 // not implemented yet
//#define DEBUG_VARIABLE_DUMP
//#define DEBUG_BUTTONS
//#define DEBUG_COMMAND_MODE
//#define DEBUG_GET_CW_INPUT_FROM_USER
//#define DEBUG_POTENTIOMETER

// Workaround for http://gcc.gnu.org/bugzilla/show_bug.cgi?id=34734
//   #undef PROGMEM
//   #define PROGMEM __attribute__((section(".progmem.data")))
// However this breaks Serial.print(F("")); functionality, so I manually replaced PROGMEMs with this macro - K3NG

// Pins

#define paddle_left 7
#define paddle_right 8
#define tx_key_line 6       // (high = key down/tx on)
#define sidetone_line 9         // connect a speaker for sidetone
#define potentiometer A0        // Speed potentiometer (0 to 5 V) Use pot from 1k to 10k
#define analog_buttons_pin A3

// Initial and hardcoded settings
#define initial_speed_wpm 20             // "factory default" keyer speed setting
#define initial_sidetone_freq 600        // "factory default" sidetone frequency setting
#define hz_high_beep 1500                // frequency in hertz of high beep
#define hz_low_beep 400                  // frequency in hertz of low beep
#define initial_dah_to_dit_ratio 300     // 300 = 3 / normal 3:1 ratio
#define initial_qrss_dit_length 1        // QRSS dit length in seconds
#define initial_pot_wpm_low_value 13     // Potentiometer WPM fully CCW
#define initial_pot_wpm_high_value 35    // Potentiometer WPM fully CW
#define potentiometer_change_threshold 1 // don't change the keyer speed until pot wpm has changed more than this
#define default_serial_baud_rate 115200
#define send_buffer_size 50
#define default_length_letterspace 3
#define default_length_wordspace 7
#define default_keying_compensation 0    // number of milliseconds to extend all dits and dahs - for QSK on boatanchors
#define default_first_extension_time 0   // number of milliseconds to extend first sent dit or dah
#define default_pot_full_scale_reading 1023
#define default_weighting 50             // 50 = weighting factor of 1 (normal)
#define number_of_memories byte(3)            // the number of memories (duh)
#define memory_area_start 20             // the eeprom location where memory space starts
#define memory_area_end 1023             // the eeprom location where memory space ends
#define lcd_columns 16
#define lcd_rows 2
#define eeprom_magic_number 73
#define program_memory_limit_consec_spaces 1
#define serial_leading_zeros 1            // set to 1 to activate leading zeros in serial numbers (i.e. #1 = 001)
#define serial_cut_numbers 0              // set to 1 to activate cut numbers in serial numbers (i.e. #10 = 1T, #19 = 1N)
#define analog_buttons_number_of_buttons 4

// Variable macros
#define STRAIGHT 1
#define IAMBIC_B 2
#define IAMBIC_A 3
#define BUG 4
#define ULTIMATIC 5

#define PADDLE_NORMAL 0
#define PADDLE_REVERSE 1

#define NORMAL 0
#define BEACON 1
#define COMMAND 2

#define OMIT_LETTERSPACE 1

#define SIDETONE_OFF 0
#define SIDETONE_ON 1
#define SIDETONE_PADDLE_ONLY 2

#define SENDING_NOTHING 0
#define SENDING_DIT 1
#define SENDING_DAH 2

#define SPEED_NORMAL 0
#define SPEED_QRSS 1

#define SERIAL_SEND_BUFFER_SPECIAL_START     129  
#define SERIAL_SEND_BUFFER_WPM_CHANGE        130
#define SERIAL_SEND_BUFFER_TIMED_KEY_DOWN    131
#define SERIAL_SEND_BUFFER_TIMED_WAIT        132
#define SERIAL_SEND_BUFFER_NULL              133
#define SERIAL_SEND_BUFFER_PROSIGN           134
#define SERIAL_SEND_BUFFER_HOLD_SEND         135
#define SERIAL_SEND_BUFFER_HOLD_SEND_RELEASE 136
#define SERIAL_SEND_BUFFER_MEMORY_NUMBER     137
#define SERIAL_SEND_BUFFER_SPECIAL_END       138

#define SERIAL_SEND_BUFFER_NORMAL 0
#define SERIAL_SEND_BUFFER_TIMED_COMMAND 1
#define SERIAL_SEND_BUFFER_HOLD 2

#define AUTOMATIC_SENDING 0
#define MANUAL_SENDING 1

#define ULTIMATIC_NORMAL 0
#define ULTIMATIC_DIT_PRIORITY 1
#define ULTIMATIC_DAH_PRIORITY 2


// Variables and stuff
LiquidCrystal lcd(0);

unsigned int wpm;
byte command_mode_disable_tx = 0;
unsigned int hz_sidetone = initial_sidetone_freq;
unsigned int dah_to_dit_ratio = initial_dah_to_dit_ratio;
byte qrss_dit_length = initial_qrss_dit_length;
byte machine_mode;   // NORMAL, BEACON, COMMAND
byte paddle_mode;    // PADDLE_NORMAL, PADDLE_REVERSE
byte keyer_mode;     // STRAIGHT, IAMBIC_A, IAMBIC_B, BUG
byte sidetone_mode;  // SIDETONE_OFF, SIDETONE_ON, SIDETONE_PADDLE_ONLY
byte key_tx;         // 0 = tx_key_line control suppressed
byte dit_buffer;     // used for buffering paddle hits in iambic operation
byte dah_buffer;     // used for buffering paddle hits in iambic operation
byte button0_buffer;
byte being_sent;     // SENDING_NOTHING, SENDING_DIT, SENDING_DAH
byte key_state;      // 0 = key up, 1 = key down
byte config_dirty;
byte speed_mode = SPEED_NORMAL;
unsigned int serial_number = 1;
byte pause_sending_buffer = 0;
byte length_letterspace = default_length_letterspace;
byte length_wordspace = default_length_wordspace;
byte keying_compensation = default_keying_compensation;
byte first_extension_time = default_first_extension_time;
byte weighting = default_weighting;
byte ultimatic_mode = ULTIMATIC_NORMAL;
byte last_sending_type = MANUAL_SENDING;
byte zero = 0;
byte iambic_flag = 0;

enum lcd_statuses {
  LCD_CLEAR, LCD_REVERT, LCD_TIMED_MESSAGE, LCD_SCROLL_MSG};
#define default_display_msg_delay 1000

byte lcd_status = LCD_CLEAR;
unsigned long lcd_timed_message_clear_time = 0;
byte lcd_previous_status = LCD_CLEAR;
byte lcd_scroll_buffer_dirty = 0;
String lcd_scroll_buffer[lcd_rows];
byte lcd_scroll_flag = 0;
byte lcd_paddle_echo = 1; // should sent dit/dahs echo to the lcd panel 1=yes
byte lcd_send_echo = 1; 
byte lcd_paddle_echo_buffer = 1;  // variable in which send dit/dah builds a character bit by bit
unsigned long lcd_paddle_echo_buffer_decode_time = 0;

#ifdef DEBUG_VARIABLE_DUMP
long dit_start_time;
long dit_end_time;
long dah_start_time;
long dah_end_time;
#endif

//These values assume 4 button front panel. 
//Voltage divided comprised of a 10k R1, and switched values of R2 which are, from switch 0 to 3:
//0, 33, 15, and 10k.  Values are calculated such that chording 2+3 results in a unique value, allowing
//these buttons to be used in a pinch for iambic keying...like when you forget to bring the paddles,
//the connecting wire, the 3.5mm adapter, etc.
//calculations are based on up to 10% tolerance resistors. http://goo.gl/pN383
int button_array_high_limit[analog_buttons_number_of_buttons+1] = {
  100,819,661,562,336};
int button_array_low_limit[analog_buttons_number_of_buttons+1] = {
  0,746, 563, 460,432};
long button_last_add_to_send_buffer_time = 0;

#ifdef FEATURE_FARNSWORTH
unsigned int wpm_farnsworth = 0;  // 0 = disabled
#endif

byte pot_wpm_low_value;
byte pot_wpm_high_value;
byte last_pot_wpm_read;
int pot_full_scale_reading = default_pot_full_scale_reading;

#ifdef FEATURE_SERIAL
byte incoming_serial_byte;
long serial_baud_rate;
#ifdef FEATURE_COMMAND_LINE_INTERFACE
byte serial_backslash_command;
byte cli_paddle_echo = 0;  // should sent dit/dahs echo for display over the serial port? 1=yes
                           // enabling this *and* the lcd_echo introduces latency at moderate code speeds
byte cli_paddle_echo_buffer = 1;  // variable where send dit/dah builds a character, bit by bit
unsigned long cli_paddle_echo_buffer_decode_time = 0;
byte cli_prosign_flag = 0;
byte cli_wait_for_cr_to_send_cw = 0;
#endif //FEATURE_COMMAND_LINE_INTERFACE
#endif //FEATURE_SERIAL

byte send_buffer_array[send_buffer_size];
byte send_buffer_bytes = 0;
byte send_buffer_status = SERIAL_SEND_BUFFER_NORMAL;

#ifdef FEATURE_MEMORIES
byte play_memory_prempt = 0;
long last_memory_button_buffer_insert = 0;
#endif //FEATURE_MEMORIES

#ifdef FEATURE_SERIAL
#ifdef FEATURE_COMMAND_LINE_INTERFACE
prog_uchar string_k3ng_keyer[] __attribute__((section(".progmem.data"))) = {
  "\n\rK3NG Keyer Version "};
#ifdef FEATURE_SERIAL_HELP
prog_uchar string_enter_help[] __attribute__((section(".progmem.data"))) = {
  "\n\rEnter \\? for help\n\n\r"};
#endif
prog_uchar string_qrss_mode[] __attribute__((section(".progmem.data"))) = {
  "Setting keyer to QRSS Mode. Dit length: "};
prog_uchar string_setting_serial_number[] __attribute__((section(".progmem.data"))) = {
  "Setting serial number to "};
prog_uchar string_setting_dah_to_dit_ratio[] __attribute__((section(".progmem.data"))) = {
  "Setting dah to dit ratio to "};
#endif //FEATURE_COMMAND_LINE_INTERFACE
#endif //FEATURE_SERIAL


#ifdef FEATURE_SERIAL_HELP
#ifdef FEATURE_SERIAL
#ifdef FEATURE_COMMAND_LINE_INTERFACE
prog_uchar serial_help_string[] __attribute__((section(".progmem.data"))) = {
  "\n\rK3NG Keyer Help\n\r\n\r"
    "CLI commands:\n\r"
    "\\#\t\t: play memory # x\n\r"
    "\\A\t\t: Iambic A\n\r"
    "\\B\t\t: Iambic B\n\r"
    "\\D\t\t: Ultimatic\n\r"
    "\\E####\t\t: Set serial number to ####\n\r"
    "\\F####\t\t: Set sidetone to #### hz\n\r"
    "\\G\t\t: switch to Bug mode\n\r"
    "\\I\t\t: TX line disable/enable\n\r"
    "\\J###\t\t: Set Dah to Dit Ratio\n\r"
    "\\L##\t\t: Set weighting (50 = normal)\n\r"
#ifdef FEATURE_FARNSWORTH
    "\\M###\t\t: Set Farnsworth Speed\n\r"
#endif
    "\\N\t\t: toggle paddle reverse\n\r"
    "\\Px<string>\t: program memory #x with <string>\n\r"
    "\\Q#[#]\t\t: Switch to QRSS mode with ## second dit length\n\r"
    "\\R\t\t: Switch to regular speed (wpm) mode\n\r"
    "\\S\t\t: status report\n\r"
    "\\T\t\t: Tune mode\n\r"
    "\\W#[#][#]\t: Change WPM to ###\n\r"
    "\\Y#\t\t: Change wordspace to #\n\r"
#ifdef FEATURE_AUTOSPACE
    "\\Z\t\t: Autospace on/off\n\r"
#endif
    "\\\\\t\t: Empty keyboard send buffer\n\r"
    "\\#\t\t: Jump to memory #\n\r"
    "\\D###\t\t: Delay for ### seconds\n\r"
    "\\E\t\t: Send serial number\n\r"
    "\\F####\t\t: Set sidetone to #### hz\n\r"
    "\\N\t\t: Decrement serial number\n\r"
    "\\Q##\t\t: Switch to QRSS with ## second dit length\n\r"
    "\\R\t\t: Switch to regular speed mode\n\r"
    "\\T###\t\t: Transmit for ### seconds\n\r"
    "\\^\t\t: Toggle send CW immediately\n\r"
    "\\+\t\t: Prosign\n\r"};
#endif
#endif
#endif

// EEPROM Memory Locations
#define EEPROM_wpm_high 1
#define EEPROM_wpm_low 2
#define EEPROM_paddle_mode 3
#define EEPROM_keyer_mode 4
#define EEPROM_sidetone_mode 5
#define EEPROM_hz_sidetone_low 6
#define EEPROM_hz_sidetone_high 7
#define EEPROM_dah_to_dit_ratio_low 8
#define EEPROM_dah_to_dit_ratio_high 9
//eprom location 10 available
#define EEPROM_length_wordspace 11
#ifdef FEATURE_AUTOSPACE
#define EEPROM_autospace_active 12
#endif //FEATURE_AUTOSPACE

#define SIDETONE_HZ_LOW_LIMIT 299
#define SIDETONE_HZ_HIGH_LIMIT 2001

#ifdef FEATURE_DEAD_OP_WATCHDOG
byte dead_op_watchdog_active = 1;
byte dit_counter = 0;
byte dah_counter = 0;
#endif

#ifdef FEATURE_AUTOSPACE
byte autospace_active = 1;
#endif


//---------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------

void setup()
{

  initialize_pins();

  key_state = 0;
  key_tx = 1;
  wpm = initial_speed_wpm;

  pot_wpm_low_value = initial_pot_wpm_low_value;

  pinMode(potentiometer,INPUT);
  pot_wpm_high_value = initial_pot_wpm_high_value;
  last_pot_wpm_read = pot_value_wpm();

  // setup default modes
  machine_mode = NORMAL;
  paddle_mode = PADDLE_NORMAL;
  keyer_mode = IAMBIC_B;
  sidetone_mode = SIDETONE_ON;

  delay(250);  // wait a little bit for the caps to charge up on the paddle lines

#ifdef OPTION_WATCHDOG_TIMER
  wdt_enable(WDTO_4S);
#endif //OPTION_WATCHDOG_TIMER

  // do an eeprom reset to defaults if paddles are squeezed
  if (digitalRead(paddle_left) == LOW && digitalRead(paddle_right) == LOW) {
    while (digitalRead(paddle_left) == LOW && digitalRead(paddle_right) == LOW) {
    }
    write_settings_to_eeprom(1);
    beep_boop();
    beep_boop();
    beep_boop();
  }

  // read settings from eeprom and initialize eeprom if it has never been written to
  if (read_settings_from_eeprom()) {
    write_settings_to_eeprom(1);
    beep_boop();
    beep_boop();
    beep_boop();
  }

  // check for beacon mode (paddle_left == low) or straight key mode (paddle_right == low)
  if (digitalRead(paddle_left) == LOW) {
#ifdef FEATURE_BEACON
    machine_mode = BEACON;
#endif
  } 
  else {
    if (digitalRead(paddle_right) == LOW) {
      keyer_mode = STRAIGHT;
    }
  }

#ifdef DEBUG_CAPTURE_COM_PORT
  Serial.begin(serial_baud_rate);
  debug_capture();
#endif


  // initialize serial port
#ifdef FEATURE_SERIAL
#ifdef FEATURE_COMMAND_LINE_INTERFACE
  serial_baud_rate = default_serial_baud_rate;
#endif // FEATURE_COMMAND_LINE_INTERFACE

  Serial.begin(serial_baud_rate);

#ifdef DEBUG_STARTUP
  Serial.println(F("setup: serial port opened"));
#endif

#ifndef OPTION_SUPPRESS_SERIAL_BOOT_MSG
#ifdef FEATURE_COMMAND_LINE_INTERFACE
  serial_print(string_k3ng_keyer);
  Serial.write(CODE_VERSION);
#ifdef FEATURE_SERIAL_HELP
  serial_print(string_enter_help);
#endif
#ifdef DEBUG_MEMORYCHECK
  memorycheck();
#endif //DEBUG_MEMORYCHECK
#endif //FEATURE_COMMAND_LINE_INTERFACE
#endif //ifndef OPTION_SUPPRESS_SERIAL_BOOT_MSG
#endif //FEATURE_SERIAL

  lcd.begin(lcd_columns, lcd_rows);
  lcd.setBacklight(HIGH);
  lcd_center_print_timed("Testing",0,4000);

  if (machine_mode != BEACON) {
#ifdef FEATURE_SAY_HI
    // say HI
    // store current setting (compliments of DL2SBA - http://dl2sba.com/ )
    byte oldKey = key_tx; 
    byte oldSideTone = sidetone_mode;
    key_tx = 0;
    sidetone_mode = SIDETONE_ON;     

    //delay(201);
    lcd_center_print_timed("h",1,4000);
    send_char(16,NORMAL);
    lcd_center_print_timed("hi",1,4000);
    send_char(4,NORMAL);

    sidetone_mode = oldSideTone; 
    key_tx = oldKey;     
#endif //FEATURE_SAY_HI

  }

#ifdef DEBUG_STARTUP
  initialize_debug_startup();
#endif

}

// --------------------------------------------------------------------------------------------

void loop()
//Main event loop

{  
#ifdef OPTION_WATCHDOG_TIMER
  wdt_reset();
#endif  //OPTION_WATCHDOG_TIMER

#ifdef FEATURE_BEACON
#ifdef FEATURE_MEMORIES
  if (machine_mode == BEACON) {
    delay(201);
    while (machine_mode == BEACON) {  // if we're in beacon mode, just keep playing memory 0
      if (!send_buffer_bytes) {
        play_memory(0);
      }
      service_send_buffer();
#ifdef FEATURE_SERIAL
      check_serial();
#endif
    }
  }
#endif //FEATURE_MEMORIES
#endif //FEATURE_BEACON

  if (machine_mode == NORMAL) {
    check_command_buttons();
    check_paddles();
    service_dit_dah_buffers();

#ifdef FEATURE_SERIAL       
    check_serial();
    check_paddles();            
    service_dit_dah_buffers();
#ifdef FEATURE_COMMAND_LINE_INTERFACE  
    service_serial_paddle_echo();
#endif //FEATURE_COMMAND_LINE_INTERFACE
#endif //FEATURE_SERIAL

    service_send_buffer();
    check_potentiometer();
    check_for_dirty_configuration();

#ifdef FEATURE_DEAD_OP_WATCHDOG
    check_for_dead_op();
#endif

    check_paddles();
    service_dit_dah_buffers();
    service_send_buffer();
    service_lcd_paddle_echo();
    service_display();

  }
}

// Subroutines --------------------------------------------------------------------------------------------


// Are you a radio artisan ?
void service_display() {

#ifdef DEBUG_LOOP
  Serial.println(F("loop: entering service_display"));
#endif    

  byte x = 0;

  if (lcd_status == LCD_REVERT) {
    lcd_status = lcd_previous_status;
    switch (lcd_status) {
    case LCD_CLEAR: 
      lcd_clear(); 
      break;
    case LCD_SCROLL_MSG:
      lcd.clear();
      for (x = 0;x < lcd_rows;x++){
        //clear_display_row(x);
        lcd.setCursor(0,x);
        lcd.print(lcd_scroll_buffer[x]);
      }         
      lcd_scroll_flag = 0; 
      lcd_scroll_buffer_dirty = 0;         
      break;
    }
  } 
  else {
    switch (lcd_status) {
    case LCD_CLEAR : 
      break;
    case LCD_TIMED_MESSAGE:
      if (millis() > lcd_timed_message_clear_time) {
        lcd_status = LCD_REVERT;
      }
    case LCD_SCROLL_MSG:
      if (lcd_scroll_buffer_dirty) { 
        if (lcd_scroll_flag) {
          lcd.clear();
          lcd_scroll_flag = 0;
        }         
        for (x = 0;x < lcd_rows;x++){
          //clear_display_row(x);
          lcd.setCursor(0,x);
          lcd.print(lcd_scroll_buffer[x]);
        }
        lcd_scroll_buffer_dirty = 0;
      }
      break;
    }
  }

}

//-------------------------------------------------------------------------------------------------------

void service_lcd_paddle_echo()
// converts lcd_paddle_echo_buffer containing a single character code to ascii 
// and sends that character to serial out, then resets the buffer

{

#ifdef DEBUG_LOOP
  Serial.println(F("loop: entering service_lcd_paddle_echo"));
#endif    

  static byte lcd_paddle_echo_space_sent = 1;

  if ((lcd_paddle_echo_buffer > 1) && (lcd_paddle_echo) && (millis() > lcd_paddle_echo_buffer_decode_time)) {
    display_scroll_print_char(byte(convert_cw_number_to_ascii(lcd_paddle_echo_buffer)));
    lcd_paddle_echo_buffer = 1;
    lcd_paddle_echo_buffer_decode_time = millis() + (float(600/wpm)*length_letterspace);
    lcd_paddle_echo_space_sent = 0;
  }
  if ((lcd_paddle_echo_buffer == 1) && (lcd_paddle_echo) && (millis() > (lcd_paddle_echo_buffer_decode_time + (float(1200/wpm)*(length_wordspace-length_letterspace)))) && (!lcd_paddle_echo_space_sent)) {
    display_scroll_print_char(' ');
    lcd_paddle_echo_space_sent = 1;
  }
}

//-------------------------------------------------------------------------------------------------------

void display_scroll_print_char(char charin){

  static byte column_pointer = 0;
  static byte row_pointer = 0;
  byte x = 0;

  if (lcd_status != LCD_SCROLL_MSG) {
    lcd_status = LCD_SCROLL_MSG;
    lcd.clear();
  }
  if (column_pointer > (lcd_columns-1)) {
    row_pointer++;
    column_pointer = 0;
    if (row_pointer > (lcd_rows-1)) {
      for (x = 0; x < (lcd_rows-1); x++) {
        lcd_scroll_buffer[x] = lcd_scroll_buffer[x+1];
      }
      lcd_scroll_buffer[x] = "";     
      row_pointer--;
      lcd_scroll_flag = 1;
    }    
  } 
  lcd_scroll_buffer[row_pointer].concat(charin);
  column_pointer++;
  lcd_scroll_buffer_dirty = 1; 
}

//-------------------------------------------------------------------------------------------------------
void lcd_clear() {

  //  for (byte x = 0;x < lcd_rows;x++) {
  //    clear_display_row(x);
  //  }
  lcd.clear();
  lcd_status = LCD_CLEAR;

}

//-------------------------------------------------------------------------------------------------------

void lcd_center_print_timed(String lcd_print_string, byte row_number, unsigned int duration)
{
  if (lcd_status != LCD_TIMED_MESSAGE) {
    lcd_previous_status = lcd_status;
    lcd_status = LCD_TIMED_MESSAGE;
    lcd.clear();
  } 
  else {
    clear_display_row(row_number);
  }
  lcd.setCursor(((lcd_columns - lcd_print_string.length())/2),row_number);
  lcd.print(lcd_print_string);
  lcd_timed_message_clear_time = millis() + duration;
}

//-------------------------------------------------------------------------------------------------------

void clear_display_row(byte row_number)
{
  for (byte x = 0; x < lcd_columns; x++) {
    lcd.setCursor(x,row_number);
    lcd.print(" ");
  }
}

//-------------------------------------------------------------------------------------------------------

void check_for_dirty_configuration()
{
#ifdef DEBUG_LOOP
  Serial.println(F("loop: entering check_for_dirty_configuration"));
#endif

  static long last_config_write;

  if ((config_dirty) && ((millis()-last_config_write)>30000)) {
    write_settings_to_eeprom(0);
    last_config_write = millis();
#ifdef DEBUG_EEPROM
    Serial.println(F("check_for_dirty_configuration: wrote config\n"));
#endif
  }

}


//-------------------------------------------------------------------------------------------------------

#ifdef FEATURE_DEAD_OP_WATCHDOG
void check_for_dead_op()

// if the dit or dah paddle is stuck, disable the transmitter line after 100 straight dits or dahs
// go in and out of command mode to clear or just reset the unit

{
#ifdef DEBUG_LOOP
  Serial.println(F("loop: entering check_for_dead_op"));
#endif    

  if (dead_op_watchdog_active && ((dit_counter > 100) || (dah_counter > 100))) {
    key_tx = 0;
  }
}
#endif
//-------------------------------------------------------------------------------------------------------

#ifdef DEBUG_CAPTURE_COM_PORT
void debug_capture ()
{

  byte serial_byte_in;
  int x = 1022;

  while (Serial.available() == 0) {
  }  // wait for first byte
  serial_byte_in = Serial.read();
  Serial.write(serial_byte_in);
  //if ((serial_byte_in > 47) or (serial_byte_in = 20)) { Serial.write(serial_byte_in); }  // echo back
  if (serial_byte_in == '~') {
    debug_capture_dump();    // go into dump mode if we get a tilde
  } 
  else {
    EEPROM.write(x,serial_byte_in);
    x--;
    while ( x > 400) {
      if (Serial.available() > 0) {
        serial_byte_in = Serial.read();
        EEPROM.write(x,serial_byte_in);
        EEPROM.write(x-1,255);
        send_dit(AUTOMATIC_SENDING);
        x--;
        Serial.write(serial_byte_in);
        //if ((serial_byte_in > 47) or (serial_byte_in = 20)) { Serial.write(serial_byte_in); }  // echo back
      }
    }
  }

  while (1) {
  }

}
#endif

//-------------------------------------------------------------------------------------------------------

#ifdef DEBUG_CAPTURE_COM_PORT
void debug_capture_dump()
{
  byte eeprom_byte_in;

  for ( int x = 1022; x > (1022-100); x-- ) {
    eeprom_byte_in = EEPROM.read(x);
    if (eeprom_byte_in < 255) {
      Serial.print(eeprom_byte_in,BYTE);
    } 
    else {
      x = 0;
    }
  }
  Serial.write("\n\n\r");
  for ( int x = 1022; x > (1022-100); x-- ) {
    eeprom_byte_in = EEPROM.read(x);
    if (eeprom_byte_in < 255) {
      Serial.print(eeprom_byte_in,HEX);
      Serial.write("   :");
      Serial.println(eeprom_byte_in,BYTE);
    } 
    else {
      x = 0;
    }
  }

  while (1) {
  }

}
#endif

//-------------------------------------------------------------------------------------------------------

void check_potentiometer()
{
#ifdef DEBUG_LOOP
  Serial.println(F("loop: entering check_potentiometer")); 
#endif        

  byte pot_value_wpm_read = pot_value_wpm();
  if ((abs(pot_value_wpm_read - last_pot_wpm_read) > potentiometer_change_threshold)) {
#ifdef DEBUG_POTENTIOMETER
    Serial.print(F("check_potentiometer: speed change: "));
    Serial.print(pot_value_wpm_read);
    Serial.print(F(" analog read: "));
    Serial.println(analogRead(potentiometer));
#endif
    speed_set(pot_value_wpm_read);
    last_pot_wpm_read = pot_value_wpm_read;
  }
}

//-------------------------------------------------------------------------------------------------------

byte pot_value_wpm()
{
  int pot_read = analogRead(potentiometer);
  byte return_value = map(pot_read, 0, pot_full_scale_reading, pot_wpm_low_value, pot_wpm_high_value);
  return return_value;

}

//-------------------------------------------------------------------------------------------------------

#ifdef FEATURE_MEMORIES
void put_memory_button_in_buffer(byte memory_number_to_put_in_buffer)
{

  if (memory_number_to_put_in_buffer < number_of_memories) {
#ifdef DEBUG_MEMORIES
    Serial.print(F("put_memory_button_in_buffer: memory_number_to_put_in_buffer:"));
    Serial.println(memory_number_to_put_in_buffer,DEC);
#endif
    if ((millis() - last_memory_button_buffer_insert) > 400) {    // don't do another buffer insert if we just did one - button debounce
      add_to_send_buffer(SERIAL_SEND_BUFFER_MEMORY_NUMBER);
      add_to_send_buffer(memory_number_to_put_in_buffer);
      last_memory_button_buffer_insert = millis();
    }
  } 
  else {
#ifdef DEBUG_MEMORIES
    Serial.println(F("put_memory_button_in_buffer: bad memory_number_to_put_in_buffer"));
#endif
  }
}
#endif

//-------------------------------------------------------------------------------------------------------

void check_paddles()
//determines which paddle has been closed (via check_dit_paddle and check_day paddle) and keeps track of the 
// last_closure state, appropriate to each mode (iambic, ultimatic, etc.)

{

#ifdef DEBUG_LOOP
  Serial.println(F("loop: entering check_paddles"));
#endif  

#define NO_CLOSURE 0
#define DIT_CLOSURE_DAH_OFF 1
#define DAH_CLOSURE_DIT_OFF 2
#define DIT_CLOSURE_DAH_ON 3
#define DAH_CLOSURE_DIT_ON 4

  static byte last_closure = NO_CLOSURE;

  check_dit_paddle();
  check_dah_paddle();

  if (keyer_mode == ULTIMATIC) {
    if (ultimatic_mode == ULTIMATIC_NORMAL) {
      switch (last_closure) {
      case DIT_CLOSURE_DAH_OFF:
        if (dah_buffer) {
          if (dit_buffer) {
            last_closure = DAH_CLOSURE_DIT_ON;
            dit_buffer = 0;
          } 
          else {
            last_closure = DAH_CLOSURE_DIT_OFF;
          }
        } 
        else {
          if (!dit_buffer) {
            last_closure = NO_CLOSURE;
          }
        }
        break;
      case DIT_CLOSURE_DAH_ON:
        if (dit_buffer) {
          if (dah_buffer) {
            dah_buffer = 0;
          } 
          else {
            last_closure = DIT_CLOSURE_DAH_OFF;
          }
        } 
        else {
          if (dah_buffer) {
            last_closure = DAH_CLOSURE_DIT_OFF;
          } 
          else {
            last_closure = NO_CLOSURE;
          }
        }
        break;

      case DAH_CLOSURE_DIT_OFF:
        if (dit_buffer) {
          if (dah_buffer) {
            last_closure = DIT_CLOSURE_DAH_ON;
            dah_buffer = 0;
          } 
          else {
            last_closure = DIT_CLOSURE_DAH_OFF;
          }
        } 
        else {
          if (!dah_buffer) {
            last_closure = NO_CLOSURE;
          }
        }
        break;

      case DAH_CLOSURE_DIT_ON:
        if (dah_buffer) {
          if (dit_buffer) {
            dit_buffer = 0;
          } 
          else {
            last_closure = DAH_CLOSURE_DIT_OFF;
          }
        } 
        else {
          if (dit_buffer) {
            last_closure = DIT_CLOSURE_DAH_OFF;
          } 
          else {
            last_closure = NO_CLOSURE;
          }
        }
        break;

      case NO_CLOSURE:
        if ((dit_buffer) && (!dah_buffer)) {
          last_closure = DIT_CLOSURE_DAH_OFF;
        } 
        else {
          if ((dah_buffer) && (!dit_buffer)) {
            last_closure = DAH_CLOSURE_DIT_OFF;
          } 
          else {
            if ((dit_buffer) && (dah_buffer)) {
              // need to handle dit/dah priority here
              last_closure = DIT_CLOSURE_DAH_ON;
              dah_buffer = 0;
            }
          }
        }
        break;
      }
    } 
    else {
      if ((dit_buffer) && (dah_buffer)) {   // dit or dah priority mode
        if (ultimatic_mode == ULTIMATIC_DIT_PRIORITY) {
          dah_buffer = 0;
        } 
        else {
          dit_buffer = 0;
        }
      }
    }
  }
}

//-------------------------------------------------------------------------------------------------------
void write_settings_to_eeprom(int initialize_eeprom) {

  if (initialize_eeprom) {
#ifdef FEATURE_MEMORIES
    initialize_eeprom_memories();
#endif  //FEATURE_MEMORIES
    EEPROM.write(0,eeprom_magic_number);
  }

  EEPROM.write(EEPROM_wpm_high,highByte(wpm));
  EEPROM.write(EEPROM_wpm_low,lowByte(wpm));
  EEPROM.write(EEPROM_paddle_mode,paddle_mode);
  if (keyer_mode == STRAIGHT) {
    EEPROM.write(EEPROM_keyer_mode,IAMBIC_B);
  } 
  else {
    EEPROM.write(EEPROM_keyer_mode,keyer_mode);
  }
  EEPROM.write(EEPROM_sidetone_mode,sidetone_mode);
  EEPROM.write(EEPROM_hz_sidetone_low,lowByte(hz_sidetone));
  EEPROM.write(EEPROM_hz_sidetone_high,highByte(hz_sidetone));
  EEPROM.write(EEPROM_dah_to_dit_ratio_low,lowByte(dah_to_dit_ratio));
  EEPROM.write(EEPROM_dah_to_dit_ratio_high,highByte(dah_to_dit_ratio));
  EEPROM.write(EEPROM_length_wordspace,length_wordspace);

#ifdef FEATURE_AUTOSPACE
  EEPROM.write(EEPROM_autospace_active,autospace_active);
#endif //FEATURE_AUTOSPACE

  config_dirty = 0;
}

//-------------------------------------------------------------------------------------------------------

int read_settings_from_eeprom() {

  // returns 0 if eeprom had valid settings, returns 1 if eeprom needs initialized

  if (EEPROM.read(0) == eeprom_magic_number) {
    wpm = word(EEPROM.read(EEPROM_wpm_high),EEPROM.read(EEPROM_wpm_low));
    paddle_mode = EEPROM.read(EEPROM_paddle_mode);
    keyer_mode = EEPROM.read(EEPROM_keyer_mode);
    sidetone_mode = EEPROM.read(EEPROM_sidetone_mode);
    hz_sidetone = word(EEPROM.read(EEPROM_hz_sidetone_high),EEPROM.read(EEPROM_hz_sidetone_low));
    dah_to_dit_ratio = word(EEPROM.read(EEPROM_dah_to_dit_ratio_high),EEPROM.read(EEPROM_dah_to_dit_ratio_low));
    length_wordspace = EEPROM.read(EEPROM_length_wordspace);

#ifdef FEATURE_AUTOSPACE
    autospace_active = EEPROM.read(EEPROM_autospace_active);
#endif //FEATURE_AUTOSPACE

    config_dirty = 0;
    return 0;
  } 
  else {
    return 1;
  }

}

//-------------------------------------------------------------------------------------------------------

void check_dit_paddle()
{
  byte pin_value = 0;
  byte dit_paddle = 0;

  if (paddle_mode == PADDLE_NORMAL) {
    dit_paddle = paddle_left;
  } 
  else {
    dit_paddle = paddle_right;
  }
  pin_value = digitalRead(dit_paddle);

  if (pin_value == 0) {
#ifdef FEATURE_DEAD_OP_WATCHDOG
    if (dit_buffer == 0) {
      dit_counter++;
      dah_counter = 0;
    }
#endif
    dit_buffer = 1;
  }

}

//-------------------------------------------------------------------------------------------------------

void check_dah_paddle()
{
  byte pin_value = 0;
  byte dah_paddle;

  if (paddle_mode == PADDLE_NORMAL) {
    dah_paddle = paddle_right;
  } 
  else {
    dah_paddle = paddle_left;
  }
  pin_value = digitalRead(dah_paddle);
  if (pin_value == 0) {
#ifdef FEATURE_DEAD_OP_WATCHDOG
    if (dah_buffer == 0) {
      dah_counter++;
      dit_counter = 0;
    }
#endif
    dah_buffer = 1;
  }
}

//-------------------------------------------------------------------------------------------------------

void send_dit(byte sending_type)
//trigger dit sending, but also build lcd and cli (serial) paddle echo buffers bit by bit

{
  // notes: key_compensation is a straight x mS lengthening or shortening of the key down time
  //        weighting is
  
  unsigned int character_wpm = wpm;
#ifdef FEATURE_FARNSWORTH
  if ((sending_type == AUTOMATIC_SENDING) && (wpm_farnsworth > wpm)) {
    character_wpm = wpm_farnsworth;
  }
#endif //FEATURE_FARNSWORTH

  being_sent = SENDING_DIT;
  tx_and_sidetone_key(1,sending_type);
#ifdef DEBUG_VARIABLE_DUMP
  dit_start_time = millis();
#endif
  loop_element_lengths((1.0*(float(weighting)/50)),keying_compensation,character_wpm,sending_type);
#ifdef DEBUG_VARIABLE_DUMP
  dit_end_time = millis();
#endif
  tx_and_sidetone_key(0,sending_type);
  loop_element_lengths((2.0-(float(weighting)/50)),(-1.0*keying_compensation),character_wpm,sending_type);
#ifdef FEATURE_AUTOSPACE
  if ((sending_type == MANUAL_SENDING) && (autospace_active)) {
    check_paddles();
  }
  if ((sending_type == MANUAL_SENDING) && (autospace_active) && (dit_buffer == 0) && (dah_buffer == 0)) {
    loop_element_lengths(2,0,wpm,sending_type);
  }
#endif

#ifdef FEATURE_COMMAND_LINE_INTERFACE
  if ((cli_paddle_echo) && (sending_type == MANUAL_SENDING)) {
    cli_paddle_echo_buffer = cli_paddle_echo_buffer << 1;
    if (cli_paddle_echo_buffer > 127) {
      cli_paddle_echo_buffer = 76; // 01001100b = ?
    }
    cli_paddle_echo_buffer_decode_time = millis() + (float(600/wpm)*length_letterspace);
  }
#endif

  if ((lcd_paddle_echo) && (sending_type == MANUAL_SENDING)) {
    lcd_paddle_echo_buffer = lcd_paddle_echo_buffer << 1;
    if (lcd_paddle_echo_buffer > 127) {
      lcd_paddle_echo_buffer = 76; // 01001100b = ?
    }
    lcd_paddle_echo_buffer_decode_time = millis() + (float(600/wpm)*length_letterspace);
  }

  being_sent = SENDING_NOTHING;
  last_sending_type = sending_type;
  check_paddles();

}

//-------------------------------------------------------------------------------------------------------

void send_dah(byte sending_type)
//trigger dit sending, but also build lcd and cli (serial) paddle echo buffers bit by bit

{

  unsigned int character_wpm = wpm;
#ifdef FEATURE_FARNSWORTH
  if ((sending_type == AUTOMATIC_SENDING) && (wpm_farnsworth > wpm)) {
    character_wpm = wpm_farnsworth;
  }
#endif //FEATURE_FARNSWORTH

  being_sent = SENDING_DAH;
  tx_and_sidetone_key(1,sending_type);
#ifdef DEBUG_VARIABLE_DUMP
  dah_start_time = millis();
#endif
  loop_element_lengths((float(dah_to_dit_ratio/100.0)*(float(weighting)/50)),keying_compensation,character_wpm,sending_type);
#ifdef DEBUG_VARIABLE_DUMP
  dah_end_time = millis();
#endif
  tx_and_sidetone_key(0,sending_type);
  loop_element_lengths((4.0-(3.0*(float(weighting)/50))),(-1.0*keying_compensation),character_wpm,sending_type);
#ifdef FEATURE_AUTOSPACE
  if ((sending_type == MANUAL_SENDING) && (autospace_active)) {
    check_paddles();
  }
  if ((sending_type == MANUAL_SENDING) && (autospace_active) && (dit_buffer == 0) && (dah_buffer == 0)) {
    loop_element_lengths(2,0,wpm,sending_type);
  }
#endif

#ifdef FEATURE_COMMAND_LINE_INTERFACE
  if ((cli_paddle_echo) && (sending_type == MANUAL_SENDING)) {
    cli_paddle_echo_buffer = cli_paddle_echo_buffer << 1;
    cli_paddle_echo_buffer = cli_paddle_echo_buffer | 1;
    if (cli_paddle_echo_buffer > 127) {
      cli_paddle_echo_buffer = 76; // 01001100b = ?
    }
    cli_paddle_echo_buffer_decode_time = millis() + (float(600/wpm)*length_letterspace);
  }
#endif

  if ((lcd_paddle_echo) && (sending_type == MANUAL_SENDING)) {
    lcd_paddle_echo_buffer = lcd_paddle_echo_buffer << 1;
    lcd_paddle_echo_buffer = lcd_paddle_echo_buffer | 1;
    if (lcd_paddle_echo_buffer > 127) {
      lcd_paddle_echo_buffer = 76; // 01001100b = ?
    }
    lcd_paddle_echo_buffer_decode_time = millis() + (float(600/wpm)*length_letterspace);
  }

  check_paddles();

  being_sent = SENDING_NOTHING;
  last_sending_type = sending_type;

}

//-------------------------------------------------------------------------------------------------------

void tx_and_sidetone_key (int state, byte sending_type)
{
  if ((state) && (key_state == 0)) {
    if (key_tx) {
      digitalWrite (tx_key_line, HIGH);
      if (first_extension_time) {
        delay(first_extension_time);
      }
    }
    if ((sidetone_mode == SIDETONE_ON) || (machine_mode == COMMAND) || ((sidetone_mode == SIDETONE_PADDLE_ONLY) && (sending_type == MANUAL_SENDING))) {
      tone(sidetone_line, hz_sidetone);
    }
    key_state = 1;
  } 
  else {
    if ((state == 0) && (key_state)) {
      if (key_tx) {
        digitalWrite (tx_key_line, LOW);       
      }
      if ((sidetone_mode == SIDETONE_ON) || (machine_mode == COMMAND) || ((sidetone_mode == SIDETONE_PADDLE_ONLY) && (sending_type == MANUAL_SENDING))) {
        noTone(sidetone_line);
      }
      key_state = 0;
    }
  }
}

//-------------------------------------------------------------------------------------------------------

void loop_element_lengths(float lengths, float additional_time_ms, int speed_wpm_in, byte sending_type)
{

  if ((lengths == 0) or (lengths < 0)) {
    return;
  }

  float element_length;

  if (speed_mode == SPEED_NORMAL) {
    element_length = 1200/speed_wpm_in;
  } 
  else {
    element_length = qrss_dit_length * 1000;
  }

  unsigned long endtime = millis() + long(element_length*lengths) + long(additional_time_ms);
  while ((millis() < endtime) && (millis() > 200)) {  // the second condition is to account for millis() rollover
#ifdef OPTION_WATCHDOG_TIMER
    wdt_reset();
#endif  //OPTION_WATCHDOG_TIMER
    if (keyer_mode == ULTIMATIC) {
    } 
    else {
      if ((keyer_mode == IAMBIC_A) && (digitalRead(paddle_left) == LOW ) && (digitalRead(paddle_right) == LOW )) {
        iambic_flag = 1;
      }    

      if (being_sent == SENDING_DIT) {
        check_dah_paddle();
      } 
      else {
        if (being_sent == SENDING_DAH) {
          check_dit_paddle();
        }
      }
    }
#ifdef FEATURE_MEMORIES
    check_the_memory_buttons();
#endif
    // blow out prematurely if we're automatic sending and a paddle gets hit
    if (sending_type == AUTOMATIC_SENDING && (digitalRead(paddle_left) == LOW || digitalRead(paddle_right) == LOW || analogbuttonread(0) || dit_buffer || dah_buffer)) {
      if (machine_mode == NORMAL) {
        return;
      }
    }   
  }

  if ((keyer_mode == IAMBIC_A) && (iambic_flag) && (digitalRead(paddle_left) == HIGH ) && (digitalRead(paddle_right) == HIGH )) {
    iambic_flag = 0;
    dit_buffer = 0;
    dah_buffer = 0;
  }    


}

//-------------------------------------------------------------------------------------------------------

void speed_change(int change)
{
  if (((wpm + change) > 5) && ((wpm + change) < 60)) {
    speed_set(wpm + change);
  }

  lcd_center_print_timed(String(wpm) + " wpm", 1, default_display_msg_delay);
}

//-------------------------------------------------------------------------------------------------------

void speed_set(int wpm_set)
{
  wpm = wpm_set;
  config_dirty = 1;
  lcd_center_print_timed(String(wpm) + " wpm", 0, default_display_msg_delay);
}

//-------------------------------------------------------------------------------------------------------

int get_cw_input_from_user(unsigned int exit_time_seconds) {
//  Loop until something has been entered and more than a letter space interval has passed or until it hits 
//  a time out (e.g., when called from command_program_memory, while it waits for a memory number ), or a 
//  command button is hit again.  
//  Characters are encoded as values < 128, with a 1 start bit and then up to 6 dahs as 1, dits as 0.
//  Value of 128 is returned if command button is hit
//  Spaces are encoded as zero.

 
  byte looping = 1;
  byte paddle_hit = 0;
  byte cw_char = 1;
  unsigned long last_element_time = 0;
  byte button_hit = 0;
  unsigned long entry_time = millis();

  while (looping) {
    check_potentiometer();
    check_paddles();

    if (dit_buffer) {
      send_dit(MANUAL_SENDING);
      dit_buffer = 0;
      paddle_hit = 1;
      cw_char = cw_char << 1;
      last_element_time = millis();
    }
    if (dah_buffer) {
      send_dah(MANUAL_SENDING);
      dah_buffer = 0;
      paddle_hit = 1;
      cw_char = cw_char << 1;
      cw_char = cw_char | 1;
      last_element_time = millis();
    }
    if ((paddle_hit) && (millis() > (last_element_time + (float(600/wpm) * length_letterspace)))) {
#ifdef DEBUG_GET_CW_INPUT_FROM_USER
      Serial.println(F("get_cw_input_from_user: hit length_letterspace"));
#endif
      looping = 0;
    }

    if ((!paddle_hit) && (exit_time_seconds) && ((millis() - entry_time) > (exit_time_seconds * 1000))) { // if we were passed an exit time and no paddle was hit, blow out of here
      return 0;
    }
    
    if (cw_char >  127){
      return 0; 
    }

    while (analogbuttonread(0)) {    // hit the button to get out of command mode if no paddle was hit
      looping = 0;
      button_hit = 1;
    }

#ifdef FEATURE_SERIAL
    check_serial();
#endif

  }
  if (button_hit) {
#ifdef DEBUG_GET_CW_INPUT_FROM_USER
    Serial.println(F("get_cw_input_from_user: button_hit exit 128"));
#endif
    return 128;
  } 
  else {
#ifdef DEBUG_GET_CW_INPUT_FROM_USER
    Serial.print(F("get_cw_input_from_user: exiting cw_char:"));
    Serial.println(cw_char);
#endif    
    return cw_char;
  }
}

//-------------------------------------------------------------------------------------------------------

void command_mode ()
{

  machine_mode = COMMAND;

#ifdef DEBUG_COMMAND_MODE
  Serial.println(F("command_mode: entering"));
#endif

  byte cw_char;
  byte stay_in_command_mode = 1;
  byte speed_mode_before = speed_mode;
  speed_mode = SPEED_NORMAL;                 // put us in normal speed mode (life is too short to do command mode in QRSS)
  byte keyer_mode_before = keyer_mode;
  if ((keyer_mode != IAMBIC_A) && (keyer_mode != IAMBIC_B)) {
    keyer_mode = IAMBIC_B;                   // we got to be in iambic mode (life is too short to make this work in bug mode)
  }

  command_mode_disable_tx = 0;

  boop_beep();
  lcd.clear();
  lcd_center_print_timed("Command Mode", 0, default_display_msg_delay);

  while (stay_in_command_mode) {
    cw_char = 0;
    cw_char = get_cw_input_from_user(0);
#ifdef DEBUG_COMMAND_MODE
    Serial.print(F("command_mode: cwchar: "));
    Serial.println(cw_char);
#endif
    if (cw_char > 0) {              // do the command      
      switch (cw_char) {
      case 5: // 101b - A - Iambic mode
        keyer_mode = IAMBIC_A;
        keyer_mode_before = IAMBIC_A;
        config_dirty = 1;
        lcd_center_print_timed("Iambic A", 0, default_display_msg_delay);
        send_dit(AUTOMATIC_SENDING);
        break; 
      case 24: // 11000b - Iambic mode
        keyer_mode = IAMBIC_B;
        keyer_mode_before = IAMBIC_B;
        config_dirty = 1;
        lcd_center_print_timed("Iambic B", 0, default_display_msg_delay);      
        break; 
      case 22: 
        command_program_memory(); 
        break;                       // 10110 - P - program a memory
      case 128: 
        stay_in_command_mode = 0; 
        break;                          // button was hit - exit
      default: // unknown command, send a ?
        lcd_center_print_timed("Unknown command", 0, default_display_msg_delay);          
        send_char('?',NORMAL); 
        break;                                   
      }
    }
  }
  beep_boop();
  machine_mode = NORMAL;
  speed_mode = speed_mode_before;   // go back to whatever speed mode we were in before
  keyer_mode = keyer_mode_before;
#ifdef DEBUG_COMMAND_MODE
  if (command_mode_disable_tx) {
    Serial.print(F("command_mode: command_mode_disable_tx set"));
  }
#endif //DEBUG_COMMAND_MODE
}

//-------------------------------------------------------------------------------------------------------

void adjust_dah_to_dit_ratio(int adjustment) {

  if ((dah_to_dit_ratio + adjustment) > 150 && (dah_to_dit_ratio + adjustment) < 810) {
    dah_to_dit_ratio = dah_to_dit_ratio + adjustment;
  }

  config_dirty = 1;
}

//-------------------------------------------------------------------------------------------------------

void command_dah_to_dit_ratio_adjust () {

  byte looping = 1;

  lcd_center_print_timed("Adj dah to dit", 0, default_display_msg_delay);          

  while (looping) {
    send_dit(AUTOMATIC_SENDING);
    send_dah(AUTOMATIC_SENDING);
    if (digitalRead(paddle_left) == LOW) {
      adjust_dah_to_dit_ratio(10);
    }
    if (digitalRead(paddle_right) == LOW) {
      adjust_dah_to_dit_ratio(-10);
    }
    while ((digitalRead(paddle_left) == LOW && digitalRead(paddle_right) == LOW) || (analogbuttonread(0))) { // if paddles are squeezed or button0 pressed - exit
      looping = 0;
    }

  }
  while (digitalRead(paddle_left) == LOW || digitalRead(paddle_right) == LOW || analogbuttonread(0) ) {
  }  // wait for all lines to go high
  dit_buffer = 0;
  dah_buffer = 0;
}

//-------------------------------------------------------------------------------------------------------

void command_tuning_mode() {

  byte looping = 1;
  byte latched = 0;

  lcd_center_print_timed("Tune Mode", 0, default_display_msg_delay);          

  send_dit(AUTOMATIC_SENDING);
  key_tx = 1;
  while (looping) {

    if (digitalRead(paddle_left) == LOW) {
      tx_and_sidetone_key(1,MANUAL_SENDING);
      latched = 0;
    } 
    else {
      if (digitalRead(paddle_left) == HIGH && latched == 0) {
        tx_and_sidetone_key(0,MANUAL_SENDING);
      }
    }

    if (digitalRead(paddle_right) == LOW && latched == 0) {
      latched = 1;
      tx_and_sidetone_key(1,MANUAL_SENDING);
      while ((digitalRead(paddle_right) == LOW) && (digitalRead(paddle_left) == HIGH)) {
        delay(10);
      }
    } 
    else {
      if ((digitalRead(paddle_right) == LOW) && (latched)) {
        latched = 0;
        tx_and_sidetone_key(0,MANUAL_SENDING);
        while ((digitalRead(paddle_right) == LOW) && (digitalRead(paddle_left) == HIGH)) {
          delay(10);
        }
      }
    }
    if ((analogbuttonread(0)) || ((digitalRead(paddle_left) == LOW) && (digitalRead(paddle_right) == LOW))) { // if paddles are squeezed or button0 pressed - exit
      looping = 0;
    }

  }
  tx_and_sidetone_key(0,MANUAL_SENDING);
  while (digitalRead(paddle_left) == LOW || digitalRead(paddle_right) == LOW || analogbuttonread(0) ) {
  }  // wait for all lines to go high
  key_tx = 0;
  send_dit(AUTOMATIC_SENDING);
  dit_buffer = 0;
  dah_buffer = 0;
}
//-------------------------------------------------------------------------------------------------------

void sidetone_adj(int hz) {

  if ((hz_sidetone + hz) > SIDETONE_HZ_LOW_LIMIT && (hz_sidetone + hz) < SIDETONE_HZ_HIGH_LIMIT) {
    hz_sidetone = hz_sidetone + hz;
    config_dirty = 1; 
  }
}

//-------------------------------------------------------------------------------------------------------

void command_sidetone_freq_adj() {

  byte looping = 1;

  lcd_center_print_timed("Sidetone " + String(hz_sidetone) + " Hz", 0, default_display_msg_delay);   

  while (looping) {
    tone(sidetone_line, hz_sidetone);
    if (digitalRead(paddle_left) == LOW) {
      sidetone_adj(5);      
      lcd_center_print_timed("Sidetone " + String(hz_sidetone) + " Hz", 0, default_display_msg_delay);        
      delay(10);
    }
    if (digitalRead(paddle_right) == LOW) {
      sidetone_adj(-5);
      lcd_center_print_timed("Sidetone " + String(hz_sidetone) + " Hz", 0, default_display_msg_delay);       
      delay(10);
    }
    while ((digitalRead(paddle_left) == LOW && digitalRead(paddle_right) == LOW) || (analogbuttonread(0))) { // if paddles are squeezed or button0 pressed - exit
      looping = 0;
    }
  }
  while (digitalRead(paddle_left) == LOW || digitalRead(paddle_right) == LOW || analogbuttonread(0) ) {
  }  // wait for all lines to go high
  noTone(sidetone_line);

}

//-------------------------------------------------------------------------------------------------------


#ifdef FEATURE_MEMORIES
void check_the_memory_buttons()
{

  byte analogbuttontemp = analogbuttonpressed();
  if ((analogbuttontemp > 0) && (analogbuttontemp < (number_of_memories + 1)) && ((millis() - button_last_add_to_send_buffer_time) > 400)) {
    add_to_send_buffer(SERIAL_SEND_BUFFER_MEMORY_NUMBER);
    add_to_send_buffer(analogbuttontemp - 1);
    button_last_add_to_send_buffer_time = millis();
  }
}
#endif

//------------------------------------------------------------------

byte analogbuttonpressed() {

  int analog_line_read = analogRead(analog_buttons_pin);

#ifdef DEBUG_BUTTONS
  static byte debug_flag = 0;
#endif

  if (analog_line_read < 1000) {
#ifdef DEBUG_BUTTONS
    if (!debug_flag) {
      Serial.print(F("\nanalogbuttonpressed: analog_line_read: "));
      Serial.println(analog_line_read);
    }
#endif

    for (int x = 0;x < analog_buttons_number_of_buttons;x++) {
      if ((analog_line_read > button_array_low_limit[x])&& (analog_line_read <  button_array_high_limit[x])) {
#ifdef DEBUG_BUTTONS
        if (!debug_flag) {
          Serial.print(F("analogbuttonpressed: returning: "));
          Serial.println(x);
          debug_flag = 1;
        }
#endif
        return x;
      }  
    }
  }
#ifdef DEBUG_BUTTONS
  debug_flag = 0;
#endif
  return 255; 
}


//------------------------------------------------------------------
byte analogbuttonread(byte button_number) {

  // button numbers start with 0

  int analog_line_read = analogRead(analog_buttons_pin);

#ifdef DEBUG_BUTTONS
  static byte debug_flag = 0;
#endif

  if (analog_line_read < 1000) {  
    if ((analog_line_read > button_array_low_limit[button_number])&& (analog_line_read <  button_array_high_limit[button_number])) {
#ifdef DEBUG_BUTTONS
      if (!debug_flag) {
        Serial.print(F("\nanalogbuttonread: analog_line_read: "));
        Serial.print(analog_line_read);
        Serial.print(F("  button pressed: "));
        Serial.println(button_number);
        debug_flag = 1;
      }
#endif
      return 1;
    }  
  }
#ifdef DEBUG_BUTTONS
  debug_flag = 0;
#endif  
  return 0;
}

//------------------------------------------------------------------

void check_command_buttons()
{

#ifdef DEBUG_LOOP
  Serial.println(F("loop: entering check_buttons"));
#endif

  static long last_button_action = 0;
  byte analogbuttontemp = analogbuttonpressed();
  long button_depress_time;
  byte paddle_was_hit = 0;
  byte store_key_tx = key_tx;
  byte previous_sidetone_mode = 0;
  if ((analogbuttontemp < analog_buttons_number_of_buttons) && ((millis() - last_button_action) > 200)) {
    button_depress_time = millis();
    while ((analogbuttontemp == analogbuttonpressed()) && ((millis() - button_depress_time) < 1000)) {
      if ((digitalRead(paddle_left) == LOW) || (digitalRead(paddle_right) == LOW)) {
        button_depress_time = 1001;  // if button 0 is held and a paddle gets hit, assume we have a hold and shortcut out
      }
    }
    if ((millis() - button_depress_time) < 500) {
      if (analogbuttontemp == 0) {
        key_tx = 0;
        command_mode();
        if (command_mode_disable_tx) {
          key_tx = !store_key_tx;
        } 
        else {
          key_tx = 1;
        }
      }
#ifdef FEATURE_MEMORIES
      if ((analogbuttontemp > 0) && (analogbuttontemp < (number_of_memories + 1)) && ((millis() - button_last_add_to_send_buffer_time) > 400)) {
        add_to_send_buffer(SERIAL_SEND_BUFFER_MEMORY_NUMBER);
        add_to_send_buffer(analogbuttontemp - 1);
        button_last_add_to_send_buffer_time = millis();
#ifdef DEBUG_BUTTONS
        Serial.print(F("\ncheck_buttons: add_to_send_buffer: "));
        Serial.println(analogbuttontemp - 1);
#endif //DEBUG_BUTTONS
      }
#endif
    } 
    else {
      if (analogbuttontemp == 0) {
        key_tx = 0;
        // do stuff if this is a command button hold down
        while (analogbuttonpressed() == 0) {
          // do whatever it is that needs doing while command button is held down; otherwise delete this block
        }
        key_tx = 1;
      }  //(analogbuttontemp == 0)
    }
    last_button_action = millis();
  }
}

//-------------------------------------------------------------------------------------------------------

void service_dit_dah_buffers()
//send a dit or dah according to dit/dah_buffers, then reset the buffers.

{
#ifdef DEBUG_LOOP
  Serial.println(F("loop: entering service_dit_dah_buffers"));
#endif      

  if ((keyer_mode == IAMBIC_A) || (keyer_mode == IAMBIC_B) || (keyer_mode == ULTIMATIC)) {
    if ((keyer_mode == IAMBIC_A) && (iambic_flag) && (digitalRead(paddle_left)) && (digitalRead(paddle_right))) {
      iambic_flag = 0;
      dit_buffer = 0;
      dah_buffer = 0;
    } 
    else {
      if (dit_buffer) {
        dit_buffer = 0;
        send_dit(MANUAL_SENDING);
      }
      if (dah_buffer) {
        dah_buffer = 0;
        send_dah(MANUAL_SENDING);
      }
    }
  } 
  else {
    if (keyer_mode == BUG) {
      if (dit_buffer) {
        dit_buffer = 0;
        send_dit(MANUAL_SENDING);
      }
      if (dah_buffer) {
        dah_buffer = 0;
        tx_and_sidetone_key(1,MANUAL_SENDING);
      } 
      else {
        tx_and_sidetone_key(0,MANUAL_SENDING);
      }
#ifdef FEATURE_DEAD_OP_WATCHDOG
      dah_counter = 0;
#endif
    } 
    else {
      if (keyer_mode == STRAIGHT) {
        if (dit_buffer) {
          dit_buffer = 0;
          tx_and_sidetone_key(1,MANUAL_SENDING);
        } 
        else {
          tx_and_sidetone_key(0,MANUAL_SENDING);
        }
#ifdef FEATURE_DEAD_OP_WATCHDOG
        dit_counter = 0;
#endif
      }
    }
  }

}

//-------------------------------------------------------------------------------------------------------

void beep()
{
  tone(sidetone_line, hz_high_beep, 200);
}

//-------------------------------------------------------------------------------------------------------

void boop()
{
  tone(sidetone_line, hz_low_beep);
  delay(100);
  noTone(sidetone_line);
}

//-------------------------------------------------------------------------------------------------------

void beep_boop()
{
  tone(sidetone_line, hz_high_beep);
  delay(100);
  tone(sidetone_line, hz_low_beep);
  delay(100);
  noTone(sidetone_line);
}

//-------------------------------------------------------------------------------------------------------

void boop_beep()
{
  tone(sidetone_line, hz_low_beep);
  delay(100);
  tone(sidetone_line, hz_high_beep);
  delay(100);
  noTone(sidetone_line);
}

//-------------------------------------------------------------------------------------------------------

void send_dits(int dits)
{
  for (;dits > 0;dits--) {
    send_dit(AUTOMATIC_SENDING);
  }

}

//-------------------------------------------------------------------------------------------------------

void send_dahs(int dahs)
{
  for (;dahs > 0;dahs--) {
    send_dah(AUTOMATIC_SENDING);
  }

}

//-------------------------------------------------------------------------------------------------------

void send_char(byte cw_char, byte omit_letterspace)
{
  int morse_length = 6;  //morse code characters have at most six morsebits (dits/dahs)
                         //except for prosigns CL and BK - if we want to store these, need to make exception
                         //this presumes that no control codes (values > 127) are sent to send_char
#ifdef DEBUG_SEND_CHAR
  Serial.print(F("send_char: called with cw_char:"));
  Serial.print(cw_char);
  if (omit_letterspace) {
    Serial.print(F(" OMIT_LETTERSPACE"));
  }
  Serial.println();
#endif
  if(!cw_char){ //space is represented as a zero
    loop_element_lengths((length_wordspace-length_letterspace-2),0,wpm,AUTOMATIC_SENDING);
  }
  else {
    while(!(cw_char & 64)) {  // a sendable character has value < 128 
      cw_char = cw_char << 1;   // nibble to the start bit
      morse_length--;
    }
    while(morse_length) {
      if(cw_char & 32) {       // crank the bits leftward for the length in dits/dahs of the character
        send_dah(AUTOMATIC_SENDING);
      }
      else {
        send_dit(AUTOMATIC_SENDING);
      }
      cw_char = cw_char << 1;
      morse_length--;
    } 
  } 
  if (omit_letterspace != OMIT_LETTERSPACE) {
    loop_element_lengths((length_letterspace-1),0,wpm,AUTOMATIC_SENDING); //this is minus one because send_dit and send_dah have a trailing element space
  }
}

//-------------------------------------------------------------------------------------------------------

int uppercase (int charbytein)
{
  if (((charbytein > 96) && (charbytein < 123)) || ((charbytein > 223) && (charbytein < 255))) {
    charbytein = charbytein - 32;
  }
  if (charbytein == 158) { 
    charbytein = 142; 
  }  // ≈æ -> ≈Ω
  if (charbytein == 154) { 
    charbytein = 138; 
  }  // ≈° -> ≈†

  return charbytein;
}

//-------------------------------------------------------------------------------------------------------
#ifdef FEATURE_SERIAL
#ifdef FEATURE_COMMAND_LINE_INTERFACE
void serial_print(prog_uchar str[])
{
  char c;
  while((c = pgm_read_byte(str++))) {
    Serial.write(c);
  }
}
#endif
#endif

//-------------------------------------------------------------------------------------------------------
#ifdef FEATURE_SERIAL
#ifdef FEATURE_COMMAND_LINE_INTERFACE
void serial_qrss_mode()
{
  byte looping = 1;
  byte incoming_serial_byte;
  byte numbers[4];
  byte numberindex = 0;
  String numberstring;
  byte error =0;

  while (looping) {
    if (Serial.available() == 0) {        // wait for the next keystroke
      if (machine_mode == NORMAL) {          // might as well do something while we're waiting
        check_paddles();
        service_dit_dah_buffers();
        //check_the_memory_buttons();
      }
    } 
    else {

      incoming_serial_byte = Serial.read();
      if ((incoming_serial_byte > 47) && (incoming_serial_byte < 58)) {    // ascii 48-57 = "0" - "9")
        numberstring = numberstring + incoming_serial_byte;
        numbers[numberindex] = incoming_serial_byte;
        //        Serial.write("numberindex:");
        //        Serial.print(numberindex,DEC);
        //        Serial.write("     numbers:");
        //        Serial.println(numbers[numberindex],DEC);
        numberindex++;
        if (numberindex > 2)
        {
          looping = 0;
          error = 1;
        }
      } 
      else {
        if (incoming_serial_byte == 13) {   // carriage return - get out
          looping = 0;
        } 
        else {                 // bogus input - error out
          looping = 0;
          error = 1;
        }
      }
    }
  }

  if (error) {
    Serial.println(F("Error..."));
    while (Serial.available() > 0) { 
      incoming_serial_byte = Serial.read(); 
    }  // clear out buffer
    return;
  } 
  else {
    serial_print(string_qrss_mode);
    Serial.print(numberstring);
    Serial.println(F(" seconds"));
    int y = 1;
    int set_dit_length = 0;
    for (int x = (numberindex - 1); x >= 0 ; x = x - 1) {
      set_dit_length = set_dit_length + ((numbers[x]-48) * y);
      y = y * 10;
    }
    qrss_dit_length = set_dit_length;
    speed_mode = SPEED_QRSS;
    //calculate_element_length();
  }

}
#endif
#endif
//-------------------------------------------------------------------------------------------------------

void service_send_buffer()
{
  // send or process one character out of the send buffer
  // values 129 and above do special things
  // SERIAL_SEND_BUFFER_SPECIAL_START     129  
  // SERIAL_SEND_BUFFER_WPM_CHANGE        130
  // SERIAL_SEND_BUFFER_TIMED_KEY_DOWN    131
  // SERIAL_SEND_BUFFER_TIMED_WAIT        132
  // SERIAL_SEND_BUFFER_NULL              133
  // SERIAL_SEND_BUFFER_PROSIGN           134
  // SERIAL_SEND_BUFFER_HOLD_SEND         135
  // SERIAL_SEND_BUFFER_HOLD_SEND_RELEASE 136
  // SERIAL_SEND_BUFFER_MEMORY_NUMBER     137
  // SERIAL_SEND_BUFFER_SPECIAL_END       138

#ifdef DEBUG_LOOP
  Serial.println(F("loop: entering service_send_buffer"));
#endif          

#ifdef FEATURE_MEMORIES
  play_memory_prempt = 0;
#endif

  static unsigned long timed_command_end_time;
  static byte timed_command_in_progress = 0;

  if (send_buffer_status == SERIAL_SEND_BUFFER_NORMAL) {
    if ((send_buffer_bytes > 0) && (pause_sending_buffer == 0)) {
      if ((send_buffer_array[0] > SERIAL_SEND_BUFFER_SPECIAL_START) && (send_buffer_array[0] < SERIAL_SEND_BUFFER_SPECIAL_END)) {

        if (send_buffer_array[0] == SERIAL_SEND_BUFFER_HOLD_SEND) {
          send_buffer_status = SERIAL_SEND_BUFFER_HOLD;
          remove_from_send_buffer();
        }

        if (send_buffer_array[0] == SERIAL_SEND_BUFFER_HOLD_SEND_RELEASE) {
          remove_from_send_buffer();
        }

        if (send_buffer_array[0] == SERIAL_SEND_BUFFER_MEMORY_NUMBER) {
#ifdef DEBUG_SEND_BUFFER
          Serial.println(F("service_send_buffer: SERIAL_SEND_BUFFER_MEMORY_NUMBER"));
#endif
          remove_from_send_buffer();
          if (send_buffer_bytes > 0) {
            if (send_buffer_array[0] < number_of_memories) {
#ifdef FEATURE_MEMORIES
              play_memory(send_buffer_array[0]);
#endif
            }
            remove_from_send_buffer();
          }
        }

        if (send_buffer_array[0] == SERIAL_SEND_BUFFER_WPM_CHANGE) {  // two bytes for wpm
          remove_from_send_buffer();
          if (send_buffer_bytes > 1) {
            wpm = send_buffer_array[0] * 256;
            remove_from_send_buffer();
            wpm = wpm + send_buffer_array[0];
            remove_from_send_buffer();
          }
        }

        if (send_buffer_array[0] == SERIAL_SEND_BUFFER_NULL) {
          remove_from_send_buffer();
        }

        if (send_buffer_array[0] == SERIAL_SEND_BUFFER_PROSIGN) {
          remove_from_send_buffer();
          if (send_buffer_bytes > 0) {
            send_char(send_buffer_array[0],OMIT_LETTERSPACE);
            remove_from_send_buffer();
          }
          if (send_buffer_bytes > 0) {
            send_char(send_buffer_array[0],NORMAL);
            remove_from_send_buffer();
          }
        }

        if (send_buffer_array[0] == SERIAL_SEND_BUFFER_TIMED_KEY_DOWN) {
          remove_from_send_buffer();
          if (send_buffer_bytes > 0) {
            send_buffer_status = SERIAL_SEND_BUFFER_TIMED_COMMAND;
            tx_and_sidetone_key(1,AUTOMATIC_SENDING);
            timed_command_end_time = millis() + (send_buffer_array[0] * 1000);
            timed_command_in_progress = SERIAL_SEND_BUFFER_TIMED_KEY_DOWN;
            remove_from_send_buffer();
          }
        }

        if (send_buffer_array[0] == SERIAL_SEND_BUFFER_TIMED_WAIT) {
          remove_from_send_buffer();
          if (send_buffer_bytes > 0) {
            send_buffer_status = SERIAL_SEND_BUFFER_TIMED_COMMAND;
            timed_command_end_time = millis() + (send_buffer_array[0] * 1000);
            timed_command_in_progress = SERIAL_SEND_BUFFER_TIMED_WAIT;
            remove_from_send_buffer();
          }
        }
      } 
      else {
#ifdef FEATURE_SERIAL
        Serial.write(send_buffer_array[0]);
        if (send_buffer_array[0] == 13) {
          Serial.write(10);  // if we got a carriage return, also send a line feed
        }
#endif //FEATURE_SERIAL
        if (lcd_send_echo) {
          display_scroll_print_char(convert_cw_number_to_ascii(send_buffer_array[0]));
          service_display();
        }
        send_char(send_buffer_array[0],NORMAL);
        remove_from_send_buffer();
      }
    }

  } 
  else {

    if (send_buffer_status == SERIAL_SEND_BUFFER_TIMED_COMMAND) {    // we're in a timed command

      if ((timed_command_in_progress == SERIAL_SEND_BUFFER_TIMED_KEY_DOWN) && (millis() > timed_command_end_time)) {
        tx_and_sidetone_key(0,AUTOMATIC_SENDING);
        timed_command_in_progress = 0;
        send_buffer_status = SERIAL_SEND_BUFFER_NORMAL;
      }

      if ((timed_command_in_progress == SERIAL_SEND_BUFFER_TIMED_WAIT) && (millis() > timed_command_end_time)) {
        timed_command_in_progress = 0;
        send_buffer_status = SERIAL_SEND_BUFFER_NORMAL;
      }

    }

    if (send_buffer_status == SERIAL_SEND_BUFFER_HOLD) {  // we're in a send hold ; see if there's a SERIAL_SEND_BUFFER_HOLD_SEND_RELEASE in the buffer
      if (send_buffer_bytes == 0) {
        send_buffer_status = SERIAL_SEND_BUFFER_NORMAL;  // this should never happen, but what the hell, we'll catch it here if it ever does happen
      } 
      else {
        for (int z = 0; z < send_buffer_bytes; z++) {
          if (send_buffer_array[z] ==  SERIAL_SEND_BUFFER_HOLD_SEND_RELEASE) {
            send_buffer_status = SERIAL_SEND_BUFFER_NORMAL;
            z = send_buffer_bytes;
          }
        }
      }
    }

  }

  //if the paddles are hit, dump the buffer
  check_paddles();
  if ((dit_buffer || dah_buffer) && (send_buffer_bytes  > 0)) {
    send_buffer_bytes = 0;
    send_buffer_status = SERIAL_SEND_BUFFER_NORMAL;
    dit_buffer = 0;
    dah_buffer = 0;    
  }

}

//-------------------------------------------------------------------------------------------------------

void remove_from_send_buffer()
{
  if (send_buffer_bytes > 0) {
    send_buffer_bytes--;
  }
  if (send_buffer_bytes > 0) {
    for (int x = 0;x < send_buffer_bytes;x++) {
      send_buffer_array[x] = send_buffer_array[x+1];
    }
  }
}

//-------------------------------------------------------------------------------------------------------

void add_to_send_buffer(byte incoming_serial_byte)
{
  //  if ((incoming_serial_byte == SERIAL_SEND_BUFFER_HOLD_SEND_RELEASE) && (send_buffer_status == SERIAL_SEND_BUFFER_HOLD)) {
  //    send_buffer_status = SERIAL_SEND_BUFFER_NORMAL;
  //  } else {
  if (send_buffer_bytes < send_buffer_size) {
    if (incoming_serial_byte != 127) {
      send_buffer_bytes++;
      send_buffer_array[send_buffer_bytes - 1] = incoming_serial_byte;
    } 
    else {  // we got a backspace
      send_buffer_bytes--;
    }
  }
  //  }
}

//-------------------------------------------------------------------------------------------------------

#ifdef FEATURE_COMMAND_LINE_INTERFACE
void service_command_line_interface() {

  static byte cli_wait_for_cr_flag = 0; 

  if (serial_backslash_command == 0) {
    //incoming_serial_byte = Serial.read();
    incoming_serial_byte = uppercase(incoming_serial_byte);
    if (incoming_serial_byte != 92) { // we do not have a backslash
      if (cli_prosign_flag) {
        add_to_send_buffer(SERIAL_SEND_BUFFER_PROSIGN);
        cli_prosign_flag = 0;
      }
      if (cli_wait_for_cr_to_send_cw) {
        if (cli_wait_for_cr_flag == 0) {
          if (incoming_serial_byte > 31) {
#ifdef DEBUG_CHECK_SERIAL
            Serial.println(F("check_serial: add_to_send_buffer(SERIAL_SEND_BUFFER_HOLD_SEND)"));
#endif
            add_to_send_buffer(SERIAL_SEND_BUFFER_HOLD_SEND);
            cli_wait_for_cr_flag = 1;
          }
        } 
        else {
          if (incoming_serial_byte == 13) {
#ifdef DEBUG_CHECK_SERIAL
            Serial.println(F("check_serial: add_to_send_buffer(SERIAL_SEND_BUFFER_HOLD_SEND_RELEASE)"));
#endif
            add_to_send_buffer(SERIAL_SEND_BUFFER_HOLD_SEND_RELEASE);
            cli_wait_for_cr_flag = 0;
          }
        }
      }
      if(incoming_serial_byte != 13){
        add_to_send_buffer(ascii_to_cw(incoming_serial_byte));
      }
    } 
    else {     //(incoming_serial_byte = 92)  -- we got a backslash
      serial_backslash_command = 1;
      Serial.write(incoming_serial_byte);
    }
  } 
  else { // (serial_backslash_command == 0) -- we already got a backslash
    //incoming_serial_byte = Serial.read();
    Serial.write(incoming_serial_byte);
    incoming_serial_byte = uppercase(incoming_serial_byte);
    process_serial_command();
    serial_backslash_command = 0;
    Serial.println();
  }
}
#endif //FEATURE_COMMAND_LINE_INTERFACE

//-------------------------------------------------------------------------------------------------------

#ifdef FEATURE_SERIAL
#ifdef FEATURE_COMMAND_LINE_INTERFACE
byte ascii_to_cw(byte ascii_char) {
// convert ASCII to cw encoding

  switch(ascii_char) {
  case 'A': return 5;
  case 'B': return 24;
  case 'C': return 26;
  case 'D': return 12;
  case 'E': return 2;
  default: return 76;
  }
}

#endif
#endif



//-------------------------------------------------------------------------------------------------------

#ifdef FEATURE_SERIAL
void check_serial()
{

#ifdef DEBUG_LOOP
  Serial.println(F("loop: entering check_serial")); 
#endif 

  while (Serial.available() > 0) {
    incoming_serial_byte = Serial.read();
#ifndef FEATURE_COMMAND_LINE_INTERFACE
    //incoming_serial_byte = Serial.read();
    Serial.println(F("No serial features enabled..."));
#endif

    // yea, this is a bit funky below

#ifdef FEATURE_COMMAND_LINE_INTERFACE    
    service_command_line_interface();
#endif //FEATURE_COMMAND_LINE_INTERFACE

  }  //while (Serial.available() > 0)
}
#endif

//---------------------------------------------------------------------

#ifdef FEATURE_SERIAL
#ifdef FEATURE_COMMAND_LINE_INTERFACE
void process_serial_command() {

  Serial.println();
  switch (incoming_serial_byte) {
  case 126: 
    wdt_enable(WDTO_30MS); 
    while(1) {
    } 
    ; 
    break;  // ~ - reset unit
  case 42:                                                // * - paddle echo on / off
    if (cli_paddle_echo) {
      cli_paddle_echo = 0;
    } 
    else {
      cli_paddle_echo = 1;
    }
    break;
  case 43: 
    cli_prosign_flag = 1; 
    break;
#ifdef FEATURE_SERIAL_HELP
  case 63: 
    serial_print(serial_help_string); 
    break;                         // ? = print help
#endif //FEATURE_SERIAL_HELP
  case 65: 
    keyer_mode = IAMBIC_A; 
    config_dirty = 1; 
    Serial.println(F("Iambic A")); 
    break;    // A - Iambic A mode
  case 66: 
    keyer_mode = IAMBIC_B; 
    config_dirty = 1; 
    Serial.println(F("Iambic B")); 
    break;    // B - Iambic B mode
  case 68: 
    keyer_mode = ULTIMATIC; 
    config_dirty = 1; 
    Serial.println(F("Ultimatic")); 
    break;  // D - Ultimatic mode
  case 69: 
    serial_set_serial_number(); 
    break;                                   // E - set serial number
  case 70: 
    serial_set_sidetone_freq(); 
    break;                                   // F - set sidetone frequency
  case 71: 
    keyer_mode = BUG; 
    config_dirty = 1; 
    Serial.println(F("Bug")); 
    break;              // G - Bug mode
  case 73:                                                                      // I - transmit line on/off
    Serial.print(F("TX o"));
    if (key_tx) {
      key_tx = 0;
      Serial.println(F("ff"));
    } 
    else {
      key_tx = 1;
      Serial.println(F("n"));
    }
    break;
#ifdef FEATURE_MEMORIES
  case 49: 
    serial_play_memory(0); 
    break;     // 1 - play memory 1  (0)
  case 50: 
    serial_play_memory(1); 
    break;     // 2 - play memory 2  (1)
  case 51: 
    serial_play_memory(2); 
    break;     // 3 - play memory 3  (2)
  case 52: 
    serial_play_memory(3); 
    break;     // 4 - play memory 4  (3)

  case 80: 
    serial_program_memory(); 
    break;                                // P - program memory
#endif //FEATURE_MEMORIES
  case 81: 
    serial_qrss_mode(); 
    break; // Q - activate QRSS mode
  case 82: 
    speed_mode = SPEED_NORMAL; 
    Serial.println(F("QRSS Off")); 
    break; // R - activate regular timing mode
  case 83: 
    serial_status(); 
    break;                                              // S - Status command
  case 74: 
    serial_set_dit_to_dah_ratio(); 
    break;                          // J - dit to dah ratio
  case 76: 
    serial_set_weighting(); 
    break;
#ifdef FEATURE_FARNSWORTH
  case 77: 
    serial_set_farnsworth(); 
    break;                                // M - set Farnsworth speed
#endif
  case 78:                                                                // N - paddle reverse
    Serial.print(F("Paddles "));
    if (paddle_mode == PADDLE_NORMAL) {
      paddle_mode = PADDLE_REVERSE;
      Serial.println(F("reversed"));
    } 
    else {
      paddle_mode = PADDLE_NORMAL;
      Serial.println(F("normal"));
    }
    config_dirty = 1;
    break;  // case 78
  case 79:                                                                // O - toggle sidetone on/off
    Serial.print(F("Sidetone O"));
    if ((sidetone_mode == SIDETONE_ON) || (sidetone_mode == SIDETONE_PADDLE_ONLY)) {
      sidetone_mode = SIDETONE_OFF;
      Serial.println(F("FF"));
    } 
    else {
      sidetone_mode = SIDETONE_ON;
      Serial.println(F("N"));
    }
    config_dirty = 1;
    break; // case 79
  case 84: // T - tune
    serial_tune_command(); 
    break;
  case 89: 
    serial_change_wordspace(); 
    break;
#ifdef FEATURE_AUTOSPACE
  case 90:
    Serial.print(F("Autospace O"));
    if (autospace_active) {
      autospace_active = 0;
      config_dirty = 1;
      Serial.println(F("ff"));
    } 
    else {
      autospace_active = 1;
      config_dirty = 1;
      Serial.println(F("n"));
    }
    break;
#endif
#ifdef FEATURE_MEMORIES
  case 92:
    send_buffer_bytes = 0;
    play_memory_prempt = 1;
    break;                           // \ - double backslash - clear serial send buffer
#endif
  case 94:                           // ^ - toggle send CW send immediately
    if (cli_wait_for_cr_to_send_cw) {
      cli_wait_for_cr_to_send_cw = 0;
      Serial.println(F("Send CW immediately"));
    } 
    else {
      cli_wait_for_cr_to_send_cw = 1;
      Serial.println(F("Wait for CR to send CW"));
    }
    break;
  default: 
    Serial.println(F("Unknown command")); 
    break;
  }

}
#endif //FEATURE_SERIAL
#endif //FEATURE_COMMAND_LINE_INTERFACE
//---------------------------------------------------------------------
#ifdef FEATURE_SERIAL
#ifdef FEATURE_COMMAND_LINE_INTERFACE

void service_serial_paddle_echo()
// converts cli_paddle_echo_buffer containing a single character code to ascii 
// and sends that character to serial out, then resets the buffer

{
#ifdef DEBUG_LOOP
  Serial.println(F("loop: entering service_serial_paddle_echo"));
#endif          

  static byte cli_paddle_echo_space_sent = 1;

  if ((cli_paddle_echo_buffer > 1) && (cli_paddle_echo) && (millis() > cli_paddle_echo_buffer_decode_time)) {
    Serial.write(byte(convert_cw_number_to_ascii(cli_paddle_echo_buffer)));
    cli_paddle_echo_buffer = 1;
    cli_paddle_echo_buffer_decode_time = millis() + (float(600/wpm)*length_letterspace);
    cli_paddle_echo_space_sent = 0;
  }
  if ((cli_paddle_echo_buffer == 1) && (cli_paddle_echo) && (millis() > (cli_paddle_echo_buffer_decode_time + (float(1200/wpm)*(length_wordspace-length_letterspace)))) && (!cli_paddle_echo_space_sent)) {
    Serial.write(" ");
    cli_paddle_echo_space_sent = 1;
  }
}
#endif
#endif

//---------------------------------------------------------------------

#ifdef FEATURE_SERIAL
#ifdef FEATURE_COMMAND_LINE_INTERFACE
#ifdef FEATURE_MEMORIES
void serial_play_memory(byte memory_number) {

  if (memory_number < number_of_memories) {
    add_to_send_buffer(SERIAL_SEND_BUFFER_MEMORY_NUMBER);
    add_to_send_buffer(memory_number);
  }

}
#endif
#endif
#endif

//---------------------------------------------------------------------

#ifdef FEATURE_SERIAL
#ifdef FEATURE_COMMAND_LINE_INTERFACE
int serial_get_number_input(byte places,int lower_limit, int upper_limit)
{
  byte incoming_serial_byte = 0;
  byte looping = 1;
  byte error = 0;
  String numberstring = "";
  byte numberindex = 0;
  int numbers[6];

  while (looping) {
    if (Serial.available() == 0) {        // wait for the next keystroke
      if (machine_mode == NORMAL) {          // might as well do something while we're waiting
        check_paddles();
        service_dit_dah_buffers();
        service_send_buffer();

        check_potentiometer();
      }
    } 
    else {
      incoming_serial_byte = Serial.read();
      if ((incoming_serial_byte > 47) && (incoming_serial_byte < 58)) {    // ascii 48-57 = "0" - "9")
        numberstring = numberstring + incoming_serial_byte;
        numbers[numberindex] = incoming_serial_byte;
        numberindex++;
        if (numberindex > places){
          looping = 0;
          error = 1;
        }
      } 
      else {
        if (incoming_serial_byte == 13) {   // carriage return - get out
          looping = 0;
        } 
        else {                 // bogus input - error out
          looping = 0;
          error = 1;
        }
      }
    }
  }
  if (error) {
    Serial.write("Error...\n\r");
    while (Serial.available() > 0) { 
      incoming_serial_byte = Serial.read(); 
    }  // clear out buffer
    return(-1);
  } 
  else {
    int y = 1;
    int return_number = 0;
    for (int x = (numberindex - 1); x >= 0 ; x = x - 1) {
      return_number = return_number + ((numbers[x]-48) * y);
      y = y * 10;
    }
    if ((return_number > lower_limit) && (return_number < upper_limit)) {
      return(return_number);
    } 
    else {
      Serial.write("Error...\n\r");
      return(-1);
    }
  }
}
#endif
#endif

//---------------------------------------------------------------------

#ifdef FEATURE_SERIAL
#ifdef FEATURE_COMMAND_LINE_INTERFACE
void serial_change_wordspace()
{
  int set_wordspace_to = serial_get_number_input(2,0,100);
  if (set_wordspace_to > 0) {
    config_dirty = 1;
    length_wordspace = set_wordspace_to;
    Serial.write("Wordspace set to ");
    Serial.println(set_wordspace_to,DEC);
  }
}
#endif
#endif

//---------------------------------------------------------------------

#ifdef FEATURE_SERIAL
#ifdef FEATURE_COMMAND_LINE_INTERFACE
void serial_set_dit_to_dah_ratio()
{
  int set_ratio_to = serial_get_number_input(4, 99, 1000);
  if ((set_ratio_to > 99) && (set_ratio_to < 1000)) {
    dah_to_dit_ratio = set_ratio_to;
    serial_print(string_setting_dah_to_dit_ratio);
    Serial.println((float(dah_to_dit_ratio)/100));
    config_dirty = 1;
  }
}
#endif
#endif

//---------------------------------------------------------------------
#ifdef FEATURE_SERIAL
#ifdef FEATURE_COMMAND_LINE_INTERFACE
void serial_set_serial_number()
{
  int set_serial_number_to = serial_get_number_input(4,0,10000);
  if (set_serial_number_to > 0) {
    serial_number = set_serial_number_to;
    serial_print(string_setting_serial_number);
    Serial.println(serial_number);
  }
}
#endif
#endif

//---------------------------------------------------------------------
#ifdef FEATURE_SERIAL
#ifdef FEATURE_COMMAND_LINE_INTERFACE
void serial_set_sidetone_freq()
{
  int set_sidetone_hz = serial_get_number_input(4,(SIDETONE_HZ_LOW_LIMIT-1),(SIDETONE_HZ_HIGH_LIMIT+1));
  if ((set_sidetone_hz > SIDETONE_HZ_LOW_LIMIT) && (set_sidetone_hz < SIDETONE_HZ_HIGH_LIMIT)) {
    Serial.write("Setting sidetone to ");
    Serial.print(set_sidetone_hz,DEC);
    Serial.write(" hz\n\r");
    hz_sidetone = set_sidetone_hz;
    config_dirty = 1;
  }
}
#endif
#endif

//---------------------------------------------------------------------
#ifdef FEATURE_SERIAL
#ifdef FEATURE_COMMAND_LINE_INTERFACE
#ifdef FEATURE_FARNSWORTH
void serial_set_farnsworth()
{
  int set_farnsworth_wpm = serial_get_number_input(3,-1,1000);
  if (set_farnsworth_wpm > 0) {
    wpm_farnsworth = set_farnsworth_wpm;
    Serial.write("Setting Farnworth WPM to ");
    Serial.println(set_farnsworth_wpm,DEC);
    config_dirty = 1;
  }
}
#endif
#endif
#endif

//---------------------------------------------------------------------
#ifdef FEATURE_SERIAL
#ifdef FEATURE_COMMAND_LINE_INTERFACE
void serial_set_weighting()
{
  int set_weighting = serial_get_number_input(2,9,91);
  if (set_weighting > 0) {
    weighting = set_weighting;
    Serial.write("Setting weighting to ");
    Serial.println(set_weighting,DEC);
  }
}
#endif
#endif

//---------------------------------------------------------------------
#ifdef FEATURE_SERIAL
#ifdef FEATURE_COMMAND_LINE_INTERFACE
void serial_tune_command ()
{
  byte incoming;

  delay(100);
  while (Serial.available() > 0) {  // clear out the buffer if anything is there
    incoming = Serial.read();
  }

  tx_and_sidetone_key(1,MANUAL_SENDING);
  Serial.write("Keying transmitter - press a key to unkey...\n\r");
  while ((Serial.available() == 0) && (!analogbuttonread(0))) {
  }  // keystroke or button0 hit gets us out of here
  while (Serial.available() > 0) {  // clear out the buffer if anything is there
    incoming = Serial.read();
  }
  tx_and_sidetone_key(0,MANUAL_SENDING);

}
#endif
#endif
//---------------------------------------------------------------------

#ifdef FEATURE_SERIAL
#ifdef FEATURE_COMMAND_LINE_INTERFACE
void serial_status() {

  Serial.println();
  switch (keyer_mode) {
  case IAMBIC_A: 
    Serial.write("Iambic A"); 
    break;
  case IAMBIC_B: 
    Serial.write("Iambic B"); 
    break;
  case BUG: 
    Serial.write("Bug"); 
    break;
  case STRAIGHT: 
    Serial.write("Straightkey"); 
    break;
  case ULTIMATIC: 
    Serial.write("Ultimatic"); 
    break;
  }
  Serial.write("\n\r");
  if (speed_mode == SPEED_NORMAL) {
    Serial.write("WPM: ");
    Serial.println(wpm,DEC);
#ifdef FEATURE_FARNSWORTH
    Serial.write("Farnsworth WPM: ");
    if (wpm_farnsworth < wpm) {
      Serial.write("disabled\n\r");
    } 
    else {
      Serial.println(wpm_farnsworth,DEC);
    }
#endif //FEATURE_FARNSWORTH
  } 
  else {
    Serial.print(F("QRSS Mode Activated - Dit Length: "));
    Serial.print(qrss_dit_length,DEC);
    Serial.println(" seconds");
  }

  Serial.print(F("Sidetone Hz: "));
  Serial.println(hz_sidetone,DEC);
  Serial.print(F("Sidetone:"));
  switch (sidetone_mode) {
  case SIDETONE_ON: 
    Serial.println(F("ON")); 
    break;
  case SIDETONE_OFF: 
    Serial.println(F("OFF")); 
    break;
  case SIDETONE_PADDLE_ONLY: 
    Serial.println(F("Paddle Only")); 
    break;
  }
  Serial.print(F("Dah to dit ratio: "));
  Serial.println((float(dah_to_dit_ratio)/100));
  Serial.print(F("Weighting: "));
  Serial.println(weighting,DEC);
  Serial.print(F("Serial Number: "));
  Serial.println(serial_number,DEC);
  Serial.print(F("Potentiometer WPM: "));
  Serial.println(pot_value_wpm(),DEC);
#ifdef FEATURE_AUTOSPACE
  Serial.write("Autospace: ");
  if (autospace_active) {
    Serial.write("On");
  } 
  else {
    Serial.write("Off");
  }
  Serial.println();
#endif
  Serial.write("Wordspace: ");
  Serial.println(length_wordspace,DEC);
#ifdef FEATURE_MEMORIES
  serial_status_memories();
#endif

#ifdef DEBUG_MEMORYCHECK
  memorycheck();
#endif

#ifdef DEBUG_VARIABLE_DUMP
  Serial.println(wpm);
#ifdef FEATURE_FARNSWORTH
  Serial.println(wpm_farnsworth);
#endif //FEATURE_FARNSWORTH
  Serial.println(1.0*(float(weighting)/50));
  Serial.println(keying_compensation,DEC);
  Serial.println(2.0-(float(weighting)/50));
  Serial.println(-1.0*keying_compensation);
  Serial.println((dit_end_time-dit_start_time),DEC);
  Serial.println((dah_end_time-dah_start_time),DEC);
  Serial.println(millis(),DEC);
#endif //DEBUG_VARIABLE_DUMP

#ifdef DEBUG_BUTTONS
  for (int x = 0;x < analog_buttons_number_of_buttons;x++) {
    Serial.print(F("analog_button_array:   "));
    Serial.print(x);
    Serial.print(F(" button_array_low_limit: "));
    Serial.print(button_array_low_limit[x]);
    Serial.print(F("  button_array_high_limit: "));
    Serial.println(button_array_high_limit[x]);
  }
#endif


}
#endif
#endif
//---------------------------------------------------------------------

char convert_cw_number_to_ascii (byte number_in)
{

  switch (number_in) {
  case 5:   return 'A';      
  case 24:  return 'B';
  case 26:  return 'C';
  case 12:  return 'D';
  case 2:   return 'E';
  case 18:  return 'F';
  case 14:  return 'G';
  case 16:  return 'H';
  case 4:   return 'I';
  case 23:  return 'J';
  case 13:  return 'K';
  case 20:  return 'L';
  case 7:   return 'M'; 
  case 6:   return 'N';
  case 15:  return 'O'; 
  case 22:  return 'P'; 
  case 29:  return 'Q';
  case 10:  return 'R';
  case 8:   return 'S'; 
  case 3:   return 'T'; 
  case 9:   return 'U'; 
  case 17:  return 'V';
  case 11:  return 'W'; 
  case 25:  return 'X';
  case 27:  return 'Y'; 
  case 28:  return 'Z';
  case 63:  return '0';
  case 47:  return '1';
  case 39:  return '2'; 
  case 35:  return '3';
  case 33:  return '4'; 
  case 32:  return '5'; 
  case 48:  return '6';
  case 56:  return '7';
  case 60:  return '8';
  case 62:  return '9'; 
  case 76:  return '?';
  case 50:  return '/';
  case 49:  return '-';
  case 115: return ',';
  case 85:  return '.';
  case 64:  return 92; // special hack; six dahs = \ (backslash)  [note: this renders as a yen sign on my lcd - jjw]
  case 0:   return 32;
  default:  return 254; 
  }
}

#ifdef DEBUG_MEMORYCHECK
void memorycheck()
{
  void* HP = malloc(4);
  if (HP)
    free (HP);

  unsigned long free = (unsigned long)SP - (unsigned long)HP;

  //  Serial.print("Heap=");
  //  Serial.println((unsigned long)HP,HEX);
  //  Serial.print("Stack=");
  //  Serial.println((unsigned long)SP,HEX);
  //  Serial.print("Free Memory = ");
  //  Serial.print((unsigned long)free,HEX);
  //  Serial.print("  ");
  if (free > 2048) {
    free = 0;
  }
  Serial.print((unsigned long)free,DEC);
  Serial.write(" bytes free\n\r");
}
#endif

//***********************************************CURRENT MEMORY CODE***********************************************************
//*****************************************************************************************************************************
//*****************************************************************************************************************************

#ifndef EXPERIMENTAL_MEMORY_CODE
#ifdef FEATURE_MEMORIES
void initialize_eeprom_memories()
{
  for (int x = 0; x < number_of_memories; x++) {
    EEPROM.write(memory_start(x),255);
  }
}
#endif
#endif

//---------------------------------------------------------------------

#ifndef EXPERIMENTAL_MEMORY_CODE
#ifdef FEATURE_MEMORIES
#ifdef FEATURE_COMMAND_LINE_INTERFACE
void serial_status_memories()
{
  int last_memory_location;

  for (int x = 0; x < number_of_memories; x++) {
    last_memory_location = memory_end(x) + 1 ;
    Serial.write("Memory ");
    Serial.print(x+1);
    Serial.write(":");
    if ( EEPROM.read(memory_start(x)) == 255) {
      Serial.write("{empty}");
    } 
    else {
      for (int y = (memory_start(x)); (y < last_memory_location); y++) {
        if (EEPROM.read(y) < 255) {
          Serial.write(EEPROM.read(y));
        } 
        else {
          Serial.write("$");
          y = last_memory_location;
        }
      }
    }
    Serial.write("\r\n\r");
  }
}
#endif
#endif
#endif

//---------------------------------------------------------------------

#ifdef FEATURE_SERIAL
#ifndef EXPERIMENTAL_MEMORY_CODE
#ifdef FEATURE_MEMORIES
#ifdef FEATURE_COMMAND_LINE_INTERFACE
void serial_program_memory()
{

  byte incoming_serial_byte;
  byte memory_number;
  byte looping = 1;
  int memory_index = 0;

  while (Serial.available() == 0) {        // wait for the next keystroke
    if (machine_mode == NORMAL) {          // might as well do something while we're waiting
      check_paddles();
      service_dit_dah_buffers();
      //check_the_memory_buttons();
    }
  }
  incoming_serial_byte = Serial.read();
  if ((incoming_serial_byte > 48) && (incoming_serial_byte < (49 + number_of_memories))) {    // ascii 49-53 = "1" - "5")
    memory_number = incoming_serial_byte - 49;
    Serial.print(memory_number+1);
    while (looping) {
      while (Serial.available() == 0) {
        if (machine_mode == NORMAL) {          // might as well do something while we're waiting
          check_paddles();
          service_dit_dah_buffers();
        }
      }
      incoming_serial_byte = Serial.read();
      if (incoming_serial_byte == 13) {        // did we get a carriage return?
        looping = 0;
      } 
      else {
        Serial.write(incoming_serial_byte);
        incoming_serial_byte = uppercase(incoming_serial_byte);
        EEPROM.write((memory_start(memory_number)+memory_index),incoming_serial_byte);
#ifdef DEBUG_EEPROM
        Serial.print(F("serial_program_memory: wrote "));
        Serial.print(incoming_serial_byte);
        Serial.print(F(" to location "));
        Serial.println((memory_start(memory_number)+memory_index));
#endif
        memory_index++;
        if ((memory_start(memory_number) + memory_index) == memory_end(memory_number)) {    // are we at last memory location?
          looping = 0;
          Serial.println(F("Memory full, truncating."));
        }
      }
    }  //while (looping)
    // write terminating 255
    EEPROM.write((memory_start(memory_number)+memory_index),255);
#ifdef DEBUG_EEPROM
    Serial.print(F("serial_program_memory: wrote 255 to location "));
    Serial.println((memory_start(memory_number)+memory_index));
#endif
    Serial.print(F("\n\rWrote memory #"));
    Serial.print(memory_number+1);
    Serial.println();
  } 
  else {
    Serial.println(F("\n\rError"));
  }

}

#endif
#endif
#endif
#endif

//---------------------------------------------------------------------

#ifdef FEATURE_MEMORIES
void command_program_memory()
//reads in a memory number from paddles; that number becomes a parameter for program_memory routine

{
  byte cw_char;
  cw_char = get_cw_input_from_user(0);            // get another cw character from the user to find out which memory number
#ifdef DEBUG_COMMAND_MODE
  Serial.print(F("command_program_memory: cw_char: "));
  Serial.println(cw_char);
#endif
  if (cw_char > 0) {  
    switch (cw_char) {
    case 47: 
      program_memory(0); 
      break;      // 1 = memory 0; 101111b
    case 39: 
      program_memory(1);  // 100111b
      break;
    case 35: 
      program_memory(2);  //100011b
      break;
    default: 
      send_char('?',NORMAL); 
      break;
    }
  }
}
#endif

//---------------------------------------------------------------------

#ifdef FEATURE_MEMORIES
byte memory_nonblocking_delay(unsigned long delaytime)
{
  // 2012-04-20 was long starttime = millis();
  unsigned long starttime = millis();

  while ((millis() - starttime) < delaytime) {
    check_paddles();
    if (((dit_buffer) || (dah_buffer) || (analogbuttonread(0))) && (machine_mode != BEACON)) {   // exit if the paddle or button0 was hit
      dit_buffer = 0;
      dah_buffer = 0;
      while (analogbuttonread(0)) {
      }
      return 1;
    }
  }
  return 0;
}

#endif

//---------------------------------------------------------------------
void check_button0()
{
  if (analogbuttonread(0)) {
    button0_buffer = 1;
  }
}

//---------------------------------------------------------------------

#ifdef FEATURE_MEMORIES
void play_memory(byte memory_number)
{
  
  char ascii_letter = ' ';

  if (memory_number > (number_of_memories - 1)) {
    boop();
    return;
  }

  String serial_number_string;
  static byte prosign_flag = 0;
  play_memory_prempt = 0;
  byte eeprom_byte_read;

#ifdef DEBUG_PLAY_MEMORY
  Serial.print(F("play_memory: called with memory_number:"));
  Serial.println(memory_number);
#endif  

  button0_buffer = 0;

  if (machine_mode == NORMAL) {
#ifdef FEATURE_SERIAL
    Serial.println();
#endif
  }

  for (int y = (memory_start(memory_number)); (y < (memory_end(memory_number)+1)); y++) {

    if (machine_mode == NORMAL) {
      check_potentiometer();
      check_button0();
      service_display();
    }

#ifdef FEATURE_SERIAL
    check_serial();
#endif

    if ((play_memory_prempt == 0) && (pause_sending_buffer == 0)) {
      eeprom_byte_read = EEPROM.read(y);
      if (eeprom_byte_read < 255) {

#ifdef DEBUG_PLAY_MEMORY
        Serial.print(F("\n\nplay_memory:\r\n\r"));
        Serial.print(F("    Memory number:"));
        Serial.println(memory_number);
        Serial.print(F("    EEPROM location:"));
        Serial.println(y);
        Serial.print(F("    eeprom_byte_read:"));
        Serial.println(eeprom_byte_read);
#endif

        if (machine_mode == NORMAL) {
          
        //move this into feature_serial later, when characters will not be displayed on lcd...
        ascii_letter = convert_cw_number_to_ascii(eeprom_byte_read);
          
#ifdef FEATURE_SERIAL
          Serial.write(ascii_letter);
#endif //FEATURE_SERIAL

          if (lcd_send_echo) {
            display_scroll_print_char(ascii_letter); 
            service_display();
          }     
        }

        if (prosign_flag) {
          send_char(eeprom_byte_read,OMIT_LETTERSPACE);
          prosign_flag = 0;
        } 
        else {
          send_char(eeprom_byte_read,NORMAL);         // no - play the character
        }

        if (machine_mode != BEACON) {
          if ((dit_buffer) || (dah_buffer) || (button0_buffer)) {   // exit if the paddle or button0 was hit
            dit_buffer = 0;
            dah_buffer = 0;
            button0_buffer = 0;
            while (analogbuttonread(0)) {
            }
            return;
          }
        }

      } 
      else {
        if (y == (memory_start(memory_number))) {      // memory is totally empty - do a boop
          lcd_center_print_timed("Memory empty", 0, default_display_msg_delay);
        }
        return;
      }
    } 
    else {
      if (pause_sending_buffer == 0) {
        y = (memory_end(memory_number)+1);   // we got a play_memory_prempt flag, exit out
      } 
      else {
        y--;  // we're in a pause mode, so sit and spin awhile
      }
    }
  }

}
#endif

//---------------------------------------------------------------------

#ifdef FEATURE_MEMORIES
void program_memory(int memory_number)
// accumulate characters and/or spaces, with spaces encoded a 0
// characters are up to seven bits, with a "1" start bit
// and subsequently dah represented by 1 and dit represented by 0
// write to memory without converting to ascii
// optionally, trim trailing spaces
// terminal 255 marks end of message

{

  String lcd_print_string;
  lcd_print_string.concat("Pgm Memory ");
  lcd_print_string.concat(memory_number+1);
  lcd_center_print_timed(lcd_print_string, 0, default_display_msg_delay);

  send_dit(AUTOMATIC_SENDING);

  byte paddle_hit = 0;
  byte loop1 = 1;
  byte loop2 = 1;
  unsigned long last_element_time = 0;
  int memory_location_index = 0;
  byte cwchar = 1;
  byte space_count = 0;

  dit_buffer = 0;
  dah_buffer = 0;
  while ((digitalRead(paddle_left) == HIGH) && (digitalRead(paddle_right) == HIGH) && (!analogbuttonread(0))) { 
  }  // loop until user starts sending or hits the button

  while (loop2) { // recording to memory

#ifdef DEBUG_MEMORY_WRITE
    Serial.print(F("program_memory: entering loop2\r\n\r"));
#endif

    cwchar = 1;
    paddle_hit = 0;
    loop1 = 1;

    while (loop1) {  // get a word, up to space
      check_paddles();             
      if (dit_buffer) {
        send_dit(MANUAL_SENDING);
        dit_buffer = 0;
        paddle_hit = 1;
        cwchar = cwchar << 1;
        last_element_time = millis();
#ifdef DEBUG_MEMORY_WRITE
        Serial.write(".");
#endif
      }
      if (dah_buffer) {
        send_dah(MANUAL_SENDING);
        dah_buffer = 0;
        paddle_hit = 1;
        cwchar = cwchar << 1;
        cwchar = cwchar | 1;
        last_element_time = millis();
#ifdef DEBUG_MEMORY_WRITE
        Serial.write("_");
#endif
      }
      if ((paddle_hit) && (millis() > (last_element_time + (float(600/wpm) * length_letterspace)))) {   // this character is over
        loop1 = 0;
      }
      
      if (cwchar > 127) {
         cwchar = 76;  // '?' = 1001100b
      }

      if ((paddle_hit == 0) && (millis() > (last_element_time + ((float(1200/wpm) * length_wordspace)))) && (space_count < program_memory_limit_consec_spaces)) {   // we have a space
        loop1 = 0;
        cwchar = 0;
        space_count++;
      }

      while (analogbuttonread(0)) {    // hit the button to get out of command mode if no paddle was hit
        loop1 = 0;
        loop2 = 0;
      }
    }  //loop1

    if (cwchar != 0) {
      space_count = 0;
    }

    // write the character to memory
    if (cwchar != 1) {

#ifdef DEBUG_MEMORY_WRITE
      Serial.print(F("program_memory: write_character_to_memory"));
      Serial.print(F(" mem number:"));
      Serial.print(memory_number);
      Serial.print(F("  memory_location_index:"));
      Serial.print(memory_location_index);
      Serial.print(F("  EEPROM location:"));
      Serial.print(memory_start[memory_number]+memory_location_index);
      Serial.print(F("   cwchar:"));
      Serial.print(cwchar);
      //Serial.print(F("   ascii to write:"));
      //Serial.println(convert_cw_number_to_ascii(cwchar));
#endif

      EEPROM.write((memory_start(memory_number)+memory_location_index),cwchar);
      memory_location_index++;
    }

    // are we out of memory locations?
    if ((memory_start(memory_number) + memory_location_index) == memory_end(memory_number)) {
      loop1 = 0;
      loop2 = 0;
#ifdef DEBUG_MEMORY_WRITE
      Serial.println(F("program_memory: out of memory location"));
#endif
    }
  }

  //write terminating 255 at end
#ifdef DEBUG_MEMORY_WRITE
  Serial.println(F("program_memory: writing memory termination"));
#endif

  EEPROM.write((memory_start(memory_number) + memory_location_index),255);

#ifdef OPTION_PROG_MEM_TRIM_TRAILING_SPACES
  for (int x = (memory_location_index-1); x > 0; x--) {
    if (EEPROM.read((memory_start(memory_number) + x)) == 0) {
      EEPROM.write((memory_start(memory_number) + x),255);
    } 
    else {
      x = 0;
    }
  }
#endif

  lcd_center_print_timed("Done", 0, default_display_msg_delay);

  //play_memory(memory_number);
  boop;
  boop;

  //  send_dit(AUTOMATIC_SENDING);

}
#endif

//---------------------------------------------------------------------

#ifdef FEATURE_MEMORIES
int memory_start(byte memory_number) {
  return (memory_area_start + (memory_number * ((memory_area_end - memory_area_start) / number_of_memories)));
}
#endif

//---------------------------------------------------------------------

#ifdef FEATURE_MEMORIES
int memory_end(byte memory_number) {
  return (memory_start(memory_number) - 1 + ((memory_area_end - memory_area_start)/number_of_memories));
}
#endif

//---------------------------------------------------------------------

void initialize_pins() {

  pinMode (paddle_left, INPUT);
  digitalWrite (paddle_left, HIGH);
  pinMode (paddle_right, INPUT);
  digitalWrite (paddle_right, HIGH);

  pinMode (tx_key_line, OUTPUT);
  digitalWrite (tx_key_line, LOW);

  pinMode (sidetone_line, OUTPUT);
  digitalWrite (sidetone_line, LOW);

}

//---------------------------------------------------------------------

#ifdef DEBUG_STARTUP
void initialize_debug_startup(){

  serial_status();  
#ifdef FEATURE_SERIAL
  Serial.println(F("FEATURE_SERIAL"));
#endif
#ifdef FEATURE_COMMAND_LINE_INTERFACE
  Serial.println(F("FEATURE_COMMAND_LINE_INTERFACE"));
#endif
#ifdef FEATURE_SAY_HI
  Serial.println(F("FEATURE_SAY_HI"));
#endif
#ifdef FEATURE_MEMORIES
  Serial.println(F("FEATURE_MEMORIES"));
#endif
#ifdef FEATURE_BEACON
  Serial.println(F("FEATURE_BEACON"));
#endif
#ifdef FEATURE_SERIAL_HELP
  Serial.println(F("FEATURE_SERIAL_HELP"));
#endif
#ifdef FEATURE_DEAD_OP_WATCHDOG
  Serial.println(F("FEATURE_DEAD_OP_WATCHDOG"));
#endif
#ifdef FEATURE_AUTOSPACE
  Serial.println(F("FEATURE_AUTOSPACE"));
#endif
#ifdef FEATURE_FARNSWORTH
  Serial.println(F("FEATURE_FARNSWORTH"));
#endif
  Serial.println(F("setup: exiting, going into loop"));
}
#endif //DEBUG_STARTUP


