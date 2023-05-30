#include <LiquidCrystal.h>
#include <Keypad.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_Fingerprint.h>

LiquidCrystal lcd(7, 6, 3, 30, 31, 32);

// RXPIN is IN from sensor (GREEN wire)
// TXPIN is OUT from arduino  (WHITE wire)
#define RXPIN 10  // change this to whatever
#define TXPIN 11  // change this to whatever
SoftwareSerial mySerial(RXPIN, TXPIN);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

uint8_t id;

// Keypad Connections
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

#define SS_PIN 53
#define RST_PIN 5
MFRC522 rfid(SS_PIN, RST_PIN);

String userID = "";
String password = "";
String confirmPassword = "";
String rfidData = "";
bool isFirstPassword = true;
bool isEnteringUserID = true;
bool isFingerTaken = true;
bool isEnrollmentComplete = false;  // Variable to track enrollment completion

void setup() {
  Serial.begin(9600);
  while (!Serial)
    ;
  delay(100);
  lcd.begin(16, 2);
  SPI.begin();
  rfid.PCD_Init();
  lcd.print("Enter User ID:");

  finger.begin(57600);

  if (finger.verifyPassword()) {
  } else {
    Serial.println("Fingerprint error");
    while (1) {
      delay(1);
    }
  }
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
  rfidData = "";
  isFirstPassword = true;
  isEnteringUserID = true;
  isEnrollmentComplete = false;
}

void displayInitialPrompt() {
  lcd.clear();
  lcd.print("Enter User ID:");
}

void loop() {
  if (isEnrollmentComplete) {
    // Reset variables and display initial prompts
    resetVariables();
    displayInitialPrompt();
    isEnrollmentComplete = false;
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

          lcd.setCursor(0, 1);
          lcd.print("Password: " + password);
          delay(2000);

          lcd.clear();
          lcd.print("Scan RFID Key");
          rfidData = "";  // Reset RFID data
          while (!rfid.PICC_IsNewCardPresent()) {
            // Wait for RFID card to be presented
          }
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

  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    for (byte i = 0; i < rfid.uid.size; i++) {
      rfidData += String(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
      rfidData += String(rfid.uid.uidByte[i], HEX);
      if (i < rfid.uid.size - 1) {
        rfidData += " ";
      }
    }
    rfidData.toUpperCase();  // Convert the RFID data to uppercase
    lcd.clear();
    lcd.print("RFID Detected:");
    lcd.setCursor(0, 1);
    lcd.print(rfidData);
    delay(2000);

    lcd.clear();
    lcd.print("User ID: " + userID);
    lcd.setCursor(0, 1);
    lcd.print("Password: " + password);
    delay(2000);
    lcd.setCursor(0, 1);
    lcd.print("RFID Key: " + rfidData);

    Serial.println(userID);
    Serial.println(password);
    Serial.println(rfidData);
    isFingerTaken = false;
    // Set enrollment as complete
    //isEnrollmentComplete = true;
  }
   
  if(!isFingerTaken){
     
   do{
     getFingerprintEnroll();
   }while (!isFingerTaken );  
   isEnrollmentComplete = true;   
  }
}


uint8_t getFingerprintEnroll() {

  int p = -1;
  lcd.setCursor(0, 1);
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
  lcd.setCursor(0,1);
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
