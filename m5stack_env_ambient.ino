#include <M5Stack.h>
#include <Wire.h>
#include "BM1422AGMV.h"
#include <Bme280.h>
#include <WiFi.h>
#include "Ambient.h"

#define AXIS_X          (0)
#define AXIS_Y          (1)
#define AXIS_Z          (2)
#define DATA_DIGIT      (3)
#define TEMPERATURE     (0)
#define HUMIDITY        (1)
#define PRESSUR         (2)
#define DATA_1          (1)
#define DATA_2          (2)
#define DATA_3          (3)
#define DATA_4          (4)
#define DATA_5          (5)
#define DATA_6          (6)

#define TIMEOUT_VALUE   (20)

BM1422AGMV bm1422a(BM1422AGMV_ADDR_LOW);

Bme280TwoWire sensor;
Ambient ambient;
WiFiClient client;

const char* ssid = "<<YOUR WIFI SSID>>";
const char* password = "<<YOUR WIFI PASSWORD>>";
unsigned int channelId = <<YOUR ID for ambient server>>;
const char* writeKey = "<<YOUR PASSWORD for ambient server>>";

int8_t error_status = 0;

void setup() {
    int32_t result;
    int32_t timeout_count = 0;

    //Intializa LCD
    M5.begin();
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.printf("Connecting to %s\n", ssid);

    //Initialize Hardware
    Serial.begin(115200);
    Wire.begin();
        
    //Connect WiFi
    Serial.println("Connect WiFi");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED){
       delay(500);
       Serial.print('.');
       M5.Lcd.print('.');
       timeout_count++;
       if(timeout_count > TIMEOUT_VALUE){
        error_status = -1;
        break;
       }
    }

   if(error_status < 0){
      M5.Lcd.println("FAILED TO CONNECT WIFI");
      Serial.print("FAILED TO CONNECT WIFI");    
   }
   else{
      M5.Lcd.println("WIFI OK"); //But we can not see it.
      Serial.print("\r\nWiFi connected\r\nIP address: ");
      Serial.println(WiFi.localIP());

      M5.Lcd.fillScreen(TFT_GREEN);

      result = bm1422a.init();
      if (result != BM1422AGMV_OK) {
        Serial.println("bm1422a error");
        M5.Lcd.println("bm1422a error");
        error_status = -1;
      }
      
    if(error_status < 0){
      M5.Lcd.println("FAILED TO CONNECT TO BM1422AGMV");
      Serial.print("FAILED TO CONNECT TO BM1422AGMV");    
    }
    else{
      sensor.begin(Bme280TwoWireAddress::Primary);
      sensor.setSettings(Bme280Settings::indoor());
      ambient.begin(channelId, writeKey, &client);    
     }
   }
}

void loop() {
    int32_t result;
    float32 mag[BM1422AGMV_DATA_SIZE];
    float32 tph[3];

    if(error_status < 0){
        Serial.println("DO NOTHING");
        M5.Lcd.println("DO NOTHING");      
    }
    else{
      // For Single Measurement
      (void)bm1422a.start();
      delay(BM1422AGMV_MEAS_TIME);
      result = bm1422a.get_val(mag);
      if (result == BM1422AGMV_OK) {
        Serial.print("X : Y : Z [uT] = ");
        Serial.print(mag[AXIS_X], DATA_DIGIT);
        Serial.print(",");
        Serial.print(mag[AXIS_Y], DATA_DIGIT);
        Serial.print(",");
        Serial.print(mag[AXIS_Z], DATA_DIGIT);
        Serial.println();
        M5.Lcd.println( "X:" + String(mag[AXIS_X]));
        M5.Lcd.println( "Y:" + String(mag[AXIS_Y]));
        M5.Lcd.println( "Z:" + String(mag[AXIS_Z]));
      } else {
        Serial.println("bm1422a error");
        M5.Lcd.println("bm1422a error");
        error_status = -1;
      }

     
     if(error_status < 0){
        Serial.println("DO NOTHING");
        M5.Lcd.println("DO NOTHING");            
     }
     else{
       tph[TEMPERATURE] = sensor.getTemperature();
       tph[HUMIDITY] = sensor.getHumidity();
       tph[PRESSUR] = (sensor.getPressure() / 100.0);
    
       String temperature = String(tph[TEMPERATURE]) + " °C";
       String humidity = String(tph[HUMIDITY]) + " %";
       String pressure = String(tph[PRESSUR]) + " hPa";

       String measurements = temperature + ", " + pressure + ", " + humidity;
       Serial.println(measurements);
       M5.Lcd.println("TEMP:" + temperature);
       M5.Lcd.println("HUMI:" + humidity);
       M5.Lcd.println("PRES:" + pressure);

       ambient.set(DATA_1, tph[TEMPERATURE]);
       ambient.set(DATA_2, tph[HUMIDITY]);
       ambient.set(DATA_3, tph[PRESSUR]);
       ambient.set(DATA_4, mag[AXIS_X]);
       ambient.set(DATA_5, mag[AXIS_Y]);
       ambient.set(DATA_6, mag[AXIS_Z]);
       ambient.send();    
     }
    }
    
    M5.Power.deepSleep(SLEEP_MIN(5));
    return;

}
