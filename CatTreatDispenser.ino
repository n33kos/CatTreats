/*******************************************************************
  SoftServo sketch for Adafruit Trinket.  Turn the potentiometer knob
  to set the corresponding position on the servo 
  (0 = zero degrees, full = 180 degrees)
 
  Required library is the Adafruit_SoftServo library
  available at https://github.com/adafruit/Adafruit_SoftServo
  The standard Arduino IDE servo library will not work with 8 bit
  AVR microcontrollers like Trinket and Gemma due to differences
  in available timer hardware and programming. We simply refresh
  by piggy-backing on the timer0 millis() counter
 
  Required hardware includes an Adafruit Trinket microcontroller
  a servo motor, and a potentiometer (nominally 1Kohm to 100Kohm
 
  As written, this is specifically for the Trinket although it should
  be Gemma or other boards (Arduino Uno, etc.) with proper pin mappings
 
  Trinket:        USB+   Gnd   Pin #0  Pin #2 A1
  Connection:     Servo+  -    Servo1   Potentiometer wiper
 
 *******************************************************************/
#include <Adafruit_SoftServo.h>  // SoftwareServo (works on non PWM pins)


#define SERVO1PIN    0   // Servo control line (orange) on Trinket Pin #0

Adafruit_SoftServo myServo1;

int closedValue = 0;
int openValue = 90;
int isOpened = 0; //0=closed 1=opened
int isDispensing = 1;
int openTimer = 0;
int openLength = 25;
int triggerValue = 0;
int lightSensitivity = 100;
unsigned int milliseconds = 0;
unsigned int seconds = 0;
unsigned int minutes = 0;
unsigned int hours = 0;

void setup() {
  // Set up the interrupt that will refresh the servo for us automagically
  OCR0A = 0xAF;            // any number is OK
  TIMSK |= _BV(OCIE0A);    // Turn on the compare interrupt (below!)
  
  pinMode(2, INPUT);
 
 //debugging on this pins led
  pinMode(1, OUTPUT);
  digitalWrite(1,LOW);
  
  myServo1.attach(SERVO1PIN);   // Attach the servo to pin 0 on Trinket
  myServo1.write(closedValue);           // Tell servo to go to position per quirk
  delay(15);                    // Wait 15ms for the servo to reach the position
}
 
void loop()  {
  GetValues();
  RunTimers();
  RunLogic();
  SetValues();
  delay(15);// wait
}

void GetValues() {
    //Get Values
    triggerValue = analogRead(1);
}

void RunLogic() {
    //Dispensation Logic gets fired every 12 hours
    if(isOpened == 0 && hours >= 12){
      isDispensing = 1;
      openTimer = 0;
      milliseconds = 0;
      hours = 0;
      minutes = 0;
      seconds = 0;
    }
}

void SetValues() {
    //Set Values
    if(isDispensing == 0 && isOpened == 1) {
      CloseHopper();
    }else if(isDispensing == 1 && isOpened == 0) {
      OpenHopper();
    }
}

void RunTimers() {
  //Increment Open Timer
  if(isDispensing == 1) {
    openTimer = openTimer + 1;
    if(openTimer > openLength){
      isDispensing = 0;
      openTimer = 0;
    }
  }
  
}

void OpenHopper() {
  myServo1.write(openValue);
  isOpened = 1;
}

void CloseHopper() {
  myServo1.write(closedValue);
  isOpened = 0;
}

// We'll take advantage of the built in millis() timer that goes off
// to keep track of time, and refresh the servo every 20 milliseconds
// The SIGNAL(TIMER0_COMPA_vect) function is the interrupt that will be
// Called by the microcontroller every 2 milliseconds
volatile uint8_t counter = 0;
SIGNAL(TIMER0_COMPA_vect) {
  // this gets called every 2 milliseconds
  counter += 2;

  //Refresh servo and count milliseconds every 20 milliseconds
  if (counter >= 20) {
    milliseconds = milliseconds + 20;
    counter = 0;
    myServo1.refresh();
  }
  
  if(milliseconds >= 1000) {
    milliseconds = 0;
    seconds = seconds+1;
  }
  if (seconds >= 60) {
    seconds = 0;
    minutes = minutes+1;
  }
  if (minutes >= 60) {
    minutes = 0;
    hours = hours+1;
  }
  if (hours >= 24) {
    //hours = 0;
    //Were going to remove the hour counter zero-out since we are resetting the value after dispensation anyway
  }
}
