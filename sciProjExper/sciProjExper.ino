#include <TimeLib.h>
#include <SPI.h>
#include <SD.h>
#include <XMLWriter.h>

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


#define AMP_1_PIN ((byte)1)
#define AMP_2_PIN ((byte)2)
#define VOLT_PIN ((byte)3)
#define COLOR_1R_PIN ((byte)4)
#define COLOR_1B_PIN ((byte)5)
#define ROCKER_PIN ((byte)6)
#define STATUS_LED_PIN ((byte)7)
#define MOTOR_PIN ((byte)8)


double voltScale = .25;
double ampFac = (1.0);
double frictionCoeff = (1.0);
double frictionPotCoeff = (1.0);

File results = SD.open("results.xml");
XMLWriter XmlSd(&results);
XMLWriter XmlUsb(&Serial);

volatile int redEncCt = 0,
  blueEncCt = 0;

void redIncrement() {
  redEncCt++;
}

void blueIncrement() {
  blueEncCt++;
}



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
    byte pin;
    int val;

    byte setPin(byte pin) { return this->pin = pin; }

    
  public:
    int update() { return val = analogRead(pin); }
    int getVal() { return val; }
    AnaSensor(byte pin) {
      setPin(pin);
    }

};

class Voltmeter : public AnaSensor {
  private:
    double scale;

  public:
    double getScale() { return scale; }
    Voltmeter(byte pin, double scale = voltScale) : AnaSensor(pin) {
      this->scale = scale;
    }
    int getVoltage() { return val; }
    double update() {
      val = (1.0 * analogRead(pin)) / voltScale;
    }
};

class Ammeter {
  private: 
    int drop;
    double current;
    double convertFac;
    
  public:
    Voltmeter volt1, volt2;
    Ammeter(byte voltPin1, byte voltPin2, double volt1Scale, double volt2Scale, double convertFac) : volt1(voltPin1, volt1Scale), volt2(voltPin2, volt2Scale) {
      drop = current = 0;
    }
    double update() {
      drop = (volt1.getScale() * volt1.update())- (volt2.getScale() * volt2.update());
      return current = (1.0 * drop) / convertFac;
    }
    double getCurrent() { return current; }
};

class SensorArray {
  public:
    Ammeter amp;
    Voltmeter volt;
    Encoder encR, encB;
    

    SensorArray(byte ampPin1, byte ampPin2, byte voltPin, byte encPinR, byte encPinB, double ampScale, double convertToAmps, double voltScale, double convertToFriction, double frictionPot) :
      amp(ampPin1, ampPin2, ampScale, ampScale, convertToAmps),
      volt(voltPin, voltScale),
      encR(encPinR), encB(encPinB) {
      
    }
    
    void update() {
      encR.update();
      encB.update();
      amp.update();
      volt.update();
    }
      
};

class Experiment {
    
  public:
    double torque,
      speed,
      current,
      voltage,
      efficiency;
    time_t time;

    Motor motor;

    SensorArray sensors;
    
    void update() {
      sensors.update();
      motor.update(256);
      time = now();
      speed = 0.5 * (sensors.encR.getSpeed() + sensors.encB.getSpeed());
      current = sensors.amp.getCurrent();
      voltage = sensors.volt.getVoltage();
      efficiency = (6.283185 * torque * speed) / (voltage * current);
    }
    
    void update(int pwr) {
      sensors.update();
      motor.update(pwr);
      time = now();
      speed = 0.5 * (sensors.encR.getSpeed() + sensors.encB.getSpeed());
      current = sensors.amp.getCurrent();
      voltage = sensors.volt.getVoltage();
      efficiency = (6.283185 * torque * speed) / (voltage * current);
    }

    void init() {
      motor.update(0);
      time = now();
    }

    Experiment(byte ampPin1, byte ampPin2, byte voltPin, byte encPinR, byte encPinB, byte mtrPin, double ampScale, double convertToAmps, double voltScale, double convertToFriction, double frictionPot) :
      sensors(ampPin1, ampPin2, voltPin, encPinR, encPinB, ampScale, convertToAmps, voltScale, convertToFriction, frictionPot), motor(mtrPin) {

      torque = speed = current = voltage = 0;
    }

    void write(XMLWriter* xml) {
       xml->tagOpen("Values");
       xml->writeNode("time", time);
       xml->writeNode("current", current);
       xml->writeNode("voltage", voltage);
       xml->writeNode("torque", torque);
       xml->writeNode("speed", speed);
       xml->writeNode("efficiency", efficiency);
       xml->tagClose();
    }
    
};

Experiment experiment = Experiment(AMP_1_PIN, AMP_2_PIN,
    VOLT_PIN,
    COLOR_1R_PIN, COLOR_1B_PIN,
    MOTOR_PIN,
    voltScale, ampFac, voltScale, frictionCoeff, frictionPotCoeff);

void setup() {
  // put your setup code here, to run once:
  attachInterrupt(digitalPinToInterrupt(COLOR_1R_PIN), redIncrement, RISING);
  attachInterrupt(digitalPinToInterrupt(COLOR_1B_PIN), blueIncrement, RISING);
  XmlSd.header();
  XmlSd.tagOpen("Values");
  XmlSd.writeNode("time", "Time");
  XmlSd.writeNode("current", "Current");
  XmlSd.writeNode("voltage", "Voltage");
  XmlSd.writeNode("torque", "Torque");
  XmlSd.writeNode("speed", "Speed");
  XmlSd.writeNode("efficiency", "Efficiency");
  XmlSd.tagClose();
  XmlUsb.header();
  XmlUsb.tagOpen("Values");
  XmlUsb.writeNode("time", "Time");
  XmlUsb.writeNode("current", "Current");
  XmlUsb.writeNode("voltage", "Voltage");
  XmlUsb.writeNode("torque", "Torque");
  XmlUsb.writeNode("speed", "Speed");
  XmlUsb.writeNode("efficiency", "Efficiency");
  XmlUsb.tagClose();
  experiment.init();
}

void loop() {
  // put your main code here, to run repeatedly:
  experiment.update();
  if(ROCKER_PIN) {
    experiment.write(&XmlSd);
    experiment.write(&XmlUsb);
  }
  delay(50);
}
