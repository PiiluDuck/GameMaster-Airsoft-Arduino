#include "LCDutils.h"
#include "localization.h"

void playBuzzer(int frequency, unsigned long interval, bool twoPeeps = false);

// Domination Game Mode
void domination() {
  doStatus = true;
  demineer = false;
  bool buttonReleasedAfterAction = true;

  unsigned long gameStartTime = millis();
  unsigned long gameEndTime = gameStartTime + (unsigned long)GAMEMINUTES * 60 * 1000;

  unsigned long lastUpdateTime = millis();
  unsigned long team1StartTime = 0, team2StartTime = 0;
  unsigned long team1TotalTime = 0, team2TotalTime = 0;

  bool team1Zone = false, team2Zone = false, neutralZone = true;
  printLCDFromPROGMEM(startGame);
  playBuzzer(tonepin, tonoAlarm1, 1000);  // Play a 1-second peep
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
      if (doStatus) {
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
          printLCDFlash(reinterpret_cast<const __FlashStringHelper*>(alreadteam1Active));
          delay(800);
        } else if (team2Zone) {
          handleZoneNeutralization(team1Zone, team2Zone, neutralZone);
        } else if (neutralZone) {
          handleZoneLogic(true, false, reinterpret_cast<const char*>(team1Text), team1Zone, team2Zone, neutralZone, team1StartTime, team1TotalTime);
        }
        buttonReleasedAfterAction = false;
      }
    }

    // Handle Team 2 Actions
    if (key == 'd' && demineer) {
      if (buttonReleasedAfterAction) {
        if (team2Zone) {
          printLCDFlash(reinterpret_cast<const __FlashStringHelper*>(alreadteam1Active));
          delay(800);
        } else if (team1Zone) {
          handleZoneNeutralization(team1Zone, team2Zone, neutralZone);
        } else if (neutralZone) {
          handleZoneLogic(false, true, reinterpret_cast<const char*>(team2Text), team1Zone, team2Zone, neutralZone, team2StartTime, team2TotalTime);
        }
        buttonReleasedAfterAction = false;
      }
    }

    if (!demineer) buttonReleasedAfterAction = true;

    // LCD Update
    if (currentMillis - lastLCDUpdate >= lcdUpdateInterval) {
      lastLCDUpdate = currentMillis;

      if (neutralZone) {
        printLCDFlash(reinterpret_cast<const __FlashStringHelper*>(neutZone));
        printTimeToLCD((gameEndTime > currentMillis) ? (gameEndTime - currentMillis) : 0, 1);
        ringNeutralBlinking = true;
        ring1Blinking = false;
        ring2Blinking = false;
      } else {
        // Active Zone Display
        displayTeamActiveTime(team1Zone, team2Zone, team1TotalTime, team2TotalTime, neutralZone, gameEndTime);

        // Update LED Blinking States
        ring1Blinking = team1Zone;
        ring2Blinking = team2Zone;
        ringNeutralBlinking = neutralZone;
      }
    }
    // Call blinking logic
    handleBlinking(gameEndTime - gameStartTime, currentMillis - gameStartTime);

    // End Game Logic
    if (currentMillis >= gameEndTime) {
      ring1.clear();
      ring1.show();
      ring2.clear();
      ring2.show();
      printLCDFromPROGMEM(timeOverText), (gameOverText);
      mosfetEnable = true;
      activateMosfet_2();
      delay(10000);
      dominationSplash(team1TotalTime, team2TotalTime);
      doStatus = false;
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

  printLCDFromPROGMEM(neut);

  unsigned long armingTimeMillis = ACTIVATESECONDS * 1000;
  unsigned long startTime = millis();
  unsigned int percent = 0;

  arming = true;
  char neutralizingTeam = '\0';  // Track the team performing neutralization

  while (true) {
    char key = keypad.getKey();
    if (key) keypadEvent(key);

    if (activeRing == &ring1 && demineer) {
      neutralizingTeam = 'c';  // Team 1
    } else if (activeRing == &ring2 && demineer) {
      neutralizingTeam = 'd';  // Team 2;
    }

    if (!demineer) {
      arming = false;

      ring1Blinking = false;
      ring2Blinking = false;
      ringNeutralBlinking = false;

      ring1.clear();
      ring1.show();
      ring2.clear();
      ring2.show();

      printLCDFlash(reinterpret_cast<const __FlashStringHelper*>(neutReset));
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
      //sendBlynkProgressBar(percent);
      if (neutralizingTeam == 'c') {
        armAnimaLEDRingW(ring2, startTime, armingTimeMillis);  // Update LED animation
      }
      if (neutralizingTeam == 'd') {
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

      printLCDFlash(reinterpret_cast<const __FlashStringHelper*>(neutZone));

      // If the same button remains held, start arming the same team
      if (neutralizingTeam == 'c' && demineer && activeRing == &ring1) {
        handleZoneLogic(true, false, reinterpret_cast<const char*>(team1Text), team1Zone, team2Zone, neutralZone, team1StartTime, team1TotalTime);
      } else if (neutralizingTeam == 'd' && demineer && activeRing == &ring2) {
        handleZoneLogic(false, true, reinterpret_cast<const char*>(team2Text), team1Zone, team2Zone, neutralZone, team2StartTime, team2TotalTime);
      }
      return;
    }

    delay(50);
  }
}

// Handle Zone Logic (for capturing)
void handleZoneLogic(bool activateTeam1, bool activateTeam2, const char* teamName, bool& team1Zone, bool& team2Zone, bool& neutralZone, unsigned long& zoneStartTime, unsigned long& teamTotalTime) {
  char armingMessage[17];
  char teamBuffer[10];
  char armingBuffer[10];

  // Copy "Yellow"/"Blue" and "Arming" from PROGMEM to RAM
  if (activateTeam1) {
    strncpy_P(teamBuffer, team1Text, sizeof(teamBuffer) - 1);
  } else if (activateTeam2) {
    strncpy_P(teamBuffer, team2Text, sizeof(teamBuffer) - 1);
  }
  strncpy_P(armingBuffer, armingText, sizeof(armingBuffer) - 1);

  // Null-terminate the buffers
  teamBuffer[sizeof(teamBuffer) - 1] = '\0';
  armingBuffer[sizeof(armingBuffer) - 1] = '\0';

  // Combine the strings
  snprintf(armingMessage, sizeof(armingMessage), "%s %s", teamBuffer, armingBuffer);

  // Use the dynamic message for the LCD
  printLCDFlash(armingMessage, (const char*)nullptr);
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

      printLCDFlash(reinterpret_cast<const __FlashStringHelper*>(armReset));
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
      } else if (activateTeam2) {
        team2Zone = true;
        team1Zone = false;
        neutralZone = false;
        zoneStartTime = millis();

        ring2Blinking = true;
        ring1Blinking = false;
        ringNeutralBlinking = false;
      }

      // Zone captured; let the main logic handle updates and display
      return;
    }
    delay(50);
  }
}

