#include <TimeLib.h>
#include <SD.h>
#include <SPI.h>
#include "definitions.h"

//TODO: errorRecord[respective tank num] that is set equal to one needs
//to be moved after the print out statement when fxn is called to avoid multiple print outs

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
  pinMode (tank1Error, OUTPUT);
  pinMode (tank2Low, INPUT_PULLUP);
  pinMode (tank2High, INPUT_PULLUP);
  pinMode (tank2Clean, INPUT_PULLUP);
  pinMode (tank2Error, OUTPUT);
  pinMode (tank3Low, INPUT_PULLUP);
  pinMode (tank3High, INPUT_PULLUP);
  pinMode (tank3Clean, INPUT_PULLUP);
  pinMode (tank3Error, OUTPUT);
  pinMode (tank4Low, INPUT_PULLUP);
  pinMode (tank4High, INPUT_PULLUP);
  pinMode (tank4Clean, INPUT_PULLUP);
  pinMode (tank4Error, OUTPUT);

  pinMode (cistern, INPUT_PULLUP);
  pinMode (pumpControl, OUTPUT);
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
  //tank status in various conditions
  bool tank1PumpRun = (digitalRead(tank1High) == openSwitch);
  bool tank1PumpError = (digitalRead(tank1Low) == openSwitch) && (digitalRead(tank1High) == closedSwitch);
  bool tank1Cleaning = (digitalRead(tank1Clean) == closedSwitch);

  bool tank2PumpRun = (digitalRead(tank2High) == openSwitch);
  bool tank2PumpError = (digitalRead(tank2Low) == openSwitch) && (digitalRead(tank2High) == closedSwitch);
  bool tank2Cleaning = (digitalRead(tank2Clean) == closedSwitch);

  bool tank3PumpRun = (digitalRead(tank3High) == openSwitch);
  bool tank3PumpError = (digitalRead(tank3Low) == openSwitch) && (digitalRead(tank3High) == closedSwitch);
  bool tank3Cleaning = (digitalRead(tank3Clean) == closedSwitch);

  bool tank4PumpRun = (digitalRead(tank4High) == openSwitch);
  bool tank4PumpError = (digitalRead(tank4Low) == openSwitch) && (digitalRead(tank4High) == closedSwitch);
  bool tank4Cleaning = (digitalRead(tank4Clean) == closedSwitch);

  //variables for each tank is used in header file for statuses: clean, high, and low states
  // Change hard coded times to the defined variables
  bool canRunPump = (((hour() >= timepumpStartsHr) && (hour() < timepumpStopsHr)) && waterRun == false);

  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  if (tank1Cleaning) {
    tank1PumpRun  = true;
    cleanLogMessage(dataFile, tank1CleanStatus);
  } else {
    tank1PumpRun = (digitalRead(tank1High) == openSwitch);
    cleanRecord1 = 0;
  }

  if (tank2Cleaning) {
    tank2PumpRun = true;
    cleanLogMessage(dataFile, tank2CleanStatus);
  } else {
    tank2PumpRun = (digitalRead(tank2High) == openSwitch);
    cleanRecord2 = 0;
  }

  if (tank3Cleaning) {
    tank3PumpRun = true;
    cleanLogMessage(dataFile, tank3CleanStatus);
  } else {
    tank3PumpRun = (digitalRead(tank3High) == openSwitch);
    cleanRecord3 = 0;
  }

  if (tank4Cleaning) {
    tank4PumpRun = true;
    cleanLogMessage(dataFile, tank4CleanStatus);
  } else {
    tank4PumpRun = (digitalRead(tank4High) == openSwitch);
    cleanRecord4 = 0;
  }
  //reference all tanks for code readability
  bool allTanksPumpRun = (tank1PumpRun && tank2PumpRun && tank3PumpRun && tank4PumpRun);
  bool allTanksPumpable = (canRunPump && allTanksPumpRun);

  //TODO: May consider combining tankCleaning and tankpumpRun

  //Portion of code sets teensy clock to RTC
  if (Serial.available()) {
    time_t t = processSyncMessage();
    if (t != 0) {
      Teensy3Clock.set(t); // set the RTC
      setTime(t);
    }
  }



  /* Tank Test code goes here*/

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
        if (allTanksPumpable) {
          digitalWrite(pumpControl, HIGH);
          digitalWrite(tank1Error, LOW); //light off
          digitalWrite(tank2Error, LOW); //light off
          digitalWrite(tank3Error, LOW); //light off
          digitalWrite(tank4Error, LOW); //light off
          errRecord1 = 0;
          errRecord2 = 0;
          errRecord3 = 0;
          errRecord4 = 0;


          //Data logging once to avoid repetitive messages
          logMessage(dataFile, startPumpMessage);

          if (!startPump) {
            timeStart = now();
            startPump = true;
            currentDay = day();
          }
        }
        else if ((tank1PumpError && !tank1Cleaning) || (tank2PumpError && !tank2Cleaning) ||
                 (tank3PumpError && !tank3Cleaning) || (tank4PumpError && !tank4Cleaning)) {
          //if low level float sensor is down and high level float sensor is up
          //pump turns off and error light is turned on
          digitalWrite(pumpControl, LOW); //pump off

          if (tank1PumpError) {
            digitalWrite(tank1Error, HIGH); //light on
            errorLogMessage(dataFile, tank1ErrorFloatStatus);

          }

          if (tank2PumpError) {
            digitalWrite(tank2Error, HIGH); //light on
            errorLogMessage(dataFile, tank2ErrorFloatStatus);

          }

          if (tank3PumpError) {
            digitalWrite(tank3Error, HIGH); //light on
            errorLogMessage(dataFile, tank3ErrorFloatStatus);

          }

          if (tank4PumpError) {
            digitalWrite(tank4Error, HIGH); //light on
            errorLogMessage(dataFile, tank4ErrorFloatStatus);

          }
        }
        else {
          //when high level float sensor is up, turn pump off
          digitalWrite(pumpControl, LOW); //pump off
          digitalWrite(tank1Error, LOW); //light off
          digitalWrite(tank2Error, LOW); //light off
          digitalWrite(tank3Error, LOW); //light off
          digitalWrite(tank4Error, LOW); //light off

          //TODO: errorRecord[respective tank num] that is set equal to one needs
          //to be moved after the print out statement when fxn is called to avoid multiple print outs
          errRecord1 = 0;
          errRecord2 = 0;
          errRecord3 = 0;
          errRecord4 = 0;

          //Data logging once to avoid repetitive messages
          logMessage(dataFile, Neutral);
        }
      }
      else {//Manual Else
        if (manualPumpRun == false) {
          digitalWrite(pumpControl, HIGH);//pump on
          digitalWrite(tank1Error, LOW); //light off
          digitalWrite(tank2Error, LOW); //light off
          digitalWrite(tank3Error, LOW); //light off
          digitalWrite(tank4Error, LOW); //light off
          //TODO: errorRecord[respective tank num] that is set equal to one needs
          //to be moved after the print out statement when fxn is called to avoid multiple print outs
          errRecord1 = 0;
          errRecord2 = 0;
          errRecord3 = 0;
          errRecord4 = 0;

          //Data logging once to avoid repetitive messages
          logMessage(dataFile, manualOverideMessage);

          //this addresses the fact that with manual override the pump has ran
          //manually for the day
          if (!manualStartPump) {
            manualTimeStart = now();
            manualStartPump = true;
            currentDay = day();
          }
        }
        else {
          digitalWrite(pumpControl, LOW);//pump off
          digitalWrite(tank1Error, LOW); //light off
          digitalWrite(tank2Error, LOW); //light off
          digitalWrite(tank3Error, LOW); //light off
          digitalWrite(tank4Error, LOW); //light off
          //TODO: errorRecord[respective tank num] that is set equal to one needs
          //to be moved after the print out statement when fxn is called to avoid multiple print outs
          errRecord1 = 0;
          errRecord2 = 0;
          errRecord3 = 0;
          errRecord4 = 0;
          logMessage(dataFile, manualPumpDailyRun);
        }

      }
    }
    else {//Cistern else
      digitalWrite(pumpControl, LOW); //pump off
      digitalWrite(tank1Error, LOW); //light off
      digitalWrite(tank2Error, LOW); //light off
      digitalWrite(tank3Error, LOW); //light off
      digitalWrite(tank4Error, LOW); //light off
      //TODO: errorRecord[respective tank num] that is set equal to one needs
      //to be moved after the print out statement when fxn is called to avoid multiple print outs
      errRecord1 = 0;
      errRecord2 = 0;
      errRecord3 = 0;
      errRecord4 = 0;

      //TODO: Have this not happen when initially running
      //potentially set pump off message to be default message
      //Data logging once to avoid repetitive messages
      logMessage(dataFile, cisternMessage);
    }
  }
  else {//eShutoff else
    digitalWrite(pumpControl, LOW); //pump off
    digitalWrite(tank1Error, LOW); //light off
    digitalWrite(tank2Error, LOW); //light off
    digitalWrite(tank3Error, LOW); //light off
    digitalWrite(tank4Error, LOW); //light off
    //TODO: errorRecord[respective tank num] that is set equal to one needs
    //to be moved after the print out statement when fxn is called to avoid multiple print outs
    errRecord1 = 0;
    errRecord2 = 0;
    errRecord3 = 0;
    errRecord4 = 0;

    //Data logging once to avoid repetitive messages
    logMessage(dataFile, eShutoffMessage);
  }

  if (tank1Cleaning) {
    //Data logging once to avoid repetitive messages
    logMessage(dataFile, cleaningStatus);
  }

  /* For testing purposes this displays time second to second
      digitalClockDisplay3();
      delay(1000);
  */


  dataFile.close();

}// END OF VOID LOOP()

