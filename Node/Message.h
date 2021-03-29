/*
  1- REQ_ID, 2- RES_ID, 
  3- SEND_Status, 4- RECV_Control,
  7- SEND_HeartBeat, 8- RECV_HeartBeat,
  10- DISCONNECT
*/
typedef struct{
  byte opcode; 
  byte mac[6];
  int id;
  byte action;
  int data;
}MESSAGE;
