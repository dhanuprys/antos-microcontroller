#include <SPI.h>
#include <LoRa.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ESPAsyncWebServer.h>
#include <HardwareSerial.h>

#define WIFI_INDICATOR_PIN 13
#define MQTT_INDICATOR_PIN 12

#define RXp2 16
#define TXp2 17

// SSID dan password access point pada ESP32
const char* ssid = "ANTOS_GATEWAY";
const char* password = "antos123";

// HOST default untuk terhubung ke MQTT_BROKER
// Dapat diganti melalui webserver ESP32
char MQTT_BROKER[15] = "192.168.4.2";

unsigned long currentMillis = 0;
unsigned long previousWifiMillis = 0;
unsigned long previousMqttMillis = 0;
unsigned long previousIndicatorMillis = 0;

const int csPin = 5;
const int resetPin = 14;
const int irqPin = 2;

byte localAddress = 0xFF;  // alamat device pengirim
long lastSendTime = 0;
int interval = 2000;
int count = 0, countB = 0;
String dataMobil[100], dataMobilB[100];
int x = 0, y = 0;
String sensorData;
String incomingData[20];
int arr = 0, xx = 0;
int datax;
String gateway_serial;

HardwareSerial SerialPort(2);
WiFiClient espClient;
PubSubClient client(espClient);
AsyncWebServer server(80);

#ifdef __cplusplus
extern "C" {
#endif
  uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();

void initWiFi() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  // ------------------------------------------------------
  // DIAKTIFKAN JIKA MENGGUNAKAN ESP32 sebagai station
  // WiFi.begin(ssid, password);
  // Serial.print("Connecting to WiFi ..");
  // while (WiFi.status() != WL_CONNECTED) {
  //   Serial.print('.');
  //   delay(1000);
  // }
  // ------------------------------------------------------
}

void connectToMQTT() {
  Serial.print("Connecting to broker: ");
  Serial.println(MQTT_BROKER);
  client.disconnect();
  client.setServer(MQTT_BROKER, 1883);
  client.connect("ESP32");

  // delay(100);
}

void setup() {
  Serial.begin(9600);
  SerialPort.begin(9600, SERIAL_8N1, RXp2, TXp2);
  Serial.println("Start LoRa duplex");

  pinMode(MQTT_INDICATOR_PIN, OUTPUT);

  LoRa.setPins(csPin, resetPin, irqPin);

  if (!LoRa.begin(915E6)) {
    Serial.println("LoRa init failed. Check your connections.");
    while (true) {}
  }

  initWiFi();

  // // Membuat url endpoint untuk melakukan perubahan MQTT_BROKER secara dinamis
  server.on("/mqtt_host", HTTP_GET, [](AsyncWebServerRequest* request) {
    AsyncWebParameter* p = request->getParam(0);

    if (p->name() == "v") {
      p->value().toCharArray(MQTT_BROKER, 15);
      client.disconnect();
      client.setServer(MQTT_BROKER, 1883);
      client.connect("ESP32");
      request->send(200, "text/plain", p->value());
      return;
    }

    request->send(200, "text/plain", "");
  });

  // Memulai web server
  server.begin();
  connectToMQTT();
}

void loop() {
  //  suhuChip();
  currentMillis = millis();

  receiveMessage(LoRa.parsePacket());
  int paket = LoRa.parsePacket();
  if (paket) {
    terimaDataGateway();
  }

  checkMQTTConnectivity();

  updateIndicator();

  client.loop();

  if (SerialPort.available()) {
    String data = "";

    while (SerialPort.available() > 0) {
      data += (char)SerialPort.read();
      delay(10);
    }

    Serial.println(data);
    client.publish("light", data.c_str());
  }
}

void updateIndicator() {
  // Secara berkala (1 detik) mengecek konektivitas beberapa komponen dan mengubahnya
  // ke dalam indikator LED
  if (currentMillis - previousIndicatorMillis >= 2000) {
    // digitalWrite(WIFI_INDICATOR_PIN, WiFi.softAPgetStationNum() > 0);
    digitalWrite(MQTT_INDICATOR_PIN, client.connected());

    delay(10);

    previousIndicatorMillis = currentMillis;
  }
}

void checkWiFiConnectivity() {
  // -------------------------------------------------------------------------------------------
  // DIAKTIFKAN KETIKA MENGGUNAKAN ESP32 SEBAGAI STATION
  // if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousWifiMillis >= 30000)) {
  //   Serial.println("Reconnecting to WiFi...");
  //   WiFi.disconnect();
  //   WiFi.reconnect();
  //   previousWifiMillis = currentMillis;
  // }
  // -------------------------------------------------------------------------------------------
}

void checkMQTTConnectivity() {
  // Secara berkala mengecek konektivitas antara ESP32 dan Webserver untuk
  // keperluan pertukaran data
  if (!client.connected() && (currentMillis - previousMqttMillis >= 10000)) {
    connectToMQTT();

    previousMqttMillis = currentMillis;
  }
}


