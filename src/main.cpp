#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// ===== WIFI STA =====
const char* ssid = "SMART_VEHICLE";
const char* password = "87PROJECT.AI";

// ===== RELAY LOW TRIGGER =====
#define R1 D1
#define R2 D2
#define R3 D3
#define R4 D4

ESP8266WebServer server(80);
bool relayState[4] = {0,0,0,0};

// ===== FUTURISTIC UI =====
String webpage(){
return R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Neo Relay Control</title>
<style>
body{
background:radial-gradient(circle at top,#061b3a,#010a18);
font-family:Arial;
text-align:center;
color:#bde9ff;
}
h1{color:#00c8ff;text-shadow:0 0 15px #00c8ff;}
.card{
background:rgba(0,40,90,.6);
margin:15px;
padding:20px;
border-radius:20px;
box-shadow:0 0 25px rgba(0,200,255,.3);
}
.btn{
width:170px;height:60px;
border-radius:40px;
border:none;
font-size:18px;
font-weight:bold;
transition:.2s;
}
.on{background:#00d2ff;color:#001;}
.off{background:#081622;color:#777;}
</style>

<script>
function toggleRelay(id){
fetch("/toggle?relay="+id)
.then(r=>r.text())
.then(state=>{
 let b=document.getElementById("btn"+id);
 b.innerHTML=state;
 b.className="btn "+(state=="ON"?"on":"off");
});
}

function refresh(){
for(let i=1;i<=4;i++){
 fetch("/status?relay="+i)
 .then(r=>r.text())
 .then(state=>{
  let b=document.getElementById("btn"+i);
  b.innerHTML=state;
  b.className="btn "+(state=="ON"?"on":"off");
 });
}
}
setInterval(refresh,2000);
</script>
</head>

<body onload="refresh()">

<h1>NEO RELAY PANEL</h1>

<div class="card">Relay 1<br><br>
<button id="btn1" class="btn off" onclick="toggleRelay(1)">OFF</button>
</div>

<div class="card">Relay 2<br><br>
<button id="btn2" class="btn off" onclick="toggleRelay(2)">OFF</button>
</div>

<div class="card">Relay 3<br><br>
<button id="btn3" class="btn off" onclick="toggleRelay(3)">OFF</button>
</div>

<div class="card">Relay 4<br><br>
<button id="btn4" class="btn off" onclick="toggleRelay(4)">OFF</button>
</div>

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

 // OFF awal (LOW trigger relay)
 digitalWrite(R1,HIGH);
 digitalWrite(R2,HIGH);
 digitalWrite(R3,HIGH);
 digitalWrite(R4,HIGH);

 // ===== WIFI CONNECT =====
 WiFi.begin(ssid,password);

 unsigned long t0 = millis();
 while(WiFi.status()!=WL_CONNECTED && millis()-t0<15000){
  delay(500);
  Serial.print(".");
 }

 if(WiFi.status()==WL_CONNECTED){
  Serial.println("\nConnected!");
  Serial.println(WiFi.localIP());
 }else{
  Serial.println("\nFAILED -> AP MODE");
  WiFi.softAP("WEMOS-RELAY","12345678");
  Serial.println(WiFi.softAPIP());
 }

 // ===== ROUTES =====
 server.on("/",[]{ server.send(200,"text/html",webpage()); });

 server.on("/toggle",[]{
  int r=server.arg("relay").toInt()-1;
  relayState[r]=!relayState[r];
  digitalWrite(relayPin(r),relayState[r]?LOW:HIGH);
  server.send(200,"text/plain",relayState[r]?"ON":"OFF");
 });

 server.on("/status",[]{
  int r=server.arg("relay").toInt()-1;
  server.send(200,"text/plain",relayState[r]?"ON":"OFF");
 });

 server.begin();
}

void loop(){
 server.handleClient();
}
