#include <WiFi.h>
#include <WebServer.h>
#include "config.h"

// Node 3: fog/edge gateway, local processing, dashboard and alerts.
static const int RED_LED_PIN = 14;
static const int GREEN_LED_PIN = 12;
static const int RELAY_PIN = 26;

static const float TEMP_WARNING_DELTA = 2.0F;
static const float TEMP_CRITICAL_DELTA = 4.0F;
static const float GAS_WARNING_RATIO = 1.25F;
static const float GAS_CRITICAL_RATIO = 1.50F;
static const unsigned long NODE_TIMEOUT_MS = 9000;
static const unsigned long BASELINE_SAMPLES = 12;

WebServer server(80);

struct Node1State {
  float temp = NAN;
  float pressure = NAN;
  float lux = NAN;
  int ldr = 0;
  int gas = 0;
  unsigned long lastSeen = 0;
  unsigned long samples = 0;
};

struct Node2State {
  int motion = 0;
  int sound = 0;
  int ldr = 0;
  unsigned long lastSeen = 0;
  unsigned long samples = 0;
};

Node1State node1;
Node2State node2;

float tempSum = 0.0F;
float gasSum = 0.0F;
unsigned long baselineCount = 0;
float tempBaseline = NAN;
float gasBaseline = NAN;

int riskScore = 0;
String riskStatus = "NORMAL";
String riskReason = "System operating within learned limits.";

bool node1Online() { return node1.lastSeen > 0 && millis() - node1.lastSeen <= NODE_TIMEOUT_MS; }
bool node2Online() { return node2.lastSeen > 0 && millis() - node2.lastSeen <= NODE_TIMEOUT_MS; }

String fjson(float v, int digits = 2) {
  if (!isfinite(v)) return "null";
  return String(v, digits);
}

void updateBaseline() {
  if (!isfinite(node1.temp) || node1.gas <= 0) return;
  if (baselineCount >= BASELINE_SAMPLES) return;
  tempSum += node1.temp;
  gasSum += node1.gas;
  baselineCount++;
  if (baselineCount == BASELINE_SAMPLES) {
    tempBaseline = tempSum / BASELINE_SAMPLES;
    gasBaseline = gasSum / BASELINE_SAMPLES;
    Serial.printf("[Gateway] Baseline learned: temp %.2f C, gas %.1f ADC\n", tempBaseline, gasBaseline);
  }
}

void evaluateRisk() {
  int score = 0;
  String reason;

  if (!node1Online() || !node2Online()) {
    score = max(score, 1);
    reason += "Node connectivity warning. ";
  }

  if (node1Online() && isfinite(tempBaseline) && isfinite(node1.temp)) {
    float delta = node1.temp - tempBaseline;
    if (delta >= TEMP_CRITICAL_DELTA) {
      score += 2;
      reason += "Critical temperature rise. ";
    } else if (delta >= TEMP_WARNING_DELTA) {
      score += 1;
      reason += "Temperature above baseline. ";
    }
  }

  if (node1Online() && isfinite(gasBaseline) && gasBaseline > 1.0F) {
    float ratio = (float)node1.gas / gasBaseline;
    if (ratio >= GAS_CRITICAL_RATIO) {
      score += 2;
      reason += "Strong MQ135 signal elevation. ";
    } else if (ratio >= GAS_WARNING_RATIO) {
      score += 1;
      reason += "MQ135 signal elevated. ";
    }
  }

  if (node2Online() && node2.motion && node2.sound) {
    score += 2;
    reason += "Motion + sound context detected. ";
  } else if (node2Online() && node2.motion) {
    score += 1;
    reason += "Motion detected. ";
  } else if (node2Online() && node2.sound) {
    score += 1;
    reason += "Sound event detected. ";
  }

  riskScore = min(score, 6);
  if (riskScore >= 4) riskStatus = "CRITICAL";
  else if (riskScore >= 2) riskStatus = "WARNING";
  else riskStatus = "NORMAL";

  riskReason = reason.length() ? reason : "System operating within learned limits.";

  digitalWrite(RED_LED_PIN, riskScore >= 2 ? HIGH : LOW);
  digitalWrite(GREEN_LED_PIN, riskScore < 2 ? HIGH : LOW);
  digitalWrite(RELAY_PIN, riskScore >= 4 ? HIGH : LOW);
}

