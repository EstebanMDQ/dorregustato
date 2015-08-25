// Date and time functions using a DS1307 RTC connected via I2C and Wire lib

#include <Wire.h>
#include "RTClib.h"

RTC_DS1307 rtc;

// temperaturas de dia y noche
const float on_temp_min = 19.0;  // temp dia
const float on_temp_max = 20.0;  // temp dia
const float off_temp_min = 17.0;   // temp noche
const float off_temp_max = 19.0;   // temp noche

// configuracion de pines
const int tempPin = A5;    // the analog pin used to read temp
const int heaterPin =  7;      // caldera on/off
const int statusheaterPin =  8;      // status on/off
const int overrideHeaterPin =  9;      // override led on/off
const int overridePin = 10; // override switch
const int overrideWorkModePin =  6;      // override work mode on/off

// configuracion horarios
int onHours[] = {6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20}; // horas en las que queda prendido
//int onHours[] = {9, 10, 11, 12, 16, 17, 18}; // horas en las que queda prendido
int onDow[] = {1, 2, 3, 4, 5}; // 0 dom 6 sab , esta de lunes a viernes

// intervalo de checkeo
const unsigned long overrideInterval = 600000; // 10 minutes system override
const unsigned long intervalCheck = 60000; // 1 minute
const unsigned long workModeOverrideInterval = 10800000; // 3 horas

unsigned long interval; 
unsigned long previousMillis; // usado para checkear el loop cada 5 minutos
unsigned long currentMillis;

unsigned long workModeOverriden;
int workModeOverrideStatus = 0;
int heaterOverrideStatus = 0;
int heater_status;

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
  pinMode(overrideWorkModePin, INPUT); // switch work mode override
  digitalWrite(heaterPin, LOW);  
  digitalWrite(statusheaterPin, LOW); 
 
  workModeOverriden = 0; 
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
  Serial.print('  ');
//  Serial.println();
  
//  Serial.print(" since midnight 1/1/1970 = ");
//  Serial.print(d.unixtime());
//  Serial.print("s = ");
//  Serial.print(d.unixtime() / 86400L);
//  Serial.println("d");
  
}

void serial_send_status(DateTime d, float temp, float target_temp_min, float target_temp_max, int override, int heater_status) {
  print_date(d);
  
  Serial.print(" Temp Trabajo: ");
  Serial.print(target_temp_min);
  Serial.print(" / ");
  Serial.print(target_temp_max);

  Serial.print(" Temp Actual: ");
  Serial.print(temp);
  
  Serial.print(" Estado: ");
  Serial.print(heater_status);
  Serial.print(" Override: ");
  Serial.print(override);
  
  Serial.println("");
}

boolean check_hour(DateTime t) {
  if( currentMillis < workModeOverriden ) {
     return true; 
  }
  
  int h = t.hour();
  int dow = t.dayOfWeek();
  boolean r = false;
  for( int j=0; j<sizeof(onDow); j++) {
    if( dow == onDow[j] ) {
      for( int i=0; i<sizeof(onHours); i++) {
        if(h == onHours[i]) {
          return true;
        }
      }
    }
  }
  return false;  
}

void loop () {

  currentMillis = millis();
  DateTime d = rtc.now();
  float temp;
  float target_temp_min;
  float target_temp_max;

// desactive el switch de override 
//  if( heaterOverrideStatus == 0 && digitalRead(overridePin) == HIGH ){
//    heaterOverrideStatus = 1;
//    interval = currentMillis - previousMillis + overrideInterval;
//    digitalWrite(heaterPin, LOW);  // caldera on
//    digitalWrite(overrideHeaterPin, HIGH);  // led override on
//    temp = measure_temp(tempPin);
//    serial_send_status(d, temp, 0, 0, 1, 1);
//  } 
  
  if( workModeOverrideStatus == 0 && digitalRead(overrideWorkModePin) == HIGH ) {
    workModeOverrideStatus = 1;
    workModeOverriden = currentMillis + workModeOverrideInterval;
    Serial.println("work mode overriden");
  }
  if( currentMillis > workModeOverrideInterval && workModeOverrideStatus == 1) {
    workModeOverrideStatus = 0;
    workModeOverriden = 0;
    Serial.println("work mode override finished");
  }
  
  if(currentMillis - previousMillis >= interval) {
    // set the override heater check status to 0
    heaterOverrideStatus = 0;
    
    // save the last time you blinked the LED 
    previousMillis = currentMillis;   
    digitalWrite(overrideHeaterPin, LOW);  // led override off

    // vuelvo a establecer interval al valor por defecto
    // en caso de que lo hallamos cambiado con el override
    interval = intervalCheck; 

    
//    print_date(d);    
    
    // choose the target temp
    if( check_hour(d) ){
      digitalWrite(statusheaterPin, HIGH);  // horario de trabajo
      target_temp_min = on_temp_min;
      target_temp_max = on_temp_max;      
    } else {
      digitalWrite(statusheaterPin, LOW);  // fuera de horario
      target_temp_min = off_temp_min;
      target_temp_max = off_temp_max;
    }
    temp = measure_temp(tempPin);
  
    if( temp < target_temp_min ) {
      digitalWrite(heaterPin, LOW);  // caldera on
      heater_status = 1;
    }  
    if( temp > target_temp_max ) {
      digitalWrite(heaterPin, HIGH);  // caldera off
      heater_status = 0;
    }
//    Serial.print("temperature = ");
//    Serial.print(temp);
//    Serial.print("*C");
//    Serial.println();

    serial_send_status(d, temp, target_temp_min, target_temp_max, 0, heater_status);
  }
}
