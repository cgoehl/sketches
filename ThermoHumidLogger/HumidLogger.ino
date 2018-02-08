// include the library code:
#include <LiquidCrystal.h>
#include <Wire.h>
#include <DS3231.h>
#include <SPI.h>
#include <SD.h>

DS3231 clock;
RTCDateTime now;

// initialize the library with the numbers of the interface pins
// Pin 4 is used for SD CS
LiquidCrystal lcd(3,5,6,7,8,9);

// for DHT11, 
//      VCC: 5V or 3V
//      GND: GND
//      DATA: 2
int pinDHT11 = 2;
#include <SimpleDHT.h>
SimpleDHT11 dht11;

struct DhtReading
{
    byte temperature;
    byte humidity;
};

char filename[13];

DhtReading dht;

void setup() {
  clock.begin();
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  log("Init...");
  log("Init SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(4)) {
    log("Failed: SD card");
    // don't do anything more:
    return;
  }
  now = clock.getDateTime();
  sprintf(filename, "%s", clock.dateFormat("ymdH.i", now));
  log("Filename: ");
  log(filename);
  delay(1000);
  log("READY! :)");
  update();
  appendCsv();
}

void log(const char* string) {
  lcd.clear();
  printLcd(0, string);
  delay(500);
}

void printLcd(int row, const char* string) {
  lcd.setCursor(0, row);
  lcd.print(string);
}

DhtReading readDht() {
  byte temperature = 0;
  byte humidity = 0;
  dht11.read(pinDHT11, &temperature, &humidity, NULL);
  DhtReading reading;
  reading.temperature = temperature;
  reading.humidity = humidity;
  return reading;
}

void appendCsv() {
  log("Writing...");
  File file = SD.open(filename, FILE_WRITE);
  if (!file) {
    log("Failed: Open");
    return;
  }
  file.print(clock.dateFormat("U", now));
  file.print(';');
  file.print(clock.dateFormat("ymd H:i:s", now));
  file.print(';');
  file.print(dht.temperature);
  file.print(';');
  file.print(dht.humidity);
  file.println();
  file.close();
  log("Write done");
}


void update() {
  now = clock.getDateTime();
  dht = readDht();
  char tempString[30];
  sprintf(tempString, "%dC %dH", dht.temperature, dht.humidity);
  lcd.clear();
  printLcd(0, tempString);
  printLcd(1, clock.dateFormat("ymd H:i:s", now));
  delay(1000);
}

uint8_t lastMinute = 255;
void loop() {
  update();
  if (now.minute != lastMinute && now.minute % 10 == 0) {
    appendCsv();
  }
  lastMinute = now.minute;
}

