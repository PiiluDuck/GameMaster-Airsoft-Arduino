#include "LCDutils.h"

// Define the external LCD object
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Adjust address and dimensions as needed

const char* flashToCString(const __FlashStringHelper* flashString) {
  static char buffer[17];  // Static buffer to persist after function returns
  if (flashString) {
    strncpy_P(buffer, (PGM_P)flashString, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';  // Null-terminate
  } else {
    buffer[0] = '\0';  // Empty string for nullptr
  }
  return buffer;
}

void printLCDFlash(const __FlashStringHelper* line1, const __FlashStringHelper* line2) {
  lcd.clear();
  if (line1) {
    lcd.setCursor(0, 0);
    lcd.print(flashToCString(line1));
  }
  if (line2) {
    lcd.setCursor(0, 1);
    lcd.print(flashToCString(line2));
  }
}

void printLCDFlash(const __FlashStringHelper* line1) {
  printLCDFlash(line1, static_cast<const char*>(nullptr));
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
}

void printLCDFlash(const char* line1, const __FlashStringHelper* line2) {
  lcd.clear();
  if (line1) {
    lcd.setCursor(0, 0);
    lcd.print(line1);
  }
  if (line2) {
    lcd.setCursor(0, 1);
    lcd.print(flashToCString(line2));
  }
}

void printLCDFlash(const char* line1, const char* line2) {
  lcd.clear();
  if (line1) {
    lcd.setCursor(0, 0);
    lcd.print(line1);
  }
  if (line2) {
    lcd.setCursor(0, 1);
    lcd.print(line2);
  }
}

void printLCDFlash(const String& line1, const String& line2) {
  lcd.clear();
  if (line1.length() > 0) {
    lcd.setCursor(0, 0);
    lcd.print(line1);
  }
  if (line2.length() > 0) {
    lcd.setCursor(0, 1);
    lcd.print(line2);
  }
}

void printLCDFlashWithBuffers(const __FlashStringHelper* label1, const char* buffer1,
                              const __FlashStringHelper* label2, const char* buffer2) {
  lcd.clear();
  if (label1 || buffer1) {
    lcd.setCursor(0, 0);
    if (label1) lcd.print(flashToCString(label1));
    if (buffer1) lcd.print(buffer1);
  }
  if (label2 || buffer2) {
    lcd.setCursor(0, 1);
    if (label2) lcd.print(flashToCString(label2));
    if (buffer2) lcd.print(buffer2);
  }
}

void printLCDFromPROGMEM(const char* line1 = nullptr, const char* line2 = nullptr, bool clearLCD = true) {
    const size_t bufferSize = 17; // Buffer size for a 16x2 LCD (16 chars + null terminator)
    char buffer[bufferSize];

    if (clearLCD) {
        lcd.clear(); // Clear the entire LCD
    }

    // Handle Row 1
    if (line1) {
        strncpy_P(buffer, line1, bufferSize - 1);
        buffer[bufferSize - 1] = '\0'; // Null-terminate
        lcd.setCursor(0, 0);
        lcd.print(buffer);
    } else {
        // Clear Row 1 if no content provided
        lcd.setCursor(0, 0);
        lcd.print("                "); // Fill with spaces
    }

    // Handle Row 2
    if (line2) {
        strncpy_P(buffer, line2, bufferSize - 1);
        buffer[bufferSize - 1] = '\0'; // Null-terminate
        lcd.setCursor(0, 1);
        lcd.print(buffer);
    } else {
        // Clear Row 2 if no content provided
        lcd.setCursor(0, 1);
        lcd.print("                "); // Fill with spaces
    }
}

void printTime(bool isRemaining, unsigned long totalTimeMillis, int row) {
  unsigned long remainingTime = isRemaining ? totalTimeMillis - millis() : millis();
  unsigned long totalSeconds = remainingTime / 1000;

  int mins = totalSeconds / 60;
  int secs = totalSeconds % 60;

  char timeBuffer[16];
  snprintf(timeBuffer, sizeof(timeBuffer), "Time: %02d:%02d", mins, secs);

  lcd.setCursor(0, row);
  lcd.print(timeBuffer);
}

void truncateTimeForLCD(const char* fullTime, char* lcdTime, size_t lcdBufferSize) {
  snprintf(lcdTime, lcdBufferSize, "%.16s", fullTime);
}

void printTimeToLCD(unsigned long timeMillis, int row) {
  static unsigned long lastUpdateTime = 0;
  unsigned long currentMillis = millis();

  if (currentMillis - lastUpdateTime >= 1000) {
    lastUpdateTime = currentMillis;

    char fullTimeBuffer[20];
    char lcdTimeBuffer[17];

    formatTimeFull(timeMillis, fullTimeBuffer, sizeof(fullTimeBuffer), false);
    truncateTimeForLCD(fullTimeBuffer, lcdTimeBuffer, sizeof(lcdTimeBuffer));

    lcd.setCursor(0, row);
    lcd.print(lcdTimeBuffer);
  }
}

void formatTimeFull(unsigned long timeMillis, char* buffer, size_t bufferSize, bool includeMillis) {
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
