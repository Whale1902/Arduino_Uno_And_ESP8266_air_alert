#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

char ssid[] = "network SSID (name)";   // network SSID (name)
char password[] = "m6v8aze8l";  // network key

WiFiClientSecure client;

// base of the URL
#define TEST_HOST "alarmmap.online"

// finferprint of the site
#define TEST_HOST_FINGERPRINT "44 98 1C 40 E4 B9 1A 3D 2E 41 E8 4F 88 13 CC 7A 16 06 39 E6"
// The finger print will change every few months.

void setup() {
  Serial.begin(115200);

  // Connect to the WiFI
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // Attempt to connect to Wifi network:
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);

  client.setFingerprint(TEST_HOST_FINGERPRINT);
}

bool isTarget = false;
bool isAlarm = true;
const String targetReference = "Запорізька_область";
const String targetReference2 = "м_Нікополь_та_Нікопольська_територіальна_громада";
const String targetReference3 = "Херсонська_область";


void makeHTTPRequest() {
  // Opening connection to server (Use 80 as port if HTTP)
  if (!client.connect(TEST_HOST, 443)) {
    Serial.println(F("Connection failed"));
    return;
  }

  // give the esp a breather
  yield();

  // Send HTTP request
  client.print(F("GET "));

  // This is the second half of a request (everything that comes after the base URL)
  client.print("/assets/json/_alarms/siren.json");  // %2C == ,
  client.println(F(" HTTP/1.1"));

  //Headers
  client.print(F("Host: "));
  client.println(TEST_HOST);

  client.println(F("Cache-Control: no-cache"));

  if (client.println() == 0) {
    Serial.println(F("Failed to send request"));
    return;
  }

  // Check HTTP status
  char status[32] = { 0 };
  client.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    return;
  }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)) {
    Serial.println(F("Invalid response"));
    return;
  }

  StaticJsonDocument<800> filter;

  JsonObject filter_0 = filter.createNestedObject();
  filter_0["district"] = true;
  filter_0["__v"] = false;
  filter_0["start"] = false;
  filter_0["sirenType"] = false;

  DynamicJsonDocument doc(2000);

  String input = client.readStringUntil('\n');

  DeserializationError error = deserializeJson(doc, input, DeserializationOption::Filter(filter));

  if (error) {
    Serial.println(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  for (JsonObject item : doc.as<JsonArray>()) {
    const String district = item["district"];
    if (district == targetReference || district == targetReference2 || district == targetReference3) {
      isTarget = true;
      Serial.println(district);
    }
  }
}

char alarmNumbers[3] = "11";
char cancelNumbers[3] = "22";
char duringAlarmNumbers[3] = "33";
char duringCancellNumber[3] = "44";

void alarmController() {
  if (isTarget == true && isAlarm == false) {
    isAlarm = true;
    Serial.println(alarmNumbers);
  } else if (isTarget == false && isAlarm == true) {
    isAlarm = false;
    Serial.println(cancelNumbers);
  } else if (isTarget == true && isAlarm == true) {
    Serial.println(duringAlarmNumbers);
  } else if (isTarget == false && isAlarm == false) {
    Serial.println(duringCancellNumber);
  }
  isTarget = false;
}

unsigned long previousCheckMillis = 0;

int checkInterval = 3000;

void loop() {
  unsigned long currentMillis = millis();

  if ((currentMillis - previousCheckMillis) > checkInterval) {

    makeHTTPRequest();
    alarmController();

    previousCheckMillis = currentMillis;
  }
}
