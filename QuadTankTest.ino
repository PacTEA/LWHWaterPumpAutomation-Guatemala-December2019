#include <TimeLib.h>
#include <SD.h>
#include <SPI.h>
#include "definitions.h"

//  * - can be taken out of final code
//  t - needs to be reworked

bool waterRun = false; //state to show when pump has run, currently (false = its has  not)
bool manualPumpRun = false;// state to show when manual pump has run for the day !!!!!!FOR FUTURE WORK!!!!!
bool startPump = false;
bool manualStartPump = false;

int currentDay;
time_t timeStart; //When we start pumping the tank
time_t manualTimeStart; //separate time var to address when manualOverride occurs


void setup() {

  /**/
  pinMode (tank1Low, INPUT_PULLUP);
  pinMode (tank1High, INPUT_PULLUP);
  pinMode (tank1Clean, INPUT_PULLUP);
  pinMode (cistern, INPUT_PULLUP);
  pinMode (pump, OUTPUT);
  pinMode (error, OUTPUT);
  pinMode (eShutoff, INPUT_PULLUP);
  pinMode (manualOveride, INPUT_PULLUP);


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
  // bool tank1pumpRun = (digitalRead(tank1Low) ==  openSwitch) && (digitalRead(tank1High) == openSwitch);
  bool tank1pumpRun = (digitalRead(tank1High) == openSwitch);
  bool tank1pumpError = (digitalRead(tank1Low) == openSwitch) && (digitalRead(tank1High) == closedSwitch);
  bool tank1Cleaning = (digitalRead(tank1Clean) == closedSwitch);
  // Change hard coded times to the defined variables
  bool canRunPump = (((second() >= 10) && (second() < 36)) && waterRun == false);

  //TODO: May consider combining tankCleaning and tankpumpRun

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

  if (digitalRead(eShutoff) != closedSwitch) {
    if (digitalRead(cistern) == closedSwitch) {
      //sends the execution of this fxn for manual override to track time
      manualPumpRun = manualOverrideTime(waterRun);
      // Check to see if it is a different day and reset variables
      manualPumpDayReset();


      if ((digitalRead(manualOveride) != closedSwitch)) {
        // Check to see if ran for an hour
        waterRun = pumptimeRestrict(waterRun);
        // Check to see if it is a different day and reset variables
        pumpDayReset();

        // When Low level float sensor is down, turn pump on
        if (tank1pumpRun && !tank1Cleaning && canRunPump) {
          digitalWrite(pump, HIGH);
          digitalWrite(error, LOW); //light off

          //Data logging once to avoid repetitive messages
          logMessage(dataFile, startPumpMessage);

          if (!startPump) {
            timeStart = now();
            startPump = true;
            currentDay = minute();
          }
        }
        else if (tank1pumpError && !tank1Cleaning) {
          //if low level float sensor is down and high level float sensor is up
          //pump turns off and error light is turned on
          digitalWrite(pump, LOW); //pump off
          digitalWrite(error, HIGH); //light on

          //Data logging once to avoid repetitive messages
          logMessage(dataFile, errorFloatStatus);
        }
        else {
          //when high level float sensor is up, turn pump off
          digitalWrite(pump, LOW); //pump off
          digitalWrite(error, LOW); //light off

          //Data logging once to avoid repetitive messages
          logMessage(dataFile, Neutral);
        }
      }
      else {//Manual Else
        if (manualPumpRun == false) {
          digitalWrite(pump, HIGH);//pump on
          digitalWrite(error, LOW); //light off

          //Data logging once to avoid repetitive messages
          logMessage(dataFile, manualOverideMessage);

          //this addresses the fact that with manual override the pump has ran
          //manually for the day
          if (!manualStartPump) {
            manualTimeStart = now();
            manualStartPump = true;
            currentDay = minute();
          }
        }
        else {
          digitalWrite(pump, LOW);//pump off
          digitalWrite(error, LOW); //light off
          logMessage(dataFile, manualPumpDailyRun);
        }

      }
    }
    else {//Cistern else
      digitalWrite(pump, LOW); //pump off
      digitalWrite(error, LOW); //light off

      //TODO: Have this not happen when initially running
      //potentially set pump off message to be default message
      //Data logging once to avoid repetitive messages
      logMessage(dataFile, cisternMessage);
    }
  }
  else {//eShutoff else
    digitalWrite(pump, LOW); //pump off
    digitalWrite(error, LOW); //light off

    //Data logging once to avoid repetitive messages
    logMessage(dataFile, eShutoffMessage);
  }

  /*
    if (waterRun) {
      //Data logging once to avoid repetitive messages
      logMessage(dataFile, pumpDailyRun);
    }
  */
  if (tank1Cleaning) {
    //Data logging once to avoid repetitive messages
    logMessage(dataFile, cleaningStatus);
  }
  digitalClockDisplay3();
  delay(1000);


  dataFile.close();

}// END OF VOID LOOP()

void pumpDayReset()
{
  if (currentDay != minute()) {
    startPump = false;
    waterRun = false;
  }
}

void manualPumpDayReset()
{
  if (currentDay != minute()) {
    manualStartPump = false;
    waterRun = false;
  }
}

//once the pump has ran for the allotted hour we switch states
//in order to shut off the pump
bool pumptimeRestrict(bool restriction)
{
  bool itRan = restriction;
  if ((now() >= (timeStart + 15)) && (currentDay == minute())) {
    //TODO
    // Change hard coded times to the defined variables

    itRan = true;

    return itRan;
  }
  else {
    return itRan;
  }
}

bool manualOverrideTime(bool manualTimedShutoff) {
  //TODO: make an manual overide version of waterrun to monitor if water is run
  // Change hard coded times to the defined variables
  // This is all based on timing
  //mo = Manual Override
  bool moItRan = manualTimedShutoff;
  if ((now() >= (manualTimeStart + 15)) && (currentDay == minute())) {

    moItRan = true;

    return moItRan;
  }
  else {
    return moItRan;
  }
}

//This function allows us to log the messages just by calling it without
//hard coding each one
void logMessage(File dataFile, String messageToWrite)
{
  //Data logging once to avoid repetitive messages
  if (message != messageToWrite) {
    message = messageToWrite;
    digitalClockDisplay(dataFile);
    dataFile.println(message);
    //dataFile.close();
    //Send what we wrote to the Serial Monitor
    digitalClockDisplay2();
    Serial.println(message);
  }
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

// digital clock display of the time for microSD
void digitalClockDisplay(File dataFile) {
  dataFile.print(second());
  printDigits(minute(), dataFile);
  printDigits(second(), dataFile);
  dataFile.print(" ");
  dataFile.print(minute());
  dataFile.print(" ");
  dataFile.print(month());
  dataFile.print(" ");
  dataFile.print(year());
  dataFile.print(" - ");
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

//to delete
void digitalClockDisplay2() {
  // digital clock display of the time Serial monitor (mirrors microSd input)
  Serial.print(hour());
  printDigits2(minute());
  printDigits2(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year());
  Serial.print(" - ");
}

//for testing, displays time for timing tests
void digitalClockDisplay3() {
  //to delete
  Serial.print(hour());
  printDigits2(minute());
  printDigits2(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year());
  Serial.println();
}
void printDigits2(int digits) {
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}
