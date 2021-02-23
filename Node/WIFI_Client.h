void WifiInit(uint8_t* mac);
//void recvData();
MESSAGE recvData();
void printWifiStatus();
void printMacAddress(byte mac[]);
//void sendToServ(uint8_t* msg);
//void sendToServ(byte msg[]);
void sendToServ(MESSAGE message);
