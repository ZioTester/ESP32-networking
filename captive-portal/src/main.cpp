#include <Arduino.h>
#include <AsyncTCP.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <WiFi.h>

#include "ESPAsyncWebServer.h"

const byte DNS_PORT = 53;
IPAddress apIP(8, 8, 8, 8);
IPAddress subnet(255, 255, 255, 0);

DNSServer dnsServer;
AsyncWebServer server(80);

class CaptiveRequestHandler : public AsyncWebHandler {
 public:
  CaptiveRequestHandler() {
    server.on("/app", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(200, "text/plain", "Hello, world");
    });
  }
  virtual ~CaptiveRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request) {
    // request->addInterestingHeader("ANY");
    return true;
  }

  void handleRequest(AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->print("<!DOCTYPE html><html><head><title>Captive Portal</title></head><body>");
    response->print("<p>This is out captive portal front page.</p>");
    response->printf("<p>You were trying to reach: http://%s%s</p>", request->host().c_str(), request->url().c_str());
    response->printf("<p>Try opening <a href='http://%s/app'>this link</a> instead</p>", WiFi.softAPIP().toString().c_str());
    response->print("</body></html>");
    request->send(response);
  }
};

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP);
  WiFi.softAP("ZioTesterLab");
  WiFi.softAPConfig(apIP, apIP, subnet);
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

  if (!MDNS.begin("ziotester")) {
    Serial.println("Error starting mDNS");
    return;
  }

  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);
  server.begin();
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
}

void loop() { dnsServer.processNextRequest(); }