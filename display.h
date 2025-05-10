#include "global.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Fonts/TomThumb.h>

#ifndef DISPLAY_h
#define DISPLAY_h


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // note, for WIRE on Teensy 4.1
                                                                                             // SDA = 18, SCL = 19


extern uint8_t systemMode;
extern uint8_t systemSubMode;


extern uint8_t cmdValue;
extern uint8_t byteValue;

extern uint parmCtr;

// test screen refresh counter
long lastdrawTestScreen = 0;


// toggle used for flickering in UI
int toggle = -1;

// cursor position for text editing
uint textCursorPos = 0;

// active knob for configuration screen
long configKnobID = -1;


extern uint8_t activeKnob;
extern long getActiveKnob(long divisor);

// small text buffer used in various places, mostly for text buffers
char buffer[25];


/* ---------------------------------------------------------------
   |  drawKnobArrow - draws the arrow on the knob                |
   --------------------------------------------------------------- */
void drawKnobArrow(int x, int y, int deg) {
  int cx = x + 4;
  int cy = y + 3;

  deg = deg - 45;
  if(deg < 0)
    deg += 360;

  // Apply 120-degree rotation offset (like original float math)
  int index = deg % 360;

  int ex = cx + arrow_dx[index];
  int ey = cy + arrow_dy[index];

  display.drawLine(cx, cy, ex, ey, SH110X_BLACK);
}


/* .-----------------------------------------------------------------------------------------------------.
   |  drawKnob() - draws a knob at a given x, y location with a given i value from 0 - 255 for the arrow |
   '-----------------------------------------------------------------------------------------------------' */
void drawKnob(int knobid, int x, int y, int i) {
  display.fillRect(x+2, y+1, 6, 4, SH110X_WHITE);
  display.drawLine(x+3, y, x+6, y, SH110X_WHITE);
  display.drawLine(x+3, y+5, x+6, y+5, SH110X_WHITE);
  display.drawLine(x+1, y+2, x+1, y+3, SH110X_WHITE);
  display.drawLine(x+8, y+2, x+8, y+3, SH110X_WHITE);

  if(systemMode == MODE_CONFIG) {
    if(getActiveKnob(NUM_KNOBS) == knobid) {
      toggle = -toggle;
      if(toggle > 0) {
          display.fillRect(x+2, y+1, 6, 4, SH110X_BLACK);
      }
      else {
          display.fillRect(x+2, y+1, 6, 4, SH110X_WHITE);
      }
    }
  }

  if(i != -1) {
    // i ranges from 0 - 255
    int j = (float)(360 * ((float)i/255));
    drawKnobArrow(x,y,j);
  }
  else {
    if(bEncoderBtn)
        display.fillRect(x+2, y+1, 6, 4, SH110X_BLACK);
  }
}

/* .-------------------------------------------------------------------------------------------------------------.
   |  drawButton() - Draws a push button at a given x, y position, with the LEDs drawn and whether its depressed |
   '-------------------------------------------------------------------------------------------------------------' */
void drawButton(int x, int y, bool red, bool green, uint8_t depressed) {
  display.fillRect(x+1, y, 8, 6, SH110X_WHITE);
  display.drawLine(x+2, y+2, x+7, y+2, SH110X_BLACK);
  
  if(depressed)
    display.fillRect(x+2, y+2, 6, 3, SH110X_BLACK);

  if(green) 
    display.drawLine(x+2,y+1, x+3, y+1, SH110X_BLACK);
  if(red)
    display.drawLine(x+6,y+1, x+7, y+1, SH110X_BLACK);
}

void drawRunningScreen() {
  if(millis() - lastdrawTestScreen < 33) 
    return;

  lastdrawTestScreen = millis();

  display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SH110X_BLACK); // clear screen
  display.setTextColor(SH110X_WHITE);
  if(millis() < 12000) {
    display.setCursor(10,0);
    display.print("JMR-800 Controller");
    display.setCursor(10,20);
    display.print(__DATE__); 
    display.setCursor(10,30);
    display.print(__TIME__);;
    display.setCursor(10,50);
    display.print("Booting...");
    display.display();
    return;
  }

  display.setCursor(20,20);
  display.print(knobConfigurations[activeKnob].name);
  display.setCursor(20,30);
  display.print(typeCodes[knobConfigurations[activeKnob].typecode].typeCodeName);
  display.setCursor(20,40);
  display.print(knobValueAt(activeKnob));

  if(systemSubMode == SUBMODE_2 || systemSubMode == SUBMODE_3) {
    display.drawLine(4, 60, 4 + (parmCtr*2), 60, SH110X_WHITE);
  }

  display.display();
}


