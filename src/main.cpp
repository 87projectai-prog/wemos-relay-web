#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>

#define R1 D1
#define R2 D2
#define R3 D3
#define R4 D4

const char* ssid = "SMART_VEHICLE";
const char* password = "87PROJECT.AI";

ESP8266WebServer server(80);
WebSocketsServer webSocket(81);

bool relayState[4] = {0,0,0,0};

// ================= WEBPAGE =================
String webpage(){
  return R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Neo Relay Control</title>
<style>
body{background:radial-gradient(circle at top,#061b3a,#010a18);font-family:Arial;text-align:center;color:#bde9ff;margin:0;padding:0;}
h1{color:#00c8ff;text-shadow:0 0 15px #00c8ff;}
.card{background:rgba(0,40,90,.6);margin:15px;padding:20px;border-radius:20px;box-shadow:0 0 25px rgba(0,200,255,.3);}
.btn{width:170px;height:60px;border-radius:40px;border:none;font-size:18px;font-weight:bold;transition:.2s;}
.on{background:#00d2ff;color:#001;}
.off{background:#081622;color:#777;}
footer{margin-top:20px;padding:10px;font-size:14px;color:#00c8ff;text-shadow:0 0 5px #00c8ff;}
</style>
<script>
let ws = new WebSocket('ws://'+location.hostname+':81');
ws.onmessage = function(event){
  let [relay,state] = event.data.split(':');
  let b = document.getElementById("btn"+relay);
  b.innerHTML = state;
  b.className="btn "+(state=="ON"?"on":"off");
  speakFeedback(relay,state);
};

function toggleRelay(id){
  fetch("/toggle?relay="+id)
  .then(r=>r.text())
  .then(state=>{
    let b=document.getElementById("btn"+id);
    b.innerHTML=state;
    b.className="btn "+(state=="ON"?"on":"off");
    speakFeedback(id,state);
  });
}

function startVoice() {
  if(!('webkitSpeechRecognition' in window)){alert("Browser tidak mendukung voice recognition!");return;}
  let recognition = new webkitSpeechRecognition();
  recognition.lang = 'id-ID';
  recognition.interimResults=false;
  recognition.maxAlternatives=1;
  recognition.onresult=function(event){
    let command=event.results[0][0].transcript.toLowerCase();
    let relayId=0,state="";
    if(command.includes("nyalakan relay 1")||command.includes("hidupkan relay 1")) relayId=1,state="ON";
    else if(command.includes("matikan relay 1")||command.includes("off relay 1")) relayId=1,state="OFF";
    else if(command.includes("nyalakan relay 2")||command.includes("hidupkan relay 2")) relayId=2,state="ON";
    else if(command.includes("matikan relay 2")||command.includes("off relay 2")) relayId=2,state="OFF";
    else if(command.includes("nyalakan relay 3")||command.includes("hidupkan relay 3")) relayId=3,state="ON";
    else if(command.includes("matikan relay 3")||command.includes("off relay 3")) relayId=3,state="OFF";
    else if(command.includes("nyalakan relay 4")||command.includes("hidupkan relay 4")) relayId=4,state="ON";
    else if(command.includes("matikan relay 4")||command.includes("off relay 4")) relayId=4,state="OFF";
    else if(command.includes("relay 1 on")||command.includes("turn on relay 1")) relayId=1,state="ON";
    else if(command.includes("relay 1 off")||command.includes("turn off relay 1")) relayId=1,state="OFF";
    else if(command.includes("relay 2 on")||command.includes("turn on relay 2")) relayId=2,state="ON";
    else if(command.includes("relay 2 off")||command.includes("turn off relay 2")) relayId=2,state="OFF";
    else if(command.includes("relay 3 on")||command.includes("turn on relay 3")) relayId=3,state="ON";
    else if(command.includes("relay 3 off")||command.includes("turn off relay 3")) relayId=3,state="OFF";
    else if(command.includes("relay 4 on")||command.includes("turn on relay 4")) relayId=4,state="ON";
    else if(command.includes("relay 4 off")||command.includes("turn off relay 4")) relayId=4,state="OFF";
    if(relayId>0) toggleRelay(relayId);
    else alert("Perintah tidak dikenali: "+command);
  };
  recognition.start();
}

function speakFeedback(relay,state){
  let msg = "Relay " + relay + " " + (state=="ON"?"menyala":"mati");
  let utter = new SpeechSynthesisUtterance(msg);
  utter.lang = "id-ID";
  speechSynthesis.speak(utter);
}
</script>
</head>
<body>
<h1>NEO RELAY PANEL</h1>
<div class="card">Relay 1<br><br><button id="btn1" class="btn off" onclick="toggleRelay(1)">OFF</button></div>
<div class="card">Relay 2<br><br><button id="btn2" class="btn off" onclick="toggleRelay(2)">OFF</button></div>
<div class="card">Relay 3<br><br><button id="btn3" class="btn off" onclick="toggleRelay(3)">OFF</button></div>
<div class="card">Relay 4<br><br><button id="btn4" class="btn off" onclick="toggleRelay(4)">OFF</button></div>
<div class="card">Voice Control<br><br><button id="voiceBtn" class="btn off" onclick="startVoice()">🎤 Bicara</button></div>
<footer>Created by 87PROJECT.AI</footer>
</body>
</html>
)rawliteral";
}

// =================== UTILITY ===================
int relayPin(int r){ return r==0?R1:r==1?R2:r==2?R3:R4; }
void sendRelayStatus(int r){ webSocket.broadcastTXT(String(r+1)+":"+(relayState[r]?"ON":"OFF")); }

// =================== SETUP ===================
void setup(){
  Serial.begin(115200);

  pinMode(R1,OUTPUT); pinMode(R2,OUTPUT);
  pinMode(R3,OUTPUT); pinMode(R4,OUTPUT);
  digitalWrite(R1,HIGH); digitalWrite(R2,HIGH); digitalWrite(R3,HIGH); digitalWrite(R4,HIGH);

  WiFi.begin(ssid,password);
  unsigned long t0 = millis();
  while(WiFi.status()!=WL_CONNECTED && millis()-t0<15000){ delay(500); Serial.print("."); }
  if(WiFi.status()==WL_CONNECTED){ Serial.println("\nConnected!"); Serial.println(WiFi.localIP()); }
  else{ Serial.println("\nFAILED -> AP MODE"); WiFi.softAP("87PROJECT-RELAY","87PROJECT.AI"); Serial.println(WiFi.softAPIP()); }

  server.on("/",[]{ server.send(200,"text/html",webpage()); });
  server.on("/toggle",[]{ 
    int r=server.arg("relay").toInt()-1;
    relayState[r]=!relayState[r];
    digitalWrite(relayPin(r), relayState[r]?LOW:HIGH);
    sendRelayStatus(r);
    server.send(200,"text/plain",relayState[r]?"ON":"OFF"); 
  });
  server.on("/status",[]{ 
    int r=server.arg("relay").toInt()-1;
    server.send(200,"text/plain",relayState[r]?"ON":"OFF"); 
  });

  server.begin();
  webSocket.begin();
  webSocket.onEvent([](uint8_t, WStype_t, uint8_t*, size_t){});
}

// =================== LOOP ===================
void loop(){
  server.handleClient();
  webSocket.loop();
}
