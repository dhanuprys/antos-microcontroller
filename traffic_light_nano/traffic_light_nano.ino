
enum phases {
  MULAI, TRANSISI_A, TRANSISI_B, TRANSISI_C, A, B, C,  AA, BB, CC
};

#include <SoftwareSerial.h>
#include <ArduinoJson.h>
SoftwareSerial SerialPort(2, 3);
phases phase, nextPhase;
unsigned long lightsTimer = 0;
unsigned long timerA = 1000UL;
unsigned long timerB = 1000UL;// rx2 - d2 . tx2 - d3
unsigned long timerC = 1000UL;


void komunikasiSerial() {
  if (SerialPort.available()) {
    String datax = "";
    while (SerialPort.available()) {
      char inputan = (char)SerialPort.read();
      datax = datax + inputan;
    }
    Serial.println(datax);
    StaticJsonDocument<128> doc;
    DeserializationError error = deserializeJson(doc, datax);
    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      return;
    }
    const char* gateway = doc["gateway"];
    int jumlah = doc["jumlah"];
    String gateway_str = String(gateway);
    int jumlah_data = jumlah;

    Serial.println(gateway_str);
    Serial.println(jumlah_data);
    if ((gateway_str == "GA") && (jumlah_data > 0)) {
      timerA = jumlah_data * 1000UL;
      Serial.println(timerA);
    } else if ((gateway_str == "GB") && (jumlah_data > 0)) {
      timerB = jumlah_data * 1000UL;
      Serial.println(timerB);
    }
  }
  delay(1000);
}




void setup() {
  Serial.begin(9600);
  Serial.println("Traffic Controller");
  SerialPort.begin(9600);
  phase = A;

  for (int i = 4; i <= 13; i++) {
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
  }

  for (int ii = 4; ii <= 13; ii++) {
    digitalWrite(ii, HIGH);
  }

  delay(5000);
  for (int ii = 4; ii <= 13; ii++) {
    digitalWrite(ii, LOW);
  }
}

void loop() {
  komunikasiSerial();
  phaseChange();
}

