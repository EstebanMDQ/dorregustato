// Date and time functions using a DS1307 RTC connected via I2C and Wire lib

#include <Wire.h>
#include "RTClib.h"

RTC_DS1307 rtc;

// temp sensor
float temp;
float target_temp = 24.0;
const int tempPin = A5;    // the analog pin used to read temp
const int ledPin =  7;      // the number of the LED pin

/*

LM 35
5v    --|\
A0    --| |
GND   --|/


LED

GND    -------------------|\  (-)
PIN3   ---1k-/\/\/\-------|/  (+)

*/


void setup () {
  Serial.begin(9600);
#ifdef AVR
  Wire.begin();
#else
  Wire1.begin(); // Shield I2C pins connect to alt I2C bus on Arduino Due
#endif
  rtc.begin();

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  // initialize pin 
  pinMode(ledPin, OUTPUT);   
  digitalWrite(ledPin, HIGH);  
  delay(200);
  digitalWrite(ledPin, LOW);  
  delay(200);
  digitalWrite(ledPin, HIGH);  
  delay(200);
  digitalWrite(ledPin, LOW);  
  delay(200);


}

float measure_temp()
{
  float tempK = (((analogRead(tempPin)/ 1023.0) * 5.0) * 100.0);  
  //Converts Kelvin to Celsius minus 2.5 degrees error
  float tempC = tempK - 273.0;   
//  float tempF = ((tempK - 2.5) * 9 / 5) - 459.67;
  return tempK;
}

void loop () {
    DateTime now = rtc.now();
    
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(' ');
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
    
    // calculate a date which is 7 days and 30 seconds into the future
    DateTime future (now.unixtime() + 7 * 86400L + 30);
    
    Serial.print(" now + 7d + 30s: ");
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


    temp = measure_temp();
    if( temp > target_temp ) {
      digitalWrite(ledPin, LOW);
    } else {
      digitalWrite(ledPin, HIGH);
    }
  
    Serial.print("temperature = ");
    Serial.print(temp);
    Serial.print("*C");
    Serial.println();
   
    Serial.println();
    delay(3000);
}
