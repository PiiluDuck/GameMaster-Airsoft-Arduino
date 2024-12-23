#define MOSFET_TIME 200
#define MOSFET_TIME_OFF 50

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

// Draw progress bar to send to Blynk
/*void sendBlynkProgressBar(byte percent) {
  char blynkBar[17] = "";         // Create a text-based progress bar for Blynk
  for (int i = 0; i < 16; i++) {  // Simulate a 16-character progress bar
    if (i < (percent / 6.25)) {   // Scale percentage to 16 characters
      blynkBar[i] = '#';          // Completed part
    } else {
      blynkBar[i] = ' ';  // Remaining part
    }
  }
  blynkBar[16] = '\0';  // Null-terminate the string

  // Send the progress bar to Blynk
  sendToESP("", blynkBar);  // Send only the progress bar to the second line
}*/

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

void displayTeamActiveTime(bool team1Zone, bool team2Zone, unsigned long team1Time, unsigned long team2Time, bool neutralZone) {
  char team1Buffer[9];
  char team2Buffer[9];
  formatTimeFull(team1Time, team1Buffer, sizeof(team1Buffer), false);
  formatTimeFull(team2Time, team2Buffer, sizeof(team2Buffer), false);

  if (neutralZone) {
    printLCDFlash(F("Neutral Zone"), F(""));
  } else if (team1Zone) {
    printLCDFlash(F("Team 1 Active"), team1Buffer);
  } else if (team2Zone) {
    printLCDFlash(F("Team 2 Active"), team2Buffer);
  }
}

// Calculate Dynamic Interval for Buzzer
unsigned long calculateDynamicInterval(unsigned int percent, unsigned long maxInterval, unsigned long minInterval) {
  return maxInterval - (percent * (maxInterval - minInterval) / 100);
}

// Activate MOSFET for a specified duration
void activateMosfet() {
  if (!mosfetEnable) return;  // Exit if MOSFET is not enabled

  unsigned long startMillis = millis(); // Record the start time
  unsigned long lastToggleMillis = 0;   // Track the last toggle time
  bool mosfetState = false;             // Keep track of the MOSFET state (on/off)

  while (millis() - startMillis < 5000) {  // Run for 5 seconds
    unsigned long currentMillis = millis();

    // Check if it's time to toggle the MOSFET state
    if (mosfetState && (currentMillis - lastToggleMillis >= MOSFET_TIME)) {
      digitalWrite(mosfet, LOW);  // Turn the MOSFET off
      mosfetState = false;        // Update state
      lastToggleMillis = currentMillis; // Update the last toggle time
    } else if (!mosfetState && (currentMillis - lastToggleMillis >= MOSFET_TIME_OFF)) {
      digitalWrite(mosfet, HIGH); // Turn the MOSFET on
      mosfetState = true;         // Update state
      lastToggleMillis = currentMillis; // Update the last toggle time
    }
  }

  // Ensure the MOSFET is off at the end
  digitalWrite(mosfet, LOW);
}

void startGameCount() {
  unsigned long countdownMillis = 5000; // 5 seconds total
  unsigned long startMillis = millis();
  unsigned long elapsedMillis = 0;
  const unsigned long interval = 1000; // 1-second interval for peeps

  playBuzzer(tonoAlarm1, 100, false); // Single short beep
  unsigned long lastPeepTime = millis();

  while (elapsedMillis < countdownMillis) {
    unsigned long currentMillis = millis();
    elapsedMillis = currentMillis - startMillis;

    // Calculate progress bar percentage
    byte percent = (elapsedMillis * 100) / countdownMillis;

    // Draw progress bar
    lcd.setCursor(0, 0);
    lcd.print(F("Starting in..."));
    lcd.setCursor(0, 1);
    drawNativeLCDProgressBar(percent);

    // Play a peep every subsequent second
    if (currentMillis - lastPeepTime >= interval) {
      playBuzzer(tonoAlarm1, 100, false); // Single short beep
      lastPeepTime = currentMillis; // Update last peep time
    }

    delay(50); // Small delay to smooth LCD updates
  }

  lcd.clear();
}

//BASIC LED LOGIC

// Update Red LED Blinking Logic
/*void updateRedLED(unsigned long refTime) {
  unsigned long elapsedTime = (millis() - refTime) % 1000;
  if (elapsedTime < 150) {
    digitalWrite(REDLED, HIGH);  // Turn on LED for the first 150ms of the second
  } else {
    digitalWrite(REDLED, LOW);  // Turn off LED for the rest of the second
  }
}*/

// Update Green LED Blinking Logic
/*void updateGreenLED(unsigned long refTime) {
  unsigned long elapsedTime = (millis() - refTime) % 1000;
  if (elapsedTime < 150) {
    digitalWrite(GREENLED, HIGH);  // Turn on LED for the first 150ms of the second
  } else {
    digitalWrite(GREENLED, LOW);  // Turn off LED for the rest of the second
  }
}*/

