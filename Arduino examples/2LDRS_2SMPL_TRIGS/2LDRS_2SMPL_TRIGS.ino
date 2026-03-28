/* 
TWO LDRS AND 2 TRESHOLDS THAT TRIGGER THE SAME SAMPLE
POT 1 AND BTN 1 CONTROLS NEOPIXELS MODES - LIKE ALWAYS
POT 2 CTRL PITCH
BTN 2 ALSO triggers sample
EXTRA: WHEN TRERSHOLD PASSED RANDOMIZE START/END/LOOPING OF SAMPLE! 
PRETTY COOL :)
*/

// NEOPIXEL INIT
#include <Adafruit_NeoPixel.h>
#define NEOPIXEL_PIN1 6  // Neopixel data pin
#define NEOPIXEL_PIN2 7  // Neopixel data pin
#define NUM_PIXELS 1     // Number of Neopixels

// AUDIO LIBRARY INIT
#define MOZZI_CONTROL_RATE 1024  //256 512 4096. 8192 16384
#include <Mozzi.h>
#include <Sample.h>  // Sample template
//#include <samples/abomb16384_int8.h>
#include <samples/burroughs1_18649_int8.h>
//#include <samples/amen2_0_int8.h>


const char POT_LED_PIN = 0;    // knob analog pin 0 - CONTRL LEDS
const char POT_AUDIO_PIN = 1;  // analog pin 1 knob for audio control
const char LDR1_PIN = 3;       // set the input for the LDR to analog pin 2
const char LDR2_PIN = 2;       // set the input for the LDR to analog pin 3
const char BUTTON_PIN = 2;     // set the DIGITAL input pin for the BTN
const char BUTTON_PIN2 = 3;    // set the DIGITAL input pin for the BTN

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip1(NUM_PIXELS, NEOPIXEL_PIN1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2(NUM_PIXELS, NEOPIXEL_PIN2, NEO_GRB + NEO_KHZ800);

// BUTTON FOR MODE CHANGING
int buttonState = HIGH;
int lastButtonState = HIGH;
int modeCounter = 0;                 // Mode counter for cycling through modes
unsigned long lastDebounceTime = 0;  // Time for debounce
unsigned long debounceDelay = 50;    // Debounce delay

// Blinking variables NO DELAY
unsigned long previousMillis = 0;
int blinkInterval = 40;  // Default blink interval
bool ledState = false;   // Track LED on/off state

// var for random clock led
unsigned long lastRandomBlink = 0;
unsigned long randomInterval = 500;
bool randomLedState = false;

// use: Sample <table_size, update_rate, interpolation > SampleName (wavetable)
Sample<BURROUGHS1_18649_NUM_CELLS, MOZZI_AUDIO_RATE, INTERP_LINEAR> aSample(BURROUGHS1_18649_DATA);

// use: Sample <table_size, update_rate, interpolation > SampleName (wavetable)
//Sample<amen_NUM_CELLS, MOZZI_AUDIO_RATE, INTERP_LINEAR> aSample(amen_DATA);


float recorded_pitch = (float)BURROUGHS1_18649_SAMPLERATE / (float)BURROUGHS1_18649_NUM_CELLS;

//float recorded_pitch = (float)amen_SAMPLERATE / (float)amen_NUM_CELLS;

int fadeValue;  // value for fading leds

const int threshold1 = 300;  // threshold FOR LDR1
const int threshold2 = 300;  // threshold FOR LDR2

// var to know the state of trigger
boolean triggered1 = false;
boolean triggered2 = false;

// function to set random start
unsigned int chooseStart() {
  return random((unsigned int)BURROUGHS1_18649_NUM_CELLS);
  //return random((unsigned int)amen_NUM_CELLS);
}

// function to set random end
unsigned int chooseEnd(unsigned int startpos) {
   return random(startpos, (unsigned int)BURROUGHS1_18649_NUM_CELLS);
  //return random(startpos, (unsigned int)amen_NUM_CELLS);
}

// FUNCTION RANDOMLY TURNING LOOPING ON/OFF
void RandLoop() {
  if (random(2) == 0) {
    aSample.setLoopingOn();
  } else {
    aSample.setLoopingOff();
  }
}


// FUNCTION TO set start and end
void chooseStartEnd() {

  unsigned int s, e;
  {
    s = chooseStart();
    e = chooseEnd(s);
  }
  aSample.setStart(s);
  aSample.setEnd(e);
}


// FOR ANOTHER LDR
unsigned int startpos2 = 0;
unsigned int endpos2 = 0;


void setup() {
  // Serial.begin(9600);  // for Teensy 3.1, beware printout can cause glitches

  // BUTTON AS INPUT PULLUP
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PIN2, INPUT_PULLUP);

  // INITALIZE LED NEOPIXEL
  strip1.setBrightness(50);  // Set BRIGHTNESS to about 1/5 (max = 255)
  strip2.setBrightness(50);  // Set BRIGHTNESS to about 1/5 (max = 255)
  strip1.begin();
  strip2.begin();
  strip1.show();  // Start with LEDs off
  strip2.show();  // Start with LEDs off

  startMozzi();
}


