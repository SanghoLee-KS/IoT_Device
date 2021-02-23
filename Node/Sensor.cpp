#include "Arduino.h"
#include "Sensor.h"
#include <Arduino_LSM6DS3.h>

float AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;


float dt; //시간 단위
float accel_angle_x, accel_angle_y, accel_angle_z;
float gyro_angle_x, gyro_angle_y, gyro_angle_z;
float filtered_angle_x, filtered_angle_y, filtered_angle_z;
float baseAcX, baseAcY, baseAcZ;
float baseGyX, baseGyY, baseGyZ;
float gyro_x, gyro_y, gyro_z; //각속도 저장 전역변수 //각속도 : 단위시간당 회전한 각도
bool isRightDoor;

unsigned long t_now;
unsigned long t_prev;

void SensorInit() {
  if (!IMU.begin()) {   //가속도 자이로 센서 통신 여부 확인
    Serial.println("Failed to initialize IMU!");

    while (1);
  }

  /*초기 코드*/
  Serial.print("Accelerometer sample rate = ");
  Serial.print(IMU.accelerationSampleRate());
  Serial.println(" Hz");
  Serial.println();
  Serial.println("Acceleration in G's");
  Serial.println("X\tY\tZ");
  Serial.print("Gyroscope sample rate = ");
  Serial.print(IMU.gyroscopeSampleRate());
  Serial.println(" Hz");
  Serial.println();
  Serial.println("Gyroscope in degrees/second");
  Serial.println("X\tY\tZ");


  //센서 보정 루틴
  calibAccelGyro();
  //시간 간격에 대한 초기화
  initDT();
  
}

void CheckDoorPosition() {
  for(int i=0; i<2; i++) {
    //가속도, 자이로 센서 값 읽기
    readAccelGyro();
    //시간 간격 계산
    calcDT();
    //가속도 센서 처리 루틴
    calcAccelYPR();
    //자이로 센서 처리 루틴
    calcGyroYPR(); 
    calcFilteredYPR();
  }
  
}

float ReadSensor() {
  for(int i=0; i<2; i++) {
    //가속도, 자이로 센서 값 읽기
    readAccelGyro();
    //시간 간격 계산
    calcDT();
    //가속도 센서 처리 루틴
    calcAccelYPR();
    //자이로 센서 처리 루틴
    calcGyroYPR(); 
    calcFilteredYPR();
  }
  SendDataToSerial();
  return filtered_angle_z;
}  


void readAccelGyro() {
  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(AcX, AcY, AcZ);
  } else Serial.println("ERROR ACC");
 /*
    Serial.print(AcX);
    Serial.print('\t');
    Serial.print(AcY);
    Serial.print('\t');
    Serial.println(AcZ);
    */
  delay(100);
  if (IMU.gyroscopeAvailable()) {
    IMU.readGyroscope(GyX, GyY, GyZ);
  }else Serial.println("ERROR GYR");
  
  /*
  Serial.print(GyX);
    Serial.print('\t');
    Serial.print(GyY);
    Serial.print('\t');
    Serial.println(GyZ);
    */
}

//센서 들의 기본값들의 평균을 내는 루틴 (센서 보정 루틴)
void calibAccelGyro() {
  float sumAcX = 0, sumAcY = 0, sumAcZ = 0;
  float sumGyX = 0, sumGyY = 0, sumGyZ = 0;
  Serial.println("CALIB_ACCEL_GYRO");
  //가속도 자이로 센서 읽기
  readAccelGyro();

  //읽은 값을 토대로 평균값 구하기
  for(int i=0; i<10; i++)
  {
    readAccelGyro();
    sumAcX += AcX;  sumAcY += AcY;  sumAcZ += AcZ;
    sumGyX += GyX;  sumGyY += GyY;  sumGyZ += GyZ;
    delay(100); //0.1초
  }

  baseAcX = sumAcX / 10;
  baseAcY = sumAcY / 10;
  baseAcZ = sumAcZ / 10;

  baseGyX = sumGyX / 10;
  baseGyY = sumGyY / 10;
  baseGyZ = sumGyZ / 10;
  /*
  Serial.print("ACC : ");
  Serial.print(baseAcX); Serial.print("\t");
  Serial.print(baseAcY); Serial.print("\t");
  Serial.print(baseAcZ); Serial.println("\t");
  Serial.print("GYR : ");
  Serial.print(baseGyX); Serial.print("\t");
  Serial.print(baseGyY); Serial.print("\t");
  Serial.print(baseGyZ); Serial.println("\t");
  */
}

