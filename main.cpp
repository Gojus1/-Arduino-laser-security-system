#include <Servo.h>
#include <SoftwareSerial.h>

SoftwareSerial bluetooth(11, 12); // RX, TX

Servo laserServo;
const int servoPin = 9;
const int laserPin = 7;
const int buzzerPin = 8;

const int ldrPins[3] = {A0, A1, A2};
const int zonePositions[3] = {30, 90, 150};
String zoneNames[3] = {"LEFT", "CENTER", "RIGHT"};

const int diodePin = 6;
const int transistorPin = 10;

const int requiredIterations = 5;
const int checkDelay = 200;
const int positionHoldTime = 2000;

int noLightCount = 0;
bool motorStopped = false;
int currentZone = 0;

void setup() {
  Serial.begin(9600);
  bluetooth.begin(9600);

  laserServo.attach(servoPin);
  pinMode(laserPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(laserPin, HIGH);
  pinMode(diodePin, OUTPUT);
  pinMode(transistorPin, OUTPUT);
  
  Serial.println("=== BLUETOOTH LASER SCANNER ===");
  bluetooth.println("Security System Ready");
  bluetooth.println("Commands: STATUS, RESET");
}

void loop() {
  checkBluetooth();
  
  if (motorStopped) {
    checkForLightRecovery();
    return;
  }
  
  for (currentZone = 0; currentZone < 3; currentZone++) {
    if (motorStopped) break;
    checkCurrentZoneOnly();
  }
}

void checkBluetooth() {
  if (bluetooth.available()) {
    String command = bluetooth.readString();
    command.trim();
    
    if (command == "STATUS") {
      bluetooth.println("System: RUNNING");
      bluetooth.print("Zone: ");
      bluetooth.println(zoneNames[currentZone]);
    } else if (command == "RESET") {
      motorStopped = false;
      noLightCount = 0;
      bluetooth.println("System: RESET");
    } else {
      bluetooth.println("Unknown command");
    }
  }
}

void checkCurrentZoneOnly() {
  laserServo.write(zonePositions[currentZone]);
  
  Serial.print(">>> Scanning ");
  Serial.println(zoneNames[currentZone]);

  static unsigned long lastBTmessage = 0;
  if (millis() - lastBTmessage > 5000) {
    bluetooth.print("Scanning ");
    bluetooth.println(zoneNames[currentZone]);
    lastBTmessage = millis();
  }

  digitalWrite(diodePin, HIGH);
  delay(100);
  digitalWrite(diodePin, LOW);
  
  delay(500);
  
  unsigned long startTime = millis();
  noLightCount = 0;
  
  while (millis() - startTime < positionHoldTime && !motorStopped) {
    int ldrValue = analogRead(ldrPins[currentZone]);
    
    Serial.print("  ");
    Serial.print(zoneNames[currentZone]);
    Serial.print(": ");
    Serial.print(ldrValue);
    
    if (ldrValue <= 25) {
      noLightCount++;
      Serial.print(" X (");
      Serial.print(noLightCount);
      Serial.print("/");
      Serial.print(requiredIterations);
      Serial.println(")");

      if (noLightCount == 3) {
        bluetooth.println("ALERT: blockage in " + zoneNames[currentZone]);
      }
      
      digitalWrite(transistorPin, (noLightCount % 2 == 0));
      
      if (noLightCount >= requiredIterations) {
        triggerAlarm(zoneNames[currentZone]);
        motorStopped = true;
        return;
      }
    } else {
      noLightCount = 0;
      digitalWrite(transistorPin, LOW);
    }
    
    delay(checkDelay);
  }
}

void checkForLightRecovery() {
  Serial.println("SYSTEM PAUSED");

  digitalWrite(diodePin, HIGH);
  digitalWrite(transistorPin, HIGH);
  
  int ldrValue = analogRead(ldrPins[currentZone]);
  Serial.print("Checking ");
  Serial.print(zoneNames[currentZone]);
  Serial.print(": ");
  Serial.println(ldrValue);
  
  bluetooth.println("System PAUSED - Checking: " + zoneNames[currentZone]);
  
  if (ldrValue > 2) {
    noLightCount = 0;
    motorStopped = false;
    Serial.println("RESUMING");
    bluetooth.println("System RESUMING");

    digitalWrite(diodePin, HIGH);
    digitalWrite(transistorPin, HIGH);
    delay(200);
    digitalWrite(diodePin, LOW);
    digitalWrite(transistorPin, LOW);
    
    delay(1000);
  } else {
    Serial.println("Waiting...");
    bluetooth.println("Still blocked - waiting...");
    tone(buzzerPin, 600, 300);
    delay(700);
  }
  
  digitalWrite(diodePin, LOW);
  digitalWrite(transistorPin, LOW);
}

void triggerAlarm(String zoneName) {
  Serial.println("ALARM!");
  Serial.print("BREACH: ");
  Serial.println(zoneName);
  
  bluetooth.println("ALARM! Zone: " + zoneName);
  
  for (int i = 0; i < 10; i++) {
    digitalWrite(diodePin, HIGH);
    digitalWrite(transistorPin, HIGH);
    tone(buzzerPin, 1200, 300);
    delay(400);
    
    digitalWrite(diodePin, LOW);
    digitalWrite(transistorPin, LOW);
    delay(400);
    
    if (i % 3 == 0) {
      bluetooth.println("ALARM ACTIVE...");
    }
  }
}
