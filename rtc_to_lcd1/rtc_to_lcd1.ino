/******************************************************************************

Test of rtc and lcd on same i2c bus

SETUP:    I2C-RTC => Arduino

          PIN1 => A5, PIN2 => A4, PIN3 => ground, PIN6 => +5V
          

Note:     The program is written for address 0xD0 (Arduino address 0x68 -- since
          Arduino addresses 7 bits rather than 8; i.e., D0h/2= 68h.

          This program was tested using Arduino Demilanove
          
          I2C-LCD (ada 181); HD4480-based LCD: 16x2 display with LED backlight
          Tinsharp LCM module TC1602a-01T
          
          Connections from MCP2300 port expander to Arduino:
          
          MCP  LCD    Arduino/Board
          9    Pin1 => gnd
          18   Pin2 => 5V+
          --   Pin3 => dimmer wiper input (0-5V)
          11   Pin4 => RS D7
          --   Pin5 => RW gnd
          12   Pin6 => EN D8
          --   Pin7 => nc
          --   Pin8 => nc
          --   Pin9 => nc
          --   Pin10 => nc
          13   Pin11 => D9
          14   Pin12 => D10
          15   Pin13 => D11
          16   Pin14 => D12
          --   Pin15 => 5V
          17   2n2222 base; Pin15 to collector; emitter to ground
          1    SCL; pull down with 4.7k resistor; tie to rtc pin 1 (to arduino A5)
          2    SDA; pull down with 4.7k resistor; tie to rtc pin 2 (to arduino A4)
          3    to ground (for address 00h)
          4    "
          5    "
          6    (reset, active low) to gnd
          7    nc
          8    nc
          10   nc (extra data port - not used at present)
 
This builds on the I2C_RTC and HelloWorld_i2c_jjw sketches
JJW 1-1169

Document: DS1340 datasheet
          MCP2300
          
Updated:  November 3, 2012

E-mail:  dhakajack@gmail.com

RTC code originated at Gravitech
(C) Copyright 2008 All Rights Reserved

*******************************************************************************/

#include <Wire.h> 
#include <LiquidCrystal.h>

#define Binary 0

#define Hex 1



/*******************************************************************************

                      Function Prototype

*******************************************************************************/  

unsigned int SerialNumRead (byte);

void SetTime();

void DisplayTime();

void TimeOnLCD();



/*******************************************************************************

                    Global variables

*******************************************************************************/ 

const int clock_address = 0x68;  // I2C write address for clock

byte    Second;     // Store second value

byte    Minute;     // Store minute value

byte    Hour;       // Store hour value

byte    Day;        // Store day value

byte    Date;       // Store date value

byte    Month;      // Store month value

byte    Year;       // Store year value

LiquidCrystal  lcd(0); // default address for LCD



/*******************************************************************************

                      Setup

*******************************************************************************/ 

void setup() 

{ 

  Serial.begin(9600);
  
  Wire.begin();        // join i2c bus (address optional for master) 
  
  lcd.begin(16,2);
  lcd.setBacklight(HIGH);
  lcd.print("RTC Clock Test");
  
  delay(1000);

} 

 

/*******************************************************************************

                      Main Loop

*******************************************************************************/  

void loop() 

{ 

  boolean Readtime;       // Set/Read time flag

  unsigned int Incoming;  // Incoming serial data



  // Display prompt

  Serial.println("What would you like to do?");

  Serial.println("(0) To set the current time.");

  Serial.println("(1) To display the current time.");

  Serial.print("Enter 0 or 1: ");

   

  Incoming = SerialNumRead (Binary);  // Get input command

  Serial.println(Incoming, DEC);      // Echo the value

  Serial.println();

  

  if (Incoming == 0)                  // Process input command

    SetTime();

  else if (Incoming == 1)

    DisplayTime();

  TimeOnLCD();

  delay (1000);

}



/*******************************************************************************

       Read a input number from the Serial Monitor ASCII string

       Return: A binary number or hex number

*******************************************************************************/ 

unsigned int SerialNumRead (byte Type)

