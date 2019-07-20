#include <SPI.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "displaySymbols.h"
#include <Fonts/FreeSerif9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

static const char PROGMEM id[] = "ESP098990";
#define BUFF_SIZE 1000   //max size of the buffer
#define LOAD1 16        //load resistance of 564ohm
#define LOAD2 5        //load resistance of 232ohm
#define LOAD3 4       //load resistance of 116ohm
#define DUT   13       //load is the Device Under Test (DUT)
#define SW   0           //define the pin connected to the push button
#define SCL_PIN  14      //define pins for I2C communication
#define SDA_PIN  2
#define EN_VOLTAGE_DIVIDER 12 //the gate of a mosfet which enables the voltage divider for analog input.
#define Vin A0            //input: battery voltage 
#define SCREEN_WIDTH 128 // OLED display width, in pixels 
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
const char LOADSTR1[]="464"; //define string containing load values
const char LOADSTR2[]="232";
const char LOADSTR3[]="155";
const char LOADSTR4[]="116";
const char LOADSTR5[]="093";
const char LOADSTR6[]="077";
const char LOADSTR7[]="066";
const char LOADSTR8[]="DUT";
String filename = "analog";
int discharged = 0;
const float VLowerCutOff = 2.100000;  //define the lower cut off voltage or the end of discharge voltage for the battery
const int Vin_max = 3;           //maximum voltage that can be given as analog input
const int Vin_min = 0;           //minimum voltage that can be given as analog input
int selected = 0;                //to store the previously selected load
int front;                       //front index of queue (next empty location)
int rear;                        //rear index of queue  (first filled location)
int typeOfPress = 0;              //store whether long press or short press
float Queue[BUFF_SIZE];           //buffer stucture for storing the values if connection is lost
unsigned long int _time, CurTime, StartTime, EndTime = 0, displayedAt = 0;
unsigned short int days = 0, hours = 0, minutes = 0, seconds = 0;  //variables to store elapsed time
boolean DISP_ON = 1;    //flag to store whether to keep display on or off (cleared)
boolean ONCE_DISCHARGED = 0;   //flag to save once the batteries get discharged
boolean DISP_CLEAR_REQUIRED = 0;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);    //define display object

void queueInit() {     //initialize pointers (queue is empty)
  front = 0;
  rear = 0;
  for (int i; i < BUFF_SIZE; i++)
    Queue[i] = 0;
}

void enqueue(float V) {   //write a value to the buffer
  if( front == (BUFF_SIZE - 1) )    //if queue is full
    front = 0;                      //store again in first position (circular behavior)
  Queue[front] = V;                  
  front++;                          //increment to point to next empty location
}

float dequeue() {     //read value from the buffer
  float val;
    val = Queue[rear];
    rear++;         //increment rear to point to next filled location
    if ( rear == BUFF_SIZE )
      rear = 0;   //if overflows set it back to 0 (circular behavior)
  return val;
}

void turnOffAllLoad() {     //turns off all the loads
  digitalWrite(LOAD1,HIGH);
  digitalWrite(LOAD2,HIGH);
  digitalWrite(LOAD3,HIGH);
  digitalWrite(DUT,HIGH);
}

float readVoltage() {  //reads voltage at Vin by enabling the voltage divider
  int v;
  digitalWrite(EN_VOLTAGE_DIVIDER, HIGH);
  v=analogRead(Vin);
  digitalWrite(EN_VOLTAGE_DIVIDER, LOW);
  return ((float(Vin_max - Vin_min)*v)/1024);
}

void checkBattery() {
  float checkVoltage;
  checkVoltage = readVoltage();
  if (checkVoltage < VLowerCutOff) {   //if the read voltage is less than lower cut off display error message
    display.clearDisplay();
    display.setFont(&FreeSerif9pt7b);
    display.setCursor(35,12);
    display.print("ERROR");
    display.setFont();
    display.setCursor(0,14);
    display.print("Battery not connected\nor already discharged");
    display.display();
    while (checkVoltage < VLowerCutOff) {
      checkVoltage = readVoltage();
      delay(500);
    }
  }
  return;
}

int buttonPressed() {     //function to detect if the button is pressed / not pressed / long pressed 
  long int EndTime0, StartTime0;
  StartTime0 = millis();
  if(!digitalRead(SW)) {       //if button is pressed it gives 'LOW' on the pin
    delay(10);                //debounce
    if(!digitalRead(SW)) {     //verify again
        while(!digitalRead(SW)) {    //keep looping until the button is released
          delay(0);                //avoid rebooting of the MCU
        }
        EndTime0 = millis();
        if((EndTime0 - StartTime0) >= 1950 )
          return 2;         //2 means long press (about 2 sec.)
        else
          return 1;         //1 means short press
    }
    else return 0;          //0 means button is not pressed
  }
  else return 0;            //0 means button is not pressed
}

