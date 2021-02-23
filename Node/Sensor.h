/* Functions */
void SensorInit();
float ReadSensor();
void readAccelGyro();
//센서 들의 기본값들의 평균을 내는 루틴 (센서 보정 루틴)
void calibAccelGyro();
void SendDataToSerial();
void initDT();
void calcDT();
//가속도 센서 처리 루틴
void calcAccelYPR();
void calcGyroYPR();
void calcFilteredYPR();
