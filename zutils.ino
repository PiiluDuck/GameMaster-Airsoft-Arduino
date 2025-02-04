#include "LCDutils.h"
#include "localization.h"

#define MOSFET_TIME_1 600
#define MOSFET_TIME_OFF_1 50
#define MOSFET_TIME_2 50
#define MOSFET_TIME_OFF_2 100

// Draw progress bar on LCD based on percentage
void drawNativeLCDProgressBar(byte percent) {
  lcd.setCursor(0, 1);              // Start at the beginning of the second row
  int aDrawP = (8 * percent) / 10;  // Calculate the number of custom characters to draw
  int drawnChars = 0;               // Track how many characters are drawn

  // Draw full bars
  while (aDrawP >= 5) {
    lcd.write(4);  // Full bar character
    aDrawP -= 5;
    drawnChars++;
  }

  // Draw partial bar
  if (aDrawP > 0) {
    lcd.write(aDrawP - 1);  // Partial bar characters (0 to 4)
    drawnChars++;
  }

  // Clear remaining spaces
  for (int i = drawnChars; i < 16; i++) {
    lcd.print(' ');  // Fill remaining row with spaces
  }
}

// Updates progress bar and returns progress percentage based on dynamic time
unsigned int updateProgressBar(unsigned long startTime, unsigned long totalTimeMillis) {
  unsigned long elapsed = millis() - startTime;
  if (elapsed > totalTimeMillis) {
    return 100;  // Cap at 100% if time exceeds the total time
  }
  return (elapsed * 100) / totalTimeMillis;  // Calculate percentage
}

void updateTeamTime(bool team1Zone, bool team2Zone, unsigned long& team1Time, unsigned long& team2Time, unsigned long& lastUpdateTime, bool neutralZone) {
  unsigned long currentMillis = millis();
  if (!neutralZone) {
    if (team1Zone) {
      team1Time += currentMillis - lastUpdateTime;
    } else if (team2Zone) {
      team2Time += currentMillis - lastUpdateTime;
    }
  }
  lastUpdateTime = currentMillis;
}
// Calculate Dynamic Interval for Buzzer
unsigned long calculateDynamicInterval(unsigned int percent, unsigned long maxInterval, unsigned long minInterval) {
  return maxInterval - (percent * (maxInterval - minInterval) / 100);
}

// Activate MOSFET for a specified duration
void activateMosfet_1() {
  if (!mosfetEnable || !soundEnable) return;  // Exit if MOSFET is not enabled

  unsigned long startMillis = millis();  // Record the start time
  unsigned long lastToggleMillis = 0;    // Track the last toggle time
  bool mosfetState = false;              // Keep track of the MOSFET state (on/off)

  while (millis() - startMillis < 10000) {  // Run for 10 seconds
    unsigned long currentMillis = millis();

    // Check if it's time to toggle the MOSFET state
    if (mosfetState && (currentMillis - lastToggleMillis >= MOSFET_TIME_1)) {
      digitalWrite(mosfet, LOW);         // Turn the MOSFET off
      mosfetState = false;               // Update state
      lastToggleMillis = currentMillis;  // Update the last toggle time
    } else if (!mosfetState && (currentMillis - lastToggleMillis >= MOSFET_TIME_OFF_2)) {
      digitalWrite(mosfet, HIGH);        // Turn the MOSFET on
      mosfetState = true;                // Update state
      lastToggleMillis = currentMillis;  // Update the last toggle time
    }
  }

  // Ensure the MOSFET is off at the end
  digitalWrite(mosfet, LOW);
}

void activateMosfet_2() {
  if (!mosfetEnable || !soundEnable) return;  // Exit if MOSFET is not enabled

  unsigned long startMillis = millis();  // Record the start time
  unsigned long lastToggleMillis = 0;    // Track the last toggle time
  bool mosfetState = false;              // Keep track of the MOSFET state (on/off)

  while (millis() - startMillis < 10000) {  // Run for 10 seconds
    unsigned long currentMillis = millis();

    // Check if it's time to toggle the MOSFET state
    if (mosfetState && (currentMillis - lastToggleMillis >= MOSFET_TIME_2)) {
      digitalWrite(mosfet, LOW);         // Turn the MOSFET off
      mosfetState = false;               // Update state
      lastToggleMillis = currentMillis;  // Update the last toggle time
    } else if (!mosfetState && (currentMillis - lastToggleMillis >= MOSFET_TIME_OFF_2)) {
      digitalWrite(mosfet, HIGH);        // Turn the MOSFET on
      mosfetState = true;                // Update state
      lastToggleMillis = currentMillis;  // Update the last toggle time
    }
  }

  // Ensure the MOSFET is off at the end
  digitalWrite(mosfet, LOW);
}

