#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <U8g2lib.h>

// WiFi-Zugangsdaten
const char* ssid = "Your ssid";
const char* password = "password";

// Tankerkönig API-Details
const char* apiKey = "apiKey";
const char* stationID = "Tankstellen-ID"; // Tankstellen-ID

// OLED-Display-Setup (I2C mit SCL=10, SDA=8)
U8G2_SSD1306_64X48_ER_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 10, /* data=*/ 8, /* reset=*/ U8X8_PIN_NONE);


void setup() {
  // Serieller Monitor für Debugging
  Serial.begin(115200);

  // WLAN-Verbindung herstellen
  WiFi.begin(ssid, password);
  Serial.print("Verbinde mit WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi verbunden!");

  // OLED-Display initialisieren
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_5x8_tr);
  u8g2.drawStr(0, 25, "Starte...");
  u8g2.sendBuffer();
}

void loop() {
  // Benzinpreis abrufen und auf dem Display anzeigen
  fetchAndDisplayFuelPrice();

  // 10 Minuten warten, bevor die nächste Abfrage erfolgt
  delay(600000);
}

// Funktion: Benzinpreis abrufen und auf dem OLED-Display anzeigen
void fetchAndDisplayFuelPrice() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Tankerkönig API-URL zusammenstellen
    String url = "https://creativecommons.tankerkoenig.de/json/prices.php?apikey=";
    url += apiKey;
    url += "&ids=";
    url += stationID;

    http.begin(url);
    int httpCode = http.GET();

    if (httpCode == 200) { // Erfolgreiche Anfrage
      String payload = http.getString();
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, payload);

      if (doc["ok"]) {
        float price = doc["prices"][stationID]["e5"];
        // Kleinere Schriftart und neue Positionierung
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_5x8_tr); // Kleine Schriftart
        u8g2.setCursor(0,12);
        u8g2.print("Benzinpreis: ");
        u8g2.setFont(u8g2_font_6x10_tr);
        u8g2.setCursor(0,24);
        u8g2.print(price, 3);
        u8g2.print(" EURO");
        u8g2.sendBuffer();

      } else {
        u8g2.clearBuffer();
        u8g2.drawStr(0, 8, "Fehler API"); // Fehleranzeige
        u8g2.sendBuffer();
      }
    } else {
      u8g2.clearBuffer();
      u8g2.drawStr(0, 8, "HTTP Fehler!"); // HTTP-Fehleranzeige
      u8g2.sendBuffer();
    }
    http.end();
  } else {
    u8g2.clearBuffer();
    u8g2.drawStr(0, 8, "WiFi Error"); // WLAN-Fehleranzeige
    u8g2.sendBuffer();
  }
}
