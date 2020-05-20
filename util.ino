//  initialize clock
void initClock(){
	if (! rtc.begin()) {
		tft.setCursor(0, 0); // 設定游標位置在第一行行首
		tft.print(F("Timer error"));
		while (1){tone(BUZZER, 300, 200);noTone(BUZZER);delay(200);}
	}
	Ds3231SqwPinMode mode = Ds3231SqwPinMode::DS3231_SquareWave1Hz; //output 1Hz at SQW pin
	rtc.writeSqwPinMode(mode);
	if (rtc.lostPower()) {
		tft.setCursor(0, 0); // 設定游標位置在第一行行首
		// tft.print(F("Clock lost power"));
		tft.setCursor(0, 1); // 設定游標位置在第一行行首
		// tft.print(F("Pls set the time!"));
		while(1){tone(BUZZER, 300, 200);noTone(BUZZER);delay(200);}
	}
}
void initSD(){
// initialize SD card  
	if(sd.begin(SS, SD_SCK_MHZ(50))){
		DateTime now = rtc.now();
		sprintf(fileName, "L%d%02d%02d_%02d%02d%02d", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
		
		if(!file.open(fileName, O_CREAT | O_WRITE))_isSD = false;
		else _isSD = true;
	}
	if(!_isSD){
		tft.setCursor(0, 0); // 設定游標位置在第一行行首
		tft.print(F("SD error"));
		while (1){tone(BUZZER, 300, 200);noTone(BUZZER);delay(200);}
	}
}
void singleClickLog(){
	Serial.println(F("logging"));
	if (_isLogging){_isLogging = false;}
	else{
		if (_isSD)
		{
			_curtime = 0;
			_isLogging = true;
		}else{
			// show no sd card
		}
	}
}