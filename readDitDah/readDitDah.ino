/*
 Read dit-dah using internal pull up resistors
 
 
 
 */
 
const int ditPin = 8;
const int dahPin = 7;

void setup() {
  //start serial connection
  Serial.begin(9600);
  //configure pin2 as an input and enable the internal pull-up resistor
  pinMode(ditPin, INPUT_PULLUP);
  pinMode(dahPin, INPUT_PULLUP); 

}

void loop() {
  //read the pushbutton value into a variable
  int ditVal = digitalRead(ditPin);
  int dahVal = digitalRead(dahPin);
  int paddleVal = 0;
  
 
  if (!ditVal) {
    if (!dahVal) {
       paddleVal = 3;
    }
    else 
      paddleVal = 1;
  }
  else if (!dahVal) {
    paddleVal = 2;
  }
  //print out the value of the pushbutton
  Serial.println(paddleVal);   
  
  // Keep in mind the pullup means the pushbutton's
  // logic is inverted. It goes HIGH when it's open,
  // and low when pressed.
}



