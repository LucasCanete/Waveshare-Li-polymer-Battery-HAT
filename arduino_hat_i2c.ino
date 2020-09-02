/*
 
Code for communication between arduino and Li-polymer battery Hat trough I2C protocol.
This code is for analyzing:

1. The voltage of the battery attached to the HAT
2. The temperature of the built-in NTC of the HAT
3. Any hardware-related issues received by the IRQ Pin.

Extra information regarding:

2. Received value of the Temperature corresponds to the ADC Value and must be checked with Datasheet for further information.
3. Hardware related issue is received as a 1 byte binary value. If any bit is 1 this means an issue according to the bit position.

-To the HAT:

* I2C adress: 0x3C
* Register adresses for Battery: 0x14, 0x15
* Register adresses for temperature: 0x1B, 0x1C
* Register adress for IRQ-received issues: 0x0A

September 2020

Lucas Canete

*/


#include "Wire.h" // This library allows you to communicate with I2C devices.
const int MPU_ADDR = 0x3C; // I2C address of SW6106 IC of the HAT

byte READ_LENGTH = 1; //amount of bytes to read from every adress 

void setup() {
  
  Serial.begin(9600);
  Wire.begin();


}

void loop() {

 
  //################ DATA OUTPUT ############################
  float vbat = getBattery();
  uint16_t temperature = getTemperature();
  uint8_t error = getError();


  Serial.print("Vbat: ");
  Serial.print(vbat);
  Serial.print(" v.");
  Serial.print(" NTC Temperature: ");
  Serial.print(temperature);
  Serial.print(" IRQ Error: ");
  Serial.println(error,BIN); // If any bit of the IRQ binary value is 1 that means an issue has occured. Look in datasheet
}


 //##################### BATTERY READING #################################
float getBattery(){
  
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x14); //First register adress for the battery 
  Wire.endTransmission(true); // the parameter indicates that the Arduino will send a restart. As a result, the connection is kept active.
  Wire.requestFrom(MPU_ADDR, READ_LENGTH);  //read 1 byte from the i2c adress

  byte LSB = Wire.read();


  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x15); //Second register adress for the battery
  Wire.endTransmission(true);
  Wire.requestFrom(MPU_ADDR, READ_LENGTH); 

  byte MSB = Wire.read();

  //Set last four bits to 0 as they are unused
  for(int i = 4; i<=7;i++){
    bitClear(MSB,i);
    }


  uint16_t val = ((MSB << 8) | LSB); // Battery Voltage
  
  float vbat = val*0.0012;
  
  return vbat;
  
  }


//###################### NTC Temperature #####################################
int getTemperature(){
  
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0X1B); //First adress to determine ntc temperature
  Wire.endTransmission(true);
  Wire.requestFrom(MPU_ADDR, READ_LENGTH);

  byte ntc_temp_1b = Wire.read();
  
  //clears from bit 0 to 3 as they are not used
  for (int i = 0; i < 4; i++) {
    bitClear(ntc_temp_1b, i);
  }

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1C); //Second register adress to determine ntc temperature
  Wire.endTransmission(true);
  Wire.requestFrom(MPU_ADDR, READ_LENGTH);

  byte ntc_temp_1c = Wire.read();
  //Final ADC value that corresponds to the NTC temperature. For more information look Datasheet
  uint16_t ntc_temp = ((ntc_temp_1b << 4) | ntc_temp_1c ); //Shift only four to left not 8 as indicated in datasheet.
  return ntc_temp;
  
  }

  
//########### ERROR DETECTION WITH IRQ #############
int getError(){
  
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x0A); //Requesting information to register for IRQ-related issues
  Wire.endTransmission(true);

  Wire.requestFrom(MPU_ADDR,READ_LENGTH);

  byte IRQ = Wire.read();
  return IRQ;

  }