void displayTeamActiveTime(bool team1Zone, bool team2Zone, unsigned long team1Time, unsigned long team2Time, bool neutralZone, unsigned long gameEndTime) {
  static unsigned long lastToggleTime = millis();  // Track the last toggle time
  static bool showCaptureTime = true;              // Toggle state between capture and remaining time
  unsigned long currentMillis = millis();

  // Alternate between "Capture Time" and "Remaining Time" every 2 seconds
  if (currentMillis - lastToggleTime >= 2000) {
    showCaptureTime = !showCaptureTime;
    lastToggleTime = currentMillis;
  }

  char team1Buffer[17] = { 0 };
  char team2Buffer[17] = { 0 };
  char remainingBuffer[17] = { 0 };

  // Format time strings
  formatTimeFull(team1Time, team1Buffer, sizeof(team1Buffer), false);
  formatTimeFull(team2Time, team2Buffer, sizeof(team2Buffer), false);

  unsigned long remainingTime = (gameEndTime > currentMillis) ? (gameEndTime - currentMillis) : 0;
  formatTimeFull(remainingTime, remainingBuffer, sizeof(remainingBuffer), false);

  // Update the LCD display
  if (neutralZone) {
    // Neutral Zone Display
    printLCDFlash(reinterpret_cast<const __FlashStringHelper*>(neutZone), remainingBuffer);
  } else if (team1Zone) {
    if (showCaptureTime) {
      // Display Team 1 Capture Time
      printLCDFlash(reinterpret_cast<const __FlashStringHelper*>(team1Active), team1Buffer);
    } else {
      // Display Remaining Game Time
      printLCDFlash(reinterpret_cast<const __FlashStringHelper*>(remTime), remainingBuffer);
    }
  } else if (team2Zone) {
    if (showCaptureTime) {
      // Display Team 2 Capture Time
      printLCDFlash(reinterpret_cast<const __FlashStringHelper*>(team2Active), team2Buffer);
    } else {
      // Display Remaining Game Time
      printLCDFlash(reinterpret_cast<const __FlashStringHelper*>(remTime), remainingBuffer);
    }
  }
}
