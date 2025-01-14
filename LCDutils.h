#ifndef LCDUTILS_H
#define LCDUTILS_H
#include <Arduino.h>
#include <avr/pgmspace.h>
#include <LiquidCrystal_I2C.h> // Include the library you use for LCD

// Declare the external LCD object (make sure to define it in your main file)
extern LiquidCrystal_I2C lcd;

// Function declarations
const char* flashToCString(const __FlashStringHelper* flashString);

void printLCDFlash(const __FlashStringHelper* line1, const __FlashStringHelper* line2);
void printLCDFlash(const __FlashStringHelper* line1);
void printLCDFlash(const __FlashStringHelper* line1, const char* line2);
void printLCDFlash(const char* line1, const __FlashStringHelper* line2);
void printLCDFlash(const char* line1, const char* line2);
void printLCDFlash(const String& line1, const String& line2);

void printLCDFlashWithBuffers(const __FlashStringHelper* label1 = nullptr, const char* buffer1 = nullptr,
                              const __FlashStringHelper* label2 = nullptr, const char* buffer2 = nullptr);

void printLCDFromPROGMEM(const char* line1 = nullptr, const char* line2 = nullptr, bool clearLCD = true);

void printTime(bool isRemaining, unsigned long totalTimeMillis, int row = 1);
void printTimeToLCD(unsigned long timeMillis, int row);
void formatTimeFull(unsigned long timeMillis, char* buffer, size_t bufferSize, bool includeMillis = true);

#endif // LCDUTILS_H
