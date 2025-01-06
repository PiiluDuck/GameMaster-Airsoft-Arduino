extern GameData lastGameData;

// Domination Splash
void dominationSplash(unsigned long team1Time, unsigned long team2Time) {
  char timeBuffer1[9], timeBuffer2[9];
  formatTimeFull(team1Time, timeBuffer1, sizeof(timeBuffer1), false);
  formatTimeFull(team2Time, timeBuffer2, sizeof(timeBuffer2), false);

  const char* winner = team1Time > team2Time ? "YELLOW TEAM" : team2Time > team1Time ? "BLUE TEAM"
                                                                                     : "TIE";

  lastSplash = DOMINATION_SPLASH;
  lastGameData.team1Time = team1Time;
  lastGameData.team2Time = team2Time;
  lastGameData.winner = winner;

  // Initial display for 8 seconds
  unsigned long startMillis = millis();
  while (millis() - startMillis < 8000) {
    printLCDFlashWithBuffers(F("Y:"), timeBuffer1, F("B:"), timeBuffer2);
    delay(2000);
    printLCDFlash(F("WINNER"), winner);
    delay(2000);
  }
  handleDecision();  // Centralized decision handling
}

// Disarmed Splash
void disarmedSplash() {

  lastSplash = DISARMED_SPLASH;

  printLCDFlash(F("BOMB DISARMED"), F("GOODS WIN"));
  delay(5000);
  handleDecision();  // Centralized decision handling
}

// Explode Splash
void explodeSplash() {

  lastSplash = EXPLODE_SPLASH;

  printLCDFlash(F("TERRORISTS WIN"), F("GAME OVER"));
  delay(5000);
  handleDecision();  // Centralized decision handling
}


// End Splash
void endSplash() {
  // Initial display for 5 seconds
  lastSplash = END_SPLASH;

  printLCDFlash(F("TIME OVER"), F("GOODS WIN"));
  delay(5000);
  handleDecision();  // Centralized decision handling
}

// Helper Function to Wait for Decisions
void handleDecision() {
  while (true) {
    // Display the appropriate splash cycling logic
    switch (lastSplash) {
      case END_SPLASH:
        printLCDFlash(F("TIME OVER"), F("GOODS WIN"));
        delay(4000);
        break;

      case DOMINATION_SPLASH:
        {
          char timeBuffer1[9], timeBuffer2[9];
          formatTimeFull(lastGameData.team1Time, timeBuffer1, sizeof(timeBuffer1), false);
          formatTimeFull(lastGameData.team2Time, timeBuffer2, sizeof(timeBuffer2), false);

          printLCDFlashWithBuffers(F("Y:"), timeBuffer1, F("B:"), timeBuffer2);
          delay(2000);
          printLCDFlash(F("WINNER"), lastGameData.winner);
          delay(2000);
          break;
        }

      case DISARMED_SPLASH:
        printLCDFlash(F("BOMB DISARMED"), F("GOODS WIN"));
        delay(4000);
        break;

      case EXPLODE_SPLASH:
        printLCDFlash(F("TERRORISTS WIN"), F("GAME OVER"));
        delay(4000);
        break;

      default:
        return;  // Exit if no valid splash type
    }

    // Check for user decision
    char decision = checkForDecision();
    if (decision == 'a') {  // Play Again
      resetGameState();

      // Restore last game settings
      GAMEMINUTES = lastGameSettings.gameTime;
      BOMBMINUTES = lastGameSettings.bombTime;
      ACTIVATESECONDS = lastGameSettings.armingTime;
      soundEnable = lastGameSettings.soundEnabled;

      if (lastGameMode == 0) {  // Restart Search & Destroy
        sdStatus = true;
        searchAndDestroyMode = true;
        startGameCount();
        search();
      } else if (lastGameMode == 1) {  // Restart Domination
        doStatus = true;
        dominationMode = true;
        startGameCount();
        domination();
      }
      return;
    } else if (decision == 'b') {  // Exit to Main Menu
      resetGameState();
      lastGameMode = -1;  // Clear last game mode
      printLCDFlash(F("Goodbye!"), F(""));
      delay(2000);
      menuPrincipal();
      return;
    }
  }
}

//Waits for keypress under handleDecision
char checkForDecision() {
  char var = keypad.getKey();
  if (var == 'a') {  // Play Again
    tone(tonepin, 2400, 30);
    return 'a';
  } else if (var == 'b') {  // Exit to Main Menu
    tone(tonepin, 2400, 30);
    return 'b';
  }
  return '\0';  // No decision yet
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
  lastSplash = NONE;
}
