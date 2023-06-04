#include <LiquidCrystal.h>
#include <Keypad.h>
#include <Adafruit_Fingerprint.h>
#include <SPI.h>
#include <UIPEthernet.h>

static byte mymac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

// Destination server details
static byte serverIP[] = {192, 168, 0, 2};
static const int serverPort = 80;
static const char* serverPath = "/api/v1/devices/add?";

static char responseBuffer[512];

static EthernetClient client;

LiquidCrystal lcd(7, 6, 3, 30, 31, 32);

// RXPIN is IN from sensor (GREEN wire)
// TXPIN is OUT from arduino  (WHITE wire)
#define RXPIN 10  // change this to whatever
#define TXPIN 11  // change this to whatever
SoftwareSerial mySerial(RXPIN, TXPIN);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

uint8_t id;

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

String response = "";
String userID = "";
String password = "";
String confirmPassword = "";
bool isFirstPassword = true;
bool isEnteringUserID = true;
bool isFingerTaken = true;
bool isEnrollmentComplete = false;
bool isFingerDone = false;

String p = "222";
String e = "111";
String r = "666";
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
  confirmPassword = "";
  isFingerTaken = true;
  isFirstPassword = true;
  isEnteringUserID = true;
  isEnrollmentComplete = false;
  isFingerDone = false;
  response = "";
}

void displayInitialPrompt() {
  lcd.clear();
  lcd.print("Enter User ID:");
}

void loop() {
  // put your main code here, to run repeatedly:
  if (isEnrollmentComplete) {
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
  } else {
    lcd.setCursor(0, 1);
    lcd.print(confirmPassword);
  }

  if (customKey) {
    if (customKey == '*') {
      if (isEnteringUserID) {
        userID = "";
      } else if (isFirstPassword) {
        password = "";
      } else {
        confirmPassword = "";
      }
    } else if (customKey == '#') {
      if (isEnteringUserID) {
        isEnteringUserID = false;
        lcd.clear();
        lcd.print("Enter Password:");
      } else if (isFirstPassword) {
        isFirstPassword = false;
        lcd.clear();
        lcd.print("Confirm Password:");
      } else {
        lcd.clear();
        lcd.print("Checking Password...");
        delay(500);

        if (password == confirmPassword) {
          lcd.clear();
          lcd.print("User ID: " + userID);
          id = (uint8_t)userID.toInt();
          Serial.println(userID);
          lcd.setCursor(0, 1);
          lcd.print("Password: " + password);
          r = password;
           Serial.println(password);
          delay(2000);

          lcd.clear();
          
          isFingerDone = true;
        } else {
          lcd.clear();
          lcd.print("Passwords do not match");
          delay(2000);
          lcd.clear();
          password = "";
          confirmPassword = "";
          isFirstPassword = true;
          lcd.print("Enter Password:");
        }
      }
    } else {
      if (isEnteringUserID) {
        userID += customKey;
      } else if (isFirstPassword) {
        password += customKey;
      } else {
        confirmPassword += customKey;
      }
    }
  }
  if(isFingerDone){
    lcd.setCursor(0,0);
    lcd.println("Please wait ");
  do{
    /*Ethernet.begin(mymac);
    delay(1000);
    
    Serial.println("Ethernet connected");
    Serial.print("IP address: ");
    Serial.println(Ethernet.localIP());*/
    //for( int i = 0; i<10;i++){
    if (client.available()) {
    char c = client.read();
    Serial.print(c);
    
    }

  if (!client.connected()) {
    sendHttpGetRequest();
    delay(1000); // Delay between consecutive requests
  }
    //}
    client.stop();
    isFingerTaken = false;
    //isFingerDone = false;
    
  
  }while(isFingerDone);
   isFingerTaken = false;
   Serial.println(response);
  }
  if (response.indexOf("HTTP/1.1 200 OK") != -1) {
  if(!isFingerTaken){
    
    
   do{
     getFingerprintEnroll();
   }while (!isFingerTaken ); 
    lcd.clear();
      lcd.println("User enrolled ");
       digitalWrite(redPin, LOW);
       digitalWrite(greenPin, HIGH);
       digitalWrite(bluePin, LOW);
      delay(3000);
       digitalWrite(redPin, LOW);
       digitalWrite(greenPin, LOW);
       digitalWrite(bluePin, LOW);
      isEnrollmentComplete = true;
  }
  } else if (response.indexOf("HTTP/1.1 404 Not Found") != -1){
      lcd.clear();
      lcd.println("Unexpected error ");
      digitalWrite(redPin, HIGH);
       digitalWrite(greenPin, LOW);
       digitalWrite(bluePin, LOW);
      delay(3000);
       digitalWrite(redPin, LOW);
       digitalWrite(greenPin, LOW);
       digitalWrite(bluePin, LOW);
      isEnrollmentComplete = true;
    }else if (response.indexOf("HTTP/1.1 500 Internal Server Error") != -1){
      lcd.clear();
      lcd.println("Unexpected error ");
      digitalWrite(redPin, HIGH);
       digitalWrite(greenPin, LOW);
       digitalWrite(bluePin, LOW);
      delay(3000);
       digitalWrite(redPin, LOW);
       digitalWrite(greenPin, LOW);
       digitalWrite(bluePin, LOW);
      isEnrollmentComplete = true;
    }
  
  

  
    
}

uint8_t getFingerprintEnroll() {
  lcd.clear();
  int p = -1;
  lcd.setCursor(0, 0);
  lcd.println("Scan a finger...");

  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      lcd.setCursor(0, 0);
      lcd.print("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
    
      lcd.setCursor(0, 0);
      lcd.print("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      lcd.setCursor(0, 0);
      lcd.print("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      lcd.setCursor(0, 0);
      lcd.print("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      lcd.setCursor(0, 0);
      lcd.print("Could not find fingerprint features");
      return p;
    default:
      lcd.setCursor(0, 0);
      lcd.print("Unknown error");
      return p;
  }
 lcd.clear();  
  lcd.setCursor(0,1);
  lcd.print("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  lcd.setCursor(0,0);
  lcd.print("Place same finger again");
  
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      lcd.setCursor(0, 0);
      lcd.print("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      lcd.setCursor(0, 0);
      lcd.print("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      lcd.setCursor(0, 0);
      lcd.print("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      lcd.setCursor(0, 0);
      lcd.print("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      lcd.setCursor(0, 0);
      lcd.print("Could not find fingerprint features");
      return p;
    default:
      lcd.setCursor(0, 0);
      lcd.print("Unknown error");
      return p;
  }

  // OK converted!
  lcd.clear();
  lcd.print("Model for #");  
  lcd.print(id);
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    lcd.setCursor(0,1);
    lcd.print("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    lcd.setCursor(0,1);
    lcd.print("Did not match");
    delay(2000);
    isFingerTaken = false;
    return isFingerTaken;
  } else {
    Serial.println("Unknown error");
    return p;
  }
  delay(2000);
  lcd.clear();
  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {

    lcd.print("Stored!");
    delay(2000);
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }
  
  isFingerTaken = true;
  
   
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
    //isFingerTaken = false;
    isFingerDone = false;
    
    
    
    
  } else {
    Serial.println("Failed to connect to server");
    //isFingerDone= true;
  }
}
