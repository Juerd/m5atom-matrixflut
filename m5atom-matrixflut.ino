#include <FastLED.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <WiFiSettings.h>
#include <MQTT.h>
#include <ArduinoOTA.h>

String mqtt_topic;

const int    ledpin  = 27;
const int    numleds = 25;
const int    buttonpin = 39;

CRGB         leds[numleds];
WiFiClient   wificlient;
MQTTClient   mqtt;

void setup_ota() {
    ArduinoOTA.setHostname(WiFiSettings.hostname.c_str());
    ArduinoOTA.setPassword(WiFiSettings.password.c_str());
    ArduinoOTA.begin();
}

void setup() {

    FastLED.addLeds<WS2812B, ledpin, GRB>(leds, numleds);
    FastLED.setBrightness(20);

    Serial.begin(115200);
    SPIFFS.begin(true);
    pinMode(buttonpin, INPUT);

    WiFiSettings.onWaitLoop = []() {
        static CHSV color(0, 255, 255);
        color.hue += 10;
        FastLED.showColor(color);
        if (! digitalRead(buttonpin)) WiFiSettings.portal();
        return 50;
    };
    WiFiSettings.onSuccess = []() {
        FastLED.showColor(CRGB::Green);
        setup_ota();
        delay(200);
    };
    WiFiSettings.onPortal = []() {
        setup_ota();
    };
    WiFiSettings.onPortalWaitLoop = []() {
        static CHSV color(0, 255, 255);
        color.saturation--;
        FastLED.showColor(color);
        ArduinoOTA.handle();
    };

    String server = WiFiSettings.string("mqtt_server", 64, "test.mosquitto.org");
    int    port   = WiFiSettings.integer("mqtt_port", 0, 65535, 1883);
    mqtt_topic    = WiFiSettings.string("matrixflut_topic", "matrixflut");

    WiFiSettings.connect();

    mqtt.begin(server.c_str(), port, wificlient);
    mqtt.onMessageAdvanced(mqtt_callback);
}

void mqtt_callback(MQTTClient* client, char* topic, char* message, int length) {
    Serial.println(topic);
    Serial.println(length);
    if (length != numleds * 3) return;

    for (int i = 0; i < numleds; i++) {
        leds[i].r = *message++;
        leds[i].g = *message++;
        leds[i].b = *message++;
    }
    FastLED.show();
}

void loop() {
    while (!mqtt.connected()) {
        FastLED.showColor(CRGB::Blue);
        if (mqtt.connect("")) {
            Serial.println("MQTT connected!");
            FastLED.showColor(CRGB::Green);

            mqtt.subscribe(mqtt_topic);
        } else {
            Serial.println("MQTT connection failed...");
            FastLED.showColor(CRGB::Red);
            delay(1000);
        }
    }

    mqtt.loop();
    ArduinoOTA.handle();
}
