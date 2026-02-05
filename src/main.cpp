#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "SMART VEHICLE";
const char* password = "87PROJECT.AI";

#define R1 D1
#define R2 D2
#define R3 D3
#define R4 D4

ESP8266WebServer server(80);
bool relayState[4] = {0,0,0,0};

String webpage() {
  return R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Smart Relay</title>
</head>
<body>
<h2>SMART RELAY PANEL</h2>
<button onclick="fetch('/toggle?relay=1')">Relay1</button>
<button onclick="fetch('/toggle?relay=2')">Relay2</button>
<button onclick="fetch('/toggle?relay=3')">Relay3</button>
<button onclick="fetch('/toggle?relay=4')">Relay4</button>
</body>
</html>
)rawliteral";
}

int relayPin(int r){
  return r==0?R1:r==1?R2:r==2?R3:R4;
}

void setup(){
  Serial.begin(9600);

  pinMode(R1,OUTPUT);
  pinMode(R2,OUTPUT);
  pinMode(R3,OUTPUT);
  pinMode(R4,OUTPUT);

  // relay LOW trigger -> OFF awal HIGH
  digitalWrite(R1,HIGH);
  digitalWrite(R2,HIGH);
  digitalWrite(R3,HIGH);
  digitalWrite(R4,HIGH);

  WiFi.begin(ssid,password);
  while(WiFi.status()!=WL_CONNECTED){
    delay(500);
  }

  server.on("/",[]{ server.send(200,"text/html",webpage()); });

  server.on("/toggle",[]{
    int r=server.arg("relay").toInt()-1;
    relayState[r]=!relayState[r];
    digitalWrite(relayPin(r),relayState[r]?LOW:HIGH);
    server.send(200,"text/plain","OK");
  });

  server.begin();
}

void loop(){
  server.handleClient();
}
