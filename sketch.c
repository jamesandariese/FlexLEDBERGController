#include <WiFi.h>

// Change these before burning your own

#define WIFI_SSID "SSID"
#define WIFI_PASSWORD "PASSWORD"
#define RED_PIN 2
#define GREEN_PIN 22
#define BLUE_PIN 18

// All done :D


#define LED_RED 0
#define LED_GREEN 1
#define LED_BLUE 2
#define freq 5000
#define resolution 8

#define minmax(i,m,x) ((i<m)?m:((i>x)?x:i))

WiFiServer server(80);

int r = 0;
int g = 0;
int b = 0;

void setup() {
  Serial.begin(115200);

  pinMode(19, OUTPUT);
  digitalWrite(19, HIGH);

  delay(10);

  Serial.println("getting started");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWifi Connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  
  ledcSetup(LED_RED, freq, resolution);
  ledcAttachPin(2, LED_RED);
  ledcAttachPin(22, LED_GREEN);
  ledcAttachPin(18, LED_BLUE);

  ledcWrite(LED_RED, r);
  ledcWrite(LED_GREEN, g);
  ledcWrite(LED_BLUE, b);

  server.begin();
}

 
void loop() {
  WiFiClient client = server.available();

  int code = 500;
  String response = "unknown";
  String success = "false";

  if (client) {
    Serial.println("New Client.");
    String line = "";

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        if (c == '\n') {
          if (line.startsWith("GET /rgb ") || line.endsWith("GET /rgb")) {
            response = ("{\"red\":"+String(r)+
                        ",\"green\":" + String(g)+
                        ",\"blue\":" + String(b)+
                       ",\"success\": true}");
          } else if (line.startsWith("GET /rgb/")) {
            String rest = line.substring(9);
            r = rest.substring(0, rest.indexOf('/')).toInt();
            rest = rest.substring(rest.indexOf('/')+1);
            g = rest.substring(0, rest.indexOf('/')).toInt();
            rest = rest.substring(rest.indexOf('/')+1);
            b = rest.substring(0, rest.indexOf(' ')).toInt();
            r = minmax(r,0,255);
            g = minmax(g,0,255);
            b = minmax(b,0,255);
            Serial.print("Red   ");
            Serial.println(r);
            Serial.print("Green ");
            Serial.println(g);
            Serial.print("Blue  ");
            Serial.println(b);
            ledcWrite(LED_RED, r);
            ledcWrite(LED_GREEN, g);
            ledcWrite(LED_BLUE, b);
            code = 200;
            response = "{\"response\": \"ok\", \"success\": true}";
          } else {
            code = 404;
          }
          break;
        } else if (c != '\r') {
          line += c;
        }
      }
    }
    client.print("HTTP/1.1 ");
    client.print(code);
    client.println(" OK");
    client.println("Content-type: text/json");
    client.println();
    client.println(response);
    client.flush();
    
    client.stop();
    Serial.println("Bye, client");
  }
}
