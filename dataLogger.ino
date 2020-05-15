#include <SPI.h>
#include "Adafruit_MAX31855.h"
#include "RTClib.h"
#include "SdFat.h"
#include <LiquidCrystal_I2C.h>


#define BUZZER 4
#define CST0 5
#define CST1 6
#define MAXCLK 7
#define MAXDO 8


RTC_DS3231 rtc;

const uint8_t chipSelect = SS;
SdFat sd;
SdFile file;

Adafruit_MAX31855 ts0(MAXCLK, 5, MAXDO);
Adafruit_MAX31855 ts1(MAXCLK, 6, MAXDO);

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

void setup() {
  delay(2000);
  char fileName[13] = "data_cyk.txt";
  
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  pinMode(A4, INPUT);

  pinMode(BUZZER, OUTPUT);
  Serial.begin(115200);
  
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  if (!sd.begin(chipSelect, SD_SCK_MHZ(50))) {
    sd.initErrorHalt();
  }
  if (!file.open(fileName, O_CREAT | O_WRITE)) {
    Serial.println("Error occure when opening files");
  }
  lcd.begin(16, 2);
}

void loop() {
  double v0 = (analogRead(A0)+0.5)/1024.0*40.100164290005600000;
  double v1 = (analogRead(A1)+0.5)/1024.0*40.100164290005600000;
  double v2 = (analogRead(A2)+0.5)/1024.0*40.100164290005600000;
  double v3 = (analogRead(A3)+0.5)/1024.0*40.100164290005600000;

  DateTime now = rtc.now();
  double TS0Room = ts0.readInternal();
  double TS0TC = ts0.readCelsius();
  double TS1Room = ts1.readInternal();
  double TS1TC = ts1.readCelsius();
  double TSTimer = rtc.getTemperature();
  
  if(isnan(TS0TC))TS0TC=100.;
  if(isnan(TS1TC))TS1TC=100.;
  char buffer[100]={0};
  char temp[7];
  sprintf(buffer, "%04d/%02d%02dT%02d:%02d:%02d,", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
  dtostrf(v0, 2, 2, temp); sprintf(buffer+strlen(buffer), "%s,", temp);
  dtostrf(v1, 2, 2, temp); sprintf(buffer+strlen(buffer), "%s,", temp);
  dtostrf(v2, 2, 2, temp); sprintf(buffer+strlen(buffer), "%s,", temp);
  dtostrf(v3, 2, 2, temp); sprintf(buffer+strlen(buffer), "%s,", temp);
  dtostrf(TS0Room, 2, 2, temp); sprintf(buffer+strlen(buffer), "%s,", temp);
  dtostrf(TS0TC, 2, 2, temp); sprintf(buffer+strlen(buffer), "%s,", temp);
  dtostrf(TS1Room, 2, 2, temp); sprintf(buffer+strlen(buffer), "%s,", temp);
  dtostrf(TS1TC, 2, 2, temp); sprintf(buffer+strlen(buffer), "%s,", temp);
  dtostrf(TSTimer, 2, 2, temp); sprintf(buffer+strlen(buffer), "%s", temp);
  sprintf(buffer+strlen(buffer), "\n");  
  Serial.print(buffer);

  file.print(buffer);
  file.flush();

  double tuneValue = analogRead(A4);
  int interval = map(tuneValue, 0, 1024, 1, 6)*1000;
  tone(BUZZER, 300);
  delay(200);
  noTone(BUZZER);
  delay(interval-200);

  lcd.setCursor(0, 0); // 設定游標位置在第一行行首
  lcd.print("Hello, world!");
  lcd.setCursor(0, 1); // 設定游標位置在第二行行首
  lcd.print("turingTW");

}
