void formatTimeFull(unsigned long timeMillis, char* buffer, size_t bufferSize, bool includeMillis = true);
//void updateRedLED(unsigned long refTime);
//void updateGreenLED(unsigned long refTime);
void playBuzzer(int frequency, unsigned long interval, bool twoPeeps = false);

// Domination Game Mode
void domination() {
  dominationMode = true;
  demineer = false;
  bool buttonReleasedAfterAction = true;

  unsigned long gameStartTime = millis();
  unsigned long gameEndTime = gameStartTime + (unsigned long)GAMEMINUTES * 60 * 1000;

  unsigned long lastUpdateTime = millis();
  unsigned long team1StartTime = 0, team2StartTime = 0;
  unsigned long team1TotalTime = 0, team2TotalTime = 0;

  bool team1Zone = false, team2Zone = false, neutralZone = true;

  printLCDFlash(F("Game Start"), "");
  tone(tonepin, tonoAlarm1, 1000);  // Play a 1-second peep
  delay(1050);

  // Start Neutral blinking
  ring1Blinking = false;
  ring2Blinking = false;
  ringNeutralBlinking = true;

  unsigned long lastLCDUpdate = millis();
  const unsigned long lcdUpdateInterval = 1000;

  while (true) {
    unsigned long currentMillis = millis();

    // Keypad Input
    char key = keypad.getKey();
    if (key) {
      if (dominationMode) {
        if (key == 'c') demineer = true;  // Team 1
        if (key == 'd') demineer = true;  // Team 2
      } else {
        keypadEvent(key);
      }
    }

    updateTeamTime(team1Zone, team2Zone, team1TotalTime, team2TotalTime, lastUpdateTime, neutralZone);

    // Handle Team 1 Actions
    if (key == 'c' && demineer) {
      if (buttonReleasedAfterAction) {
        if (team1Zone) {
          printLCDFlash("Already Active", nullptr);
          delay(800);
        } else if (team2Zone) {
          handleZoneNeutralization(team1Zone, team2Zone, neutralZone);
        } else if (neutralZone) {
          handleZoneLogic(true, false, "Yellow", team1Zone, team2Zone, neutralZone, team1StartTime, team1TotalTime);
        }
        buttonReleasedAfterAction = false;
      }
    }

    // Handle Team 2 Actions
    if (key == 'd' && demineer) {
      if (buttonReleasedAfterAction) {
        if (team2Zone) {
          printLCDFlash("Already Active", nullptr);
          delay(800);
        } else if (team1Zone) {
          handleZoneNeutralization(team1Zone, team2Zone, neutralZone);
        } else if (neutralZone) {
          handleZoneLogic(false, true, "Blue", team1Zone, team2Zone, neutralZone, team2StartTime, team2TotalTime);
        }
        buttonReleasedAfterAction = false;
      }
    }

    if (!demineer) buttonReleasedAfterAction = true;

    // LCD Update
    if (currentMillis - lastLCDUpdate >= lcdUpdateInterval) {
      lastLCDUpdate = currentMillis;

      if (neutralZone) {
        printLCDFlash("Neutral Zone", nullptr);
        printTimeToLCD((gameEndTime > currentMillis) ? (gameEndTime - currentMillis) : 0, 1);
        ringNeutralBlinking = true;
        ring1Blinking = false;
        ring2Blinking = false;
      } else {
        displayTeamActiveTime(team1Zone, team2Zone, team1TotalTime, team2TotalTime, neutralZone);
        ring1Blinking = team1Zone;
        ring2Blinking = team2Zone;
        ringNeutralBlinking = neutralZone;
      }
    }
    
    // Call blinking logic
    handleBlinking(gameEndTime - gameStartTime, currentMillis - gameStartTime);

    // End Game Logic
    if (currentMillis >= gameEndTime) {
      activateMosfet_2();
      printLCDFlash(F("Times Up!"), F("Game Over!"));
      delay(5000);
      ring1.clear();
      ring1.show();
      ring2.clear();
      ring2.show();
      dominationSplash(team1TotalTime, team2TotalTime);
      dominationMode = false;
      return;
    }

    delay(10);
  }
}

