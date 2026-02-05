#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define R1 D1
#define R2 D2
#define R3 D3
#define R4 D4

const char* ssid = "SMART_VEHICLE";
const char* password = "87PROJECT.AI";

ESP8266WebServer server(80);
bool relayState[4] = {0,0,0,0};

// ================= WEBPAGE =================
String webpage(){
  String html = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  html += "<title>Neo Relay Control</title>";
  html += "<style>body{background:#010a18;color:#bde9ff;font-family:Arial;text-align:center;} h1{color:#00c8ff;} ";
  html += ".card{background:rgba(0,40,90,.6);margin:15px;padding:20px;border-radius:20px;} ";
  html += ".btn{width:170px;height:60px;border-radius:40px;border:none;font-size:18px;font-weight:bold;} ";
  html += ".on{background:#00d2ff;color:#001;} .off{background:#081622;color:#777;} ";
  html += "footer{margin-top:20px;color:#00c8ff;}</style></head><body>";
  html += "<h1>NEO RELAY PANEL</h1>";

  for(int i=0;i<4;i++){
    html += "<div class='card'>Relay "+String(i+1)+"<br><br>";
    html += "<button id='btn"+String(i+1)+"' class='btn off' onclick='toggleRelay(" + String(i+1) + ")'>OFF</button></div>";
  }

  html += "<div class='card'>Voice Control<br><br>";
  html += "<button onclick='startVoice()'>🎤 Bicara</button></div>";
  html += "<footer>Created by 87PROJECT.AI</footer>";

  html += "<script>";
  html += "function toggleRelay(id){ fetch('/toggle?relay='+id).then(r=>r.text()).then(state=>{ ";
  html += "let b=document.getElementById('btn'+id); b.innerHTML=state; b.className='btn '+(state=='ON'?'on':'off'); ";
  html += "let msg='Relay '+id+' '+(state=='ON'?'menyala':'mati'); speechSynthesis.speak(new SpeechSynthesisUtterance(msg)); ";
  html += "}); }";

  html += "function startVoice(){ if(!('webkitSpeechRecognition' in window)){alert('Browser tidak support!'); return;} ";
  html += "let rec=new webkitSpeechRecognition(); rec.lang='id-ID'; rec.onresult=function(e){let cmd=e.results[0][0].transcript.toLowerCase();";
  for(int i=1;i<=4;i++){
    html += "if(cmd.includes('nyalakan relay "+String(i)+"')||cmd.includes('relay "+String(i)+" on')) toggleRelay("+String(i)+"); ";
    html += "if(cmd.includes('matikan relay "+String(i)+"')||cmd.includes('relay "+String(i)+" off')) toggleRelay("+String(i)+"); ";
  }
  html += "}; rec.start(); }";
  html += "setInterval(()=>{ for(let i=1;i<=4;i++){ fetch('/status?relay='+i).then(r=>r.text()).then(state=>{ ";
  html += "let b=document.getElementById('btn'+i); b.innerHTML=state; b.className='btn '+(state=='ON'?'on':'off'); }); }},1000);";
  html += "</script></body></html>";
  return html;
}

// ================= UTILITY =================
int relayPin(int r){ return r==0?R1:r==1?R2:r==2?R3:R4; }

// ================= SETUP =================
void setup(){
  Serial.begin(115200);

  pinMode(R1,OUTPUT); pinMode(R2,OUTPUT);
  pinMode(R3,OUTPUT); pinMode(R4,OUTPUT);
  digitalWrite(R1,HIGH); digitalWrite(R2,HIGH); digitalWrite(R3,HIGH); digitalWrite(R4,HIGH);

  WiFi.begin(ssid,password);
  unsigned long t0=millis();
  while(WiFi.status()!=WL_CONNECTED && millis()-t0<15000){ delay(500); Serial.print("."); }
  if(WiFi.status()==WL_CONNECTED){ Serial.println("\nConnected!"); Serial.println(WiFi.localIP()); }
  else{ Serial.println("\nFAILED -> AP MODE"); WiFi.softAP("87PROJECT-RELAY","87Project.ai"); Serial.println(WiFi.softAPIP()); }

  server.on("/",[]{ server.send(200,"text/html",webpage()); });
  server.on("/toggle",[]{ 
    int r=server.arg("relay").toInt()-1;
    relayState[r]=!relayState[r];
    digitalWrite(relayPin(r), relayState[r]?LOW:HIGH);
    server.send(200,"text/plain",relayState[r]?"ON":"OFF");
  });
  server.on("/status",[]{ 
    int r=server.arg("relay").toInt()-1;
    server.send(200,"text/plain",relayState[r]?"ON":"OFF");
  });

  server.begin();
}

// ================= LOOP =================
void loop(){ server.handleClient(); }
