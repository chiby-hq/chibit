# Chibit Sensor Bracelet for ESP82xx

## Web application

Upon startup, the Chibit master node will startup an asynchronous Web Server that
can show the status of all connected bracelets and allow various interactions.
To develop the web application, you can invoke the maven build tool, that will
download and start a local web server. In this mode, bracelet interactions are simulated
by mouse clicks.

To assemble the web application :

* On Linux / Mac OSX :  ```./mvnw compile jetty:run```
* On Windows : ```mvnw.cmd compile jetty:run```

It will download all web libraries and resources and run a webserver at http://localhost:8080.
During execution, the `src/main/webapp` folder will be scanned and changes reflected in your running application (upon reloading the web page).

To prepare the `data` folder before upload to the SPIFFS image on the ESP8266 :

* On Linux / Mac OSX :  ```./mvnw package```
* On Windows : ```mvnw.cmd package```

Note that only minified resources will be copied, to the root of the file system.

## Bracelet documentation

### Sensor design principles
The bend sensor should be set against the wrist, palm and fingers vertical above the thumb and below the wrist.
The bend sensor's resistance increases with the curvature, we should measure the difference between two extreme positions and the difference of resistance.

:warning: Two different sensors might offer different resistance levels. Calibration data will be collected when the microcontroller starts up if needed (or if the onboard FLASH button is long-pressed).


### Sensor activity sequence diagram
```plantuml
@startuml
actor Alice
actor Bob
participant Node1
participant "Node n" as NodeN
participant "Wifi networks" as Networks
participant "Wifi AP" as AP
participant "MQTT broker" as MQTT
participant "Embedded Web Server" as WebServer

participant "Web Client" as WebClient

group Establish an access point
  Alice -> Node1: Reset
  Node1 -> Node1: Read config

  loop 3 seconds
    Node1 -> Networks: Scan
    Node1 -> Node1: Select network
  end

  alt Node1 finds no suitable infrastructure AP

    Node1 -> AP: Join
    Node1 -> WebServer : Start

  else Node1 finds suitable embedded AP

    Node1-> AP: Join

  else Node1 finds suitable infrastructure AP

    Node1 -> AP: Join
    Node1 -> AP: Locate MQTT broker
    Node1 -> MQTT: Advertise node (Name, Avatar, Color)

  end

  loop
    Node1 -> MQTT: Publish sensor values (ADC)
    Node1 -> MQTT: Update cluster metrics and status
  end

end

group Node n wakes up and joins
  Bob -> NodeN: Reset
  NodeN -> NodeN: Read config
  NodeN -> Networks: Scan
  NodeN -> AP: Join
  NodeN -> MQTT: Advertise node (Name, Avatar, Color)
  loop
    NodeN -> MQTT: Publish sensor values (ADC)
  end
end

group Web Client viewer connects
  WebClient -> WebServer: open index page
  WebClient -> MQTT: subscribe to cluster metrics and all sensor topics
  loop
    MQTT --> WebClient: Sensor update
    MQTT --> WebClient: Cluster metrics update
  end
end

@enduml
```

### Web application : Sensor Manager sequence diagram
```plantuml
@startuml
participant "Node 1" as Node1
participant "Node 2" as Node2
participant "Sensor Manager" as SM
participant "Web Client" as WC

group New node joins
  Node1 -> WC : send sensor reading
  WC -> SM : if sensor known ?

  alt Sensor not known
    SM -> SM : register node with default attributes
  end

end

group Update node details
  WC -> SM : update node details
end

group Web Client viewer connects
  WebClient -> WebServer: open index page
  WebClient -> MQTT: subscribe to cluster metrics and all sensor topics
  loop
    MQTT --> WebClient: Sensor update
    MQTT --> WebClient: Cluster metrics update
  end
end

@enduml
```



### Monitoring web interface

The monitoring web interface is used to check that sensors are publishing values correctly and currently connected to the MQTT broker.

```plantuml
@startuml
participant "Monitoring UI" as UI
participant "WS MQTT Broker" as Broker
UI -> Broker : Subscribe to all
Broker --> UI : OnMessage
activate UI
UI -> UI :
@enduml
```
