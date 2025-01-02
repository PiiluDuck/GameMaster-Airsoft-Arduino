
#include <limits.h>
#include <Wire.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_NeoPixel.h>
//#include <SoftwareSerial.h>
/*
Original Code by: Ignacio Lillo
95% of Code has been changed and rewritten by: Kaimar Laiva

Updated to work with modern Arduino systems
Updated to work with flash Memory
Updated to fix timing code math
Updated with RingLED animations
Domination Code re-written with new Logic and dynamics
Search and Destroy -//-
Removed Unnesseseary Modes and code
Wifi Shield with ESP8266 Implimented, but not perfected. -currently not used 

This Code is funtional but not fully optomized. 
*/
// Define constants and macros
#define FPSTR(pstr_pointer) (reinterpret_cast<const __FlashStringHelper*>(pstr_pointer))
#define F(string_literal) (FPSTR(PSTR(string_literal)))
#define NUMPIXELS 12
#define LED_PIN1 6
#define LED_PIN2 7
#define LED_COUNT 12

// Setup RGB Rings
Adafruit_NeoPixel ring1 = Adafruit_NeoPixel(NUMPIXELS, LED_PIN1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ring2 = Adafruit_NeoPixel(NUMPIXELS, LED_PIN2, NEO_GRB + NEO_KHZ800);

// LCD and Keypad Setup
LiquidCrystal_I2C lcd(0x27, 16, 2);
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  { '1', '2', '3', 'a' },
  { '4', '5', '6', 'b' },
  { '7', '8', '9', 'c' },
  { '*', '0', '#', 'd' }
};
byte rowPins[ROWS] = { 5, 4, 3, 2 };
byte colPins[COLS] = { A0, A1, A2, A3 };
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ESP8266 Communication
//SoftwareSerial espSerial(2, 3);

// Game Status Variables
const char BT_UP = 'a';
const char BT_DOWN = 'b';
const char BT_SEL = 'd';
const char BT_CANCEL = 'c';
const char BT_EXIT = '*';

// Menu Items
const char menuItems[][17] PROGMEM = {
  "Search & Destroy",
  "Domination",
  "Config"
};
const char configItems[][17] PROGMEM = {
  "Test Sound",
  "Test Mosfet 1",
  "Test Mosfet 2",
  "Back to Menu"
};
// State Variables
unsigned long lastBuzzerTime = 0;
unsigned long gameStartTime, gameEndTime, armingStartTime;
unsigned long team1StartTime, team2StartTime, team1TotalTime = 0, team2TotalTime = 0;
bool team1Zone = false, team2Zone = false, neutralZone = true;

// Sound and Miscellaneous
bool soundEnable = true;
bool searchAndDestroyMode = false;
bool dominationMode = false;
bool buttonHeld = false;
bool demineer = false;
bool cancel = false;
bool buttonReleasedAfterPhase = true;
bool sdStatus = false;  // Search and Destroy Status
bool doStatus = false;  // Domination Status
bool arming = false;

// LED and Mosfet Pins
//const int REDLED = 11;
//const int GREENLED = 10;
unsigned long timeCalcVar;
const int mosfet = 9;
bool mosfetEnable = false;

// Sound Tones
const int tonepin = 8;
const int tonoAlarm1 = 700;
const int tonoAlarm2 = 2600;
const int tonoActive = 1330;
static bool tonePlayedForC = false;
static bool tonePlayedForD = false;

// Timer Variables
int GAMEMINUTES = 45;
int ACTIVATESECONDS = 5;
int BOMBMINUTES = 0;

//LED Rings
Adafruit_NeoPixel* activeRing = nullptr;
bool ring1Blinking = false;
bool ring2Blinking = false;
bool ringNeutralBlinking = false;
bool ringBlinkingRed = false;
unsigned long lastLEDUpdateTime = 0;

// Setup
void setup() {
  Serial.begin(115200);
//  espSerial.begin(115200);

  // LED Setup
 // pinMode(GREENLED, OUTPUT);
 // pinMode(REDLED, OUTPUT);
  pinMode(mosfet, OUTPUT);
  digitalWrite(mosfet, LOW);

  ring1.begin();
  ring2.begin();
  ring1.setBrightness(50);
  ring2.setBrightness(50);
  ring1.clear();
  ring2.clear();
  ring1.show();
  ring2.show();

  lcd.init();
  lcd.backlight();
  //Gives time for wifi connection
  lcd.setCursor(0, 0);
  lcd.print(F("SYSTEM LOADING..."));
  // Animate progress bar and play startup tune
  unsigned long startTime = millis();
  unsigned long totalTimeMillis = 3000;
  while (millis() - startTime < totalTimeMillis) {
    unsigned int percent = updateProgressBar(startTime, totalTimeMillis);
    drawNativeLCDProgressBar(percent);
    //sendBlynkProgressBar(percent);
    playStartupTune();
    delay(50);
  }

  // Welcome Message
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print(F("SOFTAIR"));
  lcd.setCursor(1, 1);
  lcd.print(F("AIRSOFT SYSTEM"));
  tone(tonepin, 2400, 30);
  delay(2000);

  // Keypad Configuration
  keypad.setHoldTime(50);
  keypad.setDebounceTime(50);
  keypad.addEventListener(keypadEvent);

  setupCustomCharacters();
}

