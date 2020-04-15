#include <FastLED.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <WiFiConfig.h>
#include <PubSubClient.h>

const char*  mqtt_server = "test.mosquitto.org";
const char*  mqtt_topic  = "matrixflut";

const int    ledpin  = 27;
const int    numleds = 25;
const int    buttonpin = 39;

CRGB         leds[numleds];
WiFiClient   espClient;
PubSubClient mqtt(espClient);

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

    WiFiConfig.connect();


    mqtt.setServer(mqtt_server, 1883);
    mqtt.setCallback(mqtt_callback);
}

void mqtt_callback(char* topic, byte* message, unsigned int length) {
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
        Serial.printf("Connecting to %s...", mqtt_server);
        if (mqtt.connect("")) {
            Serial.println(" connected!");
            FastLED.showColor(CRGB::Green);

            mqtt.subscribe(mqtt_topic);
        } else {
            Serial.println(" failed...");
            FastLED.showColor(CRGB::Red);
            delay(1000);
        }
    }

    mqtt.loop();
}
