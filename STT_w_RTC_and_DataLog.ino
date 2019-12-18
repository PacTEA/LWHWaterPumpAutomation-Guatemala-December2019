#include <TimeLib.h>
#include <SD.h>
#include <SPI.h>
#include "definitions.h"

//  * - can be taken out of final code
//  t - needs to be reworked


// change this to match your SD shield or module;
// Teensy 3.5 & 3.6 on-board: BUILTIN_SDCARD
//const int chipSelect = BUILTIN_SDCARD;


// Error Flag that tells us whether or not we have recorded the error
int errRecord = 0;
void setup() {


  /**/
  pinMode (tank1Low, INPUT_PULLUP);
  pinMode (tank1High, INPUT_PULLUP);
  pinMode (cistern, INPUT_PULLUP);
  pinMode (pump, OUTPUT);
  pinMode (error, OUTPUT);
  //pinMode (eShutoff,INPUT_PULLUP);
  //pinMode (manualOveride,INPUT_PULLUP);


  // set the Time library to use Teensy 3.0's RTC to keep time
  setSyncProvider(getTeensy3Time);

  Serial.begin(115200);                                             //*
  while (!Serial);  // Wait for Arduino Serial Monitor to open      //*
  delay(100);                                                       //*
  if (timeStatus() != timeSet) {                                    //t
    Serial.println("Unable to sync with the RTC");                  //t
  } else {                                                          //t
    Serial.println("RTC has set the system time");                  //t
  }                                                                 //t

  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");

}

void loop() {
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  if (Serial.available()) {
    time_t t = processSyncMessage();
    if (t != 0) {
      Teensy3Clock.set(t); // set the RTC
      setTime(t);
    }
  }





  /* Single Tank Test code goes here*/

  //how the cistern fxns and interacts with the other pumps based on water levels
  //currently it runs when the cistern has water otherwise it does not run
  if (digitalRead(cistern) == closedSwitch) {
    
    if ((digitalRead(tank1Low) ==  openSwitch) && (digitalRead(tank1High) == openSwitch)) {
      digitalWrite(pump, HIGH);
      // When Low level float sensor is down, turn pump on
    }
    else if ((digitalRead(tank1Low) == openSwitch) && (digitalRead(tank1High) == closedSwitch)) {
      digitalWrite(pump, LOW); //pump off
      digitalWrite(error, HIGH); //light on
      errRecord = 1;// set error flag high
      //if low level float sensor is down and high level float sensor is up
      //pump turns off and error light is turned on
    }
    else {
      //when high level float sensor is up, turn pump off
      digitalWrite(pump, LOW); //pump off
    }
  }
  else {
    digitalWrite(pump, LOW); //pump off
  }



  // if the file is available, write to it:
  if (dataFile && (errRecord == 1)) {
    errRecord = 0;
    //digitalWrite(error, LOW); //light off
    digitalClockDisplay(dataFile);
    dataFile.close();
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }


  //Serial.println(hour());


  delay(1000);
}









/*  code to process time sync messages from the serial port   */
#define TIME_HEADER  "T"   // Header tag for serial time sync message

unsigned long processSyncMessage() {
  unsigned long pctime = 0L;
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013

  if (Serial.find(TIME_HEADER)) {
    pctime = Serial.parseInt();
    return pctime;
    if ( pctime < DEFAULT_TIME) { // check the value is a valid time (greater than Jan 1 2013)
      pctime = 0L; // return 0 to indicate that the time is not valid
    }
  }
  return pctime;
}

// digital clock display of the time
void digitalClockDisplay(File dataFile) {
  dataFile.print(hour());
  printDigits(minute(), dataFile);
  printDigits(second(), dataFile);
  dataFile.print(" ");
  dataFile.print(day());
  dataFile.print(" ");
  dataFile.print(month());
  dataFile.print(" ");
  dataFile.print(year());
  dataFile.println();
}


//Getter function for RTC
time_t getTeensy3Time()
{
  return Teensy3Clock.get();
}

// utility function for digital clock display: prints preceding colon and leading 0
void printDigits(int digits, File dataFile) {
  dataFile.print(":");
  if (digits < 10)
    dataFile.print('0');
  dataFile.print(digits);
}
