// Date and time functions using a DS1307 RTC connected via I2C and Wire lib

#include <Wire.h>
#include "RTClib.h"

RTC_DS1307 rtc;

// temperaturas de dia y noche
const float on_temp = 22.0;  // temp dia
const float off_temp = 15.0;   // temp noche

// configuracion de pines
const int tempPin = A5;    // the analog pin used to read temp
const int heaterPin =  7;      // caldera on/off
const int statusheaterPin =  8;      // status on/off
const int overrideHeaterPin =  9;      // override led on/off
const int overridePin = 6; // override switch

// configuracion horarios
int onHours[] = {9, 10, 11, 12, 13, 14, 15, 16, 17, 18}; // horas en las que queda prendido
int onDow[] = {1, 2, 3, 4, 5}; // 0 dom 6 sab , esta de lunes a viernes

// intervalo de checkeo
const unsigned long overrideInterval = 600000; // 10 minutes system override
const unsigned long intervalCheck = 300000; // 5 minutes
unsigned long interval; // 5 minutes
unsigned long previousMillis; // usado para checkear el loop cada 5 minutos

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
  pinMode(heaterPin, OUTPUT);  // caldera
  pinMode(statusheaterPin, OUTPUT);  // termostato funcionando
  pinMode(overrideHeaterPin, OUTPUT);  // modo override
  pinMode(overridePin, INPUT); // switch override
  digitalWrite(heaterPin, LOW);  
  digitalWrite(statusheaterPin, LOW);  
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

  unsigned long currentMillis = millis();
 
//  este deber'a ser el mecanismo de override, pero esta mal!!! 
//  if( digitalRead(overridePin) == HIGH ){
//    interval += overrideInterval;
//    digitalWrite(heaterPin, HIGH);  // caldera on
//    digitalWrite(overrideHeaterPin, HIGH);  // led override on
//  } 
 
  if(currentMillis - previousMillis >= interval) {
    // debug code
    Serial.println(currentMillis);
    Serial.println(previousMillis);
    Serial.println(intervalCheck);
    Serial.println("");

    // save the last time you blinked the LED 
    previousMillis = currentMillis;   
    digitalWrite(overrideHeaterPin, LOW);  // led override off

    // vuelvo a establecer interval al valor por defecto
    // en caso de que lo hallamos cambiado con el override
    interval = intervalCheck; 

    float temp;
    float target_temp;
    DateTime d = rtc.now();
    
    print_date(d);    
    
    // choose the target temp
    if( check_hour(d) ){
      digitalWrite(statusheaterPin, HIGH);  // horario de trabajo
      target_temp = on_temp;
    } else {
      digitalWrite(statusheaterPin, LOW);  // fuera de horario
      target_temp = off_temp;
    }
    temp = measure_temp(tempPin);
  
    if( temp > target_temp ) {
      digitalWrite(heaterPin, LOW);  // caldera off
    } else {
      digitalWrite(heaterPin, HIGH);  // caldera on
    }
    Serial.print("temperature = ");
    Serial.print(temp);
    Serial.print("*C");
    Serial.println();
  }
}
