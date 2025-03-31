#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h> // Use the correct Servo library for ESP32

// --- WiFi Credentials ---
const char* ssid = "free";
const char* password = "22334455";

// --- Servo Setup ---
const int servoPin = 13; // GPIO pin connected to the servo signal wire - CHANGE IF NEEDED
Servo myservo;           // Create a servo object
int servoAngle = 90;     // Global variable to track the current angle

// --- Web Server Setup ---
WebServer server(80);

// --- Helper function for servo sweep ---
void performSweep() {
  Serial.println("Performing sweep...");
  for (int pos = 0; pos <= 180; pos += 2) { // sweeps from 0 degrees to 180 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  for (int pos = 180; pos >= 0; pos -= 2) { // sweeps from 180 degrees to 0 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  myservo.write(servoAngle); // Return to the last set angle
  Serial.println("Sweep complete.");
}

// --- Helper function for servo pulse ---
void performPulse() {
    Serial.println("Performing pulse...");
    int currentAngle = myservo.read(); // Get current position
    int pulseAmount = 20; // How far to pulse

    myservo.write(constrain(currentAngle + pulseAmount, 0, 180));
    delay(200);
    myservo.write(constrain(currentAngle - pulseAmount, 0, 180));
    delay(200);
    myservo.write(currentAngle); // Return to original position
    Serial.println("Pulse complete.");
}

// --- Helper function for Min/Max ---
void performMinMax() {
    Serial.println("Performing Min/Max...");
    myservo.write(0);
    delay(500);
    myservo.write(180);
    delay(500);
    myservo.write(servoAngle); // Return to last set angle
    Serial.println("Min/Max complete.");
}


// --- HTML content for the web page (Arabic) ---
String htmlContent = R"rawliteral(
<!DOCTYPE HTML>
<html lang="ar" dir="rtl">
<head>
  <title>تحكم في سيرفو ESP32</title>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    :root {
      --primary-color: #007bff;
      --secondary-color: #6c757d;
      --background-color: #f8f9fa;
      --card-bg-color: #ffffff;
      --text-color: #333;
      --border-radius: 8px;
      --box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
    }
    body {
      font-family: 'Tahoma', 'Arial', sans-serif; /* Common Arabic fonts */
      text-align: center;
      padding: 20px;
      background-color: var(--background-color);
      color: var(--text-color);
      margin: 0;
    }
    h1 {
      color: var(--primary-color);
      margin-bottom: 25px;
    }
    p {
      font-size: 1.2em;
      margin-bottom: 15px;
    }
    .container {
        max-width: 600px;
        margin: 20px auto;
        padding: 30px;
        background-color: var(--card-bg-color);
        border-radius: var(--border-radius);
        box-shadow: var(--box-shadow);
    }
    .slider-container {
        margin-bottom: 30px;
    }
    input[type=range] {
      width: 90%;
      cursor: pointer;
      height: 10px;
      background: linear-gradient(to right, var(--primary-color) 0%, var(--secondary-color) 100%);
      border-radius: 5px;
      appearance: none; /* Override default look */
      -webkit-appearance: none;
    }
    input[type=range]::-webkit-slider-thumb {
        -webkit-appearance: none;
        appearance: none;
        width: 25px;
        height: 25px;
        background: var(--primary-color);
        border-radius: 50%;
        cursor: pointer;
    }
    input[type=range]::-moz-range-thumb {
        width: 25px;
        height: 25px;
        background: var(--primary-color);
        border-radius: 50%;
        cursor: pointer;
        border: none; /* Necessary for Firefox */
    }
    #angleValue {
        font-weight: bold;
        color: var(--primary-color);
        font-size: 1.3em;
    }
    .button-grid {
        display: grid;
        grid-template-columns: repeat(auto-fit, minmax(120px, 1fr));
        gap: 15px;
        margin-top: 30px;
    }
    button {
        padding: 12px 15px;
        font-size: 1em;
        font-weight: bold;
        color: white;
        background-color: var(--primary-color);
        border: none;
        border-radius: var(--border-radius);
        cursor: pointer;
        transition: background-color 0.3s ease, transform 0.1s ease;
    }
    button:hover {
        background-color: #0056b3; /* Darker shade on hover */
    }
    button:active {
        transform: scale(0.95);
    }
    .footer {
        margin-top: 40px;
        font-size: 0.9em;
        color: var(--secondary-color);
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>تحكم في سيرفو ESP32</h1>

    <div class="slider-container">
        <p>زاوية السيرفو: <span id="angleValue">90</span> درجة</p>
        <input type="range" min="0" max="180" value="90" id="angleSlider" oninput="updateAngle(this.value)">
    </div>

    <h2>وظائف إضافية ممتعة!</h2>
    <div class="button-grid">
        <button onclick="triggerAction('/sweep')">مسح (Sweep)</button>
        <button onclick="triggerAction('/center')">توسيط (Center)</button>
        <button onclick="triggerAction('/random')">عشوائي (Random)</button>
        <button onclick="triggerAction('/pulse')">نبض (Pulse)</button>
        <button onclick="triggerAction('/minmax')">أقصى/أدنى (Min/Max)</button>
    </div>
  </div>

  <div class="footer">
    مدعوم بواسطة ESP32!
  </div>

  <script>
    // Update slider value display and send command
    function updateAngle(angle) {
      document.getElementById("angleValue").innerText = angle;
      sendRequest("/setAngle?angle=" + angle);
    }

    // Trigger a pre-defined action
    function triggerAction(path) {
      console.log("Triggering action: " + path); // For debugging
      sendRequest(path);
      // Optional: You could update the slider after an action,
      // but that requires the ESP32 to send back the new angle.
      // For simplicity, we won't update the slider automatically here.
    }

    // General function to send requests to ESP32
    function sendRequest(path) {
      var xhr = new XMLHttpRequest();
      xhr.open("GET", path, true);
      xhr.onload = function () {
        if (xhr.status === 200) {
            console.log("ESP32 Response: " + xhr.responseText);
        } else {
            console.error("Error sending request to ESP32. Status: " + xhr.status);
        }
      };
      xhr.onerror = function () {
           console.error("Request failed.");
      };
      xhr.send();
    }

    // Set initial slider value display on page load
    document.addEventListener('DOMContentLoaded', (event) => {
        var initialAngle = document.getElementById('angleSlider').value;
        document.getElementById('angleValue').innerText = initialAngle;
    });
  </script>
</body>
</html>
)rawliteral";

