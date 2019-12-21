//Serial Monitor Varible Input for use with Adafruit Capacitive Arduino Touchscreen shield
//Written by Joseph Schroedl
//August 25-26, 2017
//
//Updates:
//8/31/2017 - John Schroedl - Added control keyword processing for two-way communcation.

// Sparkfun Redboard Turbo uses SerialUSB, std Arduino uses Serial
//#define Serial SerialUSB

#include <Adafruit_GFX.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_ILI9341.h>

#define CAPACITIVE_TS 0     // 1 = Using FT6206, 0 = Using STMPE610

#if CAPACITIVE_TS
// Adafruit Capacative Touch Shield
#include <Adafruit_FT6206.h>
#else
// Adafruit Resistive Touch Shield
#include <Adafruit_STMPE610.h>
#endif

bool verboseMode = false;

long baudRate = 115200;

bool OnorOff = false;
bool CountingUp = true;
int count = 0;

long serialDelay = 100;
long tempSerialDelay = 0;

Adafruit_GFX_Button serialSetSpeedButton = Adafruit_GFX_Button();
Adafruit_GFX_Button OnButton = Adafruit_GFX_Button();
Adafruit_GFX_Button OffButton = Adafruit_GFX_Button();
//Number pad buttons
Adafruit_GFX_Button Number0 = Adafruit_GFX_Button();
Adafruit_GFX_Button Number1 = Adafruit_GFX_Button();
Adafruit_GFX_Button Number2 = Adafruit_GFX_Button();
Adafruit_GFX_Button Number3 = Adafruit_GFX_Button();
Adafruit_GFX_Button Number4 = Adafruit_GFX_Button();
Adafruit_GFX_Button Number5 = Adafruit_GFX_Button();
Adafruit_GFX_Button Number6 = Adafruit_GFX_Button();
Adafruit_GFX_Button Number7 = Adafruit_GFX_Button();
Adafruit_GFX_Button Number8 = Adafruit_GFX_Button();
Adafruit_GFX_Button Number9 = Adafruit_GFX_Button();
Adafruit_GFX_Button NumberDelete = Adafruit_GFX_Button();

// The currently-pressed button.
Adafruit_GFX_Button* pressedButton = NULL;

// The touchscreen

#if CAPACITIVE_TS

Adafruit_FT6206 ts = Adafruit_FT6206();

#else // Resistive Screen

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 150
#define TS_MINY 130
#define TS_MAXX 3800
#define TS_MAXY 4000

// The STMPE610 uses hardware SPI on the shield, and #8
#define STMPE_CS 8
Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);

#endif


#define TFT_CS 10
#define TFT_DC 9
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

int OnOffButtonXPos = 0;
int OnOffButtonYPos = 0;
int OnOffButtonWidth = 120;
int OnOffButtonHeight = 120;

int SerialSpeedButtonXPos = 200;
int SerialSpeedButtonYPos = 0;
int SerialSpeedButtonWidth = 120;
int SerialSpeedButtonHeight = 120;

void clearScreen()
{
  tft.fillScreen(ILI9341_BLACK);
  if (verboseMode) {
    SerialUSB.println("Cleared Screen");
  }
}

void drawOffButton()
{
  OffButton.drawButton();
  /*tft.fillRect(OnOffButtonXPos, OnOffButtonYPos, OnOffButtonWidth, OnOffButtonHeight, ILI9341_RED);
    tft.setCursor((OnOffButtonWidth / 10), (OnOffButtonHeight / 3.5));
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(6);
    tft.println("OFF");*/
}

void drawOnButton()
{
  OnButton.drawButton();
  /*tft.fillRect(OnOffButtonXPos, OnOffButtonYPos, OnOffButtonWidth, OnOffButtonHeight, ILI9341_GREEN);
    tft.setCursor((OnOffButtonWidth / 7), (OnOffButtonHeight / 5));
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(8);
    tft.println("ON");*/
}

void checkOperationStatus()
{
  if (OnorOff == true)
  {
    drawOnButton();
    if (verboseMode) {
      SerialUSB.println("Now ON");
    }
    //delay(1000);
  }
  else if (OnorOff == false)
  {
    drawOffButton();
    if (verboseMode) {
      SerialUSB.println("Now OFF");
    }
    //delay(1000);
  }
}

