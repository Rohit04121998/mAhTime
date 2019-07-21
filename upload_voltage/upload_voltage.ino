/********************************************************************************
BSD 3-Clause License

Copyright (c) 2019, Sudheendra Hegde
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*****************************************************************************************/
#include "DateTime.h"
#include "config.h"
#include "DischargeBattery.h"

char V_val[10];
char file[50];
float Vbat = 0, saved = 0;
float last = -1;
int stat = 0;
void setup() {
  //set pinMode
  pinMode(LOAD1,OUTPUT);  //load resistance of 464ohm
  pinMode(LOAD2,OUTPUT);  //load resistance of 232ohm
  pinMode(LOAD3,OUTPUT);  //load resistance of 116ohm
  pinMode(DUT,OUTPUT);       //load is the Device Under Test (DUT)
  pinMode(SW,INPUT_PULLUP);         //push button (active high input)
  pinMode(EN_VOLTAGE_DIVIDER,OUTPUT);  //pin connected to gate of the mosfet which enables the voltage divider
  pinMode(Vin,INPUT);
  
  turnOffAllLoad();   //make sure all loads are off initially
  Wire.begin(SDA_PIN,SCL_PIN);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);            // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {      // Address 0x3C for 128x32
    for(;;);                                            // SSD1306 allocation failed, Don't proceed, loop forever
  }
  display.clearDisplay();                       //clear the display buffer
  display.setRotation(2);                       //rotate 180 degerees
  display.drawBitmap(                           //draw bitmap image
    (display.width()  - logo_width ) / 2,       //align it at the center
    (display.height() - logo_height) / 2,
    logo, logo_width, logo_height, 1);
  display.display();                            //actually display the logo
  delay(2000);
  display.clearDisplay();        //clear the display buffer
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(3,0);
  display.println("    Connecting to   ");
  display.setFont(&FreeSerif9pt7b);
  display.setCursor(0,24);
  display.println(ssid);
  display.display(); // actually display all of the above
  
   // Connect to the WiFi
  WiFi.begin(ssid, wifi_password);
  
  // wait for a connection
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  display.clearDisplay();
  display.setCursor(25,18);
  display.println("Connected");
  if (client.connect(clientID, mqtt_username, mqtt_password)) {
    display.println("Connected to MQTT Broker!");
  }
  else {
    display.println("Connection to MQTT Broker failed...");
  }
  
  display.display();
  delay(1000);
  queueInit();     //initialize queue
  checkBattery();   //verify whther connected battery has voltage > VLowerCutOff
  loadSelect();           //let the user select load / DUT
  display.setFont(&FreeSans9pt7b);  //change font to Sans
  StartTime = millis();     //save the starting time
}

void loop() {
  client.loop();
  _time = millis();
  
  if( !ONCE_DISCHARGED )  
    Vbat = readVoltage();
  if( !(Vbat < VLowerCutOff) )
    enqueue(Vbat);    //enqueue the values
  
  if(Vbat < VLowerCutOff)  {   //if the battery Vbat falls below the lower cut off
      ONCE_DISCHARGED = 1;         //clear the ONCE_DISCHARGED so that it does not oscillate when terminal Vbat rises slightly.
      turnOffAllLoad();    //turn OFF ALL loads when batteries got fully discharged.
      EndTime = millis();  //store end time
      DISP_ON = 1; //display that the batteries got discharged
      displayedAt = millis ();  //this is required when display is turned on using DISP_ON
      while(1){
        if (rear != front)    //queue is not empty
          if((!(WiFi.status() < WL_CONNECTED)) && client.connect(clientID, mqtt_username, mqtt_password)) {
            filename.toCharArray(file,50);
            if(stat == 1){
            saved = dequeue();            //read value from the queue 
            dtostrf(saved,4,3,V_val);     //convert float to char array
            client.publish(mqtt_topic, V_val);    //publish value to the MQTT broker
              }
          else if(client.publish(mqtt_topic,file) && stat == 0){  // The values read can be sent only after sending  filename
            stat = 1;
            }
          }
         if(rear == front && discharged == 0){  // This message is sent only after queue is empty and only once per discharge
          if(client.publish(mqtt_topic,"Fully Discharged"))
          discharged == 1;
          }

        checkForUpdate();
        yield();  
     }
  }
  
  while( (millis() - _time) < 4000){    //wait for 4 seconds
      checkForUpdate();
      if (rear != front)    //queue is not empty
        if((!(WiFi.status() < WL_CONNECTED)) && client.connect(clientID, mqtt_username, mqtt_password)) {
           filename.toCharArray(file,50);
           if(stat == 1){
            saved = dequeue();            //read value from the queue 
            dtostrf(saved,4,3,V_val);     //convert float to char array
            client.publish(mqtt_topic, V_val);    //publish value to the MQTT broker
            }
          else if(client.publish(mqtt_topic,file) && stat == 0){
            stat = 1;
            }
          }

  
      yield();
  }
}