// --- Request Handlers ---

// Handle the root path ("/")
void handleRoot() {
  Serial.println("Client requested root page.");
  server.send(200, "text/html", htmlContent);
}

// Handle setting the servo angle (/setAngle) from slider
void handleSetAngle() {
  if (server.hasArg("angle")) {
    String angleStr = server.arg("angle");
    int angle = angleStr.toInt();
    angle = constrain(angle, 0, 180); // Ensure angle is within bounds

    servoAngle = angle; // Update global angle tracker
    myservo.write(servoAngle);
    Serial.print("Setting servo angle to: ");
    Serial.println(servoAngle);
    server.send(200, "text/plain", "OK: Angle set to " + String(servoAngle));
  } else {
    server.send(400, "text/plain", "Error: Angle parameter missing.");
  }
}

// --- Handlers for Fun Functions ---

void handleSweep() {
    Serial.println("Received /sweep request");
    performSweep(); // Call the helper function
    server.send(200, "text/plain", "OK: Sweep initiated");
}

void handleCenter() {
    Serial.println("Received /center request");
    servoAngle = 90;
    myservo.write(servoAngle);
    server.send(200, "text/plain", "OK: Centered");
}

void handleRandom() {
    Serial.println("Received /random request");
    servoAngle = random(0, 181); // Generate random angle 0-180
    myservo.write(servoAngle);
    Serial.print("Setting random angle: ");
    Serial.println(servoAngle);
    server.send(200, "text/plain", "OK: Random angle set to " + String(servoAngle));
}

void handlePulse() {
    Serial.println("Received /pulse request");
    performPulse(); // Call the helper function
    server.send(200, "text/plain", "OK: Pulse performed");
}

void handleMinMax() {
    Serial.println("Received /minmax request");
    performMinMax(); // Call the helper function
    server.send(200, "text/plain", "OK: Min/Max performed");
}


// Handle 404 Not Found
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

// --- Setup Function ---
void setup() {
  Serial.begin(115200);
  delay(10);

  randomSeed(analogRead(0)); // Seed random number generator

  myservo.attach(servoPin); // Attach the servo to the specified pin
  myservo.write(servoAngle); // Set initial servo position

  Serial.println();
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    attempts++;
    if (attempts > 30) { // Timeout after ~15 seconds
        Serial.println("\nFailed to connect to WiFi.");
        // Optional: You could enter a deep sleep mode or loop forever here
        while(true) { delay(1000); }
    }
  }

  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // --- Setup Web Server Routes ---
  server.on("/", HTTP_GET, handleRoot);
  server.on("/setAngle", HTTP_GET, handleSetAngle); // Handle slider updates

  // Routes for fun functions
  server.on("/sweep", HTTP_GET, handleSweep);
  server.on("/center", HTTP_GET, handleCenter);
  server.on("/random", HTTP_GET, handleRandom);
  server.on("/pulse", HTTP_GET, handlePulse);
  server.on("/minmax", HTTP_GET, handleMinMax);

  server.onNotFound(handleNotFound); // Handle invalid paths

  server.begin(); // Start the server
  Serial.println("HTTP server started");
}

// --- Loop Function ---
void loop() {
  server.handleClient(); // Listen for and process client requests
  // No servo movement code needed directly in the loop for this example,
  // as movements are triggered by web requests.
  delay(2); // Small delay to prevent watchdog timeout and allow tasks to run
}