void handleNode1() {
  if (!server.hasArg("temp") || !server.hasArg("pressure") || !server.hasArg("lux") || !server.hasArg("ldr") || !server.hasArg("gas")) {
    server.send(400, "text/plain", "Missing node1 arguments");
    return;
  }
  node1.temp = server.arg("temp").toFloat();
  node1.pressure = server.arg("pressure").toFloat();
  node1.lux = server.arg("lux").toFloat();
  node1.ldr = server.arg("ldr").toInt();
  node1.gas = server.arg("gas").toInt();
  node1.lastSeen = millis();
  node1.samples++;
  updateBaseline();
  evaluateRisk();
  server.send(200, "text/plain", "NODE1_OK");
}

void handleNode2() {
  if (!server.hasArg("motion") || !server.hasArg("sound") || !server.hasArg("ldr")) {
    server.send(400, "text/plain", "Missing node2 arguments");
    return;
  }
  node2.motion = server.arg("motion").toInt() ? 1 : 0;
  node2.sound = server.arg("sound").toInt() ? 1 : 0;
  node2.ldr = server.arg("ldr").toInt();
  node2.lastSeen = millis();
  node2.samples++;
  evaluateRisk();
  server.send(200, "text/plain", "NODE2_OK");
}

void handleState() {
  String json;
  json.reserve(850);
  json += "{\"node1\":{";
  json += "\"temp\":" + fjson(node1.temp) + ",";
  json += "\"pressure\":" + fjson(node1.pressure) + ",";
  json += "\"lux\":" + fjson(node1.lux) + ",";
  json += "\"ldr\":" + String(node1.ldr) + ",";
  json += "\"gas\":" + String(node1.gas) + ",";
  json += "\"online\":" + String(node1Online() ? "true" : "false") + ",";
  json += "\"age_ms\":" + String(node1.lastSeen ? millis()-node1.lastSeen : 0) + ",";
  json += "\"samples\":" + String(node1.samples);
  json += "},\"node2\":{";
  json += "\"motion\":" + String(node2.motion) + ",";
  json += "\"sound\":" + String(node2.sound) + ",";
  json += "\"ldr\":" + String(node2.ldr) + ",";
  json += "\"online\":" + String(node2Online() ? "true" : "false") + ",";
  json += "\"age_ms\":" + String(node2.lastSeen ? millis()-node2.lastSeen : 0) + ",";
  json += "\"samples\":" + String(node2.samples);
  json += "},\"system\":{";
  json += "\"risk\":" + String(riskScore) + ",";
  json += "\"status\":\"" + riskStatus + "\",";
  json += "\"reason\":\"" + riskReason + "\",";
  json += "\"temp_baseline\":" + fjson(tempBaseline) + ",";
  json += "\"gas_baseline\":" + fjson(gasBaseline,1) + ",";
  json += "\"uptime_s\":" + String(millis()/1000UL);
  json += "}}";
  server.send(200, "application/json", json);
}

