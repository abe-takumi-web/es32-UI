#include <Arduino.h>
#include <WiFi.h>

void doInitialize();
void httpListen();
void httpRequestProccess(String*);
void httpSendResponse(WiFiClient*);
void connectToWifi();

#define WIFI_SSID "xxxxxxxxxxxxx"
#define WIFI_PASSWORD "xxxxxxxxxxxxxxx"
int bno055_value = 0;

char SliderValue[10] = "0"; // 文字列として扱うためのバッファ

#define SPI_SPEED   115200
#define HTTP_PORT 80

WiFiServer server(HTTP_PORT);

const String strResponseHeader = "HTTP/1.1 200 OK\r\nContent-Type:text/html\r\n"
        "Connection:close\r\n\r\n";

/* HTMLページ構成要素 */
const String strHtmlHeader = R"rawliteral(
<!DOCTYPE HTML>
<html>
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
      html { font-family: Helvetica; display: inline-block; margin: 0px auto;text-align: center;} 
      h1 {font-size:28px;} 
      .btn_on { padding:12px 30px; text-decoration:none; font-size:24px; background-color:
        #668ad8; color: #FFF; border-bottom: solid 4px #627295; border-radius: 2px;}
      .btn_on:active { -webkit-transform: translateY(0px); transform: translateY(0px);
        border-bottom: none;}
      .btn_off { background-color: #555555; border-bottom: solid 4px #333333;}
      .slider { width: 400px;}
    </style>
    <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.4.1/jquery.min.js"></script>
  </head>
)rawliteral";

const String strHtmlBody = R"rawliteral(
  <body><h1>%PAGE_TITLE%</h1>
    <p>LED State : %LED_STATE%</p>
    <p>%BUTTON_STATE%</p>
    <p>Brightness (<span id="brightValue"></span>)</p>
    <input type="range" min="0" max="100" step="5" class="slider" id="slideBar" onchange="slideValue(this.value)" 
    value="%SLIDER_VALUE%" />
    <script> var obj = document.getElementById("slideBar");
    var target = document.getElementById("brightValue");
    target.innerHTML = obj.value;
    obj.oninput = function() { obj.value = this.value; target.innerHTML = this.value; }
    function slideValue(val) { $.get("/?value=" + val + '&'); { Connection: close}; }
    </script>
    <head>
    <meta charset="UTF-8" />
    <title>doughnut and pie chart</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js@3.7.1"></script>
    <script>
      window.onload = function () {
        let context = document.querySelector("#sushi_circle").getContext('2d')
        new Chart(context, {
          type: 'doughnut',
          data: {
            labels: ["bno055",""],
            datasets: [{
                backgroundColor: ["#fa8072", "#00ff7f"],
              data: [%BNO055_VALUE%,360-%BNO055_VALUE%]
            }]
          },
          options: {
            responsive: false,
          }
        });
      }
    </script>
  </head>
  <body>
    <canvas id="sushi_circle" width="400" height="400"></canvas>
  </body>
  </body>
</html>
)rawliteral";

const String strButtonOn = R"rawliteral(
    <a href="/ON"><button class="btn_on">&nbsp;ON&nbsp;</button></a> )rawliteral";
const String strButtonOff = R"rawliteral(
    <a href="/OFF"><button class="btn_on btn_off">OFF</button></a> )rawliteral";

void setup() {
    
    doInitialize();
    connectToWifi();
}

void loop() {
    bno055_value++;
    if(bno055_value >100){
        bno055_value = 0;
    }
    httpListen();
}

void doInitialize() {
    Serial.begin(SPI_SPEED);
}

void httpListen() {
    String strBuffer = "";
    WiFiClient client = server.available();

    if (client) {
        String currentLine = "";
        while (client.connected()) {
            if (client.available()) {
                char c = client.read();
                strBuffer += c;
                if (c == '\n') {
                    if (currentLine.length() == 0) {
                        httpRequestProccess(&strBuffer);
                        httpSendResponse(&client);
                        break;
                    } else {
                        currentLine = "";
                    }
                } else if (c != '\r') {
                    currentLine += c;
                }
            }
        }
        client.stop();
    }
}

void httpRequestProccess(String* strbuf) {
    if (strbuf->indexOf("GET /ON") >= 0) {
        Serial.println("GET /ON");
    } else if (strbuf->indexOf("GET /OFF") >= 0) {
        Serial.println("GET /OFF");
    } 
    if (strbuf->indexOf("GET /?value=") >= 0) {
        int pos1 = strbuf->indexOf('=');
        int pos2 = strbuf->indexOf('&');
        String strSlider = strbuf->substring(pos1 + 1, pos2);
        Serial.print("Value="); Serial.println(strSlider);
        strSlider.toCharArray(SliderValue, sizeof(SliderValue));
    }
}

void httpSendResponse(WiFiClient* client) {
    client->println(strResponseHeader);
    client->println(strHtmlHeader);

    String buf = strHtmlBody;
    buf.replace("%PAGE_TITLE%", "Web Server");
    buf.replace("%LED_STATE%", "Unknown");
    buf.replace("%BUTTON_STATE%", strButtonOn);
    buf.replace("%SLIDER_VALUE%", SliderValue);
    buf.replace("%BNO055_VALUE%", String(bno055_value)); 
    client->println(buf);
    client->println();
}

void connectToWifi() {
    // Wi-Fi接続して
    Serial.print("Connecting to Wi-Fi ");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.print("  *IP address: ");
    Serial.println(WiFi.localIP());
    server.begin();
}
