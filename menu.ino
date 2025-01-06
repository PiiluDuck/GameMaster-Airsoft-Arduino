// Simplified Key Input Check
boolean isPressed(char key) {
  return (keypad.getKey() == key);
}

//Control time input in menus max and min inputs
bool editTimeInput(const __FlashStringHelper* prompt, char* timeBuffer, int& cursorIndex, bool allowHours, bool allowMinutes, bool allowSeconds) {
  // Reset the cursor position based on editable fields
  if (allowHours) {
    cursorIndex = 0;  // Start at hours
  } else if (allowMinutes) {
    cursorIndex = 3;  // Start at minutes
  } else if (allowSeconds) {
    cursorIndex = 6;  // Start at seconds
  }

  // Display the prompt on row 1 and the time buffer on row 2
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(prompt);
  lcd.setCursor(0, 1);
  lcd.print(timeBuffer);
  lcd.setCursor(cursorIndex, 1);
  lcd.cursor();
  lcd.blink();

  while (true) {
    char key = keypad.waitForKey();

    // Exit without saving
    if (key == BT_EXIT) {
      lcd.noCursor();
      lcd.noBlink();
      printLCDFlash(F("Exiting Config"), F(""));
      delay(2000);
      return false;
    }

    // Handle numeric input
    if (key >= '0' && key <= '9') {
      int inputValue = key - '0';
      bool isValid = true;  // Assume valid until checked

      if ((cursorIndex < 3 && allowHours) || (cursorIndex >= 3 && cursorIndex < 6 && allowMinutes) || (cursorIndex >= 6 && allowSeconds)) {
        char oldValue = timeBuffer[cursorIndex];  // Save old value in case input is invalid
        timeBuffer[cursorIndex] = key;

        // Validate input based on the field
        if (cursorIndex < 2 && allowHours) {  // Validate hours (00-24)
          int hours = (timeBuffer[0] - '0') * 10 + (timeBuffer[1] - '0');
          if (hours > 24) isValid = false;
        } else if (cursorIndex >= 3 && cursorIndex < 5 && allowMinutes) {  // Validate minutes (00-59)
          int minutes = (timeBuffer[3] - '0') * 10 + (timeBuffer[4] - '0');
          if (minutes > 59) isValid = false;
        } else if (cursorIndex >= 6 && allowSeconds) {  // Validate seconds (00-59)
          int seconds = (timeBuffer[6] - '0') * 10 + (timeBuffer[7] - '0');
          if (seconds > 59) isValid = false;
        }

        if (isValid) {
          tone(tonepin, 2400, 30);
          lcd.setCursor(0, 1);
          lcd.print(timeBuffer);
        } else {
          timeBuffer[cursorIndex] = oldValue;  // Revert to old value
          tone(tonepin, 800, 30);              // Error tone
        }

        // Move cursor to the next editable field if input was valid
        if (isValid) {
          do {
            cursorIndex++;
            if (cursorIndex == 2 || cursorIndex == 5) cursorIndex++;  // Skip colons

            // Auto-confirm if last editable field is completed
            if ((cursorIndex >= 8) || (!allowSeconds && cursorIndex >= 6) || (!allowMinutes && cursorIndex >= 3 && cursorIndex < 6)) {
              lcd.noCursor();
              lcd.noBlink();
              return true;
            }
          } while (!((cursorIndex < 3 && allowHours) || (cursorIndex >= 3 && cursorIndex < 6 && allowMinutes) || (cursorIndex >= 6 && allowSeconds)));

          lcd.setCursor(cursorIndex, 1);
        }
      }
    }
  }
}

// Display arrow indicators for menu navigation
void checkArrows(byte i, byte maxIndex) {
  if (i == 0) {
    lcd.setCursor(15, 1);
    lcd.write(6);  // Down arrow
  } else if (i == maxIndex) {
    lcd.setCursor(15, 0);
    lcd.write(5);  // Up arrow
  } else {
    lcd.setCursor(15, 1);
    lcd.write(6);  // Down arrow
    lcd.setCursor(15, 0);
    lcd.write(5);  // Up arrow
  }
}

