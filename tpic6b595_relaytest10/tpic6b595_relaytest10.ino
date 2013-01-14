//**************************************************************//
//  Name    : modified shiftOutCode, Hello World
//  Author  : Jack
//  based on original by Carlyn Maw,Tom Igoe, David A. Mellis 
//  Date    : 6 Jan 2013                           
//  Version : 1.0                                             
//  Notes   : Code for using a TPIC6B595 Shift Register          //
//          : to drive a double  latching relay  
//          : notebook 1-158
//          : PNP to drive rows, shift register for columns
//****************************************************************

const int MAXROWS = 4;
const int MAXCOLS = 7;
//the shift register takes care of columns - tpic6b595 is current drain
//Pin connected to RCK (Relay clock - latch) of TPIC6B595
int latchPin = 2;
////Pin connected to Serial In of TPIC6B595
int dataPin = 4 ;
//Pin connected to SRCK of TPIC6B595 (Shift register clock)
int clockPin = 3;

//set up relay "rows" - mosfets controlling current source
int rowEnable[MAXROWS] = {10,11,12,13};

void setup() {
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
  for (int row=0; row < MAXROWS; row++) {
    for (int col=0; col < MAXCOLS; col++) {
      fireRelay(col, row);
      delay(500);   
    }
  }
}

void zeroRelays() {
  for (int row=1; row < MAXROWS; row +=2) {
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

  
