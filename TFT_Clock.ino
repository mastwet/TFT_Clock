/*
  An example analogue clock using a TFT LCD screen to show the time
  use of some of the drawing commands with the ST7735 library.

  For a more accurate clock, it would be better to use the RTClib library.
  But this is just a demo.

  Uses compile time to set the time so a reset will start with the compile time again

  Gilchrist 6/2/2014 1.0
  Updated by Bodmer
*/

#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include <WiFi.h>
#include "font.h"
#include "RTClib.h"
#include <NTPClient.h>

RTC_DS1307 rtc;

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

#define TFT_GREY 0xBDF7

float sx = 0, sy = 1, mx = 1, my = 0, hx = -1, hy = 0;    // Saved H, M, S x & y multipliers
float sdeg = 0, mdeg = 0, hdeg = 0;
uint16_t osx = 90, osy = 119, omx = 90, omy = 119, ohx = 90, ohy = 119; // Saved H, M, S x & y coords
uint16_t x0 = 0, x1 = 0, yy0 = 0, yy1 = 0;
uint32_t targetTime = 0;                    // for next 1 second timeout


byte omm = 99, oss = 99;
byte xcolon = 0, xsecs = 0;
unsigned int colour = 0;

static uint8_t conv2d(const char* p) {
  uint8_t v = 0;
  if ('0' <= *p && *p <= '9')
    v = *p - '0';
  return 10 * v + *++p - '0';
}

uint8_t hh = conv2d(__TIME__), mm = conv2d(__TIME__ + 3), ss = conv2d(__TIME__ + 6); // Get H, M, S from compile time

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP,"ntp.aliyun.com");
char* WIFI_SSID = "";
char* WIFI_PASSWD = "";

bool initial = 1;

void setup(void) {

  pinMode(23, OUTPUT);
  digitalWrite(23, HIGH);
  Serial.begin(115200);
  Wire.begin(21, 22);

  int rtc_ok = 0;

  delay(100);
  tft.init();
  tft.fillScreen(TFT_BLACK);
  tft.setRotation(1);
  tft.setCursor(0, 0, 4);
  tft.setTextColor(TFT_BLUE, TFT_BLACK);

  if (! rtc.begin()) {
    tft.setTextColor(0x39C4, TFT_BLACK);
    tft.print(">>> Couldn't find RTC,Try network Time");
    while (1) delay(10);
  }
  else{
    rtc_ok = 1;
  }
  
  tft.println(">>> Connecting to WiFi:");
  tft.println(WIFI_SSID);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWD);
  unsigned long timeout = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    tft.print(".");
    if (millis() - timeout > 11451) {
      tft.println("");
      tft.println(">>> Connect Timeout !");
      while(1) delay(50);
    }
  }
  if((WiFi.status() == WL_CONNECTED)){
  tft.println(">>> Connect Sucess !Updating Times...");
  
  timeClient.begin();
  timeClient.setTimeOffset(28800); 
  timeClient.update();
  
  int currentSec = timeClient.getSeconds();
  int currentMinute = timeClient.getMinutes();
  int currentHour = timeClient.getHours();
  tft.print(">>> Adjust time to:");
  tft.print(currentHour);
  tft.print(":");
  tft.print(currentMinute);
  tft.print(":");
  tft.println(currentSec);
  if(rtc_ok){
      rtc.adjust(DateTime(2014, 1, 21, currentHour, currentMinute, currentSec));
      tft.print(">>> Update Sucess!");
  }
  timeClient.end();
  WiFi.disconnect(true);//断开wifi网络
  WiFi.mode(WIFI_OFF);//关闭网络
  
  }

  delay(2000);

//  if (! rtc.isrunning()) {
//    Serial.println("RTC is NOT running, let's set the time!");
//    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
//    // This line sets the RTC with an explicit date & time, for example to set
//    // January 21, 2014 at 3am you would call:
//    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
//  }


  DateTime now = rtc.now();
  hh = now.hour();
  mm = now.minute();
  ss = now.second();


  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);  // Adding a black background colour erases previous text automatically

  // Draw clock face
  tft.fillCircle(90, 119, 71, TFT_BLUE);
  tft.fillCircle(90, 119, 67, TFT_BLACK);

  // Draw 12 lines
  for (int i = 0; i < 360; i += 30) {
    sx = cos((i - 90) * 0.0174532925);
    sy = sin((i - 90) * 0.0174532925);
    x0 = sx * 67 + 90;
    yy0 = sy * 67 + 119;
    x1 = sx * 60 + 90;
    yy1 = sy * 60 + 119;

    tft.drawLine(x0, yy0, x1, yy1, TFT_BLUE);
  }

  // Draw 60 dots
  for (int i = 0; i < 360; i += 6) {
    sx = cos((i - 90) * 0.0174532925);
    sy = sin((i - 90) * 0.0174532925);
    x0 = sx * 63 + 90;
    yy0 = sy * 63 + 119;

    tft.drawPixel(x0, yy0, TFT_BLUE);
    if (i == 0 || i == 180) tft.fillCircle(x0, yy0, 1, TFT_CYAN);
    if (i == 0 || i == 180) tft.fillCircle(x0 + 1, yy0, 1, TFT_CYAN);
    if (i == 90 || i == 270) tft.fillCircle(x0, yy0, 1, TFT_CYAN);
    if (i == 90 || i == 270) tft.fillCircle(x0 + 1, yy0, 1, TFT_CYAN);
  }

  tft.fillCircle(90, 120, 3, TFT_RED);

  // Draw text at position 119,125 using fonts 4
  // Only font numbers 2,4,6,7 are valid. Font 6 only contains characters [space] 0 1 2 3 4 5 6 7 8 9 : . a p m
  // Font 7 is a 7 segment font and only contains characters [space] 0 1 2 3 4 5 6 7 8 9 : .

  targetTime = millis() + 1000;
  //String test2 = "时间"; // Unicodes 0x4EDD, 0x5000
  //tft.loadFont(font123);
  //tft.drawCentreString(test2,64,130,1);
}

