// Helper Function: Convert __FlashStringHelper* to const char* 
const char* flashToCString(const __FlashStringHelper* flashString) {
  static char buffer[17]; // Static buffer to persist after function returns
  if (flashString) {
    strncpy_P(buffer, (PGM_P)flashString, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0'; // Null-terminate
  } else {
    buffer[0] = '\0'; // Empty string for nullptr
  }
  return buffer;
}

// Consolidated sendToESP Functions
/*void sendToESP(const char* line1, const char* line2) {
  if (line1 && line2) {
    espSerial.println(String(line1) + "\n" + String(line2));
  } else if (line1) {
    espSerial.println(line1);
  } else if (line2) {
    espSerial.println(line2);
  } else {
    espSerial.println();
  }
}

void sendToESP(const __FlashStringHelper* line1, const __FlashStringHelper* line2) {
  sendToESP(flashToCString(line1), flashToCString(line2));
}

void sendToESP(const __FlashStringHelper* line1, const char* line2) {
  sendToESP(flashToCString(line1), line2);
}

void sendToESP(const char* line1, const __FlashStringHelper* line2) {
  sendToESP(line1, flashToCString(line2));
}
*/
// Updated printLCDFlash Functions
void printLCDFlash(const __FlashStringHelper* line1, const __FlashStringHelper* line2 = nullptr) {
  lcd.clear();
  if (line1) {
    lcd.setCursor(0, 0);
    lcd.print(flashToCString(line1));
  }
  if (line2) {
    lcd.setCursor(0, 1);
    lcd.print(flashToCString(line2));
  }
  //sendToESP(line1, line2);
}

void printLCDFlashWithBuffers(const __FlashStringHelper* label1 = nullptr, const char* buffer1 = nullptr, const __FlashStringHelper* label2 = nullptr, const char* buffer2 = nullptr) {
  lcd.clear();

  // First row: Label1 and Buffer1
  if (label1 || buffer1) {
    lcd.setCursor(0, 0);
    if (label1) {
      lcd.print(flashToCString(label1));
    }
    if (buffer1) {
      lcd.print(buffer1);
    }
  }

  // Second row: Label2 and Buffer2
  if (label2 || buffer2) {
    lcd.setCursor(0, 1);
    if (label2) {
      lcd.print(flashToCString(label2));
    }
    if (buffer2) {
      lcd.print(buffer2);
    }
  }

 /* // Utilize existing sendToESP function variants for compatibility
  if (label1 && label2) {
    sendToESP(label1, label2);  // __FlashStringHelper overload
  } else if (label1 && buffer2) {
    sendToESP(label1, buffer2);  // Mixed overload: FlashStringHelper + char*
  } else if (buffer1 && label2) {
    sendToESP(buffer1, label2);  // Mixed overload: char* + FlashStringHelper
  } else {
    sendToESP(
      label1 ? flashToCString(label1) : buffer1,
      label2 ? flashToCString(label2) : buffer2
    );  // Fallback to char* overload
  }*/
}

void printLCDFlash(const __FlashStringHelper* line1, const char* line2) {
  lcd.clear();
  if (line1) {
    lcd.setCursor(0, 0);
    lcd.print(flashToCString(line1));
  }
  if (line2) {
    lcd.setCursor(0, 1);
    lcd.print(line2);
  }
  //sendToESP(line1, line2);
}

void printLCDFlash(const char* line1, const char* line2 = nullptr) {
  lcd.clear();
  if (line1) {
    lcd.setCursor(0, 0);
    lcd.print(line1);
  }
  if (line2) {
    lcd.setCursor(0, 1);
    lcd.print(line2);
  }
  //sendToESP(line1, line2);
}

void printLCDFlash(const String& line1, const String& line2 = "") {
  lcd.clear();
  if (line1.length() > 0) {
    lcd.setCursor(0, 0);
    lcd.print(line1);
  }
  if (line2.length() > 0) {
    lcd.setCursor(0, 1);
    lcd.print(line2);
  }
  //sendToESP(line1.c_str(), line2.c_str());
}

// Utility Function: Print from PROGMEM
void printLCDFromPROGMEM(const char* progMemStr, const char* line2 = nullptr) {
  char buffer[17];
  lcd.clear();

  if (progMemStr) {
    strncpy_P(buffer, progMemStr, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0'; // Null-terminate
    lcd.setCursor(0, 0);
    lcd.print(buffer);
  }

  if (line2) {
    lcd.setCursor(0, 1);
    lcd.print(line2);
  }

 // sendToESP(buffer, line2 ? line2 : "");
}

// Print Time in hh:mm:ss format with row flexibility
void printTime(bool isRemaining, unsigned long totalTimeMillis, int row = 1) {
  unsigned long remainingTime = isRemaining ? totalTimeMillis - millis() : millis();
  unsigned long totalSeconds = remainingTime / 1000;

  int mins = totalSeconds / 60;
  int secs = totalSeconds % 60;

  char timeBuffer[16];
  snprintf(timeBuffer, sizeof(timeBuffer), "Time: %02d:%02d", mins, secs);

  lcd.setCursor(0, row);
  lcd.print(timeBuffer);

 // sendToESP("", timeBuffer);
}

// Truncate the full time string to fit the LCD (16 characters)
void truncateTimeForLCD(const char* fullTime, char* lcdTime, size_t lcdBufferSize) {
  snprintf(lcdTime, lcdBufferSize, "%.16s", fullTime);
}

// Print time in hh:mm:ss:ms format on the specified LCD row, updating only once per second
void printTimeToLCD(unsigned long timeMillis, int row) {
  static unsigned long lastUpdateTime = 0; // Track the last update time
  unsigned long currentMillis = millis();

  // Only update the LCD if 1 second has passed since the last update
  if (currentMillis - lastUpdateTime >= 1000) {
    lastUpdateTime = currentMillis; // Update the last refresh time

    char fullTimeBuffer[20];
    char lcdTimeBuffer[17];

    formatTimeFull(timeMillis, fullTimeBuffer, sizeof(fullTimeBuffer), false);
    truncateTimeForLCD(fullTimeBuffer, lcdTimeBuffer, sizeof(lcdTimeBuffer));

    lcd.setCursor(0, row);
    lcd.print(lcdTimeBuffer);
  }
}

// Time formatting logic
void formatTimeFull(unsigned long timeMillis, char* buffer, size_t bufferSize, bool includeMillis = true) {
  unsigned long totalSeconds = timeMillis / 1000;
  unsigned long milliseconds = timeMillis % 1000;
  unsigned long hours = totalSeconds / 3600;
  unsigned long minutes = (totalSeconds % 3600) / 60;
  unsigned long seconds = totalSeconds % 60;

  if (includeMillis) {
    snprintf(buffer, bufferSize, "%02lu:%02lu:%02lu:%03lu", hours, minutes, seconds, milliseconds);
  } else {
    snprintf(buffer, bufferSize, "%02lu:%02lu:%02lu", hours, minutes, seconds);
  }
}
