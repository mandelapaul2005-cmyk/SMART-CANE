#include <LiquidCrystal.h>

// LCD connections: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(A2, A3, 5, 4, 3, 2);

// Pin assignments - Original ground-level sensor
const int trigPin = 9;
const int echoPin = 8;

// Pin assignments - New chest-level sensor
const int trigPinChest = A4;
const int echoPinChest = A5;

const int waterSensor = A0;
const int ldrPin = A1;

const int motorPin = 10;
const int buzzerPin = 11;

const int whiteLED = 12;
const int greenLED = 6;
const int yellowLED = 7;

// Obstacle detection distance in centimetres
const int obstacleDistance = 80;

// Adjust this threshold according to your LDR readings
const int darknessThreshold = 500;

// Water sensor output: adjust if your sensor logic is reversed
const int waterDetectedLevel = LOW;

long duration;
long durationChest;
int distance;
int distanceChest;
int ldrValue;

bool obstacleDetected;
bool chestObstacleDetected;
bool liquidDetected;

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  pinMode(trigPinChest, OUTPUT);
  pinMode(echoPinChest, INPUT);

  pinMode(motorPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  pinMode(whiteLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);

  pinMode(waterSensor, INPUT);

  lcd.begin(16, 2);

  digitalWrite(trigPin, LOW);
  digitalWrite(trigPinChest, LOW);
  digitalWrite(motorPin, LOW);
  digitalWrite(buzzerPin, LOW);

  digitalWrite(whiteLED, LOW);
  digitalWrite(greenLED, LOW);
  digitalWrite(yellowLED, LOW);

  lcd.setCursor(0, 0);
  lcd.print("SMART CANE ON");
  delay(1500);
  lcd.clear();
}

void loop() {
  // --- 1. Read Original Ultrasonic Sensor (Ground/Lower level) ---
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH, 30000);
  if (duration == 0) {
    distance = 999; 
  } else {
    distance = duration * 0.0343 / 2;
  }

  // --- 2. Read New Chest-Level Ultrasonic Sensor ---
  digitalWrite(trigPinChest, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPinChest, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPinChest, LOW);

  durationChest = pulseIn(echoPinChest, HIGH, 30000);
  if (durationChest == 0) {
    distanceChest = 999; 
  } else {
    distanceChest = durationChest * 0.0343 / 2;
  }

  // --- 3. Read Water & Light Sensors ---
  int waterValue = digitalRead(waterSensor);
  ldrValue = analogRead(ldrPin);

  // --- 4. Determine Detected Conditions ---
  obstacleDetected = (distance > 0 && distance <= obstacleDistance);
  bool chestObstacle = (distanceChest > 0 && distanceChest <= obstacleDistance);
  liquidDetected = (waterValue == waterDetectedLevel);

  // Automatic night light & Day/Night condition
  bool isNight = (ldrValue < darknessThreshold);
  if (isNight) {
    digitalWrite(whiteLED, HIGH); // Turn on white light during darkness
  } else {
    digitalWrite(whiteLED, LOW);  // Turn off white light during day
  }

  // Reset warning outputs (except buzzer/motor which will be handled per state)
  digitalWrite(greenLED, LOW);
  digitalWrite(yellowLED, LOW);
  digitalWrite(motorPin, LOW);
  digitalWrite(buzzerPin, LOW);

  // --- 5. Display & Feedback Logic ---

  // NORMAL / IDLE STATE
  if (!obstacleDetected && !chestObstacle && !liquidDetected) {
    lcd.setCursor(0, 0);
    // Shows DAY or NIGHT on the top line
    if (isNight) {
      lcd.print("NIGHT (LED ON)  ");
    } else {
      lcd.print("DAY (LED OFF)   ");
    }
    
    lcd.setCursor(0, 1);
    lcd.print("Path is clear   ");

    digitalWrite(greenLED, HIGH);
    delay(100); // Standard loop delay when safe
  }

  // CHEST-LEVEL OBSTACLE ONLY
  else if (!obstacleDetected && chestObstacle && !liquidDetected) {
    lcd.setCursor(0, 0);
    lcd.print("CHEST OBSTACLE! ");
    lcd.setCursor(0, 1);
    lcd.print("Dist: ");
    lcd.print(distanceChest);
    lcd.print("cm   ");

    digitalWrite(yellowLED, HIGH);
    digitalWrite(motorPin, HIGH);
    
    // Buzzer ON longer (louder/fuller tone) and slowed down
    digitalWrite(buzzerPin, HIGH);
    delay(400);  // Beep duration 
    digitalWrite(buzzerPin, LOW);
    delay(400);  // Pause between beeps 
  }

  // LOWER OBSTACLE DETECTED
  else if (obstacleDetected && !chestObstacle && !liquidDetected) {
    lcd.setCursor(0, 0);
    lcd.print("OBSTACLE DETECTED");
    lcd.setCursor(0, 1);
    lcd.print("Dist: ");
    lcd.print(distance);
    lcd.print("cm   ");

    digitalWrite(yellowLED, HIGH);
    digitalWrite(motorPin, HIGH);
    
    digitalWrite(buzzerPin, HIGH);
    delay(400); 
    digitalWrite(buzzerPin, LOW);
    delay(400); 
  }

  // LIQUID DETECTED ONLY
  else if (!obstacleDetected && !chestObstacle && liquidDetected) {
    lcd.setCursor(0, 0);
    lcd.print("LIQUID DETECTED ");
    lcd.setCursor(0, 1);
    lcd.print("Caution: Water  ");

    digitalWrite(yellowLED, HIGH);
    digitalWrite(motorPin, HIGH); 
    
    digitalWrite(buzzerPin, HIGH);
    delay(400); 
    digitalWrite(buzzerPin, LOW);
    delay(400); 
  }

  // MULTIPLE HAZARDS (Obstacle + Liquid or Multiple Obstacles)
  else {
    lcd.setCursor(0, 0);
    lcd.print("MULTIPLE HAZARD ");
    lcd.setCursor(0, 1);
    lcd.print("DANGER!         ");

    digitalWrite(yellowLED, HIGH);
    digitalWrite(motorPin, HIGH);
    
    // Faster, urgent beeps for multiple hazards
    digitalWrite(buzzerPin, HIGH);
    delay(200); 
    digitalWrite(buzzerPin, LOW);
    delay(100); 
  }
}