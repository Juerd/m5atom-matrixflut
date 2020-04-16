#include <FastLED.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <WiFiConfig.h>
#include <MQTT.h>

String mqtt_topic;

const int    ledpin  = 27;
const int    numleds = 25;
const int    buttonpin = 39;

CRGB         leds[numleds];
WiFiClient   wificlient;
MQTTClient   mqtt;

void setup() {

    FastLED.addLeds<WS2812B, ledpin, GRB>(leds, numleds);
    FastLED.setBrightness(20);

    Serial.begin(115200);
    SPIFFS.begin(true);
    pinMode(buttonpin, INPUT);

    WiFiConfig.onWaitLoop = []() {
        static CHSV color(0, 255, 255);
        color.hue += 10;
        FastLED.showColor(color);
        if (! digitalRead(buttonpin)) WiFiConfig.portal();
        return 50;
    };
    WiFiConfig.onSuccess = []() {
        FastLED.showColor(CRGB::Green);
        delay(200);
    };
    WiFiConfig.onPortalWaitLoop = []() {
        static CHSV color(0, 255, 255);
        color.saturation--;
        FastLED.showColor(color);
    };

    String server = WiFiConfig.string("mqtt_server", 64, "test.mosquitto.org");
    int    port   = WiFiConfig.integer("mqtt_port", 0, 65535, 1883);
    mqtt_topic    = WiFiConfig.string("matrixflut_topic", "matrixflut");

    WiFiConfig.connect();

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
}
