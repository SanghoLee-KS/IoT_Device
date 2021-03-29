void hwSetup();
void connectToServer();

void registerID(); 
void doAction(); 
void recvHeartBeat(); 
void disconnectServer(); 
void chkState();
void chkHeartBeat();

//interrupt
void TC5_Handler (void);
void tcConfigure(int sampleRate);
bool tcIsSyncing();
void tcStartCounter();
void tcReset();
void tcDisable();
