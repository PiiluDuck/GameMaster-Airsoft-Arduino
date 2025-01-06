void formatTimeFull(unsigned long timeMillis, char* buffer, size_t bufferSize, bool includeMillis = true);
//void updateRedLED(unsigned long refTime);
//void updateGreenLED(unsigned long refTime);

// Before Bomb is Armed
void search() {
  demineer = false;
  searchAndDestroyMode = true;
  buttonHeld = false;
  // digitalWrite(REDLED, LOW);
  // digitalWrite(GREENLED, LOW);

  unsigned long totalTimeMillis = (unsigned long)GAMEMINUTES * 60 * 1000;  // Total game time in milliseconds
  unsigned long startTime = millis();

  bool buttonReleasedAfterArming = true;

  printLCDFlash(F("Game Start"), "");
  tone(tonepin, tonoAlarm1, 1000);  // Play a 1-second peep
  delay(1050);

  ringBlinkingRed = false;
  ringNeutralBlinking = true;

  while (true) {
    char key = keypad.getKey();
    if (key) {
      keypadEvent(key);
    }

    // Handle arming logic
    if (demineer) {
      if (buttonReleasedAfterArming) {
        handleArmingLogic();
        buttonReleasedAfterArming = false;
      }
    } else {
      buttonReleasedAfterArming = true;
    }

    // Calculate elapsed time and remaining time
    unsigned long elapsed = millis() - startTime;
    unsigned long remainingTime = (elapsed >= totalTimeMillis) ? 0 : (totalTimeMillis - elapsed);

    handleBlinking(totalTimeMillis, elapsed);

    // Last minute logic: peep every 3 seconds
    if (remainingTime <= 60000) {  // Last minute
      static unsigned long lastPeepTime = 0;
      if (millis() - lastPeepTime >= 3000) {  // 3-second interval
        tone(tonepin, tonoAlarm1, 100);       // Peep for 100ms
        lastPeepTime = millis();
      }
    }

    // End game if time is up
    if (remainingTime == 0) {
      ring1.clear();
      ring1.show();
      ring2.clear();
      ring2.show();
      printLCDFlash(F("Game Over!"), F("Time Expired!"));
            mosfetEnable = true;
      activateMosfet_2();
      delay(10000);
      endSplash();
      return;
    }

    // Update remaining time display
    static unsigned long lastDisplayedTime = ULONG_MAX;  // Initialize to an impossible value

    // Only update the LCD if the remaining time has changed
    if (remainingTime / 1000 != lastDisplayedTime / 1000) {  // Check seconds only
      char timeBuffer[17];
      formatTimeFull(remainingTime, timeBuffer, sizeof(timeBuffer), false);
      printLCDFlash(F("Remaining Time"), timeBuffer);

      lastDisplayedTime = remainingTime;  // Update the last displayed time
    }
  }
}

