//Includes für Onewire
#include <OneWire.h>
#include <DallasTemperature.h>

//Includes für ESP8266 WLAN und Webserver
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

//Einstellungen für Onewire Sensor - GPIO5
#define ONE_WIRE_BUS 5 // PIN 6 für Sensor
OneWire ourWire(ONE_WIRE_BUS); /* Ini oneWire instance */
DallasTemperature sensors(&ourWire);/* Dallas Temperature Library für Nutzung der oneWire Library vorbereiten */
float temperature = 0;

//WLAN und Webserver Einstellungen
const char* ssid = "wifissid";
const char* password = "wifipassword";
MDNSResponder mdns;
ESP8266WebServer server(80);

//Thingspeak Einstellungen
int thingspeakport = 80;
const char* thingspeakaddress = "api.thingspeak.com";
const char* thingspeakurl = "/update";
const char* thingspeakapi = "7MSP70OE7LWRGZZR";

//Das tun wenn Webserver angesprochen wird
void website() {
  server.send(200, "text/plain", "hello from esp8266!");

  //Zeige Temperatur auf Serial wenn Webseite geöffnet wird
  Serial.println();  
  sensors.requestTemperatures(); // Temperaturen abfragen
  float temperature = sensors.getTempCByIndex(0);  
  Serial.print(temperature);
}


//Starte ESP8266
void setup()
{
  
  Serial.begin(115200);
  
  //Sensoren starten und Dallas Library initialisieren
  Serial.println("Starte Sensoren");
  delay(1000);
  sensors.begin();
  delay(2000);
  Serial.println("----Sensoren gestartet");
  Serial.println("");

  //WLAN verbinden
  Serial.println("Starte WLAN");
  WiFi.begin(ssid, password);
  Serial.println("Warte auf Verbindung");
    while (WiFi.status() != WL_CONNECTED) {
    delay(10000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Verbunden mit ");
  Serial.println(ssid);
  Serial.print("IP-Adresse: ");
  Serial.println(WiFi.localIP());

  
  if (mdns.begin("esp8266", WiFi.localIP())) {
    Serial.println("MDNS responder started");
  }
  
  //HTTP Server starten und Funktion website aufrufen wenn Seite aufgerufen wird
  server.on("/", website);
  server.begin();
  Serial.println("HTTP Server gestartet");
  
 }

void loop()
{
  mdns.update();
  server.handleClient();
  updateThingspeak();
}

void updateThingspeak()
{
  WiFiClient client;

  sensors.requestTemperatures(); // Temperaturen abfragen
  float temperature = sensors.getTempCByIndex(0);  
  Serial.print(temperature);
  
  //Zu ThingSpeak verbinden
  if (!client.connect(thingspeakaddress, thingspeakport)) {
    Serial.println("connection failed");
    return;
  }

  //Thingspeak URL bauen
  //   /update?key=7MSP70OE7LWRGZZR&field1=34
  String url = thingspeakurl;
  url += "?key=";
  url += thingspeakapi;
  url += "&field1=";
  url += temperature;

  Serial.print("Requesting URL: ");
  Serial.println(url);
  delay(100);
  //Request senden
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
              "Host: " + thingspeakaddress + "\r\n" + 
              "Connection: close\r\n\r\n");
  delay(10);

  //Antwort ausgeben
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  
  Serial.println();
  Serial.println("closing connection");

  delay(15000);
}