void drawSerialSpeedButton()
{
  serialSetSpeedButton.drawButton();
  /*tft.fillRect(SerialSpeedButtonXPos, SerialSpeedButtonYPos, SerialSpeedButtonWidth, SerialSpeedButtonHeight, ILI9341_LIGHTGREY);
    tft.setCursor(SerialSpeedButtonXPos + (SerialSpeedButtonWidth / 10), SerialSpeedButtonYPos + (SerialSpeedButtonHeight / 3.5));
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(3);
    //tft.setTextWrap(false);
    tft.print("Output");
    tft.setCursor(SerialSpeedButtonXPos + (SerialSpeedButtonWidth / 10), SerialSpeedButtonYPos + (SerialSpeedButtonHeight / 2));
    tft.print("Rate");*/
}

void drawNumberPad()
{
  Number0.drawButton();
  Number1.drawButton();
  Number2.drawButton();
  Number3.drawButton();
  Number4.drawButton();
  Number5.drawButton();
  Number6.drawButton();
  Number7.drawButton();
  Number8.drawButton();
  Number9.drawButton();
  NumberDelete.drawButton();
}

void drawSpeedMeterLabel()
{
  tft.setCursor(130, 5);
  tft.setTextColor(ILI9341_BLUE);
  tft.setTextSize(2);
  tft.print("Speed:");
}

void drawSpeedMeter()
{
  tft.fillRect(120, 25, 80, 14, ILI9341_BLACK);
  tft.setCursor(120, 25);
  tft.setTextColor(ILI9341_BLUE);
  tft.setTextSize(2);
  tft.print(serialDelay);
  tft.print("ms");
}

void drawTempSpeedLabel()
{
  tft.setCursor(130, 80);
  tft.setTextColor(ILI9341_ORANGE);
  tft.setTextSize(2);
  tft.print("Temp:");
}

void drawTempSpeed()
{
  tft.fillRect(120, 100, 80, 14, ILI9341_BLACK);
  tft.setCursor(120, 100);
  tft.setTextColor(ILI9341_ORANGE);
  tft.setTextSize(2);
  tft.print(tempSerialDelay);
  tft.print("ms");
}

void setup()
{
  SerialUSB.begin(baudRate);
  if (verboseMode)
    SerialUSB.println("Starting setup...");

  tft.begin();

  if (!ts.begin(40)) {
    if (verboseMode) {
      SerialUSB.println("Unable to start touchscreen.");
    }
  }
  else {
    if (verboseMode) {
      SerialUSB.println("Touchscreen started.");
    }
  }
  //Initialize all buttons
  serialSetSpeedButton.initButtonUL(&tft, SerialSpeedButtonXPos, SerialSpeedButtonYPos, SerialSpeedButtonWidth, SerialSpeedButtonHeight, ILI9341_DARKCYAN, ILI9341_DARKCYAN, ILI9341_WHITE, "SET SPEED", 2);
  OnButton.initButtonUL(&tft, OnOffButtonXPos, OnOffButtonYPos, OnOffButtonWidth, OnOffButtonHeight, ILI9341_GREEN, ILI9341_GREEN, ILI9341_WHITE, "ON", 5);
  OffButton.initButtonUL(&tft, OnOffButtonXPos, OnOffButtonYPos, OnOffButtonWidth, OnOffButtonHeight, ILI9341_RED, ILI9341_RED, ILI9341_WHITE, "OFF", 5);
  //Number Pad Buttons
  Number0.initButtonUL(&tft, 0, 140, 40, 40, ILI9341_DARKGREY, ILI9341_DARKGREY, ILI9341_WHITE, "0", 3);
  Number1.initButtonUL(&tft, 50, 140, 40, 40, ILI9341_DARKGREY, ILI9341_DARKGREY, ILI9341_WHITE, "1", 3);
  Number2.initButtonUL(&tft, 100, 140, 40, 40, ILI9341_DARKGREY, ILI9341_DARKGREY, ILI9341_WHITE, "2", 3);
  Number3.initButtonUL(&tft, 150, 140, 40, 40, ILI9341_DARKGREY, ILI9341_DARKGREY, ILI9341_WHITE, "3", 3);
  Number4.initButtonUL(&tft, 200, 140, 40, 40, ILI9341_DARKGREY, ILI9341_DARKGREY, ILI9341_WHITE, "4", 3);
  Number5.initButtonUL(&tft, 250, 140, 40, 40, ILI9341_DARKGREY, ILI9341_DARKGREY, ILI9341_WHITE, "5", 3);
  Number6.initButtonUL(&tft, 0, 190, 40, 40, ILI9341_DARKGREY, ILI9341_DARKGREY, ILI9341_WHITE, "6", 3);
  Number7.initButtonUL(&tft, 50, 190, 40, 40, ILI9341_DARKGREY, ILI9341_DARKGREY, ILI9341_WHITE, "7", 3);
  Number8.initButtonUL(&tft, 100, 190, 40, 40, ILI9341_DARKGREY, ILI9341_DARKGREY, ILI9341_WHITE, "8", 3);
  Number9.initButtonUL(&tft, 150, 190, 40, 40, ILI9341_DARKGREY, ILI9341_DARKGREY, ILI9341_WHITE, "9", 3);
  NumberDelete.initButtonUL(&tft, 240, 200, 80, 40, ILI9341_MAROON, ILI9341_MAROON, ILI9341_WHITE, "Delete", 2);

  tft.setRotation(1);

  clearScreen();

  drawSerialSpeedButton();
  checkOperationStatus();
  drawSpeedMeterLabel();
  drawSpeedMeter();
  drawNumberPad();
  drawTempSpeedLabel();
  drawTempSpeed();
  if (verboseMode) {
    SerialUSB.println("Done Drawing Setup Buttons");
  }
}

