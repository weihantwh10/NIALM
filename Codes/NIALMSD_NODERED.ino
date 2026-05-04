#include <PZEM004Tv30.h>
#include <WiFi.h>
extern "C" {
  #include "freertos/FreeRTOS.h"
  #include "freertos/timers.h"
}
#include <AsyncMqttClient.h>
#include <Wire.h>
#include "Statistic.h"
#include <LiquidCrystal_I2C.h>

/* Hardware Serial2 is only available on certain boards.
 * For example the Arduino MEGA 2560
*/
#if defined(ESP32)
PZEM004Tv30 pzem(Serial2, 16, 17);
#else
PZEM004Tv30 pzem(Serial2);
#endif

//replace with your network credentials
#define WIFI_SSID "weihantwh"
#define WIFI_PASSWORD "jsjh0195"

// Raspberry Pi Mosquitto MQTT Broker
#define MQTT_HOST IPAddress(192, 168, 180, 77)
#define MQTT_PORT 1883

//MQTT Topics
#define MQTT_PUB_VOL "esp32/voltage"
#define MQTT_PUB_CUR  "esp32/current"
#define MQTT_PUB_POW "esp32/power"
#define MQTT_PUB_ENE  "esp32/energy"
#define MQTT_PUB_FRE "esp32/frequency"
#define MQTT_PUB_PF "esp32/pf"
#define MQTT_PUB_VOLSD "esp32/voltageSD"
#define MQTT_PUB_CURSD  "esp32/currentSD"
#define MQTT_PUB_POWSD "esp32/powerSD"
#define MQTT_PUB_ENESD  "esp32/energySD"
#define MQTT_PUB_FRESD "esp32/frequencySD"
#define MQTT_PUB_PFSD "esp32/pfSD"

Statistic stats[6];
LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C address 0x27, 16 column and 2 rows

unsigned long previousMillis = 0; // will store last time sensor was updated
const long interval = 1000; // interval between each update (milliseconds)
int i = 0;

AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void WiFiEvent(WiFiEvent_t event) {
  Serial.printf("[WiFi-event] event: %dn", event);
  switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      connectToMqtt();
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
      xTimerStop(mqttReconnectTimer, 0); 
      xTimerStart(wifiReconnectTimer, 0);
      break;
  }
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");
  if (WiFi.isConnected()) {
    xTimerStart(mqttReconnectTimer, 0);
  }
}

void onMqttPublish(uint16_t packetId) {
  Serial.print("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}


void setup() {
  Serial.begin(9600);
  //Init LCD
  lcd.init(); 
  lcd.backlight();
  lcd.setCursor(3, 0);
  lcd.print("Welcome to");
  lcd.setCursor(4, 1);
  lcd.print("NIALM IoT");
  delay(3000);
  lcd.clear();
  delay(1000);

  // Uncomment in order to reset the internal energy counter
  // pzem.resetEnergy()
  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));

  WiFi.onEvent(WiFiEvent);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  connectToWifi();

  //explicitly start clean
  for (int j=0; j<6; j++)
  {
    stats[j].clear(); 
  }

  //Setup Data Logging to Excel
  Serial.println("CLEARDATA");
  Serial.println("LABEL, DATE, TIME, TIME ELAPSED, COUNT, VOLTAGE, CURRENT, POWER, ENERGY, FREQUENCY, PF, SDVOLTAGE, SDCURRENT, SDPOWER, SDENERGY, SDFREQUENCY, SDPF");
}


