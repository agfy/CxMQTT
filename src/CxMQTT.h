/*
  CxMQTT.h - Library for sending telemetry and receiving commands from arduino esp8266 via mqtt.
  Created by Petrovskiy Maxim 25 April 2024.
  Released into the public domain.
*/
#ifndef CxMQTT_h
#define CxMQTT_h

#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

class CxMQTT
{
  public:
    CxMQTT(String wifi, String password, String brokerHost, int brokerPort, const char *caCert, const char *clientCert, const char *clientKey, String tenantId, String contexts, String clientId);
    void setup();
    void connectToWifi();
    void loop();
    void sendTelemetry(String telemetry);

  private:
    String _wifi, _password, _clientId, _telemetryTopic, _commandTopic, _brokerHost;
    const char *_caCert, *_clientCert, *_clientKey;

    NTPClient timeClient;
    BearSSL::WiFiClientSecure wifiClient;
    BearSSL::X509List *rootCert, *clientCert;
    BearSSL::PrivateKey* clientKey;
    PubSubClient mqtt;
    WiFiUDP ntpUDP;
    char _buf[50];
};

#endif