// Test all the buttons for a tap and return a
// pointer to the button tapped or NULL if no
// button was hit.
Adafruit_GFX_Button* buttonHitTest(TS_Point p)
{
  if (OnButton.contains(p.x, p.y)) {
    return &OnButton;
  }
  else if (OffButton.contains(p.x, p.y)) {
    return &OffButton;
  }
  else if (serialSetSpeedButton.contains(p.x, p.y)) {
    return &serialSetSpeedButton;
  }
  else if (Number0.contains(p.x, p.y)) {
    return &Number0;
  }
  else if (Number1.contains(p.x, p.y)) {
    return &Number1;
  }
  else if (Number2.contains(p.x, p.y)) {
    return &Number2;
  }
  else if (Number3.contains(p.x, p.y)) {
    return &Number3;
  }
  else if (Number4.contains(p.x, p.y)) {
    return &Number4;
  }
  else if (Number5.contains(p.x, p.y)) {
    return &Number5;
  }
  else if (Number6.contains(p.x, p.y)) {
    return &Number6;
  }
  else if (Number7.contains(p.x, p.y)) {
    return &Number7;
  }
  else if (Number8.contains(p.x, p.y)) {
    return &Number8;
  }
  else if (Number9.contains(p.x, p.y)) {
    return &Number9;
  }
  else if (NumberDelete.contains(p.x, p.y)) {
    return &NumberDelete;
  }
  else {
    return NULL;
  }
}

// Transform touch point to screen coordinate.
TS_Point transformTouchPoint(TS_Point const & touchPoint)
{
  TS_Point screenPoint = touchPoint;


#if CAPACITIVE_TS
  // Swap x & y
  screenPoint.x = touchPoint.y;
  screenPoint.y = touchPoint.x;

  // Reverse x: [0,320] -> [320,0]
  screenPoint.x = map(screenPoint.x, 0, 320, 320, 0);
#else

  screenPoint.x = map(touchPoint.y, TS_MINX, TS_MAXX, 0, tft.width());
  screenPoint.y = map(touchPoint.x, TS_MINY, TS_MAXY, tft.height(), 0);

  //screenPoint.x = map(screenPoint.x, 0, 320, 320, 0);
#endif

  if (verboseMode)
  {
    SerialUSB.print("X: ");
    SerialUSB.println(screenPoint.x);
    SerialUSB.print("Y: ");
    SerialUSB.println(screenPoint.y);
  }

  return screenPoint;
}

