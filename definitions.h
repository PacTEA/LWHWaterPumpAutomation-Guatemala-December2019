#define closedSwitch false
#define openSwitch true

//tanks and switches are defined below
#define cistern 25 // Main Cistern Low level float switch
#define error 21 // LED for when an error case is thrown
#define pumpControl 20 //signal that tells pump to turn on/off

#define eShutoff 22
#define manualOveride 23

#define tank4Clean 24
#define tank4Low 12
#define tank4High 11

#define tank3Clean 10
#define tank3Low 9
#define tank3High 8

#define tank2Clean 7
#define tank2Low 6
#define tank2High 5

#define tank1Clean 4
#define tank1Low 3
#define tank1High 2

//each tanks error pin
#define tank1Error 14
#define tank2Error 15
#define tank3Error 16
#define tank4Error 17

#define chipSelect BUILTIN_SDCARD
//MISO,sClock, MOSI are already defined, do nothing

//Single Tank test
#define pump 13

//Time defintions for when pump is active
#define timepumpStartsHr 10
#define timepumpStopsHr 16
#define howlongAutoPumpRanSec 3600
#define howlongManPumpRanSec 500

String message = "Pump has been turned off";;
String startPumpMessage = "Automated pump has started";
String eShutoffMessage = "Emergency Shutoff switch has been intiated";
String cisternMessage = "Cistern is low, pump disabled.";
String manualOverideMessage = "Manual override has been engaged";
String manualPumpDailyRun = "Manual Pump has been run for the day";
String pumpOffMessage = "Pump has been turned off";
String pumpDailyRun = "Pump has run for the day";
String cleaningStatus = "Pump is being cleaned";
String errorFloatStatus = "High float switch is closed, low level float switch is open";
String Neutral = "Pump is off, cistern level is met, tanks are full, or time allotted has been met";
