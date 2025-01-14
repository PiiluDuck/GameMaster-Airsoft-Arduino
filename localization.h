// localization.h
#ifndef LOCALIZATION_H
#define LOCALIZATION_H

#include <avr/pgmspace.h>

// Menu Items
const char menuItems[][17] PROGMEM = {
  "Search & Destroy",
  "Domination",
  "Config"
};
const char configItems[][17] PROGMEM = {
  "Test Sound",
  "Test Mosfet 1",
  "Test Mosfet 2",
  "Back to Menu"
};

// Config Menu
const char testSound[] PROGMEM = "Test Sound";
const char testMosf1[] PROGMEM = "Test Mosfet 1";
const char testMosf2[] PROGMEM = "Test Mosfet 2";
const char mosfetOff[] PROGMEM = "Mosfet Off";
const char select[] PROGMEM = "A: Yes B: No";
const char exitMen[] PROGMEM = "Exiting Menu";
const char sound[] PROGMEM = "Enable Sound?";
const char setGameTime[] PROGMEM = "Set Game Time";
const char setBombTime[] PROGMEM = "Set Bomb Time";
const char setArmTime[] PROGMEM = "Set Arming Time";

// System Messages
const char loadingText[] PROGMEM = "SYSTEM LOADING...";
const char welcomeText[] PROGMEM = "SOFTAIR";
const char initSysText[] PROGMEM = "Airsoft System";

// Splash Messages
const char timeOverText[] PROGMEM = "Time Expired";
const char gWinText[] PROGMEM = "GOODS WIN";
const char terWinText[] PROGMEM = "TERRORISTS WIN";
const char bombDisText[] PROGMEM = "BOMB DISARMED";
const char gameOverText[] PROGMEM = "GAME OVER";
const char winnerText[] PROGMEM = "WINNER";
const char team1Name[] PROGMEM = "YELLOW TEAM";
const char team2Name[] PROGMEM = "BLUE TEAM";
const char tieText[] PROGMEM = "TIE";
const char team1Label[] PROGMEM = "Y:";  // Label for Yellow team
const char team2Label[] PROGMEM = "B:";   // Label for Blue team
const char goodbyeText[] PROGMEM = "Goodbye!";

// Game Messages
//Global
const char startGame[] PROGMEM = "Game Start";
const char armReset[] PROGMEM = "Arming Reset";
const char gameStartText[] PROGMEM = "Starting in....";

//Domination
const char alreadteam1Active[] PROGMEM = "Already Active";
const char neutZone[] PROGMEM = "Neutral Zone";
const char neut[] PROGMEM = "Neutralizing...";
const char neutReset[] PROGMEM = "Neutral Reset";
const char team1Active[] PROGMEM = "Yellow Active";
const char team2Active[] PROGMEM = "Blue Active";
const char remTime[] PROGMEM = "Remaining Time";
const char yellowText[] PROGMEM = "Yellow";
const char blueText[] PROGMEM = "Blue";
const char armingText[] PROGMEM = "Arming";

//SearchDestroy
const char bombArm[] PROGMEM = "Bomb Armed";
const char bombExp[] PROGMEM = "Bomb Exploded";
const char bombArmProg[] PROGMEM = "Arming Bomb";
const char bombDis[] PROGMEM = "Bomb Disarmed";
const char disBomb[] PROGMEM = "Disarming";
const char disReset[] PROGMEM = "Disarm Reset";

#endif
