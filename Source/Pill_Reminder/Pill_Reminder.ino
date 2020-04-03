#include <Adafruit_SleepyDog.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#include <Wire.h> 
//
#define ENABLE_INTRO_HOURS 0
//
#define BTWILLY 2 // Top //
#define BTMONET 3 // Bottom //
#define MUTE A1 // Lowest one //
#define BUZZER 8
#define RTCON A3
#define WILLY_LED A2
#define MONET_LED A0
#define BELL 1
#define MUTE_MACHINE 0
//
tmElements_t tm;
char secondsCounter = 99;
bool doAlarm = false;
char alarmPosition = 0;
uint8_t alarmMutedTime = 0;
char willyPos = 0;
char monetPos = 0;
char willyPosPrev = 0;
char monetPosPrev = 0;
char prevMute = 0;
bool doQuickBuzz = false;
bool isWilly = false;
bool isMonet = false;
char prevHour = 0;
char prevMuteMachine = 0;

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup() 
{
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);
  pinMode(RTCON, OUTPUT);
  digitalWrite(RTCON, LOW);
  pinMode(MUTE, INPUT);
  digitalWrite(MUTE, HIGH);
  prevMute = digitalRead(MUTE);
  pinMode(WILLY_LED, OUTPUT);
  pinMode(MONET_LED, OUTPUT);
  pinMode(MUTE_MACHINE, INPUT);
  digitalWrite(MUTE_MACHINE, HIGH);
  pinMode(BELL, OUTPUT);
  digitalWrite(BELL, LOW);
  //
  digitalWrite(MONET_LED, HIGH);
  delay(100);
  digitalWrite(MONET_LED, LOW);
  //
  delay(500);
  digitalWrite(WILLY_LED, HIGH);
  delay(100);
  digitalWrite(WILLY_LED, LOW);
  //
  setTime(9, 59, 56, 1, 1, 2019);
  //
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  //
  digitalWrite(2, HIGH);
  digitalWrite(3, HIGH);
  //
  willyPos = digitalRead(2);
  monetPos = digitalRead(3);
  //
  willyPosPrev = willyPos;
  monetPosPrev = monetPos;
  //
  attachInterrupt(0, interruptFunction, CHANGE);
  attachInterrupt(1, interruptFunction, CHANGE);
  //
  digitalWrite(RTCON, HIGH);
  //
  #if ENABLE_INTRO_HOURS
    if (RTC.read(tm))
    {
      for (int x = 0; x < tm.Hour; x++)
      {
        digitalWrite(BUZZER, HIGH);
        delay(4);
        digitalWrite(BUZZER, LOW);
        delay(290);
      }
      //
      delay(2000);
      //
      for (int x = 0; x < tm.Minute; x++)
      {
        digitalWrite(BUZZER, HIGH);
        delay(4);
        digitalWrite(BUZZER, LOW);
        delay(290);
      }  
    }
    else
    {
      digitalWrite(RTCON, LOW);
      digitalWrite(BUZZER, HIGH);
      delay(1000);
      digitalWrite(BUZZER, LOW);
      delay(1000);
      while (1) { Watchdog.sleep(8000); }
    }
  #else
    if (RTC.read(tm))
    {
      digitalWrite(RTCON, LOW);
      digitalWrite(BUZZER, HIGH);
      delay(20);
      digitalWrite(BUZZER, LOW);
      delay(200);
      digitalWrite(BUZZER, HIGH);
      delay(20);
      digitalWrite(BUZZER, LOW);
      delay(2000);
    }
    else
    {
      digitalWrite(RTCON, LOW);
      digitalWrite(BUZZER, HIGH);
      delay(1000);
      digitalWrite(BUZZER, LOW);
      delay(1000);
      while (1) { Watchdog.sleep(8000); }
    }
  #endif
  //
  playBellHour();
 }

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void loop() 
{
  if (digitalRead(MUTE_MACHINE) == LOW)
  {
    if (prevMuteMachine == 0) quickBuzz();
    prevMuteMachine = 1;
    Watchdog.sleep(8000);
    return;
  }
  else
  {
    if (prevMuteMachine == 1) quickBuzz();
    prevMuteMachine = 0;
  }
  //
  willyPos = digitalRead(2);
  monetPos = digitalRead(3);
  //
  willyPosPrev = willyPos;
  monetPosPrev = monetPos;
  //
  if (doQuickBuzz)
  {
    doQuickBuzz = false;
    //
    digitalWrite(WILLY_LED, HIGH);    
    digitalWrite(MONET_LED, HIGH);    
    //
    for (char ww = 0; ww < 8; ww++)
    {
      digitalWrite(BUZZER, HIGH);
      delay(1);
      digitalWrite(BUZZER, LOW);
      delay(22);
    }
    //
    digitalWrite(WILLY_LED, LOW);    
    digitalWrite(MONET_LED, LOW);     
  }
  //
  if (doAlarm)
  {
    digitalWrite(RTCON, HIGH);
    delay(100);
    //    
    if (RTC.read(tm))
    {
      digitalWrite(RTCON, LOW);
      //
      // Check Willy //
      bool continueAlarm = false;
      if (willyPos == LOW && tm.Hour >= 10 && tm.Hour < 15) { continueAlarm = true; isWilly = true; }
      if (willyPos == HIGH && tm.Hour >= 22 && tm.Minute >= 30) { continueAlarm = true; isWilly = true; }
      //
      // Check Monet //
      if (monetPos == LOW && tm.Hour >= 10 && tm.Hour < 14) { continueAlarm = true; isMonet = true; }
      if (monetPos == HIGH && tm.Hour >= 16 && tm.Hour < 22) { continueAlarm = true; isMonet = true; }
      //
      if (!continueAlarm) 
      {
        doAlarm = false;
        doQuickBuzz = true;
        return;
      }
      //
      if (tm.Hour >= 23 && tm.Minute >= 55 && tm.Hour <= 9)
      {
        doAlarm = false;
        return;
      }
    } 
    digitalWrite(RTCON, LOW);
    //
    if (alarmMutedTime > 0)
    {
      Watchdog.sleep(8000);
      alarmMutedTime--;
    }
    else
    {
      if (prevMute != digitalRead(MUTE))
      {
        prevMute = digitalRead(MUTE);
        alarmMutedTime = 224;
        doQuickBuzz = true;
        return;
      }
      //
      if (alarmPosition <= 20)
      {
        if (isWilly) digitalWrite(WILLY_LED, HIGH);    
        if (isMonet) digitalWrite(MONET_LED, HIGH);    
        //
        playBell();
        //
        delay(500);
        if (isWilly) digitalWrite(WILLY_LED, LOW);    
        if (isMonet) digitalWrite(MONET_LED, LOW);    
        //
        Watchdog.sleep(8000);
      }
      else if (alarmPosition <= 30)
      {
        if (isWilly) digitalWrite(WILLY_LED, HIGH);    
        if (isMonet) digitalWrite(MONET_LED, HIGH);    
        //
        playBell();
        //
        delay(500);
        if (isWilly) digitalWrite(WILLY_LED, LOW);    
        if (isMonet) digitalWrite(MONET_LED, LOW);    
        //
        Watchdog.sleep(4000);
      }
      else if (alarmPosition <= 40)
      {
        if (isWilly) digitalWrite(WILLY_LED, HIGH);    
        if (isMonet) digitalWrite(MONET_LED, HIGH);    
        //
        playBell();
        //
        delay(250);
        if (isWilly) digitalWrite(WILLY_LED, LOW);    
        if (isMonet) digitalWrite(MONET_LED, LOW);    
        //
        Watchdog.sleep(2000);
      }
      else if (alarmPosition <= 50)
      {
        if (isWilly) digitalWrite(WILLY_LED, HIGH);    
        if (isMonet) digitalWrite(MONET_LED, HIGH);    
        //
        playBell();
        //
        delay(250);
        if (isWilly) digitalWrite(WILLY_LED, LOW);    
        if (isMonet) digitalWrite(MONET_LED, LOW);    
        //
        Watchdog.sleep(1000);
      }
      else
      {
        if (isWilly) digitalWrite(WILLY_LED, HIGH);    
        if (isMonet) digitalWrite(MONET_LED, HIGH);    
        //
        playBell();
        delay(250);
        digitalWrite(BUZZER, HIGH);
        delay(500);
        digitalWrite(BUZZER, LOW);
        //
        if (isWilly) digitalWrite(WILLY_LED, LOW);    
        if (isMonet) digitalWrite(MONET_LED, LOW);    
        //
        Watchdog.sleep(1000);       
      }
      //
      alarmPosition++;
      if (alarmPosition > 70) 
      {
        alarmPosition = 0;
        alarmMutedTime = 224;
      }
    }
  }
  else
  {
    digitalWrite(WILLY_LED, LOW);    
    digitalWrite(MONET_LED, LOW);    
    //    
    Watchdog.sleep(8000);
    //
    secondsCounter++;
    if (secondsCounter >= 10)
    {
      secondsCounter = 0;
      //
      digitalWrite(RTCON, HIGH);
      delay(100);
      //
      if (RTC.read(tm))
      {
        digitalWrite(RTCON, LOW);
        //
        // Check Willy //
        if (willyPos == LOW && tm.Hour >= 10 && tm.Hour < 15) startAlarm();
        if (willyPos == HIGH && tm.Hour >= 22 && tm.Minute >= 30) startAlarm();
        //
        // Check Monet //
        if (monetPos == LOW && tm.Hour >= 10 && tm.Hour < 15) startAlarm();
        if (monetPos == HIGH && tm.Hour >= 16 && tm.Hour < 22) startAlarm();
        //
        if (!doAlarm && prevHour != tm.Hour && tm.Hour >= 9 && tm.Hour <= 23)
        {
          prevHour = tm.Hour;
          playBellHour();
        }
      } 
      else 
      {
        secondsCounter = 99;
      }
      //
      digitalWrite(RTCON, LOW);
    }
  }
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void startAlarm()
{
  doAlarm = true;
  alarmPosition = 0;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void interruptFunction()
{
  secondsCounter = 99;
  doAlarm = false;
  alarmPosition = 0;
  isWilly = false;
  isMonet = false;
  alarmMutedTime = 0;
  doQuickBuzz = true;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void playBell()
{
  digitalWrite(BELL, HIGH);
  delay(8);
  digitalWrite(BELL, LOW);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void playBellHour()
{
  digitalWrite(BELL, HIGH);
  delay(8);
  digitalWrite(BELL, LOW);
  delay(662);
  digitalWrite(BELL, HIGH);
  delay(8);
  digitalWrite(BELL, LOW);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void quickBuzz()
{
  digitalWrite(WILLY_LED, HIGH);    
  digitalWrite(MONET_LED, HIGH);    
  //
  for (char ww = 0; ww < 10; ww++)
  {
    digitalWrite(BUZZER, HIGH);
    delay(1);
    digitalWrite(BUZZER, LOW);
    delay(22);
  }
  //
  digitalWrite(WILLY_LED, LOW);    
  digitalWrite(MONET_LED, LOW);     
}
