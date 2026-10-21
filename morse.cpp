/*
Created by Justin C.
Created on October 10th, 2026
Last Edited: October 20th, 2026

Translating button pushes inspired by Morse Code to convert
Into letters on an lcd screen

Has buzzer, led, and lcd for visual outputs

3-Buttons
*/


#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <avr/pgmspace.h>

//Defines the screen
LiquidCrystal_I2C lcd(0x27, 16, 2); 


//Defines Constants
const int dotPin = 12;
const int dashPin = 11;
const int spacePin = 10;

const int BUZZER = 2;

const int spaceLEDPin = 9;

bool dotPrevious = false; 
bool dashPrevious = false;
bool spacePrevious = false;

//Limits the messages to 6 inputs
char message[6] = "";
int bufferIndex = 0; //Current index of the char to keep track

const int debounceDelay = 50;

//Tracking for the cursor the current row and column
int lcdRow = 0;
int lcdCol = 0;

unsigned long buzzerStartTime = 0;
unsigned long buzzerDuration = 0;
bool buzzerState = false;

const int dotBuzzDuration = 200;
const int dashBuzzDuration = 600;

const char* morseCodeBook[] = {
  ".-",   "-...", "-.-.", "-..",  ".",    "..-.", "--.",  "....", "..",    // A - I
  ".---", "-.-",  ".-..", "--",   "-.",   "---",  ".--.", "--.-", ".-.",  // J - R
  "...",  "-",    "..-",  "...-", ".--",  "-..-", "-.--", "--.."         // S - Z
};

void setup() {
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0,0);

  pinMode(dotPin, INPUT_PULLUP);
  pinMode(dashPin, INPUT_PULLUP);
  pinMode(spacePin, INPUT_PULLUP);

  pinMode(spaceLEDPin, OUTPUT);
  digitalWrite(spaceLEDPin, LOW);
  pinMode(BUZZER, OUTPUT);
}

void loop() {
  //Updates the buzzer everytime it runs
  updateBuzzer();

  //Reads all of them
  bool dotCurrent = digitalRead(dotPin);
  bool dashCurrent = digitalRead(dashPin);
  bool spaceCurrent = digitalRead(spacePin);

  if (!dotCurrent || !dashCurrent || !spaceCurrent) { //Essentially, if any of them are pushed
    
    if ((dotCurrent == LOW) && (dotPrevious == false)) { //Appends a dot to message if pressed
      if (bufferIndex < 5) {
        appendMessage('.');
      }
      delay(debounceDelay);
  }

    if ((dashCurrent == LOW) && (dashPrevious == false)) { //Appends a dash to message if pressed
      if (bufferIndex < 5) {
        appendMessage('-');
      }
      delay(debounceDelay);
    }

    if ((spaceCurrent == LOW) && (spacePrevious == false)) { //If Space is pressed
      if (bufferIndex == 0) { //If Double pressed, print a space on the screen
        printToLCD(' ');

        analogWrite(spaceLEDPin, 150); //~3V for the LED to avoid flashbangs lol
        delay(150);
        analogWrite(spaceLEDPin, 0);
      } else {
        char decodedChar = decodeMorse(message); //Prints the messages (morse) on the LCD (english)
        printToLCD(decodedChar);

        //Resets values
        bufferIndex = 0;
        message[0] = '\0';
      }
    }
    delay(debounceDelay); //Bouncy bouncy delay :D
  }

  //Resets previous
  dotPrevious = (dotCurrent == LOW);
  dashPrevious = (dashCurrent == LOW);
  spacePrevious = (spaceCurrent == LOW);
}



void printToLCD(char letter) {
  if (lcdCol >= 16) { //If goes too far to the right

    //Realigns on the next line
    lcdCol = 0;
    lcdRow++;
  }

  if (lcdRow >= 2) {// If goes too far down

    //Starts fresh
    lcd.clear();
    lcdCol = 0;
    lcdRow = 0;
  }

  //Printing logic
  lcd.setCursor(lcdCol, lcdRow);
  lcd.print(letter);

  //Sets to next space over
  lcdCol++;
}

void updateBuzzer(){
  if (buzzerState == false) {
    return; //Void can't return anything, so it just exits the function
  }

  if (millis() - buzzerStartTime >= buzzerDuration) { //Watiing until it's time to turn it off
    digitalWrite(BUZZER, LOW);
    buzzerState = false;
  }
}

void triggerBuzzer(unsigned long duration) {
  buzzerStartTime = millis(); //Begins a timer and to not use delay (blocking)
  buzzerDuration = duration;
  buzzerState = true;
  digitalWrite(BUZZER, HIGH); //Buzzes
}

void appendMessage(char appendage) {
  message[bufferIndex] = appendage; //Apends the char to the message
  bufferIndex++; //Moves the index by one
  message[bufferIndex] = '\0';//Locks the string array with the null terminator safety guard

  if (appendage == '.'){
    triggerBuzzer(dotBuzzDuration); //Buzzes shortly
  } else if (appendage == '-') {
    triggerBuzzer(dashBuzzDuration); //Buzzes slightly longer
  }
}

char decodeMorse(const char* currentMessage) {
  for (int letterIndex = 0; letterIndex < 26; letterIndex++) { //Goes through each letter from 1 - 26
    if (strcmp(currentMessage, morseCodeBook[letterIndex]) == 0) { //Returns zero if found within the morseCodebook dictionary
      return 'A' + letterIndex; // ASCII allows addition, ex. 'A' + 1 = 'B'

    }
  }

  return '*'; //If not found in the morseCodebook dictionary
}