void startGameCount() {
  unsigned long countdownMillis = 5000;  // 5 seconds total
  unsigned long startMillis = millis();
  unsigned long elapsedMillis = 0;
  const unsigned long interval = 1000;  // 1-second interval for peeps

  playBuzzer(tonoAlarm1, 100, false);  // Single short beep
  unsigned long lastPeepTime = millis();

  while (elapsedMillis < countdownMillis) {
    unsigned long currentMillis = millis();
    elapsedMillis = currentMillis - startMillis;

    // Calculate progress bar percentage
    byte percent = (elapsedMillis * 100) / countdownMillis;

    // Draw progress bar
    lcd.setCursor(0, 0);
    printLCDFromPROGMEM(gameStartText);
    lcd.setCursor(0, 1);
    drawNativeLCDProgressBar(percent);

    // Play a peep every subsequent second
    if (currentMillis - lastPeepTime >= interval) {
      playBuzzer(tonoAlarm1, 100, false);  // Single short beep
      lastPeepTime = currentMillis;        // Update last peep time
    }

    delay(50);  // Small delay to smooth LCD updates
  }

  lcd.clear();
}

// BUZZER SETTING

// Buzzer Function
void playBuzzer(int frequency, unsigned long interval, bool twoPeeps = false) {
  if (!soundEnable) return;

  static unsigned long lastBuzzerTime = 0;
  static bool secondPeep = false;
  unsigned long currentMillis = millis();

  if (twoPeeps) {
    // Two short peeps every interval
    if (currentMillis - lastBuzzerTime >= (secondPeep ? interval / 2 : interval)) {
      tone(tonepin, tonoAlarm1, 100);  // Play tone for 100ms
      lastBuzzerTime = currentMillis;
      secondPeep = !secondPeep;  // Toggle peep state
    }
  } else {
    // Single beep at regular intervals
    if (currentMillis - lastBuzzerTime >= interval) {
      tone(tonepin, tonoAlarm1, 100);  // Play tone for 100ms
      lastBuzzerTime = currentMillis;
    }
  }
}

// LED RING ANIMATIONS

// Define Notes for Peep Frequencies
const int peepFrequencies[] = { 700, 710, 720, 740, 760, 780 };
const int peepCount = sizeof(peepFrequencies) / sizeof(peepFrequencies[0]);

// Define Timings
const unsigned long firstPeepTime = 200;
const unsigned long secondPeepTime = 200;
const unsigned long singlePeepOnTime = 140;   // Time the peep is on
const unsigned long singlePeepOffTime = 120;  // Time between peeps

// Arming Animation: Green -> Yellow -> Red
void armingAnimationLEDRing(Adafruit_NeoPixel& ring, unsigned long startTime, unsigned long totalTimeMillis) {
  unsigned int percent = updateProgressBar(startTime, totalTimeMillis);
  int segment = (percent * NUMPIXELS) / 100;

  static bool quickPeepsDone = false;  // Flag for the initial two peeps
  static unsigned long lastPeepTime = 0;
  static int randomizedPeepOrder[peepCount];
  static int currentPeepIndex = 0;

  unsigned long elapsedTime = millis() - startTime;

  // Handle the first two peeps
  if (!quickPeepsDone) {
    if (elapsedTime < firstPeepTime) {
      tone(tonepin, 523);  // First peep
    } else if (elapsedTime < firstPeepTime + secondPeepTime) {
      tone(tonepin, 523);  // Second peep
    } else {
      noTone(tonepin);          // Stop peep
      quickPeepsDone = true;    // Mark quick peeps as done
      lastPeepTime = millis();  // Initialize timing for single peeps
    }
    return;  // Return early while handling the first two peeps
  }

  // Handle single peeps
  if (elapsedTime < totalTimeMillis) {
    if (millis() - lastPeepTime >= singlePeepOnTime + singlePeepOffTime) {
      // Generate a random order of peeps at the start of the cycle
      if (currentPeepIndex == 0) {
        memcpy(randomizedPeepOrder, peepFrequencies, sizeof(peepFrequencies));
        for (int i = 0; i < peepCount; i++) {
          int randomIndex = random(i, peepCount);
          int temp = randomizedPeepOrder[i];
          randomizedPeepOrder[i] = randomizedPeepOrder[randomIndex];
          randomizedPeepOrder[randomIndex] = temp;
        }
      }

      // Play the current peep
      tone(tonepin, randomizedPeepOrder[currentPeepIndex]);
      delay(singlePeepOnTime);  // Play peep for the specified duration
      noTone(tonepin);

      // Update timing and move to the next peep
      lastPeepTime = millis();
      currentPeepIndex = (currentPeepIndex + 1) % peepCount;
    }
  } else {
    noTone(tonepin);  // Stop all tones when the animation ends
  }

  // Handle LED animation
  for (int i = 0; i < NUMPIXELS; i++) {
    if (i < segment) {
      if (i < 4) {
        ring.setPixelColor(i, ring.Color(0, 255, 0));  // Green
      } else if (i < 8) {
        ring.setPixelColor(i, ring.Color(255, 255, 0));  // Yellow
      } else {
        ring.setPixelColor(i, ring.Color(255, 0, 0));  // Red
      }
    } else {
      ring.setPixelColor(i, ring.Color(0, 0, 0));  // Off
    }
  }
  ring.show();
}

