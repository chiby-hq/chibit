# Setup

* Install Adafruit ampy
```
sudo pip3 install adafruit-ampy
```
* Connect to the REPL and perform a first Wifi connection to install micropython homie
```
import network
wlan = network.WLAN(network.STA_IF)
wlan.active(True)
wlan.connect('wifi-name', 'wifi-secret')
# wait a few seconds
wlan.isconnected()  # test if wlan is connected
# answer should be : True
wlan.ifconfig()  # get wlan interface config
# answer should be something like : ('192.168.42.2', '255.255.255.0', '192.168.42.1', '192.168.42.1')
```
* Install micropython homie
```
import upip
upip.install('microhomie')
```
* Create the settings.py from template using the provided src/main/python/settings.example.py

* Upload settings and main
```
ampy --port /dev/ttyUSBx put src/main/python/main.py
ampy --port /dev/ttyUSBx put src/main/python/settings.py
```

* Startup an MQTT host
```
docker run -it -p 1883:1883 -p 9001:9001 eclipse-mosquitto
```
* And an MQTT client to dump readings :
```
docker run -it --net=host --rm efrecon/mqtt-client sub -h localhost -t "#" -v
```