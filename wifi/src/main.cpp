#include <WiFi.h>
#include <stdlib.h>

#define DATAPRICE 11
#define HEADER 127
const char* ssid = "123456789"//自分が使用するSSID
const char* password = "qwerty";//パスワード

WiFiServer server(80);

String header;

int value = 0;

String output26State = "off";
String output27State = "off";

const int output26 = 26;
const int output27 = 27;

unsigned long currentTime = millis();
unsigned long previousTime = 0; 
const long timeoutTime = 2000;

void sendSensorData(WiFiClient& client, const char sensorData[]) {
  client.print("Sensor Data: ");
  for (int i = 0; i < DATAPRICE; i++) {
    client.print(sensorData[i]);
    client.print(" ");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(output26, OUTPUT);
  pinMode(output27, OUTPUT);
  digitalWrite(output26, LOW);
  digitalWrite(output27, LOW);

  Serial2.begin(115200, SERIAL_8N1, 16, 17);

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop() {
  WiFiClient client = server.available();

  char data[12];
  data[0] = 127;
  data[1] = 127;
  data[11] = 0;
  char getdata[DATAPRICE + 2];
  int flag = 0;

  // データの送信処理（Serial2からのデータ送信）
  for (int i = 2; i < 11; i++) {
    data[i] = (int)(i * 14);
  }
  if (Serial2.available()) {
    if (Serial2.read() == (int)HEADER) {
      flag++;
    }
    if (flag == 2) {
      for (int i = 0; i < DATAPRICE; i++) {
        getdata[i] = Serial2.read();
      }
      printf("success");
      flag = 0;
    }

    // データ送信処理
    for (int i = 0; i < 11; i++) {
      getdata[i] = i;
      printf("%3d-", getdata[i]);
    }
    printf("|");
    printf("%3d %3d %3d", (int)getdata[0] * 100 + (int)getdata[1], getdata[2] * 100 + getdata[3], getdata[4] * 100 + getdata[5]);
    int yaw = getdata[0] * 100 + getdata[1];
    printf("\n");
    for (int i = 2; i < 11; i++) {
      data[11] += data[i];
    }
    for (int i = 0; i < 12; i++) {
      Serial2.print(data[i]);
    }
  } else {
    printf("not success \n");
  }

  if (client) {
    currentTime = millis();
    previousTime = currentTime;
    String currentLine = "";

    while (1) {
      currentTime = millis();
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        header += c;
        if (c == '\n') {
          if (currentLine.length() == 0) {
            Serial.println("New Client.");
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            sendSensorData(client, getdata);
            client.println();
            if (header.indexOf("GET /26/on") >= 0) {
              Serial.println("GPIO 26 on");
              output26State = "on";
              digitalWrite(output26, HIGH);
            } else if (header.indexOf("GET /26/off") >= 0) {
              Serial.println("GPIO 26 off");
              output26State = "off";
              digitalWrite(output26, LOW);
            } else if (header.indexOf("GET /27/on") >= 0) {
              Serial.println("GPIO 27 on");
              output27State = "on";
              digitalWrite(output27, HIGH);
            } else if (header.indexOf("GET /27/off") >= 0) {
              Serial.println("GPIO 27 off");
              output27State = "off";
              digitalWrite(output27, LOW);
            }
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            client.println("<body><h1>ESP32 Web Server</h1>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<p>GPIO 26 - State " + output26State + "</p>");
            if (output26State == "off") {
              client.println("<p><a href=\"/26/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/26/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            client.println("<p>GPIO 27 - State " + output27State + "</p>");
            if (output27State == "off") {
              client.println("<p><a href=\"/27/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/27/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            client.println("</body></html>");
            client.println();
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }

    header = "";
  }
}
