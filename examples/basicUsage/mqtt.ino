#include <CxMQTT.h>
#include "tls_data.h"

CxMQTT mqttClient("wifi_name", "wifi_password", "broker_url", 8883, ca_cert, thing_cert, thing_key, "thing_name", "telemetry_topic_name", "command_topic_name");

void setup() {
    mqttClient.setup();
}

void loop() {
    mqttClient.loop();
    mqttClient.sentTelemetry("my data");
}