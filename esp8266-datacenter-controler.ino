#include "DHT.h"
#include <FirebaseArduino.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <LiquidCrystal_I2C.h>


#define DHTPIN 0
#define DHTTYPE DHT22 
  
#define redled 12
#define blueled 2
#define greenled 3

#define gas A0

#define flame 16
#define buzzer 10

int timeSinceLastRead = 0;


#define FIREBASE_HOST "datacenter-e8f69-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "G0GkF3Uap27dvezyjk8slM8tKpiBrjRoCt9K4Mze"
#define WIFI_SSID "TOPNET_3700"
#define WIFI_PASSWORD "6es8vks04x"
//#define WIFI_SSID "HUAWEI Y6s"
//#define WIFI_PASSWORD "@teemo@@"

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

const float max_t = 27 ;
const float max_h = 60 ;
const int max_gaz = 400 ;

const long utcOffsetInSeconds = 3600;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);


void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(redled,OUTPUT);
  pinMode(blueled,OUTPUT);
  pinMode(greenled,OUTPUT);
  pinMode(gas,INPUT);
  pinMode(flame,INPUT_PULLUP);
  pinMode(buzzer,OUTPUT);

  while(!Serial) { }
  
  dht.begin();

  Serial.println("Device Started");
  Serial.println("-------------------------------------");
  Serial.println("Running DHT!");
  Serial.println("-------------------------------------");

  WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
  Serial.print("connecting");

  
  while (WiFi.status() != WL_CONNECTED)
    {
      Serial.print(".");
      delay(500);
     }
  
  Serial.print(" \n connected to : ");
  Serial.println(WIFI_SSID);
  Serial.print("addressIP:");
  Serial.println(WiFi.localIP());




 Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  
    if(Firebase.failed()) {
       Serial.print("Firebase connexion failed");
       Serial.println(Firebase.error());
       }
       
    else{
       Serial.println("Firebase Connected");
       }
       
timeClient.begin();
delay(200);   
  
  lcd.init();                
  lcd.backlight();

}

