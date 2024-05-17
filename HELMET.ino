#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "JITHU4G"
#define WIFI_PASSWORD "jithu2004"

// Insert Firebase project API Key
#define API_KEY "AIzaSyARJLa0PPzXvxiVYIxRlXnkk-8iRr5mios"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://fire-836b7-default-rtdb.firebaseio.com/" 

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int intValue;
float floatValue;
bool signupOK = false;

const int sensorPin = 34;   // Analog pin (ADC) for MQ sensor
const int relayPin = 5;     // Digital pin for relay control (changed to pin 5)
int led = 2;

void setup() {
  pinMode(sensorPin, INPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(led, OUTPUT);  


  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  }
  else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  // Read the sensor value
  int x = analogRead(sensorPin);

  if (x < 2600) {
    digitalWrite(relayPin, HIGH);
    Serial.println("Drunk");
    Serial.println(x);
    delay(500);
  } else {
    digitalWrite(relayPin, LOW);
    Serial.println("NOT-Drunk");
    Serial.println(x);
    delay(500);
  }

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 500 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    if (Firebase.RTDB.getInt(&fbdo, "/helmet_detection/helmet_detected")) {
      if (fbdo.dataType() == "int") {
        intValue = fbdo.intData();
        Serial.println(intValue);

        if (intValue == 1){
          digitalWrite(led, HIGH);
          digitalWrite(relayPin, HIGH);
          Serial.println("HELMET DETECTED");
        }
        else{
          digitalWrite(led, LOW);
          digitalWrite(relayPin, LOW);
          Serial.println("HELMET NOT DETECTED");
        }
      }
    }
    else {
      Serial.println(fbdo.errorReason());
    }
  }
}