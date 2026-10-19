#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>

const char *ssid = "";
const char *password = "";

#define pin_sensor_dht D2
#define type_sensor DHT11

ESP8266WebServer server(80);
DHT dht(pin_sensor_dht, type_sensor);

// 1. Головна HTML сторінка з JavaScript-дашбордом
void handleRoot()
{
  String html = R"(
    <!DOCTYPE html>
    <html>
    <head>
      <meta charset="utf-8">
      <title>ESP8266 Dashboard</title>
      <style>
        body { font-family: Arial, sans-serif; text-align: center; margin-top: 50px; background: #f4f4f9; }
        .card { background: white; padding: 25px; border-radius: 12px; display: inline-block; box-shadow: 0 4px 12px rgba(0,0,0,0.1); }
        h2 { color: #333; }
        .value { font-size: 2.5em; color: #007BFF; font-weight: bold; }
        .unit { font-size: 1.2em; color: #555; }
      </style>
    </head>
    <body>
      <div class="card">
        <h2>ESP8266 Sensor Dashboard</h2>
        <p>Temperature: <span id="temp" class="value">--</span> <span class="unit">&deg;C</span></p>
        <p>Humidity: <span id="hum" class="value">--</span> <span class="unit">%</span></p>
      </div>

      <script>
        function updateData() {
          fetch('/json')
            .then(response => response.json())
            .then(data => {
              document.getElementById('temp').innerText = data.temperature;
              document.getElementById('hum').innerText = data.humidity;
            })
            .catch(error => console.error('Помилка оновлення даних:', error));
        }

        // Оновлювати дані кожні 2 секунди
        setInterval(updateData, 2000);
        updateData();
      </script>
    </body>
    </html>
  )";

  server.send(200, "text/html", html);
}

// 2. Ендпоінт для видачі даних у форматі JSON
void handleJson()
{
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (isnan(temp))
    temp = 0.0;
  if (isnan(hum))
    hum = 0.0;

  String json = "{";
  json += "\"temperature\":" + String(temp, 1) + ",";
  json += "\"humidity\":" + String(hum, 1);
  json += "}";

  server.send(200, "application/json", json);
}

void setup()
{
  Serial.begin(115200);
  dht.begin();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.println();
  Serial.printf("Connecting to %s ", ssid);

  unsigned long startAttemptTime = millis();

  // Чекаємо підключення максимум 20 секунд із захистом від зависання
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 20000)
  {
    delay(500);
    Serial.print(".");
    yield();
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("\nWiFi connected successfully!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    server.on("/", HTTP_GET, handleRoot);
    server.on("/json", HTTP_GET, handleJson);

    server.begin();
    Serial.println("Web server started!");
  }
  else
  {
    Serial.println("\nFailed to connect to Wi-Fi. Check SSID/Password or router band (2.4 GHz required).");
  }
}

void loop()
{
  server.handleClient();
}
