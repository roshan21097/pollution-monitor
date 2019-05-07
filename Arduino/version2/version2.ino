#include <Arduino.h>
#include <BMP280.h>
#include <SoftwareSerial.h>
#include <DHT.h>

#define LENG 31   //0x42 + 31 bytes equal to 32 bytes
#define P0 1013.25
#define ELEVATION 1189

#define DHTPIN 7     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino


BMP280 bmp;
SoftwareSerial PMSerial(10, 11); // RX, TX For PM Sensor
SoftwareSerial GSMSerial(3, 5); // RX, TX For GSM Module

unsigned char buf[LENG]; // Reading bug for PM Sensor
unsigned long OledTimer = millis();

double T = 0; //Starting temperature value
double P = 0; //Starting pressure value
int PM01Value = 0;        //define PM1.0 value of the air detector module
int PM2_5Value = 0;       //define PM2.5 value of the air detector module
int PM10Value = 0;       //define PM10 value of the air detector module
char measure = 0;
const int NO2=3; //NO2 sensor on analog 3
const int CO=2; //CO sensor on analog 2
float Vout = 0; //output voltage
float Rs = 0; //Sensor Resistance
float ppbNO2 = 0; //ppb NO2
float ppmCO = 0; //ppm CO

float H; // Humidity value



float adjustPressure(float pressure, float elevation)
{
  return ((pressure) / pow((1 - (ELEVATION) / 44330), 5.255));
}


void collectPressureData()
{
  measure = bmp.startMeasurment();
  if (measure != 0) {
    delay(measure);
    measure = bmp.getTemperatureAndPressure(T, P);
    if (measure != 0) {
      P = P + 17; // '+17' is a correction for the sensor error
      P = adjustPressure(P, ELEVATION);
      T = T - 0.8; // like said above
      Serial.print("T: ");
      Serial.print(T);
      Serial.println(" C");
      Serial.print("P: ");
      Serial.print(P);
      Serial.println(" hPa");
    }
    else
      Serial.println("Error Code 1 in BMP Sensor");
  }
  else
    Serial.println("Error Code 2 in BMP Sensor");
}


char checkValue(unsigned char *thebuf, char leng)
{
  char receiveflag = 0;
  int receiveSum = 0;

  for (int i = 0; i < (leng - 2); i++) {
    receiveSum = receiveSum + thebuf[i];
  }
  receiveSum = receiveSum + 0x42;

  if (receiveSum == ((thebuf[leng - 2] << 8) + thebuf[leng - 1])) //check the serial data
  {
    receiveSum = 0;
    receiveflag = 1;
  }
  return receiveflag;
}

int transmitPM01(unsigned char *thebuf)
{
  int PM01Val;
  PM01Val = ((thebuf[3] << 8) + thebuf[4]); //count PM1.0 value of the air detector module
  return PM01Val;
}

//transmit PM Value to PC
int transmitPM2_5(unsigned char *thebuf)
{
  int PM2_5Val;
  PM2_5Val = ((thebuf[5] << 8) + thebuf[6]); //count PM2.5 value of the air detector module
  return PM2_5Val;
}

//transmit PM Value to PC
int transmitPM10(unsigned char *thebuf)
{
  int PM10Val;
  PM10Val = ((thebuf[7] << 8) + thebuf[8]); //count PM10 value of the air detector module
  return PM10Val;
}


