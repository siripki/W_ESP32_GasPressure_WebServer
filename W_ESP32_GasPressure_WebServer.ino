// //SPIFFS
#include "SPIFFS.h"
// #include "FS.h"

//WebServer and WiFi Parameter
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>  //if not use ethernet
#include <AsyncElegantOTA.h>
AsyncWebServer server(80);           //GUI server
AsyncEventSource events("/events");  // Create an SSE Event Source on /events

#include <Wire.h>
#include <LiquidCrystal_I2C.h>  //library lcd
// alamat lcd 0x27
LiquidCrystal_I2C lcd(0x27, 16, 2);

#include "adcLookupTable.h"
#define adcPin 35
#define buzzer 15  //pin buzzer

float gasPressure = 0.0;

void setup() {
  Serial.begin(115200);
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, HIGH);

  spiffSetup();

  WiFi.mode(WIFI_AP);
  WiFi.softAP("Gas Presure Meter", "12345678");
  serverSetup();

  xTaskCreatePinnedToCore(
    loop2,   /* Task function. */
    "loop2", /* name of task. */
    20000,   /* Stack size of task */
    NULL,    /* parameter of the task */
    1,       /* priority of the task */
    NULL,    /* Task handle to keep track of created task */
    0);      /* pin task to core 0 */
}

void loop() {  //core 1 handle webserver
}

void loop2(void* pvParameters) {  //core 0 handle pengukuran, lcd, dan buzzer
  lcdSetup();
  while (1) {
    static unsigned long readingTimer = 0;
    if (millis() - readingTimer > 1000) { //ambil data tiap 1 detik
      int adc = adcReading(adcPin);
      Serial.println("ADC : " + String(adc));
      // gasPressure = mapPecahan(adc, 322, 1867, 0, 500);
      gasPressure = getRegress(adc);
      Serial.println("Gas Pressure : " + String(gasPressure) + " kPa");
      serverSentEvents();
      tampilHasil(gasPressure);
      readingTimer = millis();
    }

    static unsigned long buzzTimer = 0;
    if (millis() - buzzTimer > 2000) { //buzzer nyala tiap 2 detik jika susuai ketentuan
      if (gasPressure < 400.0 || gasPressure > 500.0) beep();
      buzzTimer = millis();
    }
    // vTaskDelay(1000);
  }
}

float getRegress(int& adc) {
  float result;
  result = 0.3293 * adc - 106.11;  // y = 0,3293x - 106,11
  if (result < 1.0) result = 0;
  return result;
}

// float mapPecahan(float value, float fromLow, float fromHigh, float toLow, float toHigh)
// {
//   return (value - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow;
// }

// float adcReading(uint8_t _pin) {
//   int _rawADC = 0;
//   int _calibratedADC = 0;
//   float _adcValues[10];

//   for (int i = 0; i < 10; i++) {
//     _rawADC = analogRead(_pin);              // read value from ADC
//     _calibratedADC = (int)ADC_LUT[_rawADC];  // get the calibrated value from LookupTable
//     _adcValues[i] = (float)_calibratedADC;
//   }

//   // Calculate the average of the 10 readings
//   float _sum = 0;
//   for (int i = 0; i < 10; i++) {
//     _sum += _adcValues[i];
//   }
//   float _average = _sum / 10.0;

//   return _average;
// }

float adcReading(uint8_t _pin) {
  float totalAverage = 0.0;

  for (int j = 0; j < 5; j++) { // Lakukan 5 kali
    int _rawADC = 0;
    int _calibratedADC = 0;
    float _adcValues[10];

    for (int i = 0; i < 10; i++) { // Baca 10 kali
      _rawADC = analogRead(_pin);              // read value from ADC
      _calibratedADC = (int)ADC_LUT[_rawADC];  // get the calibrated value from LookupTable
      _adcValues[i] = (float)_calibratedADC;
    }

    // Calculate the average of the 10 readings
    float _sum = 0;
    for (int i = 0; i < 10; i++) {
      _sum += _adcValues[i];
    }
    float _average = _sum / 10.0;

    totalAverage += _average; // Tambahkan rata-rata ke total
  }

  // Hitung rata-rata dari 5 kali pengambilan data
  float finalAverage = totalAverage / 5.0;

  return finalAverage;
}

void lcdSetup() {
  lcd.init();
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("  GAS PRESSURE  ");
  lcd.setCursor(0, 1);
  lcd.print("      METER     ");
  vTaskDelay(1000);
}

void tampilHasil(float hasil) {  //fungsi untuk display hasil
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("    PRESSURE    ");
  lcd.setCursor(1, 1);
  lcd.print(hasil, 1);
  lcd.setCursor(12, 1);
  lcd.print("kPa");
}

void beep() {  //untuk menyalakan buzzer
  digitalWrite(buzzer, LOW);
  vTaskDelay(100);
  digitalWrite(buzzer, HIGH);
}
