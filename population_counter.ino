#include <Arduino.h>
#include <EEPROM.h>

// How often to poll in miliseconds
#define PERIOD 600000

int incrButton = 11;
int decrButton = 12;
int pin0 = 6;
int pin1 = 10;
int pin2 = 9;
int pin3 = 7;

byte count = 0;
byte maxCountForPeriod = 0;
byte lastCount = 0;

long lastTime; // Only used to prevent switch bounce

short periodDiff;    // Number of polls that have passed without a change in count
short currentPeriod; // Short for storing the overall current period

short address; // addresses to write to on EEPROM

void setup() {
  Serial.begin(9600);
  pinMode(incrButton, INPUT);
  pinMode(decrButton, INPUT);
  pinMode(pin0, OUTPUT);
  pinMode(pin1, OUTPUT);
  pinMode(pin2, OUTPUT);
  pinMode(pin3, OUTPUT);

  readaddress();
  periodDiff = 250; // Sentinel value that will be printed if turned off and back on
  lastTime = millis();
  lastCount = -1;
  currentPeriod = 0;

  // start();
  /* 
   * Uncomment the above line to reset the counter
   * Run the sketch after uncommenting the line to reset
   * Then recomment the line and run to resume recording
   */
   
  Serial.print("address is ");
  Serial.print(address);
  Serial.print("\n");
}

void loop() {
  if(!digitalRead(decrButton) && !digitalRead(incrButton) && millis() > lastTime + 500) {
    read();
    lastTime = millis();
  }
  else if (!digitalRead(incrButton)) {
    if ((millis() > lastTime + 500)) {
      lastTime = millis();
      count++;
      if (count > maxCountForPeriod) {
        maxCountForPeriod = count;
      }
      printCount();
    }
  } else if (!digitalRead(decrButton)) {
    if ((millis() > lastTime + 500)) {
      lastTime = millis();
      if (count > 0) count--;
      printCount();
    }
  }
  

  // Check for a new period
  if (((millis() / PERIOD) % 256) != currentPeriod) {
    log();
    currentPeriod = (millis() / PERIOD) % 256;
  }
}

void log() {
  if (lastCount != maxCountForPeriod) {
    // Because periodDiff might get as large as 256, meaning ~1 day of inactivity
    // and count will not go over 15, we give 4 additional bits to periodDiff
    // The following three lines are to move the 4 most significant bits of
    // periodDiff to the count byte
    byte firstByte = periodDiff & 0xFF;
    byte secondByte = (periodDiff & 0xF00) >> 4;
    secondByte += maxCountForPeriod & 0xF;
    EEPROM.write(address, firstByte);
    EEPROM.write(address + 1, secondByte);

    address += 2;
    if (address == 0) address = 2;
    logaddress();
    lastCount = maxCountForPeriod;
    maxCountForPeriod = count;
    periodDiff = 0;
  } else {
    maxCountForPeriod = count;
    periodDiff++;
  }

}

void read() {
  Serial.println("-----------------------");
  for (int i = 0; i < address; i += 2) {
    // For decoding the work of log()
    short readTimeDiff = EEPROM.read(i);
    readTimeDiff += (EEPROM.read(i + 1) & 0xF0) << 4;
    byte readCount = EEPROM.read(i + 1) & 0xF;
    Serial.print(">> ");
    Serial.print(readTimeDiff);
    Serial.print("\t");
    Serial.print(readCount);;
    Serial.print("\n");
  }
}

void logaddress() {
  // Write the current value of the address so turning off
  // won't lose our place logging
  EEPROM.write(0, ((address >> 8) & 0xFF));
  EEPROM.write(1, (address & 0xFF));
}

void readaddress() {
  address = EEPROM.read(0) << 8;
  address += EEPROM.read(1);
}

void printCount() {
  Serial.println(count);
  byte toPrint = count;

  // Normally 0 will display nothing on the 7-segment now
  // displays a '-' instead by replacing the values used
  if (count == 0) {
    Serial.println("Is now 0");
    toPrint = 10;
  } else if (count == 10) {
    Serial.println("Is now 15");
    toPrint = 15;
  } else if (count == 15) {
    Serial.println("Is now 0");
    toPrint = 0;
  }
  if (toPrint & 1) digitalWrite(pin0, HIGH);
  else digitalWrite(pin0, LOW);
  if (toPrint & 2) digitalWrite(pin1, HIGH);
  else digitalWrite(pin1, LOW);
  if (toPrint & 4) digitalWrite(pin2, HIGH);
  else digitalWrite(pin2, LOW);
  if (toPrint & 8) digitalWrite(pin3, HIGH);
  else digitalWrite(pin3, LOW);
}

// Run to start the counter from the beginning
void start() {
  address = 2;
  currentPeriod = 0;
  periodDiff = 0;
  logaddress();
}
