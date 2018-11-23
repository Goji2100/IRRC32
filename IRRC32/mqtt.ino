#ifdef USE_MQTT

#include <PubSubClient.h>
WiFiClient mqttClient;
PubSubClient PSClient(mqttServer, 1883, mqttClient);

void callback(char* mqttTopic, byte* payload, unsigned int length) {
  String mqttCommand = (char *)payload;
  mqttCommand = mqttCommand.substring(0, length);
  Serial.println("MQTT = [" + String(mqttTopic) + "]" + mqttCommand);
  int p = mqttCommand.indexOf("\"data\":\"");
  if (p != -1) {
    mqttCommand = mqttCommand.substring(p + 8, mqttCommand.indexOf("\"", p + 8));
    Serial.println("CMND = " + mqttCommand);
    Serial.printf("EXEC = %d\n", sr_handleMQTT(mqttCommand));
  }
}

void reconnect() {
  while (!PSClient.connected()) {
    Serial.printf("Attempting MQTT connection [%s] .. ", host);
    if (PSClient.connect(host, mqttUser, NULL)) {
      Serial.println("connected.");
      PSClient.setCallback(callback);
      PSClient.subscribe(mqttTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(PSClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void mqtt_loop() {
  if (!PSClient.connected()) reconnect();
  PSClient.loop();
}
#endif
