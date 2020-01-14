# LWHWaterPumpAutomation-Guatemala-December2019
Automation of the water pump between cistern and water tanks. 
General Notes:
If all switches are down, this means they function as normal (Pumps run automatically)
If a tank switch or tank switches are up, respective tank and/or tanks are being cleaned or are cleaning.
eShutoff
Switch should be down to allow the pump to run.
If switch is flipped up, then the emergency shut off has been activated and will not allow the pump to run
Manual Override
If switch is flipped up override is activated, and pump will run for two hours
If switch is down, water system runs normally
*Cleaning/Limpiar
To clean one tank, flip respective tank number switch up after closing intake valve. 

!(https://github.com/PacTEA/LWHWaterPumpAutomation-Guatemala-December2019/blob/master/ControlPanelDiagram.PNG)



LED/Error Handling
If any of the colored LED’s (Rojo or Amarillo) are on then the respective tank is in an error state and will need to be looked at, once the error has been corrected the LED will turn off
