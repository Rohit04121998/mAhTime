import paho.mqtt.client as mqtt
import csv

# Don't forget to change the variables for the MQTT broker!
mqtt_username = "Your_UserName"
mqtt_password = "MQTT_Password"
mqtt_topic = "Your_Topic"
mqtt_broker_ip = "Broker_IP" 
file_name = ''      # Globalizing the filename
time_elapsed = 0
client = mqtt.Client()
# Set the username and password for the MQTT client
client.username_pw_set(mqtt_username, mqtt_password)

def on_connect(client, userdata, flags, rc):
    # rc is the error code returned when connecting to the broker
    print ("Connected!" + str(rc))
    client.subscribe(mqtt_topic)
    
def on_message(client, userdata, msg):
    message = msg.payload.decode()
    global file_name,time_elapsed       # file_name and time_elapsed are the global varibles created earlier
    if(message[-4:] == ".csv"):         # create a new file if a .csv extension is found in the message
        message = message[:-10] + 'H' + message[-9:-7] + 'M' +message[-6:-4] + 'S' +message[-4:]
        # The message wil be of the form deviceID+load+date+time+.csv
        # The file_name includes load+date+time+.csv
        file_name = message[9:]
        # Create a .csv file and write the fie details in the first two lines
        with open(file_name, 'w') as csvFile:
            time_elapsed = 0
            writer = csv.writer(csvFile)
            list_1 = []
            list_2 = ['TimeElapsed', 'Value']
            list_1.append("Device ID:-" + message[:9] + ", FileName:-" + file_name + ", Load:-" + file_name[:3])
            writer.writerow(list_1)
            writer.writerow(list_2)
            csvFile.close()
    else:
        with open(file_name, 'a') as csvFile:
            writer = csv.writer(csvFile)
            if(message == "Fully Discharged"):
                v = message
            else:
                v = float(msg.payload.decode())
            b = [time_conversion(time_elapsed)]
            b.append(v)
            writer.writerow(b)
            time_elapsed += 4
            csvFile.close()
# A fuunction to convert time in seconds to time in hr:min:sec format    
def time_conversion(d):
    if(d >= 60 and d < 3600):
        d1 = d // 60
        d2 = d % 60
        if(d2 == 0):
            hours = 0
            minute = d1
            sec = 0
        else:
            hours = 0
            minute = d1
            sec = d2
    elif(d >= 3600):
        d1 = d // 3600
        d2 = d % 3600
        if(d2 < 60):
            hours = d1
            minute = 0
            sec = d2
        elif(d2 >= 60 and d2 < 3600):
            d3 = d2 // 60
            d4 = d2 % 60
            if(d4 == 0):
                hours = d1
                minute = d3
                sec = 0
            else:
                hours = d1
                minute = d3
                sec = d4
    else:
        hours = 0
        minute = 0
        sec = d
    time = "{}:{}:{}".format(hours,minute,sec)
    return time   
       
# Here, we are telling the client which functions are to be run
# on connecting, and on receiving a message
client.on_connect = on_connect
client.on_message = on_message


client.connect(mqtt_broker_ip, 1883)

# Once we have told the client to connect, let the client object run itself
client.loop_forever()
client.disconnect()
