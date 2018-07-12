#define TOOTH_COUNT 116
#define TOOTH_COUNT_OUT 36
#define TOOTH_MISSING_OUT 1

#define bitGet(p,m) ((p) & (m)) // Get the value of a bit, like bitGet(PORTB, BIT(5));
#define bitSet(p,m) ((p) |= (m)) // Set the value of a bit (set it to 1), like bitSet(PORTB, BIT(2));
#define bitClear(p,m) ((p) &= ~(m)) // Clear a bit (set it to 0), like bitClear(PORTB, BIT(2));
#define bitFlip(p,m) ((p) ^= (m))
#define bitWrite(c,p,m) (c ? bit_set(p,m) : bit_clear(p,m))
#define BIT(x) (0x01 << (x))
#define LONGBIT(x) ((unsigned long)0x00000001 << (x))

int toothCounter = 0;
int rpmCounter = 0;
int triggeringTeeth[TOOTH_COUNT_OUT];
int triggeringOffset[TOOTH_COUNT_OUT];
int currentTriggeringIndex = 0;
bool trigger;
int toothLength = 133;

bool firstRefToothDetected = false;
bool errorDetected = false;
long lastTrigger = 0;
long lastTeethTimestamp = 0;
long previousTimeBetweenTeeth = 0;
int toothLengthCoef;
byte lastState = B00000000;
bool lastSmallTooth;
bool lastRefTooth;

ISR(PCINT0_vect) {
  if (bitGet(PINB, BIT(3))) {
    rpmCounter++;
    toothCounter = 0;
    currentTriggeringIndex = TOOTH_MISSING_OUT;
    bitClear(PORTB, BIT(4));
    toothLength = previousTimeBetweenTeeth * toothLengthCoef;
  }
}

ISR(INT0_vect) {
  toothCounter++;
  if (toothCounter == triggeringTeeth[currentTriggeringIndex]) {
    trigger = true;
//  } else {
//    long aux = micros();
//    previousTimeBetweenTeeth = aux - lastTeethTimestamp;
//    lastTeethTimestamp = aux;
  }
}

void setup() {
  GIMSK = 0b01100000;    // turns on pin change interrupts
  PCMSK = 0b00001000;    // turn on interrupts on pins PB3, PB4
  bitClear(MCUCR, BIT(0));
  bitSet(MCUCR, BIT(1));
  sei();                 // enables interrupts
  toothLengthCoef = TOOTH_COUNT / (TOOTH_COUNT_OUT * 2);
  bitSet(DDRB, BIT(4));
  bitClear(DDRB, BIT(2));
  bitClear(DDRB, BIT(3));

  for (int i = 0; i < TOOTH_COUNT_OUT; i++) {
    triggeringTeeth[i] = (TOOTH_COUNT * i) / TOOTH_COUNT_OUT;
    triggeringOffset[i] = 100 * ((((float) TOOTH_COUNT * (float) i) / (float) TOOTH_COUNT_OUT) - (float) triggeringTeeth[i]);
  }

  lastTrigger = micros();
}

void loop() {
  if (trigger) {
    trigger = false;
    long currentTime = micros();
    long offsetFromLastTrigger = (previousTimeBetweenTeeth * triggeringOffset[currentTriggeringIndex]) / 100;
    delayMicroseconds(offsetFromLastTrigger);
    bitClear(PORTB, BIT(4));
    currentTriggeringIndex++;
    delayMicroseconds(150);
    bitSet(PORTB, BIT(4));
  }
}
