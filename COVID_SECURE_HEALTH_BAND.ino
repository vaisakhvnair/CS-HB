#include <Wire.h>
#include <MAX30100_PulseOximeter.h>
#define BLYNK_PRINT Serial
#include <Blynk.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>;
#include <ThingSpeak.h>;
#include <BlynkSimpleEsp8266.h>
#define REPORTING_PERIOD_MS     1000

const char* auth = "f8PclE_A5iTtZ9PnYPYu12TpAB2GaHG1";             
const char* ssid = "XXXXXXX";                                   
const char* password = "XXXXXXX";


PulseOximeter pox;

uint8_t tempPin = A0;   
uint16_t val;
uint32_t tsLastReport = 0;

float R1 = 10000;
float logR2, R2, T , TC;
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;

WiFiClient client;

unsigned long myChannelNumber = 11111111; //Your Channel Number (Without Brackets)

const char * myWriteAPIKey = "TTTTTTTTTTTT"; //Your Write API Key


// Callback (registered below) fired when a pulse is detected
void onBeatDetected()
{
  Serial.println("Beat!");
}

void setup()
{
  pinMode(16, OUTPUT);
  Serial.begin(115200);
  Blynk.begin(auth, ssid, password);
  Serial.print("Initializing pulse oximeter..");

  WiFi.begin(ssid, password);
  ThingSpeak.begin(client);


 
  if (!pox.begin())
  {
    Serial.println("FAILED");
    for (;;);
  } else {
    Serial.println("SUCCESS");
  }
  // The default current for the IR LED is 50mA and it could be changed by uncommenting the following line.
  pox.setIRLedCurrent(MAX30100_LED_CURR_24MA);
  
  pox.setOnBeatDetectedCallback(onBeatDetected);
}

void loop()
{
  analogRead(tempPin);
  val = analogRead(tempPin);
  R2 = R1 * (1023.0 / (float)val - 1.0);
  logR2 = log(R2);
  T = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));
  T = T - 273.15;
  T = (T * 9.0)/ 5.0 + 32.0; 
  TC = (T - 32) * 5/9 ;


  // Make sure to call update as fast as possible
  pox.update();
  Blynk.run();

 
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    Serial.print("Heart rate:");
    Serial.print(pox.getHeartRate());
    Serial.print("bpm / SpO2:");
    Serial.print(pox.getSpO2());
    Serial.println("%");

    Blynk.virtualWrite(V7, pox.getHeartRate());
    Blynk.virtualWrite(V8, pox.getSpO2());
    
    Serial.print("TEMPRATURE = ");
    Serial.print(T);
    Serial.print("*F");
    Serial.println();
   // Serial.println(TC);

    Blynk.virtualWrite(V9, T);
    ThingSpeak.writeField(myChannelNumber, 1,T, myWriteAPIKey);
    ThingSpeak.writeField(myChannelNumber, 2,pox.getSpO2(), myWriteAPIKey);
    ThingSpeak.writeField(myChannelNumber, 3,pox.getHeartRate(), myWriteAPIKey);
   
    tsLastReport = millis();
  }
}
