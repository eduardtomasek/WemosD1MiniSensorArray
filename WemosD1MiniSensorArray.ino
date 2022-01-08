#include "Arduino.h"

/**
 * WIFI
 */
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include "credentials.h" // WiFi SSID and password
ESP8266WebServer server(80);

/**
 * I2C
 */
#include <Wire.h>
#include <SPI.h>

/**
 * BMP280
 */
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

#define BMP280_ADDRESS (0x76) // setup device address
Adafruit_BMP280 bmp; // sensor init
float bmp280Data[2]; // 0 => temperature (°C), 1 => pressure (hPa)


void bmp280Setup() {
  if (!bmp.begin(BMP280_ADDRESS)) {
    Serial.println("BME280 sensor not found!");
  }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
}

void bmp280Read() {
  bmp280Data[0] = bmp.readTemperature();
  bmp280Data[1] = (bmp.readPressure() / 100.0F) + 32;
}

/**
 * VEML 7700, light sensor
 */
#include "Adafruit_VEML7700.h"
Adafruit_VEML7700 veml = Adafruit_VEML7700();
float vemlData[3]; // 0 => lux, 1 => white, 2 => raw ALS

void vemlSetup () {
  if (!veml.begin()) {
    Serial.println("Sensor VEML7700 not found");
  } else {
    Serial.println("Sensor VEML7700 found");
  }

  veml.setGain(VEML7700_GAIN_1);
  veml.setIntegrationTime(VEML7700_IT_800MS);

  Serial.print(F("Gain: "));
  switch (veml.getGain()) {
    case VEML7700_GAIN_1: Serial.println("1"); break;
    case VEML7700_GAIN_2: Serial.println("2"); break;
    case VEML7700_GAIN_1_4: Serial.println("1/4"); break;
    case VEML7700_GAIN_1_8: Serial.println("1/8"); break;
  }

  Serial.print(F("Integration Time (ms): "));
  switch (veml.getIntegrationTime()) {
    case VEML7700_IT_25MS: Serial.println("25"); break;
    case VEML7700_IT_50MS: Serial.println("50"); break;
    case VEML7700_IT_100MS: Serial.println("100"); break;
    case VEML7700_IT_200MS: Serial.println("200"); break;
    case VEML7700_IT_400MS: Serial.println("400"); break;
    case VEML7700_IT_800MS: Serial.println("800"); break;
  }

  // veml.setLowThreshold(10000);
  // veml.setHighThreshold(20000);
  // veml.interruptEnable(true);
}

void vemlRead () {
  vemlData[0] = veml.readLux();
  vemlData[1] = veml.readWhite();
  vemlData[2] = veml.readALS();
}

/**
 * SHT31
 */
#include "SHT31.h"
SHT31 sht;

float sht31Data[2]; // 0 => temperature, 1 => humidity

void sht31Setup () {
  Wire.begin();
  sht.begin(0x44);    //Sensor I2C Address
  Wire.setClock(100000);
  uint16_t stat = sht.readStatus();
  Serial.print(stat, HEX);
  Serial.println();
}

void sht31Read () {
  sht.read();
  sht31Data[0] = sht.getTemperature();
  sht31Data[1] = sht.getHumidity();
  
}

/**
 * MQ-135, air quality sensor
 */
#include "MQ135.h"
#define ANALOG_PIN A0

MQ135 senzorMQ = MQ135(ANALOG_PIN);

float mq135Data[1];

void mq135Read () {
  mq135Data[0] = senzorMQ.getPPM();
}

/**
 * HTTP CONTROLLERS
 */
// Serving Hello world
void getHelloWorldController() {
    server.send(200, "text/json", "{\"name\": \"Hello world\"}");
}

void getTemperatureController () {
  server.send(200, "text/json", String("{ \"temp\": \""+ String(sht31Data[0], 2) +"\", \"unit\": \"°C\" }"));
}

void getPressureController () {
  server.send(200, "text/json", String("{ \"press\": \""+ String(bmp280Data[1], 3) +"\", \"unit\": \"hPa\" }"));
}

void getLuxController () {
  server.send(200, "text/json", String("{ \"lux\": \""+ String(vemlData[0], 3) +"\", \"unit\": \"lx\" }"));
}

void getWhiteController () {
  server.send(200, "text/json", String("{ \"white\": \""+ String(vemlData[1], 3) +"\", \"unit\": \"\" }"));
}

void getALSController () {
  server.send(200, "text/json", String("{ \"als\": \""+ String(vemlData[2], 3) +"\", \"unit\": \"\" }"));
}

void getHumidityController () {
  server.send(200, "text/json", String("{ \"humidity\": \""+ String(sht31Data[1], 3) +"\", \"unit\": \"rh\" }"));
}

void getPpmController () {
  server.send(200, "text/json", String("{ \"gas\": \""+ String(mq135Data[0], 3) +"\", \"unit\": \"ppm\" }"));
}

void getAllController () {
  server.send(200, "text/json", String("[\
      { \"temp\": \""+ String(sht31Data[0], 2) +"\", \"unit\": \"°C\" }, \
      { \"press\": \""+ String(bmp280Data[1], 3) +"\", \"unit\": \"hPa\" }, \
      { \"humidity\": \""+ String(sht31Data[1], 3) +"\", \"unit\": \"rh\" }, \
      { \"lux\": \""+ String(vemlData[0], 3) +"\", \"unit\": \"lx\" }, \
      { \"white\": \""+ String(vemlData[1], 3) +"\", \"unit\": \"\" }, \
      { \"als\": \""+ String(vemlData[2], 3) +"\", \"unit\": \"\" }, \
      { \"gas\": \""+ String(mq135Data[0], 3) +"\", \"unit\": \"ppm\" } \
     ]"));
}

void handleNotFound() { // Manage not found URL
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}
 
/**
 * ROUTING
 */
void restServerRouting() {
    server.on("/", HTTP_GET, []() {
        server.send(200, F("text/html"), F("Sensor Array Web Server"));
    });
    
    server.on(F("/helloWorld"), HTTP_GET, getHelloWorldController);
    server.on(F("/temperature"), HTTP_GET, getTemperatureController);
    server.on(F("/pressure"), HTTP_GET, getPressureController);
    server.on(F("/humidity"), HTTP_GET, getHumidityController);
    server.on(F("/lux"), HTTP_GET, getLuxController);
    server.on(F("/white"), HTTP_GET, getWhiteController);
    server.on(F("/als"), HTTP_GET, getALSController);
    server.on(F("/gas"), HTTP_GET, getPpmController);

    server.on(F("/all"), HTTP_GET, getAllController);
}
 
void restAPISetup() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
 
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
 
  // Activate mDNS this is used to be able to connect to the server
  // with local DNS hostmane esp8266.local
  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }
 
  // Set server routing
  restServerRouting();
  // Set not found response
  server.onNotFound(handleNotFound);
  // Start server
  server.begin();
  Serial.println("HTTP server started");
}

void listI2CDevices() {
  byte error, address;
  int nDevices;
  
  nDevices = 0;
  for(address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
 
    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.print(address,HEX);
      Serial.println("  !");
 
      nDevices++;
    }
    else if (error==4)
    {
      Serial.print("Unknown error at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.println(address,HEX);
    }    
  }
  
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");
}
 
void setup(void) {
  Serial.begin(115200);
  restAPISetup();
  bmp280Setup();
  vemlSetup();
  sht31Setup();
}
 
void loop(void) {
  server.handleClient();
  bmp280Read();
  vemlRead();
  sht31Read();
  mq135Read();
  delay(1000);
}
