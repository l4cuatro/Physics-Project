#include <Adafruit_LEDBackpack.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <gfxfont.h>

#include <TimeLib.h>

/*
Voltmeter
  Input voltage goes to breadboard row 2
  12V - Teensy wire goes to row 9, ground goes to row 12
  9V - Teensy row 3, ground row 9
  6V - Teensy row 3, ground row 12

Ammeter
  Voltmeter 1 (pre-shunt)
    Input voltage goes to row 14 (left)
    12V - Teensy row _, ground row _
    9V - _, _
    6V - _, _

  Voltmeter 2 (post-shunt)
    Input voltage - 14 (right)
    12V - _, _
    9V - _, _
    6V - _, _



Comparator Notes
  Non-Invert is sensor voltage
  Invert is threshold voltage


*/


#define AMP_1_PIN ((byte)A2)
#define AMP_2_PIN ((byte)A1)
#define VOLT_PIN ((byte)A1)
#define BRAKE_POT_PIN ((byte)A9)
#define ROCKER_PIN ((byte)6)
#define STATUS_LED_PIN ((byte)7)
#define MOTOR_PIN ((byte)8)
#define WORKING_LED_PIN ((byte)9)

double voltScale = .25;
double ampFac = (3.3 / 1024.0) * (1000.0);

double forceTorque = (0.4);
double potForce = (750 / 1024.0);

class Motor {
  private:
    byte pin;
    int pwr;

  public:
    Motor(byte pin) {
      this->pin = pin;
      pwr = 0;
    }
    int update(int pwr) {
      if(pwr >= 0 && pwr <= 256)
        this->pwr = pwr;
      else if(pwr < 0)
        this->pwr = 0;
      else
        this->pwr = 256;
      analogWrite(pin, pwr);
      return pwr;
    }
  
};

class DigiSensor {
  protected:
    byte pin;
    int val,
      valLast;
      
    byte setPin(byte pin) { return this->pin = pin; }
    
    DigiSensor(byte pin) {
      setPin(pin);
      setVal();
    }
    int setVal(int val = 0) {
      valLast = this->val;
      return this->val = val;
    }
    
  public:
    byte getPin() { return pin; }
    int getVal() { return val; }
    virtual void update() = 0;
};

class Encoder : public DigiSensor {
  private:
    double speed;
    time_t time, timeLast;
    
  public:
    void increment() {
      val++;
    }
    void update() {
      timeLast = time;
      time = now();
      speed = (1.0 * (val - valLast) / (time - timeLast));
    }
    void update(int val) {
      valLast = this->val;
      this->val = val;
      update();
    }
    Encoder(byte pin) : DigiSensor(pin) {
      time = timeLast = 0;
      speed = 0;
    }
    int getSpeed() {
      return speed;
    }
};

class AnaSensor {
  protected:
    uint8_t pin;
    int val;

    byte setPin(byte pin) { return this->pin = pin; }

    
  public:
    int update() { return val = analogRead(pin); }
    int getVal() { return val; }
    AnaSensor(uint8_t pin) {
      setPin(pin);
    }

};

class Voltmeter : public AnaSensor {
  private:
    double scale;

  public:
    double getScale() { return scale; }
    Voltmeter(uint8_t pin, double scale = voltScale) : AnaSensor(pin) {
      this->scale = scale;
    }
    int getVoltage() { return val; }
    double update() {
      val = (1.0 * analogRead(pin)) / voltScale;
    }
};

class Ammeter {
  private: 
    double drop;
    double current;
    double convertFac;
    
  public:
    Voltmeter volt1, volt2;
    Ammeter(uint8_t voltPin1, uint8_t voltPin2, double volt1Scale, double volt2Scale, double convertFac) : volt1(voltPin1, volt1Scale), volt2(voltPin2, volt2Scale) {
      drop = current = 0;
    }
    double update() {
      drop = (volt1.getScale() * volt1.update())- (volt2.getScale() * volt2.update());
      return current = (1.0 * drop) * convertFac;
    }
    double getCurrent() { return current; }
};

