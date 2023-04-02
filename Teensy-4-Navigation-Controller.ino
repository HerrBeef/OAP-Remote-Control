#include <GT911.h>
#include <Wire.h>

//=============================================================================
//=============================================================================

/* SCREEN PIN CONNECTIONS
 *  VCC -> 5V
 *  GND -> GND
 *  CS -> 10
 *  RESET -> 8
 *  D/C -> 9
 *  MOSI/SDI -> 11
 *  SCL -> 13
 *  LED -> 3.3V
 *  SDO/MISO -> 12
 *  C_SCL -> 19
 *  C_RST -> 2
 *  -
 *  C_SDA -> 18
 *  C_INT -> 22
*/

#include <ILI9488_t3.h>
#include <ILI9488_t3_font_Arial.h>
#include <ILI9488_t3_font_ArialBold.h>
//===== ICONS ====
#include "home.c"
#include "up.c"
#include "left.c"
#include "right.c"
#include "down.c"
#include "turn_back.c"
#include "arrow_left.c"
#include "arrow_right.c"
#include "ok.c"
#include "rewind_button.c"
#include "forward_button.c"
#include "pause_play.c"
#include "phone_call.c"
#include "phone_hang_up.c"
#include "voice.c"
#include "navigation.c"
#include "sound_waves.c"
#include "dark_day_mode.c"
#include "drag_down.c"
#include "keyboard.c"
#include "lock.c"
#include "power.c"
#include "lock_big.c"
#include "volvo_logo.c"
#include "power_on.c"

#define INT_PIN 22
#define RST_PIN 2

#define TFT_RST 8
#define TFT_DC 9
#define TFT_CS 10
ILI9488_t3 tft = ILI9488_t3(&SPI, TFT_CS, TFT_DC, TFT_RST);

GT911 touch = GT911();

int lastScreenType = 0;
int SCREEN_TYPE = 0;
/* SCREENTYPE:
  0 = BOOTSCREEN / SLEEP
  1 = LOCK
  2 = Navigationremote
  3 = KEYBOARD
*/

int LOCK_STEP = 0;

unsigned long LAST_ACTION_PERFORMED = 0;


int lastX = 1000;
int lastY = 1000;

//===== REMOTE VARIABLES ======
int REMOTE_BUTTON_MARGIN = 4;
int REMOTE_BUTTON_WIDTH = 100;
int REMOTE_BUTTON_HEIGHT = 77;
int REMOTE_BUTTON_SPLITTER = 40;
int REMOTE_BUTTON_PADDING_LEFT = 32;
int REMOTE_BUTTON_PADDING_TOP = 23;

int REMOTE_BUTTON_SMALL_PADDING_LEFT = 14;
int REMOTE_BUTTON_SMALL_PADDING_TOP = 15;
int REMOTE_BUTTON_SMALL_WIDTH = 60;
int REMOTE_BUTTON_SMALL_HEIGHT = 60;
int SPECIAL_BUTTON_HEIGHT = 36;


//===== KEYBOARD VARIABLES ======
#define STRING_MAX_LENGTH   32

int KEY_ROWS      =      4;  //  jml
int KEY_WIDTH    =       48;
int KEY_HEIGHT      =    56;
int KEY_PADDING_LEFT  =  12;
int KEY_PADDING_TOP =   10;
int KEYBOARD_OFFSET_Y =  0;

int x, y, scanx, scany;
int Charset = 0;

const byte SY = '\1'; //jml - '\1' defines non-printable character
const byte SH = '\2';
const byte DL = '\3';
const byte ESC = '\4';
const byte CTRL = '\5';
const byte ALT = '\6';
const byte ALT_GR = '\7';