// Main Menu Config
void menuPrincipal() {
  const int menuSize = sizeof(menuItems) / sizeof(menuItems[0]);
  int i = 0;

  resetGameState();
  ring1.clear();
  ring1.show();
  ring2.clear();
  ring2.show();
  printLCDFromPROGMEM(menuItems[i]);
  checkArrows(i, menuSize - 1);

  while (true) {
    char var = keypad.getKey();
    if (var == BT_EXIT) {
      printLCDFlash(F("Exiting Menu"), F(""));
      tone(tonepin, 2400, 30);
      delay(2000);
      return;
    }
    if (var) {
      if (var == BT_UP && i > 0) {
        i--;
        printLCDFromPROGMEM(menuItems[i]);
        checkArrows(i, menuSize - 1);
        tone(tonepin, 2400, 30);
      } else if (var == BT_DOWN && i < menuSize - 1) {
        i++;
        printLCDFromPROGMEM(menuItems[i]);
        checkArrows(i, menuSize - 1);
        tone(tonepin, 2400, 30);
      } else if (var == BT_SEL) {
        printLCDFromPROGMEM(menuItems[i]);

        // Call appropriate functions based on selection
        switch (i) {
          case 0:              // Search & Destroy
            lastGameMode = 0;  // Set last game mode
            sdStatus = true;
            doStatus = false;
            configQuickGame();
            startGameCount();
            search();
            sdStatus = false;
            break;
          case 1:              // Domination
            lastGameMode = 1;  // Set last game mode
            doStatus = true;
            sdStatus = false;
            configQuickGame();
            startGameCount();
            domination();
            doStatus = false;
            break;
          case 2:  // Config Menu
            config();
            break;
        }
        return;
      }
    }
  }
}

// Quick game configuration
void configQuickGame() {
  if (!configureGameTime()) {
    menuPrincipal();
    return;
  }
  lastGameSettings.gameTime = GAMEMINUTES;  // Save game time

  if (sdStatus && !configureBombTime()) {
    menuPrincipal();
    return;
  }
  if (sdStatus) {
    lastGameSettings.bombTime = BOMBMINUTES;  // Save bomb time
  }

  if (!configureArmingTime()) {
    menuPrincipal();
    return;
  }
  lastGameSettings.armingTime = ACTIVATESECONDS;  // Save arming time

  if (!configureSound()) {
    menuPrincipal();
    return;
  }
  lastGameSettings.soundEnabled = soundEnable;  // Save sound setting
}

// Configuration menu
void config() {
  const int configSize = sizeof(configItems) / sizeof(configItems[0]);
  int i = 0;

  printLCDFromPROGMEM(configItems[i]);

  while (true) {
    char var = keypad.getKey();
    if (var) {
      tone(tonepin, 2400, 30);

      if (var == BT_UP && i > 0) {
        i--;
        printLCDFromPROGMEM(configItems[i]);
      } else if (var == BT_DOWN && i < configSize - 1) {
        i++;
        printLCDFromPROGMEM(configItems[i]);
      } else if (var == BT_CANCEL || var == BT_EXIT) {
        return;  // Exit config menu
      } else if (var == BT_SEL) {
        switch (i) {
          case 0:  // Test Sound
            printLCDFlash(F("Testing Sound"), F(""));
            playStartupTune();
            delay(5000);
            break;

          case 1:  // Test Mosfet
            printLCDFlash(F("Testing Mosfet 1"), F(""));
            mosfetEnable = true;
            activateMosfet_1();
            printLCDFlash(F("Mosfet 1 OFF"), F(""));
            mosfetEnable = false;
            delay(2000);
            break;

          case 2:  // Test Mosfet
            printLCDFlash(F("Testing Mosfet 2"), F(""));
            mosfetEnable = true;
            activateMosfet_2();
            printLCDFlash(F("Mosfet 2 OFF"), F(""));
            mosfetEnable = false;
            delay(2000);
            break;

          case 3:  // Back to Menu
            return;
        }
      }
    }
  }
}

