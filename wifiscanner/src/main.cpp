#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

const char* ssid = "ag_adi";         // Wi-Fi ağ adı
const char* password = "ag_sifre"; // Wi-Fi şifresi

AsyncWebServer server(80);  // Web sunucusu portu (80, HTTP için varsayılan port)

// Şifreleme türünü döndürmek için yardımcı fonksiyonun prototipi
String getEncryptionType(int encType);

void setup() {
  Serial.begin(9600);

  // Wi-Fi ağını başlat
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Wi-Fi'ye bağlanıyor...");
  }

  Serial.println("Wi-Fi'ye bağlandık!");
  Serial.print("IP Adresi: ");
  Serial.println(WiFi.localIP());

  // Web sayfası isteği geldiğinde HTML içeriğini döndür
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<html><head><title>Wi-Fi Ağları</title></head><body><h1>Çevredeki Wi-Fi Ağları</h1>";
    html += "<table border='1'><tr><th>SSID</th><th>Sinyal Gücü</th><th>Şifreleme</th></tr>";
    html += "<tbody id='wifi-table'></tbody></table>";
    html += "<script>setInterval(function() { fetch('/scan').then(response => response.json()).then(data => { let table = document.getElementById('wifi-table'); table.innerHTML = ''; data.forEach(network => { let row = table.insertRow(); row.insertCell(0).innerText = network.ssid; row.insertCell(1).innerText = network.rssi; row.insertCell(2).innerText = network.encryption; }); }); }, 5000);</script>";
    html += "</body></html>";
    request->send(200, "text/html", html);  // HTML içeriğini web tarayıcısına gönder
  });

  // Wi-Fi tarama sonuçlarını JSON formatında döndür
  server.on("/scan", HTTP_GET, [](AsyncWebServerRequest *request){
    int n = WiFi.scanNetworks();  // Çevredeki Wi-Fi ağlarını tara
    DynamicJsonDocument doc(1024);
    JsonArray array = doc.to<JsonArray>();

    for (int i = 0; i < n; i++) {
      JsonObject obj = array.createNestedObject();
      obj["ssid"] = WiFi.SSID(i);
      obj["rssi"] = WiFi.RSSI(i);
      obj["encryption"] = getEncryptionType(WiFi.encryptionType(i));
    }

    String json;
    serializeJson(doc, json);
    request->send(200, "application/json", json);  // JSON içeriğini web tarayıcısına gönder
  });

  // Web sunucusunu başlat
  server.begin();
}

void loop() {
  // Sürekli tarama yapmak isterseniz buraya eklemeler yapabilirsiniz.
}

// Şifreleme türünü döndürmek için yardımcı fonksiyon
String getEncryptionType(int encType) {
  switch (encType) {
    case WIFI_AUTH_OPEN: return "Açık";
    case WIFI_AUTH_WEP: return "WEP";
    case WIFI_AUTH_WPA_PSK: return "WPA PSK";
    case WIFI_AUTH_WPA2_PSK: return "WPA2 PSK";
    case WIFI_AUTH_WPA3_PSK: return "WPA3 PSK";
    default: return "Bilinmiyor";
  }
}