from homie.utils import get_unique_id


###
# Wifi settings
###

# Name of Wifi
WIFI_SSID = "Your_SSID"
# Password for the Wifi
WIFI_PASSWORD = "Your_PASSWORD"
###
# MQTT settings
###

# Broker IP or DNS Name
MQTT_BROKER = '192.168.1.24'

# Broker port
MQTT_PORT = 0

# Username or None for anonymous login
MQTT_USERNAME = None

# Password or None for anonymous login
MQTT_PASSWORD = None

# Defines the mqtt connection timemout in seconds
MQTT_KEEPALIVE = 60

# SSL connection to the broker. Some MicroPython implementations currently
# have problems with receiving mqtt messages over ssl connections.
MQTT_SSL = False
MQTT_SSL_PARAMS = {}

# Base mqtt topic the device publish and subscribes to, without leading slash.
# Base topic format is bytestring
MQTT_BASE_TOPIC = b'chibit'


###
# Device settings
###

# The device ID for registration at the broker. The device id is also the
# base topic of a device and must be unique
DEVICE_ID = get_unique_id()

# Friendly name of the device
DEVICE_NAME = b'gyro'

# Firmware name
DEVICE_FW_NAME = b'uhomie'

# Time in seconds the device updates device properties
DEVICE_STATS_INTERVAL = 60