void SendDataToSerial()
{
  /*
  Serial.print(F("#FIL"));
  Serial.print(filtered_angle_x, 2);
  Serial.print(F(","));
  Serial.print(filtered_angle_y, 2);
  Serial.print(F(","));
  Serial.print(filtered_angle_z, 2);
  Serial.println(F(""));
  delay(5);
  */
}


void initDT() {
  t_prev = millis();
}

void calcDT() {
  t_now = millis();
  dt = (t_now - t_prev) / 1000.0;
  t_prev = t_now;
}

//가속도 센서 처리 루틴
void calcAccelYPR() {
  //가속도 센서 최종적인 보정값
  float accel_x, accel_y, accel_z;
  float accel_xz, accel_yz;
  const float RADIANS_TO_DEGREES = 180 / 3.14159;

  //가속도(직선) x축에 대한 현재 값 - 가속도 센서의 평균 값
  accel_x = AcX - baseAcX;  
  accel_y = AcY - baseAcY;
  accel_z = AcZ + (16384 - baseAcZ);

  //직선 +x축이 기울어진 각도를 구한다.
  accel_yz = sqrt(pow(accel_y, 2) + pow(accel_z, 2));
  accel_angle_y = atan(-accel_x / accel_yz) * RADIANS_TO_DEGREES;
  
  accel_xz = sqrt(pow(accel_x, 2) + pow(accel_z, 2));
  accel_angle_x = atan(accel_y / accel_xz) * RADIANS_TO_DEGREES;

  accel_angle_z = 0;
}


void calcGyroYPR(){
  const float GYROXYZ_TO_DEGREES_PER_SED = 131;
                                  //131 값은 1초동안 1도 돌때 자이로 값이 131이다.

  gyro_x = (GyX - baseGyX) / GYROXYZ_TO_DEGREES_PER_SED;
  gyro_y = (GyY - baseGyY) / GYROXYZ_TO_DEGREES_PER_SED;
  gyro_z = (GyZ - baseGyZ) / GYROXYZ_TO_DEGREES_PER_SED;

  gyro_angle_x += gyro_x * dt; //변화된 각 : 각속도 x 측정 주기 시간
  gyro_angle_y += gyro_y * dt;
  gyro_angle_z += gyro_z * dt;
}

void calcFilteredYPR(){
  const float ALPHA = 0.96;
  float tmp_angle_x, tmp_angle_y, tmp_angle_z;  //이전 필터 각도(prev)

  tmp_angle_x = filtered_angle_x + gyro_x * dt; //자이로 각도 = 각속도 x 센서 입력 주기
                                                //각속도 = 단위시간당 회전한 각도 -> 회전한 각도 / 단위시간
  tmp_angle_y = filtered_angle_y + gyro_y * dt;
  tmp_angle_z = filtered_angle_z + gyro_z * dt;

  filtered_angle_x = ALPHA * tmp_angle_x + (1.0 - ALPHA) * accel_angle_x;
  filtered_angle_y = ALPHA * tmp_angle_y + (1.0 - ALPHA) * accel_angle_y;
  filtered_angle_z = tmp_angle_z; //곡선 +Z 축은 자이로 센서만 이용해서 나타내고 있음(가속도 센서는 안함)
  //그래서 위에(calcAccelYPR) 보게 되면 accel_angle_z 는 0이다.
}
