/*
button 1 and pot 1 (on the left) control LEDS
right pot controls frequency of two oscillators together with 2 LDRs
oscillators  - sine and triangle
change the control rate from 128 to 64 (default) to get rid of cliks
playing with control rate gives nice results - at low rate like 8 or 16 it becomes rhitm, at high rate 1024 or more it becomes an audio modulation making crazy timbres
*/


// NEOPIXEL INIT
#include <Adafruit_NeoPixel.h>
#define NEOPIXEL_PIN1 6  // Neopixel data pin
#define NEOPIXEL_PIN2 7  // Neopixel data pin
#define NUM_PIXELS 1     // Number of Neopixels

// AUDIO LIBRARY INIT
#define MOZZI_CONTROL_RATE 1024
#include <Mozzi.h>
#include <Oscil.h>                             // oscillator
#include <tables/sin2048_int8.h>               // table for Oscils to play
#include <tables/triangle_analogue512_int8.h>  // table for Oscils to play
#include <Smooth.h>

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

// var to know the state of button
bool pressed = false;
bool btnVal2;
// var for random clock led
unsigned long lastRandomBlink = 0;
unsigned long randomInterval = 500;
bool randomLedState = false;

int freqOffset;
int freqOff2;
int frequency1;  // value for oscilator frequency
int frequency2;  // value for oscilator frequency

// use: Oscil <table_size, update_rate> oscilName (wavetable), look in .h file of table #included above
Oscil<SIN2048_NUM_CELLS, MOZZI_AUDIO_RATE> aSin(SIN2048_DATA);
Oscil<TRIANGLE_ANALOGUE512_NUM_CELLS, MOZZI_AUDIO_RATE> bTri(TRIANGLE_ANALOGUE512_DATA);

int fadeValue;  // value for fading leds

void setup() {
  // Serial.begin(9600);  // for Teensy 3.1, beware printout can cause glitches

  // BUTTON AS INPUT PULLUP
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PIN2, INPUT_PULLUP);

  // INITALIZE LED NEOPIXEL

  strip1.begin();
  strip2.begin();
  strip1.show();  // Start with LEDs off
  strip2.show();  // Start with LEDs off
  strip1.setBrightness(50);  // Set BRIGHTNESS to about 1/5 (max = 255)
  strip2.setBrightness(50);   // Set BRIGHTNESS to about 1/5 (max = 255)
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
  btnVal2 = digitalRead(BUTTON_PIN2);

  // generate random freq offset when btn 2 pressed
  if (btnVal2 == 0) {
      if (!pressed) {
      freqOff2 = random(1000);
        pressed = true;
      }
  }
  else {
    pressed = false;
  }
  

  freqOffset = map(AudiopotValue, 0, 1024, 0, 500) + freqOff2;
  frequency1 = map(LDR1_value, 0, 1000, 0, 2000) + freqOffset;
  frequency2 = map(LDR2_value, 0, 1000, 0, 2000) + freqOffset;


  // set the frequency
  aSin.setFreq(frequency1);
  bTri.setFreq(frequency2);


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

  return MonoOutput::from8Bit((aSin.next() + (bTri.next())) * 0.25);
  // return MonoOutput::from8Bit(aSin.next());
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
