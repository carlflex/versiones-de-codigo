
#include "WiFiEsp.h"

// Emulate Serial1 on pins 6/7 if not present
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
#include "DHT.h"
#include <Wire.h>    // incluye libreria de bus I2C
#include <Adafruit_Sensor.h>  // incluye librerias para sensor BMP280
#include <Adafruit_BMP280.h>
#include <QMC5883LCompass.h>
#define DHTPIN 2
#define DHTTYPE DHT22

QMC5883LCompass compass;

Adafruit_BMP280 bmp;
DHT dht(DHTPIN, DHTTYPE);
SoftwareSerial Serial1(10, 11); // RX, TX
#endif

char ssid[] = "Hardware";            // your network SSID (name)
char pass[] = "escuela2043";        // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

char server[] = "http://klimarios.herokuapp.com";

String Parametro;
float Temperature;
float Humidity;
float PRESION;
int x,y,z,a;      // variables de los 3 ejes
char myArray[3];
String arrayUnido;
float veloc1= 0;// entrada A0
int tiempo=0;
int cnt=0;
float v1=0;
float v2=0;

unsigned long lastConnectionTime = 0;         // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 10000L; // delay between updates, in milliseconds

// Initialize the Ethernet client object
WiFiEspClient client;

void setup()
{
  // initialize serial for debugging
  Serial.begin(9600);
  // initialize serial for ESP module
  Serial1.begin(9600);
  Wire.begin();
  dht.begin();
  compass.init(); 
  compass.setCalibration(-656, 376, -442, 500, -975, 0);
  bmp.begin();        // si falla la comunicacion con el sensor mostrar
  analogReference(INTERNAL);
  // initialize ESP module
  WiFi.init(&Serial1);
  
  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  Serial.println("You're connected to the network");
  
  printWifiStatus();
}

void loop()
{
  Temperature = dht.readTemperature();
  Humidity= dht.readHumidity();
  PRESION = bmp.readPressure()/100;
  
  //brujula_______________________
  compass.read();
  a = compass.getAzimuth();
  compass.getDirection(myArray, a);
  arrayUnido=String(myArray[0])+ String(myArray[1])+String(myArray[2]);
  arrayUnido.trim();
  //____________________________________________
  v1 =(analogRead(0)); // lectura de sensor a0
  veloc1= (v1*0.190);

  Parametro=String(Temperature)+"/"+String(Humidity)+"/"+String(PRESION);
  
  if (veloc1>v2)(v2=veloc1);
  // if there's incoming data from the net connection send it out the serial port
  // this is for debugging purposes only
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }

  // if 10 seconds have passed since your last connection,
  // then connect again and send data
  if (millis() - lastConnectionTime > postingInterval) {
    httpRequest();
  }
}

// this method makes a HTTP connection to the server
void httpRequest()
{
  String peticionHTTP= "GET /urlparam?temp=";
   peticionHTTP=peticionHTTP+Parametro+"&bru"+String(arrayUnido)+" HTTP/1.1";
Serial.println(peticionHTTP);
  Serial.println();
    
  // close any connection before send a new request
  // this will free the socket on the WiFi shield
  client.stop();

  // if there's a successful connection
  if (client.connect(server, 80)) {
    Serial.println("Connecting...");
    
    // send the HTTP PUT request
    client.println(peticionHTTP);
    client.println("Host: klimarios.herokuapp.com");
    client.println("Connection: close");
    client.println();

    // note the time that the connection was made
    lastConnectionTime = millis();
  }
  else {
    // if you couldn't make a connection
    Serial.println("Connection failed");
  }
}


void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
