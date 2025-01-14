#include "LCDutils.h"
#include "localization.h"

extern GameData lastGameData;

// Domination Splash
void dominationSplash(unsigned long team1Time, unsigned long team2Time) {
  char timeBuffer1[9], timeBuffer2[9];
  formatTimeFull(team1Time, timeBuffer1, sizeof(timeBuffer1), false);
  formatTimeFull(team2Time, timeBuffer2, sizeof(timeBuffer2), false);

  // Use PROGMEM strings for winner
  const __FlashStringHelper* winner = nullptr;
  if (team1Time > team2Time) {
    winner = reinterpret_cast<const __FlashStringHelper*>(team1Name);
  } else if (team2Time > team1Time) {
    winner = reinterpret_cast<const __FlashStringHelper*>(team2Name);
  } else {
    winner = reinterpret_cast<const __FlashStringHelper*>(tieText);
  }

  lastSplash = DOMINATION_SPLASH;
  lastGameData.team1Time = team1Time;
  lastGameData.team2Time = team2Time;

  // Convert the winner (stored in PROGMEM) to a RAM-based string
  strncpy(lastGameData.winner, flashToCString(winner), sizeof(lastGameData.winner) - 1);
  lastGameData.winner[sizeof(lastGameData.winner) - 1] = '\0'; // Null-terminate for safety

  // Initial display for 8 seconds
  unsigned long startMillis = millis();
  while (millis() - startMillis < 8000) {
    printLCDFlashWithBuffers(reinterpret_cast<const __FlashStringHelper*>(team1Label), timeBuffer1,
                             reinterpret_cast<const __FlashStringHelper*>(team2Label), timeBuffer2);
    delay(4000);
    printLCDFlash(reinterpret_cast<const __FlashStringHelper*>(winnerText), lastGameData.winner);
    delay(4000);
  }
  handleDecision();  // Centralized decision handling
}

// Disarmed Splash
void disarmedSplash() {
  lastSplash = DISARMED_SPLASH;

  printLCDFlash(reinterpret_cast<const __FlashStringHelper*>(bombDisText),
                reinterpret_cast<const __FlashStringHelper*>(gWinText));
  delay(5000);
  handleDecision();  // Centralized decision handling
}

// Explode Splash
void explodeSplash() {
  lastSplash = EXPLODE_SPLASH;

  printLCDFlash(reinterpret_cast<const __FlashStringHelper*>(terWinText),
                reinterpret_cast<const __FlashStringHelper*>(gameOverText));
  delay(5000);
  handleDecision();  // Centralized decision handling
}

// End Splash
void endSplash() {
  lastSplash = END_SPLASH;

  printLCDFlash(reinterpret_cast<const __FlashStringHelper*>(timeOverText),
                reinterpret_cast<const __FlashStringHelper*>(gWinText));
  delay(5000);
  handleDecision();  // Centralized decision handling
}

// Helper Function to Wait for Decisions
void handleDecision() {
  unsigned long lastMillis = millis();   // Last time the display was updated
  unsigned long displayInterval = 2000; // Interval duration in milliseconds (2 seconds)
  bool showWinner = false;              // State variable to toggle between team times and winner message

  while (true) {
    unsigned long currentMillis = millis();

    // Update display every `displayInterval` milliseconds
    if (currentMillis - lastMillis >= displayInterval) {
      lastMillis = currentMillis; // Reset the timer

      switch (lastSplash) {
        case END_SPLASH:
          // Display End Splash (static content)
          printLCDFlash(reinterpret_cast<const __FlashStringHelper*>(timeOverText),
                        reinterpret_cast<const __FlashStringHelper*>(gWinText));
          break;

        case DOMINATION_SPLASH: {
          char timeBuffer1[9], timeBuffer2[9];
          formatTimeFull(lastGameData.team1Time, timeBuffer1, sizeof(timeBuffer1), false);
          formatTimeFull(lastGameData.team2Time, timeBuffer2, sizeof(timeBuffer2), false);

          if (showWinner) {
            // Show the winner message
            printLCDFlash(reinterpret_cast<const __FlashStringHelper*>(winnerText), lastGameData.winner);
          } else {
            // Show team times
            printLCDFlashWithBuffers(reinterpret_cast<const __FlashStringHelper*>(team1Label), timeBuffer1,
                                     reinterpret_cast<const __FlashStringHelper*>(team2Label), timeBuffer2);
          }

          // Toggle between showing team times and winner
          showWinner = !showWinner;
          break;
        }

        case DISARMED_SPLASH:
          // Display Disarmed Splash
          printLCDFlash(reinterpret_cast<const __FlashStringHelper*>(bombDisText),
                        reinterpret_cast<const __FlashStringHelper*>(gWinText));
          break;

        case EXPLODE_SPLASH:
          // Display Exploded Splash
          printLCDFlash(reinterpret_cast<const __FlashStringHelper*>(terWinText),
                        reinterpret_cast<const __FlashStringHelper*>(gameOverText));
          break;

        default:
          return; // Exit if no valid splash type
      }
    }

    // Continuously check for user decision without blocking
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
        sdStatus = true;
        startGameCount();
        search();
      } else if (lastGameMode == 1) {  // Restart Domination
        doStatus = true;
        doStatus = true;
        startGameCount();
        domination();
      }
      return;
    } else if (decision == 'b') {  // Exit to Main Menu
      resetGameState();
      lastGameMode = -1;  // Clear last game mode
      printLCDFlash(reinterpret_cast<const __FlashStringHelper*>(goodbyeText), F(""));
      delay(500);
      menuPrincipal();
      return;
    }
  }
}

// Waits for keypress under handleDecision
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
  sdStatus = false;
  doStatus = false;
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