void attachLoad(int x) {      //function to attach the load on long press
  switch (x) {
    case 1:  {  //turn on ONE load at a time
                digitalWrite(LOAD1,LOW); 
                digitalWrite(LOAD2,HIGH); 
                digitalWrite(LOAD3,HIGH); 
                digitalWrite(DUT,HIGH);
                break ; 
              }
    case 2:  {  //turn on ONE load at a time
                digitalWrite(LOAD1,HIGH); 
                digitalWrite(LOAD2,LOW); 
                digitalWrite(LOAD3,HIGH); 
                digitalWrite(DUT,HIGH);
                break ; 
              }
    case 3:  {  //turn on specific loads
                digitalWrite(LOAD1,LOW); 
                digitalWrite(LOAD2,LOW); 
                digitalWrite(LOAD3,HIGH); 
                digitalWrite(DUT,HIGH);
                break ; 
              }
    case 4:  {  //turn on ONE load at a time
                digitalWrite(LOAD1,HIGH); 
                digitalWrite(LOAD2,HIGH); 
                digitalWrite(LOAD3,LOW); 
                digitalWrite(DUT,HIGH);
                break ; 
              }
    case 5:  {  //turn on specific loads
                digitalWrite(LOAD1,LOW); 
                digitalWrite(LOAD2,HIGH); 
                digitalWrite(LOAD3,LOW); 
                digitalWrite(DUT,HIGH);
                break ; 
              }
    case 6:  {  //turn on specific loads
                digitalWrite(LOAD1,HIGH); 
                digitalWrite(LOAD2,LOW); 
                digitalWrite(LOAD3,LOW); 
                digitalWrite(DUT,HIGH);
                break ; 
              }
    case 7:  {  //turn on specific loads
                digitalWrite(LOAD1,LOW); 
                digitalWrite(LOAD2,LOW); 
                digitalWrite(LOAD3,LOW); 
                digitalWrite(DUT,HIGH);
                break ; 
              }
    case 8:  {  //turn on ONE load at a time
                digitalWrite(LOAD1,HIGH); 
                digitalWrite(LOAD2,HIGH); 
                digitalWrite(LOAD3,HIGH); 
                digitalWrite(DUT,LOW); 
                break ; 
              }
    default: {  //turn OFF ALL loads
                digitalWrite(LOAD1,HIGH); 
                digitalWrite(LOAD2,HIGH); 
                digitalWrite(LOAD3,HIGH); 
                digitalWrite(DUT,HIGH); 
                break ; 
              }
  }   //end of switch
  return;
}

const char *LoadVal(int slctd) {      
  const char *str;
  switch (slctd) {   
      case 1: { str = LOADSTR1; } break;
      case 2: { str = LOADSTR2; } break;
      case 3: { str = LOADSTR3; } break;
      case 4: { str = LOADSTR4; } break;
      case 5: { str = LOADSTR5; } break;
      case 6: { str = LOADSTR6; } break;
      case 7: { str = LOADSTR7; } break;
      case 8: { str = LOADSTR8; } break;
      default:  {  } break;
    }     
  return str;
}
void displayLoadVal(int posX, int posY, int slctd) {      //display the selected load 

  const char *str = LoadVal(slctd);
  display.setCursor(posX,posY);     //basically sets cursor to next line & at the center
  if (str == LOADSTR8) 
    display.setCursor( (posX + 6), posY);   //for DUT move little more towrds +x as there is no ohm symbol after that
  display.print(str);
  if (str != LOADSTR8)       //no ohm symbol for load DUT
      display.drawBitmap((posX + 30),(posY-11),ohm,ohm_width,ohm_height,1);    //display ohm symbol
  
 }

void displayConnectedLd(int posX, int posY, int sl) {      //display the selected load      
  display.setFont(&FreeSerif9pt7b);
  display.clearDisplay();         //clear the display buffer
  display.setCursor(4,12);
  display.println("Connected Load");
  displayLoadVal(posX,posY,sl);
  display.display();
  delay(100);
}

