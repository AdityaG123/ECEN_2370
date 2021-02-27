To run and execute this project, you will need to plus in the EFM32 to the computer and plug in the correct pins on 
the bluetooth module. Once that is done, you will need to load the code from my course project and build it to ensure that
it is loaded onto the board. Once that is done, to check to see if the SI7021 is reading and writing data using the I2C
bus, you will have to go to click Run > Profile from Simplicity and then flip the switch on the EFM board from AEM to BAT 
and back to AEM. Make sure there is about a second gap between each switch flip otherwise the board will not start to transmit
data. Once that is done, the profiler should be able to show pulses every 2.7 seconds. If you zoom in on that, it will show a 
series of smaller pulses that signify that the bytes are being transmitted and that energy is being used in the lowest energy
state to transmit those data bytes. 

To show that this project is able to run using both I2C0 and I2C1, you will have to go to the SI7021.h file and locate the 
"#define 	I2C_PERIPHERAL			I2C1" variable definition at the top of the file. You can change the I2C1 to I2C0 
in the variable. This means that the data can be transmitted either through the I2C1 or the I2C0 bus interchangably as long as 
once it has been changed, the project is rebuilt and loaded onto the board. 

Once the project is loaded onto the board, to see the outputted values from the board via Bluetooth, make sure you are not in the 
energy profiler and connect the board to a phone or other bluetooth device using the BLE terminal app used from previous labs.
My BLE module was named "Aditya's BLE." Connect to this module from the phone and open the BLE terminal app and flip the switch 
on the board using the process above and wait until "Passes Circular Buffer Test" appears on the terminal. Wait for a few more seconds 
and the following will print onto the terminal:

Hello World
Course Project
Aditya Gopalan
Temp = "Printed temp here" F
RH = "Printed RH here" %
Temp = "Printed temp here" F
RH = "Printed RH here" %
...
...
...
...
and so on and so forth.

Once this shows up, this means that it is displaying all the relevant temperature and relative humidity via the bluetooth module to 
the phone using the I2Cx bus.

Grader's Note: This project worked in my Energy Profiler as well as debugger so there may be an issue that I may not know of on the 
grader's end. Also make sure to check both I2C0 and I2C1 using the instructions above. Thanks! 

P.S.: To whoever it may concern... Have a great Christmas and rest of the year and thank you for a stressful but fantastic semester!

