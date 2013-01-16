#include <stdio.h>
#include <EEPROM.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
//#include <LiquidCrystal.h>      // uncomment for FEATURE_DISPLAY in combination with FEATURE_LCD_4BIT and LiquidCrystal lines below
//#include <Wire.h>               // uncomment for any I2C feature
//#include <Adafruit_MCP23017.h>       // uncomment for FEATURE_DISPLAY in combination with FEATURE_LCD_I2C and Adafruit_RGBLCDShield lines below
//#include <Adafruit_RGBLCDShield.h>   // uncomment for FEATURE_DISPLAY in combination with FEATURE_LCD_I2C and Adafruit_RGBLCDShield lines below


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
// * PS2 keyboard
// * Hellscreiber
// * Disable ability to not have a potentiometer - The potentiometer is an integral part of SO and the only way to set speed
// * Disable multiple transmitters - a single keyer output remains
// * Disable all PTT lines

// Command Line Interface ("CLI") (USB Port) (Note: turn on carriage return if using Arduino Serial Monitor program)
//    CW Keyboard: type what you want the keyer to send (all commands are preceded with a backslash ( \ )
//    \?     Help                                      (requires FEATURE_SERIAL_HELP)
//    \#     Play memory #                             (requires FEATURES_MEMORIES; can only play memories 1 - 9)
//    \a     Iambic A mode
//    \b     Iambic B mode
//    \d     Ultimatic mode
//    \e#### Set serial number to ####
//    \f#### Set sidetone frequency to #### hertz
//    \g     Bug mode
//    \i     Transmit enable/disable
//    \j###  Dah to dit ratio (300 = 3.00)
//    \k     Callsign receive practice
//    \l##   Set weighting (50 = normal)
//    \m###  Set Farnsworth speed
//    \n     Toggle paddle reverse
//    \o     Toggle sidetone on/off
//    \p#    Program memory #
//    \q##   Switch to QRSS mode, dit length ## seconds
//    \r     Switch to regular speed mode
//    \s     Status
//    \t     Tune mode
//    \y#    Change wordspace to # elements (# = 1 to 9)
//    \z     Autospace on/off
//    \+     Create prosign
//    \!##   Repeat play memory
//    \|#### Set memory repeat (milliseconds)
//    \*     Toggle paddle echo
//    \^     Toggle wait for carriage return to send CW / send CW immediately
//    \~     Reset unit

// Buttons
//    button 0: command mode / command mode exit
//    button 0 + left paddle:  increase cw speed
//    button 0 + right paddle: decrease cw speed
//    button 1 - 12 hold + left paddle: repeat memory

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
//    Y#### Change memory repeat delay to #### mS
//    Z  Autospace On/Off
//    #  Play a memory without transmitting

// Memory Macros
//    \#     Jump to memory #
//    \c     Play serial number with cut numbers
//    \d###  Delay for ### seconds
//    \e     Play serial number, then increment
//    \f#### Change sidetone to #### hertz (must be four digits - use leading zero below 1000 hz)
//    \n     Decrement serial number, do not send
//    \q##   Switch to QRSS mode, dit length ## seconds
//    \r     Switch to regular speed mode
//    \t###  Transmit for ### seconds (must be three digits, use leading zeros if necessary)
//    \w###  Set regular mode speed to ### WPM (must be three digits, use leading zeros if necessary)
//    \y#    Increase speed # WPM
//    \z#    Decrease speed # WPM
//    \+     Prosign the next two characters

// Useful Stuff
//    Reset to defaults: squeeze both paddles at power up (good to use if you dorked up the speed and don't have the CLI)
//    Press the right paddle to enter straight key mode at power up
//    Press the left  paddle at power up to enter and stay forever in beacon mode

// compile time features and options - comment or uncomment to add or delete features
// FEATURES add more bytes to the compiled binary, OPTIONS change code behavior
#define FEATURE_SERIAL
#define FEATURE_COMMAND_LINE_INTERFACE        // this requires FEATURE_SERIAL
#define FEATURE_COMMAND_BUTTONS  // this is now required for the regular buttons and command mode (added in version 2012061601)
#define FEATURE_SAY_HI
#define FEATURE_MEMORIES
//#define FEATURE_MEMORY_MACROS
//#define FEATURE_WINKEY_EMULATION    // this requires FEATURE_SERIAL - disabling Automatic Software Reset is recommended (see documentation)
//#define OPTION_WINKEY_2_SUPPORT     // requires FEATURE_WINKEY_EMULATION
//#define FEATURE_BEACON
//#define FEATURE_CALLSIGN_RECEIVE_PRACTICE
//#define FEATURE_SERIAL_HELP
//#define FEATURE_DEAD_OP_WATCHDOG
//#define FEATURE_AUTOSPACE
//#define FEATURE_FARNSWORTH
//#define FEATURE_DL2SBA_BANKSWITCH  // Switch memory banks feature as described here: http://dl2sba.com/index.php?option=com_content&view=article&id=131:nanokeyer&catid=15:shack&Itemid=27#english
//#define FEATURE_DISPLAY            // LCD display support (include one of the hardware options below)
//#define FEATURE_LCD_4BIT           // classic LCD display using 4 I/O lines
//#define FEATURE_LCD_I2C            // I2C LCD display using MCP23017 at addr 0x20 (Adafruit)
//#define FEATURE_CW_DECODER
//#define OPTION_NON_ENGLISH_EXTENSIONS  // add support for additional CW characters (i.e. À, Å, Þ, etc.)


//#define OPTION_SUPPRESS_SERIAL_BOOT_MSG
//#define OPTION_CLI_WINKEY_AUTOSWITCH
#define OPTION_SERIAL_PORT_DEFAULT_WINKEY_EMULATION  // this will make Winkey emulation be the default at boot up; hold command button down at boot up to activate CLI mode
//#define OPTION_WINKEY_DISCARD_BYTES_AT_STARTUP     // if ASR is not disabled, you may need this to discard errant serial port bytes at startup
//#define OPTION_WINKEY_STRICT_EEPROM_WRITES_MAY_WEAR_OUT_EEPROM // with this activated the unit will write non-volatile settings to EEPROM when set by Winkey commands
//#define OPTION_WINKEY_SEND_WORDSPACE_AT_END_OF_BUFFER
//#define OPTION_WINKEY_EXTENDED_COMMANDS            // in development
//#define OPTION_REVERSE_BUTTON_ORDER                // This is mainly for the DJ0MY NanoKeyer http://nanokeyer.wordpress.com/
#define OPTION_PROG_MEM_TRIM_TRAILING_SPACES         // trim trailing spaces from memory when programming in command mode
#define OPTION_DIT_PADDLE_NO_SEND_ON_MEM_RPT
//#define OPTION_MORE_DISPLAY_MSGS                     // additional optional display messages - comment out to save memory
//#define OPTION_N1MM_WINKEY_TAB_BUG_WORKAROUND        // enable this to ignore the TAB key in the Send CW window (this breaks SO2R functionality in N1MM)
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
//#define DEBUG_WINKEY_PROTOCOL
//#define DEBUG_CHECK_SERIAL
//#define FEATURE_FREQ_COUNTER                 // not implemented yet
//#define DEBUG_VARIABLE_DUMP
//#define DEBUG_BUTTONS
//#define DEBUG_COMMAND_MODE
//#define DEBUG_GET_CW_INPUT_FROM_USER
//#define DEBUG_POTENTIOMETER
//#define DEBUG_CW_DECODER
//#define DEBUG_CW_DECODER_WPM

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
#define cw_decoder_pin A5 //A3

#ifdef FEATURE_COMMAND_BUTTONS
#define analog_buttons_pin A3
//#define analog_buttons_pin A6  //Rev A & B nanokeyer
#endif //FEATURE_COMMAND_BUTTONS

#ifdef FEATURE_LCD_4BIT
//lcd pins
#define lcd_rs A2
#define lcd_enable 10
#define lcd_d4 6
#define lcd_d5 7
#define lcd_d6 8
#define lcd_d7 9
#endif //FEATURE_LCD_4BIT

//LiquidCrystal lcd(lcd_rs, lcd_enable, lcd_d4, lcd_d5, lcd_d6, lcd_d7);  // uncomment this if FEATURE_LCD_4BIT is enabled above
//Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();      // uncomment this for FEATURE_LCD_I2C

// Initial and hardcoded settings
#define initial_speed_wpm 26             // "factory default" keyer speed setting
#define initial_sidetone_freq 600        // "factory default" sidetone frequency setting
#define hz_high_beep 1500                // frequency in hertz of high beep
#define hz_low_beep 400                  // frequency in hertz of low beep
#define initial_dah_to_dit_ratio 300     // 300 = 3 / normal 3:1 ratio
#define initial_qrss_dit_length 1        // QRSS dit length in seconds
#define initial_pot_wpm_low_value 13     // Potentiometer WPM fully CCW
#define initial_pot_wpm_high_value 35    // Potentiometer WPM fully CW
#define potentiometer_change_threshold 1 // don't change the keyer speed until pot wpm has changed more than this
//#define default_serial_baud_rate 9600
#define default_serial_baud_rate 115200
#define send_buffer_size 50
#define default_length_letterspace 3
#define default_length_wordspace 7
#define default_keying_compensation 0    // number of milliseconds to extend all dits and dahs - for QSK on boatanchors
#define default_first_extension_time 0   // number of milliseconds to extend first sent dit or dah
#define default_pot_full_scale_reading 1023
#define default_weighting 50             // 50 = weighting factor of 1 (normal)
#define number_of_memories byte(12)            // the number of memories (duh)
#define memory_area_start 20             // the eeprom location where memory space starts
#define memory_area_end 1023             // the eeprom location where memory space ends
#define winkey_c0_wait_time 1 //2000         // the number of milliseconds to wait to send 0xc0 byte after send buffer has been sent
#define winkey_discard_bytes_startup 3   // this is used if OPTION_WINKEY_DISCARD_BYTES_AT_STARTUP is enabled above
#define default_memory_repeat_time 3000  // time in milliseconds
#define lcd_columns 16
#define lcd_rows 2
#define eeprom_magic_number 73
#define program_memory_limit_consec_spaces 1
#define serial_leading_zeros 1            // set to 1 to activate leading zeros in serial numbers (i.e. #1 = 001)
#define serial_cut_numbers 0              // set to 1 to activate cut numbers in serial numbers (i.e. #10 = 1T, #19 = 1N)
#define p_command_single_digit_wait_time 2

#ifdef FEATURE_COMMAND_BUTTONS
#define analog_buttons_number_of_buttons 4  //16
#define analog_buttons_r1 10
#define analog_buttons_r2 1
#endif

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

#define CW 0

#define SERIAL_NORMAL 0
#define SERIAL_WINKEY_EMULATION 1

#define SERIAL_SEND_BUFFER_SPECIAL_START 13
#define SERIAL_SEND_BUFFER_WPM_CHANGE 14        // was 200
#define SERIAL_SEND_BUFFER_TIMED_KEY_DOWN 17    // was 203
#define SERIAL_SEND_BUFFER_TIMED_WAIT 18        // was 204
#define SERIAL_SEND_BUFFER_NULL 19              // was 205
#define SERIAL_SEND_BUFFER_PROSIGN 20           // was 206
#define SERIAL_SEND_BUFFER_HOLD_SEND 21         // was 207
#define SERIAL_SEND_BUFFER_HOLD_SEND_RELEASE 22 // was 208
#define SERIAL_SEND_BUFFER_MEMORY_NUMBER 23 //23     // was  210
#define SERIAL_SEND_BUFFER_SPECIAL_END 24

#define SERIAL_SEND_BUFFER_NORMAL 0
#define SERIAL_SEND_BUFFER_TIMED_COMMAND 1
#define SERIAL_SEND_BUFFER_HOLD 2

#ifdef FEATURE_WINKEY_EMULATION
#define WINKEY_NO_COMMAND_IN_PROGRESS 0
#define WINKEY_UNBUFFERED_SPEED_COMMAND 1
#define WINKEY_UNSUPPORTED_COMMAND 2
#define WINKEY_POINTER_COMMAND 3
#define WINKEY_ADMIN_COMMAND 4
#define WINKEY_PAUSE_COMMAND 5
#define WINKEY_KEY_COMMAND 6
#define WINKEY_SETMODE_COMMAND 7
#define WINKEY_SIDETONE_FREQ_COMMAND 8
#define WINKEY_ADMIN_COMMAND_ECHO 9
#define WINKEY_BUFFERED_SPEED_COMMAND 10
#define WINKEY_DAH_TO_DIT_RATIO_COMMAND 11
#define WINKEY_KEYING_COMPENSATION_COMMAND 12
#define WINKEY_FIRST_EXTENSION_COMMAND 13
#define WINKEY_SET_POT_PARM1_COMMAND 16
#define WINKEY_SET_POT_PARM2_COMMAND 17
#define WINKEY_SET_POT_PARM3_COMMAND 18
#define WINKEY_SOFTWARE_PADDLE_COMMAND 19
#define WINKEY_CANCEL_BUFFERED_SPEED_COMMAND 20
#define WINKEY_HSCW_COMMAND 22
#define WINKEY_BUFFERED_HSCW_COMMAND 23
#define WINKEY_WEIGHTING_COMMAND 24
#define WINKEY_KEY_BUFFERED_COMMAND 25
#define WINKEY_WAIT_BUFFERED_COMMAND 26
#define WINKEY_POINTER_01_COMMAND 27
#define WINKEY_POINTER_02_COMMAND 28
#define WINKEY_POINTER_03_COMMAND 29
#define WINKEY_FARNSWORTH_COMMAND 30
#define WINKEY_MERGE_COMMAND 31
#define WINKEY_MERGE_PARM_2_COMMAND 32
#define WINKEY_SET_PINCONFIG_COMMAND 33
#define WINKEY_EXTENDED_COMMAND 34
#ifdef OPTION_WINKEY_2_SUPPORT
#define WINKEY_SEND_MSG 35
#endif //OPTION_WINKEY_2_SUPPORT
#define WINKEY_LOAD_SETTINGS_PARM_1_COMMAND 101
#define WINKEY_LOAD_SETTINGS_PARM_2_COMMAND 102
#define WINKEY_LOAD_SETTINGS_PARM_3_COMMAND 103
#define WINKEY_LOAD_SETTINGS_PARM_4_COMMAND 104
#define WINKEY_LOAD_SETTINGS_PARM_5_COMMAND 105
#define WINKEY_LOAD_SETTINGS_PARM_6_COMMAND 106
#define WINKEY_LOAD_SETTINGS_PARM_7_COMMAND 107
#define WINKEY_LOAD_SETTINGS_PARM_8_COMMAND 108
#define WINKEY_LOAD_SETTINGS_PARM_9_COMMAND 109
#define WINKEY_LOAD_SETTINGS_PARM_10_COMMAND 110
#define WINKEY_LOAD_SETTINGS_PARM_11_COMMAND 111
#define WINKEY_LOAD_SETTINGS_PARM_12_COMMAND 112
#define WINKEY_LOAD_SETTINGS_PARM_13_COMMAND 113
#define WINKEY_LOAD_SETTINGS_PARM_14_COMMAND 114
#define WINKEY_LOAD_SETTINGS_PARM_15_COMMAND 115

#define HOUSEKEEPING 0
#define SERVICE_SERIAL_BYTE 1
#endif //FEATURE_WINKEY_EMULATION

#define AUTOMATIC_SENDING 0
#define MANUAL_SENDING 1

#define ULTIMATIC_NORMAL 0
#define ULTIMATIC_DIT_PRIORITY 1
#define ULTIMATIC_DAH_PRIORITY 2

#ifdef FEATURE_WINKEY_EMULATION
// alter these below to map alternate sidetones for Winkey emulation
#ifdef OPTION_WINKEY_2_SUPPORT
#define WINKEY_SIDETONE_1 3759
#define WINKEY_SIDETONE_2 1879
#define WINKEY_SIDETONE_3 1252
#define WINKEY_SIDETONE_4 940
#define WINKEY_SIDETONE_5 752
#define WINKEY_SIDETONE_6 625
#define WINKEY_SIDETONE_7 535
#define WINKEY_SIDETONE_8 469
#define WINKEY_SIDETONE_9 417
#define WINKEY_SIDETONE_10 375
#else //OPTION_WINKEY_2_SUPPORT
#define WINKEY_SIDETONE_1 4000
#define WINKEY_SIDETONE_2 2000
#define WINKEY_SIDETONE_3 1333
#define WINKEY_SIDETONE_4 1000
#define WINKEY_SIDETONE_5 800
#define WINKEY_SIDETONE_6 666
#define WINKEY_SIDETONE_7 571
#define WINKEY_SIDETONE_8 500
#define WINKEY_SIDETONE_9 444
#define WINKEY_SIDETONE_10 400
#endif //OPTION_WINKEY_2_SUPPORT
#endif //FEATURE_WINKEY_EMULATION



// Variables and stuff
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

#ifdef FEATURE_DISPLAY
enum lcd_statuses {LCD_CLEAR, LCD_REVERT, LCD_TIMED_MESSAGE, LCD_SCROLL_MSG};
#define default_display_msg_delay 1000
#endif //FEATURE_DISPLAY

#ifdef FEATURE_LCD_I2C
#define RED 0x1
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7
byte lcdcolor = GREEN;  // default color for RGB LCD display
#endif //FEATURE_LCD_I2C


#ifdef OPTION_WINKEY_2_SUPPORT
byte wk2_mode = 1;
byte wk2_both_tx_activated = 0;
byte wk2_paddle_only_sidetone = 0;
#endif //OPTION_WINKEY_2_SUPPORT

//bbbbb
#ifdef FEATURE_DISPLAY
byte lcd_status = LCD_CLEAR;
unsigned long lcd_timed_message_clear_time = 0;
byte lcd_previous_status = LCD_CLEAR;
byte lcd_scroll_buffer_dirty = 0;
String lcd_scroll_buffer[lcd_rows];
byte lcd_scroll_flag = 0;
byte lcd_paddle_echo = 1;
byte lcd_send_echo = 1;
long lcd_paddle_echo_buffer = 0;
unsigned long lcd_paddle_echo_buffer_decode_time = 0;

#endif

#ifdef DEBUG_VARIABLE_DUMP
long dit_start_time;
long dit_end_time;
long dah_start_time;
long dah_end_time;
#endif

#ifdef FEATURE_COMMAND_BUTTONS
int button_array_high_limit[analog_buttons_number_of_buttons];
int button_array_low_limit[analog_buttons_number_of_buttons];
long button_last_add_to_send_buffer_time = 0;
#endif

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
byte cli_paddle_echo = 0;
long cli_paddle_echo_buffer = 0;
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
byte repeat_memory = 255;
unsigned int memory_repeat_time = default_memory_repeat_time;
unsigned long last_memory_repeat_time = 0;
#endif //FEATURE_MEMORIES

#ifdef FEATURE_SERIAL
byte serial_mode = SERIAL_NORMAL;
#ifdef FEATURE_COMMAND_LINE_INTERFACE
prog_uchar string_k3ng_keyer[] __attribute__((section(".progmem.data"))) = {"\n\rK3NG Keyer Version "};
#ifdef FEATURE_SERIAL_HELP
prog_uchar string_enter_help[] __attribute__((section(".progmem.data"))) = {"\n\rEnter \\? for help\n\n\r"};
#endif
prog_uchar string_qrss_mode[] __attribute__((section(".progmem.data"))) = {"Setting keyer to QRSS Mode. Dit length: "};
prog_uchar string_setting_serial_number[] __attribute__((section(".progmem.data"))) = {"Setting serial number to "};
prog_uchar string_setting_dah_to_dit_ratio[] __attribute__((section(".progmem.data"))) = {"Setting dah to dit ratio to "};
#ifdef FEATURE_CALLSIGN_RECEIVE_PRACTICE
prog_uchar string_callsign_receive_practice[] __attribute__((section(".progmem.data"))) = {"Callsign receive practice; type in callsign and hit ENTER.\n\r"
 "If you are using the Arduino serial monitor, select \"Carriage Return\" line ending.\n\r"
 "Enter a blackslash \\ to exit.\n\r"};
#endif //FEATURE_CALLSIGN_RECEIVE_PRACTICE
#endif //FEATURE_COMMAND_LINE_INTERFACE
#endif //FEATURE_SERIAL

#ifdef FEATURE_WINKEY_EMULATION
byte winkey_serial_echo = 1;
byte winkey_host_open = 0;
unsigned int winkey_last_unbuffered_speed_wpm = 0;
byte winkey_buffer_counter = 0;
byte winkey_buffer_pointer = 0;
byte winkey_dit_invoke = 0;
byte winkey_dah_invoke = 0;
long winkey_paddle_echo_buffer = 0;
byte winkey_paddle_echo_activated = 0;
unsigned long winkey_paddle_echo_buffer_decode_time = 0;
byte winkey_sending = 0;
byte winkey_interrupted = 0;
#endif //FEATURE_WINKEY_EMULATION

