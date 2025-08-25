#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <ArduinoOTA.h>


Preferences preferences;


String ssid = "";
String password = "";
const char* ap_ssid = "gernator 3.O";
const char* ap_password = "12345678";


WebServer server(80);


#define LED_BUILTIN 2
#define RELAY1_MAIN 13
#define RELAY2_ON 12
#define RELAY3_OFF 14


#define P1_MAIN 26
#define P2_GEN 25


#define MAX_RETRIES 7
#define RETRY_DELAY 1000


bool manualModeEnabled = true;


// --- Static IP Configuration ---
IPAddress staticIP(192, 168, 1, 105);    // Replace with your desired static IP
IPAddress gateway(192, 168, 1, 1);       // Replace with your network's gateway
IPAddress subnet(255, 255, 255, 0);      // Replace with your network's subnet mask
IPAddress dns(8, 8, 8, 8);               // Replace with your preferred DNS server (e.g., Google DNS)
// --- End Static IP Configuration ---


// Function prototypes to avoid compilation warnings
void handleRoot();
void handleManualToggle();
void handleManualStatus();
void handleStartGenerator();
void handleStopGenerator();
void handleAutoStop();
void handleStatus();
void handleSetWiFi();
void handleIPAddresses();


void setupWiFi() {
  preferences.begin("wifi", false);
  ssid = preferences.getString("ssid", "ORKIRE");
  password = preferences.getString("password", "12345678");
  preferences.end();


  // Configure static IP before beginning connection
  WiFi.config(staticIP, gateway, subnet, dns);


  WiFi.begin(ssid.c_str(), password.c_str());
  Serial.print("Connecting to WiFi");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }


  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to STA network. IP:");
    Serial.println(WiFi.localIP());
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    Serial.println("\nFailed to connect to STA. Continuing with AP mode.");
  }


  WiFi.softAP(ap_ssid, ap_password, 1, 0, 1);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
}


void setupOTA() {
  ArduinoOTA.setHostname("esp32-testing base");


  ArduinoOTA.onStart([]() {
    Serial.println("Start OTA update...");
  });


  ArduinoOTA.onEnd([]() {
    Serial.println("OTA update complete.");
  });


  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("OTA Progress: %u%%\r", (progress * 100) / total);
  });


  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });


  ArduinoOTA.begin();
  Serial.println("OTA Ready");
}


void setup() {
  Serial.begin(115200);


  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);


  pinMode(RELAY1_MAIN, OUTPUT);
  pinMode(RELAY2_ON, OUTPUT);
  pinMode(RELAY3_OFF, OUTPUT);
  digitalWrite(RELAY1_MAIN, HIGH);
  digitalWrite(RELAY2_ON, HIGH);
  digitalWrite(RELAY3_OFF, HIGH); // Assuming HIGH means OFF for RELAY3_OFF initially


  pinMode(P1_MAIN, INPUT);
  pinMode(P2_GEN, INPUT);


  setupWiFi();
  setupOTA();


  server.on("/", handleRoot);
  server.on("/start", handleStartGenerator);
  server.on("/stop", handleStopGenerator);
  server.on("/status", handleStatus);
  server.on("/setwifi", handleSetWiFi);
  server.on("/manual", HTTP_POST, handleManualToggle);
  server.on("/manualstatus", handleManualStatus);
  server.on("/ipaddresses", handleIPAddresses); // New endpoint for IP addresses


  server.begin();
  Serial.println("HTTP server started");
}


void loop() {
  server.handleClient();
  handleAutoStop();
  ArduinoOTA.handle();
}


