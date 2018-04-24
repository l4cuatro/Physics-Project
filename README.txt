Experiment To Do List: 4/24

1. Construct brake assembly
	Mount brake pad assembly and cable
		Eye bolts to guide cable and secure spring scale

2. Mount color sensor and cardboard encoder disc

3. Assemble circuits
	Motor power supply
		Wire to Talon Motor Controller (Done)
		Shunt resistor between Talon and Motor (Done)
			Voltage dividers on breadboard (Done)
		Voltmeter at motor contact
			Voltage divider on breadboard (Done)
		Mount circuit assembly
			Circuits designed using crimp connectors to allow modular assembly
	Color Sensor Encoder
		NTE943M Comparator
			Potentiometer to tune encoder sensitivity
	Rocker switch to control data collection
		LED diode to show status of data collection
	Brake cable tension user interface
		Potentiometer to control force value
		Dual seven-segment displays to show force value
	Wire everything to Teensy

4. Measure cable tension and kinetic braking friction
	Calculate kinetic friction and braking torque

---------------------------------------------------------------------------------------------------

Experiment Wiring Notes:

Voltmeters:
	Wires connect to pin 3 (middle pin) on breadboard
	
Comparator:
	Non-Invert is color sensor voltage
	Invert is reference voltage (connect potentiometer's third lead)
	
Rocker Switch:

Color Sensor:
