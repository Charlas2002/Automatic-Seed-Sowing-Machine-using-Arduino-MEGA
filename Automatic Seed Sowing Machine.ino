#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include <DHT.h>
SoftwareSerial ble_soft(3, 2);
// LiquidCrystal lcd(13, 12, 11, 10, 9, 8);
LiquidCrystal lcd(8, 9, 10, 11, 12, 13);
// #define ble ble_soft
#define ble Serial3
#define splash splash1

#define soil A0
// Motor control pins
int m1A = 4;
int m1B = 5;
int m2A = 6;
int m2B = 7;

int m1C = 27;
int m1D = 29;
int m2C = 31;
int m2D = 33;


#define t1 23
#define e1 25

#define DHTPIN 2       // Pin where the DHT11 is connected
#define DHTTYPE DHT11  // Type of DHT sensor used

DHT dht(DHTPIN, DHTTYPE);

float disp_t = 15;

int dis_fr;
int dis;

int dis_alert = 50;
int dis_min = 4;

int forwardStatus = 0;
String IncomingData = "";

int soil_r, soil_fn;

int vib_r;
void setup() {
  // initialize serial communications at 9600 bps:

  // Motor control pin configuration
  pinMode(m1A, OUTPUT);
  pinMode(m1B, OUTPUT);
  pinMode(m2A, OUTPUT);
  pinMode(m2B, OUTPUT);

  pinMode(m1C, OUTPUT);
  pinMode(m1D, OUTPUT);
  pinMode(m2C, OUTPUT);
  pinMode(m2D, OUTPUT);

  pinMode(soil, INPUT);

  // Initialize motors (stop initially)
  digitalWrite(m1A, LOW);
  digitalWrite(m1B, LOW);
  digitalWrite(m2A, LOW);
  digitalWrite(m2B, LOW);

  digitalWrite(m1C, LOW);
  digitalWrite(m1D, LOW);
  digitalWrite(m2C, LOW);
  digitalWrite(m2D, LOW);


  pinMode(t1, OUTPUT);  // Sets the trigPin as an Output
  pinMode(e1, INPUT);

  Serial.begin(9600);

  ble.begin(9600);
  dht.begin();

  LcDSet();

  delay(2000);
  lcd.clear();
}

void LcDSet() {
  lcd.begin(16, 2);
  splash(0, "Agricultural");
  splash(1, "Robot");
  delay(2000);
  lcd.clear();
}

void loop() {

  robo();
  getSensor();
}

void getSensor() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  soil_r = analogRead(soil);

  soil_fn = map(soil_r, 0, 1023, 100, 0);

  if (isnan(humidity) || isnan(temperature)) {
    lcd.setCursor(0, 0);
    lcd.print("DHT11 Error       ");
  }

  dis_fr = ultracm(t1, e1);


  if (forwardStatus) {
    if (dis_fr < dis_alert) {
      stopRobot();
      splash(1, "Object Detected");
    }

    digitalWrite(m1C, HIGH);
    digitalWrite(m1D, LOW);
    digitalWrite(m2C, HIGH);
    digitalWrite(m2D, LOW);
  } else {
    digitalWrite(m1C, LOW);
    digitalWrite(m1D, LOW);
    digitalWrite(m2C, LOW);
    digitalWrite(m2D, LOW);
  }
  disp_t += 0.5;
  if (disp_t >= 15) {
    lcd.setCursor(0, 0);
    lcd.print("H: ");
    lcd.setCursor(2, 0);
    if (humidity < 10) {
      lcd.print("0");
    }
    if (humidity < 100) {
      lcd.print("0");
    }
    lcd.print(humidity, 0);
    lcd.print(" %  ");

    lcd.setCursor(9, 0);
    lcd.print("T: ");
    lcd.setCursor(11, 0);
    lcd.print(temperature, 1);
    lcd.print("C ");

    lcd.setCursor(0, 1);
    lcd.print("D:   cm             ");
    String dis_str = String(dis_fr, DEC);
    while (dis_str.length() < 3) {
      dis_str = "0" + dis_str;
    }
    lcd.setCursor(2, 1);
    lcd.print(dis_str);

    lcd.setCursor(9, 1);
    lcd.print("S: ");
    lcd.setCursor(11, 1);
    if (soil_fn < 10) {
      lcd.print("0");
    }
    if (soil_fn < 100) {
      lcd.print("0");
    }
    lcd.print(soil_fn);
    lcd.print(" % ");

    String ble_out = "";

    ble_out += "Dis : ";
    ble_out += String(dis_fr);
    ble_out += " % , ";

    ble_out += " Soil : ";
    ble_out += String(soil_fn);
    ble_out += " % , ";

    ble_out += " Temperature : ";
    ble_out += String(temperature);
    ble_out += " C , ";

    ble_out += "Humidity : ";
    ble_out += String(humidity);
    ble_out += " % , ";
ble.println(ble_out);

    disp_t = 0;
  }

  delay(30);
}