/* --------------------------------------------------------------
   |  drawTestScreen()                                          |
   -------------------------------------------------------------- */
void drawTestScreen() {
    uint knobIX = 0;
    uint btnCtr = 3;
    uint knobpos;

    if(millis() - lastdrawTestScreen < 33) 
      return;

    lastdrawTestScreen = millis();

    display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SH110X_BLACK); // clear screen
    display.setCursor(10,0);                                           // position for title
    display.printf("JMR800 TEST SCREEN");                              // title
    display.drawRect(0, 9, SCREEN_WIDTH, 54, SH110X_WHITE);            // box around UI
    display.drawRect(88, 15, 21, 11, SH110X_WHITE);                    // small screen rectangle
    display.setCursor(90, 17);
    display.printf("%03d", abs((lastEncoderPosition>>2) % 1000));
    for(int i=7; i<78; i = i + 10) {                                   // knobs on the left side
      for(int j=14; j<62; j = j + 8) {
          knobIX++;
          knobpos = getKnobValue(knobIX-1);
          drawKnob(knobIX-1,i,j,knobpos);
      }
    }
    for(int i=87; i < 98; i = i + 10) {                               // knobs on the right side
      for(int j=30; j < 62; j = j + 8) {
        knobIX++;
        knobpos = getKnobValue(knobIX-1);
        drawKnob(knobIX-1,i,j,knobpos);
      }
    }
    drawKnob(-1, 109, 18, -1);                                              // encoder knob

    for(int j=30; j < 62; j = j + 8) {                                 // buttons
      if(btnCtr == 3)
        drawButton(109, j, ledPattern & 0b10000000, ledPattern & 0b01000000, buttonStates[0]);
      else
      if(btnCtr == 2)
        drawButton(109, j, ledPattern & 0b00100000, ledPattern & 0b00010000, buttonStates[1]);
      else
      if(btnCtr == 1)
        drawButton(109, j, ledPattern & 0b00001000, ledPattern & 0b00000100, buttonStates[2]);
      else
        drawButton(109, j, ledPattern & 0b00000010, ledPattern & 0b00000001, buttonStates[3]);
      btnCtr--;
    }
    display.display();
}



