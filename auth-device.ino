#include <LiquidCrystal.h>
#include <Keypad.h>
#include <Adafruit_Fingerprint.h>
#include <SPI.h>
#include <UIPEthernet.h>
#include <Servo.h>

static byte mymac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

// Destination server details
static byte serverIP[] = {192, 168, 0, 2};
static const int serverPort = 80;
static const char* serverPath = "/api/v1/devices/aio?";

static char responseBuffer[512];

static EthernetClient client;

LiquidCrystal lcd(7, 6, 3, 30, 31, 32);

// RXPIN is IN from sensor (GREEN wire)
// TXPIN is OUT from arduino  (WHITE wire)
#define RXPIN 10  // change this to whatever
#define TXPIN 11  // change this to whatever
SoftwareSerial mySerial(RXPIN, TXPIN);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

//uint8_t id;
int id = 0;

const byte ROWS = 4;
const byte COLS = 4;
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {22, 23, 24, 25};  // Connect to the row pinouts of the keypad
byte colPins[COLS] = {A0, A1, A2, A3};  // Connect to the column pinouts of the keypad
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

const int redPin = 8;    // Pin for the red channel
const int greenPin = 9; // Pin for the green channel
const int bluePin = 12;  // Pin for the blue channel

Servo myservo;

const int buzzerPin = 37;  // Buzzer pin

String response = "";
String userID = "";
String password = "";
bool isFirstPassword = true;
bool isEnteringUserID = true;
bool isFingerTaken = true;
bool isAutentificationComplete = false;
bool isFingerDone = false;


String r = "";
  // Variable to track enrollment completion


void setup() {
  // put your setup code here, to run once:
   Serial.begin(9600);
  while (!Serial);
  delay(100);
  lcd.begin(16, 2);
  lcd.print("Enter User ID:");

  finger.begin(57600);

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  myservo.attach(13);
  pinMode(buzzerPin, OUTPUT);


  if (finger.verifyPassword()) {
  } else {
    Serial.println("Fingerprint error");
    while (1) {
      delay(1);
    }
  }

   Ethernet.begin(mymac);
    delay(1000);

    Serial.println("Ethernet connected");
    Serial.print("IP address: ");
    Serial.println(Ethernet.localIP());

  
}

uint8_t readnumber(void) {
  uint8_t num = 0;

  while (num == 0) {
    while (!Serial.available())
      ;
    num = Serial.parseInt();
  }
  return num;
}

void resetVariables() {
  userID = "";
  password = "";
  
  isFingerTaken = true;
  isFirstPassword = true;
  isEnteringUserID = true;
  isAutentificationComplete = false;
  isFingerDone = false;
  response = "";
}

void displayInitialPrompt() {
  lcd.clear();
  lcd.print("Enter User ID:");
}

void loop() {
  // put your main code here, to run repeatedly:
  if (isAutentificationComplete) {
    // Reset variables and display initial prompts
    resetVariables();
    displayInitialPrompt();
    
  }

  char customKey = customKeypad.getKey();

  if (isEnteringUserID) {
    lcd.setCursor(0, 1);
    lcd.print(userID);
  } else if (isFirstPassword) {
    lcd.setCursor(0, 1);
    lcd.print(password);
  } 

  if (customKey) {
    if (customKey == '*') {
      if (isEnteringUserID) {
        userID = "";
      } else if (isFirstPassword) {
        password = "";
      } 
    } else if (customKey == '#') {
      if (isEnteringUserID) {
        isEnteringUserID = false;
        lcd.clear();
        lcd.print("Enter Password:");
      } else {
        

        
          lcd.clear();
          lcd.print("User ID: " + userID);
         
          id = userID.toInt();
          Serial.println(userID);
          lcd.setCursor(0, 1);
          lcd.print("Password: " + password);
          r = password;
           Serial.println(password);
          delay(2000);

          lcd.clear();
          
          isFingerDone = true;
        
      }
    } else {
      if (isEnteringUserID) {
        userID += customKey;
      } else if (isFirstPassword) {
        password += customKey;
      } 
    }
  }
  if(isFingerDone){
    lcd.setCursor(0,0);
    lcd.println("Please wait ");
  do{
    
    if (client.available()) {
    char c = client.read();
    Serial.print(c);
    
    }

  if (!client.connected()) {
    sendHttpGetRequest();
    delay(1000); // Delay between consecutive requests
  }
    
    client.stop();
    isFingerTaken = false;
    
    
  
  }while(isFingerDone);
   isFingerTaken = false;
   Serial.println(response);
  }
  if (response.indexOf("HTTP/1.1 200 OK") != -1) {
  if(!isFingerTaken){
    
    
   do{
     getFingerprint();
   }while (!isFingerTaken ); 
    lcd.clear();
      
      isAutentificationComplete = true;
  }
  } else if (response.indexOf("HTTP/1.1 404 Not Found") != -1){
      lcd.clear();
      lcd.println("Unexpected error ");
      turnBuzzer();
      turnRed();
     isAutentificationComplete = true;
    }else if (response.indexOf("HTTP/1.1 500 Internal Server Error") != -1){
      lcd.clear();
      lcd.println("Unexpected error ");
     turnBuzzer();
      turnRed();
      isAutentificationComplete = true;
    }
  
  

  
    
}

uint8_t getFingerprint() {
  lcd.clear();
  // Read the user input
  int enteredID = id;



// Capture the fingerprint image
lcd.print("Put your finger on scanner");
delay(3000);
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
   
  lcd.print("Access granted!");
  Serial.print("Entered ID: ");
  Serial.println(enteredID);
  Serial.print("Matched ID: ");
  Serial.println(matchID);
  Serial.print("Confidence: ");
  Serial.println(confidence);
  turnServo();
  isFingerTaken = true;
  
} else {
  lcd.print("Not matched!");
  turnBuzzer();
  turnRed();
  isFingerTaken = true;
}
}

void sendHttpGetRequest() {
  if (client.connect(serverIP, serverPort)) {
    client.print("GET ");
    client.print(serverPath);
    client.print("e=");
    client.print(userID);
    client.print("&p=");
    client.print(password);
    client.print("&r=");
    client.print(r);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.print(serverIP[0]);
    client.print(".");
    client.print(serverIP[1]);
    client.print(".");
    client.print(serverIP[2]);
    client.print(".");
    client.println(serverIP[3]);
    client.println("Connection: close");
    client.println();
   
    
    Serial.println("Request sent");
    String c = client.readString();
     int newlinePos = c.indexOf('\n');
     String out = c.substring(0,newlinePos);
    Serial.println(out);
    response = out;
    
    isFingerDone = false;
    
    
    
    
  } else {
    Serial.println("Failed to connect to server");
    
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
