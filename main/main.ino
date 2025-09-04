#define TINY_GSM_MODEM_SIM800

#include <SoftwareSerial.h>
#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <avr/wdt.h>

#define TRIG_PIN 8
#define ECHO_PIN 9
#define FLOW_PIN 2
#define LED_GREEN 10
#define LED_YELLOW 3
#define LED_RED 4
#define MODEM_RX 7
#define MODEM_TX 6

float tinggiSensorCm = 110.0;
float lastValidDistance = -1;

volatile unsigned long pulse = 0;
float calibrationFactor = 0.1253;

unsigned long lastReadTime = 0;
unsigned long lastPublishTime = 0;
const unsigned long readInterval = 1000;
const unsigned long publishInterval = 60000;

SoftwareSerial SerialAT(MODEM_RX, MODEM_TX);

const char apn[] = "M2MAUTOTRONIC";
const char gprsUser[] = "";
const char gprsPass[] = "";

const char* broker = "18.142.250.134";
const char* mqttUsername = "IOT";
const char* mqttPassword = "iot123";
const char* mqttTopic = "sensor/data";

TinyGsm modem(SerialAT);
TinyGsmClient client(modem);
PubSubClient mqtt(client);

void increase() {
  pulse++;
}

bool mqttConnect() {
  return mqtt.connect("GsmClientN", mqttUsername, mqttPassword);
}

float bacaJarak() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long durasi = pulseIn(ECHO_PIN, HIGH, 30000);
  if (durasi == 0) return 0.0;
  return (durasi * 0.0343) / 2.0;
}

float getMedian(float a, float b, float c) {
  if ((a >= b && a <= c) || (a >= c && a <= b)) return a;
  if ((b >= a && b <= c) || (b >= c && b <= a)) return b;
  return c;
}

void blinkYellowSetup() {
  for (int i = 0; i < 6; i++) {
    digitalWrite(LED_YELLOW, HIGH);
    delay(300);
    digitalWrite(LED_YELLOW, LOW);
    delay(300);
  }
}

void blinkAllSuccess() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_YELLOW, HIGH);
    digitalWrite(LED_RED, HIGH);
    delay(200);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_RED, LOW);
    delay(200);
  }
}

void blinkRedReconnect() {
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_RED, HIGH);
    delay(300);
    digitalWrite(LED_RED, LOW);
    delay(300);
  }
}

void resetModem() {
  modem.restart();
  modem.gprsDisconnect();
  modem.gprsConnect(apn, gprsUser, gprsPass);
}

void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(FLOW_PIN, INPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(FLOW_PIN), increase, RISING);
  SerialAT.begin(9600);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);
  blinkYellowSetup();

  modem.restart();
  modem.gprsDisconnect();
  while (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    delay(500);
  }

  mqtt.setServer(broker, 1883);
  while (!mqttConnect()) {
    blinkRedReconnect();
    delay(500);
  }
  blinkAllSuccess();
}

void loop() {
  static float flowrate = 0.0;
  unsigned long now = millis();

  if (!modem.isGprsConnected()) {
    resetModem();
  }

  if (!mqtt.connected()) {
    blinkRedReconnect();
    mqttConnect();
  }

  mqtt.loop();

  if (now - lastReadTime >= readInterval) {
    lastReadTime = now;

    // Hitung debit air
    noInterrupts();
    unsigned long count = pulse;
    pulse = 0;
    interrupts();
    flowrate = count * calibrationFactor;

    // Baca 3x jarak dan ambil median
    float d1 = bacaJarak(); delay(50);
    float d2 = bacaJarak(); delay(50);
    float d3 = bacaJarak();

    if (d1 != 0 && d2 != 0 && d3 != 0) {
      float median = getMedian(d1, d2, d3);
      lastValidDistance = median;

      float tinggiAir = tinggiSensorCm - median;
      if (tinggiAir < 0) tinggiAir = 0;

      // Update indikator LED setiap detik
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_YELLOW, LOW);
      digitalWrite(LED_RED, LOW);

      float level = tinggiAir;
      if (level < 0.20) {
        digitalWrite(LED_GREEN, HIGH);
      } else if (level <= 0.80) {
        digitalWrite(LED_YELLOW, HIGH);
      } else {
        digitalWrite(LED_RED, HIGH);
      }
    }
  }

  // Kirim data ke MQTT setiap 1 menit
  if (now - lastPublishTime >= publishInterval) {
    lastPublishTime = now;
    if (lastValidDistance >= 0) {
      float tinggiAir = tinggiSensorCm - lastValidDistance;
      if (tinggiAir < 0) tinggiAir = 0;

      StaticJsonDocument<128> doc;
      doc["tinggi_air"] = tinggiAir;
      doc["debit"] = flowrate;

      char buffer[128];
      serializeJson(doc, buffer);
      mqtt.publish(mqttTopic, buffer, true);
    }
  }
}