class Brake {
	private:
		uint8_t pin;
		double force,
      torque,
			forceTorqueConv,
			potForceConv;
	public:
		Brake(byte pin, double potForceConv, double forceTorqueConv) {
			this->pin = pin;
			this->potForceConv = potForceConv;
			this->forceTorqueConv = forceTorqueConv;
			force = 0;
		}
	
		double getForce() {
			return force;
		}
		double getTorque() {
			return torque;
		}
		
		void update() {
			force =  ((int)((analogRead(pin) * potForceConv * 1000.0)))/1000.0;
			torque = force * forceTorqueConv; 
		}
};

class SensorArray {
  public:
    Ammeter amp;
    Voltmeter volt;
  	Brake brake;

    SensorArray(byte ampPin1, byte ampPin2, byte voltPin, byte brakePin,
		double ampScale, double convertToAmps, double voltScale, double potForceConv, double forceTorqueConv) :
      amp(ampPin1, ampPin2, ampScale, ampScale, convertToAmps),
      volt(voltPin, voltScale),
	    brake(brakePin, potForceConv, forceTorqueConv)
	  {
      
    }
    
    void update() {
    amp.update();
    volt.update();
	  brake.update();
    }
      
};

class Experiment {
    
  public:
    double torque,
      current,
      voltage,
      efficiency;
    time_t time;

    Motor motor;

    SensorArray sensors;
    
    void update(int pwr) {
      sensors.update();
      motor.update(pwr);
      time = now();
	    torque = sensors.brake.getTorque();
      current = sensors.amp.getCurrent();
      voltage = sensors.volt.getVoltage();
    }
	
	void update() {
		update(256);
	}

    void init() {
      motor.update(0);
      time = now();
    }


    Experiment(uint8_t ampPin1, uint8_t ampPin2, uint8_t voltPin, uint8_t brakePin, byte mtrPin, double ampScale, double convertToAmps, double voltScale, double potForce, double forceTorque) :
      sensors(ampPin1, ampPin2, voltPin, brakePin, ampScale, convertToAmps, voltScale, potForce, forceTorque), motor(mtrPin) {
      torque = current = voltage = 0;
    }

    void write(Print* printer) {
       String str = String(time) +  "," + String(voltage) +  "," + String(current) +  "," + String(torque) +  ",____" + ",_____";
       printer->println(str);
    }

    void writeInit(Print* printer) {
      String str = "Time (ms),Voltage (V),Current (A),Torque (N*m),Speed (RPM),Efficiency (%)";
      printer->println(str);
    }
    
};

Experiment experiment = Experiment(AMP_1_PIN, AMP_2_PIN,
    VOLT_PIN,
    BRAKE_POT_PIN,
    MOTOR_PIN,
    voltScale, ampFac, voltScale, potForce, forceTorque);

Adafruit_7segment SevenSeg = Adafruit_7segment();
	
byte blinkReps = 0;

void setup() {
  // put your setup code here, to run once:
  experiment.writeInit(&Serial);
  experiment.motor.update(0);
  pinMode(STATUS_LED_PIN, OUTPUT);
  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(ROCKER_PIN, INPUT_PULLUP);
  SevenSeg.begin(0x70);
  SevenSeg.setBrightness(15);
}

void loop() {
  // put your main code here, to run repeatedly:
  experiment.update();
  if(!digitalRead(ROCKER_PIN)) {
    experiment.write(&Serial);
    digitalWrite(STATUS_LED_PIN, HIGH);
  }
  else {
    digitalWrite(STATUS_LED_PIN, LOW);
  }
  SevenSeg.clear();
  SevenSeg.print(experiment.sensors.brake.getForce(),DEC);
  SevenSeg.writeDisplay();
  if(blinkReps >= 10) {
    blinkReps = 0;
    if(digitalRead(WORKING_LED_PIN))
      digitalWrite(WORKING_LED_PIN, LOW);
    else
      digitalWrite(WORKING_LED_PIN, HIGH);
  }
  blinkReps++;
  delay(100);
}
