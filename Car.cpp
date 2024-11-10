#include <WiFi.h>
#include <WebServer.h>


const char* ssid     = "*********";
const char* password = "*********";

// Motor Pin 
const int motor1Pin1 = 27, motor1Pin2 = 26, enable1Pin = 14;
const int motor2Pin1 = 33, motor2Pin2 = 25, enable2Pin = 32;


const int freq = 30000;
const int resolution = 8;
int dutyCycle = 0;


WebServer server(80);

void setup() {
  Serial.begin(115200);


  setupMotorPins();


  setupPWM();

  // Connect to Wi-Fi
  connectToWiFi();


  setupWebRoutes();

  // Start the web server
  server.begin();
}

void loop() {
  server.handleClient();
}

// Function to initialize motor pins
void setupMotorPins() {
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(motor2Pin1, OUTPUT);
  pinMode(motor2Pin2, OUTPUT);
}


void setupPWM() {
  ledcAttachPin(enable1Pin, freq, resolution);
  ledcAttachPin(enable2Pin, freq, resolution);
  ledcWrite(enable1Pin, 0);
  ledcWrite(enable2Pin, 0);
}

// Function to connect to Wi-Fi
void connectToWiFi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// Setup routes for web server
void setupWebRoutes() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/forward", HTTP_GET, handleForward);
  server.on("/left", HTTP_GET, handleLeft);
  server.on("/stop", HTTP_GET, handleStop);
  server.on("/right", HTTP_GET, handleRight);
  server.on("/reverse", HTTP_GET, handleReverse);
  server.on("/speed", HTTP_GET, handleSpeed);
}

// Root page to control the robot
void handleRoot() {
  const char html[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML><html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
      body { text-align: center; font-family: Arial, sans-serif; }
      .button { font-size: 20px; padding: 15px; margin: 10px; cursor: pointer; background-color: #4CAF50; color: white; border: none; border-radius: 5px; }
      .button2 { background-color: #555; }
    </style>
    <script>
      function moveForward() { fetch('/forward'); }
      function moveLeft() { fetch('/left'); }
      function stopRobot() { fetch('/stop'); }
      function moveRight() { fetch('/right'); }
      function moveReverse() { fetch('/reverse'); }
      function updateMotorSpeed(value) { document.getElementById('motorSpeed').innerHTML = value; fetch('/speed?value=' + value); }
    </script>
  </head>
  <body>
    <h1>ESP32 Motor Control</h1>
    <button class="button" onclick="moveForward()">Forward</button>
    <div>
      <button class="button" onclick="moveLeft()">Left</button>
      <button class="button button2" onclick="stopRobot()">Stop</button>
      <button class="button" onclick="moveRight()">Right</button>
    </div>
    <button class="button" onclick="moveReverse()">Reverse</button>
    <p>Motor Speed: <span id="motorSpeed">0</span></p>
    <input type="range" min="0" max="100" value="0" step="25" oninput="updateMotorSpeed(this.value)" />
  </body>
  </html>)rawliteral";
  
  server.send(200, "text/html", html);
}

// Move Motors 
void handleForward() {
  Serial.println("Moving Forward");
  setMotorDirection(LOW, HIGH, LOW, HIGH);
  server.send(200, "text/plain", "Moving Forward");
}


void handleLeft() {
  Serial.println("Turning Left");
  setMotorDirection(LOW, LOW, LOW, HIGH);
  server.send(200, "text/plain", "Turning Left");
}

// Stop Motors
void handleStop() {
  Serial.println("Stopping Motors");
  stopMotors();
  server.send(200, "text/plain", "Motors Stopped");
}


void handleRight() {
  Serial.println("Turning Right");
  setMotorDirection(LOW, HIGH, LOW, LOW);
  server.send(200, "text/plain", "Turning Right");
}

// Move Motors in Reverse
void handleReverse() {
  Serial.println("Moving in Reverse");
  setMotorDirection(HIGH, LOW, HIGH, LOW);
  server.send(200, "text/plain", "Moving in Reverse");
}

// Control Motor Speed
void handleSpeed() {
  if (server.hasArg("value")) {
    int speedValue = server.arg("value").toInt();
    adjustMotorSpeed(speedValue);
  }
  server.send(200, "text/plain", "Speed Adjusted");
}

// Set Motor Directions
void setMotorDirection(int m1Pin1, int m1Pin2, int m2Pin1, int m2Pin2) {
  digitalWrite(motor1Pin1, m1Pin1);
  digitalWrite(motor1Pin2, m1Pin2);
  digitalWrite(motor2Pin1, m2Pin1);
  digitalWrite(motor2Pin2, m2Pin2);
}

// Stop Motors
void stopMotors() {
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, LOW);
  ledcWrite(enable1Pin, 0);
  ledcWrite(enable2Pin, 0);
}

// Adjust Motor Speed (PWM)
void adjustMotorSpeed(int speedValue) {
  if (speedValue == 0) {
    stopMotors();
  } else {
    dutyCycle = map(speedValue, 0, 100, 0, 255);
    ledcWrite(enable1Pin, dutyCycle);
    ledcWrite(enable2Pin, dutyCycle);
    Serial.printf("Motor Speed Set to: %d\n", dutyCycle);
  }
}
