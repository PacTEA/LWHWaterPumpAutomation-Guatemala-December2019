#include "definitions.h"

void setup() {
  // put your setup code here, to run once:
  pinMode (tank1Low, INPUT_PULLUP);
  pinMode (tank1High, INPUT_PULLUP);
  pinMode (cistern, INPUT_PULLUP);
  pinMode (pump, OUTPUT);
  pinMode (error, OUTPUT);
  pinMode (eShutoff,INPUT_PULLUP);
  pinMode (manualOveride,INPUT_PULLUP);
}

void loop() {
  // put your main code here, to run repeatedly:

  /*TODO:Emergency override overrules all tanks and shuts them all down or some down as needed
    potentially involves the cleaning part of it
    Manual override is a sublevel to that and allow pumps to be used or access regardless of states
    it can turn the pump on (maybe) without the involvement of the automated system
  */
  
  
  
  //how the cistern fxns and interacts with the other pumps based on water levels
  //currently it runs when the cistern has water otherwise it does not run
  if (digitalRead(cistern) == closed) {
    if ((digitalRead(tank1Low) ==  open) && (digitalRead(tank1High) == open)) {
      digitalWrite(pump, HIGH);
      // When Low level float sensor is down, turn pump on
    }
    else if ((digitalRead(tank1High) == closed) && (digitalRead(tank1Low) == closed)) {
      digitalWrite(pump, LOW);
      //when high level float sensor is up, turn pump off
    }
    else if ((digitalRead(tank1Low) == open) && (digitalRead(tank1High) == closed)) {
      digitalWrite(pump, LOW); //pump off
      digitalWrite(error, HIGH); //light on
      //if low level float sensor is down and high level float sensor is up
      //pump turns off and error light is turned on
    }
 }

}
