//client
#include "Arduino.h"
#include "Message.h"
#include "WIFI_Client.h"
#include <SPI.h>
#include <WiFiNINA.h>


//#include "arduino_secrets.h" 
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = "KT_GiGA_2G_TEST";        
//char ssid[] = "MMV2G";        
char pass[] = "0000003151";    
//char pass[] = "mmv309309";    
int keyIndex = 0;            // your network key Index number (needed only for WEP)
char incomingByte = 0;       // InputByte

int status = WL_IDLE_STATUS;
IPAddress server(202,30,32,70);  

WiFiClient client;

void WifiInit(uint8_t* mac) {
//  while (!Serial) {
//    ; // wait for serial port to connect. Needed for native USB port only
//  }

  // Wifi Module 통신 (SPI인듯)
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  WiFi.macAddress(mac);
  

  // Wifi접속 요청
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(5000);
  }
  Serial.println("Connected to wifi");
  printWifiStatus();

  Serial.println("\nStarting connection to server...");
  // 접속 성공 시 IP, PORT로 Server 접속
  if (client.connect(server, 12344))
    Serial.println("Connected to Server");
}

//void recvData() {
MESSAGE recvData() {
/*
    while(client.available()) {
        //명령 입력
        char c = client.read();
        Serial.println(c, HEX);
        delay(1);
    }
  */
  MESSAGE recMessage = {0};
  if(client.available() >= 12){
    char* dp = (char*) &recMessage;

    byte css; int cssc=1;
    for(int i=0; i<20; i++) {
      css = *dp++ = client.read();
      Serial.print(cssc++);
      Serial.print(" ");
      Serial.write(css);
    }
//    for(int i=0; i<20; i++) *dp++ = client.read();
  }
  

  // 접속 해제 시, client종료
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting from server.");
    client.stop();

    // do nothing forevermore:
    while (true);
  }
  return recMessage;
}
/*
void sendToServ(byte msg[]){
  client.write(msg, 6);
}
*/

void sendToServ(MESSAGE message){
/*
    char* dp = (char*) &message;

    byte css; int cssc=1;
    for(int i=0; i<20; i++) {
      css = *dp++;
      Serial.print(cssc++);
      Serial.print(" ");
      Serial.write(css);
    }
*/
    
  client.write((char*)&message, 20);
}



void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0) {
      Serial.print(":");
    }
  }
  Serial.println();
}
