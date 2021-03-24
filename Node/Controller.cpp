#include "Arduino.h"
#include "Message.h"
#include "Controller.h"
#include "Sensor.h"
#include "WIFI_Client.h"


/*내부 상태 변수*/
byte state = 0; //0-미등록, 1-등록(Sensing)
byte mac[6];
int id;

/*센싱 데이터*/
float angle = 0;
boolean prev_isOpened,isOpened = false;

/*메세지 포멧*/
MESSAGE message;

/*Heart Beat*/
unsigned long Heart_prev, Heart_current;

/*LED, Buzzer*/
int ledPin = 2;
int buzzerPin = 3;


void setup() {
  Serial.begin(9600);

  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
//  while (!Serial);

  SensorInit();
  WifiInit(mac);

  Serial.print("MAC: ");
  printMacAddress(mac);
  //sendToServ(mac);
  message.opcode = 1; //REQ_ID
  memcpy(message.mac, mac, sizeof(mac));
  sendToServ(message);
  
  Heart_prev = Heart_current = millis();
}

void loop() {
  
  message=recvData();

  if(state){ //등록 완료
    
    /*Sensing*/
    angle = ReadSensor();


    if(angle>0.2 || angle<-0.2) isOpened = true;
    else isOpened = false;

    if(prev_isOpened != isOpened) { //문의 상태 변화 시 메시지 전송
      message.opcode = 3;
      if(isOpened == true) {
        Serial.println("Door is Opened");
        message.id = id;
        message.action = 1;
        message.data = (int)(angle*100);
        Serial.println(message.data);
        sendToServ(message);
        
      }
      else {
        Serial.println("Door is Closed");
        message.id = id;
        message.action = 0;
        message.data = (int) (angle*100);
        Serial.println(message.data);
        sendToServ(message);
      }
    }
    prev_isOpened = isOpened;
  }
  else{ //미등록
    if(message.opcode == 2) {
      Serial.println();
      Serial.println();
      Serial.print("[등록] 할당 받은 MAC : ");
      printMacAddress(message.mac);
      Serial.println();
      id = message.id;
      Serial.print("[등록] 할당 받은 ID : ");
      Serial.println(id);
  
      state = 1;
    }
  }
  
    //서버의 제어 메시지
    if(message.opcode == 4) { 
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


  Heart_current = millis();
  if((Heart_current - Heart_prev) >= 15000) {
    Serial.println("Heart Beat...");
    message.opcode = 7;
    message.id = id;
    message.action = 0;
    message.data = (int) (angle*100);
    Serial.println(message.data);
    sendToServ(message);
    Heart_prev = Heart_current;
  }

  if(message.opcode == 10) { //서버의 연결종료 메시지
    Serial.println();
    Serial.println("[연결 종료]");
    while(1){};
  }
}
