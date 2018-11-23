// IRServer 3.30
// -------------
#ifndef IRdefs_h
#define IRdefs_h

//#define IPFIX
//#define USE_MDNS
//#define USE_MQTT
#define CONS_CMND
#define DBG_OUTPUT_PORT Serial

#define PIN_ir_in    12   // IR Receiver
#define PIN_ir_VCC   13   // for IR receiver 3.3V
#define PIN_ir_out   14   // IR Send 
#define PIN_ir_LED   16   // Status LED

const char* ssid = "wifi-ssid";
const char* password = "wifi-password";

#ifdef  IPFIX
IPAddress Netmask(255, 255, 255,   0);
IPAddress LocalIP(192, 168,   0, 141);
IPAddress Gateway(192, 168,   0,   1);
IPAddress DNS    (192, 168,   0,   1);
#endif

char host      [32] = "ESP";
#ifdef  USE_MQTT
char mqttServer[32] = "mqtt.beebotte.com";
char mqttTopic [32] = "ifttt/data";
char mqttUser  [64] = "token:xxxxxxxxxxxxxxxxxxxxxxxxxxxx";
#endif
#endif
