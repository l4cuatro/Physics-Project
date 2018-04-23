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


#define AMP_1_PIN ((byte)1)
#define AMP_2_PIN ((byte)2)
#define VOLT_PIN ((byte)3)
#define ENCODER_PIN ((byte)4)
#define COLOR_1R_PIN ((byte)5)
#define COLOR_1G_PIN ((byte)6)
#define COLOR_1B_PIN ((byte)7)
#define COLOR_1W_PIN ((byte)8)
#define COLOR_2R_PIN ((byte)9)
#define COLOR_2G_PIN ((byte)10)
#define COLOR_2B_PIN ((byte)11)
#define COLOR_2W_PIN ((byte)12)
#define PULLEY_MTR_PIN ((byte)13)
#define PULLEY_ENCODER_PIN_1 ((byte)14)
#define PULLEY_ENCODER_PIN_2 ((byte)15)


double voltScale = .25;
double ampFac = (1.0);
double frictionCoeff = (1.0);



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
    int getPin() { return pin; }
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

class Color {
  AnaSensor red, green, blue, white;
  protected:
    int thresh;
  public:
    Color(byte rPin, byte gPin, byte bPin, byte wPin, int thresh) : red(rPin), green(gPin), blue(bPin), white(wPin) {
      this->thresh = thresh;
    }
    void update() {
      red.update();
      green.update();
      blue.update();
      white.update();
    }
    int getColor(char color) {
      if(color == 'r' || color == 'R')
        return red.getVal();
      else if(color == 'g' || color == 'G')
        return green.getVal();
      else if(color == 'b' || color == 'B')
        return blue.getVal();
      else
        return white.getVal();
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
    Encoder enc;
    

    SensorArray(byte ampPin1, byte ampPin2, byte voltPin, byte encPin, double ampScale, double convertToAmps, double voltScale, double convertToFriction) :
      amp(ampPin1, ampPin2, ampScale, ampScale, convertToAmps),
      volt(voltPin, voltScale),
      enc(encPin) {
      
    }
    
    void update() {
      enc.update();
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

    SensorArray sensors;
    
    void update() {
      sensors.update();
      time = now();
      speed = sensors.enc.getSpeed();
      current = sensors.amp.getCurrent();
      voltage = sensors.volt.getVoltage();
      efficiency = (6.283185 * torque * speed) / (voltage * current);
    }

    Experiment(byte ampPin1, byte ampPin2, byte voltPin, byte encPin, double ampScale, double convertToAmps, double voltScale, double convertToFriction) :
      sensors(ampPin1, ampPin2, voltPin, encPin, ampScale, convertToAmps, voltScale, convertToFriction) {

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
    ENCODER_PIN,
    voltScale, ampFac, voltScale, frictionCoeff);

void setup() {
  // put your setup code here, to run once:
  attachInterrupt(digitalPinToInterrupt(COLOR_1R_PIN), redIncrement, RISING);
  attachInterrupt(digitalPinToInterrupt(COLOR_1B_PIN), blueIncrement, RISING);
  File results = SD.open("results.xml");
  XMLWriter Xml(results);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  experiment.update();
  experiment.write(Xml);
  delay(50);
}