void robo() {

  while (ble.available()) {  //Check if there is an available byte to read

    delay(10);            //Delay added to make thing stable
    char c = ble.read();  //Conduct a serial read
    //    Serial.println(c);
    if (c == '#') {
      break;  //Exit the loop when the # is detected after the word
    }
    IncomingData += c;  //Shorthand for IncomingData = IncomingData + c
  }
  if (IncomingData.length() > 0) {
    Serial.println(IncomingData);

    if (IncomingData == "*forward") {
      moveForward();

      //      left();
    } else if (IncomingData == "*backward") {
      movebackward();
      //      right();
    }
    //----------Turn On One-By-One----------//
    else if (IncomingData == "*right") {
      turnRight();
      //      forward();
      // left();
    } else if (IncomingData == "*left") {

      //right();
      turnLeft();
      //      backward();
    }
    else if (IncomingData == "*stop") {
      stopRobot();
    }
    IncomingData = "";
  }
  delay(10);
}
// Function to move the robot forward
void moveForward() {
  // Serial.println("Moving forward");
  splash(1, "Forward");
  ble.println("FORWARD");  // Indicate action over BLE
  digitalWrite(m1A, HIGH);
  digitalWrite(m1B, LOW);
  digitalWrite(m2A, HIGH);
  digitalWrite(m2B, LOW);
  forwardStatus = 1;
}
void movebackward() {
  // Serial.println("Moving backward");
  splash(1, "Backward");
  ble.println("BACKWARD");  // Indicate action over BLE
  digitalWrite(m1A, LOW);
  digitalWrite(m1B, HIGH);
  digitalWrite(m2A, LOW);
  digitalWrite(m2B, HIGH);
  forwardStatus = 0;
}

// Function to stop the robot
void stopRobot() {
  // Serial.println("Stopping");
  splash(1, "Stop");
  ble.println("STOP");  // Indicate action over BLE
  digitalWrite(m1A, LOW);
  digitalWrite(m1B, LOW);
  digitalWrite(m2A, LOW);
  digitalWrite(m2B, LOW);
  forwardStatus = 0;
}
// Function to turn left
void turnLeft() {
  // Serial.println("Turning left");
  splash(1, "Turning Left");
  ble.println("LEFT");  // Indicate action over BLE
  digitalWrite(m1A, LOW);
  digitalWrite(m1B, HIGH);
  digitalWrite(m2A, HIGH);
  digitalWrite(m2B, LOW);
  forwardStatus = 0;
}
// Function to turn right
void turnRight() {
  // Serial.println("Turning right");
  splash(1, "Turning Right");
  ble.println("RIGHT");  // Indicate action over BLE
  digitalWrite(m1A, HIGH);
  digitalWrite(m1B, LOW);
  digitalWrite(m2A, LOW);
  digitalWrite(m2B, HIGH);
  forwardStatus = 0;
}
int ultracm(int trigPin, int echoPin) {
  long duration;
  int distance;
  digitalWrite(trigPin, LOW);

  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2;
  if (distance > 200) {
    distance = 200;
  }
  if (distance < 4) {
    distance = 0;
  }
  return distance;
}