void phaseChange() {
  if (millis() >= lightsTimer) {
    phase = nextPhase;
    switch (phase) {
      case MULAI:
        SerialPort.println("0|0|0");

        digitalWrite(13, HIGH);// merah
        digitalWrite(12, LOW); // kuning
        digitalWrite(11, LOW); // hijau

        digitalWrite(10, HIGH);
        digitalWrite(9, LOW);
        digitalWrite(8, LOW);

        digitalWrite(7, HIGH);
        digitalWrite(6, LOW);
        digitalWrite(5, LOW);

        nextPhase = A;
        lightsTimer = millis() + 1000UL;
        Serial.println(lightsTimer);
        break;

      case A:
        SerialPort.println("2|0|0");

        digitalWrite(13, LOW);
        digitalWrite(12, LOW);
        digitalWrite(11, HIGH); // HIJAU

        digitalWrite(10, HIGH);
        digitalWrite(9, LOW);
        digitalWrite(8, LOW);

        digitalWrite(7, HIGH);
        digitalWrite(6, LOW);
        digitalWrite(5, LOW);

        nextPhase = AA;
        lightsTimer = millis() + timerA; // JALUR GATEWAY A
        Serial.println(lightsTimer);
        break;

      case AA:
        SerialPort.println("1|1|0");

        digitalWrite(13, LOW);
        digitalWrite(12, HIGH);
        digitalWrite(11, HIGH); //HIJAU

        digitalWrite(10, HIGH);
        digitalWrite(9, HIGH);
        digitalWrite(8, LOW);

        digitalWrite(7, HIGH);
        digitalWrite(6, HIGH);
        digitalWrite(5, LOW);

        nextPhase = TRANSISI_A;
        lightsTimer = millis() + 1000UL;
        Serial.println(lightsTimer);
        break;

      case TRANSISI_A:
        SerialPort.println("0|0|2");

        digitalWrite(13, HIGH);
        digitalWrite(12, LOW);
        digitalWrite(11, LOW);
 
        digitalWrite(10, HIGH);
        digitalWrite(9, LOW);
        digitalWrite(8, LOW);// HIJAU

        digitalWrite(7, HIGH);
        digitalWrite(6, LOW);
        digitalWrite(5, LOW);

        nextPhase = B;
        lightsTimer = millis() + 1000UL;
        Serial.println(lightsTimer);
        break;

      case B:
        SerialPort.println("0|2|0");

        digitalWrite(13, HIGH);
        digitalWrite(12, LOW);
        digitalWrite(11, LOW);

        digitalWrite(10, LOW);
        digitalWrite(9, LOW);
        digitalWrite(8, HIGH); // HIJAU

        digitalWrite(7, HIGH);
        digitalWrite(6, LOW);
        digitalWrite(5, LOW);

        nextPhase = BB;
        lightsTimer = millis() + timerB;
        Serial.println(lightsTimer);
        break;

      case BB:
        SerialPort.println("1|2|0");

        digitalWrite(13, HIGH);
        digitalWrite(12, HIGH);
        digitalWrite(11, LOW);

        digitalWrite(10, LOW);
        digitalWrite(9, HIGH);
        digitalWrite(8, HIGH); // HIJAU

        digitalWrite(7, HIGH);
        digitalWrite(6, HIGH);
        digitalWrite(5, LOW);

        nextPhase = TRANSISI_B;
        lightsTimer = millis() + 1000UL;
        Serial.println(lightsTimer);
        break;

      case TRANSISI_B:
        SerialPort.println("0|0|0");

        digitalWrite(13, HIGH);
        digitalWrite(12, LOW);
        digitalWrite(11, LOW);

        digitalWrite(10, HIGH);
        digitalWrite(9, LOW);
        digitalWrite(8, LOW);// HIJAU

        digitalWrite(7, HIGH);
        digitalWrite(6, LOW);
        digitalWrite(5, LOW);

        nextPhase = C;
        lightsTimer = millis() + 1000UL;
        Serial.println(lightsTimer);
        break;


      case C:
        SerialPort.println("0|0|2");

        digitalWrite(13, HIGH);
        digitalWrite(12, LOW);
        digitalWrite(11, LOW);

        digitalWrite(10, HIGH);
        digitalWrite(9, LOW);
        digitalWrite(8, LOW);

        digitalWrite(7, LOW);
        digitalWrite(6, LOW);
        digitalWrite(5, HIGH); // HIJAU

        nextPhase = CC;
        lightsTimer = millis() + timerC;
        Serial.println(lightsTimer);
        break;

      case CC:
        SerialPort.println("0|1|2");

        digitalWrite(13, HIGH);
        digitalWrite(12, HIGH);
        digitalWrite(11, LOW);

        digitalWrite(10, HIGH);
        digitalWrite(9, HIGH);
        digitalWrite(8, LOW);

        digitalWrite(7, LOW);
        digitalWrite(6, HIGH);
        digitalWrite(5, HIGH); // HIJAU

        nextPhase = TRANSISI_C;
        lightsTimer = millis() + 1000UL;
        Serial.println(lightsTimer);
        break;

      case TRANSISI_C:
        SerialPort.println("0|0|0");
        
        digitalWrite(13, HIGH);
        digitalWrite(12, LOW);
        digitalWrite(11, LOW);

        digitalWrite(10, HIGH);
        digitalWrite(9, LOW);
        digitalWrite(8, LOW);// HIJAU

        digitalWrite(7, HIGH);
        digitalWrite(6, LOW);
        digitalWrite(5, LOW);

        nextPhase = A;
        lightsTimer = millis() + 1000UL;
        Serial.println(lightsTimer);
        break;


      default:
        SerialPort.println("0|0|0");

        digitalWrite(13, LOW);
        digitalWrite(12, LOW);
        digitalWrite(11, LOW);

        digitalWrite(10, LOW);
        digitalWrite(9, LOW);
        digitalWrite(8, LOW);

        digitalWrite(7, LOW);
        digitalWrite(6, LOW);
        digitalWrite(5, LOW);

        break;
    }
  }
}