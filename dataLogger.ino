#include <SPI.h>
#include <TimerOne.h>
#include "Adafruit_MAX31855.h"
#include "RTClib.h"
#include "SdFat.h"
#include <OneButton.h>
#include "String.h"
#include "TFT_ILI9341.h"

#define TFT_GREY 0x7BEF


#define CLKSQW 2
#define BUZZER 12
#define CST0 5
#define CST1 6
#define CSSD 3
#define MAXCLK 8
#define MAXDO 9
#define BTN 46
#define BTNTIME 4

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
Adafruit_MAX31855 ts0(CST0);
Adafruit_MAX31855 ts1(CST1);

// TFT LCD
TFT_ILI9341 tft = TFT_ILI9341(); 

// button
OneButton btnLog(BTN, true);
OneButton btnTime(BTNTIME, true);


bool _isPPS = false;
bool _isLogging = false;
bool _isNotRefresh = false;
bool _isSD = false;
char fileName[16];

int _curtime = 0;
int _curRefreshTime = 0;

int b;
int idx = 0; //smoothing idx
int storePeriod[5] = {1, 10, 60, 600,3600};
int _pIdx=1;
short _prevIdx = 1; //for file system

// external interrupt
void onePPS(){ 
  noInterrupts();
  _isPPS = true;
  TCNT1 = 12449; //set timer1 counter 1 step before interrupt (pulse out), so that there'be a pulse right after receiving PPS signal
  interrupts();
}
//5Hz timer interrupt
ISR(TIMER1_COMPA_vect){ //5Hz
	_isNotRefresh = true;
}



void setup() {
	Serial.begin(115200);

	pinMode(A0, INPUT);
	pinMode(A1, INPUT);
	pinMode(A2, INPUT);
	pinMode(A3, INPUT);
	pinMode(CLKSQW, INPUT);
	pinMode(BUZZER, OUTPUT);
	pinMode(CSSD, OUTPUT);
	pinMode(CST0, OUTPUT);
	pinMode(CST1, OUTPUT);
//  init serial monitor
	
 
// initialize LCD  
	digitalWrite(CST0, HIGH);
	digitalWrite(CST1, HIGH);
	digitalWrite(CSSD, HIGH);
	tft.init();
	tft.setRotation(2);
	tft.fillScreen(TFT_BLACK);
	// Serial.println("done8");
  
// init clock module
	initClock();
	// Serial.println("done7");

// init SD module
  initSDFile(); 
	// Serial.println("done6");

// initialize interrupt
	TimerInit();	
	attachInterrupt(digitalPinToInterrupt(2), onePPS, RISING);
	// Serial.println("done5");

// set external voltage reference
	analogReference(INTERNAL2V56);
	// Serial.println("done4");

// button
	btnLog.attachClick(singleClickLog);
  btnTime.attachClick(singleClickTime);
	// Serial.println("done3");


	// Serial.println("done2");

// inittialize thermocoutple
	ts0.begin();
	ts1.begin();
	// Serial.println("done");

// initialize smoothing param
	idx = 0;
	_isLogging = false;
	_isNotRefresh = true;
// initialize LCD
  printTime();
  printSDStatus();
  printInterval();
  printLogStatus();
  printVoltage();
  printTemperature();
}

void loop() {
  
	if (_isPPS){
    printSerialVoltage();
    Serial.println();
		printTime();
		printVoltage();
		printTemperature();	

		if(_isLogging){
			
			_curtime = _curtime%storePeriod[_pIdx];
			if (_curtime==0)
			{
				storeData();
			}
      _curtime++;
		}
		_isPPS = false;
	}
	if(_isNotRefresh){
		_curRefreshTime++;
		readData();
		_isNotRefresh = false;
			
	}
  if(!_isLogging){
    btnTime.tick();  
  }
	btnLog.tick();
}