void pumpDayReset()
{
  if (currentDay != day()) {
    startPump = false;
    waterRun = false;
  }
}

void manualPumpDayReset()
{
  if (currentDay != day()) {
    manualStartPump = false;
    waterRun = false;
  }
}

//once the pump has ran for the allotted hour we switch states
//in order to shut off the pump
bool pumptimeRestrict(bool restriction)
{
  bool itRan = restriction;
  if ((now() >= (timeStart + howlongAutoPumpRanSec)) && (currentDay == day())) {
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
  if ((now() >= (manualTimeStart + howlongManPumpRanSec)) && (currentDay == day())) {

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

//TODO: errorRecord[respective tank num] that is set equal to one needs
//to be moved after the print out statement when fxn is called to avoid multiple print outs
void errorLogMessage(File dataFile, String messageToWrite)
{ //Data logging once to avoid repetitive error messages
  if (errRecord1 == 0) {
    errRecord1 = 1;
    message = messageToWrite;
    digitalClockDisplay(dataFile);
    dataFile.println(message);
    //dataFile.close();
    //Send what we wrote to the Serial Monitor
    digitalClockDisplay2();
    Serial.println(message);
  }

  if (errRecord2 == 0) {
    errRecord2 = 1;
    message = messageToWrite;
    digitalClockDisplay(dataFile);
    dataFile.println(message);
    //dataFile.close();
    //Send what we wrote to the Serial Monitor
    digitalClockDisplay2();
    Serial.println(message);
  }

  if (errRecord3 == 0) {
    errRecord3 = 1;
    message = messageToWrite;
    digitalClockDisplay(dataFile);
    dataFile.println(message);
    //dataFile.close();
    //Send what we wrote to the Serial Monitor
    digitalClockDisplay2();
    Serial.println(message);
  }

  if (errRecord4 == 0) {
    errRecord4 = 1;
    message = messageToWrite;
    digitalClockDisplay(dataFile);
    dataFile.println(message);
    //dataFile.close();
    //Send what we wrote to the Serial Monitor
    digitalClockDisplay2();
    Serial.println(message);
  }

}

//TODO: cleanRecord[respective tank num] that is set equal to one needs
//to be moved after the print out statement to avoid double print outs
void cleanLogMessage(File dataFile, String messageToWrite)
{ //Data logging once to avoid repetitive clean messages
  if (cleanRecord1 == 0) {
    cleanRecord1 = 1;
    message = messageToWrite;
    digitalClockDisplay(dataFile);
    dataFile.println(message);
    //dataFile.close();
    //Send what we wrote to the Serial Monitor
    digitalClockDisplay2();
    Serial.println(message);
  }

  if (cleanRecord2 == 0) {
    cleanRecord2 = 1;
    message = messageToWrite;
    digitalClockDisplay(dataFile);
    dataFile.println(message);
    //dataFile.close();
    //Send what we wrote to the Serial Monitor
    digitalClockDisplay2();
    Serial.println(message);
  }

  if (cleanRecord3 == 0) {
    cleanRecord3 = 1;
    message = messageToWrite;
    digitalClockDisplay(dataFile);
    dataFile.println(message);
    //dataFile.close();
    //Send what we wrote to the Serial Monitor
    digitalClockDisplay2();
    Serial.println(message);
  }

  if (cleanRecord4 == 0) {
    cleanRecord4 = 1;
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
  dataFile.print(hour());
  printDigits(minute(), dataFile);
  printDigits(second(), dataFile);
  dataFile.print(" ");
  dataFile.print(day());
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
//For testing purposes this displays time second to second
/*void digitalClockDisplay3() {
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
  }*/

void printDigits2(int digits) {
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}
