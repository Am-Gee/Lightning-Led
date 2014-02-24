#include <Arduino.h>
#include <EEPROM.h>
#include <stdarg.h>
#include <OneWire.h>
#include <Wire.h>

#include "defines.h"
#include "fancontroller.h"
#include "lightcontroller.h"
#include "timer.h"
#include "onewiretempsensor.h"
#include "softpwm.h"
#include "communicator.h"
#include "relaycontroller.h"

/*
  My lightning LED project.

  TODOs:
  - Sicce Pumps ON/OFF with photoresistor
  - water temperature control sump fan
*/
/*
ISR(TIMER2_COMPA_vect)
{  //change the 0 to 1 for timer1 and 2 for timer2
   //interrupt commands here
    _debug_count++;
}
*/

byte addr1[8] = {0x10, 0x24, 0x7C, 0xAB, 0x2, 0x8, 0x0, 0xD9};
byte addr2[8] = {0x10, 0x3E, 0x30, 0xAD, 0x2, 0x8, 0x0, 0x8C};
byte addr3[8] = {0x10, 0x3D, 0x1B, 0xAE, 0x2, 0x8, 0x0, 0xD5};

byte addr20W1[8] = {0x10, 0xA4, 0x1E, 0xAD, 0x2, 0x8, 0x0, 0xA8};

OneWire  ds(32);

SoftPwm pwm;

//RelayController relayController;

CFanController fanController(0, &ds, addr1, addr2, addr3, addr20W1);

Communicator communicator;

CLightController lightController(&ds, &pwm, &communicator);
//COneWireTempSensor sensor(&ds, addr1, 5000);

long lastMillis = 0L;

void setup()
{
/*    TCCR1A = 0;// set entire TCCR2A register to 0
    TCCR1B = 0;// same for TCCR2B
    TCNT1  = 0;//initialize counter value to 0

    // set compare match register for 8khz increments
    //OCR2A = 0;// = (1 6*10^6) / (500 * 1024) - 1 (must be <256)
    // turn on CTC mode
    TCCR1A |= (1 << WGM21);
    // Set CS21 bit for 8 prescaler
    TCCR1B = TCCR2B & 0b11111000 | 0x00;

    TCCR1B |= (1 << CS12) | (1 << CS10);
    //TCCR2B = TCCR2B & 0b11111000 | 0x04;
    // enable timer compare interrupt
    TIMSK2 |= (1 << OCIE2A);
    */

//   EEPROM.write(0, 255);
    Wire.begin();
    Serial.begin(9600);

    pwm.Setup();

    fanController.Setup();

    lightController.Setup();

    communicator.SetLightControllerExecutor(&lightController);

    communicator.SetFanControllerExecutor(&fanController);

    communicator.Setup();

    lastMillis = millis();
}




void loop()
{
    //pwm.Update();

    //long startFrame = micros();

    communicator.Update();

    fanController.Update();

    lightController.Update();

/*
    long frameTime = micros() - startFrame;
*/
/*
    if(millis() - lastMillis > 1000)
    {
        lastMillis = millis();

        char buf[16];
        sprintf(buf, "FPS: %lu", (long)1000000 / frameTime);
        Serial.println(buf);
    }
*/
/*

  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];

  float celsius, fahrenheit;

  if ( !ds.search(addr)) {
    Serial.println("No more addresses.");
    Serial.println();
    ds.reset_search();
    delay(250);
    return;
  }

  Serial.print("ROM =");
  for( i = 0; i < 8; i++) {
    Serial.write(' ');
    Serial.print(addr[i], HEX);
  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return;
  }
  Serial.println();

  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:
      Serial.println("  Chip = DS18S20");  // or old DS1820
      type_s = 1;
      break;
    case 0x28:
      Serial.println("  Chip = DS18B20");
      type_s = 0;
      break;
    case 0x22:
      Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    default:
      Serial.println("Device is not a DS18x20 family device.");
      return;
  }

  ds.reset();
  ds.select(addr);

  ds.write(0x44, 1);        // start conversion, with parasite power on at the end

  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.

  present = ds.reset();
  ds.select(addr);
  ds.write(0xBE);         // Read Scratchpad

  Serial.print("  Data = ");
  Serial.print(present, HEX);
  Serial.print(" ");
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.print(" CRC=");
  Serial.print(OneWire::crc8(data, 8), HEX);
  Serial.println();

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;
  Serial.print("  Temperature = ");
  Serial.print(celsius);
  Serial.print(" Celsius, ");
  Serial.print(fahrenheit);
  Serial.println(" Fahrenheit");
*/
}


