//**************************************************************//
//  Name    : modified shiftOutCode, Hello World
//  Author  : Jack
//  based on original by Carlyn Maw,Tom Igoe, David A. Mellis 
//  Date    : 7 October 2012                              
//  Version : 1.0                                             
//  Notes   : Set relays interactively for automatic antenna tuner
//          : as part of significant other project
//****************************************************************

const int MAXROWS = 4;
const int MAXCOLS = 7;
//the shift register takes care of columns - tpic6b595 is current drain
//Pin connected to RCK (Relay clock - latch) of TPIC6B595
int latchPin = 2;
////Pin connected to Serial In of TPIC6B595
int dataPin = 4;
//Pin connected to SRCK of TPIC6B595 (Shift register clock)
int clockPin = 3;
//set up relay "rows" - mosfets controlling current source
int rowEnable[MAXROWS] = {10,11,12,13};

void setup() {
   Serial.begin(9600);
  //set pins to output so you can control the shift register
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  for (int i=0; i < MAXROWS; i++){
    pinMode(rowEnable[i], OUTPUT);
    digitalWrite(rowEnable[i], HIGH);
  }  
 zeroRelays();
}

void loop() {
  // Read input from keyboard (well, from serial device)
  // Doesn't bother checking that number is within range
  // since there's really no downside to trying to set a relay
  // column/row that doesn't exist.
  int selectedColumn;
  int selectedRow;
  Serial.println("Enter column:");
  selectedColumn = readSingleDigit();
  Serial.print("Selected column ");
  Serial.println(selectedColumn);
  Serial.println("Enter row:");
  selectedRow = readSingleDigit();
  Serial.print("Selected row ");
  Serial.println(selectedRow);
  Serial.println();
  Serial.println("Activating relay.");
  fireRelay(selectedColumn, selectedRow);
}

int readSingleDigit(void) {
  //grab a single integer 0-9
  int inputByte = -1;
  while (inputByte == -1) {
    while (Serial.available() <= 0)
      ;
    inputByte = Serial.read() - '0';
    if (inputByte < 0 || inputByte > 9)
      inputByte = -1;
  }
  return inputByte;
}

void zeroRelays() {
  // set capacitors to ungrounded (bypass) - red
  // set inductors to shorted (bypass) - green
  for (int row=0; row < MAXROWS; row +=3) {
    for (int col=0; col < MAXCOLS; col++){
      fireRelay(col, row);
    }
  }
  delay(1000);
}

void fireRelay(int col, int row) {
  //shift in the bit mask for column
  //make the register opaque to hide the shifting activity
  digitalWrite(latchPin, LOW);
  // shift out the bits:
  shiftOut(dataPin, clockPin, MSBFIRST, 1 << col);  
  //take the latch pin high to activate relay coil:
  digitalWrite(latchPin, HIGH);
  //disable flow through the latch and make drain active
  //activate the row by pulling the PNP transistor for that row low
  digitalWrite(rowEnable[row],LOW);
  //give some time for the mechanical relay to engage
  delay(10);
  //disable the row
  digitalWrite(rowEnable[row],HIGH);
}

  
