#include <SPI.h>
#include <TimerOne.h>
#include "Adafruit_MAX31855.h"
#include "RTClib.h"
#include "SdFat.h"
#include <OneButton.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "String.h"

#define CLKSQW 2
#define BUZZER 4
#define CST0 5
#define CST1 6
#define MAXCLK 8
#define MAXDO 9

//
const int numOfReading = 10;
const int numOfData = 8; // v0~3, tsINT0~1, tsTC0~1,
double data[numOfData][numOfReading+1]={0.};

// time module
RTC_DS3231 rtc;

// SD card
SdFat sd;
SdFile file;

// thermocouple sensor
Adafruit_MAX31855 ts0(5);
Adafruit_MAX31855 ts1(6);
// Adafruit_MAX31855 ts0(SCK, 5, MISO);
// Adafruit_MAX31855 ts1(SCK, 6, MISO);
// Adafruit_MAX31855 ts0(MAXCLK, 5, MAXDO);
// Adafruit_MAX31855 ts1(MAXCLK, 6, MAXDO);
// TFT LCD
Adafruit_ILI9341 tft = Adafruit_ILI9341(-1, 13, 12, 11, 10, -1);

// button
OneButton btnLog(A5, LOW, true);


bool _isPPS = false;
bool _isLogging = false;
bool _isNotRefresh = false;
bool _isSD = false;
char fileName[16];

int _curtime = 0;
int _curRefreshTime = 0;

int b;
int idx = 0; //smoothing idx
int storePeriod[4] = {1, 10, 60, 3600};
int _pIdx=1;

void setup() {
	pinMode(A0, INPUT);
	pinMode(A1, INPUT);
	pinMode(A2, INPUT);
	pinMode(A3, INPUT);
	pinMode(A4, INPUT);
	pinMode(CLKSQW, INPUT);
	pinMode(BUZZER, OUTPUT);
//  init serial monitor
	Serial.begin(115200);
// initialize LCD  
	tft.begin();
	tft.fillScreen(ILI9341_BLACK);
// init clock module
	initClock();
// init SD module
	initSD(); 
// initialize interrupt
	attachInterrupt(digitalPinToInterrupt(CLKSQW), pps, RISING);
	Timer1.initialize(100000);
	Timer1.attachInterrupt(readData);
// set external voltage reference
	analogReference(INTERNAL2V56);
// button
	btnLog.attachClick(singleClickLog);

// initialize smoothing param
	idx = 0;
	_isLogging = false;
	_isNotRefresh = true;
// inittialize thermocoutple
	ts0.begin();
	ts1.begin();
}
void pps(){
	_isPPS = true;
}

void loop() {
		
	if(_isLogging){
		if(_isPPS){
			_isPPS = false;
			_curtime++;
			_curtime = _curtime%storePeriod[_pIdx];
			if (_curtime==0)
			{
				storeData();
			}
		}
	}
	if(_isNotRefresh){
		_curRefreshTime++;
		if (_curRefreshTime>=2)
		{
			printVoltage();
			printTemperature();
			printTime();
			_curRefreshTime = 0;
		}
		_isNotRefresh = false;
			
	}

	btnLog.tick();
}
void printVoltage(){
	char temp[7];
	
	for(int i=0;i<4;i++){
		tft.setCursor(0, 0+60*i);
  		tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);  tft.setTextSize(2);
  		tft.print("V ");
  		tft.print(i+1);
  		tft.setCursor(0, 20+60*i);
  		tft.setTextColor(ILI9341_RED, ILI9341_BLACK);  tft.setTextSize(3);
  		dtostrf(data[i][numOfReading], 2, 1, temp);
  		tft.print(padspace(String(temp), 4));
  		tft.setTextSize(1);
  		tft.print(" V");
  		
		
	}
}
void printTemperature(){
	char temp[7];
	
	for(int i=4;i<8;i++){
		tft.setCursor(120, 0+60*(i-4));
  		tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);  tft.setTextSize(2);
  		tft.print("T ");
  		tft.print(i-3);
  		tft.setCursor(120, 20+60*(i-4));
  		tft.setTextColor(ILI9341_BLUE, ILI9341_BLACK);  tft.setTextSize(3);
  		dtostrf(data[i][numOfReading], 2, 1, temp);
  		tft.print(padspace(String(temp), 5));
  		tft.setTextSize(1);
  		tft.print(" C");
	}	
}
void printTime(){
	DateTime now = rtc.now();
	char buffer[50]={0};
	sprintf(buffer, "%04d/%02d/%02d", now.year(), now.month(), now.day());
	tft.setCursor(0, 250);
  	tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);  tft.setTextSize(2);
  	tft.print(buffer);
  	sprintf(buffer, "%02d:%02d", now.hour(), now.minute());
  	tft.setCursor(0, 270);
  	tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);  tft.setTextSize(4);
  	tft.print(buffer);
}
void storeData(){
		char buffer[100]={0};
		char temp[7];
		DateTime now = rtc.now();
		sprintf(buffer, "%04d%02d%02dT%02d%02d%02d+0800,", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());

		
		
		noInterrupts();
		for(int i=0;i<numOfData;i++){
			dtostrf(data[i][numOfReading], 2, 2, temp); 
			sprintf(buffer+strlen(buffer), "%s,", temp);    
		}
		interrupts();
		
		sprintf(buffer+strlen(buffer), "\n");  
		Serial.print(buffer);
		
		file.print(buffer);
		file.flush();  
		double tuneValue = analogRead(A4);
		int interval = map(tuneValue, 0, 1024, 1, 6)*1000;
	
		

		tone(BUZZER, 300);
		delay(50);
		noTone(BUZZER);
	
}
void readData(){
	data[0][idx] = (analogRead(A0)+0.5)/1024.0*26.3977762880763;
	data[1][idx] = (analogRead(A1)+0.5)/1024.0*26.3977762880763;
	data[2][idx] = (analogRead(A2)+0.5)/1024.0*26.3977762880763;
	data[3][idx] = (analogRead(A3)+0.5)/1024.0*26.3977762880763;
	data[4][idx] = ts0.readCelsius();
	data[5][idx] = ts1.readCelsius();
	data[6][idx] = ts0.readInternal();
	data[7][idx] = ts1.readInternal();
	idx+=1;
	if(idx>=numOfReading)idx=0;
// average
	for(int i=0;i<numOfData;i++){
		data[i][numOfReading]=0;
		for(int j=0; j<numOfReading;j++){
			data[i][numOfReading]+=data[i][j];
		}
		data[i][numOfReading]/=numOfReading;
	}
	_isNotRefresh = true;
}
String padspace(String x, int len){
	while(x.length()<len){
		x = " "+x;
	}
	return x;
}