void loop() {

  // Report every 2 seconds.
  if(timeSinceLastRead > 2000) {
  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
   String formattedDate = timeClient.getFormattedDate();
  Serial.println (formattedDate) ; 
  
    float h = dht.readHumidity();
  
    float t = dht.readTemperature();

    float f = dht.readTemperature(true);

    float g  = analogRead(gas);

    if(digitalRead(flame) == HIGH){
       digitalWrite(buzzer,HIGH);
       Serial.println("on fire");
       delay(200);
       Firebase.setBool("RealtimeData/flame",true);
    }
    else{
        digitalWrite(buzzer,LOW);
        Serial.println("safe");
        delay(200);
       Firebase.setBool("RealtimeData/flame",false);
      }

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t) || isnan(f)) {
      Serial.println("Failed to read from DHT sensor!");
      timeSinceLastRead = 0;
      return;
       }
       
    if (isnan(g)) {
      Serial.println("Failed to read from MQ-9 sensor!");
      timeSinceLastRead = 0;
      return;
    }

  

    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.println(" %");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.print(" *C \t");
    Serial.print(f);
    Serial.println(" *F");
    Serial.print("Gaz: ");
    Serial.print(g);
    Serial.println(" ppm ");


    lcd.setCursor(0,0);
    lcd.print("Temperature:"+ String(t));
    lcd.setCursor(0,1);
    lcd.print("Humidity:"+String(h));
    delay(2000);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Gas:"+String(g));
    lcd.setCursor(0,1);
    if (Firebase.getBool("RealtimeData/flame")== true){
    lcd.print("on fire");}
    else {lcd.print("no fire");}
    delay(1000);


    Firebase.setFloat("RealtimeData/Humidity",h);
            if (Firebase.failed()) {
                Serial.print("setting Humidity failed :");
                Serial.println(Firebase.error()); 
                delay(200);
               }

      
   Firebase.setFloat("RealtimeData/Temperature",t);
            if (Firebase.failed()) {
                Serial.print("setting Temperature failed :");
                Serial.println(Firebase.error()); 
                delay(200);
                  }
      
   Firebase.setFloat("RealtimeData/Gas",g);
            if (Firebase.failed()) {
                Serial.print("setting Gas failed :");
                Serial.println(Firebase.error()); 
                delay(200);
                  }

    Firebase.setString("RealtimeData/Time",formattedDate);
            if (Firebase.failed()) {
                Serial.print("setting time failed :");
                Serial.println(Firebase.error()); 
                delay(200);
                  }
   
   if ( t >= max_t){
        digitalWrite(redled,HIGH);
        delay(200);
        digitalWrite(redled,LOW);
        delay(200);
        Serial.println("!!!!! Temperature so HIGH !!!!! ");

        Firebase.setBool("RealtimeData/tempStatue",true);
        
           if (Firebase.failed()) {
              Serial.print("setting tempStatue failed :");
              Serial.println(Firebase.error()); 
              delay(200);
              }
       }
    else { Firebase.setBool("RealtimeData/tempStatue",false);}

       if ( h >= max_h){
          digitalWrite(blueled,HIGH);
          delay(200);
          digitalWrite(blueled,LOW);
          delay(200);
          Serial.println("!!!!! humidity so HIGH !!!!!");


          Firebase.setBool("RealtimeData/humdStatue",true);
              if (Firebase.failed()) {
                  Serial.print("setting humdStatue failed :");
                  Serial.println(Firebase.error()); 
                  delay(200);
                  }
        
      }
      else {Firebase.setBool("RealtimeData/humdStatue",false);}
      

      if ( g >= max_gaz){
          digitalWrite(greenled,HIGH);
          delay(200);
          digitalWrite(greenled,LOW);
          delay(200);
          Serial.println("!!!!! Gas do HIGH !!!!!");


          Firebase.setBool("RealtimeData/gasStatue",true);
                 if (Firebase.failed()) {
                     Serial.print("setting gasStatue failed :");
                     Serial.println(Firebase.error()); 
                     delay(200);
                  }
                  
      }
      else {Firebase.setBool("RealtimeData/gasStatue",false);}

    Firebase.setFloat("Database/" + formattedDate + "/_Humidity",h);
      if (Firebase.failed()) {
          Serial.print("setting Humidity failed :");
          Serial.println(Firebase.error()); 
          delay(200);
      }
      Firebase.setFloat("Database/" + formattedDate + "/_Temperature",t); 
      if (Firebase.failed()) {
          Serial.print("setting Temperature failed :");
          Serial.println(Firebase.error()); 
          delay(200);
      }

 Firebase.setFloat("Database/" + formattedDate + "/_Gas",g); 
      if (Firebase.failed()) {
          Serial.print("setting Gas failed :");
          Serial.println(Firebase.error()); 
          delay(200);
      }

bool tempStatue = Firebase.getBool("RealtimeData/tempStatue");
bool humdStatue = Firebase.getBool("RealtimeData/humdStatue");
bool gasStatue = Firebase.getBool("RealtimeData/gasStatue");


Firebase.setBool("Database/" + formattedDate + "/_tempStatue",tempStatue); 
      if (Firebase.failed()) {
          Serial.print("setting tempStatue failed :");
          Serial.println(Firebase.error()); 
          delay(200);
      }

      Firebase.setBool("Database/" + formattedDate + "/_humdStatue",humdStatue); 
      if (Firebase.failed()) {
          Serial.print("setting humdStatue failed :");
          Serial.println(Firebase.error()); 
          delay(200);
      }
      Firebase.setBool("Database/" + formattedDate + "/_gasStatue",gasStatue); 
      if (Firebase.failed()) {
          Serial.print("setting gasStatue failed :");
          Serial.println(Firebase.error()); 
          delay(200);
      }
      
       timeSinceLastRead = 0;
  }
  
  delay(100);
  timeSinceLastRead += 100;

}
