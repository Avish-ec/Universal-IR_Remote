"""
    Author : Avish Amin
    Date : 18/06/2022
    Database connectivity and data publishing to MQTT broker

"""

# Importing required modules
import mysql.connector
import random
import time
from paho.mqtt import client as mqtt
# Setting variables
msg = ""
state = True
broker = 'broker.hivemq.com'
port = 1883
topic = "MYdata"
prompt = "Enter a command : "


# This function is called when broker responds to our connection request
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected with result code ")
    else:
        print("Not connected")


# This function is called to publish the data
def pub(topic, msg):
    time.sleep(0.5)
    client.publish(topic, msg)


# Establishing connection with database
mydb = mysql.connector.connect(host="localhost", user="Avish", passwd="avi@123", database="code_values")

# Setting a loop which iterates until user enters a valid data
while state:                                        # State is initially True
    # Setting a pointer in database
    mycursor = mydb.cursor()
    mycursor.execute("select * from ir_values")
    # Ask user for command
    command = input(prompt)
    prompt = "Enter a valid command: "
    # Searches command in database table
    for i in mycursor:
        temp = list(i)                              # Converting raw values into list
        # If found the command in database change the state and store it's value in message
        if command.lower() == temp[0].lower():
            msg = temp[1]
            print("Got data " + str(msg))
            state = False                           # Changing loop state

# Setting a random client ID
client_id = f'python-mqtt-{random.randint(0, 1000)}'
client = mqtt.Client(client_id)
client.username_pw_set("Avish", "123456")
# Connecting to Broker
client.on_connect = on_connect
client.connect(broker, port, 60)
# Publishing messages
pub(topic, msg)                                      # Function to publish message
client.loop_start()                                  # he loop_start() starts a new thread, that calls the loop method at regular intervals