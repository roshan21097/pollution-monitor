//#include <SPI.h>

//#include <LiquidCrystal.h> //Library for the  screen
#include <BMP280.h> // Library for the BMP280 sensor

BMP280 bmp; //Initialize your sensor
//LiquidCrystal (12, 11, 5, 4, 3, 2); /*Initialize your , make sure you wired it correctly */

#define P0 1013.25 //Standard atmospheric pressure
#define contrast 9 //9 and 10 are the pins where you wire the matching Serial pins
#define brightness 10 //for contrast and brightness regulation
#define ELEVATION 1189

float adjustPressure(float pressure, float elevation)
{
  return ((pressure)/pow((1-(ELEVATION)/44330), 5.255));
}


double T = 0; //Starting temperature value
double P = 0; //Starting pressure value
char measure = 0;

void collectData() {
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

void setup() {
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
    delay(2000);
    collectData();
}

void loop() {
  collectData();
  delay(2000);
}