// Custom Characters
void setupCustomCharacters() {
  byte bar1[8] = { B10000, B10000, B10000, B10000, B10000, B10000, B10000, B10000 };
  byte bar2[8] = { B11000, B11000, B11000, B11000, B11000, B11000, B11000, B11000 };
  byte bar3[8] = { B11100, B11100, B11100, B11100, B11100, B11100, B11100, B11100 };
  byte bar4[8] = { B11110, B11110, B11110, B11110, B11110, B11110, B11110, B11110 };
  byte bar5[8] = { B11111, B11111, B11111, B11111, B11111, B11111, B11111, B11111 };
  byte up[8] = { B00000, B00100, B01110, B11111, B11111, B00000, B00000 };
  byte down[8] = { B00000, B00000, B11111, B11111, B01110, B00100, B00000 };

  lcd.createChar(0, bar1);
  lcd.createChar(1, bar2);
  lcd.createChar(2, bar3);
  lcd.createChar(3, bar4);
  lcd.createChar(4, bar5);
  lcd.createChar(5, up);
  lcd.createChar(6, down);
}

// Keypad Event
void keypadEvent(KeypadEvent key) {
  if (dominationMode) {  // Domination Mode Logic
    if (key == 'c') {
      if (keypad.getState() == PRESSED || keypad.getState() == HOLD) {
        activeRing = &ring1;
        demineer = true;  // Set demineer when key is pressed or held
      } else if (keypad.getState() == RELEASED) {
        demineer = false;  // Reset demineer when key is released
        activeRing = nullptr;
      }
    } else if (key == 'd') {
      if (keypad.getState() == PRESSED || keypad.getState() == HOLD) {
        activeRing = &ring2;
        demineer = true;
      } else if (keypad.getState() == RELEASED) {
        demineer = false;
        activeRing = nullptr;
      }
    }
  } else if (searchAndDestroyMode) {  // Search and Destroy Mode Logic
    if (key == 'c' || key == 'd') {
      if (keypad.getState() == PRESSED || keypad.getState() == HOLD) {
        demineer = true;
      } else if (keypad.getState() == RELEASED) {
        demineer = false;
      }
    }
  } else {  // Default Logic
    if (key == 'c') {
      if (!tonePlayedForC) {  // Play tone only if not already played
        tone(tonepin, 2400, 30);
        tonePlayedForC = true;  // Mark as played
      }
      cancel = (keypad.getState() == HOLD);
    } else if (key == 'd') {
      if (!tonePlayedForD) {  // Play tone only if not already played
        tone(tonepin, 2400, 30);
        tonePlayedForD = true;  // Mark as played
      }
      demineer = (keypad.getState() == HOLD);
    }

    // Reset the tone flags when the key is released
    if (keypad.getState() == RELEASED) {
      if (key == 'c') {
        tonePlayedForC = false;
      } else if (key == 'd') {
        tonePlayedForD = false;
      }
    }
  }
}


// Main Loop
void loop() {
  menuPrincipal();
}

// Define the Notes and durations
#define NOTE_E5 659
#define NOTE_D5 587
#define NOTE_FS4 370
#define NOTE_GS4 415
#define NOTE_CS5 554
#define NOTE_B4 494
#define NOTE_D4 294
#define NOTE_E4 330
#define NOTE_A4 440
#define NOTE_CS4 277

// Tempo of the tune
int tempo = 220;

// Define the melody and durations
int melody[] = {
  NOTE_E5, 8, NOTE_D5, 8, NOTE_FS4, 4, NOTE_GS4, 4, 
  NOTE_CS5, 8, NOTE_B4, 8, NOTE_D4, 4, NOTE_E4, 4, 
  NOTE_B4, 8, NOTE_A4, 8, NOTE_CS4, 4, NOTE_E4, 4,
  NOTE_A4, 2,
};

// Number of notes in the melody
int notes = sizeof(melody) / sizeof(melody[0]) / 2;

// Calculate the duration of a whole note in milliseconds
int wholenote = (60000 * 4) / tempo;

void playStartupTune() {
  static int thisNote = 0;
  static unsigned long lastNoteTime = 0;
  static int noteDuration = 0;

  // Check if all notes are played
  if (thisNote >= notes * 2) {
    noTone(tonepin);  // Ensure the buzzer is off
    return;
  }

  unsigned long currentMillis = millis();

  // If it's time for the next note
  if (currentMillis - lastNoteTime >= noteDuration) {
    // Turn off the current tone
    noTone(tonepin);

    // Get the next note
    int divider = melody[thisNote + 1];
    if (divider > 0) {
      noteDuration = (wholenote) / divider;
    } else if (divider < 0) {
      noteDuration = (wholenote) / abs(divider);
      noteDuration *= 1.5;  // Dotted notes are 1.5 times the regular duration
    }

    // Play the next note for 90% of the duration
    tone(tonepin, melody[thisNote], noteDuration * 0.9);

    // Move to the next note
    thisNote += 2;
    lastNoteTime = currentMillis;
  }
}
