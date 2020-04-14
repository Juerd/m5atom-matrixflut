#include <FastLED.h>
#include <WiFi.h>
#include <PubSubClient.h>

const char*  mqtt_server = "test.mosquitto.org";
const char*  mqtt_topic  = "matrixflut";
const char*  wifi_ssid   = "YOUR_WIFI_HERE";
const char*  wifi_psk    = "YOUR_WIFI_HERE";

const int    ledpin  = 27;
const int    numleds = 25;

CRGB         leds[numleds];
WiFiClient   espClient;
PubSubClient mqtt(espClient);

void setup() {

    FastLED.addLeds<WS2812B, ledpin, GRB>(leds, numleds);
    FastLED.setBrightness(20);
    WiFi.begin(wifi_ssid, wifi_psk);

    Serial.begin(115200);
    Serial.print("Connecting to wifi...");
    CHSV color(0, 255, 255);
    while (WiFi.status() != WL_CONNECTED) {
        delay(50);
        color.hue++;
        FastLED.showColor(color);
    }
    Serial.println(" connected!");
    FastLED.showColor(CRGB::Green);

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