void collectPMData()
{

  Serial.println("Starting PM Sensor Serial");

  PMSerial.begin(9600);
  delay(2000);

  if (PMSerial.find(0x42))
  {
    PMSerial.readBytes(buf, LENG);
    if (buf[0] == 0x4d)
    {
      if (checkValue(buf, LENG))
      {
        PM01Value = transmitPM01(buf); //count PM1.0 value of the air detector module
        PM2_5Value = transmitPM2_5(buf); //count PM2.5 value of the air detector module
        PM10Value = transmitPM10(buf); //count PM10 value of the air detector module
      }
    }
  }
  if (millis() - OledTimer >= 1000)
  {
    OledTimer = millis();

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
  Serial.println("Received Data from PM Sensor. Terminating Serial Communication");
  PMSerial.end();
}

void collectDHTData()
{
  H = dht.readHumidity();
  Serial.print("Humidity: ");
  Serial.print(H);
  Serial.println("%");
}


void setup() {
  // put your setup code here, to run once:

  //Initialize Serial for PC
  Serial.begin(9600);

  // Initialize DHT22
  dht.begin();

  // Initialize BMP Sensor
  if (!bmp.begin()) {            // you prefer
    delay(1000);
    Serial.println("Initializing for BMP Sensor failed.");
    delay(1000);
    Serial.println("Check wiring.");
    while (1);
  }
  else
    Serial.println("Init. OK.");
  bmp.setOversampling(4);
  delay(10000);

}

void MiCS()
{
  // read NO2 sensor:
  Vout = analogRead(NO2)/409.2; // take reading and convert ADC value to voltage
  Rs = 22000/((5/Vout) - 1);   // find sensor resistance from Vout, using 5V input & 22kOhm load resistor
  ppbNO2 = (.000008*Rs - .0194)*1000;    //convert Rs to ppb concentration NO2 (equation derived from data found on http://a...content-available-to-author-only...i.es/sensors.php
  Serial.print("PPB NO2= ");
  Serial.println(ppbNO2);
  // read CO sensor:
  Vout = analogRead(CO)/409.2; // take reading and convert ADC value to voltage
  Rs = 100000/((5/Vout) - 1);   // find sensor resistance from Vout, using 5V input & 100kOhm load resistor
  ppmCO = 911.19*pow(2.71828,(-8.577*Rs/100000));    //convert Rs to ppm concentration CO
  Serial.print("PPM CO= ");
  Serial.println(ppmCO);
}

void loop() {
  // put your main code here, to run repeatedly:
  collectPressureData();
  collectPMData();
  MiCS();
  collectDHTData();
  sendGSM();
  delay(20000);

}


void sendGSM()
{
  Serial.println("--------------------------------------------------");
  Serial.println("Now GSM is sending the data");
  GSMSerial.begin(9600);
  delay(1000);
//  GSMSerial.println("AT");
//  GSMSerial.println("AT+CSCLK=0\r\n");
//  delay(1000);
//  GSMSerial.println("AT");
//  delay(1000);
//  GSMSerial.println("AT+CSCLK=0\r\n");
//  delay(1000);
//  GSMSerial.println("AT+CSCLK?\r\n");
  delay(1000);
  GSMSerial.println("AT+CGATT?");
  delay(1000);
  toSerial();
  GSMSerial.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");
  delay(1000);
  toSerial();
  GSMSerial.println("AT+SAPBR=3,1,\"APN\",\"airtelgprs.com\"");
  delay(1000);
  toSerial();
  GSMSerial.println("AT+SAPBR=1,1");
  delay(1000);
  toSerial();
  GSMSerial.println("AT+SAPBR=2,1");
  delay(1000);
  toSerial();
  GSMSerial.println("AT+HTTPINIT");
  delay(1000);
  toSerial();
//  GSMSerial.print("AT+HTTPPARA=\"URL\",\"http://landslidemonitoring.esy.es/db_insertion.php?api_key=3WU63XFVOKEC1VBM&triplet=t1&t1s1=10,80&t1s2=1013.48&t1s3=0&t1s4=278&t1s5=0.67,0.44,0.92,0,10,10,40,50,0&t1s6=1&t1s7=1");
  GSMSerial.print("AT+HTTPPARA=\"URL\",\"pollution-monitor.herokuapp.com/?t=");
  Serial.print("AT+HTTPPARA=\"URL\",\"pollution-monitor.herokuapp.com/?t=");
  GSMSerial.print(T);
  Serial.print(T);
  GSMSerial.print("&p=");
  Serial.print("&p=");
  GSMSerial.print(P);
  GSMSerial.print("&co=");
  Serial.print("&co=");
  GSMSerial.print(ppmCO);
  GSMSerial.print("&no2=");
  Serial.print("&no2=");
  GSMSerial.print(ppbNO2);
  Serial.print(P);
  GSMSerial.print("&pm01=");
  Serial.print("&pm01=");
  GSMSerial.print(PM01Value);
  Serial.print(PM01Value);
  GSMSerial.print("&pm25=");
  Serial.print("&pm25=");
  GSMSerial.print(PM2_5Value);
  Serial.print(PM2_5Value);
  GSMSerial.print("&pm10=");
  Serial.print("&pm10=");
  GSMSerial.print(PM10Value);
  Serial.print(PM10Value);
  GSMSerial.print("&h=");
  Serial.print("&h=");
  GSMSerial.print(H);
  Serial.print(H);
  GSMSerial.println("\"");
  toSerial();
  delay(1000);

  GSMSerial.println("AT+HTTPACTION=0");
  delay(15000);
  toSerial();


  GSMSerial.println("AT+HTTPREAD");
  delay(2000);
  toSerial();
//  delay(2000);
//  toSerial();
//  delay(2000);
//  toSerial();

  GSMSerial.println("AT+HTTPTERM");
  

  delay(2000);
  toSerial();
//  delay(2000);
//  toSerial();
//  delay(2000);

  toSerial();

  toSerial();

  Serial.println("Sent and sleeping now:");
 
  delay(500);
  toSerial();
  GSMSerial.end();

}

void toSerial()
{

  while (GSMSerial.available() > 0)
  {
    Serial.write(GSMSerial.read());
  }
  Serial.println("---------------------");
}
