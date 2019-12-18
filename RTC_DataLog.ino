#include <TimeLib.h>
#include <SD.h>
#include <SPI.h>

// change this to match your SD shield or module;
// Teensy 3.5 & 3.6 on-board: BUILTIN_SDCARD
const int chipSelect = BUILTIN_SDCARD;
/*  for checking if an hour has passed
int rHour;
int rMin;
int rSec;
int tempH;
int tempM;
int tempS;
*/

void setup() {
    // set the Time library to use Teensy 3.0's RTC to keep time
  setSyncProvider(getTeensy3Time);

  Serial.begin(115200);                                             
  while (!Serial);  // Wait for Arduino Serial Monitor to open      
  delay(100);                                                    
  if (timeStatus()!= timeSet) {                                   
    Serial.println("Unable to sync with the RTC");               
  } else {                                                       
    Serial.println("RTC has set the system time");
  }                                                               

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

  int hours = hour();

   // if the file is available, write to it:
  if (dataFile) {
    digitalClockDisplay(dataFile); 
    dataFile.close();
  }  
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }
  Serial.println(now());
  //Serial.println(hour());
  //Serial.println(minute());
  //Serial.println(second());
  delay(1000);
}

/*  code to process time sync messages from the serial port   */
#define TIME_HEADER  "T"   // Header tag for serial time sync message

unsigned long processSyncMessage() {
  unsigned long pctime = 0L;
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013 

  if(Serial.find(TIME_HEADER)) {
     pctime = Serial.parseInt();
     return pctime;
     if( pctime < DEFAULT_TIME) { // check the value is a valid time (greater than Jan 1 2013)
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
void printDigits(int digits, File dataFile){
  dataFile.print(":");
  if(digits < 10)
    dataFile.print('0');
  dataFile.print(digits);
}

/* For checking if an hour has passed
bool checkHour(){
  
}*/
