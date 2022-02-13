/* 
  Check the new incoming messages, and print via serialin 115200 baud rate.
  
  by Aaron.Lee from HelTec AutoMation, ChengDu, China
  成都惠利特自动化科技有限公司
  www.heltec.cn
  
  this project also realess in GitHub:
  https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series
*/

#include "heltec.h"
#include <WiFi.h>
#include "secrets.h"
#include "ThingSpeak.h" // always include thingspeak header file after other header files and custom macros

#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  10770/* Time ESP32 will go to sleep (in seconds) */

RTC_DATA_ATTR int bootCount = 0;

#define BAND    433E6  //you can set band here directly,e.g. 868E6,915E6

char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

char myData[7];
int number1;
int number2;
int number3;
int number4;

int LED_OUT = 13;

unsigned long previousMillis = 0;
unsigned long tempoLigado = 60000; 

int dataSorting2(char myData[7]){

    char w = myData[3];
    char xy = myData[4];

    String fz = String(w) + String(xy);
    int f2 = fz.toInt();

    return f2;
  }

int dataSorting4(char myData[7]){
    char x = myData[0];

    String fx = String(x);
    int f4 = fx.toInt();

    return f4;
  }

  int dataSorting3(char myData[7]){
    char y = myData[1];
    char z = myData[2];

    String fy = String(y) + String(z);
    int f3 = fy.toInt();

    return f3;
  }

  int dataSorting1(char myData[7]){
    char xz = myData[5];
    char xw = myData[6];

    String fw = String(xz) + String(xw);
    int f1 = fw.toInt();

    return f1;
  }


void WiFiConnect(){
     if(WiFi.status() != WL_CONNECTED){
      Serial.print("Attempting to connect to SSID: ");
      Serial.println(SECRET_SSID);
      while(WiFi.status() != WL_CONNECTED){
        WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
        Serial.print(".");
        delay(5000);     
        } 
      Serial.println("\nConnected.");
    }
  }

void LoRaReceive(){
  
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    int i = 0;
    while (LoRa.available()) {
      //Serial.println((char)LoRa.read());
      myData[i] = (char)LoRa.read();
      i += 1;
    }

    number1 = dataSorting1(myData);
    //Serial.println(number1);
    number2 = dataSorting2(myData);
    //Serial.println(number2);
    number3 = dataSorting3(myData);
    //Serial.println(number3);
    number4 = dataSorting4(myData);
    //Serial.println(number4);
    
    ThingSpeak.setField(1, number1);
    ThingSpeak.setField(2, number2);
    ThingSpeak.setField(3, number3);
    ThingSpeak.setField(4, number4);  
    
    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    if(x == 200){
      Serial.println("Channel update successful.");
    }
    else{
      Serial.println("Problem updating channel. HTTP error code " + String(x));
    }
  
    digitalWrite(LED_OUT, HIGH);
    delay(1000);
    digitalWrite(LED_OUT, LOW);
  }
  
 }

void setup() {
    //WIFI Kit series V1 not support Vext control
  Heltec.begin(false /*DisplayEnable Enable*/, true /*Heltec.LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);
  
  pinMode(LED_OUT, OUTPUT);
  
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));
  delay(10);

  Serial.println("Acordei!");
  
  WiFi.mode(WIFI_STA);   
  ThingSpeak.begin(client);  // Initialize ThingSpeak

  while(millis() < tempoLigado){
    WiFiConnect();
    LoRaReceive();
  }
   
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  delay(10);
  
  LoRa.end();
  LoRa.sleep();
  delay(100);

  Serial.println("Indo dormir");
  esp_deep_sleep_start();
}

void loop() {
  
}
