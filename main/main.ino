#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <millisDelay.h>
#include <DHT.h>
#include <LiquidCrystal.h>
#include <ESP8266WiFi.h>
#include "secret.h"
#define dht_pin D0
#define mq_pin A0
#include "ThingSpeak.h"

char ssid[] = SECRET_SSID; 
char pass[] = SECRET_PASS; 

TinyGPSPlus gps;  // The TinyGPS++ object
SoftwareSerial ss(5, 2); // The serial connection to the GPS device
WiFiClient  client;

float latitude , longitude;
int year , month , date, hour , minute , second;
String date_str , time_str , lat_str , lng_str;
int pm;
WiFiServer server(80);

const char * myWriteAPIKey = SECRET_WRITE_APIKEY;
unsigned long myChannelNumber = SECRET_CH_ID_COUNTER;

const int RS = D2, EN = D3, d4 = D5, d5 = D6, d6 = D7, d7 = D8;   
LiquidCrystal lcd(RS, EN, d4, d5, d6, d7);

DHT dht;

//millis delay
millisDelay sync_delay;

void setup()
{
  Serial.begin(115200); // Initialize serial
  ss.begin(9600); // Initialize soft-serial for GPS
  while (!Serial) {
    ;
  }
  WiFi.mode(WIFI_STA);   
  ThingSpeak.begin(client); // Initialize ThingSpeak
  dht.setup(dht_pin); // Initialize DHT11
  lcd.begin(16, 2); // Initialize LCD

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
    server.begin(); // starting server
    Serial.println("Server started");
    // Print the IP address of the server
    Serial.println(WiFi.localIP());
    Serial.println();
    Serial.println("Status\tHumidity (%)\tTemperature (C)\t(F) \tAir Quality(PPM)");

    sync_delay.start(5000);
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

if (sync_delay.justFinished())
{
  sync_delay.repeat();
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
    Serial.println(" ");
}
  //delay(10000); // Wait 10 seconds to update the channel again

  if (ss.available() > 0){

    if (gps.encode(ss.read()))
    {
      if (gps.location.isValid())
      {
        latitude = gps.location.lat();
        lat_str = String(latitude, 6);
        
        longitude = gps.location.lng();
        lng_str = String(longitude , 6);
      }

      if (gps.date.isValid())
      {
        date_str = "";
        date = gps.date.day();
        month = gps.date.month();
        year = gps.date.year();

        if (date < 10)
         date_str = '0';
        date_str += String(date);
        date_str += " / ";
        
        if (month < 10)
          date_str += '0';
        date_str += String(month);
        date_str += " / ";

        if (year < 10)
          date_str += '0';
        date_str += String(year);
      }

      if (gps.time.isValid())
      {
        time_str = "";
        hour = gps.time.hour();
        minute = gps.time.minute();
        second = gps.time.second();
        minute = (minute + 30);
        
        if (minute > 59)
        {
          minute = minute - 60;
          hour = hour + 1;
        }

        hour = (hour + 5);
        if (hour > 23)
          hour = hour - 24;

        if (hour >= 12)
          pm = 1;
        else
          pm = 0;

        hour = hour % 12;
        if (hour < 10)
        time_str = '0';
        time_str += String(hour);
        time_str += " : ";

        if (minute < 10)
          time_str += '0';

        time_str += String(minute);
        time_str += " : ";

        if (second < 10)
          time_str += '0';

        time_str += String(second);

        if (pm == 1)
          time_str += " PM ";
        else
          time_str += " AM ";
      }
    }
    
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client)
  {
    return;
  }

  // Prepare the response
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n <!DOCTYPE html> <html> <head> <title>GPS Interfacing with NodeMCU</title> <style>";
  s += "a:link {background-color: YELLOW;text-decoration: none;}";
  s += "table, th, td {border: 1px solid black;} </style> </head> <body> <h1  style=";
  s += "font-size:300%;";
  s += " ALIGN=CENTER> GPS Interfacing with NodeMCU</h1>";
  s += "<p ALIGN=CENTER style=""font-size:150%;""";
  s += "> <b>Location Details</b></p> <table ALIGN=CENTER style=";
  s += "width:50%";
  s += "> <tr> <th>Latitude</th>";
  s += "<td ALIGN=CENTER >";
  s += lat_str;
  s += "</td> </tr> <tr> <th>Longitude</th> <td ALIGN=CENTER >";
  s += lng_str;
  s += "</td> </tr> <tr>  <th>Date</th> <td ALIGN=CENTER >";
  s += date_str;
  s += "</td></tr> <tr> <th>Time</th> <td ALIGN=CENTER >";
  s += time_str;
  s += "</td>  </tr> </table> ";

  if (gps.location.isValid())
  {
    s += "<p align=center><a style=""color:RED;font-size:125%;"" href=""http://maps.google.com/maps?&z=15&mrt=yp&t=k&q=";
    s += lat_str;
    s += "+";
    s += lng_str;
    s += """ target=""_top"">Click here!</a> To check the location in Google maps.</p>";
  }

  s += "</body> </html> \n";
  client.print(s);
  //delay(100);
  }
  
}