void loadSelect() {    //gets input from the user and attaches the load
  display.clearDisplay();
  display.setFont(&FreeSerif9pt7b);
  display.setCursor(21,12);
  display.print("Select Load\n");
  display.setFont();    //set default font
  display.setCursor(4,14);
  display.print("Short press: Options\n");
  display.setCursor(10,23);
  display.print("Long press: Select");
  display.display();
  while(1) {
  while((digitalRead(SW)) && (selected <= 0)) {
    delay(0);      //stops resetting the MCU
  }
  typeOfPress = buttonPressed();
  if ((selected == 0) && (typeOfPress != 0))
    typeOfPress = 1;           //first time long press or short press, consider it as short press
  if ( typeOfPress == 1 ) {    //if short press detected
    selected++; 
    if(selected == 9) 
      selected=1;     //roll over condition
    display.setFont(&FreeSerif9pt7b);
    display.clearDisplay();         //clear the display buffer
    display.setCursor(21,12);
    display.print("Select Load\n");
    displayLoadVal(42,30,selected);
    display.display();
  }
  if( typeOfPress == 2 ) {//if long press detected (no need to check the validity of 'selected', that's taken care by the switch statement)
       const char *load = LoadVal(selected);
       String ld(load);
       filename = String(id);
       String date_time = dateTime();
       discharged = 0;
       filename = filename + ld + date_time +".csv";
       attachLoad(selected);            //attach the load
       displayConnectedLd(42,30,selected);     //display the attached load
       break;                  //break from the while(1)
  }
  delay(0);   // avoid rebooting of the MCU
 }
 return;
}

void updateElapsedTime(boolean TILL_WHEN) {
   unsigned long int TimeDiff;
   CurTime = millis ();  //get the current time in milliseconds
   if (TILL_WHEN)
      TimeDiff = CurTime - StartTime;  //store the difference between current time and start time
   if (!TILL_WHEN)
      TimeDiff = EndTime - StartTime;  //store the difference between end time and start time
   TimeDiff /= 1000;   //convert to sec. from milli sec.
   days = int ( (TimeDiff)/86400 );  //one day = 86,400 s
   TimeDiff %= 86400;      //store the reminder for rest of the calculations
   hours = int ( (TimeDiff)/3600 );  //one hour = 3,600 s
   TimeDiff %= 3600;
   minutes = int ( (TimeDiff)/60 );
   TimeDiff %= 60;
   seconds = TimeDiff;

    display.clearDisplay();
    display.setFont();        //default font
    display.setCursor(0,0);
    display.println("Elapsed Time : ");
    display.setCursor(0,9);
    display.print(days/10);   //first digit of days
    display.print(days%10);   //second digit of days
    display.print("d:");
    display.print(hours/10);   //first digit of hours
    display.print(hours%10);   //second digit of hours
    display.print("h:");
    display.print(minutes/10);   //first digit of minutes
    display.print(minutes%10);   //second digit of minutes
    display.print("m:");
    display.print(seconds/10);   //first digit of seconds
    display.print(seconds%10);   //second digit of seconds
    display.print("s");
   return;
}

void updateDisplay() {  //function to update display contents
  if (ONCE_DISCHARGED) {      //message if the batteries are discharged
    updateElapsedTime(0);     //difference between StartTime and EndTime
    display.setCursor(114,4);
    display.print("x");
    display.setFont(&FreeSans9pt7b);
    display.setCursor(0,30);
    display.print("Dischrgd");
    display.setCursor(70,28);
    display.print(".,");
    WiFi.mode(WIFI_OFF);  //turn off wifi
  }
  else {  //message when batteries are being discharged
    updateElapsedTime(1);     //difference between StartTime and CurTime
    display.setFont(&FreeSans9pt7b);
    display.setCursor(0,30);
    display.print(Queue[front-1]);
    display.print("V  Ld:");
  }
  display.drawBitmap(114,0,wifi,wifi_width,wifi_height,1);
  displayLoadVal(80,30,selected);
  display.display();
  DISP_CLEAR_REQUIRED = 1;
  return;
}

void checkForUpdate() {
  if ((_time - StartTime) < 10000) {    //update the display for first ten seconds
        updateDisplay();
        DISP_ON = 0;
      }
  else if ( !digitalRead(SW)){       //any time if the user presses the push button update the display
    updateDisplay();
    DISP_ON = 1;
    displayedAt = millis();       //save the time at which it displayed
  }
  else if (DISP_ON) {
     updateDisplay();
  }
  else if (DISP_CLEAR_REQUIRED && (!DISP_ON)) {
    display.clearDisplay();
    display.display();
  }
  else;
  
  if ((millis() - displayedAt) > 9900 )   // at least display for ~10 sec.
    DISP_ON = 0;            //otherwise keep the display clear

  return;
}
