import paho.mqtt.client as mqtt
import psycopg2
from psycopg2.extras import execute_values
from datetime import datetime
import pytz
import os
from dotenv import load_dotenv

# Load environment variables from .env file
load_dotenv()

# MQTT Broker Settings
broker = os.getenv("MQTT_BROKER")
port = int(os.getenv("MQTT_PORT"))
username = os.getenv("MQTT_USERNAME")
password = os.getenv("MQTT_PASSWORD")

# MQTT Topics
topics = ["sensor/data", "hello/topic", "sensor/temperature", "sensor/humidity", "sensor/datetime"]

# PostgreSQL Database Settings
db_config = {
    "dbname": os.getenv("DB_NAME"),
    "user": os.getenv("DB_USER"),
    "password": os.getenv("DB_PASSWORD"),
    "host": os.getenv("DB_HOST"),
    "port": int(os.getenv("DB_PORT"))
}

# Callback function when connecting to the broker
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT Broker!")
        for topic in topics:
            client.subscribe(topic)
            print(f"Subscribed to {topic}")
    else:
        print(f"Failed to connect, return code {rc}")

# Callback function when a message is received
def on_message(client, userdata, msg):
    print(f"Message received on topic: {msg.topic}")
    payload = msg.payload.decode()
    print(f"Payload: {payload}")
    print("-----------------------")

    # Insert the received data into PostgreSQL
    insert_to_db(msg.topic, payload)

# Insert data into PostgreSQL
def insert_to_db(topic, payload):
    try:
        # Establish database connection
        conn = psycopg2.connect(**db_config)
        cursor = conn.cursor()

        jakarta_tz = pytz.timezone('Asia/Jakarta')
        current_time = datetime.now(jakarta_tz)

        # Insert data
        query = """
        INSERT INTO mqtt_data (topic, payload, received_at)
        VALUES (%s, %s, %s);
        """
        data = (topic, payload, current_time)
        cursor.execute(query, data)

        # Commit and close
        conn.commit()
        print("Data inserted into PostgreSQL")
        cursor.close()
        conn.close()
    except Exception as e:
        print(f"Error inserting data into PostgreSQL: {e}")

# Initialize the MQTT Client
client = mqtt.Client(client_id="PythonSubscriber", protocol=mqtt.MQTTv311)

# Set username and password
client.username_pw_set(username, password)

# Assign callback functions
client.on_connect = on_connect
client.on_message = on_message

# Connect to the broker
print(f"Connecting to MQTT Broker at {broker}:{port}...")
client.connect(broker, port)

# Keep the script running to listen for messages
try:
    client.loop_forever()
except KeyboardInterrupt:
    print("Exiting...")
    client.disconnect()
