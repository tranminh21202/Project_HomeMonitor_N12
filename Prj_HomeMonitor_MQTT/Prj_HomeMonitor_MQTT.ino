#include <DHT.h>
#include <Wire.h>
#include <WebServer.h>
#include <ArduinoJson.h> 
#include <SoftwareSerial.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085.h>
#include <PubSubClient.h>

#define DHTPIN 2    // Pin kết nối cho cảm biến DHT11
#define DHTTYPE DHT11  // Loại cảm biến DHT
#define BMP_SDA 21
#define BMP_SCL 22
#define GAS_PIN 35  // Pin kết nối cho cảm biến khí gas MQ-02
#define DIG_PIN 14 
#define LED_PIN 13    // Pin kết nối cho đèn LED cảnh báo
#define BUZZER_PIN 12 // Pin kết nối cho module còi báo hiệu
#define LED_CONTROL 25    // Pin kết nối cho điều khiển đèn LED cảnh báo
#define BUZZER_CONTROL 18 // Pin kết nối cho điều khiển còi báo hiệu


// Thông số cho cảm biến gas
#define GAS_THRESHOLD 1900

// Thông số cho cảm biến DHT
#define TEMP_THRESHOLD 60

// Thông số cho SIM800L
#define SIM800_RX_PIN 16
#define SIM800_TX_PIN 17
#define PHONE_NUMBER "837633760"  // Thay đổi số điện thoại nhận cảnh báo

// Thông số cho kết nối WiFi

const char* ssid = "Tang 1";  // Your SSID
const char* password = "phuongnvH"; // Your Password

// const char* ssid = "Dung";  // Your SSID
// const char* password = "12346789"; // Your Password

unsigned long previousMillis = 0; // Tracks the time since last event fired

const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* mqtt_username = "tranminh21202";
const char* mqtt_password = "Tranminh21202";

float temperature; // holds the temperature value
float humidity;// holds the Humidity value
float pressure;
int gasValue;
int smokeValue;

DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085 bmp;
SoftwareSerial sim800lSerial(SIM800_TX_PIN, SIM800_RX_PIN); // RX, TX

// Kết nối tới MQTT Broker
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Wire.begin();
  Serial.begin(115200);
  sim800lSerial.begin(115200);
  dht.begin();
  if (!bmp.begin()) {
    Serial.println("Starting BMP180 sensor!");
    while (1);
  }
  pinMode(GAS_PIN,INPUT);
  pinMode(DIG_PIN,INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_CONTROL, OUTPUT);
  pinMode(BUZZER_CONTROL, OUTPUT);

  // Kết nối WiFi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  Serial.println("System initialized. Monitoring started...");
}


void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  pressure = bmp.readPressure() / 100.0F;
  gasValue = analogRead(GAS_PIN);
  smokeValue = digitalRead(DIG_PIN);
  //temperature = 65;
  // humidity = random(100);
  //gasValue = 700;
  //smokeValue = 0;
  unsigned long currentMillis = millis(); 
  if ((unsigned long)(currentMillis - previousMillis) >= 12000) { 
    String temp = String(temperature, 2); 
    client.publish("/prj/temp", temp.c_str()); 
    
    String hum = String(humidity, 2); 
    client.publish("/prj/hum", hum.c_str());

    String pre = String(pressure); 
    client.publish("/prj/pre", pre.c_str());

    String gas = String(gasValue); 
    client.publish("/prj/gas", gas.c_str());

    String smoke = String(smokeValue); 
    client.publish("/prj/smoke", smoke.c_str());

    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print(" °C, Humidity: ");
    Serial.print(humidity);
    Serial.print(" %, Pressure: ");
    Serial.print(pressure);
    Serial.print(" hPa, Gas Value: ");
    Serial.print(gasValue);
    Serial.print(" ");
    if(!smokeValue){
      Serial.println("  |  Smoke: Detected!");
    }
    else{
      Serial.println("  |  Smoke: None!");
    }

    // Kiểm tra nhiệt độ
    if (temperature > TEMP_THRESHOLD || gasValue > GAS_THRESHOLD) {
      if(temperature > TEMP_THRESHOLD && gasValue > GAS_THRESHOLD && smokeValue == 0){
        digitalWrite(LED_PIN, HIGH);  // Bật đèn cảnh báo
        tone(BUZZER_PIN, 1000);        // Bật còi báo hiệu
        sendsms(PHONE_NUMBER, "Canh bao chay no nghiem trong!!!");
      }
      else if(temperature > TEMP_THRESHOLD){
        digitalWrite(LED_PIN, HIGH);  // Bật đèn cảnh báo
        tone(BUZZER_PIN, 1000);        // Bật còi báo hiệu
        sendsms(PHONE_NUMBER, "Nhiet do cao bat thuong!!");
      }
      else if(gasValue > GAS_THRESHOLD && smokeValue == 0){
        digitalWrite(LED_PIN, HIGH);  // Bật đèn cảnh báo
        tone(BUZZER_PIN, 1000);        // Bật còi báo hiệu
        sendsms(PHONE_NUMBER, "Khi gas cao bat thuong!!");
      }
    }
    // Kiểm tra khí gas
    else{
      turnoffAlarm();
    }
    previousMillis = currentMillis;   // Use the snapshot to set track time until next event
  }

}

