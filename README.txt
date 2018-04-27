Experiment To Do List: 4/24

1. Mount IR transmitter and receiver
	Drill hole through spare gear

2. Assemble circuits
	Rocker switch to control data collection
		LED diode to show status of data collection (Done)
	Brake cable tension user interface
		Potentiometer to control force value (Done)
		Quad 7-segment display to show force value
	Wire everything to Teensy

3. Measure cable tension and kinetic braking friction
	Calculate kinetic friction and braking torque

4. Execute experiment

---------------------------------------------------------------------------------------------------

Experiment Wiring Notes:

Voltmeters:
	Wires connect to pin 3 (middle pin) on protoboard
	
Rocker Switch:

----------------------------------------------------------------------------------------------------

Circuits to Build

Ammeter voltmeter 1
Ammeter voltmeter 2
Voltmeter
Rocker switch
Data status LED
Force sensor potentiometer
Voltage dividers
7-Segment Display

------------------------------------------------------------------------------------------------------

SOFTWARE TODO:

1. Install Adafruit 7-segment display library and dependency
	7-Segment Display library found here: https://github.com/adafruit/Adafruit_LED_Backpack
	Dependency (Adafruit_GFX) found here: https://github.com/adafruit/Adafruit-GFX-Library
	Install through Library Manager
	
2. Ensure code compiles

3. Write test program
