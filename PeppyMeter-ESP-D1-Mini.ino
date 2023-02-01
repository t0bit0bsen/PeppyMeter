#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <ESP8266WiFi.h>


WiFiManager wifiManager;
WiFiServer server(8001);   // Port des Web Servers setzen

#define analogOutLeft  D7
#define analogOutRight  D2
#define analogOutMono  D1

int left = 0;
int right = 0;
int mono = 0;
unsigned long timestamp = millis(); // timestamp for timeout
const long timeout = 1000;  // timeout in ms


void setup() {
  pinMode(analogOutLeft, OUTPUT);
  pinMode(analogOutRight, OUTPUT);
  pinMode(analogOutMono, OUTPUT);

  Serial.begin(115200);
  wifiManager.setConfigPortalTimeout(180);
  wifiManager.autoConnect("PeppyMeterAP");

  // Start the server
  server.begin();
  Serial.println("Server started");

}


void loop() {
  // put your main code here, to run repeatedly:

  WiFiClient client = server.available();   // wait for client (PeppyMeter) to connect

  if (client) {                             // Client connected
    String req = client.readStringUntil('}');  // read full request
    if (req.startsWith("PUT /vumeter")) {
      String temp = req.substring(req.indexOf('{', 160) + 1); // cur payload e.g. "left": 7.0, "right": 7.25, "mono": 7.125

      // Extract Channel Data
      temp = temp.substring(temp.indexOf('l') + 7);
      left = min(255, int(temp.toFloat() * 2.56));
      temp = temp.substring(temp.indexOf('r') + 8);
      right = min(255, int(temp.toFloat() * 2.56));
      temp = temp.substring(temp.indexOf('m') + 7);
      mono = min(255, int(temp.toFloat() * 2.56));

      // Set analog Outputs
      analogWrite(analogOutLeft, left);
      analogWrite(analogOutRight, right);
      analogWrite(analogOutMono, mono);

      // end connection
      client.flush();

      // update timestamp
      timestamp = millis();
      
    } // if PUT /vumeter
  }  // if client

  if (Serial.available()) {
    String ser = Serial.readStringUntil('\n');

    // Extract Channel Data
    String temp2 = ser.substring(ser.indexOf('l') + 1);
    left = min(255, int(temp2.toFloat() * 2.56));
    temp2 = ser.substring(ser.indexOf('r') + 1);
    right = min(255, int(temp2.toFloat() * 2.56));
    mono = int((left + right) / 2);

    // Set analog Outputs
    analogWrite(analogOutLeft, left);
    analogWrite(analogOutRight, right);
    analogWrite(analogOutMono, mono);

    // update timestamp
    timestamp = millis();
  }

  if (millis() - timestamp >= timeout) {
    analogWrite(analogOutLeft, 0);
    analogWrite(analogOutRight, 0);
    analogWrite(analogOutMono, 0);
  }
}