void komunikasiSerial() {
  //datax = count;
  //  while (Serial2.available()) {
  //    Serial.print(char(Serial2.read()));
  //  }

  String data_jsonx = "";
  StaticJsonDocument<128> docsx;
  docsx["gateway"] = gateway_serial;

  if (gateway_serial == "GA") {
    datax = count;
  } else if (gateway_serial == "GB") {
    datax = countB;
  }

  docsx["jumlah"] = datax;
  serializeJson(docsx, data_jsonx);
  Serial2.println(data_jsonx);
  Serial.println(data_jsonx);
  //delay(1000);
}

void terimaDataGateway() {
  if (millis() - lastSendTime > 5000) {

    for (xx = 0; xx <= 20; xx++) {
      StaticJsonDocument<128> doc;
      DeserializationError error = deserializeJson(doc, incomingData[xx]);

      if (error) {
        //      Serial.print("deserializeJson() failed: ");
        //      Serial.println(error.c_str());
        return;
      }

      const char* gateway = doc["gateway"];
      const char* mobil = doc["mobil"];
      String gateway_str = String(gateway);
      gateway_serial = gateway_str;

      if (gateway_str == "GA") {
        dataMobil[x] = mobil;
        x++;
            //  Serial.println("Data mobil GA: " + dataMobil[x]);

        if (x >= 20) {
          duplikatDataGA();
          komunikasiSerial();
          x = 0;
        }
      } else {
      }


      if (gateway_str == "GB") {
        dataMobilB[y] = mobil;
        y++;
            //  Serial.println("Data mobil GB: " + dataMobilB[y]);

        if (y >= 20) {
          duplikatDataGB();
          komunikasiSerial();
          y = 0;
        }
      } else {
      }


      lastSendTime = millis();
    }
  }
}

void receiveMessage(int packetSize) {
  //  Serial.println("Standby.......................!");
  if (packetSize == 0) return;

  int recipient = LoRa.read();
  byte sender = LoRa.read();
  byte incomingLength = LoRa.read();

  String incoming = "";

  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }

  if (incomingLength != incoming.length()) {
    //Serial.println("Error: Message length does not match length");
    return;
  }

  if (recipient != localAddress) {
    //Serial.println("Error: Recipient address does not match local address");
    return;
  }
  incomingData[arr] = incoming;
  //  Serial.println("ID mobil terdeteksi : " + incomingData[arr]);
  arr++;
  if (arr >= 20) {
    arr = 0;
  }
}


void duplikatDataGA() {
  count = 0;
  int i, j, k, sizeA = 20;

  for (i = 0; i < sizeA; i++) {
    for (j = i + 1; j < sizeA; j++) {
      // use if statement to check duplicate element
      if (dataMobil[i] == dataMobil[j]) {
        // delete the current position of the duplicate element
        for (k = j; k < sizeA - 1; k++) {
          dataMobil[k] = dataMobil[k + 1];
        }
        // decrease the size of array after removing duplicate element
        sizeA--;

        // if the position of the elements is changes, don't increase the index j
        j--;
      }
    }
  }

  for (i = 0; i < sizeA; i++) {
    Serial.println("Data Filter GA : " + dataMobil[i]);
    count++;
    //Serial.println("Jumlah  : " + i);
  }

  Serial.print("------------------------------------>> jumlah kendaraan GA : ");
  Serial.println(count);
  client.publish("vehicle", (String(count) + "|" + String(countB) + "|0").c_str());
}

void duplikatDataGB() {
  countB = 0;
  int ib, jb, kb, sizeB = 20;

  for (ib = 0; ib < sizeB; ib++) {
    for (jb = ib + 1; jb < sizeB; jb++) {
      // use if statement to check duplicate element
      if (dataMobilB[ib] == dataMobilB[jb]) {
        // delete the current position of the duplicate element
        for (kb = jb; kb < sizeB - 1; kb++) {
          dataMobil[kb] = dataMobil[kb + 1];
        }
        // decrease the size of array after removing duplicate element
        sizeB--;

        // if the position of the elements is changes, don't increase the index j
        jb--;
      }
    }
  }

  for (ib = 0; ib < sizeB; ib++) {
    Serial.println("Data Filter GB : " + dataMobilB[ib]);
    countB++;
    //Serial.println("Jumlah  : " + i);
  }
  Serial.print("------------------------------------>> jumlah kendaraan GB : ");
  Serial.println(countB);
  client.publish("vehicle", (String(count) + "|" + String(countB) + "|0").c_str());
}



void suhuChip() {
  Serial.print("Temperature: ");
  // Convert raw temperature in F to Celsius degrees
  Serial.print((temprature_sens_read() - 32) / 1.8);
  Serial.println(" C");
}