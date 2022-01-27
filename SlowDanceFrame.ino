//Base frequency and trimmer Ranges
#define BASE_FREQ 80.0 //80 is on the spot for many flowers. Feel free to play with this +/-5Hz

// eg. f=0.5 -> T=2 -> 2 seconds per slow motion cycle
float phase = 0.1;

// MAGNET: Timer 2, prescaler 1024 = CS111 = 64us/tick
#define MAGNET_PIN 3
float magDuty = 15; // be carefull: not overheat the magnet! better adjust force through magnet position
float magFreq = BASE_FREQ;
long magTime = round(16000000 / 1024 / magFreq);

// LIGHT: Timer 1, prescaler 8 = CS010 = 0.5 us/tick
#define LIGHT_PIN 10
float lightDuty = 7;
float lightFreq = magFreq + phase;
long lightTime = round(16000000 / 8 / lightFreq);

// button & led
#define BTN_PIN 6
#define LED_PIN 5
// led modes
#define LED_MAX_DUTY 64.0
#define blinkFunc()  ((LED_MAX_DUTY * 3.0 / 4) + LED_MAX_DUTY * sin( (millis() / 10000.0) * (2.0 * PI)))
#define ledBlink() analogWrite(LED_PIN, max(0, blinkFunc()))
#define ledSleep() analogWrite(LED_PIN, LED_MAX_DUTY / 4.0)

// read potentiometers: motion speed (phase shift), led brightness, magnet strength
#define getAnalog(pin, minval, maxval) (maxval - (maxval - minval) / 1023L * analogRead(pin))
#define readPhaseShift()  (-getAnalog(A2, 0.25, 2.0))
#define readBrightness() getAnalog(A1, 0.0, 20.0)
#define readStrength() getAnalog(A0, 5.0, 25.0)

void setup() {
	pinMode(BTN_PIN, INPUT);
	pinMode(LED_PIN, OUTPUT);
	pinMode(A1, INPUT);
	pinMode(A2, INPUT);
	pinMode(A3, INPUT);
	pinMode(MAGNET_PIN, OUTPUT);
	pinMode(LIGHT_PIN, OUTPUT);
	magnet_off();
	OCR2A = round(magTime);
	OCR2B = round(magDuty * magTime / 100L);
	light_off();
	OCR1A = round(lightTime);
	OCR1B = round(lightDuty * lightTime / 100L);
	sei();
}

// mode: 0 off -> 1 slow motion -> 2 distorted reality -> 0 off
int mode = 0;
bool modeChanged = true;
bool btnReleased = true;

void loop() {
	// read button released event
	if (btnReleased && digitalRead(BTN_PIN) == HIGH) {
		if (++mode > 2) mode = 0;
		modeChanged = true;
		btnReleased = false;
		delay(100);
	} else if (!btnReleased && digitalRead(BTN_PIN) == LOW) {
		btnReleased = true;
	}
	// process mode changes
	if (modeChanged) {
		modeChanged = false;
		if (mode == 0) {
			magnet_off();
			light_off();
		} else if (mode == 1) {
			magnet_on();
			light_on();
			magFreq = BASE_FREQ;
			ledSleep();
		}
	}
	if (mode) {
		// read potentiometers
		phase = readPhaseShift();
		delay(3);
		lightDuty = readBrightness();
		delay(3);
		magDuty = readStrength();
		// configure oscilations
		lightFreq = magFreq * mode + phase;
		magTime = round(16000000L / 1024L / magFreq);
		lightTime = round(16000000L / 8L / lightFreq);
		OCR2A = round(magTime);
		OCR2B = round(magDuty * magTime / 100L);
		OCR1A = round(lightTime);
		OCR1B = round(lightDuty * lightTime / 100L);
	} else {
		ledBlink();
	}
}

void magnet_on() {
	TCCR2A = 0;
	TCCR2B = 0;
	TCCR2A = _BV(COM2A0) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
	TCCR2B = _BV(WGM22) | _BV(CS22) | _BV(CS21) | _BV(CS20);
}

void magnet_off() {
	TCCR2A = 0;
	TCCR2B = 0;
	TCCR2A = _BV(COM2A0) | _BV(COM2B1);
	TCCR2B = _BV(CS22) | _BV(CS21) | _BV(CS20);
}

void light_on() {
	TCCR1A = 0;
	TCCR1B = 0;
	TCCR1A = _BV(COM1A0) | _BV(COM1B1) | _BV(WGM11) | _BV(WGM10);
	TCCR1B =  _BV(WGM13) | _BV(WGM12)  |  _BV(CS11);
}

void light_off() {
	TCCR1A = 0;
	TCCR1B = 0;
	TCCR1A = _BV(COM1A0) | _BV(COM1B1);
	TCCR1B =  _BV(CS11);
}
