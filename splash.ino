// Domination Splash
void dominationSplash(unsigned long team1Time, unsigned long team2Time) {
  char timeBuffer1[9], timeBuffer2[9];
  formatTimeFull(team1Time, timeBuffer1, sizeof(timeBuffer1), false);
  formatTimeFull(team2Time, timeBuffer2, sizeof(timeBuffer2), false);

  const char* winner = team1Time > team2Time ? "YELLOW" : team2Time > team1Time ? "BLUE"
                                                                                : "TIE";

  // Initial 4 cycles without "Play Again?"
  for (int loop = 0; loop < 4; loop++) {
    // Display team scores for 4 seconds
    printLCDFlashWithBuffers(F("Y:"), timeBuffer1, F("B:"), timeBuffer2);
    delay(2000);

    // Display winner for 2 seconds
    printLCDFlash(F("WINNER"), winner);
    delay(2000);
  }

  // Infinite loop with "Play Again?" prompt
  while (true) {
    printLCDFlashWithBuffers(F("Y:"), timeBuffer1, F("B:"), timeBuffer2);
    delay(2000);
    waitForDecision(2000);

    printLCDFlash(F("WINNER"), winner);
    waitForDecision(2000);

    printLCDFlash(F("Play Again?"), F("A: YES B: NO"));
    waitForDecision(3000);
  }
}

// Helper Function to Allow Decisions During Delays
void waitForDecision(unsigned long durationMillis) {
  unsigned long start = millis();
  while (millis() - start < durationMillis) {
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

// Disarmed Splash
void disarmedSplash() {
  for (int loop = 0; loop < 4; loop++) {
    if (searchAndDestroyMode) {
      printLCDFlash(F("BOMB DISARMED"), F("GOODS WIN"));
     // digitalWrite(GREENLED, HIGH);
      delay(4000);
    }
  }

  while (true) {
    if (searchAndDestroyMode) {
      printLCDFlash(F("BOMB DISARMED"), F("GOODS WIN"));
    //  digitalWrite(GREENLED, HIGH);
      delay(4000);
      waitForDecision(2000);
    }

    printLCDFlash(F("Play Again?"), F("A: YES B: NO"));
    waitForDecision(3000);
  }
}

// Explode Splash
void explodeSplash() {
  for (int loop = 0; loop < 4; loop++) {
    if (searchAndDestroyMode) {
      printLCDFlash(F("TERRORISTS WIN"), F("GAME OVER"));
      delay(4000);
    }
  }

  while (true) {
    if (searchAndDestroyMode) {
      printLCDFlash(F("TERRORISTS WIN"), F("GAME OVER"));
      delay(4000);
      waitForDecision(3000);
    }

    printLCDFlash(F("Play Again?"), F("A: YES B: NO"));
    waitForDecision(3000);
  }
}

// End Splash
void endSplash() {
  for (int loop = 0; loop < 4; loop++) {
    if (searchAndDestroyMode) {
      printLCDFlash(F("TIME OVER"), F("GOODS WIN"));
    //  digitalWrite(GREENLED, HIGH);
      delay(4000);
    }
  }

  while (true) {
    if (searchAndDestroyMode) {
      printLCDFlash(F("TIME OVER"), F("GOODS WIN"));
    //  digitalWrite(GREENLED, HIGH);
      delay(4000);
      waitForDecision(2000);
    }

    printLCDFlash(F("Play Again?"), F("A: YES B: NO"));
    waitForDecision(3000);
  }
}

// Reset Game State
void resetGameState() {
  soundEnable = true;
  searchAndDestroyMode = false;
  dominationMode = false;
  buttonHeld = false;
  demineer = false;
  cancel = false;
  buttonReleasedAfterPhase = true;
  sdStatus = false;
  doStatus = false;
  arming = false;
}