// BUZZER SETTING

// Buzzer Function
void playBuzzer(int frequency, unsigned long interval, bool twoPeeps = false) {
  static unsigned long lastBuzzerTime = 0;
  static bool secondPeep = false;
  unsigned long currentMillis = millis();

  if (twoPeeps) {
    // Two short peeps every interval
    if (currentMillis - lastBuzzerTime >= (secondPeep ? interval / 2 : interval)) {
      tone(tonepin, frequency, 100);  // Play tone for 100ms
      lastBuzzerTime = currentMillis;
      secondPeep = !secondPeep;  // Toggle peep state
    }
  } else {
    // Single beep at regular intervals
    if (currentMillis - lastBuzzerTime >= interval) {
      tone(tonepin, frequency, 100);  // Play tone for 100ms
      lastBuzzerTime = currentMillis;
    }
  }
}

// LED RING ANIMATIONS

// Arming Animation: Green -> Yellow -> Red
void armingAnimationLEDRing(Adafruit_NeoPixel& ring, unsigned long startTime, unsigned long totalTimeMillis) {
  unsigned int percent = updateProgressBar(startTime, totalTimeMillis);
  int segment = (percent * NUMPIXELS) / 100;

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

// Armed Blinking Blue
void ringLEDBlinkBlue(Adafruit_NeoPixel& ring, unsigned long currentMillis) {
  bool ledsOn = (currentMillis / 1000) % 2 == 0;

  for (int i = 0; i < NUMPIXELS; i++) {
    ring.setPixelColor(i, ledsOn ? ring.Color(0, 26, 255) : ring.Color(0, 0, 0));  // Blue
  }
  ring.show();
}

// Armed Blinking Yellow
void ringLEDBlinkYellow(Adafruit_NeoPixel& ring, unsigned long currentMillis) {
  bool ledsOn = (currentMillis / 1000) % 2 == 0;

  for (int i = 0; i < NUMPIXELS; i++) {
    ring.setPixelColor(i, ledsOn ? ring.Color(255, 239, 0) : ring.Color(0, 0, 0));  // Yellow
  }
  ring.show();
}

// Armed Blinking Red
void ringLEDArmedRed(Adafruit_NeoPixel& ring, unsigned long currentMillis, unsigned long totalTimeMillis, unsigned long elapsedTime) {
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
        float progress = (10000.0f - remainingTime) / 10000.0f; // Progress from 0 to 1 in last 10 seconds
        interval = 1000 - (progress * progress * 500); // Gradual curve, minimum 500ms
        toneDuration = interval / 2; // Shorter peeps
    }

    bool ledsOn = (currentMillis / interval) % 2 == 0;

    for (int i = 0; i < NUMPIXELS; i++) {
        ring.setPixelColor(i, ledsOn ? ring.Color(255, 0, 0) : ring.Color(0, 0, 0));  // Red
    }
    ring.show();
}

// Neutral Blinking Logic (No Sound)
void ringLEDNeutral(Adafruit_NeoPixel& ring, unsigned long currentMillis) {
  bool ledsOn = (currentMillis / 1000) % 2 == 0;

  for (int i = 0; i < NUMPIXELS; i++) {
    ring.setPixelColor(i, ledsOn ? ring.Color(225, 225, 255) : ring.Color(0, 0, 0));  // Neutral Blue
  }
  ring.show();
}

// Main Blinking Logic
void handleBlinking(unsigned long totalTimeMillis, unsigned long elapsedTime) {
  unsigned long currentMillis = millis();

  if (ringBlinkingRed) {
    syncLEDAndBuzzer(ring1, currentMillis, totalTimeMillis, elapsedTime, tonoAlarm1);
    syncLEDAndBuzzer(ring2, currentMillis, totalTimeMillis, elapsedTime, tonoAlarm1);
  }

  if (ring2Blinking) {
    ringLEDBlinkBlue(ring2, currentMillis);
  }

  if (ring1Blinking) {
    ringLEDBlinkYellow(ring1, currentMillis);
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
        float progress = (10000.0f - remainingTime) / 10000.0f; // Progress from 0 to 1 in last 10 seconds
        interval = 1000 - (progress * progress * 500); // Gradual curve, minimum 500ms
        toneDuration = interval / 2; // Shorter peeps
    }

    bool isOn = (currentMillis / interval) % 2 == 0;

    // Update LEDs
    for (int i = 0; i < NUMPIXELS; i++) {
        ring.setPixelColor(i, isOn ? ring.Color(255, 0, 0) : ring.Color(0, 0, 0));  // Red
    }
    ring.show();

    // Update Buzzer
    if (isOn) {
        tone(tonepin, buzzerFrequency, interval / 2);  // Play tone for half the interval
    } else {
        noTone(tonepin);  // Turn off buzzer
    }
}
