#include <Arduino.h>
#include <WiFi101.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include "arduino_secrets.h"

char ssid[] = SECURE_SSID;
char pass[] = SECURE_PASS;

// Constants


// Operational Varibles
int flag = 0;
int status = WL_IDLE_STATUS;
long timeoutTimer;
char displayBuffer[4] = {' ', ' ', ' ', ' '};
String replyBuffer;

// Objects
WiFiServer server(9000);
Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();

// functions
void clearDisplay(){
  for (int i = 0; i < 4; i++){ displayBuffer[i] = ' '; }  // clean buffer varible
  for (int i = 0; i < 4; i++){ alpha4.writeDigitAscii(i, displayBuffer[i]); }
  alpha4.writeDisplay();
}

void writeDisplay(char characters[4]){
  for(int i = 0; i < 4; i++){ alpha4.writeDigitAscii(i, characters[i]); }
  alpha4.writeDisplay();
}

void scrollText(String text, int delayTime){
  for(int i = 0; i < text.length(); i++){
    char letter = text[i];
    displayBuffer[0] = displayBuffer[1];
    displayBuffer[1] = displayBuffer[2];
    displayBuffer[2] = displayBuffer[3];
    displayBuffer[3] = letter;
    writeDisplay(displayBuffer);
    delay(delayTime);
  }
  clearDisplay();
}

void printWiFiStatus(){
  // SSID
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // IP Address
  IPAddress ip  = WiFi.localIP();
  Serial.print("IP: ");
  Serial.println(ip);

  // RSSI (strength of wifi signal) while
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void setup(){
  // setup alphanum display //
  alpha4.begin(0x70);
  clearDisplay();

  // setup misc. //
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(9600);


  // setup WiFi101 shield //

  WiFi.setPins(8, 7, 4, 2);
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // attempt to connect to WiFi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);  // attempt to connect to network
    delay(10000); // wait for connection
  }

  // you're connected now, so print out the data:
  Serial.print("You're connected to the network");
  server.begin(); // I want to say "hit it Chewy!", anyway start the server.
  printWiFiStatus();
  scrollText("SERVER ONLINE", 300);
}

void loop(){
  WiFiClient client = server.available();
  if (client){
    Serial.println("New client connected");
    timeoutTimer = millis();
    while (flag == 0){
      // recieve and handle the client's data:
      if (client.available() > 0) {
        // reset the timeoutTimer so that the client is not kicked
        timeoutTimer = millis();

        // get the client's response
        replyBuffer = "";
        while (client.available() > 0) {
          char currentChar = client.read();
          replyBuffer = replyBuffer + currentChar;
        }
        // clean the data
        replyBuffer[replyBuffer.length()-1] = '\0';

        // decide what to do with the response
        if (replyBuffer == "quit"){
          flag = 1;
        } else {
          replyBuffer[replyBuffer.length()-1] = ' ';
          scrollText(replyBuffer, 300);
        }
      }

      // if client is idle for 5 seconds quit them:
      if (millis() - timeoutTimer >= 15000){
        Serial.println("Client was kicked due to inactivity.");
        flag = 1;
      }
    }
    flag = 0;
    client.stop();
  }
}
