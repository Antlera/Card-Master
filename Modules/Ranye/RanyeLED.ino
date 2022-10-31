/**********************************************************************
项目名称/Project          : 零基础入门学用物联网
程序名称/Program name     : ranye_iot_led
团队/Team                : 太极创客团队 / Taichi-Maker (www.taichi-maker.com)
作者/Author              : CYNO朔
日期/Date（YYYYMMDD）     : 2021-10-01
程序目的/Purpose          :
本程序旨在演示如何使用然也物联(www.ranye-iot.net)社区版实现板上LED控制。

-----------------------------------------------------------------------
本示例程序为太极创客团队制作的《零基础入门学用物联网》中示例程序。
该教程为对物联网开发感兴趣的朋友所设计和制作。如需了解更多该教程的信息，请参考以下网页：
http://www.taichi-maker.com/homepage/esp8266-nodemcu-iot/iot-tuttorial/mqtt-application/
***********************************************************************/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "arduino_secrets.h"

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);


//遗嘱相关信息
const char *willMsg = "esp8266 offline"; // 遗嘱主题信息
const int willQos = 0;                   // 遗嘱QoS
const int willRetain = false;            // 遗嘱保留

const int subQoS = 1;            // 客户端订阅主题时使用的QoS级别（截止2020-10-07，仅支持QoS = 1，不支持QoS = 2）
const bool cleanSession = false; // 清除会话（如QoS>0必须要设为false）

bool ledStatus = HIGH;

void setup()
{
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

  if ((char)payload[0] == '1')
  { // 如果收到的信息以“1”为开始
    ledStatus = LOW;
    digitalWrite(BUILTIN_LED, ledStatus); // 则点亮LED。
  }
  else
  {
    ledStatus = HIGH;
    digitalWrite(BUILTIN_LED, ledStatus); // 否则熄灭LED。
  }

  pubMQTTmsg();
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
  char *pubMessage;

  if (ledStatus == LOW)
  {
    pubMessage = "LED ON";
  }
  else
  {
    pubMessage = "LED OFF";
  }

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