void sendAlert(String message) {
  // Serial.println("Sending alert message..." + message);
  digitalWrite(LED_PIN, HIGH);  // Bật đèn cảnh báo
  tone(BUZZER_PIN, 1000);        // Bật còi báo hiệu

  // Gửi tin nhắn cảnh báo
  //sendATCommand(sms);
  //sendATCommand(message);
  // String sdt = "837633760";
  // sendsms(sdt, message);
  //sendATCommand(str(26));  // Ctrl+Z để kết thúc tin nhắn
}

// void sendATCommand(String command) {
//   sim800lSerial.println("AT"); 
//   updateSerial();
//   delay(500);
//   sim800lSerial.println("AT+CMGF=1");
//   updateSerial();
//   delay(500);
//   sim800lSerial.println("AT+CMGS=\"0837633760\"");
//   updateSerial();
//   delay(500);
//   sim800lSerial.println(command);
//   updateSerial();
//   delay(500);
//   sim800lSerial.write(26);
//   //Serial.println(command);
//   //sim800lSerial.println(char(26));
// }

void sendsms(String sdt, String content){
  sim800lSerial.println("AT");
  updateSerial();
  Serial.println("Đang gửi tin nhắn");
  sim800lSerial.println("AT+CMGF=1");
  updateSerial();
  delay(500);
  sim800lSerial.println("AT+CMGS=\"+84"+sdt+"\"");
  updateSerial();
  delay(500);
  sim800lSerial.print(content);
  updateSerial();
  delay(5500);
  // sim800lSerial.write(26);
  sim800lSerial.print(char(26));
  delay(2000);
  Serial.println("Send oke!");
}

void updateSerial()
{
  delay(500);
  while (Serial.available()) 
  {
    sim800lSerial.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while(sim800lSerial.available()) 
  {
    Serial.write(sim800lSerial.read());//Forward what Software Serial received to Serial Port
  }
}

void turnoffAlarm() {
  Serial.println("Turning off alarm...");
  delay(1000);  // Bật đèn và còi báo hiệu trong 5 giây
  digitalWrite(LED_PIN, LOW);   // Tắt đèn cảnh báo
  noTone(BUZZER_PIN);            // Tắt còi báo hiệu
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";
    //String clientId = "clientId-fCN3lAzzqQ";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("Connected to " + clientId);
      
      //client.publish("/Minh_127/p/mqtt", "Minh_127"); 
      
      client.subscribe("/prj/temp"); 
      client.subscribe("/prj/hum");
      client.subscribe("/prj/pre");
      client.subscribe("/prj/gas");
      client.subscribe("/prj/smoke");
      client.subscribe("/prj/led");
      client.subscribe("/prj/buz");
      client.subscribe("/prj/control/led");
      client.subscribe("/prj/control/buz");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String incommingMessage = "";
  for (int i = 0; i < length; i++) incommingMessage+=(char)payload[i];

  Serial.println("Message arrived ["+String(topic)+"] "+incommingMessage);

  //--- check the incomming message
  if( strcmp(topic,"/prj/led") == 0){
    if (incommingMessage.equals("on")){
      digitalWrite(LED_PIN, HIGH);   // Turn the LED on
      delay(5000);
    } 
    else if (incommingMessage.equals("off")) digitalWrite(LED_PIN, LOW);  // Turn the LED off
  }
  if( strcmp(topic,"/prj/buz") == 0){
    if (incommingMessage.equals("on")){
      tone(BUZZER_PIN, 1000);   // Turn the Buz on
      delay(5000);
    } 
    else if (incommingMessage.equals("off")) noTone(BUZZER_PIN);   // Turn the Buz off
  }

  if( strcmp(topic,"/prj/control/led") == 0){
    if (incommingMessage.equals("on")){
      digitalWrite(LED_CONTROL, HIGH);   // Turn the LED on
      delay(500);
    } 
    else if (incommingMessage.equals("off")) digitalWrite(LED_CONTROL, LOW);  // Turn the LED off
  }
  if( strcmp(topic,"/prj/control/buz") == 0){
    if (incommingMessage.equals("on")){
      tone(BUZZER_CONTROL, 1000);   // Turn the Buz on
      delay(500);
    } 
    else if (incommingMessage.equals("off")) noTone(BUZZER_CONTROL);   // Turn the Buz off
  }
}
