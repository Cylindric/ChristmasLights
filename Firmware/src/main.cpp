#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include "christmas.h"

#ifndef WIFI_SSID
  #define WIFI_SSID ""
#endif
#ifndef WIFI_PASSWORD
  #define WIFI_PASSWORD ""
#endif
#ifndef MQTT_SERVER
  #define MQTT_SERVER ""
#endif
#ifndef MQTT_PORT
  #define MQTT_PORT 1883
#endif
#ifndef MQTT_USER
  #define MQTT_USER ""
#endif
#ifndef MQTT_PASSWORD
  #define MQTT_PASSWORD ""
#endif

const char* host = "172.29.14.100";
const char* version = "0.0.2";
const char* mqtt_name ="christmas01";
const char* mqtt_server = MQTT_SERVER;
const char* mqtt_user = MQTT_USER;
const char* mqtt_pass = MQTT_PASS;
const char* mqtt_port = MQTT_PORT;

const char* will_topic = "tele/chocolate01/LWT";
const char* power_command_topic = "christmas01/cmnd/POWER";
const char* info1_topic = "christmas01/tele/INFO1";
const char* state_topic = "christmas01/tele/STATE";
const char* lwt_topic = "tele/christmas01/LWT";
const char* hass_state_topic = "christmas01/tele/HASS_STATE";
const char* hass_switch_topic = "homeassistant/switch/XMAS_1/config";

WebServer server(80);

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

Christmas christmas; 

const char* homeAssistant =
  "{"
    "\"name\":\"Christmas01\","
    "\"cmd_t\":\"~cmnd/POWER\","
    "\"stat_t\":\"~tele/STATE\","
    "\"val_tpl\":\"{{value_json.POWER}}\","
    "\"pl_off\":\"OFF\","
    "\"pl_on\":\"ON\","
    "\"avty_t\":\"~tele/LWT\","
    "\"pl_avail\":\"Online\","
    "\"pl_not_avail\":\"Offline\","
    "\"uniq_id\":\"XMAS_1\","
    "\"device\":{\"identifiers\":[\"XMAS_1\"]},"
    "\"~\":\"christmas01/\""
  "}";

/*
 * Login page
 */
const char* loginIndex = 
 "<form name='loginForm'>"
    "<table width='20%' bgcolor='A09F9F' align='center'>"
        "<tr>"
            "<td colspan=2>"
                "<center><font size=4><b>ESP32 Login Page</b></font></center>"
                "<br>"
            "</td>"
            "<br>"
            "<br>"
        "</tr>"
        "<td>Username:</td>"
        "<td><input type='text' size=25 name='userid'><br></td>"
        "</tr>"
        "<br>"
        "<br>"
        "<tr>"
            "<td>Password:</td>"
            "<td><input type='Password' size=25 name='pwd'><br></td>"
            "<br>"
            "<br>"
        "</tr>"
        "<tr>"
            "<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
        "</tr>"
    "</table>"
"</form>"
"<script>"
    "function check(form)"
    "{"
    "if(form.userid.value=='admin' && form.pwd.value=='admin')"
    "{"
    "window.open('/serverIndex')"
    "}"
    "else"
    "{"
    " alert('Error Password or Username')/*displays error message*/"
    "}"
    "}"
"</script>";
 
/*
 * Server Index Page
 */
 
const char* serverIndex = 
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
   "<input type='file' name='update'>"
        "<input type='submit' value='Update'>"
    "</form>"
 "<div id='prg'>progress: 0%</div>"
 "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var form = $('#upload_form')[0];"
  "var data = new FormData(form);"
  " $.ajax({"
  "url: '/update',"
  "type: 'POST',"
  "data: data,"
  "contentType: false,"
  "processData:false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var per = evt.loaded / evt.total;"
  "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success:function(d, s) {"
  "console.log('success!')" 
 "},"
 "error: function (a, b, c) {"
 "}"
 "});"
 "});"
 "</script>";


void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if (String(topic) == power_command_topic) {
    Serial.print("Changing state to ");
    if(messageTemp == "ON") {
      Serial.println("on");
      christmas.start();
    }
    else if (messageTemp == "OFF") {
      Serial.println("off");
      christmas.stop();
    }
  }
}

void setupMqtt() {
  client.setBufferSize(500);
  client.setServer(mqtt_server, atoi(mqtt_port));
  client.setCallback(callback);
}

bool publishMessage(const char* topic, const char* message) {
  boolean rc = client.publish(topic, message);
  if(rc) {
    Serial.println(message);
  } else {
    Serial.print("mqtt failed, rc=");
    Serial.print(client.state());
  }
  return rc;
}

void publishInitial() {
    publishMessage(info1_topic, "{\"Module\":\"CylChristmas\",\"Version\":\"0.0.1\",\"GroupTopic\":\"christmas\"}");
}

void publishStatus() {
  if(christmas.state) {
    publishMessage(state_topic, "{\"POWER\":\"ON\"}");
  } else {
    publishMessage(state_topic, "{\"POWER\":\"OFF\"}");
  }
  publishMessage(lwt_topic, "Online");
}

void publishHomeassistant() {
  publishMessage(hass_state_topic, "{\"Module\":\"CylChristmas\",\"Version\":\"0.0.1\"}");
  publishMessage(hass_switch_topic, homeAssistant);
}

void reconnect() {
  if (!client.connected()) {
    Serial.print("Attempting MQTT connection to ");
    Serial.print(mqtt_server);
    Serial.println("...");
    // Attempt to connect
    if (client.connect("ChristmasLights", "mqtt", "chocolatemqtt", will_topic, 0, false, "Offline")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe(power_command_topic);
      publishInitial();
      publishStatus();
      publishHomeassistant();
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      delay(1000);
    }
  }
}

void setupWifi() {
  if (WiFi.status() != WL_CONNECTED) {

    Serial.println("Connect to WiFi network...");
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.println("");

    // Wait for connection
    uint32_t timeout = 5000;
    while (timeout > 0 && WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      timeout -= 500;
    }
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("ERROR connecting to WiFi.");
      Serial.println(WiFi.status());
      return;
    }

    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(WIFI_SSID);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    /*use mdns for host name resolution*/
    if (!MDNS.begin(host)) { //http://esp32.local
      Serial.println("Error setting up MDNS responder!");
      while (1) {
        delay(1000);
      }
    }
    Serial.println("mDNS responder started");
  }
}

void setupWebServer() {
  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
  });
  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  server.begin();
}

void loopWebServer() {
  server.handleClient();
}

void loopMqtt() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  EVERY_N_SECONDS( 60 ) { 
    publishStatus();
    publishHomeassistant();
  }
}

void setupOTA() {
  ArduinoOTA.setHostname("christmas.home.cylindric.net");
  ArduinoOTA.begin(); 
}

void loopOTA() {
  ArduinoOTA.handle();
}

/*
 * setup function
 */
void setup(void) {
  Serial.begin(115200);
  Serial.print("Version: ");
  Serial.println(version);

  setupWifi();
  setupWebServer();
  setupOTA();
  setupMqtt();

  christmas.setupChristmas();
  // christmas.setupCylon();
}

void loop(void) {

  if (WiFi.status() != WL_CONNECTED) {
    setupWifi();
  }

  loopOTA();
  loopWebServer();
  loopMqtt();

  christmas.loopChristmas();
  //christmas.loopCylon();
  delay(1);
}