// Disarm Animation: Red -> Yellow -> Green
void disarmAnimationLEDRing(Adafruit_NeoPixel& ring, unsigned long startTime, unsigned long totalTimeMillis) {
  unsigned int percent = updateProgressBar(startTime, totalTimeMillis);
  int segment = (percent * NUMPIXELS) / 100;

  static bool quickPeepsDone = false;  // Flag for the initial two peeps
  static unsigned long lastPeepTime = 0;
  static int randomizedPeepOrder[peepCount];
  static int currentPeepIndex = 0;

  unsigned long elapsedTime = millis() - startTime;

  // Handle the first two peeps
  if (!quickPeepsDone) {
    if (elapsedTime < firstPeepTime) {
      tone(tonepin, 523);  // First peep
    } else if (elapsedTime < firstPeepTime + secondPeepTime) {
      tone(tonepin, 523);  // Second peep
    } else {
      noTone(tonepin);          // Stop peep
      quickPeepsDone = true;    // Mark quick peeps as done
      lastPeepTime = millis();  // Initialize timing for single peeps
    }
    return;  // Return early while handling the first two peeps
  }

  // Handle single peeps
  if (elapsedTime < totalTimeMillis) {
    if (millis() - lastPeepTime >= singlePeepOnTime + singlePeepOffTime) {
      // Generate a random order of peeps at the start of the cycle
      if (currentPeepIndex == 0) {
        memcpy(randomizedPeepOrder, peepFrequencies, sizeof(peepFrequencies));
        for (int i = 0; i < peepCount; i++) {
          int randomIndex = random(i, peepCount);
          int temp = randomizedPeepOrder[i];
          randomizedPeepOrder[i] = randomizedPeepOrder[randomIndex];
          randomizedPeepOrder[randomIndex] = temp;
        }
      }

      // Play the current peep
      tone(tonepin, randomizedPeepOrder[currentPeepIndex]);
      delay(singlePeepOnTime);  // Play peep for the specified duration
      noTone(tonepin);

      // Update timing and move to the next peep
      lastPeepTime = millis();
      currentPeepIndex = (currentPeepIndex + 1) % peepCount;
    }
  } else {
    noTone(tonepin);  // Stop all tones when the animation ends
  }

  // Handle LED animation
  for (int i = 0; i < NUMPIXELS; i++) {
    if (i < segment) {
      if (i < 4) {
        ring.setPixelColor(i, ring.Color(255, 0, 0));  // Red
      } else if (i < 8) {
        ring.setPixelColor(i, ring.Color(255, 255, 0));  // Yellow
      } else {
        ring.setPixelColor(i, ring.Color(0, 255, 0));  // Green
      }
    } else {
      ring.setPixelColor(i, ring.Color(0, 0, 0));  // Off
    }
  }
  ring.show();
}

// Armed Blinking Blue with Tone
void ringLEDBlinkBlue(Adafruit_NeoPixel& ring, unsigned long currentMillis, int buzzerFrequency) {
  bool ledsOn = (currentMillis / 1000) % 2 == 0;

  // Update LEDs
  for (int i = 0; i < NUMPIXELS; i++) {
    ring.setPixelColor(i, ledsOn ? ring.Color(0, 26, 255) : ring.Color(0, 0, 0));  // Blue
  }
  ring.show();
}

// Armed Blinking Yellow with Tone
void ringLEDBlinkYellow(Adafruit_NeoPixel& ring, unsigned long currentMillis, int buzzerFrequency) {
  bool ledsOn = (currentMillis / 1000) % 2 == 0;

  // Update LEDs
  for (int i = 0; i < NUMPIXELS; i++) {
    ring.setPixelColor(i, ledsOn ? ring.Color(255, 239, 0) : ring.Color(0, 0, 0));  // Yellow
  }
  ring.show();
}

