#include <LiquidCrystal.h>
#include <Keypad.h>

// initialize the library with the numbers of the interface pins
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

char customKey;
String password = "";
String confirmPassword = "";
bool isFirstPassword = true;

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2); // set up the LCD's number of columns and rows
}

void loop() {
  customKey = customKeypad.getKey();

  if (isFirstPassword) {
    lcd.setCursor(0, 0);
    lcd.print("Enter Password:");
    lcd.setCursor(0, 1);
    lcd.print(password);
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Confirm Password:");
    lcd.setCursor(0, 1);
    lcd.print(confirmPassword);
  }

  if (customKey){
    if (customKey == '*') {
      if (isFirstPassword) {
        password = "";
      } else {
        confirmPassword = "";
      }
    } else if (customKey == '#') {
      if (isFirstPassword) {
        isFirstPassword = false;
        lcd.clear();
      } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        if (password == confirmPassword) {
          lcd.print("Your Password is:");
          lcd.setCursor(0, 1);
          lcd.print(password);
          delay(2000);
          lcd.clear();
          password = "";
          confirmPassword = "";
          isFirstPassword = true;
        } else {
          lcd.print("Passwords do not match");
          delay(2000);
          lcd.clear();
          password = "";
          confirmPassword = "";
          isFirstPassword = true;
        }
      }
    } else {
      if (isFirstPassword) {
        password += customKey;
      } else {
        confirmPassword += customKey;
      }
    }
  }
}
