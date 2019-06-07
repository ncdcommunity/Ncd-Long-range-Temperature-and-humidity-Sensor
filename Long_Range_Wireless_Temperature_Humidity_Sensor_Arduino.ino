///this code is written and tested for ncd.io wireless temperature humidity sensor with arduino due
///sensor data structure can be found here https://ncd.io/long-range-iot-wireless-temperature-humidity-sensor-product-manual/
/// sesnro can be found here https://store.ncd.io/product/industrial-long-range-wireless-temperature-humidity-sensor/
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <HardwareSerial.h>
#define WIFISSID "Dcube_web" // Put your WifiSSID here
#define PASSWORD "D@154321#" // Put your wifi password here
#define TOKEN "BBFF-fnIElEyDoFJvlYm52daR1hhxTB9Txy" // Put your Ubidots' TOKEN
#define MQTT_CLIENT_NAME "9cdef787" // MQTT client Name, please enter your own 8-12 alphanumeric character ASCII string; 
//it should be a random and unique ascii string and different from all other devices

/****************************************
   Define Constants
 ****************************************/
#define VARIABLE_LABEL "sensor" // Assing the variable label
//#define VARIABLE_LABEL2 "Battery"
#define VARIABLE_LABEL3 "Humidity"
#define DEVICE_LABEL "esp32" // Assig the device label
uint8_t data[29];
int k = 10;
int i;
char mqttBroker[]  = "industrial.api.ubidots.com";
char payload[100];
char topic[150];
//char topic2[150];
char topic3[150];
// Space to store values to send
char str_sensor[10];
//char str_sensorbat[10];
char str_sensorhumidity[10];
/****************************************
   Auxiliar Functions
 ****************************************/
WiFiClient ubidots;
PubSubClient client(ubidots);

void callback(char* topic, byte* payload, unsigned int length) {
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = NULL;
  String message(p);
  Serial.write(payload, length);
  Serial.println(topic);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");

    // Attemp to connect
    if (client.connect(MQTT_CLIENT_NAME, TOKEN, "")) {
      Serial.println("Connected");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      // Wait 2 seconds before retrying
      delay(2000);
    }
  }
}
void setup()
{
  Serial.begin(115200);
  WiFi.begin(WIFISSID, PASSWORD);
  // Assign the pin as INPUT
  Serial1.begin(115200, SERIAL_8N1, 16, 17); // pins 16 rx2, 17 tx2, 19200 bps, 8 bits no parity 1 stop bit​
  Serial.begin(9600);
  Serial.println("ncd.io IoT ESP32 Temperature and Humidity sensor");

  Serial.println();
  Serial.print("Wait for WiFi...");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  client.setServer(mqttBroker, 1883);
  client.setCallback(callback);

}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  data[0] = Serial1.read();
  delay(k);
  if (data[0] == 0x7E)
  {
    while (!Serial1.available());
    for ( i = 1; i < 29; i++)
    {
      data[i] = Serial1.read();
      delay(1);
    }
    if (data[15] == 0x7F) /////// to check if the recive data is correct
    {
      if (data[22] == 1) //////// make sure the sensor type is correct
      {
        float humidity = ((((data[24]) * 256) + data[25]) / 100.0);
        int16_t cTempint = (((uint16_t)(data[26]) << 8) | data[27]);
        float cTemp = (float)cTempint / 100.0;
        float fTemp = cTemp * 1.8 + 32;
        float battery = ((data[18] * 256) + data[19]);
        float voltage = 0.00322 * battery;
        Serial.print("Sensor Number  ");
        Serial.println(data[16]);
        Serial.print("Sensor Type  ");
        Serial.println(data[22]);
        Serial.print("Firmware Version  ");
        Serial.println(data[17]);
        Serial.print("Relative Humidity :");
        Serial.print(humidity);
        Serial.println(" %RH");
        Serial.print("Temperature in Celsius :");
        Serial.print(cTemp);
        Serial.println(" C");
        Serial.print("Temperature in Fahrenheit :");
        Serial.print(fTemp);
        Serial.println(" F");
        Serial.print("ADC value:");
        Serial.println(battery);
        Serial.print("Battery Voltage:");
        Serial.print(voltage);
        Serial.println("\n");




        if (voltage < 1)
        {
          Serial.println("Time to Replace The Battery");
        }
        dtostrf(cTemp, 4, 2, str_sensor);
       // dtostrf(voltage, 4, 2, str_sensorbat);
        dtostrf(humidity, 4, 2, str_sensorhumidity);
      }
    }
    else
    {
      for ( i = 0; i < 29; i++)
      {
        Serial.print(data[i]);
        Serial.print(" , ");
        delay(1);
      }
    }
  }
  sprintf(topic, "%s", ""); // Cleans the topic content
  sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL);

  sprintf(payload, "%s", ""); // Cleans the payload content
  sprintf(payload, "{\"%s\":", VARIABLE_LABEL); // Adds the variable label
  sprintf(payload, "%s {\"value\": %s", payload, str_sensor); // Adds the value
  sprintf(payload, "%s } }", payload); // Closes the dictionary brackets
  client.publish(topic, payload);
/*
  sprintf(topic2, "%s", ""); // Cleans the topic content
  sprintf(topic2, "%s%s", "/v1.6/devices/", DEVICE_LABEL);

  sprintf(payload, "%s", ""); // Cleans the payload content
  sprintf(payload, "{\"%s\":", VARIABLE_LABEL2); // Adds the variable label
  sprintf(payload, "%s {\"value\": %s", payload,  str_sensorbat); // Adds the value
  sprintf(payload, "%s } }", payload); // Closes the dictionary brackets

  client.publish(topic2, payload);*/
  sprintf(topic3, "%s", ""); // Cleans the topic content
  sprintf(topic3, "%s%s", "/v1.6/devices/", DEVICE_LABEL);

  sprintf(payload, "%s", ""); // Cleans the payload content
  sprintf(payload, "{\"%s\":", VARIABLE_LABEL3); // Adds the variable label
  sprintf(payload, "%s {\"value\": %s", payload,  str_sensorhumidity); // Adds the value
  sprintf(payload, "%s } }", payload); // Closes the dictionary brackets

  client.publish(topic3, payload);

  client.loop();
  delay(1000);
}
