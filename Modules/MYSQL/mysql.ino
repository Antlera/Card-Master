#include <ESP8266WiFi.h>      // esp8266库
#include <MySQL_Connection.h> // Arduino连接Mysql的库
#include <MySQL_Cursor.h>

#include "lib/arduino_secrets.h"

// Mysql中添加一条数据的命令
// arduino_test，TestTable：刚才创建的数据和表
char INSERT_SQL[] = "INSERT INTO  arduino_test.TestTable(CardUUID) VALUES ('%d')";


WiFiClient client; // 声明一个Mysql客户端，在连接Mysql中使用
MySQL_Connection conn(&client);
MySQL_Cursor *cursor; //

int isConnection = 0;
int cID = 20221012;
// 读取传感器的数据并写入到数据库
void WriteData()
{
    char buff[128]; // 定义存储传感器数据的数组
    sprintf(buff, INSERT_SQL, cID);             // 将SQL语句与数据放入buff中
    MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn); // 创建一个Mysql实例
    cur_mem->execute(buff);                          
    Serial.println("读取传感器数据，并写入数据库");
    delete cur_mem; // 删除mysql实例
    cID ++;//cID 自增
}

void setup()
{
    Serial.begin(9600);
    while (!Serial)
        ; //  等待端口的释放
    Serial.printf("\nConnecting to %s", ssid);
    WiFi.begin(ssid, wifi_password); // 连接WiFi
    while (WiFi.status() != WL_CONNECTED)
    { // 如果WiFi没有连接，一直循环打印点
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nConnected to network");
    Serial.print("My IP address is: ");
    Serial.println(WiFi.localIP()); // 打印开发板的IP地址

    Serial.print("Connecting to SQL...  ");

    IPAddress server_addr(120,79,147,30);                              // 安装Mysql的电脑的IP地址
    if (conn.connect(server_addr, 13306, mysql_username, mysql_password)) // 连接数据库
    {
        // isConnection = 1;
        delay(3000);
        Serial.println("成功连接数据库---OK.");
    }
    else
    {
        // isConnection = 0;
        delay(3000);
        Serial.println("连接数据库失败---FAILED.");
    }
    // cursor = new MySQL_Cursor(&conn); // 创建一个数据库游标实例
}

void loop()
{
    // if (isConnection == 1)
    // {
    //     WriteData();
    //     delay(5000);
    // }
}