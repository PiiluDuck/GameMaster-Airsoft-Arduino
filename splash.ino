// Domination Splash
void dominationSplash(unsigned long team1Time, unsigned long team2Time) {
  char timeBuffer1[9], timeBuffer2[9];
  formatTimeFull(team1Time, timeBuffer1, sizeof(timeBuffer1), false);
  formatTimeFull(team2Time, timeBuffer2, sizeof(timeBuffer2), false);

  const char* winner = team1Time > team2Time ? "YELLOW TEAM" : team2Time > team1Time ? "BLUE TEAM" : "TIE";

  // Initial display for 5 seconds
  unsigned long startMillis = millis();
  while (millis() - startMillis < 5000) {
    printLCDFlashWithBuffers(F("Y:"), timeBuffer1, F("B:"), timeBuffer2);
    delay(2000);
    printLCDFlash(F("WINNER"), winner);
    delay(2000);
  }

  // Infinite loop waiting for user input
  while (true) {
    printLCDFlashWithBuffers(F("Y:"), timeBuffer1, F("B:"), timeBuffer2);
    delay(2000);
    printLCDFlash(F("WINNER"), winner);
    delay(2000);
    waitForDecision();
  }
}

// Disarmed Splash
void disarmedSplash() {
  // Initial display for 5 seconds
  unsigned long startMillis = millis();
  while (millis() - startMillis < 5000) {
    printLCDFlash(F("BOMB DISARMED"), F("GOODS WIN"));
    delay(4000);
  }

  // Infinite loop waiting for user input
  while (true) {
    printLCDFlash(F("BOMB DISARMED"), F("GOODS WIN"));
    delay(4000);
    waitForDecision();
  }
}

// Explode Splash
void explodeSplash() {
  // Initial display for 5 seconds
  unsigned long startMillis = millis();
  while (millis() - startMillis < 5000) {
    printLCDFlash(F("TERRORISTS WIN"), F("GAME OVER"));
    delay(4000);
  }

  // Infinite loop waiting for user input
  while (true) {
    printLCDFlash(F("TERRORISTS WIN"), F("GAME OVER"));
    delay(4000);
    waitForDecision();
  }
}

// End Splash
void endSplash() {
  // Initial display for 5 seconds
  unsigned long startMillis = millis();
  while (millis() - startMillis < 5000) {
    printLCDFlash(F("TIME OVER"), F("GOODS WIN"));
    delay(4000);
  }

  // Infinite loop waiting for user input
  while (true) {
    printLCDFlash(F("TIME OVER"), F("GOODS WIN"));
    delay(4000);
    waitForDecision();
  }
}

// Helper Function to Wait for Decisions
void waitForDecision() {
  while (true) {
    char var = keypad.getKey();
    if (var == 'a') {  // Play Again
      tone(tonepin, 2400, 30);

      resetGameState();  // Reset states

      if (sdStatus) {
        configQuickGame();
        startGameCount();
        search();
      } else if (doStatus) {
        configQuickGame();
        startGameCount();
        domination();
      }
      return;
    } else if (var == 'b') {  // Exit to Main Menu
      tone(tonepin, 2400, 30);

      resetGameState();  // Reset all states
      printLCDFlash(F("Goodbye!"), F(""));
      delay(2000);

      menuPrincipal();  // Redirect to the main menu
      return;
    }
  }
}

// Reset Game State
void resetGameState() {
  soundEnable = true;
  mosfetEnable = false;
  searchAndDestroyMode = false;
  dominationMode = false;
  buttonHeld = false;
  demineer = false;
  cancel = false;
  buttonReleasedAfterPhase = true;
  sdStatus = false;
  doStatus = false;
  arming = false;
  Adafruit_NeoPixel* activeRing = nullptr;
  ring1Blinking = false;
  ring2Blinking = false;
  ringNeutralBlinking = false;
  ringBlinkingRed = false;
  lastLEDUpdateTime = 0;
  tonePlayedForC = false;
  tonePlayedForD = false;
  lastBuzzerTime = 0;
}
