Experiment To Do List: 4/24

1. Mount IR transmitter and receiver
	Drill hole through spare gear

2. Assemble circuits
	Rocker switch to control data collection
		LED diode to show status of data collection
	Brake cable tension user interface
		Potentiometer to control force value
		Quad 7-segment display to show force value
	Wire everything to Teensy

3. Measure cable tension and kinetic braking friction
	Calculate kinetic friction and braking torque

4. Execute experiment

---------------------------------------------------------------------------------------------------

Experiment Wiring Notes:

Voltmeters:
	Wires connect to pin 3 (middle pin) on protoboard
	
Comparator:
	Non-Invert is color sensor voltage
	Invert is reference voltage (connect potentiometer's third lead)
	
Rocker Switch:

IR-Emitting Diode (NTE3029B):
	Wired like regular LED
	Digital port with resistor

IR Photodiode (NTE3033):
	Analog port

----------------------------------------------------------------------------------------------------

Circuits to Build

Ammeter voltmeter 1
Ammeter voltmeter 2
Voltmeter
Rocker switch
Data status LED
Force sensor potentiometer
IR transmitter
IR receiver
Voltage dividers