void print_test() {
  DateTime now = rtc.now();

  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" (");
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();

  Serial.print(" since midnight 1/1/1970 = ");
  Serial.print(now.unixtime());
  Serial.print("s = ");
  Serial.print(now.unixtime() / 86400L);
  Serial.println("d");

  // calculate a date which is 7 days, 12 hours, 30 minutes, and 6 seconds into the future
  DateTime future (now + TimeSpan(7, 12, 30, 6));

  Serial.print(" now + 7d + 12h + 30m + 6s: ");
  Serial.print(future.year(), DEC);
  Serial.print('/');
  Serial.print(future.month(), DEC);
  Serial.print('/');
  Serial.print(future.day(), DEC);
  Serial.print(' ');
  Serial.print(future.hour(), DEC);
  Serial.print(':');
  Serial.print(future.minute(), DEC);
  Serial.print(':');
  Serial.print(future.second(), DEC);
  Serial.println();

  Serial.println();
}

void loop() {
  if (targetTime < millis()) {
    targetTime = millis() + 1000;
    ss++;              // Advance second
    if (ss == 60) {
      ss = 0;
      mm++;            // Advance minute
      if (mm > 59) {
        mm = 0;
        hh++;          // Advance hour
        if (hh > 23) {
          hh = 0;
        }
      }
    }


    // Update digital time
    int xpos = 180;
    int ypos = 100; // Top left corner ot clock text, about half way down
    int ysecs = ypos;

    if (omm != mm) { // Redraw hours and minutes time every minute
      omm = mm;
      // Draw hours and minutes
      if (hh < 10) xpos += tft.drawChar('0', xpos, ypos, 6); // Add hours leading zero for 24 hr clock
      xpos += tft.drawNumber(hh, xpos, ypos, 6);             // Draw hours
      xcolon = xpos; // Save colon coord for later to flash on/off later
      xpos += tft.drawChar(':', xpos, ypos, 6);
      if (mm < 10) xpos += tft.drawChar('0', xpos, ypos, 6); // Add minutes leading zero
      xpos += tft.drawNumber(mm, xpos, ypos, 6);             // Draw minutes
      xsecs = xpos; // Sae seconds 'x' position for later display updates
    }
    if (oss != ss) { // Redraw seconds time every second
      oss = ss;
      xpos = xsecs;

      if (ss % 2) { // Flash the colons on/off
        tft.setTextColor(0x39C4, TFT_BLACK);        // Set colour to grey to dim colon
        tft.drawChar(':', xcolon, ypos, 6);     // Hour:minute colon
        xpos += tft.drawChar(':', xsecs + 220, ysecs + 48, 4); // Seconds colon
        tft.setTextColor(TFT_GREEN, TFT_BLACK);    // Set colour back to yellow
      }
      else {
        tft.drawChar(':', xcolon, ypos, 6);     // Hour:minute colon
        xpos += tft.drawChar(':', xsecs + 220, ysecs + 48, 4); // Seconds colon
      }

      //Draw seconds
      if (ss < 10) xpos += tft.drawChar('0', xpos + 220, ysecs + 48, 4); // Add leading zero
      tft.drawNumber(ss, xpos + 220, ysecs + 48, 4);                 // Draw seconds
    }

    // Pre-compute hand degrees, x & y coords for a fast screen update
    sdeg = ss * 6;                // 0-59 -> 0-354
    mdeg = mm * 6 + sdeg * 0.01666667; // 0-59 -> 0-360 - includes seconds
    hdeg = hh * 30 + mdeg * 0.0833333; // 0-11 -> 0-360 - includes minutes and seconds
    hx = cos((hdeg - 90) * 0.0174532925);
    hy = sin((hdeg - 90) * 0.0174532925);
    mx = cos((mdeg - 90) * 0.0174532925);
    my = sin((mdeg - 90) * 0.0174532925);
    sx = cos((sdeg - 90) * 0.0174532925);
    sy = sin((sdeg - 90) * 0.0174532925);

    if (ss == 0 || initial) {
      initial = 0;
      // Erase hour and minute hand positions every minute
      tft.drawLine(ohx, ohy, 90, 120, TFT_BLACK);
      ohx = hx * 33 + 90;
      ohy = hy * 33 + 120;
      tft.drawLine(omx, omy, 90, 120, TFT_BLACK);
      omx = mx * 44 + 90;
      omy = my * 44 + 120;
    }

    // Redraw new hand positions, hour and minute hands not erased here to avoid flicker
    tft.drawLine(osx, osy, 90, 120, TFT_BLACK);
    tft.drawLine(ohx, ohy, 90, 120, TFT_WHITE);
    tft.drawLine(omx, omy, 90, 120, TFT_WHITE);
    osx = sx * 47 + 90;
    osy = sy * 47 + 120;
    tft.drawLine(osx, osy, 90, 120, TFT_RED);

    tft.fillCircle(90, 120, 3, TFT_RED);
    //print_test();
  }
}
