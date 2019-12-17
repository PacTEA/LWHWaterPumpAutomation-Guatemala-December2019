#include "definitions.h"
//error is visual display for the pump to shut off
//LOW means off for LED
//HIGH means on for LED

void setup() {
  // put your setup code here, to run once:
  pinMode (tank1Low, INPUT_PULLUP);
  pinMode (tank1High, INPUT_PULLUP);
  pinMode (pump, OUTPUT);
  pinMode (error, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  if ((digitalRead(tank1Low) ==  open) && (digitalRead(tank1High) == open)) {
    digitalWrite(pump, HIGH);
    // When Low level float sensor is down, turn pump on
  }
  else if((digitalRead(tank1High) == closed)&&(digitalRead(tank1Low) == closed)){
    digitalWrite(pump, LOW);
    //when high level float sensor is up, turn pump off
  } 
  else if((digitalRead(tank1Low) == open) && (digitalRead(tank1High) == closed)){
    digitalWrite(pump, LOW); //pump off
    digitalWrite(error, HIGH); //light on
    //if low level float sensor is down and high level float sensor is up
    //pump turns off and error light is turned on
  }
 /* else{
    //blow up
    //pump is turned off and light is thrown 
   digitalWrite(pump,HIGH);
   digitalWrite(error, HIGH);
  }*/
}