{

  unsigned int Number = 0;       // Serial receive number

  unsigned int digit = 1;        // Digit

  byte  i = 0, j, k=0, n;        // Counter

  byte  ReceiveBuf [5];          // for incoming serial data

  

  while (Serial.available() <= 0);

  

  while (Serial.available() > 0)  // Get serial input

  {

    // read the incoming byte:

    ReceiveBuf[i] = Serial.read();

    i++;

    delay(10);

  }

  

  for (j=i; j>0; j--)

  {

    digit = 1;

    

    for (n=0; n<k; n++)          // This act as pow() with base = 10

    {

      if (Type == Binary)

        digit = 10 * digit;

      else if (Type == Hex)

        digit = 16 * digit;

    }

        

    ReceiveBuf[j-1] = ReceiveBuf[j-1] - 0x30;    // Change ASCII to BIN

    Number = Number + (ReceiveBuf[j-1] * digit); // Calcluate the number

    k++;

  }

  return (Number);    

}



/*******************************************************************************

                   Set time function

*******************************************************************************/ 

void SetTime()

{

  Serial.print("Enter hours (00-23): ");

  Hour = (byte) SerialNumRead (Hex);

  Serial.println(Hour, HEX);        // Echo the value

  Hour = Hour & 0x3F;               // Disable century

  Serial.print("Enter minutes (00-59): ");

  Minute = (byte) SerialNumRead (Hex);

  Serial.println(Minute, HEX);      // Echo the value

  Serial.print("Enter seconds (00-59): ");

  Second = (byte) SerialNumRead (Hex);

  Serial.println(Second, HEX);      // Echo the value

  Second = Second & 0x7F;           // Enable oscillator

  Serial.print("Enter day (01-07): ");

  Day = (byte) SerialNumRead (Hex);

  Serial.println(Day, HEX);         // Echo the value

  Serial.print("Enter date (01-31): ");

  Date = (byte) SerialNumRead (Hex);

  Serial.println(Date, HEX);        // Echo the value

  Serial.print("Enter month (01-12): ");

  Month = (byte) SerialNumRead (Hex);

  Serial.println(Month, HEX);       // Echo the value

  Serial.print("Enter year (00-99): ");

  Year = (byte) SerialNumRead (Hex);

  Serial.println(Year, HEX);        // Echo the value



  Wire.beginTransmission(clock_address);

  Wire.write(0);

  Wire.write(Second);

  Wire.write(Minute);

  Wire.write(Hour);

  Wire.write(Day);

  Wire.write(Date);

  Wire.write(Month);

  Wire.write(Year);

  Wire.endTransmission();

  //I2COUT SDA, I2C_WR, [0,Second,Minute,Hour,Day,Date,Month,Year]

  Serial.println();

  Serial.println ("The current time has been successfully set!");

}



/*******************************************************************************

                   Display time function

*******************************************************************************/ 

void DisplayTime()

{

  char tempchar [7];

  byte i = 0;

  Wire.beginTransmission(clock_address);

  Wire.write(0);

  Wire.endTransmission();

  

  Wire.requestFrom(clock_address, 7);

  

  while(Wire.available())          // Checkf for data from slave

  { 

    tempchar [i] = Wire.read(); // receive a byte as character 

    i++;

  } 

  Second = tempchar [0];

  Minute = tempchar [1];

  Hour   = tempchar [2];

  Day    = tempchar [3];

  Date   = tempchar [4];

  Month  = tempchar [5];

  Year   = tempchar [6];

  

  // Display time

  Serial.print("The current time is ");

  Serial.print(Month, HEX);

  Serial.print("/");

  Serial.print(Date, HEX);

  Serial.print("/20");

  if (Year<10)

    Serial.print("0");

  Serial.print(Year, HEX);

  Serial.print("    ");

  Serial.print(Hour, HEX);

  Serial.print(":");

  Serial.print(Minute, HEX);

  Serial.print(".");

  Serial.println(Second, HEX);

}

void TimeOnLCD() {
 
 lcd.setCursor(0,1);
 lcd.print(Hour, HEX);
 lcd.print(":");
 lcd.print(Minute,HEX);
 
}