void handleRoot() {
  static const char PAGE[] PROGMEM = R"HTML(
<!doctype html><html><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Fog & Edge Environmental Monitor</title>
<style>
:root{--bg:#07111e;--p:#0d1a2b;--p2:#12243a;--t:#eaf2ff;--m:#9db1ca;--l:#223853;--ok:#36d399;--w:#f8c95c;--bad:#ff647c;--a:#68a8ff}*{box-sizing:border-box}body{margin:0;background:linear-gradient(160deg,#06101b,#0a1728 50%,#07111e);color:var(--t);font:14px system-ui,Segoe UI,sans-serif}.wrap{max-width:1250px;margin:auto;padding:20px}.top{display:flex;justify-content:space-between;align-items:flex-start;gap:16px;margin-bottom:16px}.title h1{margin:0 0 5px;font-size:27px}.title p{margin:0;color:var(--m)}.status{border:1px solid var(--l);border-radius:999px;padding:8px 12px;font-weight:800;font-size:12px}.grid{display:grid;grid-template-columns:repeat(4,1fr);gap:12px}.panel{background:rgba(13,26,43,.92);border:1px solid var(--l);border-radius:18px;padding:15px}.full{grid-column:1/-1}.wide{grid-column:span 2}.panel h2{margin:0 0 12px;font-size:15px}.cards{display:grid;grid-template-columns:repeat(5,1fr);gap:9px}.kpi{background:var(--p2);border:1px solid var(--l);border-radius:13px;padding:10px}.name{color:var(--m);font-size:11px}.value{font-size:19px;font-weight:800;margin-top:4px}.row{display:flex;justify-content:space-between;gap:10px;padding:9px 0;border-bottom:1px solid var(--l)}.row:last-child{border:0}.muted{color:var(--m)}canvas{width:100%;height:180px;background:#0a1524;border:1px solid var(--l);border-radius:11px}.foot{color:var(--m);font-size:11px;line-height:1.6;margin-top:10px}.pill{display:inline-block;padding:6px 9px;border-radius:999px;font-size:11px;font-weight:900}.normal{background:#36d39922;color:var(--ok)}.warning{background:#f8c95c22;color:var(--w)}.critical{background:#ff647c22;color:var(--bad)}
@media(max-width:950px){.grid{grid-template-columns:repeat(2,1fr)}.cards{grid-template-columns:repeat(3,1fr)}}@media(max-width:620px){.wrap{padding:12px}.grid{grid-template-columns:1fr}.wide{grid-column:span 1}.cards{grid-template-columns:repeat(2,1fr)}.top{flex-direction:column}}
</style></head><body><div class="wrap"><div class="top"><div class="title"><h1>Fog & Edge Environmental Monitoring</h1><p>3-node ESP32 network • local processing • context-aware alerts</p></div><div id="conn" class="status">CONNECTING</div></div>
<div class="grid"><section class="panel full"><h2>System status</h2><div class="cards"><div class="kpi"><div class="name">Risk state</div><div class="value" id="risk">--</div></div><div class="kpi"><div class="name">Score</div><div class="value" id="score">--</div></div><div class="kpi"><div class="name">Node 1</div><div class="value" id="n1">--</div></div><div class="kpi"><div class="name">Node 2</div><div class="value" id="n2">--</div></div><div class="kpi"><div class="name">Uptime</div><div class="value" id="up">--</div></div></div><div class="row"><span class="muted">Alert explanation</span><strong id="reason">--</strong></div></section>
<section class="panel wide"><h2>Node 1 • Environmental</h2><div class="cards"><div class="kpi"><div class="name">Temperature</div><div class="value" id="temp">--</div></div><div class="kpi"><div class="name">Pressure</div><div class="value" id="pressure">--</div></div><div class="kpi"><div class="name">BH1750</div><div class="value" id="lux">--</div></div><div class="kpi"><div class="name">LDR</div><div class="value" id="ldr1">--</div></div><div class="kpi"><div class="name">MQ135</div><div class="value" id="gas">--</div></div></div><div class="foot">MQ135 is shown as raw ADC/relative signal. PPM conversion requires calibration.</div></section>
<section class="panel wide"><h2>Node 2 • Safety</h2><div class="cards"><div class="kpi"><div class="name">Motion</div><div class="value" id="motion">--</div></div><div class="kpi"><div class="name">Sound</div><div class="value" id="sound">--</div></div><div class="kpi"><div class="name">LDR</div><div class="value" id="ldr2">--</div></div></div><div class="foot">Tune the sound module potentiometer for the desired threshold.</div></section>
<section class="panel wide"><h2>Temperature</h2><canvas id="ct" width="700" height="180"></canvas></section><section class="panel wide"><h2>MQ135 signal</h2><canvas id="cg" width="700" height="180"></canvas></section><section class="panel wide"><h2>Light</h2><canvas id="cl" width="700" height="180"></canvas></section><section class="panel wide"><h2>Safety events</h2><canvas id="cs" width="700" height="180"></canvas></section>
<section class="panel full"><h2>Gateway diagnostics</h2><div class="row"><span>Node 1 packet age</span><strong id="age1">--</strong></div><div class="row"><span>Node 2 packet age</span><strong id="age2">--</strong></div><div class="row"><span>Temperature baseline</span><strong id="tb">--</strong></div><div class="row"><span>MQ135 baseline</span><strong id="gb">--</strong></div></section></div><div class="foot">Prototype for educational demonstration. Use safe, controlled test conditions and only properly powered low-voltage loads on relay contacts.</div></div>
<script>
const h={t:[],g:[],l:[],s:[]},$=x=>document.getElementById(x);function push(a,v){a.push(v);if(a.length>40)a.shift()}function fmt(v,d=1){return v==null||Number.isNaN(Number(v))?'--':Number(v).toFixed(d)}function draw(id,a){const c=$(id),x=c.getContext('2d'),w=c.clientWidth*devicePixelRatio,h=c.clientHeight*devicePixelRatio;c.width=w;c.height=h;x.clearRect(0,0,w,h);x.strokeStyle='#223853';for(let i=1;i<5;i++){x.beginPath();x.moveTo(0,i*h/5);x.lineTo(w,i*h/5);x.stroke()}if(a.length<2)return;const mn=Math.min(...a),mx=Math.max(...a),sp=Math.max(.0001,mx-mn);x.strokeStyle='#68a8ff';x.lineWidth=2*devicePixelRatio;x.beginPath();a.forEach((v,i)=>{const px=i*(w/(a.length-1)),py=h-12-(v-mn)/sp*(h-24);i?x.lineTo(px,py):x.moveTo(px,py)});x.stroke()}
async function tick(){try{const s=await(await fetch('/api/state',{cache:'no-store'})).json();$('conn').textContent='ONLINE';$('conn').style.color=s.system.status==='CRITICAL'?'#ff647c':s.system.status==='WARNING'?'#f8c95c':'#36d399';$('risk').innerHTML='<span class="pill '+s.system.status.toLowerCase()+'">'+s.system.status+'</span>';$('score').textContent=s.system.risk;$('n1').textContent=s.node1.online?'ONLINE':'OFFLINE';$('n2').textContent=s.node2.online?'ONLINE':'OFFLINE';$('up').textContent=Math.floor(s.system.uptime_s/60)+'m';$('reason').textContent=s.system.reason;$('temp').textContent=fmt(s.node1.temp,2);$('pressure').textContent=fmt(s.node1.pressure,1);$('lux').textContent=fmt(s.node1.lux,1);$('ldr1').textContent=s.node1.ldr;$('gas').textContent=s.node1.gas;$('motion').textContent=s.node2.motion?'YES':'NO';$('sound').textContent=s.node2.sound?'YES':'NO';$('ldr2').textContent=s.node2.ldr;$('age1').textContent=s.node1.online?s.node1.age_ms+' ms':'offline';$('age2').textContent=s.node2.online?s.node2.age_ms+' ms':'offline';$('tb').textContent=fmt(s.system.temp_baseline,2)+' °C';$('gb').textContent=fmt(s.system.gas_baseline,1)+' ADC';push(h.t,Number(s.node1.temp)||0);push(h.g,Number(s.node1.gas)||0);push(h.l,Number(s.node1.lux)||0);push(h.s,(s.node2.motion?1:0)+(s.node2.sound?1:0));draw('ct',h.t);draw('cg',h.g);draw('cl',h.l);draw('cs',h.s)}catch(e){$('conn').textContent='OFFLINE';$('conn').style.color='#ff647c'}}tick();setInterval(tick,2000);window.addEventListener('resize',()=>{draw('ct',h.t);draw('cg',h.g);draw('cl',h.l);draw('cs',h.s)});
</script></body></html>
)HTML";
  server.send_P(200, "text/html; charset=utf-8", PAGE);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, HIGH);
  digitalWrite(RELAY_PIN, LOW);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("[Gateway] WiFi");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(250);
    Serial.print('.');
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("[Gateway] IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("[Gateway] WiFi connection failed");
  }

  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/state", HTTP_GET, handleState);
  server.on("/api/node1", HTTP_GET, handleNode1);
  server.on("/api/node2", HTTP_GET, handleNode2);
  server.begin();
  Serial.println("[Gateway] Web server started");
}

void loop() {
  server.handleClient();
  static unsigned long lastEval = 0;
  if (millis() - lastEval >= 1000) {
    lastEval = millis();
    evaluateRisk();
  }
}