void drawConfigKnobScreen() {
  long highlight = getActiveKnob(5);
  toggle = -toggle;
  display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SH110X_BLACK); // clear screen
  display.drawRect(87, 27, 1, 1, SH110X_WHITE);
  display.drawPixel(87, 27, SH110X_BLACK);
  display.drawRect(0, 8, 128, 56, SH110X_BLACK);
  display.drawRect(0, 0, 128, 64, SH110X_WHITE);
  display.drawLine(0, 0, 127, 0, SH110X_BLACK);
  display.drawLine(0, 0, 0, 8, SH110X_BLACK);
  display.drawLine(1, 9, 127, 10, SH110X_WHITE);
  display.drawLine(65, 9, 127, 9, SH110X_WHITE);
  display.drawLine(65, 10, 126, 10, SH110X_BLACK);
  display.drawLine(127, 8, 127, 1, SH110X_BLACK);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(4, 1);
  display.print("Config - Update");
  display.setCursor(4, 11);
  display.setTextColor(SH110X_WHITE);
  display.print("Knob #" + String(configKnobID));

  if(highlight == KNOB_CONFIG_NAME_HIGHLIGHTED && systemSubMode == SUBMODE_2) {
    if(toggle > 0)
      display.drawRect(38,19,84,11, SH110X_WHITE);
    else
      display.drawRect(38,19,84,11, SH110X_BLACK);
  }
  display.setCursor(4, 21);
  display.setTextColor(SH110X_WHITE);
  display.print("Name: " + String(knobConfigurations[configKnobID].name));
  
  if(highlight == KNOB_CONFIG_CMD_HIGHLIGHTED && systemSubMode == SUBMODE_2) {
    if(toggle > 0)
      display.drawRect(61,28,29,11, SH110X_WHITE);
    else
      display.drawRect(61,28,29,11, SH110X_BLACK);
  }
  display.setCursor(4, 30);
  display.setTextColor(SH110X_WHITE);
  sprintf(buffer, "CMD Byte: 0x%02X",  knobConfigurations[configKnobID].cmdbyte);
  display.print(buffer);

  if(highlight == KNOB_CONFIG_TYPECMD_HIGHLIGHTED && systemSubMode == SUBMODE_2) {
    if(toggle > 0)
      display.drawRect(32,37,90,11, SH110X_WHITE);
    else
      display.drawRect(32,37,90,11, SH110X_BLACK);
  }
  display.setCursor(4, 39);
  display.setTextColor(SH110X_WHITE);
  display.print("Type:" + String(typeCodes[knobConfigurations[configKnobID].typecode].typeCodeName));

  if(highlight == KNOB_CONFIG_OKAY_HIGHLIGHTED && systemSubMode == SUBMODE_2) {
    if(toggle > 0)
      display.drawRect(30,51,19,11, SH110X_WHITE);
    else
      display.drawRect(30,51,19,11, SH110X_BLACK);
  }

  display.setCursor(34, 53);
  display.setTextColor(SH110X_WHITE);
  display.print("OK");

  if(highlight == KNOB_CONFIG_CANCEL_HIGHLIGHTED && systemSubMode == SUBMODE_2) {
    if(toggle > 0)
      display.drawRect(58,51,44,11, SH110X_WHITE);
    else
      display.drawRect(58,51,44,11, SH110X_BLACK);
  }

  display.setCursor(62, 53);
  display.setTextColor(SH110X_WHITE);
  display.print("Cancel");
  display.drawRect(32, 51, 15, 10, SH110X_WHITE);
  display.drawPixel(32, 51, SH110X_WHITE);
  display.drawPixel(32, 51, SH110X_BLACK);
  display.drawPixel(46, 51, SH110X_BLACK);
  display.drawPixel(45, 60, SH110X_BLACK);
  display.drawPixel(44, 60, SH110X_BLACK);
  display.drawPixel(43, 60, SH110X_BLACK);
  display.drawPixel(42, 60, SH110X_BLACK);
  display.drawPixel(41, 60, SH110X_BLACK);
  display.drawPixel(40, 60, SH110X_BLACK);
  display.drawPixel(38, 60, SH110X_BLACK);
  display.drawPixel(39, 60, SH110X_BLACK);
  display.drawPixel(37, 60, SH110X_BLACK);
  display.drawPixel(36, 60, SH110X_BLACK);
  display.drawPixel(35, 60, SH110X_BLACK);
  display.drawPixel(34, 60, SH110X_BLACK);
  display.drawPixel(33, 60, SH110X_BLACK);
  display.drawLine(33, 61, 45, 61, SH110X_WHITE);
  display.drawRect(60, 51, 39, 11, SH110X_WHITE);
  display.drawPixel(60, 51, SH110X_BLACK);
  display.drawPixel(60, 61, SH110X_BLACK);
  display.drawPixel(98, 61, SH110X_BLACK);
  display.drawPixel(98, 51, SH110X_BLACK);

  if(systemSubMode == SUBMODE_2_CMD ) {
    display.fillRect (15,15,100,45,SH110X_BLACK);
    display.drawRect (15,15,100,45,SH110X_WHITE);
    display.drawPixel(15,15,SH110X_BLACK);
    display.drawPixel(15,60,SH110X_BLACK);
    display.drawPixel(115,15,SH110X_BLACK);
    display.drawPixel(115,60,SH110X_BLACK);
    display.setCursor(55,33);
    sprintf(buffer, "0x%02X",  knobConfigurations[configKnobID].cmdbyte);
    display.print(buffer);
  }
  else
  if(systemSubMode == SUBMODE_2_TYPE) {
    display.fillRect (20,8,80,56,SH110X_BLACK);
    display.drawRect (20,8,80,56,SH110X_WHITE);
    display.setFont(&TomThumb);
    display.setTextColor(SH110X_WHITE);

    for(uint i=0; i < NUM_TYPE_CODES; i++) {
      display.setCursor(40, 14 + i*6);
      display.print(typeCodes[i].typeCodeName);
      if(i == textCursorPos) {
        if(toggle > 0) {
          display.setTextColor(SH110X_WHITE);
          display.setCursor(35, 14 + i*6);
          display.print(">");
        }
        else {
          display.setTextColor(SH110X_BLACK);
          display.setCursor(35, 14 + i*6);
          display.print(">");
          display.setTextColor(SH110X_WHITE);
        }
      }
    }

    display.setFont(); 
    display.setTextColor(SH110X_WHITE);
  }
  else
  if(systemSubMode == SUBMODE_2_NAME) {
    display.fillRect (7,12,114,48,SH110X_BLACK);
    display.drawRect (7,12,114,48,SH110X_WHITE);
    display.setCursor(8,50);
    display.print(" B1=Left  B2=Right");
    display.setCursor(21,21);
    uint slen = strlen(knobConfigurations[configKnobID].name);
    
    for(uint i=0; i < 15; i++) {
      display.setCursor(13 + i*7, 21);
      if(i < slen) {
        display.write(knobConfigurations[configKnobID].name[i]);
      }
      if(i == textCursorPos) {
        if(toggle > 0)
          display.drawLine(13 + i*7, 30, 17 + i*7, 30, SH110X_WHITE);
        else
          display.drawLine(13 + i*7, 30, 17 + i*7, 30, SH110X_BLACK);
      }
      else 
          display.drawLine(13 + i*7, 30, 17 + i*7, 30, SH110X_WHITE);
    }
  }

  display.display();
}


