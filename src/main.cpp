#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/* ===== ESP SDK (RF FIX) ===== */
extern "C" {
  #include "user_interface.h"
}

/* ============ CONFIG ============ */
#define WIFI_SSID     "Leicha"
#define WIFI_PASSWORD "87Project.ai"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET   -1

#define NAV_TIMEOUT 15000
/* ================================= */

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
ESP8266WebServer server(80);

/* ============ STATE ============= */
enum Mode { MODE_EYE, MODE_NAV };
Mode mode = MODE_EYE;

unsigned long lastNav = 0;
unsigned long lastFrame = 0;

/* ========== NAV DATA ============ */
String navDir = "STRAIGHT";
int navDist = 0;
String navRoad = "";

/* ======= ANIMATION VARS ======== */
int pulse = 0;
bool pulseUp = true;

int eyeOffset = 0;
bool eyeDir = true;

bool blink = false;
unsigned long lastBlink = 0;

/* ============ WIFI ============== */
void initWiFi() {
  Serial.println("\n[WIFI INIT]");

  WiFi.persistent(false);
  wifi_set_opmode(STATION_MODE);
  WiFi.mode(WIFI_STA);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.disconnect();
  delay(300);

  Serial.print("Scanning...");
  int n = WiFi.scanNetworks();
  Serial.println();

  for (int i = 0; i < n; i++) {
    Serial.printf(" %d: %s (%d dBm)\n",
      i + 1,
      WiFi.SSID(i).c_str(),
      WiFi.RSSI(i));
  }

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting");

  unsigned long t = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t < 20000) {
    Serial.print(".");
    delay(300);
    yield();
  }

  Serial.println();
  Serial.print("WiFi status: ");
  Serial.println(WiFi.status());

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  }
}

/* ========= CYBER EYE ============ */
void drawEye() {
  display.clearDisplay();

  int cx = 64 + eyeOffset;
  int cy = 32;

  display.drawCircle(64, 32, 28, SSD1306_WHITE);
  display.drawCircle(64, 32, 26, SSD1306_WHITE);

  if (blink) {
    display.fillRect(36, 30, 56, 4, SSD1306_WHITE);
  } else {
    display.fillCircle(cx, cy, 10, SSD1306_WHITE);
    display.fillCircle(cx, cy, 5, SSD1306_BLACK);

    for (int i = -24; i <= 24; i += 6) {
      display.drawPixel(64 + i, 32, SSD1306_WHITE);
    }
  }

  display.display();
}

/* ============ NAV HUD ============ */
void drawNav() {
  display.clearDisplay();

  int ox = pulse;
  int oy = pulse;

  if (navDir == "LEFT") {
    display.fillTriangle(30 + ox, 32, 60 + ox, 10, 60 + ox, 54, SSD1306_WHITE);
  } else if (navDir == "RIGHT") {
    display.fillTriangle(98 - ox, 32, 68 - ox, 10, 68 - ox, 54, SSD1306_WHITE);
  } else {
    display.fillTriangle(64, 8 + oy, 40, 40 + oy, 88, 40 + oy, SSD1306_WHITE);
  }

  display.setTextSize(1);
  display.setCursor(0, 48);
  display.print(navRoad);

  display.setCursor(92, 48);
  display.print(navDist);
  display.print("m");

  display.display();
}

/* ========= HTTP INPUT ============ */
void handleNav() {
  navDir = server.arg("dir");
  navDist = server.arg("dist").toInt();
  navRoad = server.arg("road");

  lastNav = millis();
  mode = MODE_NAV;

  server.send(200, "text/plain", "OK");
}

/* ============== SETUP ============== */
void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.print("Free heap: ");
  Serial.println(ESP.getFreeHeap());

  /* === WIFI FIRST (CRITICAL) === */
  initWiFi();

  /* === OLED AFTER WIFI === */
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (1) yield();
  }

  display.clearDisplay();
  display.display();

  server.on("/nav", handleNav);
  server.begin();
}

/* =============== LOOP =============== */
void loop() {
  server.handleClient();

  if (millis() - lastFrame < 50) return;
  lastFrame = millis();

  pulse += pulseUp ? 1 : -1;
  if (pulse > 4 || pulse < -4) pulseUp = !pulseUp;

  eyeOffset += eyeDir ? 1 : -1;
  if (eyeOffset > 6 || eyeOffset < -6) eyeDir = !eyeDir;

  if (millis() - lastBlink > random(3000, 6000)) {
    blink = !blink;
    lastBlink = millis();
  }

  if (mode == MODE_NAV && millis() - lastNav > NAV_TIMEOUT) {
    mode = MODE_EYE;
  }

  if (mode == MODE_NAV) drawNav();
  else drawEye();
}