const char Keys[3][5][10] =
{
  { // Lowercase
    { ESC, CTRL, CTRL, ALT, ALT_GR, ALT_GR, ' ', ' ', ' ', ' '},
    { 'q', 'w', 'e', 'r', 't', 'z', 'u', 'i', 'o', 'p' },
    { 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', '\'' },
    { 'y', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '-' },
    { SY , SH , SH , ' ', ' ', ' ', ' ', ' ', DL , DL  }
  },

  { // Uppercase
    { ESC, CTRL, CTRL, ALT, ALT_GR, ALT_GR, ' ', ' ', ' ', ' '},
    { 'Q', 'W', 'E', 'R', 'T', 'Z', 'U', 'I', 'O', 'P' },
    { 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', '?' },
    { 'Y', 'X', 'C', 'V', 'B', 'N', 'M', ';', ':', '_' },
    { SY , SH , SH , ' ', ' ', ' ', ' ', ' ', DL , DL  }
  },

  { // Symbols
    { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' },
    { '?', '\'', '"', '$', '{', '}', ' ', ' ', ' ', ' '},
    { '!', '\\', '~', ',', '%', '&', '/', '(', ')', '=' },
    { '|', '<', '>', '[', ']', '^', '+', '*', '@', '#' },
    { SY , SH , SH , ' ', ' ', ' ', ' ', ' ', DL , DL  }
  }
};

// ====== END KEYBOARD VARIABLES =====



//=============================================================================
// Setup
//=============================================================================
void setup()
{
  tft.begin();

  delay(100);

  tft.fillScreen(ILI9488_BLACK);
  Serial.begin(115200);
  Wire.setClock(400000);
  Wire.begin();
  delay(300);

  drawSleepScreen();
  touchStart();
}

void drawSleepScreen()
{
  tft.setRotation(2); // 90
  tft.fillScreen(ILI9488_BLACK);
  tft.writeRect(320 / 2 - 100, 480 / 2 - 150, 200, 200, (uint16_t*)volvo_logo);

  tft.writeRect(320 / 2 - 32, 400, 64, 64, (uint16_t*)power_on);
}

void drawLock()
{
  tft.setRotation(2); // 90
  tft.fillScreen(ILI9488_BLACK);
  tft.writeRect(320 / 2 - 64, 480 / 2 - 64, 128, 128, (uint16_t*)lock_big);
}

void drawRemote()
{
  tft.setRotation(2); // 90
  tft.fillScreen(ILI9488_BLACK);
  tft.setTextColor(ILI9488_BLACK, ILI9488_WHITE);

  // BIG BUTTONS

  tft.fillRoundRect(REMOTE_BUTTON_MARGIN, 0, REMOTE_BUTTON_WIDTH, REMOTE_BUTTON_HEIGHT, 5, ILI9488_WHITE);
  tft.drawRoundRect(REMOTE_BUTTON_MARGIN, 0, REMOTE_BUTTON_WIDTH, REMOTE_BUTTON_HEIGHT, 5, ILI9488_BLACK);
  tft.writeRect(REMOTE_BUTTON_MARGIN + REMOTE_BUTTON_PADDING_LEFT, REMOTE_BUTTON_PADDING_TOP, 32, 32, (uint16_t*)arrow_left);

  tft.fillRoundRect(REMOTE_BUTTON_WIDTH + REMOTE_BUTTON_MARGIN * 2, 0, REMOTE_BUTTON_WIDTH, REMOTE_BUTTON_HEIGHT, 5, ILI9488_WHITE);
  tft.drawRoundRect(REMOTE_BUTTON_WIDTH + REMOTE_BUTTON_MARGIN * 2, 0, REMOTE_BUTTON_WIDTH, REMOTE_BUTTON_HEIGHT, 5, ILI9488_BLACK);
  tft.writeRect(REMOTE_BUTTON_MARGIN * 2 + REMOTE_BUTTON_PADDING_LEFT + REMOTE_BUTTON_WIDTH, REMOTE_BUTTON_PADDING_TOP, 32, 32, (uint16_t*)up);

  tft.fillRoundRect(REMOTE_BUTTON_WIDTH * 2 + REMOTE_BUTTON_MARGIN * 3, 0, REMOTE_BUTTON_WIDTH, REMOTE_BUTTON_HEIGHT, 5, ILI9488_WHITE);
  tft.drawRoundRect(REMOTE_BUTTON_WIDTH * 2 + REMOTE_BUTTON_MARGIN * 3, 0, REMOTE_BUTTON_WIDTH, REMOTE_BUTTON_HEIGHT, 5, ILI9488_BLACK);
  tft.writeRect(REMOTE_BUTTON_MARGIN * 3 + REMOTE_BUTTON_PADDING_LEFT + REMOTE_BUTTON_WIDTH * 2, REMOTE_BUTTON_PADDING_TOP, 32, 32, (uint16_t*)arrow_right);



  tft.fillRoundRect(REMOTE_BUTTON_MARGIN, REMOTE_BUTTON_HEIGHT + REMOTE_BUTTON_MARGIN, REMOTE_BUTTON_WIDTH, REMOTE_BUTTON_HEIGHT, 5, ILI9488_WHITE);
  tft.drawRoundRect(REMOTE_BUTTON_MARGIN, REMOTE_BUTTON_HEIGHT + REMOTE_BUTTON_MARGIN, REMOTE_BUTTON_WIDTH, REMOTE_BUTTON_HEIGHT, 5, ILI9488_BLACK);
  tft.writeRect(REMOTE_BUTTON_MARGIN + REMOTE_BUTTON_PADDING_LEFT, REMOTE_BUTTON_PADDING_TOP + REMOTE_BUTTON_HEIGHT + REMOTE_BUTTON_MARGIN, 32, 32, (uint16_t*)left);

  tft.fillRoundRect(REMOTE_BUTTON_WIDTH + REMOTE_BUTTON_MARGIN * 2, REMOTE_BUTTON_HEIGHT + REMOTE_BUTTON_MARGIN, REMOTE_BUTTON_WIDTH, REMOTE_BUTTON_HEIGHT, 5, ILI9488_WHITE);
  tft.drawRoundRect(REMOTE_BUTTON_WIDTH + REMOTE_BUTTON_MARGIN * 2, REMOTE_BUTTON_HEIGHT + REMOTE_BUTTON_MARGIN, REMOTE_BUTTON_WIDTH, REMOTE_BUTTON_HEIGHT, 5, ILI9488_BLACK);
  tft.writeRect(REMOTE_BUTTON_MARGIN * 2 + REMOTE_BUTTON_PADDING_LEFT + REMOTE_BUTTON_WIDTH, REMOTE_BUTTON_PADDING_TOP + REMOTE_BUTTON_HEIGHT + REMOTE_BUTTON_MARGIN, 32, 32, (uint16_t*)ok);

  tft.fillRoundRect(REMOTE_BUTTON_WIDTH * 2 + REMOTE_BUTTON_MARGIN * 3, REMOTE_BUTTON_HEIGHT + REMOTE_BUTTON_MARGIN, REMOTE_BUTTON_WIDTH, REMOTE_BUTTON_HEIGHT, 5, ILI9488_WHITE);
  tft.drawRoundRect(REMOTE_BUTTON_WIDTH * 2 + REMOTE_BUTTON_MARGIN * 3, REMOTE_BUTTON_HEIGHT + REMOTE_BUTTON_MARGIN, REMOTE_BUTTON_WIDTH, REMOTE_BUTTON_HEIGHT, 5, ILI9488_BLACK);
  tft.writeRect(REMOTE_BUTTON_MARGIN * 3 + REMOTE_BUTTON_PADDING_LEFT + REMOTE_BUTTON_WIDTH * 2, REMOTE_BUTTON_PADDING_TOP + REMOTE_BUTTON_HEIGHT + REMOTE_BUTTON_MARGIN, 32, 32, (uint16_t*)right);



  tft.fillRoundRect(REMOTE_BUTTON_MARGIN, REMOTE_BUTTON_HEIGHT * 2 + REMOTE_BUTTON_MARGIN * 2, REMOTE_BUTTON_WIDTH, REMOTE_BUTTON_HEIGHT, 5, ILI9488_WHITE);
  tft.drawRoundRect(REMOTE_BUTTON_MARGIN, REMOTE_BUTTON_HEIGHT * 2 + REMOTE_BUTTON_MARGIN * 2, REMOTE_BUTTON_WIDTH, REMOTE_BUTTON_HEIGHT, 5, ILI9488_BLACK);
  tft.writeRect(REMOTE_BUTTON_MARGIN + REMOTE_BUTTON_PADDING_LEFT, REMOTE_BUTTON_PADDING_TOP + REMOTE_BUTTON_HEIGHT * 2 + REMOTE_BUTTON_MARGIN * 2, 32, 32, (uint16_t*)home);

  tft.fillRoundRect(REMOTE_BUTTON_WIDTH + REMOTE_BUTTON_MARGIN * 2, REMOTE_BUTTON_HEIGHT * 2 + REMOTE_BUTTON_MARGIN * 2, REMOTE_BUTTON_WIDTH, REMOTE_BUTTON_HEIGHT, 5, ILI9488_WHITE);
  tft.drawRoundRect(REMOTE_BUTTON_WIDTH + REMOTE_BUTTON_MARGIN * 2, REMOTE_BUTTON_HEIGHT * 2 + REMOTE_BUTTON_MARGIN * 2, REMOTE_BUTTON_WIDTH, REMOTE_BUTTON_HEIGHT, 5, ILI9488_BLACK);
  tft.writeRect(REMOTE_BUTTON_MARGIN * 2 + REMOTE_BUTTON_PADDING_LEFT + REMOTE_BUTTON_WIDTH, REMOTE_BUTTON_PADDING_TOP + REMOTE_BUTTON_HEIGHT * 2 + REMOTE_BUTTON_MARGIN * 2, 32, 32, (uint16_t*)down);

  tft.fillRoundRect(REMOTE_BUTTON_WIDTH * 2 + REMOTE_BUTTON_MARGIN * 3, REMOTE_BUTTON_HEIGHT * 2 + REMOTE_BUTTON_MARGIN * 2, REMOTE_BUTTON_WIDTH, REMOTE_BUTTON_HEIGHT, 5, ILI9488_WHITE);
  tft.drawRoundRect(REMOTE_BUTTON_WIDTH * 2 + REMOTE_BUTTON_MARGIN * 3, REMOTE_BUTTON_HEIGHT * 2 + REMOTE_BUTTON_MARGIN * 2, REMOTE_BUTTON_WIDTH, REMOTE_BUTTON_HEIGHT, 5, ILI9488_BLACK);
  tft.writeRect(REMOTE_BUTTON_MARGIN * 3 + REMOTE_BUTTON_PADDING_LEFT + REMOTE_BUTTON_WIDTH * 2, REMOTE_BUTTON_PADDING_TOP + REMOTE_BUTTON_HEIGHT * 2 + REMOTE_BUTTON_MARGIN * 2, 32, 32, (uint16_t*)turn_back);

  //SMALL BUTTONS


  tft.fillRoundRect(REMOTE_BUTTON_MARGIN, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 2 + REMOTE_BUTTON_SPLITTER, REMOTE_BUTTON_SMALL_WIDTH, REMOTE_BUTTON_SMALL_HEIGHT, 5, ILI9488_WHITE);
  tft.drawRoundRect(REMOTE_BUTTON_MARGIN, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 2 + REMOTE_BUTTON_SPLITTER, REMOTE_BUTTON_SMALL_WIDTH, REMOTE_BUTTON_SMALL_HEIGHT, 5, ILI9488_BLACK);
  tft.writeRect(REMOTE_BUTTON_MARGIN + REMOTE_BUTTON_SMALL_PADDING_LEFT, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 2 + REMOTE_BUTTON_SPLITTER + REMOTE_BUTTON_SMALL_PADDING_TOP, 32, 32, (uint16_t*)voice);

  tft.fillRoundRect(REMOTE_BUTTON_MARGIN * 2 + REMOTE_BUTTON_SMALL_WIDTH, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 2 + REMOTE_BUTTON_SPLITTER, REMOTE_BUTTON_SMALL_WIDTH, REMOTE_BUTTON_SMALL_HEIGHT, 5, ILI9488_WHITE);
  tft.drawRoundRect(REMOTE_BUTTON_MARGIN * 2 + REMOTE_BUTTON_SMALL_WIDTH, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 2 + REMOTE_BUTTON_SPLITTER, REMOTE_BUTTON_SMALL_WIDTH, REMOTE_BUTTON_SMALL_HEIGHT, 5, ILI9488_BLACK);
  tft.writeRect(REMOTE_BUTTON_MARGIN * 2 + REMOTE_BUTTON_SMALL_WIDTH + REMOTE_BUTTON_SMALL_PADDING_LEFT, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 2 + REMOTE_BUTTON_SPLITTER + REMOTE_BUTTON_SMALL_PADDING_TOP, 32, 32, (uint16_t*)rewind_button);

  tft.fillRoundRect(REMOTE_BUTTON_MARGIN * 3 + REMOTE_BUTTON_SMALL_WIDTH * 2, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 2 + REMOTE_BUTTON_SPLITTER, REMOTE_BUTTON_SMALL_WIDTH, REMOTE_BUTTON_SMALL_HEIGHT, 5, ILI9488_WHITE);
  tft.drawRoundRect(REMOTE_BUTTON_MARGIN * 3 + REMOTE_BUTTON_SMALL_WIDTH * 2, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 2 + REMOTE_BUTTON_SPLITTER, REMOTE_BUTTON_SMALL_WIDTH, REMOTE_BUTTON_SMALL_HEIGHT, 5, ILI9488_BLACK);
  tft.writeRect(REMOTE_BUTTON_MARGIN * 3 + REMOTE_BUTTON_SMALL_WIDTH * 2 + REMOTE_BUTTON_SMALL_PADDING_LEFT, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 2 + REMOTE_BUTTON_SPLITTER + REMOTE_BUTTON_SMALL_PADDING_TOP, 32, 32, (uint16_t*)pause_play);

  tft.fillRoundRect(REMOTE_BUTTON_MARGIN * 4 + REMOTE_BUTTON_SMALL_WIDTH * 3, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 2 + REMOTE_BUTTON_SPLITTER, REMOTE_BUTTON_SMALL_WIDTH, REMOTE_BUTTON_SMALL_HEIGHT, 5, ILI9488_WHITE);
  tft.drawRoundRect(REMOTE_BUTTON_MARGIN * 4 + REMOTE_BUTTON_SMALL_WIDTH * 3, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 2 + REMOTE_BUTTON_SPLITTER, REMOTE_BUTTON_SMALL_WIDTH, REMOTE_BUTTON_SMALL_HEIGHT, 5, ILI9488_BLACK);
  tft.writeRect(REMOTE_BUTTON_MARGIN * 4 + REMOTE_BUTTON_SMALL_WIDTH * 3 + REMOTE_BUTTON_SMALL_PADDING_LEFT, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 2 + REMOTE_BUTTON_SPLITTER + REMOTE_BUTTON_SMALL_PADDING_TOP, 32, 32, (uint16_t*)forward_button);

  tft.fillRoundRect(REMOTE_BUTTON_MARGIN * 5 + REMOTE_BUTTON_SMALL_WIDTH * 4, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 2 + REMOTE_BUTTON_SPLITTER, REMOTE_BUTTON_SMALL_WIDTH, REMOTE_BUTTON_SMALL_HEIGHT, 5, ILI9488_WHITE);
  tft.drawRoundRect(REMOTE_BUTTON_MARGIN * 5 + REMOTE_BUTTON_SMALL_WIDTH * 4, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 2 + REMOTE_BUTTON_SPLITTER, REMOTE_BUTTON_SMALL_WIDTH, REMOTE_BUTTON_SMALL_HEIGHT, 5, ILI9488_BLACK);
  tft.writeRect(REMOTE_BUTTON_MARGIN * 5 + REMOTE_BUTTON_SMALL_WIDTH * 4 + REMOTE_BUTTON_SMALL_PADDING_LEFT, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 2 + REMOTE_BUTTON_SPLITTER + REMOTE_BUTTON_SMALL_PADDING_TOP, 32, 32, (uint16_t*)sound_waves);



  tft.fillRoundRect(REMOTE_BUTTON_MARGIN, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 3 + REMOTE_BUTTON_SPLITTER + REMOTE_BUTTON_SMALL_HEIGHT, REMOTE_BUTTON_SMALL_WIDTH, REMOTE_BUTTON_SMALL_HEIGHT, 5, ILI9488_WHITE);
  tft.drawRoundRect(REMOTE_BUTTON_MARGIN, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 3 + REMOTE_BUTTON_SPLITTER + REMOTE_BUTTON_SMALL_HEIGHT, REMOTE_BUTTON_SMALL_WIDTH, REMOTE_BUTTON_SMALL_HEIGHT, 5, ILI9488_BLACK);
  tft.writeRect(REMOTE_BUTTON_MARGIN + REMOTE_BUTTON_SMALL_PADDING_LEFT, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 3 + REMOTE_BUTTON_SPLITTER + REMOTE_BUTTON_SMALL_HEIGHT + REMOTE_BUTTON_SMALL_PADDING_TOP, 32, 32, (uint16_t*)phone_call);

  tft.fillRoundRect(REMOTE_BUTTON_MARGIN * 2 + REMOTE_BUTTON_SMALL_WIDTH, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 3 + REMOTE_BUTTON_SMALL_HEIGHT + REMOTE_BUTTON_SPLITTER, REMOTE_BUTTON_SMALL_WIDTH, REMOTE_BUTTON_SMALL_HEIGHT, 5, ILI9488_WHITE);
  tft.drawRoundRect(REMOTE_BUTTON_MARGIN * 2 + REMOTE_BUTTON_SMALL_WIDTH, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 3 + REMOTE_BUTTON_SMALL_HEIGHT + REMOTE_BUTTON_SPLITTER, REMOTE_BUTTON_SMALL_WIDTH, REMOTE_BUTTON_SMALL_HEIGHT, 5, ILI9488_BLACK);
  tft.writeRect(REMOTE_BUTTON_MARGIN * 2 + REMOTE_BUTTON_SMALL_WIDTH + REMOTE_BUTTON_SMALL_PADDING_LEFT, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 3 + REMOTE_BUTTON_SMALL_HEIGHT + REMOTE_BUTTON_SPLITTER + REMOTE_BUTTON_SMALL_PADDING_TOP, 32, 32, (uint16_t*)phone_hang_up);

  tft.fillRoundRect(REMOTE_BUTTON_MARGIN * 3 + REMOTE_BUTTON_SMALL_WIDTH * 2, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 3 + REMOTE_BUTTON_SMALL_HEIGHT + REMOTE_BUTTON_SPLITTER, REMOTE_BUTTON_SMALL_WIDTH, REMOTE_BUTTON_SMALL_HEIGHT, 5, ILI9488_WHITE);
  tft.drawRoundRect(REMOTE_BUTTON_MARGIN * 3 + REMOTE_BUTTON_SMALL_WIDTH * 2, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 3 + REMOTE_BUTTON_SMALL_HEIGHT + REMOTE_BUTTON_SPLITTER, REMOTE_BUTTON_SMALL_WIDTH, REMOTE_BUTTON_SMALL_HEIGHT, 5, ILI9488_BLACK);
  tft.writeRect(REMOTE_BUTTON_MARGIN * 3 + REMOTE_BUTTON_SMALL_WIDTH * 2 + REMOTE_BUTTON_SMALL_PADDING_LEFT, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 3 + REMOTE_BUTTON_SMALL_HEIGHT + REMOTE_BUTTON_SPLITTER + REMOTE_BUTTON_SMALL_PADDING_TOP, 32, 32, (uint16_t*)navigation);

  tft.fillRoundRect(REMOTE_BUTTON_MARGIN * 4 + REMOTE_BUTTON_SMALL_WIDTH * 3, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 3 + REMOTE_BUTTON_SMALL_HEIGHT + REMOTE_BUTTON_SPLITTER, REMOTE_BUTTON_SMALL_WIDTH, REMOTE_BUTTON_SMALL_HEIGHT, 5, ILI9488_WHITE);
  tft.drawRoundRect(REMOTE_BUTTON_MARGIN * 4 + REMOTE_BUTTON_SMALL_WIDTH * 3, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 3 + REMOTE_BUTTON_SMALL_HEIGHT + REMOTE_BUTTON_SPLITTER, REMOTE_BUTTON_SMALL_WIDTH, REMOTE_BUTTON_SMALL_HEIGHT, 5, ILI9488_BLACK);
  tft.writeRect(REMOTE_BUTTON_MARGIN * 4 + REMOTE_BUTTON_SMALL_WIDTH * 3 + REMOTE_BUTTON_SMALL_PADDING_LEFT, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 3 + REMOTE_BUTTON_SMALL_HEIGHT + REMOTE_BUTTON_SPLITTER + REMOTE_BUTTON_SMALL_PADDING_TOP, 32, 32, (uint16_t*)dark_day_mode);

  tft.fillRoundRect(REMOTE_BUTTON_MARGIN * 5 + REMOTE_BUTTON_SMALL_WIDTH * 4, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 3 + REMOTE_BUTTON_SMALL_HEIGHT + REMOTE_BUTTON_SPLITTER, REMOTE_BUTTON_SMALL_WIDTH, REMOTE_BUTTON_SMALL_HEIGHT, 5, ILI9488_WHITE);
  tft.drawRoundRect(REMOTE_BUTTON_MARGIN * 5 + REMOTE_BUTTON_SMALL_WIDTH * 4, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 3 + REMOTE_BUTTON_SMALL_HEIGHT + REMOTE_BUTTON_SPLITTER, REMOTE_BUTTON_SMALL_WIDTH, REMOTE_BUTTON_SMALL_HEIGHT, 5, ILI9488_BLACK);
  tft.writeRect(REMOTE_BUTTON_MARGIN * 5 + REMOTE_BUTTON_SMALL_WIDTH * 4 + REMOTE_BUTTON_SMALL_PADDING_LEFT, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 3 + REMOTE_BUTTON_SMALL_HEIGHT + REMOTE_BUTTON_SPLITTER + REMOTE_BUTTON_SMALL_PADDING_TOP, 32, 32, (uint16_t*)drag_down);

  //BOTTOM ROW
  /* NOT NEEDED FOR NOW
    tft.fillRoundRect(REMOTE_BUTTON_MARGIN, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 6 + REMOTE_BUTTON_SPLITTER + REMOTE_BUTTON_SMALL_HEIGHT * 2, REMOTE_BUTTON_SMALL_WIDTH, REMOTE_BUTTON_SMALL_HEIGHT, 5, ILI9488_WHITE);
    tft.drawRoundRect(REMOTE_BUTTON_MARGIN, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 6 + REMOTE_BUTTON_SPLITTER + REMOTE_BUTTON_SMALL_HEIGHT * 2, REMOTE_BUTTON_SMALL_WIDTH, REMOTE_BUTTON_SMALL_HEIGHT, 5, ILI9488_BLACK);
    tft.writeRect(REMOTE_BUTTON_MARGIN + REMOTE_BUTTON_SMALL_PADDING_LEFT, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 6 + REMOTE_BUTTON_SPLITTER + REMOTE_BUTTON_SMALL_HEIGHT * 2 + REMOTE_BUTTON_SMALL_PADDING_TOP, 32, 32, (uint16_t*)phone_call);

    tft.fillRoundRect(REMOTE_BUTTON_MARGIN * 2 + REMOTE_BUTTON_SMALL_WIDTH, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 6 + REMOTE_BUTTON_SMALL_HEIGHT * 2 + REMOTE_BUTTON_SPLITTER, REMOTE_BUTTON_SMALL_WIDTH, REMOTE_BUTTON_SMALL_HEIGHT, 5, ILI9488_WHITE);
    tft.drawRoundRect(REMOTE_BUTTON_MARGIN * 2 + REMOTE_BUTTON_SMALL_WIDTH, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 6 + REMOTE_BUTTON_SMALL_HEIGHT * 2 + REMOTE_BUTTON_SPLITTER, REMOTE_BUTTON_SMALL_WIDTH, REMOTE_BUTTON_SMALL_HEIGHT, 5, ILI9488_BLACK);
    tft.writeRect(REMOTE_BUTTON_MARGIN * 2 + REMOTE_BUTTON_SMALL_WIDTH + REMOTE_BUTTON_SMALL_PADDING_LEFT, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 6 + REMOTE_BUTTON_SMALL_HEIGHT * 2 + REMOTE_BUTTON_SPLITTER + REMOTE_BUTTON_SMALL_PADDING_TOP, 32, 32, (uint16_t*)phone_hang_up);
  */
  tft.fillRoundRect(REMOTE_BUTTON_MARGIN * 3 + REMOTE_BUTTON_SMALL_WIDTH * 2, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 6 + REMOTE_BUTTON_SMALL_HEIGHT * 2 + REMOTE_BUTTON_SPLITTER, REMOTE_BUTTON_SMALL_WIDTH, REMOTE_BUTTON_SMALL_HEIGHT, 5, ILI9488_WHITE);
  tft.drawRoundRect(REMOTE_BUTTON_MARGIN * 3 + REMOTE_BUTTON_SMALL_WIDTH * 2, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 6 + REMOTE_BUTTON_SMALL_HEIGHT * 2 + REMOTE_BUTTON_SPLITTER, REMOTE_BUTTON_SMALL_WIDTH, REMOTE_BUTTON_SMALL_HEIGHT, 5, ILI9488_BLACK);
  tft.writeRect(REMOTE_BUTTON_MARGIN * 3 + REMOTE_BUTTON_SMALL_WIDTH * 2 + REMOTE_BUTTON_SMALL_PADDING_LEFT, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 6 + REMOTE_BUTTON_SMALL_HEIGHT * 2 + REMOTE_BUTTON_SPLITTER + REMOTE_BUTTON_SMALL_PADDING_TOP, 32, 32, (uint16_t*)power);

  tft.fillRoundRect(REMOTE_BUTTON_MARGIN * 4 + REMOTE_BUTTON_SMALL_WIDTH * 3, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 6 + REMOTE_BUTTON_SMALL_HEIGHT * 2 + REMOTE_BUTTON_SPLITTER, REMOTE_BUTTON_SMALL_WIDTH, REMOTE_BUTTON_SMALL_HEIGHT, 5, ILI9488_WHITE);
  tft.drawRoundRect(REMOTE_BUTTON_MARGIN * 4 + REMOTE_BUTTON_SMALL_WIDTH * 3, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 6 + REMOTE_BUTTON_SMALL_HEIGHT * 2 + REMOTE_BUTTON_SPLITTER, REMOTE_BUTTON_SMALL_WIDTH, REMOTE_BUTTON_SMALL_HEIGHT, 5, ILI9488_BLACK);
  tft.writeRect(REMOTE_BUTTON_MARGIN * 4 + REMOTE_BUTTON_SMALL_WIDTH * 3 + REMOTE_BUTTON_SMALL_PADDING_LEFT, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 6 + REMOTE_BUTTON_SMALL_HEIGHT * 2 + REMOTE_BUTTON_SPLITTER + REMOTE_BUTTON_SMALL_PADDING_TOP, 32, 32, (uint16_t*)lock);

  tft.fillRoundRect(REMOTE_BUTTON_MARGIN * 5 + REMOTE_BUTTON_SMALL_WIDTH * 4, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 6 + REMOTE_BUTTON_SMALL_HEIGHT * 2 + REMOTE_BUTTON_SPLITTER, REMOTE_BUTTON_SMALL_WIDTH, REMOTE_BUTTON_SMALL_HEIGHT, 5, ILI9488_WHITE);
  tft.drawRoundRect(REMOTE_BUTTON_MARGIN * 5 + REMOTE_BUTTON_SMALL_WIDTH * 4, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 6 + REMOTE_BUTTON_SMALL_HEIGHT * 2 + REMOTE_BUTTON_SPLITTER, REMOTE_BUTTON_SMALL_WIDTH, REMOTE_BUTTON_SMALL_HEIGHT, 5, ILI9488_BLACK);
  tft.writeRect(REMOTE_BUTTON_MARGIN * 5 + REMOTE_BUTTON_SMALL_WIDTH * 4 + REMOTE_BUTTON_SMALL_PADDING_LEFT, REMOTE_BUTTON_HEIGHT * 3 + REMOTE_BUTTON_MARGIN * 6 + REMOTE_BUTTON_SMALL_HEIGHT * 2 + REMOTE_BUTTON_SPLITTER + REMOTE_BUTTON_SMALL_PADDING_TOP, 32, 32, (uint16_t*)keyboard);

}

void drawKeyboard()
{
  tft.setRotation(3); // 180
  tft.fillScreen(ILI9488_BLACK);
  tft.setFont(Arial_14_Bold);

  // CANCEL
  tft.fillRoundRect(0, 320 - 36, 130, 36, 5, ILI9488_YELLOW);
  tft.drawRoundRect(0, 320 - 36, 130, 36, 5, ILI9488_WHITE);
  tft.setTextColor(ILI9488_BLACK, ILI9488_YELLOW);
  tft.setCursor(10, 320 - 36 + 12);
  tft.print("KEYBOARD");

  // DONE
  tft.fillRoundRect(480 - 100, 320 - 36, 120, 36, 5, ILI9488_GREEN);
  tft.drawRoundRect(480 - 100, 320 - 36, 120, 36, 5, ILI9488_WHITE);
  tft.setTextColor(ILI9488_WHITE, ILI9488_GREEN);
  tft.setCursor(480 - 120 + 40, 320 - 36 + 12);
  tft.print("ENTER");
  tft.setTextColor(ILI9488_BLACK, ILI9488_WHITE);

  for (y = 0; y < KEY_ROWS; ++y)
  {
    for (x = 0; x < 10; ++x)
    {
      tft.fillRoundRect((x * KEY_WIDTH), KEYBOARD_OFFSET_Y + (y * KEY_HEIGHT), KEY_WIDTH, KEY_HEIGHT, 5, ILI9488_WHITE);
      tft.drawRoundRect((x * KEY_WIDTH), KEYBOARD_OFFSET_Y + (y * KEY_HEIGHT), KEY_WIDTH, KEY_HEIGHT, 5, ILI9488_BLACK);

      // Print characters
      tft.setCursor(KEY_PADDING_LEFT + (x * KEY_WIDTH), KEYBOARD_OFFSET_Y + KEY_PADDING_TOP + (y * KEY_HEIGHT));
      tft.write(Keys[Charset][y][x]);
    }
  }

  // SYMBOLS
  tft.fillRoundRect(0, KEYBOARD_OFFSET_Y + (KEY_ROWS * KEY_HEIGHT), KEY_WIDTH, KEY_HEIGHT, 5, ILI9488_WHITE);
  tft.drawRoundRect(0, KEYBOARD_OFFSET_Y + (KEY_ROWS * KEY_HEIGHT), KEY_WIDTH, KEY_HEIGHT, 5, ILI9488_BLACK);
  tft.setCursor(2, KEYBOARD_OFFSET_Y + KEY_PADDING_TOP + (KEY_ROWS * KEY_HEIGHT));
  tft.print(Charset == 2 ? "ABC" : "123");

  if (Charset != 2) {

    // ESC
    tft.fillRoundRect(0, 0, KEY_WIDTH, KEY_HEIGHT, 5, ILI9488_WHITE);
    tft.drawRoundRect(0, 0, KEY_WIDTH, KEY_HEIGHT, 5, ILI9488_BLACK);
    tft.setCursor(2, KEYBOARD_OFFSET_Y + KEY_PADDING_TOP + 0);
    tft.print("ESC");

    // CTRL
    tft.fillRoundRect(KEY_WIDTH, 0, KEY_WIDTH * 2, KEY_HEIGHT, 5, ILI9488_WHITE);
    tft.drawRoundRect(KEY_WIDTH, 0, KEY_WIDTH * 2, KEY_HEIGHT, 5, ILI9488_BLACK);
    tft.setCursor(KEY_WIDTH + 14, KEYBOARD_OFFSET_Y + KEY_PADDING_TOP + 0);
    tft.print("CTRL");

    // ALT
    tft.fillRoundRect(KEY_WIDTH * 3, 0, KEY_WIDTH, KEY_HEIGHT, 5, ILI9488_WHITE);
    tft.drawRoundRect(KEY_WIDTH * 3, 0, KEY_WIDTH, KEY_HEIGHT, 5, ILI9488_BLACK);
    tft.setCursor(KEY_WIDTH * 3 + 4, KEYBOARD_OFFSET_Y + KEY_PADDING_TOP + 0);
    tft.print("ALT");

    // ALT GR
    tft.fillRoundRect(KEY_WIDTH * 4, 0, KEY_WIDTH * 2, KEY_HEIGHT, 5, ILI9488_WHITE);
    tft.drawRoundRect(KEY_WIDTH * 4, 0, KEY_WIDTH * 2, KEY_HEIGHT, 5, ILI9488_BLACK);
    tft.setCursor(KEY_WIDTH * 4 + 14, KEYBOARD_OFFSET_Y + KEY_PADDING_TOP + 0);
    tft.print("ALT GR");
  }

  // SHIFT
  tft.fillRoundRect(KEY_WIDTH, KEYBOARD_OFFSET_Y + (KEY_ROWS * KEY_HEIGHT), KEY_WIDTH * 2, KEY_HEIGHT, 5, ILI9488_WHITE);
  tft.drawRoundRect(KEY_WIDTH, KEYBOARD_OFFSET_Y + (KEY_ROWS * KEY_HEIGHT), KEY_WIDTH * 2, KEY_HEIGHT, 5, ILI9488_BLACK);
  tft.setCursor(14 + KEY_WIDTH, KEYBOARD_OFFSET_Y + KEY_PADDING_TOP + (KEY_ROWS * KEY_HEIGHT));
  tft.print(Charset ? "SHIFT" : "Shift");

  // SPACE
  tft.fillRoundRect(3 * KEY_WIDTH, KEYBOARD_OFFSET_Y + (KEY_ROWS * KEY_HEIGHT), KEY_WIDTH * 5, KEY_HEIGHT, 5, ILI9488_WHITE);
  tft.drawRoundRect(3 * KEY_WIDTH, KEYBOARD_OFFSET_Y + (KEY_ROWS * KEY_HEIGHT), KEY_WIDTH * 5, KEY_HEIGHT, 5, ILI9488_BLACK);
  tft.setCursor(30 + 4 * KEY_WIDTH, KEYBOARD_OFFSET_Y + KEY_PADDING_TOP + (KEY_ROWS * KEY_HEIGHT));
  tft.print(Charset ? "SPACE" : "Space");

  // DELETE
  tft.fillRoundRect(8 * KEY_WIDTH, KEYBOARD_OFFSET_Y + (KEY_ROWS * KEY_HEIGHT), KEY_WIDTH * 2, KEY_HEIGHT, 5, ILI9488_WHITE);
  tft.drawRoundRect(8 * KEY_WIDTH, KEYBOARD_OFFSET_Y + (KEY_ROWS * KEY_HEIGHT), KEY_WIDTH * 2, KEY_HEIGHT, 5, ILI9488_BLACK);
  tft.setCursor(6 + 8 * KEY_WIDTH, KEYBOARD_OFFSET_Y + KEY_PADDING_TOP + (KEY_ROWS * KEY_HEIGHT));
  tft.print(Charset ? "DELETE" : "Delete");
}

bool withinErrorRange(int lastPoint, int point)
{
  return fabs(lastPoint - point) <= 2;
}

void handleTouch(int8_t contacts, GTPoint* points) {

  for (uint8_t i = 0; i < contacts; i++) {
    if (millis() > (LAST_ACTION_PERFORMED + 250) && !(lastX == points[i].x) && !(lastY == points[i].y)) {   //!(withinErrorRange(lastX, points[i].x)) && !(withinErrorRange(lastY, points[i].y))
      LAST_ACTION_PERFORMED = millis();
      lastX = points[i].x;
      lastY = points[i].y;
      //Serial.printf("Point %d at %d, %d  size: %d  track ID: %d\n", i, points[i].x, points[i].y, points[i].size, points[i].trackId);

      switch (SCREEN_TYPE) {
        case 0:
          handleTouchForSleepScreen(points[i].x, points[i].y);
          break;
        case 1:
          handleTouchForLock(points[i].x, points[i].y);
          break;
        case 2:
          handleTouchForRemote(points[i].x, points[i].y);
          break;
        case 3:
          handleTouchForKeyboard(points[i].x, points[i].y);
          break;
        default:
          // statements
          break;
      }
    }
  }
}

void handleTouchForSleepScreen(int x, int y)
{
  if (y < 80 && y > 0) {
    SCREEN_TYPE = 2;
  }
}

void handleTouchForLock(int x, int y)
{
  if (y > 240 && y < 480 && x < 320 && x > 160) {
    Serial.println("LOCK STEP 1");
    LOCK_STEP = 1;
  } else if (LOCK_STEP == 1 && y < 240 && y > 0 && x < 320 && x < 160) {
    Serial.println("LOCK STEP 2");
    LOCK_STEP = 2;
  } else if (LOCK_STEP == 2 && y < 240 && y > 0 && x < 320 && x > 160) {
    Serial.println("UNLOCKED");
    SCREEN_TYPE = 2;
  } else {
    Serial.println("LOCK STEP 0");
    LOCK_STEP = 0;
  }
}

void handleTouchForRemote(int x, int y)
{
  if (y > 240 && y < 480) {
    //BIG BUTTONS
    Serial.println("BIG BUTTON AREA");

    int horizontalButtonDetectionSize = 320 / 3;
    int verticalButtonDetectionSize = 240 / 3;
    if (y > (480 - verticalButtonDetectionSize)) {
      //FIRST ROW
      Serial.println("FIRST ROW");
      if (x > (320 - horizontalButtonDetectionSize)) {
        //1 BIG BUTTON
        Serial.println("1 BIG BUTTON");
        Keyboard.press(KEY_1);
        delay(50);
        Keyboard.release(KEY_1);

      } else if (x > (320 - 2 * horizontalButtonDetectionSize)) {
        //2 BIG BUTTON
        Serial.println("2 BIG BUTTON");
        Keyboard.press(KEY_UP);
        delay(50);
        Keyboard.release(KEY_UP);
      } else {
        //3 BIG BUTTON
        Serial.println("3 BIG BUTTON");
        Keyboard.press(KEY_2);
        delay(50);
        Keyboard.release(KEY_2);
      }
    } else if (y > (480 - 2 * verticalButtonDetectionSize)) {
      //SECOND ROW
      Serial.println("SECOND ROW");
      Serial.println("FIRST ROW");
      if (x > (320 - horizontalButtonDetectionSize)) {
        //4 BIG BUTTON
        Serial.println("4 BIG BUTTON");
        Keyboard.press(KEY_LEFT);
        delay(50);
        Keyboard.release(KEY_LEFT);
      } else if (x > (320 - 2 * horizontalButtonDetectionSize)) {
        //5 BIG BUTTON
        Serial.println("5 BIG BUTTON");
        Keyboard.press(KEY_ENTER);
        delay(50);
        Keyboard.release(KEY_ENTER);
      } else {
        //6 BIG BUTTON
        Serial.println("6 BIG BUTTON");
        Keyboard.press(KEY_RIGHT);
        delay(50);
        Keyboard.release(KEY_RIGHT);
      }
    } else {
      //THIRD ROW
      Serial.println("THIRD ROW");
      Serial.println("FIRST ROW");
      if (x > (320 - horizontalButtonDetectionSize)) {
        //7 BIG BUTTON
        Serial.println("7 BIG BUTTON");
        Keyboard.press(KEY_H);
        delay(50);
        Keyboard.release(KEY_H);
      } else if (x > (320 - 2 * horizontalButtonDetectionSize)) {
        //8 BIG BUTTON
        Serial.println("8 BIG BUTTON");
        Keyboard.press(KEY_DOWN);
        delay(50);
        Keyboard.release(KEY_DOWN);
      } else {
        //9 BIG BUTTON
        Serial.println("9 BIG BUTTON");
        Keyboard.press(KEY_ESC);
        delay(50);
        Keyboard.release(KEY_ESC);
      }
    }

  } else if (y < 240 && y > 80) {
    //SMALL BUTTONS
    Serial.println("SMALL BUTTON AREA");
    int horizontalButtonDetectionSize = 320 / 5;
    int verticalButtonDetectionSize = 140 / 2;
    if (y > (220 - verticalButtonDetectionSize)) {
      //FIRST ROW
      Serial.println("FIRST ROW");
      if (x > (320 - horizontalButtonDetectionSize)) {
        //1 SMALL BUTTON
        Serial.println("1 SMALL BUTTON");
        Keyboard.press(KEY_M);
        delay(50);
        Keyboard.release(KEY_M);
      } else if (x > (320 - 2 * horizontalButtonDetectionSize)) {
        //2 SMALL BUTTON
        Serial.println("2 SMALL BUTTON");
        Keyboard.press(KEY_V);
        delay(50);
        Keyboard.release(KEY_V);
      } else if (x > (320 - 3 * horizontalButtonDetectionSize)) {
        //3 SMALL BUTTON
        Serial.println("3 SMALL BUTTON");
        Keyboard.press(KEY_B);
        delay(50);
        Keyboard.release(KEY_B);
      } else if (x > (320 - 4 * horizontalButtonDetectionSize)) {
        //4 SMALL BUTTON
        Serial.println("4 SMALL BUTTON");
        Keyboard.press(KEY_N);
        delay(50);
        Keyboard.release(KEY_N);
      } else if (x > (320 - 5 * horizontalButtonDetectionSize)) {
        //5 SMALL BUTTON
        Serial.println("5 SMALL BUTTON");
        Keyboard.press(KEY_J);
        delay(50);
        Keyboard.release(KEY_J);
      }
    } else if (y > (220 - 2 * verticalButtonDetectionSize)) {
      //SECOND ROW
      Serial.println("SECOND ROW");
      Serial.println("FIRST ROW");
      if (x > (320 - horizontalButtonDetectionSize)) {
        //6 SMALL BUTTON
        Serial.println("6 SMALL BUTTON");

        Keyboard.press(KEY_P);
        delay(50);
        Keyboard.release(KEY_P);
      } else if (x > (320 - 2 * horizontalButtonDetectionSize)) {
        //7 SMALL BUTTON
        Serial.println("7 SMALL BUTTON");
        Keyboard.press(KEY_O);
        delay(50);
        Keyboard.release(KEY_O);
      } else if (x > (320 - 3 * horizontalButtonDetectionSize)) {
        //8 SMALL BUTTON
        Serial.println("8 SMALL BUTTON");
        Keyboard.press(KEY_F);
        delay(50);
        Keyboard.release(KEY_F);
      } else if (x > (320 - 4 * horizontalButtonDetectionSize)) {
        //9 SMALL BUTTON
        Serial.println("9 SMALL BUTTON");
        Keyboard.press(KEY_F2);
        delay(50);
        Keyboard.release(KEY_F2);
      } else if (x > (320 - 5 * horizontalButtonDetectionSize)) {
        //10 SMALL BUTTON
        Serial.println("10 SMALL BUTTON");
        Keyboard.press(KEY_F6);
        delay(50);
        Keyboard.release(KEY_F6);
      }
    }
  } else if (y < 80 && y > 0) {

    int horizontalButtonDetectionSize = 320 / 5;
    //MAIN FUNCTION BUTTONS
    Serial.println("MAIN FUNCTION BUTTON AREA");
    if (x > (320 - horizontalButtonDetectionSize)) {
      //1 MAIN FUNCTION BUTTON
      Serial.println("1 MAIN FUNCTION BUTTON");
    } else if (x > (320 - 2 * horizontalButtonDetectionSize)) {
      //2 MAIN FUNCTION BUTTON
      Serial.println("2 MAIN FUNCTION BUTTON");
    } else if (x > (320 - 3 * horizontalButtonDetectionSize)) {
      //3 MAIN FUNCTION BUTTON
      Serial.println("3 MAIN FUNCTION BUTTON");
      SCREEN_TYPE = 0;
    } else if (x > (320 - 4 * horizontalButtonDetectionSize)) {
      //4 MAIN FUNCTION BUTTON
      Serial.println("4 MAIN FUNCTION BUTTON");
      SCREEN_TYPE = 1;
    } else if (x > (320 - 5 * horizontalButtonDetectionSize)) {
      //5 MAIN FUNCTION BUTTON
      Serial.println("5 MAIN FUNCTION BUTTON");
      SCREEN_TYPE = 3;
    }
  }
}

void handleTouchForKeyboard(int x, int y)
{
  if ((y < 480 && y > (480 - 130)) && (x < 320 && x > (320 - 20))) {
    SCREEN_TYPE = 2;
    return;
  }

  if ((y < 100) && (x < 320 && x > (320 - 20))) {
    Keyboard.press(KEY_ENTER);
    delay(50);
    Keyboard.release(KEY_ENTER);
    return;
  }

  int TouchX = -1, TouchY = -1;
  for (scanx = 0; scanx < 10; ++scanx)    // Scan X
    if (((480 - y) >= (scanx * KEY_WIDTH)) && ((480 - y) <= (scanx * KEY_WIDTH + KEY_WIDTH))) {
      TouchX = scanx;
      break;

    }
  for (scany = 0; scany < KEY_ROWS + 1; ++scany)  // Scan Y
    if ((x >= (KEYBOARD_OFFSET_Y + scany * KEY_HEIGHT)) && (x <= (KEYBOARD_OFFSET_Y + scany * KEY_HEIGHT + KEY_HEIGHT))) {
      TouchY = scany;
      break;

    }

  if (TouchX < 0 || TouchY < 0)  return;  // Hit a key?

  if (Keys[Charset][TouchY][TouchX] == SH) {      // Shift, change character set
    Charset = !Charset;
    drawKeyboard();
  } else if (Keys[Charset][TouchY][TouchX] == SY) {  // Switch to Symbols
    Charset = Charset < 2 ? 2 : 0;
    drawKeyboard();
  } else if (Keys[Charset][TouchY][TouchX] == DL) {  // Switch to Symbols
    Serial.println("Press key: BACKSPACE");
    Keyboard.press(KEY_BACKSPACE);
    delay(50);
    Keyboard.release(KEY_BACKSPACE);
  } else if (Keys[Charset][TouchY][TouchX] == ESC) {  // Switch to Symbols
    Serial.println("Press key: ESC");
    Keyboard.press(KEY_ESC);
    delay(50);
    Keyboard.release(KEY_ESC);
  } else if (Keys[Charset][TouchY][TouchX] == CTRL) {  // Switch to Symbols

  } else if (Keys[Charset][TouchY][TouchX] == ALT) {  // Switch to Symbols

  } else if (Keys[Charset][TouchY][TouchX] == ALT_GR) {  // Switch to Symbols

  } else {            // Type character (You should actually check for "")
    char pressKey = Keys[Charset][TouchY][TouchX];
    Serial.printf("Press key: ");
    Serial.print(pressKey);
    Serial.println();
    Keyboard.print(pressKey);
    delay(50);
  }
}

void touchStart() {
  touch.begin(INT_PIN, RST_PIN, GT911_I2C_ADDR_28, handleTouch);
  Wire.beginTransmission(touch.getI2CAddress());
  touch.readInfo(); // you can get product id and other information with:
  Serial.println(touch.getInfo().productId);
}


//=============================================================================
// Loop
//=============================================================================
void loop()
{
  if (lastScreenType != SCREEN_TYPE) {
    lastScreenType = SCREEN_TYPE;
    switch (SCREEN_TYPE) {
      case 0:
        drawSleepScreen();
        break;
      case 1:
        drawLock();
        break;
      case 2:
        drawRemote();
        break;
      case 3:
        drawKeyboard();
        break;
      default:
        SCREEN_TYPE = 0;
        lastScreenType = 0;
        drawSleepScreen();
        break;
    }
  }
  touch.loop();
}