// Handle Zone Neutralization
void handleZoneNeutralization(bool& team1Zone, bool& team2Zone, bool& neutralZone) {
  ring1.clear();
  ring1.show();
  ring2.clear();
  ring2.show();

  printLCDFlash(F("Neutralizing"), "");

  unsigned long armingTimeMillis = ACTIVATESECONDS * 1000;
  unsigned long startTime = millis();
  unsigned int percent = 0;

  arming = true;

  while (true) {
    char key = keypad.getKey();
    if (key) keypadEvent(key);

    if (!demineer) {
      arming = false;

      ring1Blinking = false;
      ring2Blinking = false;
      ringNeutralBlinking = false;

      ring1.clear();
      ring1.show();
      ring2.clear();
      ring2.show();

      printLCDFlash("Neutral Reset!", nullptr);
      delay(800);

      if (team1Zone) {
        printLCDFlash(F("Yellow Zone"), "");
      } else if (team2Zone) {
        printLCDFlash(F("Blue Zone"), "");
      }
      return;
    }

    playBuzzer(tonoAlarm1, 500);

    percent = updateProgressBar(startTime, armingTimeMillis);
    static unsigned long lastBarUpdateTime = 0;
    unsigned long currentMillis = millis();
    if (currentMillis - lastBarUpdateTime >= 50) {
      lcd.setCursor(0, 1);
      drawNativeLCDProgressBar(percent);
      //sendBlynkProgressBar(percent);
      if (activeRing == &ring1) {
        armAnimaLEDRingW(ring2, startTime, armingTimeMillis);  // Update LED animation
      }
      if (activeRing == &ring2) {
        armAnimaLEDRingW(ring1, startTime, armingTimeMillis);  // Update LED animation
      }
      lastBarUpdateTime = currentMillis;
    }

    if (percent >= 100) {
      team1Zone = false;
      team2Zone = false;
      neutralZone = true;
      arming = false;

      ring1Blinking = false;
      ring2Blinking = false;
      ringNeutralBlinking = true;

      printLCDFlash("Neutral Zone", nullptr);
      return;
    }

    delay(50);
  }
}

// Handle Zone Logic (for capturing)
void handleZoneLogic(bool activateTeam1, bool activateTeam2, const char* teamName, bool& team1Zone, bool& team2Zone, bool& neutralZone, unsigned long& zoneStartTime, unsigned long& teamTotalTime) {
  char armingMessage[17];
  snprintf(armingMessage, sizeof(armingMessage), "%s Arming", teamName);

  printLCDFlash(armingMessage, nullptr);  // Use the dynamic message for LCD
  ring1.clear();
  ring1.show();
  ring2.clear();
  ring2.show();

  unsigned long armingTimeMillis = ACTIVATESECONDS * 1000;
  unsigned long startTime = millis();
  unsigned int percent = 0;

  arming = true;

  while (true) {
    char key = keypad.getKey();
    if (key) keypadEvent(key);

    if (!demineer) {
      arming = false;

      ring1Blinking = false;
      ring2Blinking = false;
      ringNeutralBlinking = false;

      ring1.clear();
      ring1.show();
      ring2.clear();
      ring2.show();

      printLCDFlash("Arming Reset!", nullptr);
      delay(800);
      return;
    }

    playBuzzer(tonoAlarm1, 500);

    percent = updateProgressBar(startTime, armingTimeMillis);
    static unsigned long lastBarUpdateTime = 0;
    unsigned long currentMillis = millis();
    if (currentMillis - lastBarUpdateTime >= 50) {
      lcd.setCursor(0, 1);
      drawNativeLCDProgressBar(percent);
      if (activateTeam1) {
        armAnimaLEDRingY(ring1, startTime, armingTimeMillis);  // Yellow animation for Team 1
      }
      if (activateTeam2) {
        armAnimaLEDRingB(ring2, startTime, armingTimeMillis);  // Blue animation for Team 2
      }
      lastBarUpdateTime = currentMillis;
    }

    if (percent >= 100) {
      arming = false;

      if (activateTeam1) {
        team1Zone = true;
        team2Zone = false;
        neutralZone = false;
        zoneStartTime = millis();

        ring1Blinking = true;
        ring2Blinking = false;
        ringNeutralBlinking = false;

        printLCDFlash("Yellow Zone", nullptr);
      } else if (activateTeam2) {
        team2Zone = true;
        team1Zone = false;
        neutralZone = false;
        zoneStartTime = millis();

        ring2Blinking = true;
        ring1Blinking = false;
        ringNeutralBlinking = false;

        printLCDFlash("Blue Zone", nullptr);
      }
      return;
    }
    delay(50);
  }
}