// Once Bomb is Armed
void destroy() {
  demineer = false;
  searchAndDestroyMode = true;
  buttonHeld = false;
  printLCDFlash(F("Bomb Armed"), F(""));
  ring1.clear();
  ring1.show();
  ring2.clear();
  ring2.show();
  delay(1000);

  unsigned long totalTimeMillis = (unsigned long)BOMBMINUTES * 60 * 1000;
  unsigned long startTime = millis();
  bool buttonReleasedAfterPhase = true;

  ringBlinkingRed = true;
  ringNeutralBlinking = false;

  while (true) {
    char key = keypad.getKey();
    if (key) {
      keypadEvent(key);
    }

    // Handle disarming logic
    if (demineer) {
      if (buttonReleasedAfterPhase) {
        handleDisarmingLogic(BOMBMINUTES, startTime);
        buttonReleasedAfterPhase = false;
      }
    } else {
      buttonReleasedAfterPhase = true;
    }

    // Calculate elapsed time and remaining time
    unsigned long elapsed = millis() - startTime;
    unsigned long remainingTime = (elapsed >= totalTimeMillis) ? 0 : (totalTimeMillis - elapsed);

    handleBlinking(totalTimeMillis, elapsed);

    // Trigger explosion if time is up
    if (remainingTime == 0) {
      ring1.clear();
      ring1.show();
      ring2.clear();
      ring2.show();
      printLCDFlash(F("Bomb Exploded!"), F("Time Expired!"));
      mosfetEnable = true;
      activateMosfet_1();
      delay(10000);
      explodeSplash();
      return;
    }

    //Remaining time display
    static unsigned long lastDisplayedTime = ULONG_MAX;  // Initialize to an impossible value

    if (remainingTime / 1000 != lastDisplayedTime / 1000) {  // Check seconds only
      char timeBuffer[17];
      formatTimeFull(remainingTime, timeBuffer, sizeof(timeBuffer), false);
      printLCDFlash(F("Remaining Time"), timeBuffer);

      lastDisplayedTime = remainingTime;  // Update the last displayed time
    }
  }
}

// Handle Arming Logic
void handleArmingLogic() {
  printLCDFlash(F("Arming Bomb"), F(""));
  ring1.clear();
  ring1.show();
  ring2.clear();
  ring2.show();

  unsigned long armingTimeMillis = ACTIVATESECONDS * 1000;
  unsigned long startTime = millis();
  unsigned int percent = 0;

  while (demineer) {
    char key = keypad.getKey();
    if (key) keypadEvent(key);

    if (!demineer) {
      printLCDFlash(F("Arming Reset!"), F(""));
      delay(800);
      return;
    }

    unsigned long currentMillis = millis();

    //updateRedLED(startTime);
    armingAnimationLEDRing(ring1, startTime, armingTimeMillis);
    armingAnimationLEDRing(ring2, startTime, armingTimeMillis);
    percent = updateProgressBar(startTime, armingTimeMillis);

    static unsigned long lastBarUpdateTime = 0;
    if (currentMillis - lastBarUpdateTime >= 50) {
      lcd.setCursor(0, 1);
      drawNativeLCDProgressBar(percent);
      //sendBlynkProgressBar(percent);
      lastBarUpdateTime = currentMillis;
    }

    if (percent >= 100) {
      destroy();
      return;
    }
  }
}

// Handle Disarming Logic
void handleDisarmingLogic(int minut, unsigned long iTime) {
  printLCDFlash(F("Disarming"), F(""));
  ring1.clear();
  ring1.show();
  ring2.clear();
  ring2.show();

  unsigned long disarmingTimeMillis = ACTIVATESECONDS * 1000;
  unsigned long startTime = millis();
  unsigned int percent = 0;

  while (demineer) {
    char key = keypad.getKey();
    if (key) keypadEvent(key);

    if (!demineer) {
      printLCDFlash(F("Disarm Reset!"), F(""));
      delay(800);
      return;
    }

    unsigned long currentMillis = millis();

    // updateGreenLED(startTime);
    disarmAnimationLEDRing(ring1, startTime, disarmingTimeMillis);
    disarmAnimationLEDRing(ring2, startTime, disarmingTimeMillis);
    percent = updateProgressBar(startTime, disarmingTimeMillis);


    static unsigned long lastBarUpdateTime = 0;
    if (currentMillis - lastBarUpdateTime >= 50) {
      lcd.setCursor(0, 1);
      drawNativeLCDProgressBar(percent);
      //sendBlynkProgressBar(percent);
      lastBarUpdateTime = currentMillis;
    }

    if (percent >= 100) {
      ring1.clear();
      ring1.show();
      ring2.clear();
      ring2.show();
      printLCDFlash(F("Bomb Disarmed!"), F("Game Over!"));
      mosfetEnable = true;
      activateMosfet_2();
      delay(10000);
      disarmedSplash();
      return;
    }
  }
}
