/**
 * Cluck Bucket, Drew Bent © 2013
 *
 * Displays time on Parallax 2x16 Serial LCD (w/ Speaker).
 * 
 * + Uses the format: 9:22:15 Fri 19 Oct 2012
 * + Plays Ode to Joy when turned on.
 * + Saves data to EEPROM (currently only the time)
 * + Utilizes Metro timing features
 * + Displays temperature from SparkFun kit sensor
 * + Utilizes push buttons
 *
 **/
 
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h> // digital temperature sensor
#include "RTClib.h" // real time clock module
#include <Time.h> // date/time format
#include <EEPROM.h> // storage
#include <Metro.h> // recurring timed events
#include <Servo.h> // servo

#define BAUD_RATE  19200
#define BACKLIGHT  true

#define CURSOR     false
#define BLINK      false

// http://www.arduino.cc/playground/Code/EEPROMLoadAndSaveSettings
// ID of the settings block
#define CONFIG_VERSION "cb1"
// Tell it where to store your config data in EEPROM
#define CONFIG_START 32

const int sensorPin = 1; // analog
//const int SDA = 4; // analog
//const int SCL = 5; // analog

const int pushButton1Pin = 2; // digital
const int pushButton2Pin = 3; // digital
const int temperaturePin = 8; // digital

const int servoPin = 17; // analog 3
Servo myservo;
int servoVal = 0;

const int ledPin = 13; // digital

const int customCharDegree = 1; // DEC

float temperatureF;
OneWire oneWire(temperaturePin);
DallasTemperature sensors(&oneWire);

int configStage;

boolean buttonWasReleased;

Metro metroUpdateDisplay = Metro(100);
Metro metroAlarm = Metro(172800000); // 20 days = 1000*60*60*24*2 seconds
Metro metroTemperature = Metro(1000);
Metro metroTurnEggs = Metro(5000); // Turn eggs every five seconds
Metro metroRotateServo = Metro(50);

boolean turningEggs = true;

RTC_DS1307 RTC;

typedef enum
{
  SYSTEM_CONFIG = 0,
  HATCHING_STAGE_ONE, //eggs need turning and no hatching detection
  HATCHING_STAGE_TWO //no egg turning but detecting hatching progress
} 
SystemStage;
SystemStage systemStage = SYSTEM_CONFIG;

struct SavedData {
  char version[4];
  int incubationDays;
  float temperature; // Fahrenheit
  int humidity; // RH%
} savedData = {
  CONFIG_VERSION,
  21,
  99.5,
  45
};

void setup() {
  Serial.begin(BAUD_RATE);
  Wire.begin();
  RTC.begin();
  
  //alarm();
  loadConfig();
  initializeCustomChars();
  initializePins();
  initializeServo();
  initializeTime();
  initializeDisplay(); 
}


void loop()
{
  generalLoop();
    
  switch(systemStage) {
    case SYSTEM_CONFIG: 
      systemConfig();
      break;
    case HATCHING_STAGE_ONE:
      hatchingStageOne();
      break;  
    case HATCHING_STAGE_TWO:
      hatchingStageTwo();
      break;
  }
}

