#include <DHT.h>
#include <LiquidCrystal.h>
#include <ESP8266WiFi.h>
#include "secret.h"
#define dht_pin D0
#define mq_pin A0
#include "ThingSpeak.h"

char ssid[] = SECRET_SSID; 
char pass[] = SECRET_PASS; 

WiFiClient  client;

const char * myWriteAPIKey = SECRET_WRITE_APIKEY;
unsigned long myChannelNumber = SECRET_CH_ID_COUNTER;

const int RS = D2, EN = D3, d4 = D5, d5 = D6, d6 = D7, d7 = D8;   
LiquidCrystal lcd(RS, EN, d4, d5, d6, d7);

DHT dht;

void setup()
{
  Serial.begin(115200); // Initialize serial
  while (!Serial) {
    ;
  }
  WiFi.mode(WIFI_STA);   
  ThingSpeak.begin(client); // Initialize ThingSpeak
  dht.setup(dht_pin); // Initialize DHT11
  lcd.begin(16, 2); // Initialize LCD

  Serial.println();
  Serial.println("Status\tHumidity (%)\tTemperature (C)\t(F) \tAir Quality(PPM)");

  //prints welcome msg
  lcd.setCursor(4, 0);
  lcd.print("Welcome!");
  delay(2000);
  lcd.clear();
  WiFi.begin(ssid, pass);

    // Connect or reconnect to WiFi
  while(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    Serial.print(".");
      lcd.print("Connecting...");
      delay(5000);
  }
    Serial.println("\nConnected.");
    lcd.clear();
    lcd.print("Connected!");
    delay(1000);
    lcd.clear();
  
  
}

void loop()
{
  delay(dht.getMinimumSamplingPeriod()); /* Delay of amount equal to sampling period */
  float humidity = dht.getHumidity();/* Get humidity value */
  float temperature = dht.getTemperature();/* Get temperature value */
  int air_quality = analogRead(mq_pin); /* Get air quality value */

  // print values on the serial monitor
  Serial.print(dht.getStatusString());/* Print status of communication */
  Serial.print("\t");   
  Serial.print(humidity, 1);
  Serial.print("\t\t");
  Serial.print(temperature, 1);
  Serial.print("\t\t");
  Serial.print(dht.toFahrenheit(temperature), 1);/* Convert temperature to Fahrenheit units */
  Serial.print("\t\t");
  Serial.print(air_quality, 1);
  Serial.print("\t");

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Temperature");
  lcd.setCursor(0,1);
  lcd.print(temperature);
  delay(3000);
  lcd.clear();
  
  lcd.setCursor(0,0);
  lcd.print("Humidity");
  lcd.setCursor(0,1);
  lcd.print(humidity);
  delay(3000);
  lcd.clear();  

  lcd.setCursor(0,0);
  lcd.print("Air Quality");
  lcd.setCursor(0,1);
  lcd.print(air_quality);
  
  // set the fields with the values
  ThingSpeak.setField(1, temperature);  
  ThingSpeak.setField(2, humidity);
  ThingSpeak.setField(3, air_quality);

  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(x == 200){
    Serial.print("Channel update successful.");
    }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
    }
    
  delay(10000); // Wait 10 seconds to update the channel again
  Serial.println(" ");
}