// Team 1 Arming Animation: Yellow
void armAnimaLEDRingY(Adafruit_NeoPixel& ring, unsigned long startTime, unsigned long totalTimeMillis) {
  unsigned int percent = updateProgressBar(startTime, totalTimeMillis);
  int segment = (percent * NUMPIXELS) / 100;

  for (int i = 0; i < NUMPIXELS; i++) {
    if (i < segment) {
      ring.setPixelColor(i, ring.Color(255, 239, 0));  // Yellow
    } else {
      ring.setPixelColor(i, ring.Color(0, 0, 0));  // Off
    }
  }
  ring.show();
}

// Team 2 Arming Animation: Blue
void armAnimaLEDRingB(Adafruit_NeoPixel& ring, unsigned long startTime, unsigned long totalTimeMillis) {
  unsigned int percent = updateProgressBar(startTime, totalTimeMillis);
  int segment = (percent * NUMPIXELS) / 100;

  for (int i = 0; i < NUMPIXELS; i++) {
    if (i < segment) {
      ring.setPixelColor(i, ring.Color(0, 26, 255));  // Blue
    } else {
      ring.setPixelColor(i, ring.Color(0, 0, 0));  // Off
    }
  }
  ring.show();
}

// Neutral Arming Animation: White
void armAnimaLEDRingW(Adafruit_NeoPixel& ring, unsigned long startTime, unsigned long totalTimeMillis) {
  unsigned int percent = updateProgressBar(startTime, totalTimeMillis);
  int segment = (percent * NUMPIXELS) / 100;

  for (int i = 0; i < NUMPIXELS; i++) {
    if (i < segment) {
      ring.setPixelColor(i, ring.Color(225, 225, 255));  // Blue
    } else {
      ring.setPixelColor(i, ring.Color(0, 0, 0));  // Off
    }
  }
  ring.show();
}

// Neutral Blinking Logic (No Sound)
void ringLEDNeutral(Adafruit_NeoPixel& ring, unsigned long currentMillis) {
  bool ledsOn = (currentMillis / 1000) % 2 == 0;

  for (int i = 0; i < NUMPIXELS; i++) {
    ring.setPixelColor(i, ledsOn ? ring.Color(225, 225, 255) : ring.Color(0, 0, 0));  // White
  }
  ring.show();
}

// Main Blinking Logic
void handleBlinking(unsigned long totalTimeMillis, unsigned long elapsedTime) {
  unsigned long currentMillis = millis();

  if (ringBlinkingRed) {
    syncLEDAndBuzzer(ring1, currentMillis, totalTimeMillis, elapsedTime, tonoAlarm2);
    syncLEDAndBuzzer(ring2, currentMillis, totalTimeMillis, elapsedTime, tonoAlarm1);
  }

  if (ring2Blinking) {
    ringLEDBlinkBlue(ring2, currentMillis, tonoAlarm1);
  }

  if (ring1Blinking) {
    ringLEDBlinkYellow(ring1, currentMillis, tonoAlarm1);
  }

  if (ringNeutralBlinking) {
    ringLEDNeutral(ring1, currentMillis);
    ringLEDNeutral(ring2, currentMillis);
  }
}

// Synchronize LED and Buzzer for Zones with Sound
void syncLEDAndBuzzer(Adafruit_NeoPixel& ring, unsigned long currentMillis, unsigned long totalTimeMillis, unsigned long elapsedTime, int buzzerFrequency) {
  unsigned long remainingTime = totalTimeMillis > elapsedTime ? totalTimeMillis - elapsedTime : 0;

  unsigned long interval;
  unsigned long toneDuration;

  if (remainingTime > 20000) {
    // Before 20 seconds: 2 seconds interval, long peeps
    interval = 2000;
    toneDuration = 500;
  } else if (remainingTime > 10000) {
    // Between 20 and 10 seconds: 1 second interval, long peeps
    interval = 1000;
    toneDuration = 300;
  } else {
    // After 10 seconds: Speed up with current curved logic
    float progress = (10000.0f - remainingTime) / 10000.0f;  // Progress from 0 to 1 in last 10 seconds
    interval = 1000 - (progress * progress * 500);           // Gradual curve, minimum 500ms
    toneDuration = interval / 2;                             // Shorter peeps
  }

  bool isOn = (currentMillis / interval) % 2 == 0;

  // Update LEDs
  for (int i = 0; i < NUMPIXELS; i++) {
    ring.setPixelColor(i, isOn ? ring.Color(255, 0, 0) : ring.Color(0, 0, 0));  // Red
  }
  ring.show();

  // Update Buzzer
  if (isOn) {
    tone(tonepin, tonoAlarm2, interval / 2);  // Play tone for half the interval
  } else {
    noTone(tonepin);  // Turn off buzzer
  }
}