// Code that applies to all system stages.
void generalLoop() {
  if (temperatureF > savedData.temperature + 20) {
    digitalWrite(ledPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
  }
}

// TODO(drew): Separate display and input logic.
void systemConfig() {
  if (metroUpdateDisplay.check() == 1) {
    clearLCD();
    
    // Flaot between 0 and 1
    float sensorValue = getVoltage(sensorPin) / 5;
    
    if (configStage == 0) {
      Serial.print("Config? ");

      if (sensorValue >= (3.0 / 5)) {
        Serial.print("yes");
      } else {
        Serial.print("no");  
      }
      
      if (buttonWasReleased) {
        if (sensorValue >= (3.0 / 5)) {
          buttonWasReleased = false;
          configStage += 1;
        } else {
          systemStage = HATCHING_STAGE_ONE;
        }
      }
      
      // Infrared motion sensor
      /**if (digitalRead(8) == LOW) {
        Serial.print(" *");
      }**/
    }
    
    else if (configStage == 1) {
      Serial.print("# of Days? ");
      
      int dayNumber = sensorValue * 30 + 20;
            
      Serial.print(dayNumber);
      
      if (buttonWasReleased) {
        buttonWasReleased = false;
        savedData.incubationDays = dayNumber;
        configStage += 1;
      }
    }
    
    else if (configStage == 2) {
      Serial.print("Temp? ");
      
      float temperature = sensorValue * 2.5 + 97.5;
            
      Serial.print(temperature);
      
      if (buttonWasReleased) {
        savedData.temperature = temperature;
        configStage += 1;
        buttonWasReleased = false;
      }
    }
    
    else if (configStage == 3) {
      Serial.print("Humidity? ");
      
      int humidity = sensorValue * 30 + 20;
            
      Serial.print(humidity);
      
      if (buttonWasReleased) {
        buttonWasReleased = false;
        savedData.humidity = humidity;
        configStage += 1;
      }
    }
    
    else {
      systemStage = HATCHING_STAGE_ONE;
    }
  }
}

void hatchingStageOne() {
 if (metroUpdateDisplay.check() == 1) {
    clearLCD();
    digitalClockDisplay();
  }
  if (metroAlarm.check() == 1) {
    /alarm();
  }
  if (metroTemperature.check() == 1) {
    updateTemperatureVar(); 
  } 
  
  if (metroTurnEggs.check() == 1) {
    turningEggs = true;  
  }
  if (turningEggs && metroRotateServo.check() == 1) {
    servoVal += 1;
    if (servoVal >= 179) {
      servoVal = 0;
      turningEggs = false;
    }
    myservo.write(servoVal); 
    Servo::refresh();
  }
}

void hatchingStageTwo() {
  
}

void initializeTime() {
  if (!RTC.isrunning()) {
    // To set RTC time according to computer time
    //RTC.adjust(DateTime(__DATE__, __TIME__));
  }  
}

void initializeCustomChars() {
  // DEGREE (º) symbol
  Serial.write(248 + customCharDegree);
  Serial.write((byte)B00111);
  Serial.write((byte)B00101);
  Serial.write((byte)B00111);
  Serial.write((byte)00000);
  Serial.write((byte)B00000);
  Serial.write((byte)B00000);
  Serial.write((byte)B00000);
  Serial.write((byte)B00000); 
}

void initializePins() {
  pinMode(ledPin, OUTPUT);
  
  pinMode(pushButton1Pin, INPUT);
  pinMode(pushButton2Pin, INPUT);
    
  attachInterrupt(0, buttonReleased, RISING);
  attachInterrupt(1, buttonReleased, RISING);
}

void initializeServo() {
  pinMode(3, OUTPUT);
  myservo.attach(servoPin);
}

void initializeDisplay() {
  // Wait 100ms before initializing and between all consequent configurations.
  delay(100);

  clearLCD();
  
  delay(100);

  if (BACKLIGHT) {
    Serial.write(17);  
  } else {
    Serial.write(18);  
  }
  
  delay(100);
  
  if (CURSOR && BLINK) {
    Serial.write(25);
  } else if (CURSOR && !BLINK) {
    Serial.write(24);
  } else if (!CURSOR && BLINK) {
    Serial.write(23);  
  } else {
    Serial.write(22);  
  }
  
  delay(100);
}

void clearLCD() {
  Serial.write(12);
  delay(5);
}

void newLine() {
  Serial.write(13);
}

void space() {
  // Same as: Serial.print(" ");
  Serial.write(9);
}

void digitalClockDisplay(){
  //time_t t = now();
  DateTime now = RTC.now();
  
  Serial.print(dayShortStr(now.dayOfWeek2()));
  Serial.print(", ");
  Serial.print(monthShortStr(now.month()));
  space();
  Serial.print(now.day());
  newLine();
  Serial.print(now.hour());
  Serial.print(':');
  printFormattedNumber(now.minute());
  Serial.print(':');
  printFormattedNumber(now.second());
  space();
  Serial.print(temperatureF);
  Serial.write(customCharDegree);
}

void printFormattedNumber(int number) {
  // Add preceeding 0 if appropriate for clock.
  if(number < 10) {
    // Add 0 if number is less than 2 characters long.
    Serial.print(0);
  }
  Serial.print(number);
}

void loadConfig() {
  // To make sure there are settings, and they are YOURS!
  // If nothing is found it will use the default settings.
  if (EEPROM.read(CONFIG_START + 0) == CONFIG_VERSION[0] &&
      EEPROM.read(CONFIG_START + 1) == CONFIG_VERSION[1] &&
      EEPROM.read(CONFIG_START + 2) == CONFIG_VERSION[2])
    for (unsigned int t=0; t<sizeof(savedData); t++)
      *((char*)&savedData + t) = EEPROM.read(CONFIG_START + t);
}

void saveConfig() {
  for (unsigned int t=0; t<sizeof(savedData); t++) {
    EEPROM.write(CONFIG_START + t, *((char*)&savedData + t));
  }
}

void alarm() {
  Serial.write(21); // A = 440 Hz

  Serial.write(212); // 1/4 note
  
  // E, E, F, G, G, F, E, D, C, C, D, E
  int notes[] = {227, 227, 228, 230, 230, 228, 227, 225, 223, 223, 225, 227};
  for (int i=0; i < sizeof(notes) / sizeof(int); i++) {
    Serial.write(notes[i]);
  }
  
  // E, D, D
  Serial.write(213); // 1/2 note
  Serial.write(227);
  Serial.write(211); // 1/8 note
  Serial.write(225);
  Serial.write(212); // 1/4 note
  Serial.write(225);
}

// TODO(drew): This function seems to be finicky
void buttonReleased() {
  buttonWasReleased = true;
}

// HELPER FUNCTIONS
int length(int arr[]) {
  return (sizeof(arr) / sizeof(int));  
}

// TEMPERATURE
float getVoltage(int pin)
{
  return analogRead(pin) * 0.004882814;
}

void updateTemperatureVar() {
  sensors.requestTemperatures();
  temperatureF = sensors.getTempFByIndex(0);
  // TODO(drew): Do we still need this analog temp code
  //float voltage, degreesC, degreesF;
  //voltage = getVoltage(temperaturePin);
  //degreesC = (voltage - 0.5) * 100.0;
  //degreesF = degreesC * (9.0/5.0) + 32.0; 
  
  //temperatureF = degreesF;
}