void handleRoot() {
  String html = R"rawliteral(
    <html>
    <head>
      <title>test case</title>
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <style>
        body {
          text-align: center;
          font-family: sans-serif;
          margin: 0;
          padding: 20px;
        }


        button {
          padding: 15px 30px;
          font-size: 18px;
          margin: 10px;
          border-radius: 12px;
          box-shadow: 2px 2px 8px rgba(0, 0, 0, 0.3);
          border: none;
          cursor: pointer;
          transition: transform 0.2s ease, background-color 0.3s ease;
        }
        button:hover {
          transform: scale(1.05);
        }


        button:active {
          transform: scale(0.95);
        }


        .startBtn {
          background: green;
          color: white;
        }


        .stopBtn {
          background: red;
          color: white;
        }
        .manualBtn {
          background-color: #007bff;
          color: white;
        }


        input[type="text"],
        input[type="password"] {
          padding: 10px;
          font-size: 16px;
          border-radius: 8px;
          border: 1px solid #ccc;
          width: 100%;
          max-width: 200px;
        }


        .form-row {
          display: flex;
        flex-wrap: wrap;
          justify-content: center;
          align-items: center;
          gap: 10px;
          margin-top: 10px;
        }


        .saveBtn {
          background-color: #d2b48c;
          color: black;
          font-weight: bold;
          border: none;
          border-radius: 10px;
          padding: 10px 20px;
          box-shadow: 2px 2px 6px rgba(0, 0, 0, 0.3);
          cursor: pointer;
          transition: transform 0.2s ease;
        }


        .saveBtn:hover {
          transform: scale(1.05);
        }


        #result {
          margin-top: 20px;
          font-size: 18px;
        color: blue;
          white-space: pre-line;
        }


        @media (max-width: 600px) {
          .form-row {
            flex-direction: column;
            align-items: stretch;
          }


          button {
            width: 90%;
            max-width: 300px;
          }
        }
      </style>
    </head>
    <body>


      <h2>TEST CASE</h2>
      <div id="manualSection">
        <h3>Manual Mode</h3>
        <p>Status: <span id="manualStatus">Loading...</span></p>
        <button class="manualBtn" onclick="setManual(true)">Enable Manual</button>
        <button class="manualBtn" onclick="setManual(false)">Disable Manual</button>
      </div>


      <div id="genControls" style="display:none;">
        <button class='startBtn' onclick="startGenerator()">Start Generator</button><br>
        <button class='stopBtn' onclick="stopGenerator()">Stop Generator</button><br><br>


        <div style='font-size:18px; margin-top:30px;'>
          Main Supply: <span id="mainStatus">Loading...</span><br>
          Generator: <span id="genStatus">Loading...</span>
        </div>


        <form action="/setwifi" method="POST" style="margin-top:30px;">
          <h3>Change WiFi</h3>
          <div class="form-row">
            SSID: <input type="text" name="ssid">
            Password: <input type="password" name="password">
            <input type="submit" class="saveBtn" value="Save & Restart">
          </div>
        </form>


        <div id="result"></div>
      </div>
     
      <div id="ip-info" style="margin-top: 30px;">
          <h3>Network Information</h3>
          <p>STA IP: <span id="staIp">Not Connected</span></p>
          <p>AP IP: <span id="apIp">Not Connected</span></p>
      </div>


      <script>
        function updateStatus() {
          fetch('/status')
            .then(response => response.json())
            .then(data => {
              document.getElementById("mainStatus").innerText = data.main ? "ON" : "OFF";
              document.getElementById("genStatus").innerText = data.generator ? "RUNNING" : "OFF";
            });
        }


        function startGenerator() {
          document.getElementById("result").innerText = "Starting...";
          fetch('/start')
            .then(response => response.text())
            .then(data => {
              document.getElementById("result").innerText = data;
              updateStatus();
            });
        }


        function stopGenerator() {
          document.getElementById("result").innerText = "Stopping...";
          fetch('/stop')
            .then(response => response.text())
            .then(data => {
              document.getElementById("result").innerText = data;
              updateStatus();
            });
        }


        function setManual(state) {
          fetch('/manual', {
            method: 'POST',
            headers: {'Content-Type': 'application/x-www-form-urlencoded'},
            body: 'mode=' + (state ? 'on' : 'off')
          }).then(() => {
            updateManualStatus();
          });
        }


        function updateManualStatus() {
          fetch('/manualstatus')
            .then(response => response.json())
            .then(data => {
              let isManual = data.manual;
              document.getElementById("manualStatus").innerText = isManual ? "ON" : "OFF";
              document.getElementById("genControls").style.display = isManual ? "none" : "block";
            });
        }
       
        function updateIPAddresses() {
            fetch('/ipaddresses')
                .then(response => response.json())
                .then(data => {
                    document.getElementById("staIp").innerText = data.sta_ip;
                    document.getElementById("apIp").innerText = data.ap_ip;
                });
        }


        setInterval(updateStatus, 1000);
        setInterval(updateManualStatus, 2000);
        setInterval(updateIPAddresses, 5000); // Poll for IP addresses every 5 seconds
        updateManualStatus();
        updateStatus();
        updateIPAddresses();
      </script>
    </body>
    </html>
  )rawliteral";


  server.send(200, "text/html", html);
}


void handleManualToggle() {
  String mode = server.arg("mode");
  manualModeEnabled = (mode == "on");
  server.send(200, "text/plain", "Manual mode updated");
}


void handleManualStatus() {
  server.send(200, "application/json", "{\"manual\":" + String(manualModeEnabled ? "true" : "false") + "}");
}