void turnOnOrOff(bool newState)
{
  OnorOff = newState;
  checkOperationStatus();
  count = 0;
  CountingUp = true;
}

void setSpeed(int value)
{
  serialDelay = value;
  drawSpeedMeter();
}

// Read an incoming command from the serial port.
void readCommand()
{
  String cmd = SerialUSB.readStringUntil('\n');
  cmd.replace('\r', '\0'); // If it was terminated with CRLF remove it.

  if (verboseMode) {
    SerialUSB.println("Got command: " + cmd);
  }

  if (cmd == "ON") {
    turnOnOrOff(true);
  }
  else if (cmd == "OFF") {
    turnOnOrOff(false);
  }
  else if (cmd.startsWith("SPEED ")) {
    int value = cmd.substring(6).toInt();
    if (value >= 0)
      setSpeed(value);
  }
}

void loop()
{
  if (SerialUSB.available() > 0)
    readCommand();

  if (ts.touched()) {
    pressedButton = buttonHitTest(transformTouchPoint(ts.getPoint()));
    if (verboseMode) {
      SerialUSB.println("Button HIT!");
    }
    if (pressedButton != NULL && !pressedButton->isPressed())
      pressedButton->press(true);
  }
  else {
    if (pressedButton != NULL) {
      if (verboseMode) {
        SerialUSB.println("un-pressing a button");
      }
      pressedButton->press(false);
      pressedButton = NULL;
    }
  }

  if (OnButton.justPressed() || OffButton.justPressed()) {
    // Button toggles on/off
    if (verboseMode) {
      SerialUSB.println("You touched the button");
    }
    turnOnOrOff(!OnorOff);
  }
  else if (OnorOff) {
    if (CountingUp) {
      SerialUSB.println(count);
      count = count + 1;
      if (count == 100) {
        CountingUp = false;
      }
      delay(serialDelay);
    }
    else if (CountingUp == false) {
      SerialUSB.println(count);
      count = count - 1;
      if (count == 0) {
        CountingUp = true;
      }
      delay(serialDelay);
    }
  }
  else if (serialSetSpeedButton.justPressed())
  {
    setSpeed(tempSerialDelay);
    if (verboseMode) {
      SerialUSB.println("Set the speed to what user inputed");
    }

    // Reset for next time
    tempSerialDelay = 0;
    drawTempSpeed();
  }
  else if (Number0.justPressed()) {
    tempSerialDelay = tempSerialDelay * 10 + 0;
    drawTempSpeed();
  }
  else if (Number1.justPressed()) {
    tempSerialDelay = tempSerialDelay * 10 + 1;
    drawTempSpeed();
  }
  else if (Number2.justPressed()) {
    tempSerialDelay = tempSerialDelay * 10 + 2;
    drawTempSpeed();
  }
  else if (Number3.justPressed()) {
    tempSerialDelay = tempSerialDelay * 10 + 3;
    drawTempSpeed();
  }
  else if (Number4.justPressed()) {
    tempSerialDelay = tempSerialDelay * 10 + 4;
    drawTempSpeed();
  }
  else if (Number5.justPressed()) {
    tempSerialDelay = tempSerialDelay * 10 + 5;
    drawTempSpeed();
  }
  else if (Number6.justPressed()) {
    tempSerialDelay = tempSerialDelay * 10 + 6;
    drawTempSpeed();
  }
  else if (Number7.justPressed()) {
    tempSerialDelay = tempSerialDelay * 10 + 7;
    drawTempSpeed();
  }
  else if (Number8.justPressed()) {
    tempSerialDelay = tempSerialDelay * 10 + 8;
    drawTempSpeed();
  }
  else if (Number9.justPressed()) {
    tempSerialDelay = tempSerialDelay * 10 + 9;
    drawTempSpeed();
  }
  else if (NumberDelete.justPressed()) {
    tempSerialDelay = 0;
    drawTempSpeed();
    if (verboseMode) {
      SerialUSB.println("Deleted Temp Serial Speed, Reset to 0");
    }
  }
}