// Confirmation function
bool confirmSetting(const char* selectedTime) {
  // Display confirmation options and the selected time
  printLCDFlash(F("A: Yes B: No"), selectedTime);
  lcd.noCursor();  // Ensure no blinking cursor
  lcd.noBlink();

  while (true) {
    char key = keypad.getKey();
    if (key == 'a') {  // Confirm
      tone(tonepin, 2400, 30);
      return true;
    } else if (key == 'b') {  // Cancel
      tone(tonepin, 2400, 30);
      return false;
    }
  }
}

// Game Time configuration
bool configureGameTime() {
  char timeInput[] = "00:00:00";  // Initialize time input
  int cursorIndex = 0;            // Start at hours (hh)

  while (true) {
    // Prompt user to edit game time
    if (!editTimeInput(F("Set Game Time"), timeInput, cursorIndex, true, true, false)) {
      return false;  // User canceled
    }

    // Calculate total game time in minutes
    unsigned int totalMinutes = ((timeInput[0] - '0') * 10 * 60 + (timeInput[1] - '0') * 60) + ((timeInput[3] - '0') * 10 + (timeInput[4] - '0'));
    GAMEMINUTES = totalMinutes;

    // Format confirmation message
    char confirmation[10];
    snprintf(confirmation, sizeof(confirmation), "%02d:%02d:00", totalMinutes / 60, totalMinutes % 60);

    // Ask user to confirm the game time
    if (confirmSetting(confirmation)) {
      return true;  // User confirmed
    } else {
      strcpy(timeInput, "00:00:00");  // Reset time input on decline
    }
  }
}

// Bomb Time configuration
bool configureBombTime() {
  char timeInput[] = "00:00:00";  // Initialize bomb time input
  int cursorIndex = 3;            // Start cursor at minutes (mm)

  while (true) {
    // Prompt user to edit bomb time
    if (!editTimeInput(F("Set Bomb Time"), timeInput, cursorIndex, false, true, false)) {
      return false;  // User canceled
    }

    // Extract bomb time in minutes
    unsigned int bombMinutes = ((timeInput[3] - '0') * 10 + (timeInput[4] - '0'));
    BOMBMINUTES = bombMinutes;

    // Format confirmation message
    char confirmation[10];
    snprintf(confirmation, sizeof(confirmation), "00:%02d:00", bombMinutes);

    // Ask user to confirm the bomb time
    if (confirmSetting(confirmation)) {
      return true;  // User confirmed
    } else {
      strcpy(timeInput, "00:00:00");  // Reset time input on decline
    }
  }
}

// Arming Time configuration
bool configureArmingTime() {
  char timeInput[] = "00:00:00";  // Initialize arming time input
  int cursorIndex = 6;            // Start cursor at seconds (ss)

  while (true) {
    // Prompt user to edit arming time
    if (!editTimeInput(F("Set Arming Time"), timeInput, cursorIndex, false, false, true)) {
      return false;  // User canceled
    }

    // Extract arming time in seconds
    unsigned int armingSeconds = ((timeInput[6] - '0') * 10 + (timeInput[7] - '0'));
    ACTIVATESECONDS = armingSeconds;

    // Format confirmation message
    char confirmation[10];
    snprintf(confirmation, sizeof(confirmation), "00:00:%02d", armingSeconds);

    // Ask user to confirm the arming time
    if (confirmSetting(confirmation)) {
      return true;  // User confirmed
    } else {
      strcpy(timeInput, "00:00:00");  // Reset time input on decline
      cursorIndex = 6;                // Reset cursor to seconds
    }
  }
}

// Sound Configuration
bool configureSound() {
  printLCDFlash(F("Enable Sound?"), F("A: Yes B: No"));

  while (true) {
    char var = keypad.getKey();
    if (var == 'a') {
      soundEnable = true;
      tone(tonepin, 2400, 30);
      delay(500);
      break;
    } else if (var == 'b') {
      soundEnable = false;
      tone(tonepin, 2400, 30);
      delay(500);
      break;
    }
  }
  return true;
}
