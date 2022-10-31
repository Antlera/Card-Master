#ifndef ARDUINO_SECRETS_H
#define ARDUINO_SECRETS_H
//wifi设置
const char *ssid = "360WiFi-E8DF1C";
const char *wifi_password = "601zuiwudi";
const char *mqttServer = "iot.ranye-iot.net";
//sql数据库设置
#define mysql_server_ip 120, 79, 147, 30
char mysql_username[] = "root";                    // Mysql的用户名
char mysql_password[] = "123456";              // 登陆Mysql的密码
// ****************************************************
// 注意！以下需要用户根据然也物联平台信息进行修改！否则无法工作!
// ****************************************************
const char *mqttUserName = "antlers_2";   // 服务端连接用户名(需要修改)
const char *mqttPassword = "ryry0101";    // 服务端连接密码(需要修改)
const char *clientId = "antlers_2_id";    // 客户端id (需要修改)
const char *subTopic = "antlers/led_ctrl"; // 订阅主题(需要修改)
const char *pubTopic = "antlers/led_state"; // 订阅主题(需要修改)
const char *willTopic = "antlers/led_lastwill"; // 遗嘱主题名称(需要修改)
// ****************************************************
#endif