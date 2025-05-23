#include <SoftwareSerial.h>
#include <Wire.h>
#include <math.h>

#include "Adafruit_Sensor.h"
#include "Adafruit_AM2320.h"
#include <Adafruit_BMP085.h>
#include <LiquidCrystal_I2C.h>

#include "Forecaster.h"

#define seaLevelPressure_hPa 1013.25

String PORT = "80";
String AP = "YOURAP";
String PASS = "WIFIPASSWORD";

int successOrder = 0;

Adafruit_BMP085 bmp;
SoftwareSerial esp(7, 6);
Adafruit_AM2320 am2320 = Adafruit_AM2320();
Forecaster cond;
String values;

LiquidCrystal_I2C lcd(0x27, 16, 2);

uint16_t showorder = 0;
uint8_t sendorder = 0;

void setup() {
  Serial.begin(9600);
  esp.begin(9600);

  if (!bmp.begin()) {
    Serial.println("BMP180 NOT FOUND!");
    while (1) {}
  }

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("WEATHER STATION");

  successOrder = 0;

  while (!Serial) {
    delay(10);
  }

  am2320.begin();
}

void loop() {
  float temp = am2320.readTemperature();
  float humid = am2320.readHumidity();
  float pressure = bmp.readPressure();

  static uint32_t tmr;
  static uint32_t data_millis;
  static boolean first = false;
  static boolean didSetH = false;

  // Serial.println("new loop");

  if (!didSetH)
  {
   cond.setH(bmp.readAltitude(seaLevelPressure_hPa * 100));
   didSetH = true;
  }

  if (millis() - tmr >= 2*60*1000ul) {
    tmr = millis();
    cond.addP(pressure, temp);
    Serial.println(cond.getCast());
  }

  // lcd.setCursor(0, 1);
  //Serial.println(requestSendForecast);

  // if (millis() - data_millis >= 2*1000ul) {
  //   //boolean done = sendDataESP(temp, humid, pressure);
  //   Serial.println("Doing it now");
  //   boolean done = true;

  //   if (!first) {
  //     // lcd.print("Sending data....");
  //     first = true;
  //   }

  //   if (done) {
  //     data_millis = millis();
  //     clearLCDLine(1);
  //     // lcd.print("SUCCESS");
  //     first = false;
  //   }
  // }
  // else 
  // {
    clearLCDLine(1);

    switch (showorder++) {
      case 0: lcd.print("Tmp:" + String(temp) + "*C"); break;
      case 1: lcd.print("Hum:" + String(humid) + "%"); break;
      case 2: lcd.print("Prs:" + String(pressure/100) + "hPa"); break;
      case 3: lcd.print("P:" + getForecastText(round(cond.getCast()))); showorder = 0; break;
    }
  // }

  values = String(temp) + ',' + String(humid) + ',' + String(pressure) + ',' + String( round(cond.getCast()) );

  switch (sendorder++) {
    case 0: esp.flush(); break;
    case 1: Serial.println(values); esp.print(values); sendorder = 0; break;
  }

  delay(2000);
}

void clearLCDLine(int line) {
  for(int n = 0; n < 16; n++)
  {
    lcd.setCursor(n,line);
    lcd.print(" ");
  }
  lcd.setCursor(0,line);
}

String getForecastText(int zambrettiNumber) {
  switch(zambrettiNumber)   
    {
    case 1:
        return "STABLE FINE";
    case 2:
        return "FINE";
    case 3:
        return "UNSTABLE FINE";
    case 4:
        return "DEVELOPING";
    case 5:
        return "SHOWERS";
    case 6:
        return "UNSETTLED";
    case 7:
        return "RAIN";
    case 8:
        return "UNSETTLED RAIN";
    case 9:
        return "BAD RAIN";
    case 10:
        return "SETTLED FINE";
    case 11:
        return "FINE";
    case 12:
        return "UNSTABLE FINE";
    case 13:
        return "FAIRLY FINE";
    case 14:
        return "SHOWERY";
    case 15:
        return "SOME RAIN";
    case 16:
        return "UNSETTLED RAIN";
    case 17:
        return "FREQUENT RAIN";
    case 18:
        return "BAD RAIN";
    case 19:
        return "STORMY RAIN";
    case 20:
        return "STABLE FINE";
    case 21:
        return "FINE";
    case 22:
        return "TURNING FINE";
    case 23:
        return "IMPROVING";
    case 24:
        return "SHOWERS, FINE";
    case 25:
        return "SHOWERS, IMPR.";
    case 26:
        return "CHANGEABLE";
    case 27:
        return "UNSETTLED, CLEAR";
    case 28:
        return "UNSETTLED, IMPR.";
    case 29:
        return "SHORT FINE";
    case 30:
        return "V. UNSETTLED";
    case 31:
        return "STORMY, IMPR.";
    case 32:
        return "STORMY, RAIN";
    default:
        return "DATA TOO SMALL";
    }
}