#ifdef FEATURE_SERIAL_HELP
#ifdef FEATURE_SERIAL
#ifdef FEATURE_COMMAND_LINE_INTERFACE
prog_uchar serial_help_string[] __attribute__((section(".progmem.data"))) = {"\n\rK3NG Keyer Help\n\r\n\r"
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
  #ifdef FEATURE_CALLSIGN_RECEIVE_PRACTICE
  "\\K\t\t: Callsign receive practice\n\r"
  #endif
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
  "\\+\t\t: Prosign\n\r"
  "\\\\\t\t: Empty keyboard send buffer\n\r"
  "\nMemory Macros:\n\r"
  "\\#\t\t: Jump to memory #\n\r"
  "\\D###\t\t: Delay for ### seconds\n\r"
  "\\E\t\t: Send serial number\n\r"
  "\\F####\t\t: Set sidetone to #### hz\n\r"
  "\\N\t\t: Decrement serial number\n\r"
  "\\Q##\t\t: Switch to QRSS with ## second dit length\n\r"
  "\\R\t\t: Switch to regular speed mode\n\r"
  "\\T###\t\t: Transmit for ### seconds\n\r"
  "\\W###\t\t: Change WPM to ###\n\r"
  "\\Y#\t\t: Increase speed # WPM\n\r"
  "\\Z#\t\t: Decrease speed # WPM\n\r"
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
// eprom location 13 available
#define EEPROM_memory_repeat_time_high 14
#define EEPROM_memory_repeat_time_low 15

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


// this code is a friggin work of art.  free as in beer software sucks.


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
    while (digitalRead(paddle_left) == LOW && digitalRead(paddle_right) == LOW) {}
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
  } else {
    if (digitalRead(paddle_right) == LOW) {
      keyer_mode = STRAIGHT;
    }
  }

  #ifdef DEBUG_CAPTURE_COM_PORT
  Serial.begin(serial_baud_rate);
  debug_capture();
  #endif

  #ifdef FEATURE_COMMAND_BUTTONS
  initialize_analog_button_array();
  #endif

  // initialize serial port
  #ifdef FEATURE_SERIAL
  #ifdef FEATURE_WINKEY_EMULATION
  #ifdef FEATURE_COMMAND_LINE_INTERFACE
  #ifdef FEATURE_COMMAND_BUTTONS
  if (analogbuttonread(0)) {
  #endif
    #ifdef OPTION_SERIAL_PORT_DEFAULT_WINKEY_EMULATION
    serial_mode = SERIAL_NORMAL;
    serial_baud_rate = default_serial_baud_rate;
    #endif //OPTION_SERIAL_PORT_DEFAULT_WINKEY_EMULATION
    #ifndef OPTION_SERIAL_PORT_DEFAULT_WINKEY_EMULATION
    serial_mode = SERIAL_WINKEY_EMULATION;
    serial_baud_rate = 1200;
    #endif  //ifndef OPTION_SERIAL_PORT_DEFAULT_WINKEY_EMULATION
  } else {
    #ifdef OPTION_SERIAL_PORT_DEFAULT_WINKEY_EMULATION
    serial_mode = SERIAL_WINKEY_EMULATION;
    serial_baud_rate = 1200;
    #endif //OPTION_SERIAL_PORT_DEFAULT_WINKEY_EMULATION
    #ifndef OPTION_SERIAL_PORT_DEFAULT_WINKEY_EMULATION
    serial_mode = SERIAL_NORMAL;
    serial_baud_rate = default_serial_baud_rate;
    #endif  //ifndef OPTION_SERIAL_PORT_DEFAULT_WINKEY_EMULATION
  }
  #ifdef FEATURE_COMMAND_BUTTONS
  while (analogbuttonread(0)) {}
  #endif //FEATURE_COMMAND_BUTTONS
  #endif //FEATURE_WINKEY_EMULATION
  #endif //FEATURE_COMMAND_LINE_INTERFACE

  #ifndef FEATURE_WINKEY_EMULATION
  #ifdef FEATURE_COMMAND_LINE_INTERFACE
  serial_mode = SERIAL_NORMAL;
  serial_baud_rate = default_serial_baud_rate;
  #endif // FEATURE_COMMAND_LINE_INTERFACE
  #endif  //ifndef FEATURE_WINKEY_EMULATION

  #ifdef FEATURE_WINKEY_EMULATION
  #ifndef FEATURE_COMMAND_LINE_INTERFACE
  serial_mode = SERIAL_WINKEY_EMULATION;
  serial_baud_rate = 1200;
  #endif // FEATURE_COMMAND_LINE_INTERFACE
  #endif  //ifndef FEATURE_WINKEY_EMULATION

  Serial.begin(serial_baud_rate);
  
  #ifdef DEBUG_STARTUP
  Serial.println(F("setup: serial port opened"));
  #endif

  #ifndef OPTION_SUPPRESS_SERIAL_BOOT_MSG
  #ifdef FEATURE_COMMAND_LINE_INTERFACE
  if (serial_mode == SERIAL_NORMAL) {
    serial_print(string_k3ng_keyer);
    Serial.write(CODE_VERSION);
    #ifdef FEATURE_SERIAL_HELP
    serial_print(string_enter_help);
    #endif
  }
  #ifdef DEBUG_MEMORYCHECK
  memorycheck();
  #endif //DEBUG_MEMORYCHECK
  #endif //FEATURE_COMMAND_LINE_INTERFACE
  #endif //ifndef OPTION_SUPPRESS_SERIAL_BOOT_MSG
  #endif //FEATURE_SERIAL

  #ifdef FEATURE_DISPLAY
  lcd.begin(lcd_columns, lcd_rows);
  #ifdef FEATURE_LCD_I2C
  lcd.setBacklight(lcdcolor);
  #endif //FEATURE_LCD_I2C
  lcd_center_print_timed("K3NG Keyer",0,4000);
  #endif //FEATURE_DISPLAY

  if (machine_mode != BEACON) {
    #ifdef FEATURE_SAY_HI
    // say HI
    // store current setting (compliments of DL2SBA - http://dl2sba.com/ )
    byte oldKey = key_tx; 
    byte oldSideTone = sidetone_mode;
    key_tx = 0;
    sidetone_mode = SIDETONE_ON;     
    
    //delay(201);
    #ifdef FEATURE_DISPLAY
    lcd_center_print_timed("h",1,4000);
    #endif
    send_char('H',NORMAL);
    #ifdef FEATURE_DISPLAY
    lcd_center_print_timed("hi",1,4000);
    #endif
    send_char('I',NORMAL);
    
    sidetone_mode = oldSideTone; 
    key_tx = oldKey;     
    #endif
    
  }
 
  #ifdef DEBUG_STARTUP
  initialize_debug_startup();
  #endif
  

}

// --------------------------------------------------------------------------------------------

