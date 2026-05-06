#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const char* ssid = "wifi name";
const char* password = "password";

#define SDA 47
#define SCL 21

Adafruit_SSD1306 display(128, 64, &Wire);

WiFiClientSecure client;
HTTPClient http;

String titles[10]; // 存放新聞標題
int titleCount = 0;
int currentIndex = 0;
unsigned long lastSwitch = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA, SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  client.setInsecure();
  http.begin(client, "https://feeds.bbci.co.uk/news/world/rss.xml");
  int httpCode = http.GET();
  if (httpCode == 200) {
    String payload = http.getString();
    int pos = 0;
    while (titleCount < 10) { // 最多抓 10 條
      int itemStart = payload.indexOf("<item>", pos);
      if (itemStart == -1) break;
      int titleStart = payload.indexOf("<title>", itemStart);
      int titleEnd = payload.indexOf("</title>", titleStart);
      if (titleStart == -1 || titleEnd == -1) break;
      String title = payload.substring(titleStart + 7, titleEnd);
      title.replace("<![CDATA[", "");
      title.replace("]]>", "");
      titles[titleCount++] = title;
      pos = titleEnd;
    }
    Serial.printf("Got %d titles\n", titleCount);
  } else {
    Serial.printf("HTTP GET failed, code: %d\n", httpCode);
  }
  http.end();
}

void loop() {
  if (titleCount > 0 && millis() - lastSwitch > 10000) { // 每 10 秒切換
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(titles[currentIndex]);
    display.display();

    Serial.println("Show: " + titles[currentIndex]);

    currentIndex = (currentIndex + 1) % titleCount;
    lastSwitch = millis();
  }
}