void loop() {

  unsigned long currentMillis = millis();

  // Serial.print("Custom Address:");
  // Serial.println(pzem.readAddress(), HEX);

  i++;

  // Read the data from the sensor
  float voltage = pzem.voltage();
  float current = pzem.current();
  float power = pzem.power();
  float energy = pzem.energy();
  float frequency = pzem.frequency();
  float pf = pzem.pf();
  float voltageSD;
  float currentSD;
  float powerSD;
  float energySD;
  float frequencySD;
  float pfSD;

  stats[0].add(voltage);
  stats[1].add(current);
  stats[2].add(power);
  stats[3].add(energy);
  stats[4].add(frequency);
  stats[5].add(pf);

  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("ENERGY USAGE");
  lcd.setCursor(5, 1);
  lcd.print(energy);
  lcd.print("kWh");

  // Check if the data is valid
  if(isnan(voltage)){
      Serial.println("Error reading voltage");
  } else if (isnan(current)) {
      Serial.println("Error reading current");
  } else if (isnan(power)) {
      Serial.println("Error reading power");
  } else if (isnan(energy)) {
      Serial.println("Error reading energy");
  } else if (isnan(frequency)) {
      Serial.println("Error reading frequency");
  } else if (isnan(pf)) {
      Serial.println("Error reading power factor");
  } else {

  // Print the values to the Serial console
  Serial.print("Voltage: ");      Serial.print(voltage);      Serial.println("V");
  Serial.print("Current: ");      Serial.print(current);      Serial.println("A");
  Serial.print("Power: ");        Serial.print(power);        Serial.println("W");
  Serial.print("Energy: ");       Serial.print(energy,3);     Serial.println("kWh");
  Serial.print("Frequency: ");    Serial.print(frequency, 1); Serial.println("Hz");
  Serial.print("PF: ");           Serial.println(pf);

  

  Serial.print(currentMillis / 1000);
  Serial.println("s");
  Serial.print("Number of counts:");
  Serial.println(i);
  }
  
  if (currentMillis - previousMillis >= interval) {
    voltage = stats[0].average(), 4;
    current = stats[1].average(), 4;
    power = stats[2].average(), 4;
    energy = stats[3].average(), 4;
    frequency = stats[4].average(), 4;
    pf = stats[5].average(), 4;
    voltageSD = stats[0].pop_stdev(), 4;
    currentSD = stats[1].pop_stdev(), 4;
    powerSD = stats[2].pop_stdev(), 4;
    energySD = stats[3].pop_stdev(), 4;
    frequencySD = stats[4].pop_stdev(), 4;
    pfSD = stats[5].pop_stdev(), 4;
    //Data logging to Excel
    Serial.print("DATA, DATE, TIME, ");
    Serial.print(millis());
    Serial.print(", ");
    Serial.print(i);
    Serial.print(", ");
    Serial.print(voltage);
    Serial.print(", ");
    Serial.print(current);
    Serial.print(", ");
    Serial.print(power);
    Serial.print(", ");
    Serial.print(energy);
    Serial.print(", ");
    Serial.print(frequency);
    Serial.print(", ");
    Serial.print(pf);
    Serial.print(", ");
    Serial.print(voltageSD);
    Serial.print(", ");
    Serial.print(currentSD);
    Serial.print(", ");
    Serial.print(powerSD);
    Serial.print(", ");
    Serial.print(energySD);
    Serial.print(", ");
    Serial.print(frequencySD);
    Serial.print(", ");
    Serial.print(pfSD);
    Serial.print(", ");
    Serial.println("AUTOSCROLL_20");

    // Publish an MQTT message on topic esp32/voltage
    uint16_t packetIdPub1 = mqttClient.publish(MQTT_PUB_VOL, 1, true, String(voltage).c_str());                            
    //Serial.printf("Publishing on topic %s at QoS 1, packetId: %i", MQTT_PUB_VOL, packetIdPub1);
    //Serial.printf("Message: %.2f \n", voltage);
  
    // Publish an MQTT message on topic esp32/current
    uint16_t packetIdPub2 = mqttClient.publish(MQTT_PUB_CUR, 1, true, String(current).c_str());                            
    //Serial.printf("Publishing on topic %s at QoS 1, packetId: %i", MQTT_PUB_CUR, packetIdPub2);
    //Serial.printf("Message: %.2f \n", current);
  
    // Publish an MQTT message on topic esp32/power
    uint16_t packetIdPub3 = mqttClient.publish(MQTT_PUB_POW, 1, true, String(power).c_str());                            
    //Serial.printf("Publishing on topic %s at QoS 1, packetId: %i", MQTT_PUB_POW, packetIdPub3);
    //Serial.printf("Message: %.2f \n", power);
  
    // Publish an MQTT message on topic esp32/dht/energy
    uint16_t packetIdPub4 = mqttClient.publish(MQTT_PUB_ENE, 1, true, String(energy).c_str());                            
    //Serial.printf("Publishing on topic %s at QoS 1, packetId: %i", MQTT_PUB_ENE, packetIdPub4);
    //Serial.printf("Message: %.2f \n", energy);
  
    // Publish an MQTT message on topic esp32/frequency
    uint16_t packetIdPub5 = mqttClient.publish(MQTT_PUB_FRE, 1, true, String(frequency).c_str());                            
    //Serial.printf("Publishing on topic %s at QoS 1, packetId: %i", MQTT_PUB_FRE, packetIdPub5);
    //Serial.printf("Message: %.2f \n", frequency);
  
    // Publish an MQTT message on topic esp32/pf
    uint16_t packetIdPub6 = mqttClient.publish(MQTT_PUB_PF, 1, true, String(pf).c_str());                            
    //Serial.printf("Publishing on topic %s at QoS 1, packetId: %i", MQTT_PUB_PF, packetIdPub6);
    //Serial.printf("Message: %.2f \n", pf);

    // Publish an MQTT message on topic esp32/voltageSD
    uint16_t packetIdPub7 = mqttClient.publish(MQTT_PUB_VOLSD, 1, true, String(voltageSD).c_str());                            
    //Serial.printf("Publishing on topic %s at QoS 1, packetId: %i", MQTT_PUB_VOLSD, packetIdPub1);
    //Serial.printf("Message: %.2f \n", voltageSD);
  
    // Publish an MQTT message on topic esp32/currentSD
    uint16_t packetIdPub8 = mqttClient.publish(MQTT_PUB_CURSD, 1, true, String(currentSD).c_str());                            
    //Serial.printf("Publishing on topic %s at QoS 1, packetId: %i", MQTT_PUB_CURSD, packetIdPub2);
    //Serial.printf("Message: %.2f \n", currentSD);
  
    // Publish an MQTT message on topic esp32/powerSD
    uint16_t packetIdPub9 = mqttClient.publish(MQTT_PUB_POWSD, 1, true, String(powerSD).c_str());                            
    //Serial.printf("Publishing on topic %s at QoS 1, packetId: %i", MQTT_PUB_POWSD, packetIdPub3);
    //Serial.printf("Message: %.2f \n", powerSD);
  
    // Publish an MQTT message on topic esp32/dht/energySD
    uint16_t packetIdPub10 = mqttClient.publish(MQTT_PUB_ENESD, 1, true, String(energySD).c_str());                            
    //Serial.printf("Publishing on topic %s at QoS 1, packetId: %i", MQTT_PUB_ENESD, packetIdPub4);
    //Serial.printf("Message: %.2f \n", energySD);
  
    // Publish an MQTT message on topic esp32/frequencySD
    uint16_t packetIdPub11 = mqttClient.publish(MQTT_PUB_FRESD, 1, true, String(frequencySD).c_str());                            
    //Serial.printf("Publishing on topic %s at QoS 1, packetId: %i", MQTT_PUB_FRESD, packetIdPub5);
    //Serial.printf("Message: %.2f \n", frequencySD);
  
    // Publish an MQTT message on topic esp32/pfSD
    uint16_t packetIdPub12 = mqttClient.publish(MQTT_PUB_PFSD, 1, true, String(pfSD).c_str());                            
    //Serial.printf("Publishing on topic %s at QoS 1, packetId: %i", MQTT_PUB_PFSD, packetIdPub6);
    //Serial.printf("Message: %.2f \n", pfSD);
   
    previousMillis = currentMillis;
    i=0;
    for (int j=0; j<6; j++)
    {
      stats[j].clear(); 
    } 
  } 

  //Serial.println();
  //delay(10);



  

  delay(98);

}
