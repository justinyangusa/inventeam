//Purpose: To test 4 DS18B20 digital hot water sensors
//Connection:
//1. the pull up resister (4.7K ohm) must be used.
//http://bildr.org/2011/07/ds18b20-arduino/
//2. download OneWrie Library 
//http://www.pjrc.com/teensy/td_libs_OneWire.html
//3. check scheme for sensor
//http://pdf1.alldatasheet.com/datasheet-pdf/view/58557/DALLAS/DS18B20.html
//4. RealTime clock--DS1307
//https://www.sparkfun.com/products/99
//http://wiring.org.co/learning/libraries/realtimeclock.html
//5. LCD(Drew and Andy have LCD)
//link
//6. microSD shield
//https://www.sparkfun.com/products/9802
//https://www.sparkfun.com/tutorials/172
//Gunn InvenTeam


#include <OneWire.h> 


//solar water heater testing plan
//T1 for outdoor temp
//T2 for solar water heater input side temp
//T3 for solar water heater output side temp
//T4 for water tank temp
//save the data every 10 minutes to SD card
//lines of data:
//6 lines/h x 24h/day = 144 lines
//microSD holds 8GB


//unused pin: 6-13 
//no enough pin to use, we have to move to Mega from Uno board
int DS18B20_Pin1 = 2; //DS18D20 Signal pin on digital 2
int DS18B20_Pin2 = 3; //DS18D20 Signal pin on digital 3
int DS18B20_Pin3 = 4; //DS18D20 Signal pin on digital 4
int DS18B20_Pin4 = 5; //DS18D20 Signal pin on digital 5


OneWire ds1(DS18B20_Pin1); //T1 on digital pin2
OneWire ds2(DS18B20_Pin2); //T2 on digital pin3
OneWire ds3(DS18B20_Pin3); //T1 on digital pin4
OneWire ds4(DS18B20_Pin4); //T2 on digital pin5


//Real Time Clock pin -- AndrewH


//LCD pin -- AndrewS


//SD pin -- Ken


void setup(void) {
  Serial.begin(9600);
  Serial.println( "Solar Water Heater Temperatures");
  //Real Time Clock--- AndrewH
}
 
void loop(void) {
 
  //Print real time--- AndrewH
  //Serial.print(realtime);
  //Serial.print("  ");
  
  float T1_temperature = getTemp(ds1);
  float T2_temperature = getTemp(ds2);
  float T3_temperature = getTemp(ds3);
  float T4_temperature = getTemp(ds4);
   //prints in celsius
  Serial.print(T1_temperature);
  Serial.print("  "); 
  Serial.print(T2_temperature);
  Serial.print("  ");
  Serial.print(T3_temperature);
  Serial.print("  ");
  Serial.println(T4_temperature);
  
  //LCD real time display 4 temperature---AndrewS 
  //SD save realtime and 4 temperature---Ken
  
  //1000 == 1 sec
  //60 x 1000 = 1 minute
  //10 minutes = 10 x 60 x 1000 = 600000
  //delay(600000); //collect temperature every 10 minutes
  delay(6000); //for code testing
}


//Pls don't change the following code
float getTemp( OneWire & dsx){
 //returns the temperature from one DS18D20 in DEG Celsius


  byte data[12];
  byte addr[8];
 
  if ( !dsx.search(addr)) {
    //no more sensors on chain, reset search
    dsx.reset_search();
    return -1000;
  }


  if( OneWire::crc8( addr, 7) != addr[7]) {
    Serial.println("CRC is not valid!");
    return -1111;
  }


  if ( addr[0] != 0x10 && addr[0] != 0x28) {
    Serial.print("Device is not recognized");
    return -2222;
  }


  dsx.reset();
  dsx.select(addr);
  dsx.write(0x44,1); // start conversion, with parasite power on at the end


  byte present = dsx.reset();
  dsx.select(addr);  
  dsx.write(0xBE); // Read Scratchpad


  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = dsx.read();
  }
 
  dsx.reset_search();
 
  byte MSB = data[1];
  byte LSB = data[0];


  float tempRead = ((MSB << 8) | LSB); //using two's compliment
  float TemperatureSum = tempRead / 16;
 
  return TemperatureSum; 
}
