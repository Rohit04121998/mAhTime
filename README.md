# mAhTime

## Objective 
To calculate the battery life for a device using small batteries and intern use the data to calculate for any bigger battery.

## Why do we need this?

The capacity of a battery is measured in Ah (ampere-hours, for large batteries) or mAh (milliampere-hours, for small batteries). This is usually mentioned on the battery or in the datasheets provided by the manufacturer. But the capacity changes depending on the age of the battery, operating conditions, slightly different chemical composition, etc. So it affects the final result.

To get the actual capacity (close-enough-value), measuring the same is the method. Different methods are there using which one can measure the capacity, for example, Coulomb counting, Voltage monitoring, etc. (Learn more here or here).

Here we use the second method; monitoring the terminal voltage when a known load (resistive) is connected.

## Method:
The batteries are charged and connected to a custom-designed shield for the ESP8266 microcontroller and a load is selected using a button. The microcontroller logs the terminal voltage read from its pin to the MQTT broker in RPi until the lower cut off voltage for the batteries is reached. The loads are automatically disconnected using logic level p-MOS switches. 

The values read by the microconttrollr is sent to a RPi using a MQTT broker named **mosquitto**. MQTT is a pub-sub based protocol where one device reads and sends the data to a topic and another device subscribes to the same topic to get the data published by the first device. A brief explanation of how value are sent via a broker is explained in the next heading. The recieved data is run through a python script to display the discharge curve and the area under the curve (area under the V*I v/s t curve is power rating and area under the I v/s t curve is the mAh / Ah of the batteries).

Once again the batteries are charged but this time Device Under Test (DUT) is selected instead of a load (using the same microcontroller). The time taken for the DUT to discharge the batteries is found. This can then be used to calculate the life of any battery using ratios ( Eg.: If for X (mAh) it is Y (no. of days) then for Z (mAh)? = YZ/X ).

## MQTT

MQTT is one of the most commonly used protocols in IoT projects. It stands for Message Queuing Telemetry Transport.

  In addition, it is designed as a lightweight messaging protocol that uses publish/subscribe operations to exchange data between clients and the server. Furthermore, its small size, low power usage, minimized data packets and ease of implementation make the protocol ideal of the “machine-to-machine” or “Internet of Things” world.

## WHY MQTT??

MQTT has unique features you can hardly find in other protocols, like:

* It’s a lightweight protocol. So, it’s easy to implement in software and fast in data transmission.
* It’s based on a messaging technique. You know how fast your messenger/WhatsApp message delivery is. MQTT protocol is a similar one.
* Minimized data packets. Hence, low network usage.
* Low power usage. As a result, it saves the connected device’s battery.

## How MQTT works
Like any other internet protocol, MQTT is based on clients and a server. The server is the guy who is responsible for handling the client’s requests of receiving or sending data between each other. MQTT server is called a broker and the clients are simply the connected devices.
#### So:

When a device (a client) wants to send data to the broker, we call this operation a *“Publish”* and the device *"Publisher"*.
When a device (a client) wants to receive data from the broker, we call this operation a *“Subscribe”* and this device as *"Subscriber"*.

  
  <p align="center"><img src="https://wso2.com/files/mqtt-article-buddhima.png" height="350" ></p>
  
 ## MQTT Components

* *Broker*, which is the server that handles the data transmission between the clients.
* A *Topic*, which is the place a device want to put or retrieve a message to/from.
* The *Message*, which is the data that a device receives “when subscribing” from a topic or send “when publishing” to a topic.
* *Publish*, is the process a device does to send its message to the broker.
* *Subscribe*, where a device does to retrieve a message from the broker.


