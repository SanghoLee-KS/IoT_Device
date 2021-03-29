void WifiInit(uint8_t* mac);
void socketConnect() ;
//int recvData(MESSAGE message);
int recvData(char* message);
void sendData(MESSAGE message);
void printWifiStatus();
void printMacAddress(byte mac[]);
