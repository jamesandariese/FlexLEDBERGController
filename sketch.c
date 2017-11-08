#include <WiFi.h>

#define WIFI_SSID "SSID"
#define WIFI_PASSWORD "PASSWORD"

#define LED_RED 0
#define LED_GREEN 1
#define LED_BLUE 2
#define freq 5000
#define resolution 8

#define minmax(i,m,x) ((i<m)?m:((i>x)?x:i))

WiFiServer server(80);

int scale[101] = {
  0,
  1,
  1,
  1,
  1,
  1,
  1,
  2,
  2,
  3,
  3,
  4,
  4,
  5,
  5,
  6,
  7,
  8,
  9,
  10,
  11,
  12,
  13,
  14,
  15,
  16,
  18,
  19,
  20,
  22,
  23,
  25,
  27,
  28,
  30,
  32,
  34,
  35,
  37,
  39,
  41,
  43,
  45,
  48,
  50,
  52,
  54,
  57,
  59,
  62,
  64,
  67,
  69,
  72,
  75,
  78,
  80,
  83,
  86,
  89,
  92,
  95,
  99,
  102,
  105,
  108,
  112,
  115,
  118,
  122,
  125,
  129,
  133,
  136,
  140,
  144,
  148,
  152,
  156,
  160,
  164,
  168,
  172,
  176,
  180,
  185,
  189,
  193,
  198,
  202,
  207,
  212,
  216,
  221,
  226,
  231,
  235,
  240,
  245,
  250,
  255
};

int r = 0;
int g = 0;
int b = 0;
int h = 0;
float s = 0;
float v = 0;

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

  ledcWrite(LED_RED, scale[r]);
  ledcWrite(LED_GREEN, scale[g]);
  ledcWrite(LED_BLUE, scale[b]);

  server.begin();
}

void setRGB(int rp, int gp, int bp) {
  r = minmax(rp, 0, 100);
  g = minmax(gp, 0, 100);
  b = minmax(bp, 0, 100);
  Serial.print("Red   ");
  Serial.println(r);
  Serial.print("Green ");
  Serial.println(g);
  Serial.print("Blue  ");
  Serial.println(b);
  ledcWrite(LED_RED, scale[r]);
  ledcWrite(LED_GREEN, scale[g]);
  ledcWrite(LED_BLUE, scale[b]);
}

double doublemod(double i, double m) {
  while (i < m) {
    i += m;
  }
  while (i > m) {
    i -= m;
  }
  return i;
}

void setHSV(int hp, float sp, float vp) {
  h = minmax(hp, 0, 360);
  s = minmax(sp, 0.0d, 1.0d);
  v = minmax(vp, 0.0d, 1.0d);
  float c = v * s;
  float x = c * ( 1 - abs( (doublemod((h / 60.0), 2.0) ) - 1) );
  float m = v - c;
  float rp, gp, bp;
  if (h < 60) {
    rp = c;
    gp = x;
    bp = 0;
  } else if (h < 120) {
    rp = x;
    gp = c;
    bp = 0;
  } else if (h < 180) {
    rp = 0;
    gp = c;
    bp = x;
  } else if (h < 240) {
    rp = 0;
    gp = x;
    bp = c;
  } else if (h < 300) {
    rp = x;
    gp = 0;
    bp = c;
  } else {
    rp = c;
    gp = 0;
    bp = x;
  }
  setRGB((int)(((rp + m) * 100) + .5), (int)(((gp + m) * 100) + .5), (int)(((bp + m) * 100) + .5));
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
          if (line.startsWith("GET / ") || line.endsWith("GET /")) {
            response = ("{\"red\":" + String(r) +
                        ",\"green\":" + String(g) +
                        ",\"blue\":" + String(b) +
                        ",\"hue\":" + String(h) +
                        ",\"saturation\":" + String(s) +
                        ",\"value\":" + String(v) +
                        ",\"success\": true}");
            //} else if (line.startsWith("GET /rgb/")) {
            //  String rest = line.substring(9);
            //  float rp = rest.substring(0, rest.indexOf('/')).toInt();
            //  rest = rest.substring(rest.indexOf('/') + 1);
            //  float gp = rest.substring(0, rest.indexOf('/')).toInt();
            //  rest = rest.substring(rest.indexOf('/') + 1);
            //  float bp = rest.substring(0, rest.indexOf(' ')).toInt();
            //  setRGB(rp, gp, bp);
            //  code = 200;
            //  response = "{\"response\": \"ok\", \"success\": true}";
          } else if (line.startsWith("GET /hsv/")) {
            String rest = line.substring(9);
            int h = rest.substring(0, rest.indexOf('/')).toInt();
            rest = rest.substring(rest.indexOf('/') + 1);
            float s = rest.substring(0, rest.indexOf('/')).toFloat();
            rest = rest.substring(rest.indexOf('/') + 1);
            float v = rest.substring(0, rest.indexOf(' ')).toFloat();
            setHSV(h, s, v);
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
    client.println("Content-type: application/json");
    client.println();
    client.println(response);
    client.flush();

    client.stop();
    Serial.println("Bye, client");
  }
}
