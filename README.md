# chibit
A connected bracelet for Chiby based on ESP8266


# Setup procedure


## Setup the Chibit Web Service server

### on a Raspberry Pi

* Copy the following config to /etc/avahi/services/chibitws.service

```xml
<!DOCTYPE service-group SYSTEM "avahi-service.dtd">
<service-group>
 <name>chibitws</name>
 <service>
   <type>_chibitws._tcp</type>
   <port>8080</port>
   <txt-record>info=Websocket Chibit broker</txt-record>
</service>
</service-group>

```

* Restart the avahi service

* Start the application

```
java -jar chibit-web-1.0.jar
```

