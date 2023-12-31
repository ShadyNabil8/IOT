#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif
#include <ArduinoJson.h>
#include <PubSubClient.h>
#ifndef ESP8266
#include <BluetoothSerial.h>
#endif
#ifndef ESP8266
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
#endif
#include <WiFiClientSecure.h>
#include <UrlEncode.h>
#include <JC_Button.h>

#define TOPIC_SUB "T1"
#define TOPIC_PUP "T2"
#define BUZZER_BTN 19
#define BUZZER_PIN LED_BUILTIN
#define MSG_BUFFER_SIZE (50)

const char *ssid = "WE_E709BC";
const char *password = "jbq0606222";

const char *mqtt_server = "beb527c39d66419baeaf1c6df479bd92.s2.eu.hivemq.cloud";
const char *mqtt_username = "MQTTapp";
const char *mqtt_password = "MQTTapp850";
const int mqtt_port = 8883;

bool buzzer_prev_status = false;
bool buzzer_btn_on = false;
unsigned long lastMsg = 0;
char msg[MSG_BUFFER_SIZE];
bool timeOut = false;

WiFiClientSecure espClient;
PubSubClient client(espClient);
#ifndef ESP8266
BluetoothSerial SerialBT;
#endif
Button buzzer_btn(BUZZER_BTN);
#ifndef ESP8266
hw_timer_t *My_timer = NULL;
#endif

#ifndef ESP8266
static const char *root_ca PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";
#endif

void setup_wifi()
{
  delay(10);
  Serial.print("\nConnecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("\nWiFi connected\nIP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password))
    {
      Serial.println("connected");
      client.subscribe(TOPIC_SUB);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void Buzzer_toggle()
{
  if (buzzer_prev_status == false)
  {
    digitalWrite(BUZZER_PIN, HIGH);
    buzzer_prev_status = true;
  }
  else
  {
    digitalWrite(BUZZER_PIN, LOW);
    buzzer_prev_status = false;
  }
}

void callback(char *topic, byte *payload, unsigned int length)
{
  String incommingMessage = "";
  for (unsigned int i = 0; i < length; i++)
    incommingMessage += (char)payload[i];
  Serial.println("Message arrived [" + String(topic) + "]" + incommingMessage);

  StaticJsonDocument<200> msg;
  deserializeJson(msg, incommingMessage);

  if (strcmp(topic, TOPIC_SUB) == 0)
  {
    if (String((const char *)msg["event"]).equals("gps"))
    {
      Serial.print((double)msg["lat"], 8);
      Serial.println();
      Serial.print((double)msg["lng"], 8);
      Serial.println();
      double lat = msg["lat"];
      double lng = msg["lng"];

      String latStr = String(lat, 6);
      String lngStr = String(lng, 6);
      // sendMessageBT("https://www.google.com/maps/search/?api=1&query=" + latStr + "," + lngStr);
    }
    else
    {
      Buzzer_toggle();
    }
  }
}

void publishMessage(const char *topic, String payload, boolean retained)
{
  if (client.publish(topic, payload.c_str(), retained))
    Serial.println("Message publised [" + String(topic) + "]: " + payload);
}

void sendMessageBT(String message)
{
#ifndef ESP8266
  SerialBT.println(message);
#endif
}

void IRAM_ATTR onTimer()
{
  timeOut = true;
}

void setup()
{
  buzzer_btn.begin();
  pinMode(BUZZER_PIN, OUTPUT);
  Serial.begin(115200);
  while (!Serial)
    delay(1);
  setup_wifi();
#ifdef ESP8266
  espClient.setInsecure();
#else
  espClient.setCACert(root_ca); // enable this line and the the "certificate" code for secure connection
#endif
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

#ifndef ESP8266
  My_timer = timerBegin(0, 80, true);
  timerAttachInterrupt(My_timer, &onTimer, true);
  timerAlarmWrite(My_timer, 1000000, true);
  timerAlarmEnable(My_timer);
#endif

#ifndef ESP8266
  SerialBT.begin("ESP32");
#endif
}

void loop()
{
  buzzer_btn.read();
  if (!client.connected())
    reconnect();
  if (buzzer_btn.pressedFor(3000))
  {
    DynamicJsonDocument msg(1024);
    msg["event"] = "buzzer";
    char mqtt_message[128];
    serializeJson(msg, mqtt_message);
    publishMessage(TOPIC_PUP, mqtt_message, false);
    while (!buzzer_btn.wasReleased())
    {
      buzzer_btn.read();
      // esp_task_wdt_reset();
    }
  }
#ifndef ESP8266
  if (timeOut)
  {
    client.loop();
    timeOut = false;
  }
#else
  client.loop();
#endif
}

