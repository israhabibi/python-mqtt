# MQTT Listener with PostgreSQL Insertion and Supervisor Setup

This project listens to MQTT messages on specific topics, processes them, and inserts the data into a PostgreSQL database. It uses `paho.mqtt.client` to connect to the MQTT broker and `psycopg2` to interact with PostgreSQL. The project is configured to run in a production environment using `supervisord` for process management.

## Prerequisites

Before running the project, ensure you have the following installed:

- Python 3.6+ (preferably in a virtual environment)
- PostgreSQL database
- MQTT broker (e.g., Mosquitto)
- `pip` (Python package installer)

### Dependencies

Install the required Python packages using `pip`:

```bash
pip install paho-mqtt psycopg2 python-dotenv pytz
```

## Setup

### 1. **Configuration via .env**

This project uses a `.env` file to securely manage environment variables. Create a `.env` file in the root of the project directory and include the following configuration:

```plaintext
# MQTT Broker Settings
MQTT_BROKER=
MQTT_PORT=
MQTT_USERNAME=
MQTT_PASSWORD=

# PostgreSQL Database Settings
DB_NAME=
DB_USER=
DB_PASSWORD=
DB_HOST=
DB_PORT=
```

The script will use these environment variables to connect to the MQTT broker and the PostgreSQL database.

### 2. **Python Script**

The main script (`mqtt_subscriber.py`) listens for messages on the defined MQTT topics, processes the received data, and inserts it into the PostgreSQL database.

Here's how the script works:

1. **Connect to the MQTT Broker:** The script connects to an MQTT broker (configured via environment variables) and subscribes to the defined topics.
2. **Receive and Process Messages:** Whenever a message is received on a subscribed topic, it extracts the payload and inserts it into the PostgreSQL database with a timestamp.
3. **Database Insertion:** The script inserts the received data into the `mqtt_data` table in the PostgreSQL database.

### 3. **Running the Script**

To run the script manually, use:

```bash
python mqtt_subscriber.py
```

### 4. **Running the Script with Supervisor**

In a production environment, you may want to ensure that the script runs continuously and is automatically restarted in case of failure. This is where `supervisord` comes in.

#### Install Supervisor

On Ubuntu/Debian-based systems, use the following:

```bash
sudo apt install supervisor
```

#### Supervisor Configuration

Create a configuration file for the script in `/etc/supervisor/supervisord.conf`

```ini
sudo nano /etc/supervisor/supervisord.conf

add program

[program:mqtt_listener]
command=/home/gws/project/python-mqtt/mqtt_subscriber/mqtt_subscriber.sh
directory=/home/gws/project/python-mqtt/mqtt_subscriber
autostart=true
autorestart=true
stderr_logfile=/home/gws/project/python-mqtt/mqtt_subscriber/log/mqtt_subscriber.err.log
stdout_logfile=/home/gws/project/python-mqtt/mqtt_subscriber/log/mqtt_subscriber.out.log
stopasgroup=true
killasgroup=true
```

#### Update Supervisor

Once the configuration is added, update `supervisord` to read the new configuration:

```bash
sudo supervisorctl reread
sudo supervisorctl update
```

#### Start the Program

To start the program, use:

```bash
sudo supervisorctl start mqtt_subscriber
```

#### Checking Logs

You can check the logs using:

```bash
sudo tail -f /home/gws/project/python-mqtt/mqtt_subscriber/log/mqtt_subscriber.out.log
sudo tail -f /home/gws/project/python-mqtt/mqtt_subscriber/log/mqtt_subscriber.err.log
```

This will help you troubleshoot any issues.

### 5. **Make Supervisor Start Automatically**

To make sure `supervisord` starts automatically on system boot, use:

```bash
sudo systemctl enable supervisor
```

### 6. **Stopping the Service**

To stop the service, use:

```bash
sudo supervisorctl stop mqtt_listener
```

### 7. **Database Table**

The script inserts data into the `mqtt_data` table. You should have the following table in your PostgreSQL database:

```sql
CREATE TABLE mqtt_data (
    id SERIAL PRIMARY KEY,
    topic TEXT NOT NULL,
    payload TEXT NOT NULL,
    received_at TIMESTAMP NOT NULL
);
```

## Conclusion

This setup will allow your MQTT listener script to run in the background as a service, continuously processing and storing MQTT messages in PostgreSQL. `supervisord` ensures that the script is always running, automatically restarting it if needed.

```

### Key Points in the README:
1. **Environment Variables:** Storing configuration securely in `.env`.
2. **Supervisor Setup:** Detailed instructions on setting up `supervisord` to keep the script running.
3. **Database Table:** SQL command for creating the `mqtt_data` table in PostgreSQL.
4. **Logging and Monitoring:** How to access logs for debugging.