void updateControl() {

  //read the 1st button
  handleButtonPress();  // function to change button states
  // read analog ins
  int potValue = mozziAnalogRead<10>(POT_LED_PIN);  // 10bit value is 0-1023
  int AudiopotValue = mozziAnalogRead<10>(POT_AUDIO_PIN);
  int LDR1_value = mozziAnalogRead<10>(LDR1_PIN);
  int LDR2_value = mozziAnalogRead<10>(LDR2_PIN);
  //read the 2nd button
  int btnVal2 = digitalRead(BUTTON_PIN2);

  // map it to values between 0.1 and about double the recorded pitch
  float pitch = (recorded_pitch * (float)AudiopotValue / 512.f) + 0.1f;

  // set the sample playback frequency
  aSample.setFreq(pitch);

  // only trigger once each time LDR goes over/BELOW the threshold
  if ((LDR1_value < threshold1) || (btnVal2 == LOW)) {
    if (!triggered1) {

      chooseStartEnd();  //RANDOMIZE START STOP
      RandLoop();        // RANDOMIZE LOOPPING
      aSample.start();
      triggered1 = true;
    }
  } else {
    triggered1 = false;
  }

  // only trigger once each time LDR2 goes over the threshold
  // SAME BUT WIUTHOUT FUNCTION :)
  if ((LDR2_value < threshold2) || (btnVal2 == LOW)) {
    if (!triggered2) {
       startpos2 = random(((BURROUGHS1_18649_NUM_CELLS) / 2), (BURROUGHS1_18649_NUM_CELLS));
      //startpos2 = random(((amen_NUM_CELLS) / 2), (amen_NUM_CELLS));

          endpos2 = random((unsigned int)BURROUGHS1_18649_NUM_CELLS);
      //endpos2 = random((unsigned int)amen_NUM_CELLS);

      aSample.setStart(startpos2);
      aSample.setEnd(endpos2);
      RandLoop();
      aSample.start();
      triggered2 = true;
    }
  } else {
    triggered2 = false;
  }

  switch (modeCounter) {
    case 0:                                             // Blinking mode (fixed!) - inversed leds
      blinkInterval = map(potValue, 0, 1023, 10, 500);  // Blink speed

      if (millis() - previousMillis >= blinkInterval) {
        previousMillis = millis();
        ledState = !ledState;  // Toggle LED state

        if (ledState) {
          strip1.fill(strip1.Color(255, 0, 0));  // ON
          strip2.clear();                        // OFF (turn off all LEDs)

        } else {
          strip1.clear();                        // OFF (turn off all LEDs)
          strip2.fill(strip2.Color(255, 0, 0));  // ON
        }
        strip1.show();
        strip2.show();
      }
      break;

    case 1:                                             // Blinking mode (fixed!)
      blinkInterval = map(potValue, 0, 1023, 1, 1500);  // Blink speed

      if (millis() - previousMillis >= blinkInterval) {
        previousMillis = millis();
        ledState = !ledState;  // Toggle LED state

        if (ledState) {
          strip1.fill(strip1.Color(0, 255, 0));  // ON
          strip2.clear();                        //OFF (turn off al LEDs
        } else {
          strip1.clear();                        // OFF (turn off all LEDs)
          strip2.fill(strip2.Color(0, 255, 0));  // ON
        }
        strip1.show();
        strip2.show();
      }
      break;

    case 2:                                               // Blinking mode (fixed!)
      blinkInterval = map(potValue, 0, 1023, 100, 5000);  // Blink speed

      if (millis() - previousMillis >= blinkInterval) {
        previousMillis = millis();
        ledState = !ledState;  // Toggle LED state

        if (ledState) {
          strip1.fill(strip1.Color(0, 0, 255));  // ON
          strip2.clear();                        // OFF (turn off all LEDs)
        } else {
          strip1.clear();                        // OFF (turn off all LEDs)
          strip2.fill(strip2.Color(0, 0, 255));  // ON
        }
        strip1.show();
        strip2.show();
      }
      break;

    case 3:                                            // Blinking mode (fixed!)
      blinkInterval = map(potValue, 0, 1023, 1, 500);  // Blink speed

      if (millis() - previousMillis >= blinkInterval) {
        previousMillis = millis();
        ledState = !ledState;  // Toggle LED state

        if (ledState) {
          strip1.fill(strip1.Color(255, 255, 255));  // ON
          strip2.fill(strip1.Color(255, 255, 255));  // ON
        } else {
          strip1.clear();  // OFF (turn off all LEDs)
          strip2.clear();  // OFF (turn off all LEDs)
        }
        strip1.show();
        strip2.show();
      }
      break;

    case 4:  // Random color mode with 1 led going off

      blinkInterval = map(potValue, 0, 1023, 1, 1000);  // Blink speed
      if (millis() - previousMillis >= blinkInterval) {
        previousMillis = millis();
        ledState = !ledState;  // Toggle LED state
        int r = random(0, 256);
        int g = random(0, 256);
        int b = random(0, 256);

        if (ledState) {
          strip1.fill(strip1.Color(r, g, b));  // ON
          strip2.clear();
        }

        else {
          strip1.clear();                      // OFF (turn off all LEDs)
          strip2.fill(strip2.Color(r, g, b));  // ON
        }
        strip1.show();
        strip2.show();
      }
      break;

    case 5:  // Fade in/out mode

      blinkInterval = map(potValue, 0, 1024, 1, 15);  // Blink speed

      static uint8_t brightness = 0;
      static bool fadingUp = true;

      if (millis() - previousMillis >= blinkInterval) {
        previousMillis = millis();

        if (fadingUp) {
          brightness += 15;
          if (brightness >= 255) fadingUp = false;
        } else {
          brightness -= 15;
          if (brightness <= 0) fadingUp = true;
        }
        // strip.setBrightness(brightness);
        strip1.fill(strip1.Color(0, brightness, 0));
        strip2.fill(strip2.Color(255 - brightness, 0, 255 - brightness));
        strip1.show();
        strip2.show();
      }
      break;

    case 6:  // Fade in/out mode

      blinkInterval = map(potValue, 0, 1024, 1, 250);  // Blink speed
      static uint8_t brightness2 = 0;
      static bool fadingUp2 = true;

      if (millis() - previousMillis >= blinkInterval) {
        previousMillis = millis();

        if (fadingUp2) {
          brightness2 += 25;
          if (brightness2 >= 255) fadingUp2 = false;
        } else {
          brightness2 -= 25;
          if (brightness2 <= 0) fadingUp2 = true;
        }
        strip1.fill(strip1.Color(0, brightness2, 0));
        strip2.fill(strip2.Color(255 - brightness2, 0, 255 - brightness2));
        strip1.show();
        strip2.show();
      }
      break;

    case 7:  // Random color mode
      static unsigned long lastColorChange = 0;
      int randomSpeed = map(potValue, 0, 1023, 100, 2000);  // 100ms–2s

      if (millis() - lastColorChange >= randomSpeed) {
        lastColorChange = millis();

        int r = random(0, 256);
        int g = random(0, 256);
        int b = random(0, 256);
        int r2 = random(0, 256);
        int g2 = random(0, 256);
        int b2 = random(0, 256);

        strip1.fill(strip1.Color(r, g, b));
        strip2.fill(strip2.Color(r2, g2, b2));
        strip1.show();
        strip2.show();
      }
      break;

    case 8:  // Random time mode // Random interval blink mode
      if (millis() - lastRandomBlink >= randomInterval) {
        lastRandomBlink = millis();
        randomLedState = !randomLedState;

        if (randomLedState) {
          strip1.fill(strip1.Color(255, 0, 0));  // Example ON color
          strip2.clear();
        } else {
          strip1.clear();
          strip2.fill(strip2.Color(0, 0, 255));  // Example OFF color
        }

        strip1.show();
        strip2.show();

        // Now update the interval AFTER the blink occurred
        randomInterval = random(100, 2000);  // Set a new interval for next change
      }
      break;
  }
}


AudioOutput updateAudio() {
  return MonoOutput::from8Bit(aSample.next());
}

void loop() {
  audioHook();
}


void handleButtonPress() {
  int reading = digitalRead(BUTTON_PIN);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW) {  // Button pressed
        modeCounter++;
        if (modeCounter > 8) {
          modeCounter = 0;
        }
      }
    }
  }
  lastButtonState = reading;
}