void drawConfigSelectionScreen() {
    uint btnCtr = 3;
    uint8_t iKnobCtr = 0;

    display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SH110X_BLACK); // clear screen
    display.setCursor(0,0);                                            // position for title
    display.printf("Config - Pick Knob");                              // title
    display.drawRect(0, 9, SCREEN_WIDTH, 54, SH110X_WHITE);            // box around UI
    display.drawRect(88, 15, 21, 11, SH110X_WHITE);                    // small screen rectangle
    for(int i=7; i<78; i = i + 10) {                                   // knobs on the left side
      for(int j=14; j<62; j = j + 8) {
          drawKnob(iKnobCtr, i,j,0);
          iKnobCtr++;
      }
    }
    for(int i=87; i < 98; i = i + 10) {                               // knobs on the right side
      for(int j=30; j < 62; j = j + 8) {
        drawKnob(iKnobCtr, i,j,0);
        iKnobCtr++;
      }
    }
    drawKnob(-1,109, 18, -1);                                              // encoder knob
    iKnobCtr++;


    for(int j=30; j < 62; j = j + 8) {                                 // buttons
      if(btnCtr == 3)
        drawButton(109, j, ledPattern & 0b10000000, ledPattern & 0b01000000, buttonStates[0]);
      else
      if(btnCtr == 2)
        drawButton(109, j, ledPattern & 0b00100000, ledPattern & 0b00010000, buttonStates[1]);
      else
      if(btnCtr == 1)
        drawButton(109, j, ledPattern & 0b00001000, ledPattern & 0b00000100, buttonStates[2]);
      else
        drawButton(109, j, ledPattern & 0b00000010, ledPattern & 0b00000001, buttonStates[3]);
      btnCtr--;
    }
    display.display();
}



void drawConfigScreen() {
  if(millis() - lastdrawTestScreen < 33) 
    return;

  lastdrawTestScreen = millis();

  if(systemSubMode == SUBMODE_2 || systemSubMode == SUBMODE_2_NAME || systemSubMode == SUBMODE_2_CMD || systemSubMode == SUBMODE_2_TYPE) {
    drawConfigKnobScreen();
  }
  else {
    drawConfigSelectionScreen();
  }
}

void drawRawOutputScreen() {
    display.setTextColor(SH110X_WHITE);
    display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SH110X_BLACK); // clear screen
    display.setCursor(0,0);                                            // position for title
    display.print("Raw Parameter Sender");                            // title

    display.setCursor(0,30);
    display.print("B1=Cmd Up/B2=Cmd Down");
    display.setCursor(0,40);
    display.print("B3=Byt Up/B4=Byt Down");
    display.setCursor(0,55);
    display.print("Press Encoder To Send");

    display.setCursor(12,15);
    sprintf(buffer, "CMD: %02X  BYTE: %02X", cmdValue, byteValue);
    display.print(buffer);

    display.display();
}



#endif