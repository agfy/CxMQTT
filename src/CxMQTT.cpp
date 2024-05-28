/*
  CxMQTT.cpp - Library for sending telemetry and receiving commands from arduino esp8266 via mqtt.
  Created by Petrovskiy Maxim 25 April 2024.
  Released into the public domain.
*/

#include "Arduino.h"
#include "CxMQTT.h"

void callback(char* topic, byte* payload, unsigned int length);

void onNewCommand(byte* payload, unsigned int length) {
}

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Received a command [");
    Serial.print(topic);
    Serial.print("] : ");

    payload[length] = 0;  // ensure valid content is zero terminated so can treat as c-string
    Serial.println((char*)payload);
    onNewCommand(payload, length);
}

CxMQTT::CxMQTT(String wifi, String password, String brokerHost, int brokerPort, const char *caCert, const char *clientCert, const char *clientKey, String clientId, String telemetryTopic, String commandTopic) :            
               timeClient(ntpUDP, "pool.ntp.org"),
               mqtt(wifiClient)
{
  _wifi = wifi;
  _password = password;
  _caCert = caCert;
  _brokerHost = brokerHost;
  mqtt.setServer(_brokerHost.c_str(), brokerPort);
  _clientCert = clientCert;
  _clientKey = clientKey;
  _clientId = clientId;
  _telemetryTopic = telemetryTopic;
  _commandTopic = commandTopic;
}

void CxMQTT::setup()
{
    Serial.begin(9600);
    while (!Serial) {
        ;
    }

    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.print(_wifi);

    connectToWifi();

    Serial.println("Thing connected to the CloudX IoT network");
    Serial.println();

    timeClient.begin();
    Serial.print("Obtaining time...");
    while (!timeClient.update()) {
        Serial.print(".");
        timeClient.forceUpdate();
        delay(1000);
    }
    Serial.println();
    Serial.println("Time was set");

    wifiClient.setX509Time(timeClient.getEpochTime());
    rootCert = new BearSSL::X509List(_caCert);
    wifiClient.setTrustAnchors(rootCert);
    clientKey = new BearSSL::PrivateKey(_clientKey);
    clientCert = new BearSSL::X509List(_clientCert);
    wifiClient.setClientRSACert(clientCert, clientKey);
    wifiClient.setInsecure();
    mqtt.setBufferSize(512);
    mqtt.setKeepAlive(30);
}

void CxMQTT::connectToWifi()
{
    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(_wifi);

    WiFi.mode(WIFI_STA);
    WiFi.begin(_wifi, _password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    randomSeed(micros());

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void CxMQTT::loop()
{
    while (!mqtt.connected()) {
        Serial.println("Connecting to EMQX...");
        if (mqtt.connect(_clientId.c_str())) {
            Serial.println("Connected to EMQX");
            Serial.print("Subscribing to commands topic: ");
            Serial.println(_commandTopic);
            Serial.println();
            mqtt.subscribe(_commandTopic.c_str());
        } else {
            Serial.print("SSL Error: ");
            Serial.print(wifiClient.getLastSSLError(_buf, 50));
            Serial.println(_buf);
            Serial.println("Failed to connect to EMQX, retrying...");
            delay(5000);
            Serial.println(mqtt.state());
        }
    }
}


void CxMQTT::sentTelemetry(String tm)
{
    StaticJsonDocument<512> message;
    char sendBuffer[512];
    message["ID"] = _clientId;
    message["CV"] = "1.0";

    StaticJsonDocument<256> telemetryPayload;
    JsonArray telemetryArray = telemetryPayload.to<JsonArray>();
    JsonObject telemetry = telemetryArray.createNestedObject();
    telemetry["T"] = _clientId;
    telemetry["V"] = tm;
    telemetry["TS"] = timeClient.getEpochTime();
    message["TM"] = telemetryArray;

    size_t n = serializeJson(message, sendBuffer);
    serializeJson(message, Serial);
    mqtt.publish(_telemetryTopic.c_str(), sendBuffer, n);
    mqtt.flush();
    Serial.println();
}