#include <FirebaseESP32.h>
#include <HardwareSerial.h>
#include <WiFi.h>
// WiFi credentials
#define WIFI_SSID "TB1_ACTVN"
#define WIFI_PASSWORD "1234567@9"
// Firebase credentials
#define FIREBASE_AUTH "P1tLoliGeLjFXgjCvJWKLqTTV4UAoWKR3FIuUlFg"
#define FIREBASE_HOST "https://iotzigbee-546dd-default-rtdb.firebaseio.com/"
// Firebase database instance
String path = "/SmartHomeZigbee";
FirebaseData firebaseData;
HardwareSerial zigbee(2);
String data_receiv, node, data_tx, tmp1, humi1, mq2_s, tt_den, tt_quat,s = "";
int tt ;
float temperature1;
float humidity1;
int mq2;
unsigned long curr_time, prev_time;
void setup() {

  // Start serial communication
  Serial.begin(9600);
  zigbee.begin(115200, SERIAL_8N1, 16, 17);
  // Connect to WiFi network
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(10);
    Serial.println("Connecting to WiFi...");
  }
  // Initialize Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Serial.println("Connected to firebase");
  Firebase.setString(firebaseData, path + "/LED", "off");
  Firebase.setString(firebaseData, path + "/FAN", "off");
}

void loop() {
  if (zigbee.available() > 0) {
    data_receiv = zigbee.readStringUntil('\n');
    delay(1);
  }
  curr_time = millis();
  if (curr_time - prev_time > 200) {
    prev_time = curr_time;
    tt++;
    if (tt < 10) {
      //data_tx = firebaseData.intData();
      Firebase.getString(firebaseData, path + "/LED");
      tt_den = firebaseData.stringData();
      Firebase.getString(firebaseData, path + "/FAN");
      tt_quat = firebaseData.stringData();
      //tt_den = firebaseData.stringData();   // quat
      s = s + tt_den;
      Serial.println(s);
      data_tx = "10/"  + s + "/" + tt_quat;    // 10/on
      zigbee.println(data_tx);
      s = "";
      node = getValue(data_receiv, '/', 0);
      if (node == "node1") {
        tmp1 = getValue(data_receiv, '/', 1);
        humi1 = getValue(data_receiv, '/', 2);
        temperature1 = tmp1.toFloat();
        humidity1 = humi1.toFloat();
        Firebase.setFloat(firebaseData, path + "/TEMPERATURE", temperature1);
        Firebase.setFloat(firebaseData, path + "/HUMIDITY", humidity1);
        Serial.print("Temp:");
        Serial.print(temperature1);
        Serial.print("---");
        Serial.print("Humi:");
        Serial.print(humidity1);
        Serial.println();
      }
    }
    if (tt < 20 && tt > 10) {
      data_tx = "20/"  + String(tt_den);    // tt_quat
      zigbee.println(data_tx);
      node = getValue(data_receiv, '/', 0);
      if (node == "node2") {
        mq2_s = getValue(data_receiv, '/', 1);
        mq2 = mq2_s.toInt();
        Firebase.setFloat(firebaseData, path + "/MQ2", mq2);
        Serial.print("MQ2:");
        Serial.print(mq2_s);
        Serial.println();
      }
    }
    if (tt > 20) {
      tt = 0;
    }
  }
}
String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;
  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
