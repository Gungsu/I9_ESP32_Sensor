#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <Update.h>
#include "namedMesh.h"
#include <Wire.h>
#include "SparkFun_VL53L1X.h"

#define SENS_NAME "S04"
#define MESH_SSID       "rede_mesh_i9"
#define MESH_PASSWORD   "deumaoito"
#define MESH_PORT       5555
#define SDA 8
#define SCL 9
#define SPEED 400000
#define LED 3

SFEVL53L1X distanceSensor;
Scheduler userScheduler;
namedMesh mesh;
bool led_alerta, led_toggle, timeSeted;
int distanceMM = 0;
uint32_t timeOld;

String nodeName = SENS_NAME; // Name needs to be unique

Task taskSendMessage( TASK_SECOND*0.5, TASK_FOREVER, []() {
    String msg = String(distanceMM);
    String to = "central_i9";
    mesh.sendSingle(to, msg);
    // Serial.print(to);
    // Serial.print(" ");
    // Serial.println(msg);
});

bool timeSet(uint32_t timeOld, uint32_t timeTotal)
{
  if(millis()-timeOld > timeTotal){
    return true;
  } else {
    return false;
  }
}

void light_alerta() {
  if(led_alerta) {
    if(!timeSeted) {
      timeOld = millis();
      timeSeted = true;
    }
    if (timeSet(timeOld,600)) {
      led_toggle = !led_toggle;
      digitalWrite(LED, led_toggle);
      timeSeted = false;
    }
  } else {
    digitalWrite(LED, 0);
  }
}

void setup() {
  // put your setup code here, to run once:
  //Serial.begin(115200);
  pinMode(LED, OUTPUT);

  Wire.setPins(SDA,SCL);
  Wire.setClock(400000);
  Wire.begin();

  if (distanceSensor.begin(Wire) != 0) //Begin returns 0 on a good init
  {
    //Serial.println("Sensor failed to begin. Please check wiring. Freezing...");
    while (1);
  }
  //Serial.println("Sensor online!");
   mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION);  // set before init() so that you can see startup messages

  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);

  mesh.setName(nodeName); // This needs to be an unique name! 

  mesh.onReceive([](String &from, String &msg) {
    if (msg == "ON_Alerta") {
      led_alerta = true;
    } else {
      led_alerta =  false;
    }
  });

  mesh.onChangedConnections([]() {
    //Serial.printf("Changed connection\n");
  });

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
}

void loop() {
  // put your main code here, to run repeatedly:
  mesh.update();
  distanceSensor.startRanging();
  while (!distanceSensor.checkForDataReady())
  {
    delay(1);
  }
  distanceMM = distanceSensor.getDistance();
  distanceSensor.clearInterrupt();
  distanceSensor.stopRanging();
  //light_alerta();
}