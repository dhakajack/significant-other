/* 
* Test of reading multiple switches arranged as a resistive divider.
*
* Analog input reads value of point between two resistors; 15k to 5+
* and several values to ground. These values were picked such that the
* maximum errors associated with 10% variance in each resistor would 
* still not overlap. The cutoffs are the values between the upper range 
* of one worst case high value for one pair and the worst case low value
* for the next pair. 
*
* See jjw1-169
* Online spreadsheet "resistive_divider_values_significant_other"
*/

const int switchPin = 0; // read analog pin zero
const int debounceTime = 20; //milliseconds for debounce to stabilize

void setup() {
  
  Serial.begin(9600);
  
}

void loop() {
 int btn = getBtn();
 if (btn != 0) {
   Serial.print("Got button ");
   Serial.println(btn);
   delay(500);  
 } 
}

int getBtn() {

  int val = 0;
  int pressed = 0;
  
  val = analogRead(switchPin);
  if (val < 1000) {
  //  if (val > 714) return 1;
  //  else if (val > 582) return 2;
  //  else if (val > 433) return 3;
  //  else return 4;
  return val;
  }
  else return 0;

}



