#include <Adafruit_Fingerprint.h>
#include <Servo.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(7, 6, 3, 30, 31, 32);

// Initialize the servo motor
Servo myservo;

const int buzzerPin = 37;  // Buzzer pin

const int redPin = 8;    // Red LED pin
const int greenPin = 9; // Green LED pin
const int bluePin = 12;  // Blue LED pin (assuming common anode RGB LED)

#define RXPIN 10  // change this to whatever
#define TXPIN 11  // change this to whatever
SoftwareSerial mySerial(RXPIN, TXPIN);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

uint8_t storedID = 0;

void setup()
{
  // Initialize serial communication
  Serial.begin(9600);
  while (!Serial)
    delay(1); // Wait until serial port is opened

  // Initialize the fingerprint sensor
  finger.begin(57600);
  lcd.begin(16, 2);
  pinMode(buzzerPin, OUTPUT);

  // Verify the connection to the fingerprint sensor
  if (finger.verifyPassword())
  {
    Serial.println("Fingerprint sensor found!");
  }
  else
  {
    Serial.println("Couldn't find fingerprint sensor :(");
    while (1) {} // Halt execution
  }
   myservo.attach(13);
   pinMode(redPin, OUTPUT);
   pinMode(greenPin, OUTPUT);
   pinMode(bluePin, OUTPUT);
  // Prompt the user to enter the ID of the stored fingerprint
  
}

void loop()
{
  lcd.clear();
 Serial.println("Enter the fingerprint ID (1-127):");

while (Serial.available() == 0) {
  // Wait for user input
}

// Read the user input
int enteredID = Serial.parseInt();

// Check if the entered fingerprint ID is valid
if (enteredID == 0 || enteredID > 127) {
  Serial.println("Invalid fingerprint ID. Please enter a value between 1 and 127.");
  while (Serial.available()) {
    Serial.read(); // Clear any remaining characters from the input buffer
  }
  return; // Exit the function
}

// Capture the fingerprint image
lcd.print("Put your finger on scanner");
delay(2000);
uint8_t p = finger.getImage();
lcd.clear();
switch (p) {
  case FINGERPRINT_OK:
    Serial.println("Image taken");
    break;
  case FINGERPRINT_NOFINGER:
    Serial.println("No finger detected");
    return;
  case FINGERPRINT_PACKETRECIEVEERR:
    Serial.println("Communication error");
    return;
  case FINGERPRINT_IMAGEFAIL:
    Serial.println("Imaging error");
    return;
  default:
    Serial.println("Unknown error");
    return;
}

// Convert the image to a fingerprint template
p = finger.image2Tz();
switch (p) {
  case FINGERPRINT_OK:
    Serial.println("Image converted");
    break;
  case FINGERPRINT_IMAGEMESS:
    Serial.println("Image too messy");
    return;
  case FINGERPRINT_PACKETRECIEVEERR:
    Serial.println("Communication error");
    return;
  case FINGERPRINT_FEATUREFAIL:
    Serial.println("Could not find fingerprint features");
    return;
  case FINGERPRINT_INVALIDIMAGE:
    Serial.println("Could not find fingerprint features");
    return;
  default:
    Serial.println("Unknown error");
    return;
}

// Search for a matching fingerprint ID
int matchID = finger.fingerID;
int confidence = 0;
p = finger.fingerFastSearch();
if (p == FINGERPRINT_OK) {
  // Match found
  matchID = finger.fingerID;
  confidence = finger.confidence;
}

// Check if the captured fingerprint matches the entered ID
if (matchID == enteredID) {
   
  lcd.print("Fingerprint matched!");
  Serial.print("Entered ID: ");
  Serial.println(enteredID);
  Serial.print("Matched ID: ");
  Serial.println(matchID);
  Serial.print("Confidence: ");
  Serial.println(confidence);
  turnServo();
  
} else {
  lcd.print("Not matched!");
  turnBuzzer();
  turnRed();
}
}

void turnServo() {
 myservo.write(0); 
 digitalWrite(redPin, LOW);
  digitalWrite(greenPin, HIGH);
  digitalWrite(bluePin, LOW);      // Move the servo to 0 degrees
  delay(3000);            // Delay for 1 second
  myservo.write(180); 
  digitalWrite(redPin, LOW);
  digitalWrite(greenPin, LOW);
  digitalWrite(bluePin, LOW);     // Move the servo to 180 degrees
  delay(3000); 
}    

void turnRed(){
  digitalWrite(redPin, HIGH);
  digitalWrite(greenPin, LOW);
  digitalWrite(bluePin, LOW);
  delay(3000);
  digitalWrite(redPin, LOW);
  digitalWrite(greenPin, LOW);
  digitalWrite(bluePin, LOW);
}

void turnGreen(){
  digitalWrite(redPin, LOW);
  digitalWrite(greenPin, HIGH);
  digitalWrite(bluePin, LOW);
  delay(3000);

}

void turnBuzzer(){
  digitalWrite(buzzerPin, HIGH);
  digitalWrite(redPin, HIGH);
  digitalWrite(greenPin, LOW);
  digitalWrite(bluePin, LOW);
  delay(500);  // Buzz for 1 second

  // Turn off the buzzer
  digitalWrite(buzzerPin, LOW);
  // Pause for 1 second
}