void handleStartGenerator() {
  if (manualModeEnabled) {
    server.send(200, "text/plain", "❌ Manual Mode is ON. Control disabled.");
    return;
  }


  if (digitalRead(P1_MAIN) == HIGH) {
    server.send(200, "text/plain", "❌ Main power is ON. Cannot start generator.");
    return;
  }


  String responseLog = "";
  bool success = false;


  digitalWrite(RELAY1_MAIN, LOW);
  digitalWrite(RELAY3_OFF, HIGH); // Ensure stop relay is HIGH (off)


  for (int i = 0; i < MAX_RETRIES; i++) {
    responseLog += "Start Attempt " + String(i + 1) + "\n";
    digitalWrite(RELAY2_ON, LOW);
    delay(3000);
    digitalWrite(RELAY2_ON, HIGH);
    delay(RETRY_DELAY);


    if (digitalRead(P2_GEN) == HIGH) {
      success = true;
      break;
    }
  }


  digitalWrite(RELAY1_MAIN, HIGH);
  responseLog += success ? "✅ Generator STARTED successfully." : "❌ Failed to START generator.";
  server.send(200, "text/plain", responseLog);
}


void handleStopGenerator() {
  if (manualModeEnabled) {
    server.send(200, "text/plain", "❌ Manual Mode is ON. Control disabled.");
    return;
  }


  if (digitalRead(P2_GEN) == LOW) {
    server.send(200, "text/plain", "ℹ️ Generator is already OFF.");
    return;
  }


  String responseLog = "";
  bool success = false;


  digitalWrite(RELAY1_MAIN, LOW);
  delay(500);


  for (int i = 0; i < MAX_RETRIES; i++) {
    responseLog += "Stop Attempt " + String(i + 1) + "\n";
    digitalWrite(RELAY3_OFF, LOW);
    delay(3000);
    digitalWrite(RELAY3_OFF, HIGH);
    delay(RETRY_DELAY);


    if (digitalRead(P2_GEN) == LOW) {
      success = true;
      break;
    }
  }


  digitalWrite(RELAY1_MAIN, HIGH);
  responseLog += success ? "✅ Generator STOPPED successfully." : "❌ Failed to STOP generator.";
  server.send(200, "text/plain", responseLog);
}


void handleAutoStop() {
  static bool alreadyStopping = false;


  if (manualModeEnabled) return;


  if (digitalRead(P1_MAIN) == HIGH && digitalRead(P2_GEN) == HIGH && !alreadyStopping) {
    alreadyStopping = true;
    Serial.println("Auto: Main power is back. Trying to stop generator...");


    digitalWrite(RELAY1_MAIN, LOW);
    delay(500);


    for (int i = 0; i < MAX_RETRIES; i++) {
      Serial.printf("Auto Stop Attempt %d\n", i + 1);
      digitalWrite(RELAY3_OFF, LOW);
      delay(3000);
      digitalWrite(RELAY3_OFF, HIGH);
      delay(RETRY_DELAY);


      if (digitalRead(P2_GEN) == LOW) {
        Serial.println("Auto: Generator stopped.");
        break;
      }
    }


    digitalWrite(RELAY1_MAIN, HIGH);


    if (digitalRead(P2_GEN) == HIGH) {
      Serial.println("Auto: Generator failed to stop after many attempts.");
    }


    alreadyStopping = false;
  }
}


void handleStatus() {
  bool mainPower = digitalRead(P1_MAIN);
  bool generator = digitalRead(P2_GEN);


  String json = "{";
  json += "\"main\":" + String(mainPower ? "true" : "false") + ",";
  json += "\"generator\":" + String(generator ? "true" : "false");
  json += "}";


  server.send(200, "application/json", json);
}


void handleSetWiFi() {
  if (server.method() == HTTP_POST) {
    String newSsid = server.arg("ssid");
    String newPassword = server.arg("password");


    preferences.begin("wifi", false);
    preferences.putString("ssid", newSsid);
    preferences.putString("password", newPassword);
    preferences.end();


    server.send(200, "text/html", "<html><body><h3>WiFi credentials saved. Restarting...</h3></body></html>");
    delay(2000);
    ESP.restart();
  } else {
    server.send(405, "text/plain", "Method Not Allowed");
  }
}


void handleIPAddresses() {
  String staIp = WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : "Not Connected";
  String apIp = WiFi.softAPIP().toString();


  String json = "{";
  json += "\"sta_ip\":\"" + staIp + "\",";
  json += "\"ap_ip\":\"" + apIp + "\"";
  json += "}";
 
  server.send(200, "application/json", json);
}