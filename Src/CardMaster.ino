#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include "arduino_secrets.h"
//MFRC522读卡器配置
#define SS_PIN D8
#define RST_PIN D0
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key;
// Init array that will store new NUID
byte nuidPICC[4];
//Wifi连接配置
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);


//遗嘱相关信息
const char *willMsg = "esp8266 offline"; // 遗嘱主题信息
const int willQos = 0;                   // 遗嘱QoS
const int willRetain = false;            // 遗嘱保留

const int subQoS = 1;            // 客户端订阅主题时使用的QoS级别（仅支持QoS = 1，不支持QoS = 2）
const bool cleanSession = false; // 清除会话（如QoS>0必须要设为false）

bool ledStatus = HIGH;
//读卡器输出
String outputJson;
void setup()
{
  //SPI启动
  SPI.begin();     // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522
  Serial.println(F(""));
  Serial.print(F("Reader :"));
  rfid.PCD_DumpVersionToSerial();
  for (byte i = 0; i < 6; i++)
  {
    key.keyByte[i] = 0xFF;
  }
  // Wifi启动
  pinMode(LED_BUILTIN, OUTPUT);         // 设置板上LED引脚为输出模式
  digitalWrite(LED_BUILTIN, ledStatus); // 启动后关闭板上LED
  Serial.begin(9600);                   // 启动串口通讯

  //设置ESP8266工作模式为无线终端模式
  WiFi.mode(WIFI_STA);

  // 连接WiFi
  connectWifi();

  // 设置MQTT服务器和端口号
  mqttClient.setServer(mqttServer, 1883);
  mqttClient.setCallback(receiveCallback);

  // 连接MQTT服务器
  connectMQTTserver();
}

void loop()
{
  // 如果开发板未能成功连接服务器，则尝试连接服务器
  if (!mqttClient.connected())
  {
    connectMQTTserver();
  }

  // 处理信息以及心跳
  mqttClient.loop();
  cardRead();
}

// 连接MQTT服务器并订阅信息
void connectMQTTserver()
{
  // 根据ESP8266的MAC地址生成客户端ID（避免与其它ESP8266的客户端ID重名）

  /* 连接MQTT服务器
  boolean connect(const char* id, const char* user,
                  const char* pass, const char* willTopic,
                  uint8_t willQos, boolean willRetain,
                  const char* willMessage, boolean cleanSession);
  若让设备在离线时仍然能够让qos1工作，则connect时的cleanSession需要设置为false
                  */
  if (mqttClient.connect(clientId, mqttUserName,
                         mqttPassword, willTopic,
                         willQos, willRetain, willMsg, cleanSession))
  {
    Serial.print("MQTT Server Connected. ClientId: ");
    Serial.println(clientId);
    Serial.print("MQTT Server: ");
    Serial.println(mqttServer);

    subscribeTopic(); // 订阅指定主题
  }
  else
  {
    Serial.print("MQTT Server Connect Failed. Client State:");
    Serial.println(mqttClient.state());
    delay(5000);
  }
}

// 收到信息后的回调函数
void receiveCallback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message Received [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println("");
  Serial.print("Message Length(Bytes) ");
  Serial.println(length);

}

// 订阅指定主题
void subscribeTopic()
{

  // 通过串口监视器输出是否成功订阅主题以及订阅的主题名称
  // 请注意subscribe函数第二个参数数字为QoS级别。这里为QoS = 1
  if (mqttClient.subscribe(subTopic, subQoS))
  {
    Serial.print("Subscribed Topic: ");
    Serial.println(subTopic);
  }
  else
  {
    Serial.print("Subscribe Fail...");
  }
}

// 发布信息
void pubMQTTmsg()
{
  const char *pubMessage = outputJson.c_str();
  outputJson.clear();

  // 实现ESP8266向主题发布信息
  if (mqttClient.publish(pubTopic, pubMessage))
  {
    Serial.println("Publish Topic:");
    Serial.println(pubTopic);
    Serial.println("Publish message:");
    Serial.println(pubMessage);
  }
  else
  {
    Serial.println("Message Publish Failed.");
  }
}

// ESP8266连接wifi
void connectWifi()
{

  WiFi.begin(ssid, password);

  //等待WiFi连接,成功连接后输出成功信息
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi Connected!");
  Serial.println("");
}

void cardRead(){
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if (!rfid.PICC_IsNewCardPresent())
    return;
  // Verify if the NUID has been readed
  if (!rfid.PICC_ReadCardSerial())
    return;
  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));
  if (rfid.uid.uidByte[0] != nuidPICC[0] ||
      rfid.uid.uidByte[1] != nuidPICC[1] ||
      rfid.uid.uidByte[2] != nuidPICC[2] ||
      rfid.uid.uidByte[3] != nuidPICC[3])
  {
    Serial.println(F("A new card has been detected."));
    // Store NUID into nuidPICC array
    for (byte i = 0; i < 4; i++)
    {
      nuidPICC[i] = rfid.uid.uidByte[i];
    }
    Serial.println(F("The NUID tag is:"));
    Serial.print(F("In hex: "));
    createJson(rfid.uid.uidByte, rfid.uid.size,outputJson);
    Serial.println();
    pubMQTTmsg();
  }
  else
    Serial.println(F("Card read previously."));
  // Halt PICC
  rfid.PICC_HaltA();
  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}

void createJson(byte *buffer, byte bufferSize, String &output)
{
  char chars[4 * bufferSize];
  for (byte i = 0, cur = 0; i < bufferSize; i++)
  {
    if (i)
      sprintf(&chars[cur++], "%c", ' ');
    byte bits[2];
    bits[0] = (buffer[i] & 0xf0) >> 4;
    bits[1] = buffer[i] & 0x0f;
    sprintf(&chars[cur++], "%X", bits[0]);
    sprintf(&chars[cur++], "%X", bits[1]);
    if (i == bufferSize - 1)
      chars[cur + 1] = '\0';
  }
  // Null-terminate the string
  Serial.print('\n');
  Serial.print(chars);
  Serial.print('\n');
  StaticJsonDocument<200> jsonBuffer;
  jsonBuffer["request"] = "insert";
  jsonBuffer["uuid"] = chars;
  jsonBuffer["name"] = "lantingfeng";
  jsonBuffer["tel"] = "10101010101";
  serializeJson(jsonBuffer, outputJson); // 序列化JSON数据并导出字符串
  Serial.println(outputJson);
}