void loop()
{
  
  // this is where the magic happens
  
  #ifdef OPTION_WATCHDOG_TIMER
  wdt_reset();
  #endif  //OPTION_WATCHDOG_TIMER
  
  #ifdef FEATURE_BEACON
  #ifdef FEATURE_MEMORIES
  if (machine_mode == BEACON) {
    delay(201);
    while (machine_mode == BEACON) {  // if we're in beacon mode, just keep playing memory 1
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
    #ifdef FEATURE_COMMAND_BUTTONS
    check_command_buttons();
    #endif
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

    #ifdef FEATURE_MEMORIES
    check_memory_repeat();
    #endif

    #ifdef FEATURE_DISPLAY
    check_paddles();
    service_dit_dah_buffers();
    service_send_buffer();
    service_lcd_paddle_echo();
    service_display();
    #endif
    
    #ifdef FEATURE_CW_DECODER
    service_cw_decoder();
    #endif
  }
  
}

// Subroutines --------------------------------------------------------------------------------------------


// Are you a radio artisan ?

#ifdef FEATURE_DISPLAY
void service_display() {

  #ifdef DEBUG_LOOP
  Serial.println(F("loop: entering service_display"));
  #endif    

  byte x = 0;

  if (lcd_status == LCD_REVERT) {
    lcd_status = lcd_previous_status;
    switch (lcd_status) {
      case LCD_CLEAR: lcd_clear(); break;
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
  } else {
    switch (lcd_status) {
      case LCD_CLEAR : break;
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
#endif

//-------------------------------------------------------------------------------------------------------

#ifdef FEATURE_DISPLAY

void service_lcd_paddle_echo()
{

  #ifdef DEBUG_LOOP
  Serial.println(F("loop: entering service_lcd_paddle_echo"));
  #endif    

  static byte lcd_paddle_echo_space_sent = 1;

  if ((lcd_paddle_echo_buffer) && (lcd_paddle_echo) && (millis() > lcd_paddle_echo_buffer_decode_time)) {
    display_scroll_print_char(byte(convert_cw_number_to_ascii(lcd_paddle_echo_buffer)));
    lcd_paddle_echo_buffer = 0;
    lcd_paddle_echo_buffer_decode_time = millis() + (float(600/wpm)*length_letterspace);
    lcd_paddle_echo_space_sent = 0;
  }
  if ((lcd_paddle_echo_buffer == 0) && (lcd_paddle_echo) && (millis() > (lcd_paddle_echo_buffer_decode_time + (float(1200/wpm)*(length_wordspace-length_letterspace)))) && (!lcd_paddle_echo_space_sent)) {
    display_scroll_print_char(' ');
    lcd_paddle_echo_space_sent = 1;
  }
}
#endif //FEATURE_DISPLAY

//-------------------------------------------------------------------------------------------------------

#ifdef FEATURE_DISPLAY
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

#endif //FEATURE_DISPLAY


//-------------------------------------------------------------------------------------------------------
#ifdef FEATURE_DISPLAY
void lcd_clear() {

//  for (byte x = 0;x < lcd_rows;x++) {
//    clear_display_row(x);
//  }
  lcd.clear();
  lcd_status = LCD_CLEAR;

}
#endif
//-------------------------------------------------------------------------------------------------------
#ifdef FEATURE_DISPLAY
void lcd_center_print_timed(String lcd_print_string, byte row_number, unsigned int duration)
{
  if (lcd_status != LCD_TIMED_MESSAGE) {
    lcd_previous_status = lcd_status;
    lcd_status = LCD_TIMED_MESSAGE;
    lcd.clear();
  } else {
    clear_display_row(row_number);
  }
  lcd.setCursor(((lcd_columns - lcd_print_string.length())/2),row_number);
  lcd.print(lcd_print_string);
  lcd_timed_message_clear_time = millis() + duration;
}
#endif

//-------------------------------------------------------------------------------------------------------

#ifdef FEATURE_DISPLAY
void clear_display_row(byte row_number)
{
  for (byte x = 0; x < lcd_columns; x++) {
    lcd.setCursor(x,row_number);
    lcd.print(" ");
  }
}
#endif

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
#ifdef FEATURE_MEMORIES
void check_memory_repeat() {

  #ifdef DEBUG_LOOP
  Serial.println(F("loop: entering check_memory_repeat"));
  #endif    
  
  if ((repeat_memory < number_of_memories) && ((millis() - last_memory_repeat_time) > memory_repeat_time)) {
    add_to_send_buffer(SERIAL_SEND_BUFFER_MEMORY_NUMBER);
    add_to_send_buffer(repeat_memory);
    last_memory_repeat_time = millis();
    #ifdef DEBUG_MEMORIES
    Serial.print(F("check_memory_repeat: added repeat_memory to send buffer\n\r"));
    #endif
  }

}
#endif

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

  while (Serial.available() == 0) {}  // wait for first byte
  serial_byte_in = Serial.read();
  Serial.write(serial_byte_in);
  //if ((serial_byte_in > 47) or (serial_byte_in = 20)) { Serial.write(serial_byte_in); }  // echo back
  if (serial_byte_in == '~') {
    debug_capture_dump();    // go into dump mode if we get a tilde
  } else {
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

  while (1) {}

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
    } else {
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
    } else {
      x = 0;
    }
  }

  while (1) {}

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
    #ifdef FEATURE_WINKEY_EMULATION
    if ((serial_mode == SERIAL_WINKEY_EMULATION) && (winkey_host_open)) {
      Serial.write(((pot_value_wpm_read-pot_wpm_low_value)|128));
      winkey_last_unbuffered_speed_wpm = wpm;
    }
    #endif
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
    repeat_memory = 255;
    if ((millis() - last_memory_button_buffer_insert) > 400) {    // don't do another buffer insert if we just did one - button debounce
      #ifdef FEATURE_WINKEY_EMULATION
      if (winkey_sending && winkey_host_open) {
        Serial.write(0xc2);
        winkey_interrupted = 1;
      }
      #endif

      add_to_send_buffer(SERIAL_SEND_BUFFER_MEMORY_NUMBER);
      add_to_send_buffer(memory_number_to_put_in_buffer);
      last_memory_button_buffer_insert = millis();
    }
  } else {
    #ifdef DEBUG_MEMORIES
    Serial.println(F("put_memory_button_in_buffer: bad memory_number_to_put_in_buffer"));
    #endif
  }
}
#endif

//-------------------------------------------------------------------------------------------------------

void check_paddles()
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

  #ifdef FEATURE_WINKEY_EMULATION
  if (winkey_dit_invoke) {
    dit_buffer = 1;
  }
  if (winkey_dah_invoke) {
    dah_buffer = 1;
  }
  #endif //FEATURE_WINKEY_EMULATION

  if (keyer_mode == ULTIMATIC) {
    if (ultimatic_mode == ULTIMATIC_NORMAL) {
      switch (last_closure) {
        case DIT_CLOSURE_DAH_OFF:
          if (dah_buffer) {
            if (dit_buffer) {
              last_closure = DAH_CLOSURE_DIT_ON;
              dit_buffer = 0;
            } else {
              last_closure = DAH_CLOSURE_DIT_OFF;
            }
          } else {
            if (!dit_buffer) {
              last_closure = NO_CLOSURE;
            }
          }
          break;
        case DIT_CLOSURE_DAH_ON:
          if (dit_buffer) {
            if (dah_buffer) {
              dah_buffer = 0;
            } else {
              last_closure = DIT_CLOSURE_DAH_OFF;
            }
          } else {
            if (dah_buffer) {
              last_closure = DAH_CLOSURE_DIT_OFF;
            } else {
              last_closure = NO_CLOSURE;
            }
          }
          break;

        case DAH_CLOSURE_DIT_OFF:
          if (dit_buffer) {
            if (dah_buffer) {
              last_closure = DIT_CLOSURE_DAH_ON;
              dah_buffer = 0;
            } else {
              last_closure = DIT_CLOSURE_DAH_OFF;
            }
          } else {
            if (!dah_buffer) {
              last_closure = NO_CLOSURE;
            }
          }
          break;

        case DAH_CLOSURE_DIT_ON:
          if (dah_buffer) {
            if (dit_buffer) {
              dit_buffer = 0;
            } else {
              last_closure = DAH_CLOSURE_DIT_OFF;
            }
          } else {
            if (dit_buffer) {
              last_closure = DIT_CLOSURE_DAH_OFF;
            } else {
              last_closure = NO_CLOSURE;
            }
          }
          break;

        case NO_CLOSURE:
          if ((dit_buffer) && (!dah_buffer)) {
            last_closure = DIT_CLOSURE_DAH_OFF;
          } else {
            if ((dah_buffer) && (!dit_buffer)) {
              last_closure = DAH_CLOSURE_DIT_OFF;
            } else {
              if ((dit_buffer) && (dah_buffer)) {
                // need to handle dit/dah priority here
                last_closure = DIT_CLOSURE_DAH_ON;
                dah_buffer = 0;
              }
            }
          }
          break;
      }
    } else {
     if ((dit_buffer) && (dah_buffer)) {   // dit or dah priority mode
       if (ultimatic_mode == ULTIMATIC_DIT_PRIORITY) {
         dah_buffer = 0;
       } else {
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
  } else {
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
  #ifdef FEATURE_MEMORIES
  EEPROM.write(EEPROM_memory_repeat_time_low,lowByte(memory_repeat_time));
  EEPROM.write(EEPROM_memory_repeat_time_high,highByte(memory_repeat_time));  
  #endif //FEATURE_MEMORIES
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
    #ifdef FEATURE_MEMORIES
    if (EEPROM.read(EEPROM_memory_repeat_time_high) != 255 ) {
      memory_repeat_time=word(EEPROM.read(EEPROM_memory_repeat_time_high),EEPROM.read(EEPROM_memory_repeat_time_low));
    }
    #endif //FEATURE_MEMORIES
    config_dirty = 0;
    return 0;
  } else {
    return 1;
  }

}

//-------------------------------------------------------------------------------------------------------

void check_dit_paddle()
{
  byte pin_value = 0;
  byte dit_paddle = 0;
  #ifdef OPTION_DIT_PADDLE_NO_SEND_ON_MEM_RPT
  static byte memory_rpt_interrupt_flag = 0;
  #endif

  if (paddle_mode == PADDLE_NORMAL) {
    dit_paddle = paddle_left;
  } else {
    dit_paddle = paddle_right;
  }
  pin_value = digitalRead(dit_paddle);
  
  #ifdef OPTION_DIT_PADDLE_NO_SEND_ON_MEM_RPT
  if (pin_value && memory_rpt_interrupt_flag) {
    memory_rpt_interrupt_flag = 0;
    loop_element_lengths(3,0,wpm,MANUAL_SENDING);
    dit_buffer = 0;
  }
  #endif
  
  #ifdef OPTION_DIT_PADDLE_NO_SEND_ON_MEM_RPT
  if ((pin_value == 0) && (memory_rpt_interrupt_flag == 0)) {
  #else
  if (pin_value == 0) {
  #endif
    #ifdef FEATURE_DEAD_OP_WATCHDOG
    if (dit_buffer == 0) {
      dit_counter++;
      dah_counter = 0;
    }
    #endif
    dit_buffer = 1;
    #ifdef FEATURE_MEMORIES
    if (repeat_memory < 255) {
      repeat_memory = 255;
      send_buffer_bytes = 0;
      #ifdef OPTION_DIT_PADDLE_NO_SEND_ON_MEM_RPT
      dit_buffer = 0;
      while (!digitalRead(dit_paddle)) {};
      memory_rpt_interrupt_flag = 1;
      #endif
    }
    #endif
  }

}

//-------------------------------------------------------------------------------------------------------

void check_dah_paddle()
{
  byte pin_value = 0;
  byte dah_paddle;

  if (paddle_mode == PADDLE_NORMAL) {
    dah_paddle = paddle_right;
  } else {
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
    #ifdef FEATURE_MEMORIES
    repeat_memory = 255;
    #endif
  }
}

//-------------------------------------------------------------------------------------------------------

void send_dit(byte sending_type)
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

//  if (keyer_mode == IAMBIC_A) {
//    dit_buffer = 0;
//    dah_buffer = 0;
//  }
  loop_element_lengths((2.0-(float(weighting)/50)),(-1.0*keying_compensation),character_wpm,sending_type);
  #ifdef FEATURE_AUTOSPACE
  if ((sending_type == MANUAL_SENDING) && (autospace_active)) {
    check_paddles();
  }
  if ((sending_type == MANUAL_SENDING) && (autospace_active) && (dit_buffer == 0) && (dah_buffer == 0)) {
    loop_element_lengths(2,0,wpm,sending_type);
  }
  #endif

  #ifdef FEATURE_WINKEY_EMULATION
  if ((winkey_host_open) && (winkey_paddle_echo_activated) && (sending_type == MANUAL_SENDING)) {
    winkey_paddle_echo_buffer = (winkey_paddle_echo_buffer * 10) + 1;
    winkey_paddle_echo_buffer_decode_time = millis() + (float(600/wpm)*length_letterspace);
  }
  #endif

  #ifdef FEATURE_COMMAND_LINE_INTERFACE
  if ((cli_paddle_echo) && (sending_type == MANUAL_SENDING)) {
    cli_paddle_echo_buffer = (cli_paddle_echo_buffer * 10) + 1;
    cli_paddle_echo_buffer_decode_time = millis() + (float(600/wpm)*length_letterspace);
  }
  #endif

  #ifdef FEATURE_DISPLAY
  if ((lcd_paddle_echo) && (sending_type == MANUAL_SENDING)) {
    lcd_paddle_echo_buffer = (lcd_paddle_echo_buffer * 10) + 1;
    lcd_paddle_echo_buffer_decode_time = millis() + (float(600/wpm)*length_letterspace);
  }
  #endif
  
  being_sent = SENDING_NOTHING;
  last_sending_type = sending_type;
  
//  if ((keyer_mode == IAMBIC_A) && (iambic_flag)) {
//    iambic_flag = 0;
//    dit_buffer = 0;
//    dah_buffer = 0;
//  } 
  
  check_paddles();

}

//-------------------------------------------------------------------------------------------------------

void send_dah(byte sending_type)
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
//  if (keyer_mode == IAMBIC_A) {
//    dit_buffer = 0;
//    dah_buffer = 0;
//  }
  loop_element_lengths((4.0-(3.0*(float(weighting)/50))),(-1.0*keying_compensation),character_wpm,sending_type);
  #ifdef FEATURE_AUTOSPACE
  if ((sending_type == MANUAL_SENDING) && (autospace_active)) {
    check_paddles();
  }
  if ((sending_type == MANUAL_SENDING) && (autospace_active) && (dit_buffer == 0) && (dah_buffer == 0)) {
    loop_element_lengths(2,0,wpm,sending_type);
  }
  #endif

  #ifdef FEATURE_WINKEY_EMULATION
  if ((winkey_host_open) && (winkey_paddle_echo_activated) && (sending_type == MANUAL_SENDING)) {
    winkey_paddle_echo_buffer = (winkey_paddle_echo_buffer * 10) + 2;
    winkey_paddle_echo_buffer_decode_time = millis() + (float(600/wpm)*length_letterspace);
  }
  #endif

  #ifdef FEATURE_COMMAND_LINE_INTERFACE
  if ((cli_paddle_echo) && (sending_type == MANUAL_SENDING)) {
    cli_paddle_echo_buffer = (cli_paddle_echo_buffer * 10) + 2;
    cli_paddle_echo_buffer_decode_time = millis() + (float(600/wpm)*length_letterspace);
  }
  #endif

  #ifdef FEATURE_DISPLAY
  if ((lcd_paddle_echo) && (sending_type == MANUAL_SENDING)) {
    lcd_paddle_echo_buffer = (lcd_paddle_echo_buffer * 10) + 2;
    lcd_paddle_echo_buffer_decode_time = millis() + (float(600/wpm)*length_letterspace);
  }
  #endif

//  if ((keyer_mode == IAMBIC_A) && (iambic_flag)) {
//    iambic_flag = 0;
//    //dit_buffer = 0;
//    dah_buffer = 0;
//  }

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
  } else {
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
  } else {
    element_length = qrss_dit_length * 1000;
  }


  unsigned long endtime = millis() + long(element_length*lengths) + long(additional_time_ms);
  while ((millis() < endtime) && (millis() > 200)) {  // the second condition is to account for millis() rollover
    #ifdef OPTION_WATCHDOG_TIMER
    wdt_reset();
    #endif  //OPTION_WATCHDOG_TIMER
    if (keyer_mode == ULTIMATIC) {
    } else {
      if ((keyer_mode == IAMBIC_A) && (digitalRead(paddle_left) == LOW ) && (digitalRead(paddle_right) == LOW )) {
          iambic_flag = 1;
      }    
     
      if (being_sent == SENDING_DIT) {
        check_dah_paddle();
      } else {
        if (being_sent == SENDING_DAH) {
          check_dit_paddle();
        }
      }

    }
    #ifdef FEATURE_MEMORIES
    #ifdef FEATURE_COMMAND_BUTTONS
    check_the_memory_buttons();
    #endif
    #endif
    // blow out prematurely if we're automatic sending and a paddle gets hit
    #ifdef FEATURE_COMMAND_BUTTONS
    if (sending_type == AUTOMATIC_SENDING && (digitalRead(paddle_left) == LOW || digitalRead(paddle_right) == LOW || analogbuttonread(0) || dit_buffer || dah_buffer)) {
    #else
    if (sending_type == AUTOMATIC_SENDING && (digitalRead(paddle_left) == LOW || digitalRead(paddle_right) == LOW || dit_buffer || dah_buffer)) {
    #endif
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
  
  #ifdef FEATURE_DISPLAY
  lcd_center_print_timed(String(wpm) + " wpm", 1, default_display_msg_delay);
  #endif
}

//-------------------------------------------------------------------------------------------------------

void speed_set(int wpm_set)
{
    wpm = wpm_set;
    config_dirty = 1;
    #ifdef FEATURE_DISPLAY
    lcd_center_print_timed(String(wpm) + " wpm", 0, default_display_msg_delay);
    #endif
}

//-------------------------------------------------------------------------------------------------------

int get_cw_input_from_user(unsigned int exit_time_seconds) {

  byte looping = 1;
  byte paddle_hit = 0;
  int cw_char = 0;
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
      cw_char = (cw_char * 10) + 1;
      last_element_time = millis();
    }
    if (dah_buffer) {
      send_dah(MANUAL_SENDING);
      dah_buffer = 0;
      paddle_hit = 1;
      cw_char = (cw_char * 10) + 2;
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

    #ifdef FEATURE_COMMAND_BUTTONS
    while (analogbuttonread(0)) {    // hit the button to get out of command mode if no paddle was hit
      looping = 0;
      button_hit = 1;
    }
    #endif

    #ifdef FEATURE_SERIAL
    check_serial();
    #endif

  }
  if (button_hit) {
    #ifdef DEBUG_GET_CW_INPUT_FROM_USER
    Serial.println(F("get_cw_input_from_user: button_hit exit 9"));
    #endif
    return 9;
  } else {
    #ifdef DEBUG_GET_CW_INPUT_FROM_USER
    Serial.print(F("get_cw_input_from_user: exiting cw_char:"));
    Serial.println(cw_char);
    #endif    
    return cw_char;
  }
}

//-------------------------------------------------------------------------------------------------------

#ifdef FEATURE_COMMAND_BUTTONS
void command_mode ()
{

  machine_mode = COMMAND;
  
  #ifdef DEBUG_COMMAND_MODE
  Serial.println(F("command_mode: entering"));
  #endif
  
  int cw_char;
  byte stay_in_command_mode = 1;
  byte speed_mode_before = speed_mode;
  speed_mode = SPEED_NORMAL;                 // put us in normal speed mode (life is too short to do command mode in QRSS)
  byte keyer_mode_before = keyer_mode;
  if ((keyer_mode != IAMBIC_A) && (keyer_mode != IAMBIC_B)) {
    keyer_mode = IAMBIC_B;                   // we got to be in iambic mode (life is too short to make this work in bug mode)
  }

  command_mode_disable_tx = 0;

  boop_beep();
  #ifdef FEATURE_DISPLAY
  lcd.clear();
  lcd_center_print_timed("Command Mode", 0, default_display_msg_delay);
  #endif 

  while (stay_in_command_mode) {
    cw_char = 0;
    cw_char = get_cw_input_from_user(0);
    #ifdef DEBUG_COMMAND_MODE
    Serial.print(F("command_mode: cwchar: "));
    Serial.println(cw_char);
    #endif
    if (cw_char > 0) {              // do the command      
      switch (cw_char) {
        case 12: // A - Iambic mode
          keyer_mode = IAMBIC_A;
          keyer_mode_before = IAMBIC_A;
          config_dirty = 1;
          #ifdef FEATURE_DISPLAY
          lcd_center_print_timed("Iambic A", 0, default_display_msg_delay);
          #endif
          send_dit(AUTOMATIC_SENDING);
          break; 
        case 2111: // B - Iambic mode
          keyer_mode = IAMBIC_B;
          keyer_mode_before = IAMBIC_B;
          config_dirty = 1;
          #ifdef FEATURE_DISPLAY
          lcd_center_print_timed("Iambic B", 0, default_display_msg_delay);
          #endif          
          send_dit(AUTOMATIC_SENDING);
          break; 
        case 211: // D - Ultimatic mode
          keyer_mode = ULTIMATIC;
          keyer_mode_before = ULTIMATIC;
          config_dirty = 1;
          #ifdef FEATURE_DISPLAY
          lcd_center_print_timed("Ultimatic", 0, default_display_msg_delay);
          #endif                    
          send_dit(AUTOMATIC_SENDING);
          break; 
        case 1121: command_sidetone_freq_adj(); break;                    // F - adjust sidetone frequency
        case 221: // G - switch to buG mode
          keyer_mode = BUG;
          keyer_mode_before = BUG;
          config_dirty = 1;
          #ifdef FEATURE_DISPLAY
          lcd_center_print_timed("Bug", 0, default_display_msg_delay);
          #endif          
          send_dit(AUTOMATIC_SENDING);
          break;  
        case 11:                                                     // I - toggle TX enable / disable
          if (command_mode_disable_tx) {
            command_mode_disable_tx = 0;
            #ifdef FEATURE_DISPLAY
            lcd_center_print_timed("TX On", 0, default_display_msg_delay);
            #endif            
          } else {
            command_mode_disable_tx = 1;
            #ifdef FEATURE_DISPLAY
            lcd_center_print_timed("TX Off", 0, default_display_msg_delay);
            #endif            
          }
          send_dit(AUTOMATIC_SENDING);
          break;
        case 1222: command_dah_to_dit_ratio_adjust(); break;                        // J - dah to dit ratio adjust
        #ifdef FEATURE_MEMORIES
        case 1221: command_program_memory(); break;                       // P - program a memory
        case 21: // N - paddle mode toggle
          if (paddle_mode == PADDLE_NORMAL) {
            paddle_mode = PADDLE_REVERSE;
            #ifdef FEATURE_DISPLAY
            lcd_center_print_timed("Paddle Reverse", 0, default_display_msg_delay);
            #endif 
          } else {
            #ifdef FEATURE_DISPLAY
            lcd_center_print_timed("Paddle Normal", 0, default_display_msg_delay);
            #endif             
            paddle_mode = PADDLE_NORMAL;
          }
          config_dirty = 1;
          send_dit(AUTOMATIC_SENDING);
          break;  
        #endif
        case 222: // O - toggle sidetone on and off
          if ((sidetone_mode == SIDETONE_ON) || (sidetone_mode == SIDETONE_PADDLE_ONLY)) {
            #ifdef FEATURE_DISPLAY
            lcd_center_print_timed("Sidetone Off", 0, default_display_msg_delay);
            #endif 
            sidetone_mode = SIDETONE_OFF;
           } else {
             #ifdef FEATURE_DISPLAY
             lcd_center_print_timed("Sidetone On", 0, default_display_msg_delay);
             #endif 
             sidetone_mode = SIDETONE_ON;
           }
           config_dirty = 1;
           send_dit(AUTOMATIC_SENDING);
           break; 
        case 2: command_tuning_mode(); break;                             // T - tuning mode
        case 122: command_speed_mode(); break;                            // W - change wpm
        #ifdef FEATURE_MEMORIES
        case 2122: command_set_mem_repeat_delay(); break; // Y - set memory repeat delay
        #endif
        case 2112: stay_in_command_mode = 0; break;                       // X - exit command mode
        #ifdef FEATURE_AUTOSPACE
        case 2211: // Z - Autospace
          if (autospace_active) {
            autospace_active = 0;
            config_dirty = 1;
            #ifdef FEATURE_DISPLAY
            lcd_center_print_timed("Autospace Off", 0, default_display_msg_delay);
            send_dit(AUTOMATIC_SENDING);
            #else
            send_char('O',NORMAL);
            send_char('F',NORMAL);
            send_char('F',NORMAL);
            #endif
          } else {
            autospace_active = 1;
            config_dirty = 1;
            #ifdef FEATURE_DISPLAY
            lcd_center_print_timed("Autospace On", 0, default_display_msg_delay);
            send_dit(AUTOMATIC_SENDING);
            #else            
            send_char('O',NORMAL);
            send_char('N',NORMAL);
            #endif
          }
          break;
        #endif
        #ifdef FEATURE_MEMORIES
        case 12222: play_memory(0); break;
        case 11222: play_memory(1); break;
        case 11122: play_memory(2); break;
        case 11112: play_memory(3); break;
        case 11111: play_memory(4); break;
        #endif
        case 9: stay_in_command_mode = 0; break;                          // button was hit - exit
        default: // unknown command, send a ?
          #ifdef FEATURE_DISPLAY
          lcd_center_print_timed("Unknown command", 0, default_display_msg_delay);          
          #endif
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
#endif //FEATURE_COMMAND_BUTTONS

//-------------------------------------------------------------------------------------------------------

#ifdef FEATURE_MEMORIES
void command_set_mem_repeat_delay() {
 
 //ddddd
  
  byte character_count = 0;;
  int cw_char = 0;
  byte number_sent = 0;
  unsigned int repeat_value = 0;
  byte error_flag = 0;
  
  for (character_count = 0; character_count < 4; character_count++) {
    cw_char = get_cw_input_from_user(0);
    number_sent = (convert_cw_number_to_ascii(cw_char) - 48);
    if ((number_sent > -1) && (number_sent < 10)) {
      repeat_value = (repeat_value * 10) + number_sent;
    } else { // we got a bad value
      error_flag = 1;
      character_count = 5;
    }      
  }
  
  if (error_flag) {
    boop();
  } else {
    memory_repeat_time = repeat_value;
    config_dirty = 1;
    beep();
  }
  
}
#endif //FEATURE_MEMORIES

//-------------------------------------------------------------------------------------------------------

void adjust_dah_to_dit_ratio(int adjustment) {

 if ((dah_to_dit_ratio + adjustment) > 150 && (dah_to_dit_ratio + adjustment) < 810) {
   dah_to_dit_ratio = dah_to_dit_ratio + adjustment;
   #ifdef FEATURE_DISPLAY
   #ifdef OPTION_MORE_DISPLAY_MSGS
   lcd_center_print_timed("Dah/Dit: " + String(dah_to_dit_ratio), 0, default_display_msg_delay);
   service_display();
   #endif
   #endif   
 }

 config_dirty = 1;
}

//-------------------------------------------------------------------------------------------------------

#ifdef FEATURE_COMMAND_BUTTONS
void command_dah_to_dit_ratio_adjust () {

  byte looping = 1;

  #ifdef FEATURE_DISPLAY
  lcd_center_print_timed("Adj dah to dit", 0, default_display_msg_delay);          
  #endif

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
  while (digitalRead(paddle_left) == LOW || digitalRead(paddle_right) == LOW || analogbuttonread(0) ) {}  // wait for all lines to go high
  dit_buffer = 0;
  dah_buffer = 0;
}
#endif //FEATURE_COMMAND_BUTTONS

//-------------------------------------------------------------------------------------------------------

#ifdef FEATURE_COMMAND_BUTTONS
void command_tuning_mode() {

  byte looping = 1;
  byte latched = 0;
  
  
  #ifdef FEATURE_DISPLAY
  lcd_center_print_timed("Tune Mode", 0, default_display_msg_delay);          
  #endif  
  
  send_dit(AUTOMATIC_SENDING);
  key_tx = 1;
  while (looping) {

    if (digitalRead(paddle_left) == LOW) {
      tx_and_sidetone_key(1,MANUAL_SENDING);
      latched = 0;
    } else {
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
    } else {
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
  while (digitalRead(paddle_left) == LOW || digitalRead(paddle_right) == LOW || analogbuttonread(0) ) {}  // wait for all lines to go high
  key_tx = 0;
  send_dit(AUTOMATIC_SENDING);
  dit_buffer = 0;
  dah_buffer = 0;
}
#endif //FEATURE_COMMAND_BUTTONS
//-------------------------------------------------------------------------------------------------------

void sidetone_adj(int hz) {

  if ((hz_sidetone + hz) > SIDETONE_HZ_LOW_LIMIT && (hz_sidetone + hz) < SIDETONE_HZ_HIGH_LIMIT) {
    hz_sidetone = hz_sidetone + hz;
    config_dirty = 1;
    #ifdef FEATURE_DISPLAY
    #ifdef OPTION_MORE_DISPLAY_MSGS
    lcd_center_print_timed("Sidetone " + String(hz_sidetone) + " Hz", 0, default_display_msg_delay);
    #endif
    #endif   
  }

}

//-------------------------------------------------------------------------------------------------------
#ifdef FEATURE_COMMAND_BUTTONS
void command_sidetone_freq_adj() {

  byte looping = 1;

  #ifdef FEATURE_DISPLAY
  lcd_center_print_timed("Sidetone " + String(hz_sidetone) + " Hz", 0, default_display_msg_delay);   
  #endif

  while (looping) {
    tone(sidetone_line, hz_sidetone);
    if (digitalRead(paddle_left) == LOW) {
      #ifdef FEATURE_DISPLAY
      sidetone_adj(5);      
      lcd_center_print_timed("Sidetone " + String(hz_sidetone) + " Hz", 0, default_display_msg_delay);        
      #else
      sidetone_adj(1);
      #endif
      delay(10);
    }
    if (digitalRead(paddle_right) == LOW) {
      #ifdef FEATURE_DISPLAY
      sidetone_adj(-5);
      lcd_center_print_timed("Sidetone " + String(hz_sidetone) + " Hz", 0, default_display_msg_delay);       
      #else
      sidetone_adj(-1);
      #endif
      delay(10);
    }
    while ((digitalRead(paddle_left) == LOW && digitalRead(paddle_right) == LOW) || (analogbuttonread(0))) { // if paddles are squeezed or button0 pressed - exit
      looping = 0;
    }
    

  }
  while (digitalRead(paddle_left) == LOW || digitalRead(paddle_right) == LOW || analogbuttonread(0) ) {}  // wait for all lines to go high
  noTone(sidetone_line);

}
#endif //FEATURE_COMMAND_BUTTONS
//-------------------------------------------------------------------------------------------------------
#ifdef FEATURE_COMMAND_BUTTONS
void command_speed_mode()
{

  byte looping = 1;
  String wpm_string;
  
  #ifdef FEATURE_DISPLAY
  lcd_center_print_timed("Adjust Speed", 0, default_display_msg_delay);        
  #endif
  

  while (looping) {
    send_dit(AUTOMATIC_SENDING);
    if ((digitalRead(paddle_left) == LOW)) {
      speed_change(1);
    }
    if ((digitalRead(paddle_right) == LOW)) {
      speed_change(-1);
    }
    while ((digitalRead(paddle_left) == LOW && digitalRead(paddle_right) == LOW) || (analogbuttonread(0) ))  // if paddles are squeezed or button0 pressed - exit
    {
      looping = 0;
    }

  }
  while (digitalRead(paddle_left) == LOW || digitalRead(paddle_right) == LOW || analogbuttonread(0) ) {}  // wait for all lines to go high
  #ifndef FEATURE_DISPLAY
  // announce speed in CW
  wpm_string = String(wpm, DEC);
  send_char(wpm_string[0],NORMAL);
  send_char(wpm_string[1],NORMAL);
  #endif

  dit_buffer = 0;
  dah_buffer = 0;

}
#endif //FEATURE_COMMAND_BUTTONS
//------------------------------------------------------------------

#ifdef FEATURE_MEMORIES
#ifdef FEATURE_COMMAND_BUTTONS
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
#endif

//------------------------------------------------------------------

#ifdef FEATURE_COMMAND_BUTTONS
#ifdef FEATURE_DL2SBA_BANKSWITCH
void setOneButton(int button, int index) { 
    int button_value = int(1023 * (float(button * analog_buttons_r2)/float((button * analog_buttons_r2) + analog_buttons_r1))); 
    int lower_button_value = int(1023 * (float((button-1) * analog_buttons_r2)/float(((button-1) * analog_buttons_r2) + analog_buttons_r1))); 
    int higher_button_value = int(1023 * (float((button+1) * analog_buttons_r2)/float(((button+1) * analog_buttons_r2) + analog_buttons_r1))); 
    button_array_low_limit[index] = (button_value - ((button_value - lower_button_value)/2)); 
    button_array_high_limit[index] = (button_value + ((higher_button_value - button_value)/2)); 
}
#endif
#endif

//------------------------------------------------------------------
#ifdef FEATURE_COMMAND_BUTTONS
void initialize_analog_button_array() {

  
  #ifndef FEATURE_DL2SBA_BANKSWITCH
  
  int button_value;
  int lower_button_value;
  int higher_button_value;

  #ifdef OPTION_REVERSE_BUTTON_ORDER
  byte y = analog_buttons_number_of_buttons - 1;
  #endif

  for (int x = 0;x < analog_buttons_number_of_buttons;x++) {
    button_value = int(1023 * (float(x * analog_buttons_r2)/float((x * analog_buttons_r2) + analog_buttons_r1)));
    lower_button_value = int(1023 * (float((x-1) * analog_buttons_r2)/float(((x-1) * analog_buttons_r2) + analog_buttons_r1)));
    higher_button_value = int(1023 * (float((x+1) * analog_buttons_r2)/float(((x+1) * analog_buttons_r2) + analog_buttons_r1)));
    #ifndef OPTION_REVERSE_BUTTON_ORDER
    button_array_low_limit[x] = (button_value - ((button_value - lower_button_value)/2));
    button_array_high_limit[x] = (button_value + ((higher_button_value - button_value)/2));
    #else
    button_array_low_limit[y] = (button_value - ((button_value - lower_button_value)/2));
    button_array_high_limit[y] = (button_value + ((higher_button_value - button_value)/2));
    y--;
    #endif
  }
  
  #else //FEATURE_DL2SBA_BANKSWITCH
  
  setOneButton(0,0); 
  setOneButton(1,3); 
  setOneButton(2,2); 
  setOneButton(3,1); 
  setOneButton(4,9); 
  setOneButton(5,8); 
  setOneButton(6,7); 
  setOneButton(7,6); 
  setOneButton(8,5); 
  setOneButton(9,4); 
      
  #endif //FEATURE_DL2SBA_BANKSWITCH

}
#endif

//------------------------------------------------------------------

#ifdef FEATURE_COMMAND_BUTTONS
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
  
#endif

//------------------------------------------------------------------
#ifdef FEATURE_COMMAND_BUTTONS
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
#endif

//------------------------------------------------------------------

#ifdef FEATURE_COMMAND_BUTTONS
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
    #ifdef FEATURE_MEMORIES
    repeat_memory = 255;
    #endif
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
        } else {
          key_tx = 1;
        }
      }
      #ifdef FEATURE_MEMORIES
      if ((analogbuttontemp > 0) && (analogbuttontemp < (number_of_memories + 1)) && ((millis() - button_last_add_to_send_buffer_time) > 400)) {
        #ifndef OPTION_WINKEY_2_SUPPORT
        add_to_send_buffer(SERIAL_SEND_BUFFER_MEMORY_NUMBER);
        add_to_send_buffer(analogbuttontemp - 1);
        #else //OPTION_WINKEY_2_SUPPORT
        if ((winkey_host_open) && (wk2_mode == 2)) {   // if winkey is open and in wk2 mode, tell it about the button press
          byte winkey_byte_to_send = 0xc8;
          switch(analogbuttontemp) {
            case 1: winkey_byte_to_send = winkey_byte_to_send | 1; break;
            case 2: winkey_byte_to_send = winkey_byte_to_send | 2; break;
            case 3: winkey_byte_to_send = winkey_byte_to_send | 4; break;
            case 4: winkey_byte_to_send = winkey_byte_to_send | 16; break;            
          } 
          Serial.write(winkey_byte_to_send);
          Serial.write(0xc8); // tell it that the button is unpressed
        } else {  // otherwise, have the buttons act as normal
          add_to_send_buffer(SERIAL_SEND_BUFFER_MEMORY_NUMBER);
          add_to_send_buffer(analogbuttontemp - 1);
        }  
        #endif //OPTION_WINKEY_2_SUPPORT
        button_last_add_to_send_buffer_time = millis();
        #ifdef DEBUG_BUTTONS
        Serial.print(F("\ncheck_buttons: add_to_send_buffer: "));
        Serial.println(analogbuttontemp - 1);
        #endif //DEBUG_BUTTONS
      }
      #endif
    } else {
        if (analogbuttontemp == 0) {
          key_tx = 0;
          // do stuff if this is a command button hold down
          while (analogbuttonpressed() == 0) {
            if (digitalRead(paddle_left) == LOW) {                     // left paddle increase speed
              speed_change(1);
              previous_sidetone_mode = sidetone_mode;
              sidetone_mode = SIDETONE_ON; 
              send_dit(MANUAL_SENDING);
              sidetone_mode = previous_sidetone_mode;
              //speed_button_cmd_executed = 1;
              dit_buffer = 0;
              
              #ifdef DEBUG_BUTTONS
              Serial.println(F("\ncheck_buttons: speed_change(1)"));
              #endif //DEBUG_BUTTONS            

              #ifdef FEATURE_WINKEY_EMULATION
              if ((serial_mode == SERIAL_WINKEY_EMULATION) && (winkey_host_open)) {
                Serial.write(((wpm-pot_wpm_low_value)|128));
                winkey_last_unbuffered_speed_wpm = wpm;
              }
              #endif

            }
            if (digitalRead(paddle_right) == LOW) {                    // right paddle decreases speed
              speed_change(-1);
              previous_sidetone_mode = sidetone_mode;
              sidetone_mode = SIDETONE_ON; 
              send_dah(MANUAL_SENDING);
              sidetone_mode = previous_sidetone_mode;              
              //speed_button_cmd_executed = 1;
              dah_buffer = 0;

              #ifdef DEBUG_BUTTONS
              Serial.println(F("\ncheck_buttons: speed_change(-1)"));
              #endif //DEBUG_BUTTONS            

              #ifdef FEATURE_WINKEY_EMULATION
              if ((serial_mode == SERIAL_WINKEY_EMULATION) && (winkey_host_open)) {
                Serial.write(((wpm-pot_wpm_low_value)|128));
                winkey_last_unbuffered_speed_wpm = wpm;
              }
              #endif
            }
         }
         key_tx = 1;
       }  //(analogbuttontemp == 0)
       if ((analogbuttontemp > 0) && (analogbuttontemp < analog_buttons_number_of_buttons)) {
         while (analogbuttonpressed() == analogbuttontemp) {
            if (((digitalRead(paddle_left) == LOW) || (digitalRead(paddle_right) == LOW)) && (analogbuttontemp < (number_of_memories + 1))){
              #ifdef FEATURE_MEMORIES
              repeat_memory = analogbuttontemp - 1;
              last_memory_repeat_time = 0;
              #endif
            }
         }
       }
    }
    last_button_action = millis();
  }
}
#endif //FEATURE_COMMAND_BUTTONS

//-------------------------------------------------------------------------------------------------------

void service_dit_dah_buffers()
{
  #ifdef DEBUG_LOOP
  Serial.println(F("loop: entering service_dit_dah_buffers"));
  #endif      
      
  if ((keyer_mode == IAMBIC_A) || (keyer_mode == IAMBIC_B) || (keyer_mode == ULTIMATIC)) {
    if ((keyer_mode == IAMBIC_A) && (iambic_flag) && (digitalRead(paddle_left)) && (digitalRead(paddle_right))) {
      iambic_flag = 0;
      dit_buffer = 0;
      dah_buffer = 0;
    } else {
      if (dit_buffer) {
        dit_buffer = 0;
        send_dit(MANUAL_SENDING);
      }
      if (dah_buffer) {
        dah_buffer = 0;
        send_dah(MANUAL_SENDING);
      }
    }
  } else {
    if (keyer_mode == BUG) {
      if (dit_buffer) {
        dit_buffer = 0;
        send_dit(MANUAL_SENDING);
      }
      if (dah_buffer) {
        dah_buffer = 0;
        tx_and_sidetone_key(1,MANUAL_SENDING);
      } else {
        tx_and_sidetone_key(0,MANUAL_SENDING);
      }
      #ifdef FEATURE_DEAD_OP_WATCHDOG
      dah_counter = 0;
      #endif
    } else {
      if (keyer_mode == STRAIGHT) {
        if (dit_buffer) {
          dit_buffer = 0;
          tx_and_sidetone_key(1,MANUAL_SENDING);
        } else {
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

void send_char(char cw_char, byte omit_letterspace)
{
  #ifdef DEBUG_SEND_CHAR
  Serial.print(F("send_char: called with cw_char:"));
  Serial.print(cw_char);
  if (omit_letterspace) {
    Serial.print(F(" OMIT_LETTERSPACE"));
  }
  Serial.println();
  #endif

  if ((cw_char == 10) || (cw_char == 13)) { return; }  // don't attempt to send carriage return or line feed
  switch (cw_char) {
    case 'A': send_dit(AUTOMATIC_SENDING); send_dah(AUTOMATIC_SENDING); break;
    case 'B': send_dah(AUTOMATIC_SENDING); send_dits(3); break;
    case 'C': send_dah(AUTOMATIC_SENDING); send_dit(AUTOMATIC_SENDING); send_dah(AUTOMATIC_SENDING); send_dit(AUTOMATIC_SENDING); break;
    case 'D': send_dah(AUTOMATIC_SENDING); send_dits(2); break;
    case 'E': send_dit(AUTOMATIC_SENDING); break;
    case 'F': send_dits(2); send_dah(AUTOMATIC_SENDING); send_dit(AUTOMATIC_SENDING); break;
    case 'G': send_dahs(2); send_dit(AUTOMATIC_SENDING); break;
    case 'H': send_dits(4); break;
    case 'I': send_dits(2); break;
    case 'J': send_dit(AUTOMATIC_SENDING); send_dahs(3); break;
    case 'K': send_dah(AUTOMATIC_SENDING); send_dit(AUTOMATIC_SENDING); send_dah(AUTOMATIC_SENDING); break;
    case 'L': send_dit(AUTOMATIC_SENDING); send_dah(AUTOMATIC_SENDING); send_dits(2); break;
    case 'M': send_dahs(2); break;
    case 'N': send_dah(AUTOMATIC_SENDING); send_dit(AUTOMATIC_SENDING); break;
    case 'O': send_dahs(3); break;
    case 'P': send_dit(AUTOMATIC_SENDING); send_dahs(2); send_dit(AUTOMATIC_SENDING); break;
    case 'Q': send_dahs(2); send_dit(AUTOMATIC_SENDING); send_dah(AUTOMATIC_SENDING); break;
    case 'R': send_dit(AUTOMATIC_SENDING); send_dah(AUTOMATIC_SENDING); send_dit(AUTOMATIC_SENDING); break;
    case 'S': send_dits(3); break;
    case 'T': send_dah(AUTOMATIC_SENDING); break;
    case 'U': send_dits(2); send_dah(AUTOMATIC_SENDING); break;
    case 'V': send_dits(3); send_dah(AUTOMATIC_SENDING); break;
    case 'W': send_dit(AUTOMATIC_SENDING); send_dahs(2); break;
    case 'X': send_dah(AUTOMATIC_SENDING); send_dits(2); send_dah(AUTOMATIC_SENDING); break;
    case 'Y': send_dah(AUTOMATIC_SENDING); send_dit(AUTOMATIC_SENDING); send_dahs(2); break;
    case 'Z': send_dahs(2); send_dits(2); break;

    case '0': send_dahs(5); break;
    case '1': send_dit(AUTOMATIC_SENDING); send_dahs(4); break;
    case '2': send_dits(2); send_dahs(3); break;
    case '3': send_dits(3); send_dahs(2); break;
    case '4': send_dits(4); send_dah(AUTOMATIC_SENDING); break;
    case '5': send_dits(5); break;
    case '6': send_dah(AUTOMATIC_SENDING); send_dits(4); break;
    case '7': send_dahs(2); send_dits(3); break;
    case '8': send_dahs(3); send_dits(2); break;
    case '9': send_dahs(4); send_dit(AUTOMATIC_SENDING); break;

    case '=': send_dah(AUTOMATIC_SENDING); send_dits(3); send_dah(AUTOMATIC_SENDING); break;
    case '/': send_dah(AUTOMATIC_SENDING); send_dits(2); send_dah(AUTOMATIC_SENDING); send_dit(AUTOMATIC_SENDING); break;
    case ' ': loop_element_lengths((length_wordspace-length_letterspace-2),0,wpm,AUTOMATIC_SENDING); break;
    case '*': send_dah(AUTOMATIC_SENDING); send_dits(3); send_dah(AUTOMATIC_SENDING); send_dit(AUTOMATIC_SENDING); send_dah(AUTOMATIC_SENDING); break;    // using asterisk for BK
    //case '&': send_dit(AUTOMATIC_SENDING); loop_element_lengths(3); send_dits(3); break;
    case '.': send_dit(AUTOMATIC_SENDING); send_dah(AUTOMATIC_SENDING); send_dit(AUTOMATIC_SENDING); send_dah(AUTOMATIC_SENDING); send_dit(AUTOMATIC_SENDING); send_dah(AUTOMATIC_SENDING); break;
    case ',': send_dahs(2); send_dits(2); send_dahs(2); break;
    case '\'': send_dit(AUTOMATIC_SENDING); send_dahs(4); send_dit(AUTOMATIC_SENDING); break;                   // apostrophe
    case '!': send_dah(AUTOMATIC_SENDING); send_dit(AUTOMATIC_SENDING); send_dah(AUTOMATIC_SENDING); send_dit(AUTOMATIC_SENDING); send_dahs(2); break;
    case '(': send_dah(AUTOMATIC_SENDING); send_dit(AUTOMATIC_SENDING); send_dahs(2); send_dit(AUTOMATIC_SENDING); break;
    case ')': send_dah(AUTOMATIC_SENDING); send_dit(AUTOMATIC_SENDING); send_dahs(2); send_dit(AUTOMATIC_SENDING); send_dah(AUTOMATIC_SENDING); break;
    case '&': send_dit(AUTOMATIC_SENDING); send_dah(AUTOMATIC_SENDING); send_dits(3); break;
    case ':': send_dahs(3); send_dits(3); break;
    case ';': send_dah(AUTOMATIC_SENDING); send_dit(AUTOMATIC_SENDING); send_dah(AUTOMATIC_SENDING); send_dit(AUTOMATIC_SENDING); send_dah(AUTOMATIC_SENDING); send_dit(AUTOMATIC_SENDING); break;
    case '+': send_dit(AUTOMATIC_SENDING); send_dah(AUTOMATIC_SENDING); send_dit(AUTOMATIC_SENDING); send_dah(AUTOMATIC_SENDING); send_dit(AUTOMATIC_SENDING); break;
    case '-': send_dah(AUTOMATIC_SENDING); send_dits(4); send_dah(AUTOMATIC_SENDING); break;
    case '_': send_dits(2); send_dahs(2); send_dit(AUTOMATIC_SENDING); send_dah(AUTOMATIC_SENDING); break;
    case '"': send_dit(AUTOMATIC_SENDING); send_dah(AUTOMATIC_SENDING); send_dits(2); send_dah(AUTOMATIC_SENDING); send_dit(AUTOMATIC_SENDING); break;
    case '$': send_dits(3); send_dah(AUTOMATIC_SENDING); send_dits(2); send_dah(AUTOMATIC_SENDING); break;
    case '@': send_dit(AUTOMATIC_SENDING); send_dahs(2); send_dit(AUTOMATIC_SENDING); send_dah(AUTOMATIC_SENDING); send_dit(AUTOMATIC_SENDING); break;
    case '<': send_dit(AUTOMATIC_SENDING); send_dah(AUTOMATIC_SENDING); send_dit(AUTOMATIC_SENDING); send_dah(AUTOMATIC_SENDING); send_dit(AUTOMATIC_SENDING); break;     // AR
    case '>': send_dits(3); send_dah(AUTOMATIC_SENDING); send_dit(AUTOMATIC_SENDING); send_dah(AUTOMATIC_SENDING); break;               // SK
    case '\n': break;
    case '\r': break;
      
    #ifdef OPTION_NON_ENGLISH_EXTENSIONS
    case 192: send_dit(AUTOMATIC_SENDING);send_dahs(2);send_dit(AUTOMATIC_SENDING);send_dah(AUTOMATIC_SENDING); break; // 'À'
    case 194: send_dit(AUTOMATIC_SENDING);send_dah(AUTOMATIC_SENDING); send_dit(AUTOMATIC_SENDING); send_dah(AUTOMATIC_SENDING); break; // 'Â'
    case 197: send_dit(AUTOMATIC_SENDING);send_dahs(2);send_dit(AUTOMATIC_SENDING);send_dah(AUTOMATIC_SENDING); break; // 'Å' / 
    case 196: send_dit(AUTOMATIC_SENDING);send_dah(AUTOMATIC_SENDING); send_dit(AUTOMATIC_SENDING); send_dah(AUTOMATIC_SENDING); break; // 'Ä'
    case 198: send_dit(AUTOMATIC_SENDING);send_dah(AUTOMATIC_SENDING); send_dit(AUTOMATIC_SENDING); send_dah(AUTOMATIC_SENDING); break; // 'Æ'
    case 199: send_dah(AUTOMATIC_SENDING);send_dit(AUTOMATIC_SENDING); send_dah(AUTOMATIC_SENDING); send_dits(2); break;  // 'Ç'
    case 208: send_dits(2);send_dahs(2);send_dit(AUTOMATIC_SENDING);break;  // 'Ð'
    case 138: send_dahs(4);break; // 'Š'
    case 200: send_dit(AUTOMATIC_SENDING);send_dah(AUTOMATIC_SENDING);send_dits(2);send_dah(AUTOMATIC_SENDING); break; // 'È'
    case 201: send_dits(2);send_dah(AUTOMATIC_SENDING);send_dits(2);break; // 'É'
    case 142: send_dahs(2);send_dits(2);send_dah(AUTOMATIC_SENDING);send_dit(AUTOMATIC_SENDING);break; // 'Ž'
    case 209: send_dahs(2);send_dit(AUTOMATIC_SENDING);send_dahs(2);break; // 'Ñ'
    case 214: send_dahs(3);send_dit(AUTOMATIC_SENDING);break; // 'Ö'
    case 216: send_dahs(3);send_dit(AUTOMATIC_SENDING);break; // 'Ø'
    case 211: send_dahs(3);send_dit(AUTOMATIC_SENDING);break; // 'Ó'
    case 220: send_dits(2);send_dahs(2);break; // 'Ü'
    case 223: send_dits(6);break; // 'ß'
    #endif //OPTION_NON_ENGLISH_EXTENSIONS      
    
    case '|': loop_element_lengths(0.5,0,wpm,AUTOMATIC_SENDING); return; break;
    default: send_dits(2); send_dahs(2); send_dits(2); break; 
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
  if (charbytein == 158) { charbytein = 142; }  // ž -> Ž
  if (charbytein == 154) { charbytein = 138; }  // š -> Š
  
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
    } else {

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
      } else {
        if (incoming_serial_byte == 13) {   // carriage return - get out
          looping = 0;
        } else {                 // bogus input - error out
          looping = 0;
          error = 1;
        }
      }
    }
  }

  if (error) {
    Serial.println(F("Error..."));
    while (Serial.available() > 0) { incoming_serial_byte = Serial.read(); }  // clear out buffer
    return;
  } else {
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
  // send one character out of the send buffer
  // values 200 and above do special things
  // 200 - SERIAL_SEND_BUFFER_WPM_CHANGE - next two bytes are new speed
  // 203 - SERIAL_SEND_BUFFER_TIMED_KEY_DOWN
  // 204 - SERIAL_SEND_BUFFER_TIMED_WAIT
  // 205 - SERIAL_SEND_BUFFER_NULL
  // 206 - SERIAL_SEND_BUFFER_PROSIGN
  // 207 - SERIAL_SEND_BUFFER_HOLD_SEND
  // 208 - SERIAL_SEND_BUFFER_HOLD_SEND_RELEASE
  // 210 - SERIAL_SEND_BUFFER_MEMORY_NUMBER - next byte is memory number to play

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
          #ifdef FEATURE_WINKEY_EMULATION
          if (winkey_sending && winkey_host_open) {
            Serial.write(0xc2);
            winkey_interrupted = 1;
           }
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
      } else {
        #ifdef FEATURE_WINKEY_EMULATION
        if (((serial_mode == SERIAL_WINKEY_EMULATION) && (winkey_serial_echo) && (winkey_host_open)) || (serial_mode != SERIAL_WINKEY_EMULATION)) {
        #endif //FEATURE_WINKEY_EMULATION
        #ifdef FEATURE_SERIAL
        Serial.write(send_buffer_array[0]);
        if (send_buffer_array[0] == 13) {
          Serial.write(10);  // if we got a carriage return, also send a line feed
        }
        #endif //FEATURE_SERIAL
        #ifdef FEATURE_WINKEY_EMULATION
        }
        #endif //FEATURE_WINKEY_EMULATION
        #ifdef FEATURE_DISPLAY
        if (lcd_send_echo) {
          display_scroll_print_char(send_buffer_array[0]);
          service_display();
        }
        #endif //FEATURE_DISPLAY
        send_char(send_buffer_array[0],NORMAL);
        remove_from_send_buffer();
      }
    }

  } else {

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
      } else {
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
    #ifdef FEATURE_MEMORIES
    repeat_memory = 255;
    #endif
    #ifdef FEATURE_WINKEY_EMULATION
    if (winkey_sending && winkey_host_open) {
      Serial.write(0xc2);
      winkey_interrupted = 1;
    }
    #endif
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
      } else {  // we got a backspace
        send_buffer_bytes--;
      }
    }
//  }
}

//-------------------------------------------------------------------------------------------------------

#ifdef FEATURE_WINKEY_EMULATION
void winkey_unbuffered_speed_command(byte incoming_serial_byte) {

  if (incoming_serial_byte != 0) {
    wpm = incoming_serial_byte;
    winkey_last_unbuffered_speed_wpm = wpm;
    //calculate_element_length();
    #ifdef OPTION_WINKEY_STRICT_EEPROM_WRITES_MAY_WEAR_OUT_EEPROM
    config_dirty = 1;
    #endif
  }

}
#endif //FEATURE_WINKEY_EMULATION

//-------------------------------------------------------------------------------------------------------
#ifdef FEATURE_WINKEY_EMULATION
void winkey_farnsworth_command(byte incoming_serial_byte) {

  #ifdef FEATURE_FARNSWORTH
  if ((incoming_serial_byte > 9) && (incoming_serial_byte < 100)) {
    wpm_farnsworth = incoming_serial_byte;
  }
  #endif //FEATURE_FFARNSWORTH

}
#endif //FEATURE_WINKEY_EMULATION

//-------------------------------------------------------------------------------------------------------
#ifdef FEATURE_WINKEY_EMULATION
void winkey_keying_compensation_command(byte incoming_serial_byte) {

  keying_compensation = incoming_serial_byte;
}
#endif //FEATURE_WINKEY_EMULATION

//-------------------------------------------------------------------------------------------------------
#ifdef FEATURE_WINKEY_EMULATION
void winkey_first_extension_command(byte incoming_serial_byte) {

  first_extension_time = incoming_serial_byte;
  #ifdef DEBUG_WINKEY_PROTOCOL
  send_char('X',NORMAL);
  #endif
}
#endif //FEATURE_WINKEY_EMULATION

//-------------------------------------------------------------------------------------------------------
#ifdef FEATURE_WINKEY_EMULATION
void winkey_dah_to_dit_ratio_command(byte incoming_serial_byte) {

  if ((incoming_serial_byte > 32) && (incoming_serial_byte < 67)) {
    dah_to_dit_ratio = (300*(float(incoming_serial_byte)/50));
    #ifdef OPTION_WINKEY_STRICT_EEPROM_WRITES_MAY_WEAR_OUT_EEPROM
    config_dirty = 1;
    #endif
  }

}
#endif //FEATURE_WINKEY_EMULATION

//-------------------------------------------------------------------------------------------------------
#ifdef FEATURE_WINKEY_EMULATION
void winkey_weighting_command(byte incoming_serial_byte) {

  if ((incoming_serial_byte > 9) && (incoming_serial_byte < 91)) {
    weighting = incoming_serial_byte;
  }

}
#endif //FEATURE_WINKEY_EMULATION

//-------------------------------------------------------------------------------------------------------
#ifdef FEATURE_WINKEY_EMULATION
void winkey_set_pot_parm1_command(byte incoming_serial_byte) {

  //#ifdef FEATURE_POTENTIOMETER
  pot_wpm_low_value = incoming_serial_byte;
  //#endif
}
#endif //FEATURE_WINKEY_EMULATION
//-------------------------------------------------------------------------------------------------------
#ifdef FEATURE_WINKEY_EMULATION
void winkey_set_pot_parm2_command(byte incoming_serial_byte) {
  pot_wpm_high_value = (pot_wpm_low_value + incoming_serial_byte);
}
#endif //FEATURE_WINKEY_EMULATION
//-------------------------------------------------------------------------------------------------------
#ifdef FEATURE_WINKEY_EMULATION
void winkey_set_pot_parm3_command (byte incoming_serial_byte) {

  #ifdef OPTION_WINKEY_2_SUPPORT
  pot_full_scale_reading = 1031;
  #else //OPTION_WINKEY_2_SUPPORT
  if (incoming_serial_byte == 255) {
    pot_full_scale_reading = 1031;
  } else {
    if (incoming_serial_byte == 127) {
      pot_full_scale_reading = 515;
    }
  }
  #endif //OPTION_WINKEY_2_SUPPORT
}
#endif //FEATURE_WINKEY_EMULATION

//-------------------------------------------------------------------------------------------------------
#ifdef FEATURE_WINKEY_EMULATION
void winkey_setmode_command(byte incoming_serial_byte) {

  #ifdef OPTION_WINKEY_STRICT_EEPROM_WRITES_MAY_WEAR_OUT_EEPROM
  config_dirty = 1;
  #endif
  if (incoming_serial_byte & 4) {  //serial echo enable
    #ifdef DEBUG_WINKEY_PROTOCOL
    send_char('S',NORMAL);
    #endif
    winkey_serial_echo = 1;
  } else {
    winkey_serial_echo = 0;
  }
  if (incoming_serial_byte & 8) {  //paddle_swap
     paddle_mode = PADDLE_REVERSE;
  } else {
     paddle_mode = PADDLE_NORMAL;
  }
  switch (incoming_serial_byte & 48) {
    case 0: keyer_mode = IAMBIC_B;
      #ifdef DEBUG_WINKEY_PROTOCOL
      send_char('B',NORMAL);
      #endif
      break;
    case 16: keyer_mode = IAMBIC_A;
      #ifdef DEBUG_WINKEY_PROTOCOL
      send_char('A',NORMAL);
      #endif
      break;
    case 32: keyer_mode = ULTIMATIC;
      #ifdef DEBUG_WINKEY_PROTOCOL
      send_char('U',NORMAL);
      #endif
      break;
    case 48: keyer_mode = BUG;
      #ifdef DEBUG_WINKEY_PROTOCOL
      send_char('G',NORMAL);
      #endif
      break;
  }
  #ifdef FEATURE_DEAD_OP_WATCHDOG
  if ((incoming_serial_byte & 128) == 128) {  //1xxxxxxx = paddle watchdog
     dead_op_watchdog_active = 1;
  } else {
     dead_op_watchdog_active = 0;
  }
  #endif
  #ifdef FEATURE_AUTOSPACE
  if ((incoming_serial_byte & 2) == 2) {  //xxxxxx1x = autospace
     autospace_active = 1;
     #ifdef DEBUG_WINKEY_PROTOCOL
     send_char('T',NORMAL);
     #endif
  } else {
     autospace_active = 0;
  }
  #endif
  if ((incoming_serial_byte & 128) == 128) {  //xxxxxxx1 = contest wordspace
     length_wordspace = 6;
  } else {
     length_wordspace = 7;
  }

  if ((incoming_serial_byte & 64) == 64) {  //x1xxxxxx = paddle echo
     winkey_paddle_echo_activated = 1;
  } else {
     winkey_paddle_echo_activated = 0;
  }

}

#endif //FEATURE_WINKEY_EMULATION

//-------------------------------------------------------------------------------------------------------
#ifdef FEATURE_WINKEY_EMULATION
void winkey_sidetone_freq_command(byte incoming_serial_byte) {
  
  #ifdef OPTION_WINKEY_2_SUPPORT
  if (incoming_serial_byte & 128) {
    if (sidetone_mode == SIDETONE_ON) {sidetone_mode = SIDETONE_PADDLE_ONLY;}
    wk2_paddle_only_sidetone = 1;
  } else {
    if (sidetone_mode == SIDETONE_PADDLE_ONLY) {sidetone_mode = SIDETONE_ON;}
    wk2_paddle_only_sidetone = 0;
  }
  #endif
  
  switch (incoming_serial_byte & 15) {
    case 1: hz_sidetone = WINKEY_SIDETONE_1; break;
    case 2: hz_sidetone = WINKEY_SIDETONE_2; break;
    case 3: hz_sidetone = WINKEY_SIDETONE_3; break;
    case 4: hz_sidetone = WINKEY_SIDETONE_4; break;
    case 5: hz_sidetone = WINKEY_SIDETONE_5; break;
    case 6: hz_sidetone = WINKEY_SIDETONE_6; break;
    case 7: hz_sidetone = WINKEY_SIDETONE_7; break;
    case 8: hz_sidetone = WINKEY_SIDETONE_8; break;
    case 9: hz_sidetone = WINKEY_SIDETONE_9; break;
    case 10: hz_sidetone = WINKEY_SIDETONE_10; break;
  }
  #ifdef OPTION_WINKEY_STRICT_EEPROM_WRITES_MAY_WEAR_OUT_EEPROM
  config_dirty = 1;
  #endif

}
#endif //FEATURE_WINKEY_EMULATION

//-------------------------------------------------------------------------------------------------------

#ifdef FEATURE_WINKEY_EMULATION
void winkey_set_pinconfig_command(byte incoming_serial_byte) {
  
  if (incoming_serial_byte & 2) {
    #ifdef OPTION_WINKEY_2_SUPPORT
    if (wk2_paddle_only_sidetone) {
      sidetone_mode = SIDETONE_PADDLE_ONLY;
    } else {
    #endif
      sidetone_mode = SIDETONE_ON;
    #ifdef OPTION_WINKEY_2_SUPPORT
    }
    #endif
  } else {
    sidetone_mode = SIDETONE_OFF;
  }
  
  switch (incoming_serial_byte & 192) {
    case 0:  ultimatic_mode = ULTIMATIC_NORMAL; break;
    case 64: ultimatic_mode = ULTIMATIC_DAH_PRIORITY; break;
    case 128: ultimatic_mode = ULTIMATIC_DIT_PRIORITY; break;
  }

  switch(incoming_serial_byte & 12) {
    case 0:
      key_tx = 0; 
      #ifdef OPTION_WINKEY_2_SUPPORT
      wk2_both_tx_activated = 0;
      #endif
      break;
    case 4: 
      key_tx = 1;
      #ifdef OPTION_WINKEY_2_SUPPORT
      wk2_both_tx_activated = 0;
      #endif
      break;
    case 8: 
      key_tx = 1;
      #ifdef OPTION_WINKEY_2_SUPPORT
      wk2_both_tx_activated = 0;
      #endif
      break;
    case 12:
      key_tx = 1;
 
      #ifdef OPTION_WINKEY_2_SUPPORT
      wk2_both_tx_activated = 1;
      #endif
      break;
    }

}
#endif

//-------------------------------------------------------------------------------------------------------

#ifdef FEATURE_WINKEY_EMULATION
void winkey_load_settings_command(byte winkey_status,byte incoming_serial_byte) {

  switch(winkey_status) {
     case WINKEY_LOAD_SETTINGS_PARM_1_COMMAND:
       winkey_setmode_command(incoming_serial_byte);
       break;
     case WINKEY_LOAD_SETTINGS_PARM_2_COMMAND:
       winkey_unbuffered_speed_command(incoming_serial_byte);
       break;
     case WINKEY_LOAD_SETTINGS_PARM_3_COMMAND:
       winkey_sidetone_freq_command(incoming_serial_byte);
       break;
     case WINKEY_LOAD_SETTINGS_PARM_4_COMMAND:
       winkey_weighting_command(incoming_serial_byte);
       break;
     case WINKEY_LOAD_SETTINGS_PARM_7_COMMAND:
       winkey_set_pot_parm1_command(incoming_serial_byte);
       break;
     case WINKEY_LOAD_SETTINGS_PARM_8_COMMAND:
       winkey_set_pot_parm2_command(incoming_serial_byte);
       break;
     case WINKEY_LOAD_SETTINGS_PARM_9_COMMAND:
       winkey_first_extension_command(incoming_serial_byte);
       break;
     case WINKEY_LOAD_SETTINGS_PARM_10_COMMAND:
       winkey_keying_compensation_command(incoming_serial_byte);
       break;
     case WINKEY_LOAD_SETTINGS_PARM_11_COMMAND:
       winkey_farnsworth_command(incoming_serial_byte);
       break;
     case WINKEY_LOAD_SETTINGS_PARM_12_COMMAND:  // paddle switchpoint - don't need to support

       break;
     case WINKEY_LOAD_SETTINGS_PARM_13_COMMAND:
       winkey_dah_to_dit_ratio_command(incoming_serial_byte);
       break;
     case WINKEY_LOAD_SETTINGS_PARM_14_COMMAND:
       winkey_set_pinconfig_command(incoming_serial_byte);
       break;
     case WINKEY_LOAD_SETTINGS_PARM_15_COMMAND:
       winkey_set_pot_parm3_command(incoming_serial_byte);
       break;
  }
}
#endif

//-------------------------------------------------------------------------------------------------------

#ifdef FEATURE_WINKEY_EMULATION
void winkey_admin_get_values_command() {

  byte byte_to_send;

  // 1 - mode register
  byte_to_send = 0;
  if (length_wordspace != default_length_wordspace) {
    byte_to_send = byte_to_send | 1;
  }
  #ifdef FEATURE_AUTOSPACE
  if (autospace_active) {
    byte_to_send = byte_to_send | 2;
  }
  #endif
  if (winkey_serial_echo) {
    byte_to_send = byte_to_send | 4;
  }
  if (paddle_mode == PADDLE_REVERSE) {
    byte_to_send = byte_to_send | 8;
  }
  switch (keyer_mode) {
    case IAMBIC_A: byte_to_send = byte_to_send | 16; break;
    case ULTIMATIC: byte_to_send = byte_to_send | 32; break;
    case BUG: byte_to_send = byte_to_send | 48; break;
  }
  if (winkey_paddle_echo_activated) {
    byte_to_send = byte_to_send | 64;
  }
  #ifdef FEATURE_DEAD_OP_WATCHDOG
  if (dead_op_watchdog_active) {
    byte_to_send = byte_to_send | 128;
  }
  #endif //FEATURE_DEAD_OP_WATCHDOG
  Serial.write(byte_to_send);

  // 2 - speed
  if (wpm > 99) {
    Serial.write(99);
  } else {
    byte_to_send = wpm;
    Serial.write(byte_to_send);
  }

  // 3 - sidetone
  switch(hz_sidetone) {
    case WINKEY_SIDETONE_1 : Serial.write(1); break;
    case WINKEY_SIDETONE_2 : Serial.write(2); break;
    case WINKEY_SIDETONE_3 : Serial.write(3); break;
    case WINKEY_SIDETONE_4 : Serial.write(4); break;
    case WINKEY_SIDETONE_5 : Serial.write(5); break;
    case WINKEY_SIDETONE_6 : Serial.write(6); break;
    case WINKEY_SIDETONE_7 : Serial.write(7); break;
    case WINKEY_SIDETONE_8 : Serial.write(8); break;
    case WINKEY_SIDETONE_9 : Serial.write(9); break;
    case WINKEY_SIDETONE_10 : Serial.write(10); break;
    default: Serial.write(5); break;
  }

  // 4 - weight
  Serial.write(weighting);

  // 7 - pot min wpm
  Serial.write(pot_wpm_low_value);

  // 8 - pot wpm range
  byte_to_send = pot_wpm_high_value - pot_wpm_low_value;
  Serial.write(byte_to_send);

  // 9 - 1st extension
  Serial.write(first_extension_time);

  // 10 - compensation
  Serial.write(keying_compensation);

  // 11 - farnsworth wpm
  #ifdef FEATURE_FARNSWORTH
  byte_to_send = wpm_farnsworth;
  Serial.write(byte_to_send);
  #endif
  #ifndef FEATURE_FARNSWORTH
  Serial.write(zero);
  #endif

  // 12 - paddle setpoint
  Serial.write(50);  // default value

  // 13 - dah to dit ratio
  Serial.write(50);  // TODO -backwards calculate

  // 14 - pin config
  #ifdef OPTION_WINKEY_2_SUPPORT
  byte_to_send = 0;
  if ((sidetone_mode == SIDETONE_ON) || (sidetone_mode == SIDETONE_PADDLE_ONLY)) {byte_to_send | 2;}
  byte_to_send = byte_to_send | 4;
  if (wk2_both_tx_activated) {byte_to_send = byte_to_send | 12;}
  if (ultimatic_mode == ULTIMATIC_DIT_PRIORITY) {byte_to_send = byte_to_send | 128;}
  if (ultimatic_mode == ULTIMATIC_DAH_PRIORITY) {byte_to_send = byte_to_send | 64;}  
  Serial.write(byte_to_send);
  #else
  Serial.write(5); // default value
  #endif

  // 15 - pot range
  #ifdef OPTION_WINKEY_2_SUPPORT
  Serial.write(zero);
  #else
  Serial.write(0xFF);
  #endif

}
#endif
//-------------------------------------------------------------------------------------------------------

#ifdef FEATURE_SERIAL
#ifdef FEATURE_WINKEY_EMULATION
#ifdef OPTION_WINKEY_2_SUPPORT
void winkey_eeprom_download() {
  
  byte zero = 0;
  unsigned int x = 0;
  //unsigned int y = 0;
  unsigned int bytes_sent = 0;
//  byte byte_read_from_eeprom = 0;
//  byte read_memory_number = 0;
//  byte memory_byte_counter = 0;
//  byte memory_sizes[5];
//  byte total_memory_sizes = 0;
//  byte previous_memories = 0;
  
  Serial.write(0xa5); // 01 magic byte
  winkey_admin_get_values_command(); // 02-16
  
  Serial.write(byte(wpm)); // 17 cmdwpm
  bytes_sent = 17;
  
  // This is a real PITA.  The K1EL Winkey 2 doesn't store memories in ASCII, so a lookup table is required
  
  // produce memory pointers
//  for (read_memory_number = 0; read_memory_number < 6; read_memory_number++) {
//    memory_byte_counter = 0;
//    for (y = (memory_start(read_memory_number)); (y < (memory_end(read_memory_number)+1)); y++) {
//      byte_read_from_eeprom = EEPROM.read(y);
//      if (byte_read_from_eeprom == 255) { // have we found the end of the memory?
//        y = (memory_end(read_memory_number)+1); // exit the loop
//      } else { 
//        memory_byte_counter++;  // count another byte
//      }
//    }
//    memory_sizes[read_memory_number] = memory_byte_counter;
//    total_memory_sizes = total_memory_sizes + memory_byte_counter;
//  }
//  
//  Serial.write((total_memory_sizes+24));  // freeptr
//  for (x = 0; x < 6; x++) { // send memory pointers
//    if (memory_sizes[x] > 0) {
//      Serial.write((memory_sizes[x]+23+previous_memories));
//      previous_memories = previous_memories + memory_sizes[x];
//    } else {
//      Serial.write(0x10);
//    }
//  }
//  
//  bytes_sent = 24;
  

  
  // dump memories
//  for (read_memory_number = 0; read_memory_number < 6; read_memory_number++) {
//    for (y = (memory_start(read_memory_number)); (y < (memory_end(read_memory_number)+1)); y++) {
//      byte_read_from_eeprom = EEPROM.read(y);
//      if (byte_read_from_eeprom == 255) {
//        y = (memory_end(read_memory_number)+1);
//      } else {
//        if ((EEPROM.read(Y+1) == 255)) {
//          Serial.write(byte_read_from_eeprom|128);  // if this is the last byte, set bit 8
//        } else {
//          Serial.write(byte_read_from_eeprom);
//        }
//        bytes_sent++;
//      }
//    }
//  }
  
  //pad the rest with zeros    
  for (x = 0;x < (256-bytes_sent); x++) {
    Serial.write(zero);
  }  
}
#endif
#endif
#endif

//-------------------------------------------------------------------------------------------------------

#ifdef FEATURE_WINKEY_EMULATION
void service_winkey(byte action) {
   
  static byte winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
  static int winkey_parmcount = 0;
  static unsigned long winkey_last_activity;
  byte status_byte_to_send;
  static byte winkey_paddle_echo_space_sent = 1;
  #ifdef OPTION_N1MM_WINKEY_TAB_BUG_WORKAROUND
  static unsigned long winkey_connect_time = 0;
  #endif //OPTION_N1MM_WINKEY_TAB_BUG_WORKAROUND
  #ifdef OPTION_WINKEY_DISCARD_BYTES_AT_STARTUP
  static byte winkey_discard_bytes_init_done = 0;  
  if (!winkey_discard_bytes_init_done) {
    if (Serial.available()) {
      for (int z = winkey_discard_bytes_startup;z > 0;z--) {
        while (Serial.available() == 0) {}
        Serial.read();
      }
      winkey_discard_bytes_init_done = 1;
    }
  }
  #endif //OPTION_WINKEY_DISCARD_BYTES_AT_STARTUP
  
  
  
  if (action == HOUSEKEEPING) {
    if (winkey_last_unbuffered_speed_wpm == 0) {
      winkey_last_unbuffered_speed_wpm = wpm;
    }
    // Winkey interface emulation housekeeping items
    // check to see if we were sending stuff and the buffer is clear
    if (winkey_interrupted) {   // if Winkey sending was interrupted by the paddle, look at PTT line rather than timing out to send 0xc0
      winkey_sending = 0;
      winkey_interrupted = 0;
      Serial.write(0xc0);    // tell the host we've sent everything
      winkey_buffer_counter = 0;
      winkey_buffer_pointer = 0;
    } else {
      //if ((winkey_host_open) && (winkey_sending) && (send_buffer_bytes < 1) && ((millis() - winkey_last_activity) > winkey_c0_wait_time)) {
      if ((Serial.available() == 0) && (winkey_host_open) && (winkey_sending) && (send_buffer_bytes < 1) && ((millis() - winkey_last_activity) > winkey_c0_wait_time)) {
        #ifdef OPTION_WINKEY_SEND_WORDSPACE_AT_END_OF_BUFFER
        send_char(' ',NORMAL);
        #endif
        //add_to_send_buffer(' ');    // this causes a 0x20 to get echoed back to host - doesn't seem to effect N1MM program
        winkey_sending = 0;
        Serial.write(0xc0);    // tell the host we've sent everything
        winkey_buffer_counter = 0;
        winkey_buffer_pointer = 0;
      }
    }
    // failsafe check - if we've been in some command status for awhile waiting for something, clear things out
    if ((winkey_status != WINKEY_NO_COMMAND_IN_PROGRESS) && ((millis() - winkey_last_activity) > 5000)) {
      winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
      winkey_buffer_counter = 0;
      winkey_buffer_pointer = 0;
      Serial.write(0xc0);   //send a C0 back for giggles
    }  
    if ((winkey_host_open) && (winkey_paddle_echo_buffer) && (winkey_paddle_echo_activated) && (millis() > winkey_paddle_echo_buffer_decode_time)) {
      Serial.write(byte(convert_cw_number_to_ascii(winkey_paddle_echo_buffer)));
      winkey_paddle_echo_buffer = 0;
      winkey_paddle_echo_buffer_decode_time = millis() + (float(600/wpm)*length_letterspace);
      winkey_paddle_echo_space_sent = 0;
    }
    if ((winkey_host_open) && (winkey_paddle_echo_buffer == 0) && (winkey_paddle_echo_activated) && (millis() > (winkey_paddle_echo_buffer_decode_time + (float(1200/wpm)*(length_wordspace-length_letterspace)))) && (!winkey_paddle_echo_space_sent)) {
      Serial.write(" ");
      winkey_paddle_echo_space_sent = 1;
    }
  }  // if (action == HOUSEKEEPING)

  if (action == SERVICE_SERIAL_BYTE) {
    //incoming_serial_byte = Serial.read();
    winkey_last_activity = millis();
    if (winkey_status == WINKEY_NO_COMMAND_IN_PROGRESS) {
      if (incoming_serial_byte > 31) {
        byte serial_buffer_position_to_overwrite;
        if (winkey_buffer_pointer > 0) {
          serial_buffer_position_to_overwrite = send_buffer_bytes - (winkey_buffer_counter - winkey_buffer_pointer) - 1;
          if ((send_buffer_bytes > 0) && (serial_buffer_position_to_overwrite < send_buffer_bytes )) {
            send_buffer_array[serial_buffer_position_to_overwrite] = incoming_serial_byte;
          }
          winkey_buffer_pointer++;
        } else {
            #ifdef OPTION_WINKEY_EXTENDED_COMMANDS
            if (incoming_serial_byte == 36) {         // Use the $ sign to escape command
              winkey_status = WINKEY_EXTENDED_COMMAND;
            } else {
            #endif //OPTION_WINKEY_EXTENDED_COMMANDS
              add_to_send_buffer(incoming_serial_byte);
              winkey_buffer_counter++;
            #ifdef OPTION_WINKEY_EXTENDED_COMMANDS
            }
            #endif //OPTION_WINKEY_EXTENDED_COMMANDS
        }

        if (winkey_sending != 1) {
          Serial.write(0xc4);  // tell the client we're starting to send
          winkey_sending = 1;
          #ifdef FEATURE_MEMORIES
          repeat_memory = 255;
          #endif
        }
      } else {
        switch (incoming_serial_byte) {
          case 0x00:
            winkey_status = WINKEY_ADMIN_COMMAND;
            break;
          case 0x01:
            winkey_status = WINKEY_SIDETONE_FREQ_COMMAND;
            break;
          case 0x02:  // speed command - unbuffered
            winkey_status = WINKEY_UNBUFFERED_SPEED_COMMAND;
            break;
          case 0x03:  // weighting
            winkey_status = WINKEY_WEIGHTING_COMMAND;
            break;
          case 0x05:     // speed pot set
            winkey_status = WINKEY_SET_POT_PARM1_COMMAND;
            break;
          case 0x06:
            winkey_status = WINKEY_PAUSE_COMMAND;
            break;
          case 0x07:
            Serial.write(((pot_value_wpm()-pot_wpm_low_value)|128));
            break;
          case 0x08:    // backspace command
            if (send_buffer_bytes > 0) {
              send_buffer_bytes--;
            }
            break;
          case 0x09:
          //fffff
            #ifdef OPTION_N1MM_WINKEY_TAB_BUG_WORKAROUND     // this is a hack; if someone hits TAB in the send CW Window in N1MM, it sends a 0x09
            if ((millis() - winkey_connect_time) < 10000) {  // which according to the standard should be interpreted as a pinconfig command
              winkey_status = WINKEY_SET_PINCONFIG_COMMAND;  // if we've been connected for more than 10 seconds, ignore the 0x09 byte
            }
            #else
            winkey_status = WINKEY_SET_PINCONFIG_COMMAND;
            #endif
            break;
          case 0x0a:                 // 0A - clear buffer - no parms
            if (winkey_sending) {
              send_buffer_bytes = 0;
              winkey_sending = 0;
              Serial.write(0xc0);
            }
            winkey_buffer_counter = 0;
            winkey_buffer_pointer = 0;
            #ifdef FEATURE_MEMORIES
            repeat_memory = 255;
            #endif
            tx_and_sidetone_key(0,AUTOMATIC_SENDING);  // N1MM program needs this for the CTRL-T tune command to work right since it issues a 0x0a
                                     // rather than 0x0b 0x00 to clear a key down - doesn't follow protocol spec
            break;
          case 0x0b:
            winkey_status = WINKEY_KEY_COMMAND;
            break;
          case 0x0c:
            winkey_status = WINKEY_HSCW_COMMAND;
            break;
          case 0x0d:
            winkey_status = WINKEY_FARNSWORTH_COMMAND;
            break;
          case 0x0e:
            winkey_status = WINKEY_SETMODE_COMMAND;
            break;
          case 0x0f:  // bulk load of defaults
            winkey_status = WINKEY_LOAD_SETTINGS_PARM_1_COMMAND;
            break;
          case 0x10:
            winkey_status = WINKEY_FIRST_EXTENSION_COMMAND;
            break;
          case 0x11:
            winkey_status = WINKEY_KEYING_COMPENSATION_COMMAND;
            break;
          case 0x12:
            winkey_status = WINKEY_UNSUPPORTED_COMMAND;
            winkey_parmcount = 1;
            break;
          case 0x13:  // NULL command
            break;
          case 0x14:
            winkey_status = WINKEY_SOFTWARE_PADDLE_COMMAND;
            break;
          case 0x15:  // report status
            status_byte_to_send = 0xc0;
            if (winkey_sending) {
              status_byte_to_send = status_byte_to_send | 4;
            }
            if (send_buffer_status == SERIAL_SEND_BUFFER_TIMED_COMMAND) {
              status_byte_to_send = status_byte_to_send | 16;
            }
            Serial.write(status_byte_to_send);
            break;
          case 0x16:  // Pointer operation
            winkey_status = WINKEY_POINTER_COMMAND;
            break;
          case 0x17:  // dit to dah ratio
            winkey_status = WINKEY_DAH_TO_DIT_RATIO_COMMAND;
            break;
          // start of buffered commands ------------------------------
          case 0x19:
            winkey_status = WINKEY_KEY_BUFFERED_COMMAND;
            break;
          case 0x1a:
            winkey_status = WINKEY_WAIT_BUFFERED_COMMAND;
            break;
          case 0x1b:
            winkey_status = WINKEY_MERGE_COMMAND;
            break;
          case 0x1c:      // speed command - buffered
             winkey_status = WINKEY_BUFFERED_SPEED_COMMAND;
            break;
          case 0x1d:
            winkey_status = WINKEY_BUFFERED_HSCW_COMMAND;
            break;
          case 0x1e:  // cancel buffered speed command - buffered
            winkey_status = WINKEY_CANCEL_BUFFERED_SPEED_COMMAND;
            break;
          case 0x1f:  // buffered NOP - no need to do anything
            break;
        } //switch (incoming_serial_byte)
      }
    } else {

      //WINKEY_LOAD_SETTINGS_PARM_1_COMMAND IS 101
      if ((winkey_status > 100) && (winkey_status < 116)) {   // Load Settings Command - this has 15 parameters, so we handle it a bit differently
        winkey_load_settings_command(winkey_status,incoming_serial_byte);
        winkey_status++;
        if (winkey_status > 115) {
          winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
        }
      }

      #ifdef OPTION_WINKEY_EXTENDED_COMMANDS
      if (winkey_status == WINKEY_EXTENDED_COMMAND) {  // this is for command extensions - not part of Winkey protocol
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;

      }
      #endif //OPTION_WINKEY_EXTENDED_COMMANDS

      if (winkey_status == WINKEY_SET_PINCONFIG_COMMAND) {
        winkey_set_pinconfig_command(incoming_serial_byte);
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
      }

      if (winkey_status == WINKEY_MERGE_COMMAND) {
        #ifdef FEATURE_MEMORIES
        repeat_memory = 255;
        #endif
        add_to_send_buffer(SERIAL_SEND_BUFFER_PROSIGN);
        add_to_send_buffer(incoming_serial_byte);
        winkey_status = WINKEY_MERGE_PARM_2_COMMAND;
      } else {
        if (winkey_status == WINKEY_MERGE_PARM_2_COMMAND) {
          add_to_send_buffer(incoming_serial_byte);
          winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
        }
      }
      if (winkey_status == WINKEY_UNBUFFERED_SPEED_COMMAND) {
        winkey_unbuffered_speed_command(incoming_serial_byte);
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
      }

      if (winkey_status == WINKEY_FARNSWORTH_COMMAND) {
        winkey_farnsworth_command(incoming_serial_byte);
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
      }

      if (winkey_status ==  WINKEY_HSCW_COMMAND) {
        if (incoming_serial_byte != 0) {
          wpm = ((incoming_serial_byte*100)/5);
          winkey_last_unbuffered_speed_wpm = wpm;
          #ifdef OPTION_WINKEY_STRICT_EEPROM_WRITES_MAY_WEAR_OUT_EEPROM
          config_dirty = 1;
          #endif
        }
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
      }

      if (winkey_status == WINKEY_BUFFERED_SPEED_COMMAND) {
        add_to_send_buffer(SERIAL_SEND_BUFFER_WPM_CHANGE);
        add_to_send_buffer(0);
        add_to_send_buffer(incoming_serial_byte);
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
      }

      if (winkey_status == WINKEY_BUFFERED_HSCW_COMMAND) {
        add_to_send_buffer(SERIAL_SEND_BUFFER_WPM_CHANGE);
        unsigned int send_buffer_wpm = ((incoming_serial_byte*100)/5);
        add_to_send_buffer(highByte(send_buffer_wpm));
        add_to_send_buffer(lowByte(send_buffer_wpm));
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
      }

      if (winkey_status == WINKEY_KEY_BUFFERED_COMMAND) {
        add_to_send_buffer(SERIAL_SEND_BUFFER_TIMED_KEY_DOWN);
        add_to_send_buffer(incoming_serial_byte);
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
      }

      if (winkey_status == WINKEY_WAIT_BUFFERED_COMMAND) {
        add_to_send_buffer(SERIAL_SEND_BUFFER_TIMED_WAIT);
        add_to_send_buffer(incoming_serial_byte);
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
      }

      if (winkey_status == WINKEY_CANCEL_BUFFERED_SPEED_COMMAND) {
        add_to_send_buffer(SERIAL_SEND_BUFFER_WPM_CHANGE);
        add_to_send_buffer(winkey_last_unbuffered_speed_wpm);
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
      }

      if (winkey_status == WINKEY_POINTER_01_COMMAND) { // move input pointer to new positon in overwrite mode
        winkey_buffer_pointer = incoming_serial_byte;
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
      }

      if (winkey_status == WINKEY_POINTER_02_COMMAND) { // move input pointer to new position in append mode
        winkey_buffer_pointer = 0;
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
      }

      if (winkey_status == WINKEY_POINTER_03_COMMAND) { // add multiple nulls to buffer
        byte serial_buffer_position_to_overwrite;
        for (byte x = incoming_serial_byte; x > 0; x--) {
          if (winkey_buffer_pointer > 0) {
            serial_buffer_position_to_overwrite = send_buffer_bytes - (winkey_buffer_counter - winkey_buffer_pointer) - 1;
            if ((send_buffer_bytes > 0) && (serial_buffer_position_to_overwrite < send_buffer_bytes )) {
              send_buffer_array[serial_buffer_position_to_overwrite] = SERIAL_SEND_BUFFER_NULL;
            }
            winkey_buffer_pointer++;
          } else {
              add_to_send_buffer(SERIAL_SEND_BUFFER_NULL);
              winkey_buffer_counter++;
          }
        }
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
      }

      if (winkey_status == WINKEY_POINTER_COMMAND) {
        switch (incoming_serial_byte) {
          case 0x00:
            winkey_buffer_counter = 0;
            winkey_buffer_pointer = 0;
            winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
            break;
          case 0x01:
            winkey_status = WINKEY_POINTER_01_COMMAND;
            break;
          case 0x02:
            winkey_status = WINKEY_POINTER_02_COMMAND;  // move to new position in append mode
            break;
          case 0x03:
            winkey_status = WINKEY_POINTER_03_COMMAND;
            break;
          default:
            winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
            break;
        }
      }

      #ifdef OPTION_WINKEY_2_SUPPORT
      if (winkey_status == WINKEY_SEND_MSG) {
        if ((incoming_serial_byte > 0) && (incoming_serial_byte < 7)) {
          add_to_send_buffer(SERIAL_SEND_BUFFER_MEMORY_NUMBER);
          add_to_send_buffer(incoming_serial_byte - 1);
          #ifdef FEATURE_MEMORIES
          repeat_memory = 255;
          #endif
        }
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;  
      }
      #endif //OPTION_WINKEY_2_SUPPORT

      if (winkey_status == WINKEY_ADMIN_COMMAND) {
        switch (incoming_serial_byte) {
          case 0x00: winkey_status = WINKEY_UNSUPPORTED_COMMAND; winkey_parmcount = 1; break;  // calibrate command
          case 0x01: wdt_enable(WDTO_30MS); while(1) {}; break;  // reset command
          case 0x02:  // host open command - send version back to host
            #ifdef OPTION_WINKEY_2_SUPPORT
            Serial.write(23);
            #else //OPTION_WINKEY_2_SUPPORT
            Serial.write(10);
            #endif //OPTION_WINKEY_2_SUPPORT
            winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
            winkey_host_open = 1;
            #ifdef OPTION_N1MM_WINKEY_TAB_BUG_WORKAROUND
            winkey_connect_time = millis();
            #endif
            boop_beep();
            break;
          case 0x03: // host close command
            winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
            winkey_host_open = 0;
            beep_boop();
            #ifdef OPTION_WINKEY_2_SUPPORT
            Serial.end();
            Serial.begin(1200);
            #endif
            break;
          case 0x04:  // echo command
            winkey_status = WINKEY_ADMIN_COMMAND_ECHO;
            break;
          case 0x05: // paddle A2D
            Serial.write(zero);
            winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
            break;
          case 0x06: // speed A2D
            Serial.write(zero);
            winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
            break;
          case 0x07: // Get values
            winkey_admin_get_values_command();
            winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
            break;
          case 0x08: // reserved
            winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
            break;  
          case 0x09: // get cal
            Serial.write(zero);
            winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
            break;
          #ifdef OPTION_WINKEY_2_SUPPORT
          case 0x0a: // set wk1 mode
            wk2_mode = 1;
            winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
            break;
          case 0x0b: // set wk2 mode
            beep();
            beep();
            wk2_mode = 2;
            winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
            break;            
          case 0x0c: // download EEPPROM 256 bytes
            winkey_eeprom_download();
            winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
            break;  
          case 0x0d:
            winkey_status = WINKEY_UNSUPPORTED_COMMAND;  // upload EEPROM 256 bytes
            winkey_parmcount = 256;
            break;       
          case 0x0e:
            winkey_status = WINKEY_SEND_MSG;
            break;
          case 0x0f: // load xmode
            winkey_status = WINKEY_UNSUPPORTED_COMMAND;
            winkey_parmcount = 1;
            break;            
          case 0x10: // reserved
            Serial.write(zero);
            winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
            break;
          case 0x11: // set high baud rate
            Serial.end();
            Serial.begin(9600);
            winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
            break;
          case 0x12: // set low baud rate
            Serial.end();
            Serial.begin(1200);
            winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
            break;
          #endif //OPTION_WINKEY_2_SUPPORT  
          default:
            winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
            break;
          }
      } else {
        if (winkey_status == WINKEY_ADMIN_COMMAND_ECHO) {
          Serial.write(incoming_serial_byte);
          winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
        }
      }

      if (winkey_status == WINKEY_KEYING_COMPENSATION_COMMAND) {
        keying_compensation = incoming_serial_byte;
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
      }

      if (winkey_status == WINKEY_FIRST_EXTENSION_COMMAND) {
        first_extension_time = incoming_serial_byte;
        #ifdef DEBUG_WINKEY_PROTOCOL
        send_char('X',NORMAL);
        #endif
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
      }

      if (winkey_status == WINKEY_PAUSE_COMMAND) {
        if (incoming_serial_byte) {
          pause_sending_buffer = 1;
        } else {
          pause_sending_buffer = 0;
        }
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
      }

      if (winkey_status ==  WINKEY_KEY_COMMAND) {
        #ifdef FEATURE_MEMORIES
        repeat_memory = 255;
        #endif
        if (incoming_serial_byte) {
          tx_and_sidetone_key(1,AUTOMATIC_SENDING);
        } else {
          tx_and_sidetone_key(0,AUTOMATIC_SENDING);
        }
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
      }

      if (winkey_status ==  WINKEY_DAH_TO_DIT_RATIO_COMMAND) {
        winkey_dah_to_dit_ratio_command(incoming_serial_byte);
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
      }

      if (winkey_status == WINKEY_WEIGHTING_COMMAND) {
        winkey_weighting_command(incoming_serial_byte);
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
      }

      if (winkey_status == WINKEY_SET_POT_PARM1_COMMAND) {
        winkey_set_pot_parm1_command(incoming_serial_byte);
        winkey_status = WINKEY_SET_POT_PARM2_COMMAND;
      } else {
        if (winkey_status == WINKEY_SET_POT_PARM2_COMMAND) {
          winkey_set_pot_parm2_command(incoming_serial_byte);
          winkey_status = WINKEY_SET_POT_PARM3_COMMAND;
        } else {
          if (winkey_status == WINKEY_SET_POT_PARM3_COMMAND) {  // third parm is max read value from pot, depending on wiring
            winkey_set_pot_parm3_command(incoming_serial_byte); // WK2 protocol just ignores this third parm
            winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;      // this is taken care of in winkey_set_pot_parm3()
          }
        }
      }

      if (winkey_status ==  WINKEY_SETMODE_COMMAND) {
        winkey_setmode_command(incoming_serial_byte);
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
      }

      if (winkey_status ==  WINKEY_SOFTWARE_PADDLE_COMMAND) {
        #ifdef FEATURE_MEMORIES
        repeat_memory = 255;
        #endif
        switch (incoming_serial_byte) {
          case 0: winkey_dit_invoke = 0; winkey_dah_invoke = 0; break;
          case 1: winkey_dit_invoke = 1; winkey_dah_invoke = 0; break;
          case 2: winkey_dit_invoke = 0; winkey_dah_invoke = 1; break;
          case 3: winkey_dah_invoke = 1; winkey_dit_invoke = 1; break;
        }
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
      }

      if (winkey_status ==  WINKEY_SIDETONE_FREQ_COMMAND) {
        winkey_sidetone_freq_command(incoming_serial_byte);
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
      }

      if (winkey_status == WINKEY_UNSUPPORTED_COMMAND) {
        winkey_parmcount--;
        if (winkey_parmcount == 0) {
          if (winkey_sending) {
            Serial.write(0xc4);
          } else {
            Serial.write(0xc0);
          }
          winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
        }

      }
    } // else (winkey_status == WINKEY_NO_COMMAND_IN_PROGRESS)
  }  // if (action == SERVICE_SERIAL_BYTE

  
}
#endif  //FEATURE_WINKEY_EMULATION

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
        } else {
          if (incoming_serial_byte == 13) {
            #ifdef DEBUG_CHECK_SERIAL
            Serial.println(F("check_serial: add_to_send_buffer(SERIAL_SEND_BUFFER_HOLD_SEND_RELEASE)"));
            #endif
            add_to_send_buffer(SERIAL_SEND_BUFFER_HOLD_SEND_RELEASE);
            cli_wait_for_cr_flag = 0;
          }
        }
      }
      add_to_send_buffer(incoming_serial_byte);
      #ifdef FEATURE_MEMORIES
      if (incoming_serial_byte != 13) {
        repeat_memory = 255;
      }
      #endif
    } else {     //(incoming_serial_byte != 92)  -- we got a backslash
      serial_backslash_command = 1;
      Serial.write(incoming_serial_byte);
    }
  } else { // (serial_backslash_command == 0) -- we already got a backslash
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
void check_serial()
{
  
  #ifdef DEBUG_LOOP
  Serial.println(F("loop: entering check_serial")); 
  #endif 
    

  #ifdef FEATURE_WINKEY_EMULATION
  if (serial_mode == SERIAL_WINKEY_EMULATION) {
    service_winkey(HOUSEKEEPING);
  }
  #endif

  // Reminder to Goody: multi-parameter commands must be nested in if-then-elses

  while (Serial.available() > 0) {
    incoming_serial_byte = Serial.read();
    #ifndef FEATURE_WINKEY_EMULATION
    #ifndef FEATURE_COMMAND_LINE_INTERFACE
    //incoming_serial_byte = Serial.read();
    Serial.println(F("No serial features enabled..."));
    #endif
    #endif

    // yea, this is a bit funky below

    #ifdef FEATURE_WINKEY_EMULATION
    if (serial_mode == SERIAL_WINKEY_EMULATION) {
      service_winkey(SERVICE_SERIAL_BYTE);
    } else {
    #endif //FEATURE_WINKEY_EMULATION
    
    #ifdef FEATURE_COMMAND_LINE_INTERFACE    
    service_command_line_interface();
    #endif //FEATURE_COMMAND_LINE_INTERFACE
    
    #ifdef FEATURE_WINKEY_EMULATION
    } // if (serial_mode == SERIAL_WINKEY_EMULATION)
    #endif //FEATURE_WINKEY_EMULATION
  }  //while (Serial.available() > 0)
}
#endif

//---------------------------------------------------------------------

#ifdef FEATURE_SERIAL
#ifdef FEATURE_COMMAND_LINE_INTERFACE
void process_serial_command() {
        
  Serial.println();
  switch (incoming_serial_byte) {
    case 126: wdt_enable(WDTO_30MS); while(1) {} ; break;  // ~ - reset unit
    case 42:                                                // * - paddle echo on / off
      if (cli_paddle_echo) {
        cli_paddle_echo = 0;
      } else {
        cli_paddle_echo = 1;
      }
      break;
    case 43: cli_prosign_flag = 1; break;
    #ifdef FEATURE_SERIAL_HELP
    case 63: serial_print(serial_help_string); break;                         // ? = print help
    #endif //FEATURE_SERIAL_HELP
    case 65: keyer_mode = IAMBIC_A; config_dirty = 1; Serial.println(F("Iambic A")); break;    // A - Iambic A mode
    case 66: keyer_mode = IAMBIC_B; config_dirty = 1; Serial.println(F("Iambic B")); break;    // B - Iambic B mode
    case 68: keyer_mode = ULTIMATIC; config_dirty = 1; Serial.println(F("Ultimatic")); break;  // D - Ultimatic mode
    case 69: serial_set_serial_number(); break;                                   // E - set serial number
    case 70: serial_set_sidetone_freq(); break;                                   // F - set sidetone frequency
    case 71: keyer_mode = BUG; config_dirty = 1; Serial.println(F("Bug")); break;              // G - Bug mode
    case 73:                                                                      // I - transmit line on/off
      Serial.print(F("TX o"));
      if (key_tx) {
        key_tx = 0;
        Serial.println(F("ff"));
      } else {
        key_tx = 1;
        Serial.println(F("n"));
      }
      break;
    #ifdef FEATURE_MEMORIES
    case 33: repeat_play_memory(); break;      // ! - repeat play
    case 124: serial_set_memory_repeat(); break; // | - set memory repeat time
    case 49: serial_play_memory(0); break;     // 1 - play memory 1  (0)
    case 50: serial_play_memory(1); break;     // 2 - play memory 2  (1)
    case 51: serial_play_memory(2); break;     // 3 - play memory 3  (2)
    case 52: serial_play_memory(3); break;     // 4 - play memory 4  (3)
    case 53: serial_play_memory(4); break;     // 5 - play memory 5  (4)
    case 54: serial_play_memory(5); break;     // 6 - play memory 6  (5)
    case 55: serial_play_memory(6); break;     // 7 - play memory 7  (6)
    case 56: serial_play_memory(7); break;     // 8 - play memory 8  (7)
    case 57: serial_play_memory(8); break;     // 9 - play memory 9  (8)

    case 80: repeat_memory = 255; serial_program_memory(); break;                                // P - program memory
    #endif //FEATURE_MEMORIES
    case 81: serial_qrss_mode(); break; // Q - activate QRSS mode
    case 82: speed_mode = SPEED_NORMAL; Serial.println(F("QRSS Off")); break; // R - activate regular timing mode
    case 83: serial_status(); break;                                              // S - Status command
    case 74: serial_set_dit_to_dah_ratio(); break;                          // J - dit to dah ratio
    #ifdef FEATURE_CALLSIGN_RECEIVE_PRACTICE
    case 75: serial_callsign_receive_practice(); break;                     // K - callsign receive practice
    #endif //FEATURE_CALLSIGN_RECEIVE_PRACTICE
    case 76: serial_set_weighting(); break;
    #ifdef FEATURE_FARNSWORTH
    case 77: serial_set_farnsworth(); break;                                // M - set Farnsworth speed
    #endif
    case 78:                                                                // N - paddle reverse
      Serial.print(F("Paddles "));
      if (paddle_mode == PADDLE_NORMAL) {
        paddle_mode = PADDLE_REVERSE;
        Serial.println(F("reversed"));
      } else {
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
      } else {
        sidetone_mode = SIDETONE_ON;
        Serial.println(F("N"));
      }
      config_dirty = 1;
    break; // case 79
    case 84: // T - tune
      #ifdef FEATURE_MEMORIES
      repeat_memory = 255;
      #endif
      serial_tune_command(); break;
   
    case 87: serial_wpm_set();break;                                        // W - set WPM
    case 89: serial_change_wordspace(); break;
    #ifdef FEATURE_AUTOSPACE
    case 90:
      Serial.print(F("Autospace O"));
      if (autospace_active) {
        autospace_active = 0;
        config_dirty = 1;
        Serial.println(F("ff"));
      } else {
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
      repeat_memory = 255;
      break;                           // \ - double backslash - clear serial send buffer
    #endif
    case 94:                           // ^ - toggle send CW send immediately
       if (cli_wait_for_cr_to_send_cw) {
         cli_wait_for_cr_to_send_cw = 0;
         Serial.println(F("Send CW immediately"));
       } else {
         cli_wait_for_cr_to_send_cw = 1;
         Serial.println(F("Wait for CR to send CW"));
       }
      break;
    default: Serial.println(F("Unknown command")); break;
  }

}
#endif //FEATURE_SERIAL
#endif //FEATURE_COMMAND_LINE_INTERFACE
//---------------------------------------------------------------------
#ifdef FEATURE_SERIAL
#ifdef FEATURE_COMMAND_LINE_INTERFACE

void service_serial_paddle_echo()
{
  
  #ifdef DEBUG_LOOP
  Serial.println(F("loop: entering service_serial_paddle_echo"));
  #endif          

  static byte cli_paddle_echo_space_sent = 1;

  if ((cli_paddle_echo_buffer) && (cli_paddle_echo) && (millis() > cli_paddle_echo_buffer_decode_time)) {
    Serial.write(byte(convert_cw_number_to_ascii(cli_paddle_echo_buffer)));
    cli_paddle_echo_buffer = 0;
    cli_paddle_echo_buffer_decode_time = millis() + (float(600/wpm)*length_letterspace);
    cli_paddle_echo_space_sent = 0;
  }
  if ((cli_paddle_echo_buffer == 0) && (cli_paddle_echo) && (millis() > (cli_paddle_echo_buffer_decode_time + (float(1200/wpm)*(length_wordspace-length_letterspace)))) && (!cli_paddle_echo_space_sent)) {
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
void serial_set_memory_repeat() {

  int temp_int = serial_get_number_input(5, -1, 32000);
  if (temp_int > -1) {
    memory_repeat_time = temp_int;
    config_dirty = 1;
  }

}
#endif
#endif
#endif
//---------------------------------------------------------------------

#ifdef FEATURE_SERIAL
#ifdef FEATURE_COMMAND_LINE_INTERFACE
#ifdef FEATURE_MEMORIES
void repeat_play_memory() {

  byte memory_number = serial_get_number_input(2,0, (number_of_memories+1));
  if (memory_number > -1) {
    repeat_memory = memory_number - 1;
  }

}
#endif
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
    repeat_memory = 255;
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
    } else {
      incoming_serial_byte = Serial.read();
      if ((incoming_serial_byte > 47) && (incoming_serial_byte < 58)) {    // ascii 48-57 = "0" - "9")
        numberstring = numberstring + incoming_serial_byte;
        numbers[numberindex] = incoming_serial_byte;
        numberindex++;
        if (numberindex > places){
            looping = 0;
            error = 1;
        }
      } else {
        if (incoming_serial_byte == 13) {   // carriage return - get out
          looping = 0;
        } else {                 // bogus input - error out
          looping = 0;
          error = 1;
        }
      }
    }
  }
  if (error) {
    Serial.write("Error...\n\r");
    while (Serial.available() > 0) { incoming_serial_byte = Serial.read(); }  // clear out buffer
    return(-1);
  } else {
    int y = 1;
    int return_number = 0;
    for (int x = (numberindex - 1); x >= 0 ; x = x - 1) {
      return_number = return_number + ((numbers[x]-48) * y);
      y = y * 10;
    }
    if ((return_number > lower_limit) && (return_number < upper_limit)) {
      return(return_number);
    } else {
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
void serial_wpm_set()
{
  int set_wpm = serial_get_number_input(3,0,1000);
  if (set_wpm > 0) {
    speed_set(set_wpm);
    Serial.write("Setting WPM to ");
    Serial.println(set_wpm,DEC);
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
  #ifdef FEATURE_COMMAND_BUTTONS
  while ((Serial.available() == 0) && (!analogbuttonread(0))) {}  // keystroke or button0 hit gets us out of here
  #endif
  while (Serial.available() > 0) {  // clear out the buffer if anything is there
    incoming = Serial.read();
  }
  tx_and_sidetone_key(0,MANUAL_SENDING);

}
#endif
#endif
//---------------------------------------------------------------------
#ifdef FEATURE_CALLSIGN_RECEIVE_PRACTICE

String generate_callsign() {

  String callsign(10);
  char nextchar;
  long random_number = 0;

  switch (random(1,5)) {
    case 1: callsign = "K"; break;
    case 2: callsign = "W"; break;
    case 3: callsign = "N"; break;
    case 4: callsign = "A"; break;
  }
  if (callsign == "A") {                   // if the first letter is A, we definitely need a second letter before the number
    nextchar = random(65,91);
    callsign = callsign + nextchar;
  } else {
    random_number = random(0,1);           // randomly add a second letter for K, W, N prefixes
    if (random_number) {
      nextchar = random(65,91);
      callsign = callsign + nextchar;
    }
  }
  nextchar = random(48,58);               // generate the number
  callsign = callsign + nextchar;
  nextchar = random(65,91);               // generate first letter after number
  callsign = callsign + nextchar;
  if (random(1,5) < 4) {                  // randomly put a second character after the number
    nextchar = random(65,91);
    callsign = callsign + nextchar;
    if (random_number < 3) {              // randomly put a third character after the number
      nextchar = random(65,91);
      callsign = callsign + nextchar;
    }
  }
  if (random(1,16) == 1) {                // randomly put a slash something on the end like /QRP or /#
    if (random(1,4) == 1) {
      callsign = callsign + "/QRP";
    } else {
       nextchar = random(48,58);
       callsign = callsign + "/" + nextchar;
    }
  }

  return callsign;
}

#endif //FEATURE_CALLSIGN_RECEIVE_PRACTICE

//---------------------------------------------------------------------

#ifdef FEATURE_SERIAL
#ifdef FEATURE_CALLSIGN_RECEIVE_PRACTICE
#ifdef FEATURE_COMMAND_LINE_INTERFACE
void serial_callsign_receive_practice()
{

  byte looping = 1;
  byte serialwaitloop = 0;
  String callsign(10);
  char incoming_char = ' ';
  String user_entered_callsign = "";

  randomSeed(analogRead(0));
  serial_print(string_callsign_receive_practice);
  while (Serial.available() > 0) {  // clear out the buffer if anything is there
    incoming_char = Serial.read();
  }
  Serial.print(F("Press enter to start...."));
  while (Serial.available() == 0) {
  }
  while (Serial.available() > 0) {  // clear out the buffer if anything is there
    incoming_char = Serial.read();
  }

  while (looping){

    callsign = generate_callsign();

    for (byte x = 0; x < (callsign.length()); x++) {
      send_char(callsign[x],NORMAL);
    }

    serialwaitloop = 1;
    user_entered_callsign = "";
    while (serialwaitloop) {
      if (Serial.available() > 0) {
        incoming_char = Serial.read();
        Serial.print(incoming_char);
        if (incoming_char == 13) {
          serialwaitloop = 0;
        } else {
          user_entered_callsign = user_entered_callsign + incoming_char;
        }
      }
    }

    if (user_entered_callsign[0] == '\\') {
      Serial.print(F("Exiting...\n\n\r"));
      looping = 0;
    } else {
      user_entered_callsign.toUpperCase();  // the toUpperCase function was modified in 1.0; now it changes string in place
      if (callsign.compareTo(user_entered_callsign) == 0) {
        Serial.print(F("\nCorrect!\n\r"));
      } else {
        Serial.print(F("\nWrong: "));
        Serial.println(callsign);
      }
    }

    delay(100);
    #ifndef FEATURE_COMMAND_BUTTONS
    while ((digitalRead(paddle_left) == LOW) || (digitalRead(paddle_right) == LOW) || (analogbuttonread(0))) {
      looping = 0;
    }
    #endif
    delay(10);
  }
}
#endif
#endif
#endif
//---------------------------------------------------------------------

#ifdef FEATURE_SERIAL
#ifdef FEATURE_COMMAND_LINE_INTERFACE
void serial_status() {

  Serial.println();
  switch (keyer_mode) {
    case IAMBIC_A: Serial.write("Iambic A"); break;
    case IAMBIC_B: Serial.write("Iambic B"); break;
    case BUG: Serial.write("Bug"); break;
    case STRAIGHT: Serial.write("Straightkey"); break;
    case ULTIMATIC: Serial.write("Ultimatic"); break;
  }
  Serial.write("\n\r");
  if (speed_mode == SPEED_NORMAL) {
    Serial.write("WPM: ");
    Serial.println(wpm,DEC);
    #ifdef FEATURE_FARNSWORTH
    Serial.write("Farnsworth WPM: ");
    if (wpm_farnsworth < wpm) {
      Serial.write("disabled\n\r");
    } else {
      Serial.println(wpm_farnsworth,DEC);
    }
    #endif //FEATURE_FARNSWORTH
  } else {
    Serial.print(F("QRSS Mode Activated - Dit Length: "));
    Serial.print(qrss_dit_length,DEC);
    Serial.println(" seconds");
  }

  Serial.print(F("Sidetone Hz: "));
  Serial.println(hz_sidetone,DEC);
  Serial.print(F("Sidetone:"));
  switch (sidetone_mode) {
    case SIDETONE_ON: Serial.println(F("ON")); break;
    case SIDETONE_OFF: Serial.println(F("OFF")); break;
    case SIDETONE_PADDLE_ONLY: Serial.println(F("Paddle Only")); break;
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
  } else {
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

int convert_cw_number_to_ascii (long number_in)
{

 switch (number_in) {
   case 12: return 65; break;         // A
   case 2111: return 66; break;
   case 2121: return 67; break;
   case 211: return 68; break;
   case 1: return 69; break;
   case 1121: return 70; break;
   case 221: return 71; break;
   case 1111: return 72; break;
   case 11: return 73; break;
   case 1222: return 74; break;
   case 212: return 75; break;
   case 1211: return 76; break;
   case 22: return 77; break;
   case 21: return 78; break;
   case 222: return 79; break;
   case 1221: return 80; break;
   case 2212: return 81; break;
   case 121: return 82; break;
   case 111: return 83; break;
   case 2: return 84; break;
   case 112: return 85; break;
   case 1112: return 86; break;
   case 122: return 87; break;
   case 2112: return 88; break;
   case 2122: return 89; break;
   case 2211: return 90; break;    // Z

   case 22222: return 48; break;    // 0
   case 12222: return 49; break;
   case 11222: return 50; break;
   case 11122: return 51; break;
   case 11112: return 52; break;
   case 11111: return 53; break;
   case 21111: return 54; break;
   case 22111: return 55; break;
   case 22211: return 56; break;
   case 22221: return 57; break;

   case 112211: return 63; break;  // ?
   case 21121: return 47; break;   // /
   case 21112: return 45; break;   // -
   case 221122: return 44; break;  // ,
   case 2111212: return 36; break; // BK (store as ascii $)

   case 222222: return 92; break;  // special hack; six dahs = \ (backslash)

   case 9: return 32; break;       // special 9 = space
   
   #ifdef OPTION_NON_ENGLISH_EXTENSIONS
   case 12212: return 197; break;   // Å   - customize for your locality  ( 1 = dit, 2 = dah, return code is ASCII code )
   case 1212: return 196; break;    // Ä   - customize for your locality
   //case 12212: return 192; break; // À   - customize for your locality
   //case 1212: return 197; break;  // Ä   - customize for your locality
   //case 1212: return 198; break;  // Æ   - customize for your locality
   case 21211: return 199; break;   // Ç
   case 11221: return 208; break;   // Ð
   case 2222: return 138; break;    // Š
   case 12112: return 200; break;   // È
   case 11211: return 201; break;   // É
   case 221121: return 142; break;  // Ž
   case 22122: return 209; break;   // Ñ
   case 2221: return 214; break;    // Ö 
   //case 2221: return 211; break;  // Ó   - customize for your locality  ( 1 = dit, 2 = dah, return code is ASCII code )
   //case 2221: return 216; break;  // Ø   - customize for your locality
   case 1122: return 220; break;    // Ü 
   case 111111: return 223; break;   // ß
   #endif //OPTION_NON_ENGLISH_EXTENSIONS

   default: return 254; break;
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
  if (serial_mode == SERIAL_NORMAL) {
    Serial.print((unsigned long)free,DEC);
    Serial.write(" bytes free\n\r");
  }
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
    } else {
      for (int y = (memory_start(x)); (y < last_memory_location); y++) {
        if (EEPROM.read(y) < 255) {
          Serial.write(EEPROM.read(y));
        } else {
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
      } else {
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
  } else {
    Serial.println(F("\n\rError"));
  }

}

#endif
#endif
#endif
#endif

//---------------------------------------------------------------------

#ifdef FEATURE_MEMORIES
#ifdef FEATURE_COMMAND_BUTTONS
void command_program_memory()
{
  int cw_char;
  cw_char = get_cw_input_from_user(0);            // get another cw character from the user to find out which memory number
  #ifdef DEBUG_COMMAND_MODE
  Serial.print(F("command_program_memory: cw_char: "));
  Serial.println(cw_char);
  #endif
  if (cw_char > 0) {  //aaaaa
    if ((cw_char == 12222) && (number_of_memories > 9)) { // we have a 1, this could be 1 or 1x
      cw_char = get_cw_input_from_user(p_command_single_digit_wait_time);  // give the user 3 seconds to send another number
      switch (cw_char) {
        case 0: program_memory(0); break;    // we didn't get anything, it's a 1   
        case 22222: program_memory(9); break; 
        case 12222: program_memory(10); break;
        case 11222: program_memory(11); break;
        case 11122: program_memory(12); break;
        case 11112: program_memory(13); break;
        case 11111: program_memory(14); break;
        case 21111: program_memory(15); break;
        default: send_char('?',NORMAL); break;
      }
    } else {    
      switch (cw_char) {
        case 12222: program_memory(0); break;      // 1 = memory 0
        case 11222: program_memory(1); break;
        case 11122: program_memory(2); break;
        case 11112: program_memory(3); break;
        case 11111: program_memory(4); break;
        case 21111: program_memory(5); break;
        case 22111: program_memory(6); break;
        case 22211: program_memory(7); break;
        case 22221: program_memory(8); break;
        //case 22222: program_memory(9); break;
        default: send_char('?',NORMAL); break;
      }
    }
  }
}
#endif //FEATURE_COMMAND_BUTTONS
#endif

//---------------------------------------------------------------------

#ifdef FEATURE_MEMORIES
byte memory_nonblocking_delay(unsigned long delaytime)
{
  // 2012-04-20 was long starttime = millis();
  unsigned long starttime = millis();

  while ((millis() - starttime) < delaytime) {
    check_paddles();
    #ifdef FEATURE_COMMAND_BUTTONS
    if (((dit_buffer) || (dah_buffer) || (analogbuttonread(0))) && (machine_mode != BEACON)) {   // exit if the paddle or button0 was hit
    #else
    if (((dit_buffer) || (dah_buffer)) && (machine_mode != BEACON)) {   // exit if the paddle or button0 was hit
    #endif
      dit_buffer = 0;
      dah_buffer = 0;
      #ifdef FEATURE_COMMAND_BUTTONS
      while (analogbuttonread(0)) {}
      #endif
      return 1;
    }
  }
  return 0;
}

#endif

//---------------------------------------------------------------------
void check_button0()
{
  #ifdef FEATURE_COMMAND_BUTTONS
  if (analogbuttonread(0)) {button0_buffer = 1;}
  #endif
}

//---------------------------------------------------------------------

#ifdef FEATURE_MEMORIES
void play_memory(byte memory_number)
{

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
  
  #ifdef FEATURE_MEMORY_MACROS
  byte eeprom_byte_read2;
  int z;
  byte input_error;
  byte delay_result = 0;
  int int_from_macro;
  #endif //FEATURE_MEMORY_MACROS

  button0_buffer = 0;

//  #ifdef DEBUG_MEMORYCHECK
//  memorycheck();
//  #endif

  if (machine_mode == NORMAL) {
    #ifdef FEATURE_SERIAL
    #ifdef FEATURE_WINKEY_EMULATION
    if (serial_mode != SERIAL_WINKEY_EMULATION) {
      Serial.println();
    }
    #else
    Serial.println();
    #endif
    #endif
  }
  
  for (int y = (memory_start(memory_number)); (y < (memory_end(memory_number)+1)); y++) {

    if (machine_mode == NORMAL) {
      check_potentiometer();
      check_button0();
      #ifdef FEATURE_DISPLAY
      service_display();
      #endif
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

        if (eeprom_byte_read != 92) {          // do we have a backslash?
          if (machine_mode == NORMAL) {
            #ifdef FEATURE_SERIAL
            #ifndef FEATURE_WINKEY_EMULATION
            Serial.write(eeprom_byte_read);
            #else  //FEATURE_WINKEY_EMULATION
            if (((serial_mode == SERIAL_WINKEY_EMULATION) && (winkey_paddle_echo_activated) && (winkey_host_open)) || (serial_mode != SERIAL_WINKEY_EMULATION)) {
              Serial.write(eeprom_byte_read);
            }
            #endif //FEATURE_WINKEY_EMULATION
            #endif //FEATURE_SERIAL
            #ifdef FEATURE_DISPLAY
            if (lcd_send_echo) {
              display_scroll_print_char(eeprom_byte_read); 
              service_display();
            }
            #endif            
          }

          if (prosign_flag) {
            send_char(eeprom_byte_read,OMIT_LETTERSPACE);
            prosign_flag = 0;
          } else {
            send_char(eeprom_byte_read,NORMAL);         // no - play the character
          }
        } else {                               // yes - we have a backslash command ("macro")
          y++;                                 // get the next memory byte
          #ifdef FEATURE_MEMORY_MACROS
          if (y < (memory_end(memory_number)+1)) {
            eeprom_byte_read = EEPROM.read(y);            // memory macros (backslash commands)
            switch (eeprom_byte_read) {

              case 49:                         // 1 - jump to memory 1
                if (number_of_memories > 0) {
                  memory_number = 0;
                  y = ((memory_start(memory_number)) - 1);
                  if (machine_mode == NORMAL) {
                    Serial.println();
                  }
                }
                break;

              case 50:                         // 2 - jump to memory 2
                if (number_of_memories > 1) {
                  memory_number = 1;
                  y = ((memory_start(memory_number)) - 1);
                  if (machine_mode == NORMAL) {
                    Serial.println();
                  }
                }
                break;

              case 51:                         // 3 - jump to memory 3
                if (number_of_memories > 2) {
                  memory_number = 2;
                  y = ((memory_start(memory_number)) - 1);
                  if (machine_mode == NORMAL) {
                    Serial.println();
                  }
                }
                break;

              case 52:                         // 4 - jump to memory 4
                if (number_of_memories > 3) {
                  memory_number = 3;
                  y = ((memory_start(memory_number)) - 1);
                  if (machine_mode == NORMAL) {
                    Serial.println();
                  }
                }
                break;

              case 53:                         // 5 - jump to memory 5
                if (number_of_memories > 4) {
                  memory_number = 4;
                  y = ((memory_start(memory_number)) - 1);
                  if (machine_mode == NORMAL) {
                    Serial.println();
                  }
                }
                break;

              case 54:                         // 6 - jump to memory 6
                if (number_of_memories > 5) {
                  memory_number = 5;
                  y = ((memory_start(memory_number)) - 1);
                  if (machine_mode == NORMAL) {
                    Serial.println();
                  }
                }
                break;

              case 55:                         // 7 - jump to memory 7
                if (number_of_memories > 6) {
                  memory_number = 6;
                  y = ((memory_start(memory_number)) - 1);
                  if (machine_mode == NORMAL) {
                    Serial.println();
                  }
                }
                break;

              case 56:                         // 8 - jump to memory 8
                if (number_of_memories > 7) {
                  memory_number = 7;
                  y = ((memory_start(memory_number)) - 1);
                  if (machine_mode == NORMAL) {
                    Serial.println();
                  }
                }
                break;

              case 57:                         // 9 - jump to memory 9
                if (number_of_memories > 8) {
                  memory_number = 8;
                  y = ((memory_start(memory_number)) - 1);
                  if (machine_mode == NORMAL) {
                    Serial.println();
                  }
                }
                break;

              case 67:                       // C - play serial number with cut numbers T and N, then increment
                  serial_number_string = String(serial_number, DEC);
                  if (serial_number_string.length() < 3 ) {
                    send_char('T',NORMAL);
                  }
                  if (serial_number_string.length() == 1) {
                    send_char('T',NORMAL);
                  }
                  for (unsigned int a = 0; a < serial_number_string.length(); a++)  {
                    if (serial_number_string[a] == '0') {
                      send_char('T',NORMAL);
                    } else {
                     if (serial_number_string[a] == '9') {
                       send_char('N',NORMAL);
                     } else {
                       send_char(serial_number_string[a],NORMAL);
                     }
                    }
                  }
                  serial_number++;
                break;

              case 68:                      // D - delay for ### seconds
                int_from_macro = 0;
                z = 100;
                input_error = 0;
                for (int x = 1; x < 4; x ++) {
                  y++;
                  if (y < (memory_end(memory_number)+1)) {
                    eeprom_byte_read2 = EEPROM.read(y);
                    if ((eeprom_byte_read2 > 47) && (eeprom_byte_read2 < 58)) {    // ascii 48-57 = "0" - "9")
                      int_from_macro = int_from_macro + ((eeprom_byte_read2 - 48) * z);
                      z = z / 10;
                    } else {
                      x = 4;           // error - exit
                      input_error = 1;
                      y--;             // go back one so we can at least play the errant character
                    }
                  } else {
                    x = 4;
                    input_error = 1;
                  }
                }
                if (input_error != 1) {   // do the delay
                  delay_result = memory_nonblocking_delay(int_from_macro*1000);
                }
                if (delay_result) {   // if a paddle or button0 was hit during the delay, exit
                  return;
                }
                break;  // case 68

              case 69:                       // E - play serial number, then increment
                  serial_number_string = String(serial_number, DEC);
                  for (unsigned int a = 0; a < serial_number_string.length(); a++)  {
                    send_char(serial_number_string[a],NORMAL);
                  }
                  serial_number++;
                break;

              case 70:                       // F - change sidetone frequency
                int_from_macro = 0;
                z = 1000;
                input_error = 0;
                for (int x = 1; x < 5; x ++) {
                  y++;
                  if (y < (memory_end(memory_number)+1)) {
                    eeprom_byte_read2 = EEPROM.read(y);
                    if ((eeprom_byte_read2 > 47) && (eeprom_byte_read2 < 58)) {    // ascii 48-57 = "0" - "9")
                      int_from_macro = int_from_macro + ((eeprom_byte_read2 - 48) * z);
                      z = z / 10;
                    } else {
                      x = 5;           // error - exit
                      input_error = 1;
                      y--;             // go back one so we can at least play the errant character
                    }
                  }  else {
                    x = 4;
                    input_error = 1;
                  }
                }
                if ((input_error != 1) && (int_from_macro > SIDETONE_HZ_LOW_LIMIT) && (int_from_macro < SIDETONE_HZ_HIGH_LIMIT)) {
                  hz_sidetone = int_from_macro;
                }
                break;

              case 78:                       // N - decrement serial number (do not play)
                serial_number--;
                break;

              case 43:                       // + - Prosign
                prosign_flag = 1;
                break;

              case 81:                       // Q - QRSS mode and set dit length to ##
                int_from_macro = 0;
                z = 10;
                input_error = 0;
                for (int x = 1; x < 3; x ++) {
                  y++;
                  if (y < (memory_end(memory_number)+1)) {
                    eeprom_byte_read2 = EEPROM.read(y);
                    if ((eeprom_byte_read2 > 47) && (eeprom_byte_read2 < 58)) {    // ascii 48-57 = "0" - "9")
                      int_from_macro = int_from_macro + ((eeprom_byte_read2 - 48) * z);
                      z = z / 10;
                    } else {
                      x = 4;           // error - exit
                      input_error = 1;
                      y--;             // go back one so we can at least play the errant character
                    }
                  } else {
                    x = 4;
                    input_error = 1;
                  }
                }
                if (input_error == 0) {
                  speed_mode = SPEED_QRSS;
                  qrss_dit_length =  int_from_macro;
                  //calculate_element_length();
                }
              break;  //case 81

              case 82:                       // R - regular speed mode
                speed_mode = SPEED_NORMAL;
                //calculate_element_length();
              break;

              case 84:                      // T - transmit for ### seconds
                int_from_macro = 0;
                z = 100;
                input_error = 0;
                for (int x = 1; x < 4; x ++) {
                  y++;
                  if (y < (memory_end(memory_number)+1)) {
                    eeprom_byte_read2 = EEPROM.read(y);
                    if ((eeprom_byte_read2 > 47) && (eeprom_byte_read2 < 58)) {    // ascii 48-57 = "0" - "9")
                      int_from_macro = int_from_macro + ((eeprom_byte_read2 - 48) * z);
                      z = z / 10;
                    } else {
                      x = 4;           // error - exit
                      input_error = 1;
                      y--;             // go back one so we can at least play the errant character
                    }
                  } else {
                    x = 4;
                    input_error = 1;
                  }
                }
                if (input_error != 1) {   // go ahead and transmit
                  tx_and_sidetone_key(1,AUTOMATIC_SENDING);
                  delay_result = memory_nonblocking_delay(int_from_macro*1000);
                  tx_and_sidetone_key(0,AUTOMATIC_SENDING);
                }
                if (delay_result) {   // if a paddle or button0 was hit during the delay, exit
                  return;
                }
                break;  // case 84

              case 87:                      // W - change speed to ### WPM
                int_from_macro = 0;
                z = 100;
                input_error = 0;
                for (int x = 1; x < 4; x ++) {
                  y++;
                  if (y < (memory_end(memory_number)+1)) {
                    eeprom_byte_read2 = EEPROM.read(y);
                    if ((eeprom_byte_read2 > 47) && (eeprom_byte_read2 < 58)) {    // ascii 48-57 = "0" - "9")
                      int_from_macro = int_from_macro + ((eeprom_byte_read2 - 48) * z);
                      z = z / 10;
                    } else {
                      x = 4;           // error - exit
                      input_error = 1;
                      y--;             // go back one so we can at least play the errant character
                    }
                  }  else {
                    x = 4;
                    input_error = 1;
                  }
                }
                if (input_error != 1) {
                  speed_mode = SPEED_NORMAL;
                  speed_set(int_from_macro);
                }
                break;  // case 87

                case 89:                // Y - Relative WPM change (positive)
                  y++;
                  if ((y < (memory_end(memory_number)+1)) && (speed_mode == SPEED_NORMAL)) {
                    eeprom_byte_read2 = EEPROM.read(y);
                    if ((eeprom_byte_read2 > 47) && (eeprom_byte_read2 < 58)) {    // ascii 48-57 = "0" - "9")
                      speed_set(wpm + eeprom_byte_read2 - 48);
                    } else {
                      y--;             // go back one so we can at least play the errant character
                    }
                  } else {
                  }
                  break; // case 89

                case 90:                // Z - Relative WPM change (positive)
                  y++;
                  if ((y < (memory_end(memory_number)+1)) && (speed_mode == SPEED_NORMAL)) {
                    eeprom_byte_read2 = EEPROM.read(y);
                    if ((eeprom_byte_read2 > 47) && (eeprom_byte_read2 < 58)) {    // ascii 48-57 = "0" - "9")
                      speed_set(eeprom_byte_read2 - 48);
                    } else {
                      y--;             // go back one so we can at least play the errant character
                    }
                  } else {
                  }
                  break; // case 90

            }

          }
          #endif //FEATURE_MEMORY_MACROS
        }
        if (machine_mode != BEACON) {
          if ((dit_buffer) || (dah_buffer) || (button0_buffer)) {   // exit if the paddle or button0 was hit
            dit_buffer = 0;
            dah_buffer = 0;
            button0_buffer = 0;
            repeat_memory = 255;
            #ifdef FEATURE_COMMAND_BUTTONS
            while (analogbuttonread(0)) {}
            #endif  
            return;
          }
        }

      } else {
        if (y == (memory_start(memory_number))) {      // memory is totally empty - do a boop
          repeat_memory = 255;
          #ifdef FEATURE_DISPLAY
          lcd_center_print_timed("Memory empty", 0, default_display_msg_delay);
          #else
          boop();
          #endif
        }
        return;
      }
    } else {
      if (pause_sending_buffer == 0) {
        y = (memory_end(memory_number)+1);   // we got a play_memory_prempt flag, exit out
      } else {
        y--;  // we're in a pause mode, so sit and spin awhile
      }
    }

    last_memory_repeat_time = millis();
    #ifdef DEBUG_PLAY_MEMORY
    Serial.println(F("\nplay_memory: reset last_memory_repeat_time"));
    #endif

  }

}
#endif

//---------------------------------------------------------------------

#ifdef FEATURE_MEMORIES
void program_memory(int memory_number)
{

  if (memory_number > (number_of_memories-1)) {
    boop();
    return;
  }
  
  #ifdef FEATURE_DISPLAY
  String lcd_print_string;
  lcd_print_string.concat("Pgm Memory ");
  lcd_print_string.concat(memory_number+1);
  lcd_center_print_timed(lcd_print_string, 0, default_display_msg_delay);
  #endif

  send_dit(AUTOMATIC_SENDING);

  byte paddle_hit = 0;
  byte loop1 = 1;
  byte loop2 = 1;
  unsigned long last_element_time = 0;
  int memory_location_index = 0;
  long cwchar = 0;
  byte space_count = 0;

  dit_buffer = 0;
  dah_buffer = 0;
  #ifdef FEATURE_COMMAND_BUTTONS
  while ((digitalRead(paddle_left) == HIGH) && (digitalRead(paddle_right) == HIGH) && (!analogbuttonread(0))) { }  // loop until user starts sending or hits the button
  #endif

  while (loop2) {

    #ifdef DEBUG_MEMORY_WRITE
    Serial.print(F("program_memory: entering loop2\r\n\r"));
    #endif

    cwchar = 0;
    paddle_hit = 0;
    loop1 = 1;

    while (loop1) {
       check_paddles();
       if (dit_buffer) {
         send_dit(MANUAL_SENDING);
         dit_buffer = 0;
         paddle_hit = 1;
         cwchar = (cwchar * 10) + 1;
         last_element_time = millis();
         #ifdef DEBUG_MEMORY_WRITE
         Serial.write(".");
         #endif
       }
       if (dah_buffer) {
         send_dah(MANUAL_SENDING);
         dah_buffer = 0;
         paddle_hit = 1;
         cwchar = (cwchar * 10) + 2;
         last_element_time = millis();
         #ifdef DEBUG_MEMORY_WRITE
         Serial.write("_");
         #endif
       }
       if ((paddle_hit) && (millis() > (last_element_time + (float(600/wpm) * length_letterspace)))) {   // this character is over
         loop1 = 0;
       }

       if ((paddle_hit == 0) && (millis() > (last_element_time + ((float(1200/wpm) * length_wordspace)))) && (space_count < program_memory_limit_consec_spaces)) {   // we have a space
         loop1 = 0;
         cwchar = 9;
         space_count++;
       }

       #ifdef FEATURE_COMMAND_BUTTONS
       while (analogbuttonread(0)) {    // hit the button to get out of command mode if no paddle was hit
         loop1 = 0;
         loop2 = 0;
       }
       #endif
    }  //loop1

    if (cwchar != 9) {
      space_count = 0;
    }

    // write the character to memory
    if (cwchar > 0) {

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
      Serial.print(F("   ascii to write:"));
      Serial.println(convert_cw_number_to_ascii(cwchar));
      #endif

      EEPROM.write((memory_start(memory_number)+memory_location_index),convert_cw_number_to_ascii(cwchar));
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
    if (EEPROM.read((memory_start(memory_number) + x)) == 32) {
      EEPROM.write((memory_start(memory_number) + x),255);
    } else {
      x = 0;
    }
  }
  #endif

  #ifdef FEATURE_DISPLAY
  lcd_center_print_timed("Done", 0, default_display_msg_delay);
  #endif

  play_memory(memory_number);

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
  
  #ifdef FEATURE_CW_DECODER
  pinMode (cw_decoder_pin, INPUT);
  digitalWrite (cw_decoder_pin, HIGH);  
  #endif //FEATURE_CW_DECODER
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
  #ifdef FEATURE_MEMORY_MACROS
  Serial.println(F("FEATURE_MEMORY_MACROS"));
  #endif
  #ifdef FEATURE_WINKEY_EMULATION
  Serial.println(F("FEATURE_WINKEY_EMULATION"));
  #endif
  #ifdef OPTION_WINKEY_2_SUPPORT
  Serial.println(F("OPTION_WINKEY_2_SUPPORT"));
  #endif
  #ifdef FEATURE_BEACON
  Serial.println(F("FEATURE_BEACON"));
  #endif
  #ifdef FEATURE_CALLSIGN_RECEIVE_PRACTICE
  Serial.println(F("FEATURE_CALLSIGN_RECEIVE_PRACTICE"));
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
  #ifdef FEATURE_DL2SBA_BANKSWITCH
  Serial.println(F("FEATURE_DL2SBA_BANKSWITCH"));
  #endif
  #ifdef FEATURE_COMMAND_BUTTONS
  Serial.println(F("FEATURE_COMMAND_BUTTONS"));
  #endif
  #ifdef FEATURE_LCD_4BIT
  Serial.println(F("FEATURE_LCD_4BIT"));
  #endif  
  Serial.println(F("setup: exiting, going into loop"));
}
#endif //DEBUG_STARTUP
 
//--------------------------------------------------------------------- 
  
#define CW_DECODER_SCREEN_COLUMNS 40        // word wrap at this many columns
#define CW_DECODER_SPACE_PRINT_THRESH 4.5   // print space if no tone for this many element lengths
#define CW_DECODER_SPACE_DECODE_THRESH 2.0  // invoke character decode if no tone for this many element lengths
#define CW_DECODER_NOISE_FILTER 20          // ignore elements shorter than this (mS)
 
#ifdef FEATURE_CW_DECODER
void service_cw_decoder() {

  static unsigned long last_transition_time = 0;
  static unsigned long last_decode_time = 0;
  static byte last_state = HIGH;
  static int decode_elements[16];                  // this stores received element lengths in mS (positive = tone, minus = no tone)
  static byte decode_element_pointer = 0;
  static float decode_element_tone_average = 0;
  static float decode_element_no_tone_average = 0;
  byte decode_it_flag = 0;
  byte cd_decoder_pin_state = digitalRead(cw_decoder_pin);
  int element_duration = 0;
  static float decoder_wpm = wpm;
  long decode_character = 0;
  static byte space_sent = 0;
  #ifdef FEATURE_COMMAND_LINE_INTERFACE
  static byte screen_column = 0;
  static int last_printed_decoder_wpm = 0;
  #endif
 
 
  if  (last_transition_time == 0) { 
    if (cd_decoder_pin_state == LOW) {  // is this our first tone?
      last_transition_time = millis();
      last_state = LOW;
      #ifdef DEBUG_CW_DECODER
      tone(sidetone_line, 1500);
      #endif
    } else {
      if ((last_decode_time > 0) && (!space_sent) && ((millis() - last_decode_time) > ((1200/decoder_wpm)*CW_DECODER_SPACE_PRINT_THRESH))) { // should we send a space?
         #ifdef FEATURE_SERIAL
         #ifdef FEATURE_COMMAND_LINE_INTERFACE
         Serial.write(32);
         screen_column++;
         #endif //FEATURE_COMMAND_LINE_INTERFACE
         #endif //FEATURE_SERIAL
         space_sent = 1;
      }
    }
  } else {
    if (cd_decoder_pin_state != last_state) {
      // we have a transition 
      #ifdef DEBUG_CW_DECODER
      //Serial.println(F("service_cw_decoder: transition"));
      if (cd_decoder_pin_state == LOW) {
        tone(sidetone_line, 1500);
      } else {
        noTone(sidetone_line);
      } 
      #endif
      element_duration = millis() - last_transition_time;
      if (element_duration > CW_DECODER_NOISE_FILTER) {                                    // filter out noise
        if (cd_decoder_pin_state == LOW) {  // we have a tone
          decode_elements[decode_element_pointer] = (-1 * element_duration);  // the last element was a space, so make it negative
          if (decode_element_no_tone_average == 0) {
            decode_element_no_tone_average = element_duration;
          } else {
            decode_element_no_tone_average = (element_duration + decode_element_no_tone_average) / 2;
          }
          decode_element_pointer++;
          last_state = LOW;
        } else {  // we have no tone
          decode_elements[decode_element_pointer] = element_duration;  // the last element was a tone, so make it positive        
          if (decode_element_tone_average == 0) {
            decode_element_tone_average = element_duration;
          } else {
            decode_element_tone_average = (element_duration + decode_element_tone_average) / 2;
          }
          last_state = HIGH;
          decode_element_pointer++;
        }
        last_transition_time = millis();
        if (decode_element_pointer == 16) { decode_it_flag = 1; }  // if we've filled up the array, go ahead and decode it
      }
      
      
    } else {
      // no transition
      element_duration = millis() - last_transition_time;
      if (last_state == HIGH)  {
        // we're still high (no tone) - have we reached character space yet?        
        //if ((element_duration > (decode_element_no_tone_average * 2.5)) || (element_duration > (decode_element_tone_average * 2.5))) {
        if (element_duration > (float(1200/decoder_wpm)*CW_DECODER_SPACE_DECODE_THRESH)) {
          decode_it_flag = 1;
        }
      } else {
        // have we had tone for an outrageous amount of time?
       
      }
    }
   }
 
 
 
 
  if (decode_it_flag) {                      // are we ready to decode the element array?
    #ifdef DEBUG_CW_DECODER
    noTone(sidetone_line);  
    #endif
    
    // adjust the decoder wpm based on what we got
    if (decode_element_no_tone_average > 0) {
      if (abs((1200/decode_element_no_tone_average) - decoder_wpm) < 5) {
        decoder_wpm = (decoder_wpm + (1200/decode_element_no_tone_average))/2;
      } else {
        if (abs((1200/decode_element_no_tone_average) - decoder_wpm) < 10) {
          decoder_wpm = (decoder_wpm + decoder_wpm + (1200/decode_element_no_tone_average))/3;
        } else {
          if (abs((1200/decode_element_no_tone_average) - decoder_wpm) < 20) {
            decoder_wpm = (decoder_wpm + decoder_wpm + decoder_wpm + (1200/decode_element_no_tone_average))/4;    
          }      
        }
      }
    }
    
    #ifdef DEBUG_CW_DECODER_WPM
    if (abs(decoder_wpm - last_printed_decoder_wpm) > 0.9) {
      Serial.print("<");
      Serial.print(int(decoder_wpm));
      Serial.print(">");
      last_printed_decoder_wpm = decoder_wpm;
    }
    #endif //DEBUG_CW_DECODER_WPM
    
    for (byte x = 0;x < decode_element_pointer; x++) {
      if (decode_elements[x] > 0) {  // is this a tone element?
        if (decode_element_no_tone_average > 0) {
        // we have spaces to time from 
          if ((decode_elements[x]/decode_element_no_tone_average) < 2.1) {
            decode_character = (decode_character * 10) + 1; // we have a dit
          } else {
            decode_character = (decode_character * 10) + 2; // we have a dah
          }
        } else {
          // we have no spaces to time from, use the current wpm
          if ((decode_elements[x]/(1200/decoder_wpm)) < 1.3) {
            decode_character = (decode_character * 10) + 1; // we have a dit
          } else {
            decode_character = (decode_character * 10) + 2; // we have a dah
          }          
        }
      }
      #ifdef DEBUG_CW_DECODER
      Serial.print(F("service_cw_decoder: decode_elements["));
      Serial.print(x);
      Serial.print(F("]: "));
      Serial.println(decode_elements[x]);
      #endif //DEBUG_CW_DECODER
    }
    #ifdef DEBUG_CW_DECODER
    Serial.print(F("service_cw_decoder: decode_element_tone_average: "));
    Serial.println(decode_element_tone_average);
    Serial.print(F("service_cw_decoder: decode_element_no_tone_average: "));
    Serial.println(decode_element_no_tone_average);
    Serial.print(F("service_cw_decoder: decode_element_no_tone_average wpm: "));
    Serial.println(1200/decode_element_no_tone_average);
    Serial.print(F("service_cw_decoder: decoder_wpm: "));
    Serial.println(decoder_wpm);
    Serial.print(F("service_cw_decoder: decode_character: "));
    Serial.println(decode_character);
    #endif //DEBUG_CW_DECODER
    #ifdef FEATURE_SERIAL
    #ifdef FEATURE_COMMAND_LINE_INTERFACE
    Serial.write(convert_cw_number_to_ascii(decode_character));
    screen_column++;
    #endif //FEATURE_COMMAND_LINE_INTERFACE
    #endif //FEATURE_SERIAL
    #ifdef FEATURE_DISPLAY
    display_scroll_print_char(convert_cw_number_to_ascii(decode_character));
    #endif //FEATURE_DISPLAY
    // reinitialize everything
    last_transition_time = 0;
    last_decode_time = millis();
    decode_element_pointer = 0; 
    decode_element_tone_average = 0;
    decode_element_no_tone_average = 0;
    space_sent = 0;
  }
  
  #ifdef FEATURE_SERIAL
  #ifdef FEATURE_COMMAND_LINE_INTERFACE
  if (screen_column > CW_DECODER_SCREEN_COLUMNS) {
    Serial.println();
    screen_column = 0;
  }
  #endif //FEATURE_COMMAND_LINE_INTERFACE
  #endif //FEATURE_SERIAL
  
}

#endif //FEATURE_CW_DECODER
