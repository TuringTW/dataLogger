//  initialize clock
void TimerInit()
{
// timer1 for pulser
  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0; //reset timer1 counter 

  TCCR1B |= (1 << WGM12); // CTC Mode(Clear timer on compare match)
  TCCR1B |= (1 << CS12);  // 256 prescalar: setCS11 = 1

// for pulse output
  OCR1A = 12500; // compare register of counterA
  TIMSK1 |= (1 << OCIE1A); // Enable timer compare interrupt


  
  interrupts();
}


void initClock(){
	if (! rtc.begin()) {
		tft.setCursor(0, 0); // 設定游標位置在第一行行首
		tft.print(F("Timer error"));
		while (1){tone(BUZZER, 300, 200);noTone(BUZZER);delay(200);}
	}
	Ds3231SqwPinMode mode = Ds3231SqwPinMode::DS3231_SquareWave1Hz; //output 1Hz at SQW pin
	rtc.writeSqwPinMode(mode);
	if (rtc.lostPower()) {
		// tft.setCursor(0, 0); // 設定游標位置在第一行行首
		// tft.print(F("Clock lost power"));
		// tft.setCursor(0, 1); // 設定游標位置在第一行行首
		// tft.print(F("Pls set the time!"));
		// while(1){tone(BUZZER, 300, 200);noTone(BUZZER);delay(200);}
	}
}
void initSDFile(){
// initialize SD card  
	if(sd.begin(CSSD, SD_SCK_MHZ(50))){
    _isSD = true;
		
    short i = 0;
    do{
      i++;
      sprintf(fileName, "L%03d", i);
    }while(sd.exists(fileName));
    _prevIdx = i;
    sprintf(fileName, " OK ");
	}
}
void getLogFile(){
	short i = _prevIdx;
	do{
		i++;
		sprintf(fileName, "L%03d", i);
		Serial.println(fileName);
	}while(sd.exists(fileName));
  _prevIdx = i;
	Serial.println("hello");
	if(!file.open(fileName, O_CREAT | O_WRITE))_isSD = false;
	else _isSD = true;
}
void singleClickLog(){
	Serial.println(F("logging"));
	if (_isLogging){
		_isLogging = false;
		file.close();
    printLogStatus();

    tone(BUZZER, 500);
    delay(100);
    noTone(BUZZER);
    delay(100);
    tone(BUZZER, 400);
    delay(100);
    noTone(BUZZER);
    delay(100);
    tone(BUZZER, 300);
    delay(100);
    noTone(BUZZER);
    delay(100);
	}
	else{
		getLogFile();
    printSDStatus();
		if (_isSD)
		{
			_curtime = 0;
			_isLogging = true;
      printLogStatus();
      for(int i=3;i<=5;i++){
          tone(BUZZER, i*100);
          delay(100);
          noTone(BUZZER);
          delay(100);
      }
		}else{
      if(sd.begin(CSSD, SD_SCK_MHZ(50))){
        _isSD = true;
        sprintf(fileName, " OK ");
        printSDStatus();
        short i = 0;
        do{
          i++;
          sprintf(fileName, "L%03d", i);
          Serial.println(fileName);
        }while(sd.exists(fileName));
        _prevIdx = i-1;
        singleClickLog();
      }else{
  			_isLogging = false;
        for(int i=0;i<5;i++){
          tone(BUZZER, 300);
          delay(100);
          noTone(BUZZER);
          delay(50);
          tone(BUZZER, 300);
          delay(100);
          noTone(BUZZER);
          delay(200);
        }
      }
		}
      
    
    
    
	}
}
void singleClickTime(){
  _pIdx=(_pIdx+1)%5;
  tone(BUZZER, 600);
  printInterval();
  delay(50);
  tone(BUZZER, 700);
  delay(50);
  noTone(BUZZER);
  
}
void printVoltage(){
  char temp[7];
  int startPos = 92;
  for(int i=0;i<4;i++){
    tft.setCursor(0, startPos+60*i,1);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);  tft.setTextSize(2);
      tft.print("V ");
      tft.print(i+1);
      tft.setCursor(0, startPos+20+60*i);
      tft.setTextColor(TFT_RED, TFT_BLACK);  tft.setTextSize(3);
      dtostrf(data[i][numOfReading], 2, 1, temp);
      tft.print(padspace(String(temp), 4));
      tft.setTextSize(1);
      tft.print(" V");
      
    
  }
}
void printSerialVoltage(){
  char temp[10];
  for(int i=0;i<4;i++){
      dtostrf(data[i][numOfReading], 2, 4, temp);
      Serial.print(temp);
      Serial.print("\t"); 
  }
}
void printTemperature(){
  char temp[7];
  int startPos = 92;
  for(int i=4;i<8;i++){
    tft.setCursor(120, startPos+60*(i-4),1);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);  tft.setTextSize(2);
      tft.print("T ");
      tft.print(i-3);
      tft.setCursor(120, startPos+20+60*(i-4));
      tft.setTextColor(TFT_BLUE, TFT_BLACK);  tft.setTextSize(3);
      dtostrf(data[i][numOfReading], 2, 1, temp);
      tft.print(padspace(String(temp), 5));
      tft.setTextSize(1);
      tft.print(" C");
  } 
}
void printTime(){
  DateTime now = rtc.now();
  char buffer[50]={0};
  sprintf(buffer, " %04d/%02d/%02d", now.year(), now.month(), now.day());
  tft.setCursor(0, 0, 1);
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.print(buffer);
    if (now.second()%2==0){
      sprintf(buffer, "%02d:%02d", now.hour(), now.minute()); 
    }else{
      sprintf(buffer, "%02d %02d", now.hour(), now.minute()); 
    }
    tft.setTextSize(1);
    tft.setCursor(0, 28, 7);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.print(buffer);
}
void printSDStatus(){
  tft.setCursor(150, 0, 1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);  tft.setTextSize(2);
  tft.print("SD:");
  if(_isSD){
    tft.setTextColor(TFT_GREEN, TFT_BLACK);  tft.setTextSize(2);
    tft.print(fileName);
  }else{
    tft.setTextColor(TFT_RED, TFT_BLACK);  tft.setTextSize(2);
    tft.print(" NO ");
  }
}
void printInterval(){
  tft.setCursor(150, 25, 1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);  tft.setTextSize(2);
  tft.print("TI:");
  char temp[7];
  int output=storePeriod[_pIdx];
  char unit='s';
  if(output/60>0){
    output /= 60;  
    unit='m';
    if(output/60>0){
      output/=60;
      unit='h';  
    }
  }
  sprintf(temp, "%3d", output);
  tft.print(temp);
  tft.print(unit);
}
void printLogStatus(){
  if(_isLogging){
    tft.fillRoundRect(150,45,85,30, 5, TFT_RED);  
    tft.setCursor(167,50);
    tft.setTextSize(3);
    tft.setTextColor(TFT_BLACK, TFT_RED);
    tft.print("REC"); 
  }else{
    tft.fillRoundRect(150,45,85,30, 5, TFT_GREEN);  
    tft.setCursor(159,50);
    tft.setTextSize(3);
    tft.setTextColor(TFT_BLACK,TFT_GREEN);
    tft.print("STBY");
  }
  
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
  
    

    tone(BUZZER, 500);
    delay(100);
    noTone(BUZZER);
  
}
void readData(){
  data[0][idx] = (analogRead(A0)+0.5)/1024.0*46.7876549+0.13679976;
  data[1][idx] = (analogRead(A1)+0.5)/1024.0*46.7876549+0.13679976;
  data[2][idx] = (analogRead(A2)+0.5)/1024.0*46.7876549+0.13679976;
  data[3][idx] = (analogRead(A3)+0.5)/1024.0*46.7876549+0.13679976;
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
}
String padspace(String x, int len){
  while(x.length()<len){
    x = " "+x;
  }
  return x;
}
