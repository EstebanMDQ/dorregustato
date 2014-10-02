// Date and time functions using a DS1307 RTC connected via I2C and Wire lib

#include <Wire.h>
#include "RTClib.h"

RTC_DS1307 rtc;

// temp sensor

const float target_temp = 22.0;
const int tempPin = A5;    // the analog pin used to read temp
const int ledPin =  7;      // the number of the LED pin
const int ledPin2 =  8;      // the number of the LED pin

int onHours[] = {10, 12, 14, 16, 18, 20}; // horas en las que queda prendido
int onDow[] = {1, 2, 3, 4, 5}; // 0 dom 6 sab , esta de lunes a viernes

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
  pinMode(ledPin, OUTPUT);  // caldera
  pinMode(ledPin2, OUTPUT);  // termostato funcionando
  digitalWrite(ledPin, LOW);  
  digitalWrite(ledPin2, LOW);  
}

// read temp from pin p
float measure_temp(int p)
{
  float tempK = (((analogRead(p)/ 1023.0) * 5.0) * 100.0);  
  //Converts Kelvin to Celsius minus 2.5 degrees error
  float tempC = tempK - 273.0;   
//  float tempF = ((tempK - 2.5) * 9 / 5) - 459.67;
  return tempK;
}

// print datetime d to serial console
void print_date(DateTime d) {
  Serial.print(d.year(), DEC);
  Serial.print('/');
  Serial.print(d.month(), DEC);
  Serial.print('/');
  Serial.print(d.day(), DEC);
  Serial.print(' ');
  Serial.print(d.hour(), DEC);
  Serial.print(':');
  Serial.print(d.minute(), DEC);
  Serial.print(':');
  Serial.print(d.second(), DEC);
  Serial.print(' D:');
  Serial.print(d.dayOfWeek(), DEC);
  Serial.println();
  
//  Serial.print(" since midnight 1/1/1970 = ");
//  Serial.print(d.unixtime());
//  Serial.print("s = ");
//  Serial.print(d.unixtime() / 86400L);
//  Serial.println("d");
  
}

boolean check_hour(DateTime t) {
  int h = t.hour();
  int dow = t.dayOfWeek();
  boolean r = false;
  for( int j=0; j<sizeof(onDow); j++) {
    if( dow == onDow[j] ) {
      r = true;
      break;
    }
  }
  if( r ){
    for( int i=0; i<sizeof(onHours); i++) {
      if(h == onHours[i]) {
        return true;
      }
    }
  }
  return false;  
}

void loop () {
  float temp;
  DateTime d = rtc.now();
  
  print_date(d);    
  
  // si los minutos son pares, medimos la temp
  // y prendemos el rele si temp esta debajo de target_temp
  if( check_hour(d) ){
    digitalWrite(ledPin2, HIGH);  // termostato on
    temp = measure_temp(tempPin);

    if( temp > target_temp ) {
      digitalWrite(ledPin, LOW);  // caldera off
    } else {
      digitalWrite(ledPin, HIGH);  // caldera on
    }
    Serial.print("temperature = ");
    Serial.print(temp);
    Serial.print("*C");
    Serial.println();
  } else {
    digitalWrite(ledPin2, LOW);  // termostato off
    digitalWrite(ledPin, LOW);  // caldera off
  }
  
  delay(1000);  // wait 10 seconds
}
