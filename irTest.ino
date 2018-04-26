#define AMP_1_PIN ((byte)A2)
#define AMP_2_PIN ((byte)A4)
#define VOLT_PIN ((byte)A6)
#define IR_RX_PIN ((byte)4)
#define IR_RX_ANALOG ((byte)A2)
#define IR_TX_PIN ((byte)5)
#define ROCKER_PIN ((byte)6)
#define STATUS_LED_PIN ((byte)7)
#define MOTOR_PIN ((byte)8)
#define WORKING_LED_PIN ((byte)9)

void setup() {
	pinMode(IR_RX_PIN, INPUT);
	pinMode(IR_TX_PIN, OUTPUT);
	pinMode(STATUS_LED_PIN, OUTPUT);
}

void loop() {
	digitalWrite(IR_TX_PIN, HIGH);
	digitalWrite(STATUS_LED_PIN, digitalRead(IR_RX_PIN));
	Serial.print("IR Receiver Value: ");
	Serial.println(analogRead(IR_RX_ANALOG_PIN));
	delay(25);
}