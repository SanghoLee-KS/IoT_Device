#include "Arduino.h"
#include "Message.h"
#include "WIFI_Client.h"
#include <SPI.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h" 


int status = WL_IDLE_STATUS;
WiFiClient client;

void WifiInit(uint8_t* mac) {

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
}

void socketConnect() {
  int n = 0;
  Serial.println("\nStarting connection to server...");
  // 접속 성공 시 IP, PORT로 Server 접속
  while( !client.connect(server, port)){
    n++;
    Serial.print("\nStarting connection to server...");  
    Serial.println(n);

    if(n > 3){
      Serial.println("Failed Connecting Server");
      while(1);
    }
  }
  Serial.println("Connected to Server");
}

//int recvData(MESSAGE message) {
int recvData(char* message) {
  int isRecv = 0;
  if(client.available() >= 12){
    isRecv = 1;
    //char* dp = (char*) &message;
    char* dp = message;
    for(int i=0; i<20; i++) *dp++ = client.read();
  }

  // 접속 해제 시, client종료
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting from server.");
    client.stop();

    // do nothing forevermore:
    while (true);
  }
  return isRecv;
}

void sendData(MESSAGE message){
  client.write((char*)&message, sizeof(message));
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
