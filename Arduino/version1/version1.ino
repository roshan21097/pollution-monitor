#include <Arduino.h>
#include <BMP280.h>
#include <SoftwareSerial.h>

#define LENG 31   //0x42 + 31 bytes equal to 32 bytes
#define P0 1013.25
#define ELEVATION 1189

BMP280 bmp;
SoftwareSerial PMSerial(10, 11); // RX, TX
unsigned char buf[LENG];

double T = 0; //Starting temperature value
double P = 0; //Starting pressure value
int PM01Value=0;          //define PM1.0 value of the air detector module
int PM2_5Value=0;         //define PM2.5 value of the air detector module
int PM10Value=0;         //define PM10 value of the air detector module
char measure = 0;


float adjustPressure(float pressure, float elevation)
{
  return ((pressure)/pow((1-(ELEVATION)/44330), 5.255));
}


void collectPressureData() 
{
  measure = bmp.startMeasurment();
  if(measure != 0) {
    delay(measure);
    measure = bmp.getTemperatureAndPressure(T, P);
    if(measure != 0) {
      P = P + 17; // '+17' is a correction for the sensor error
      P = adjustPressure(P, ELEVATION);
      T = T - 0.8; // like said above
      Serial.print("T: ");
      Serial.print(T);
      Serial.print(" C");
      Serial.print("P: ");
      Serial.print(P);
      Serial.print(" hPa");
    }
    else
      Serial.print("Error.");
  }
  else
    Serial.print("Error.");
}
  

char checkValue(unsigned char *thebuf, char leng)
{  
  char receiveflag=0;
  int receiveSum=0;

  for(int i=0; i<(leng-2); i++){
  receiveSum=receiveSum+thebuf[i];
  }
  receiveSum=receiveSum + 0x42;
 
  if(receiveSum == ((thebuf[leng-2]<<8)+thebuf[leng-1]))  //check the serial data 
  {
    receiveSum = 0;
    receiveflag = 1;
  }
  return receiveflag;
}

int transmitPM01(unsigned char *thebuf)
{
  int PM01Val;
  PM01Val=((thebuf[3]<<8) + thebuf[4]); //count PM1.0 value of the air detector module
  return PM01Val;
}

//transmit PM Value to PC
int transmitPM2_5(unsigned char *thebuf)
{
  int PM2_5Val;
  PM2_5Val=((thebuf[5]<<8) + thebuf[6]);//count PM2.5 value of the air detector module
  return PM2_5Val;
  }

//transmit PM Value to PC
int transmitPM10(unsigned char *thebuf)
{
  int PM10Val;
  PM10Val=((thebuf[7]<<8) + thebuf[8]); //count PM10 value of the air detector module  
  return PM10Val;
}


void collectPMData()
{
  if(PMSerial.find(0x42))
  {    
    PMSerial.readBytes(buf,LENG);
    if(buf[0] == 0x4d)
    {
      if(checkValue(buf,LENG))
      {
        PM01Value=transmitPM01(buf); //count PM1.0 value of the air detector module
        PM2_5Value=transmitPM2_5(buf);//count PM2.5 value of the air detector module
        PM10Value=transmitPM10(buf); //count PM10 value of the air detector module 
      }           
    }
  }
    
    static unsigned long OledTimer=millis();  
    if (millis() - OledTimer >=1000) 
    {
      OledTimer=millis(); 
      
      Serial.print("PM1.0: ");  
      Serial.print(PM01Value);
      Serial.println("  ug/m3");            
    
      Serial.print("PM2.5: ");  
      Serial.print(PM2_5Value);
      Serial.println("  ug/m3");     
      
      Serial.print("PM1 0: ");  
      Serial.print(PM10Value);
      Serial.println("  ug/m3");   
      Serial.println();
    } 
}


void setup() {
  // put your setup code here, to run once:
  PMSerial.begin(9600);
  PMSerial.setTimeout(2000);    
  
  Serial.begin(9600);
  if(!bmp.begin()) {             // you prefer
    delay(1000);
    Serial.print("Init. failed.");
    delay(1000);
    Serial.print("Check wiring.");
    while(1);
  }
  else
    Serial.print("Init. OK.");
    
    bmp.setOversampling(4);
//    delay(2000);
//    collectPressureData();

}

void loop() {
  // put your main code here, to run repeatedly:
  collectPressureData();
  collectPMData();
  delay(2000);

}