*By now hopefully you would have an idea how a MQTT network might look or works like.* 
  
  
  The common methods used to transfer data are:
  
  * Making use of [test.mosquitto.org](http://test.mosquitto.org/) server as a MQTT broker and sending the data via this server.
  
  * Making use of a common WiFi network to share the data.
  
  * Making one of the ends (i.e either ESP8266 or the RPi) as an *Access Point* and connect the other to it.
  
    The 2nd and the 3rd methods do not use any online brokers. Therefore a broker is to be installed in the RPi. For information on how to install mosquitto broker on your RPi use the link [Getting Started with Mosquitto MQTT Broker on a Raspberry Pi](https://www.youtube.com/watch?v=AsDHEDbyLfg&t=72s).

## Hardware & Firmware:
A microcontroller unit (ESP8266-12E on LOLin board) has an inbuilt ADC (Analog to Digital Converter) with a precision of (1/1024)th of a volt. But the input voltage range is only 0 (0) - 1V (1024). To use this in 0 - 3V range a resistor voltage divider is implemented using precision resistors and an n- MOS switch to enable this voltage divider. The voltage divider is not the one which is present on the LOLin board. Those are not precision resistors, hence the readings may not be accurate. The n-MOS switch is required as the voltage divider acts as another resistive load parallel to the selected load. An OLED display is used to display the selected load, time, etc.

The hardware limits the maximum battery voltage to 3.6V as the battery voltage appears at pin A0 when voltage divider is disabled and the absolute maximum voltage given is 3.6V for esp8266. The time taken for measurement is determined by the capacity of the battery and the chosen load.

The circuit connections and the designed PCB can be found in the directory PCB_KICAD. The details about the charger and the battery can be found in the 'Charger_hacked' directory. 
Arduino IDE is used to program the microcontroller. The required header files and libraries are in the 'upload_voltage' directory.

<p align="center"><img src="https://github.com/Rohit04121998/mAhTime/blob/master/gallery/pcb_unsoldered.jpg" height="400"></p>

### <p align="center">Fig 1. Unsoldered PCB</p>

PCB Front             |  PCB Back              
:-------------------------:|:-------------------------:
<img src="https://github.com/Rohit04121998/mAhTime/blob/master/gallery/pcb_soldered.jpg" height="175" width="350"> |  <img src="https://github.com/Rohit04121998/mAhTime/blob/master/gallery/pcb_soldered_2.jpg" height="175" width="350"> 

### <p align="center">Fig 2. Soldered PCB</p>

<p align="center"><img src="https://github.com/Rohit04121998/mAhTime/blob/master/gallery/ESP_Display.JPG" height="275" width="500"></p>

### <p align="center">Fig 3. Display</p>

## Code:

* In the *upload_voltage.ino* file, **Vbat** variable is used to store the voltage value. The queue is sized to hold a maximum of 1000 values. Here, the value is stored and sent every 4 seconds.

* To get the date and time info from the ESP8266, *NTTP Client* has been used . The *getFormattedDate()* function gives the date and time format as 2018-05-28T10:00:13Z. So modifications were done and the final output was  2018-05-28_10:00:13. This was further modified in the *plot.py* by replacing ':' with 'H', 'M' and 'S'. This was then appended to the *.csv* file having the load value as its initial name.
* The code is made such that a short press of the button is conssidered as next option and a long press is considered as select. The long press for one of the load values sets the value of variable *seleted* to a umber between 1 and 8. The load is selected according to the value of variable *selected* in the function *LoadVal*. The function *attachLoad* attaches the load and changes the state of the mosfets accordingly. The discharging of the battery starts as soon as the load is attached.
* Before any value is sent through MQTT, the variable *filename* which contains the device ID, load, date and time with the .csv extension is published to inform the subscriber that a new discharge cycle has started and to create a new CSV file by the name of the variable sent. Only after the filename are the voltage values to be sent.
* Once all the values are sent over through MQTT, a "Fully Discharged" message is sent stating that te experiment's finished.

* The code for MQTT subscribe is a simple code which tells the interpreter what to do when the connection with the MQTT broker is established and when a message is recieved. The funtion on_connect will be executed as soon as connection is established. The on_message funtion is executed as soon as a message is received. The messaged is checked for a **.csv** extension and a new CSV file is created if the result of this comparision is positive.
* This message with a .csv extension contains the ID of the device which published the message,the load selected, date and time of the moment a load is selected. The name of the new CSV file created contains only load,date and time(format containing 'H' 'M' and 'S' specified in one of the above points. Here data and time are included to differentiate between experiments conducted on the same day.

* The voltage values received are added to this file along with the time elapsed. At the end of the experiment "FUlly Discharged" message is recieved once and is added to the file informing the plotter that the experiment is finished and the mAh value can be calculated.
* In the script Plot_value.py **FuncAnimate** function is used to create an interactive mode i.e to plot a real time graph. The mAh value of the battery is shown only when the message "Fully Discharged" intercepts by the interpreter.

## Result:
  The result of the experments need not be the same everytime. The mAh value might change if the battery is not fully charged before the start of the experiment. The time taken for the discharge and the numbeer of voltage values recorded varies if a different load value is used or if the initial voltage of the battery is different.

The device was tested for 3 different loads and the graph plotted were as follows:

**Load**             |  093 Ohms             |  161 Ohms(Not fully charged)                                   
:-------------------------:|:-------------------------:|:-------------------------:
**Graph**| <img src="https://github.com/Rohit04121998/mAhTime/blob/master/gallery/Graph_093.png" height="350"> | <img src="https://github.com/Rohit04121998/mAhTime/blob/master/gallery/Graph_116.png" height="350">

<p align="center"><img src="https://github.com/Rohit04121998/mAhTime/blob/master/gallery/Graph_232.JPG" height="400" width="650"></p>
<p align="center">232 Ohms</p>
