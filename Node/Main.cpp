#include "Arduino.h"
#include "Main.h"
#include "Sensor_AccGyro.h"
#include "Message.h"
#include "WIFI_Client.h"



/*내부 상태 변수*/
byte state = 0; //0-미등록, 1-등록(Sensing)
byte mac[6];
int id;

/*LED, Buzzer*/
int ledPin = 2;
int buzzerPin = 3;

/*센싱 데이터*/
float angle = 0;
boolean prev_isOpened,isOpened = false;

/*메세지 포멧*/
MESSAGE message;

/*Heart Beat*/
unsigned long ServerHeart_prev, ServerHeart_current;
uint32_t onesecondinterval=0;

void setup() {
  Serial.begin(9600);
  delay(10000);
  
  hwSetup();
  connectToServer();

  tcConfigure(1000);
}

void loop() {
  
  chkState();
    
  if(recvData((char*)&message)) {
    Serial.println("메세지 수신");
    switch(message.opcode)
    {
      case 2:
        registerID(); break;
      case 4:
        doAction(); break;
      case 8:
        recvHeartBeat(); break;
       case 10:
        disconnectServer(); break;
    }
  }
    
  chkHeartBeat();
  
}

void hwSetup(){
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  
  sensor_AccGyro_Init();
  WifiInit(mac);
}

void connectToServer() {
  socketConnect();

  message.opcode = 1; //REQ_ID
  memcpy(message.mac, mac, sizeof(mac));
  sendData(message);
}

void registerID(){
    Serial.println();
    Serial.println();
    Serial.print("[등록] 할당 받은 MAC : ");
    printMacAddress(message.mac);
    Serial.println();
    id = message.id;
    Serial.print("[등록] 할당 받은 ID : ");
    Serial.println(id);

    tcStartCounter();
}

void doAction() {
  Serial.println();
  Serial.println("[제어 메시지 수신]");
  
  Serial.print("action : ");
  Serial.println(message.action);
  Serial.print("data : ");
  Serial.println(message.data);

  //ToDO : LED + BUZZER
  digitalWrite(ledPin, HIGH); digitalWrite(buzzerPin, HIGH);
  delay(1000);
  digitalWrite(ledPin, LOW);  digitalWrite(buzzerPin, LOW);
  delay(1000);
  digitalWrite(ledPin, HIGH); digitalWrite(buzzerPin, HIGH);
  delay(1000);
  digitalWrite(ledPin, LOW);  digitalWrite(buzzerPin, LOW);
}

void recvHeartBeat(){
  ServerHeart_prev = millis();
}

void disconnectServer() {
  Serial.println();
  Serial.println("[연결 종료]");
  while(1) {};
}

void chkState() {
  angle = readSensor_AccGyro();
  //TODO : readSensor_angle(); 함수명 고민
  
  if(angle>0.2 || angle<-0.2)
    isOpened = true;
  else isOpened = false;

  if(prev_isOpened != isOpened) { //문의 상태 변화 시 메시지 전송
      message.opcode = 3;
      message.id = id;
      if(isOpened == true) {
        Serial.println("Door is Opened");
        message.action = 1;
        message.data = (int)(angle*100);        
      }
      else {
        Serial.println("Door is Closed");
        message.action = 0;
        message.data = (int) (angle*100);
      }
      Serial.print("Door angle : ");
      Serial.println(message.data);
      sendData(message);
    }
    prev_isOpened = isOpened;
}

void chkHeartBeat() {
  
  if(onesecondinterval > 15){
    Serial.println("Heart Beat...");
    message.opcode = 7;
    message.id = id;
    message.action = 0;
    message.data = (int) (angle*100);
    sendData(message);

    onesecondinterval = 0;
  }
}









/**/
//this function gets called by the interrupt at <sampleRate>Hertz
void TC5_Handler (void) {
  //YOUR CODE HERE 
  onesecondinterval++; 

//  if(heartState == false) 
//    heartState = true;
  // END OF YOUR CODE
  TC5->COUNT16.INTFLAG.bit.MC0 = 1; //Writing a 1 to INTFLAG.bit.MC0 clears the interrupt so that it will run again
}


/* 
 *  TIMER SPECIFIC FUNCTIONS FOLLOW
 *  you shouldn't change these unless you know what you're doing
 */

//Configures the TC to generate output events at the sample frequency.
//Configures the TC in Frequency Generation mode, with an event output once
//each time the audio sample frequency period expires.
void tcConfigure(int sampleRate)
{
 // select the generic clock generator used as source to the generic clock multiplexer
 GCLK->CLKCTRL.reg = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(GCM_TC4_TC5)) ;
 while (GCLK->STATUS.bit.SYNCBUSY);

 tcReset(); //reset TC5

 // Set Timer counter 5 Mode to 16 bits, it will become a 16bit counter ('mode1' in the datasheet)
 TC5->COUNT16.CTRLA.reg |= TC_CTRLA_MODE_COUNT16;
 // Set TC5 waveform generation mode to 'match frequency'
 TC5->COUNT16.CTRLA.reg |= TC_CTRLA_WAVEGEN_MFRQ;
 //set prescaler
 //the clock normally counts at the GCLK_TC frequency, but we can set it to divide that frequency to slow it down
 //you can use different prescaler divisons here like TC_CTRLA_PRESCALER_DIV1 to get a different range
 TC5->COUNT16.CTRLA.reg |= TC_CTRLA_PRESCALER_DIV1024 | TC_CTRLA_ENABLE; //it will divide GCLK_TC frequency by 1024
 //set the compare-capture register. 
 //The counter will count up to this value (it's a 16bit counter so we use uint16_t)
 //this is how we fine-tune the frequency, make it count to a lower or higher value
 //system clock should be 1MHz (8MHz/8) at Reset by default
 TC5->COUNT16.CC[0].reg = (uint16_t) (SystemCoreClock / sampleRate);
 while (tcIsSyncing());
 
 // Configure interrupt request
 NVIC_DisableIRQ(TC5_IRQn);
 NVIC_ClearPendingIRQ(TC5_IRQn);
 NVIC_SetPriority(TC5_IRQn, 0);
 NVIC_EnableIRQ(TC5_IRQn);

 // Enable the TC5 interrupt request
 TC5->COUNT16.INTENSET.bit.MC0 = 1;
 while (tcIsSyncing()); //wait until TC5 is done syncing 
} 

//Function that is used to check if TC5 is done syncing
//returns true when it is done syncing
bool tcIsSyncing()
{
  return TC5->COUNT16.STATUS.reg & TC_STATUS_SYNCBUSY;
}

//This function enables TC5 and waits for it to be ready
void tcStartCounter()
{
  TC5->COUNT16.CTRLA.reg |= TC_CTRLA_ENABLE; //set the CTRLA register
  while (tcIsSyncing()); //wait until snyc'd
}

//Reset TC5 
void tcReset()
{
  TC5->COUNT16.CTRLA.reg = TC_CTRLA_SWRST;
  while (tcIsSyncing());
  while (TC5->COUNT16.CTRLA.bit.SWRST);
}

//disable TC5
void tcDisable()
{
  TC5->COUNT16.CTRLA.reg &= ~TC_CTRLA_ENABLE;
  while (tcIsSyncing());
}
