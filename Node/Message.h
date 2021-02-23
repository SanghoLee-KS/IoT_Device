typedef struct{
  byte opcode; //1- REQ_ID, 2- RES_ID
  byte mac[6];
  int id;
  byte action;
  int data;
}MESSAGE;
