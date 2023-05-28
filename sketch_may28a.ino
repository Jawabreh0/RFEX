#include <LiquidCrystal.h>
#include <Keypad.h>
#include <SPI.h>
#include <MFRC522.h>

LiquidCrystal lcd(7, 6, 3, 30, 31, 32);

// Keypad Connections
const byte ROWS = 4;
const byte COLS = 4;
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {22,23,24,25};  // Connect to the row pinouts of the keypad
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

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  SPI.begin();
  rfid.PCD_Init();
  lcd.print("Enter User ID:");
}

void loop() {
    char customKey = customKeypad.getKey(); // Add this line to declare customKey


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
          lcd.setCursor(0, 1);
          lcd.print("Password: " + password);
          delay(2000);

          lcd.clear();
          lcd.print("Scan RFID Key");
          rfidData = ""; // Reset RFID data
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
    rfidData.toUpperCase(); // Convert the RFID data to uppercase
    lcd.clear();
    lcd.print("RFID Detected:");
    lcd.setCursor(0, 1);
    lcd.print(rfidData);
    delay(2000);

    lcd.clear();
    lcd.print("User ID: " + userID);
    lcd.setCursor(0, 1);
    lcd.print("Password: " + password);
    lcd.setCursor(0, 2);
    lcd.print("RFID Key: " + rfidData);

    // Reset variables
    userID = "";
    password = "";
    confirmPassword = "";
    isFirstPassword = true;
    isEnteringUserID = true;
    rfidData = "";
    
    delay(3000);
    lcd.clear();
    lcd.print("Enter User ID:");
  }
}
