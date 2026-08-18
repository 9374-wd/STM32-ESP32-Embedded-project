#include <Arduino.h>
#include <algorithm>
#include "AITA_DHT.h"
#include "WiFi.h"
#include <time.h>
#include "PubSubClient.h"
#include "Ticker.h"
#include <ESP32Servo.h>
#include "HX711.h"

/********对象声明************************/
AITA_DHT dht11;
Servo feederServo;
HX711 scale;
WiFiClient espClient;
PubSubClient client(espClient);
Ticker dataTicker;

/*********硬件接收数据变量****************/
uint8_t temperature, humidity;
float food_weight = 0;

/*********硬件引脚宏******************/
#define SERVO_PIN 2     // 舵机SG90
#define DOUT_PIN 16     // 称重HX711
#define SCK_PIN 17
#define CONTROL_PIN 13  // 水泵
#define BTN_FEED_PIN 5  // 按键1：控制投喂
#define BTN_WATER_PIN 6 // 按键2：控制注水 

// 按键状态变量
bool lastBtnFeedState = HIGH;
bool lastBtnWaterState = HIGH;
unsigned long lastDebounceTimeFeed = 0;
unsigned long lastDebounceTimeWater = 0; 
const unsigned long debounceDelay = 100;

/*********工作流配置****************************/
#define CALIBRATION_FACTOR 400.5
const int WEIGHT_SAMPLES = 20;
const float WEIGHT_STABILITY_THRESHOLD = 2.0;

const int SERVO_OPEN_ANGLE = 90;
const int SERVO_CLOSE_ANGLE = 0;
const int FEED_TIME_PER_GRAM = 100;
float flowRate = 25.0;

const int Hand_Time_Feed = 2000;
const int Hand_Time_Water = 2000;

// =============== 存储多时间数组（MAX->10个）===============//
#define MAX_TIMES 10
int feedHour[MAX_TIMES] = {21, -1, -1, -1, -1, -1, -1, -1, -1, -1};
int feedMin[MAX_TIMES]  = {20, -1, -1, -1, -1, -1, -1, -1, -1, -1};

int waterHour[MAX_TIMES] = {21, -1, -1, -1, -1, -1, -1, -1, -1, -1};
int waterMin[MAX_TIMES]  = {20, -1, -1, -1, -1, -1, -1, -1, -1, -1};

// 防重复触发（记录上一次执行的分钟数）
int lastFeedMinute  = -1;
int lastWaterMinute = -1;
// 日志全局变量
String lastLogSent = "";
unsigned long lastLogTime = 0;
// ✅ 小程序设置的默认量
int default_feed = 20;
int default_water = 20;

unsigned long lastPrintTime = 0; 
const unsigned long printInterval = 3000;

String serialBuffer = "";

// NTP
const char* ntpServer = "ntp.aliyun.com";
const long  gmtOffset_sec = 28800;
const int daylightOffset_sec = 0;

// WiFi & MQTT
const char *ssid = "100";
const char *password = "12345678wd";
const char *mqtt_server = "mqtts.heclouds.com";

#define mqtt_devid "Cat_Food"
#define mqtt_pubid "63Ge682gyI"
#define mqtt_password "version=2018-10-31&res=products%2F63Ge682gyI%2Fdevices%2FCat_Food&et=1988164800&method=md5&sign=8UQ98LQGCWx6m57dymqGig%3D%3D"

String dpTopic = "$sys/" mqtt_pubid "/" mqtt_devid "/dp/post/json";
String cmdTopic = "$sys/" mqtt_pubid "/" mqtt_devid "/cmd/request/+";

char msg_buf[200];
char dataTemplate[] = "{\"id\":123,\"dp\":{\"temp\":[{\"v\":%d}],\"humi\":[{\"v\":%d}],\"food_weight\":[{\"v\":%.1f}]}}";
char msgJson[150];

// 初始化称重
void setup_scale() {
  scale.begin(DOUT_PIN, SCK_PIN);
  scale.set_scale(CALIBRATION_FACTOR);
  scale.tare();
  Serial.println("Initializing scale...");
  for(int i=0; i<10; i++) {
    scale.get_units();
    delay(100);
  }
  Serial.println("Scale ready");
}

// 稳定重量，数据清洗滤波
float read_stable_weight() {
  float values[WEIGHT_SAMPLES];
  for(int i=0; i<WEIGHT_SAMPLES; i++) {
    values[i] = scale.get_units();
    delay(30);
  }
  std::sort(values, values+WEIGHT_SAMPLES);
  float median = values[WEIGHT_SAMPLES/2];
  float sum = 0;
  int count = 0;
  for(int i=0; i<WEIGHT_SAMPLES; i++) {
    if(abs(values[i]-median) < WEIGHT_STABILITY_THRESHOLD) {
      sum += values[i];
      count++;
    }
  }
  float result= count > 0 ? sum/count : median;
  result-=0.33;
  if(result<0) result=0;
  return result;
}

/************************************************
**函数名称：✅投食执行函数
**参数：克数
************************************************/
void feed(int grams) {
  if(grams <= 0 || grams > 100) {
    Serial.println("Invalid feed amount");
    return;
  }
  Serial.print("🍖Dispensing ");
  Serial.print(grams);
  Serial.println("g food");
  Feed(grams * FEED_TIME_PER_GRAM);
  delay(2000);
  food_weight = read_stable_weight();
  Serial.print("🍖Remaining food: ");
  Serial.print(food_weight);
  Serial.println("g");
  sendLog("feed");
}

/************************************************
**函数名称：✅注水执行函数
**参数：毫升数
************************************************/
void controlWater(int targetMl) {
  if(targetMl<10||targetMl>1000){
    Serial.println("Invalid amount 10~1000ml");
    return ;
  }
  float totalSeconds = targetMl / flowRate;
  int totalMillis = (int)(totalSeconds * 1000);
  Serial.println("💧Water pump turned ON.");
  Water(totalMillis);
  Serial.println("💧Water pump turned OFF.");
  sendLog("water");
}

void Feed(int Time1){
  feederServo.write(SERVO_OPEN_ANGLE);
  delay(Time1);
  feederServo.write(SERVO_CLOSE_ANGLE);
}

void Water(int Time2){
  digitalWrite(CONTROL_PIN, LOW); 
  delay(Time2); 
  digitalWrite(CONTROL_PIN, HIGH);
}
/************************************************
**函数名称：✅日志上传函数
**参数：日志类型"water"/"feed"
************************************************/
void sendLog(const char* logType) {
  static unsigned long logSeq = 0; // 自增序号，保证每次唯一
  logSeq++;

  if (client.connected()) {
    // 每次都上传：类型 + 序号，保证云端认为是新日志
    char logBuf[32];
    snprintf(logBuf, sizeof(logBuf), "%s_%lu", logType, logSeq);

    snprintf(msgJson, sizeof(msgJson), 
      "{\"id\":123,\"dp\":{\"log\":[{\"v\":\"%s\"}]}}", 
      logBuf
    );
    client.publish(dpTopic.c_str(), msgJson);
    Serial.printf("📝 日志已发送: %s\n", logBuf);
  }
}

/************************************************
**函数名称：✅ MQTT 回调函数（定时时间自动插入数组）
**参数：订阅主题，命令内容，命令长度
************************************************/
void callback(char *topic, byte *payload, unsigned int length) {
  String topicStr = topic;
  String payloadStr;
  
  for(size_t i=0; i<length; i++) {
    payloadStr += (char)payload[i];
  }
  
  Serial.println("Message received:");
  Serial.println(payloadStr);

  String uuid = topicStr.substring(topicStr.lastIndexOf('/'));
  String respTopic = "$sys/" mqtt_pubid "/" mqtt_devid "/cmd/response" + uuid;

  // 立即喂食
  if(payloadStr.indexOf("\"feed\"") >= 0) {
    int grams = payloadStr.substring(payloadStr.indexOf(":")+1).toInt();
    feed(grams);
    snprintf(msgJson, sizeof(msgJson), "{\"code\":200,\"msg\":\"Fed %dg\"}", grams);
    client.publish(respTopic.c_str(), msgJson);
  }

  // 立即加水
  else if(payloadStr.indexOf("\"water\"")>=0){
    int targetMl=payloadStr.substring(payloadStr.indexOf(":")+1).toInt();
    controlWater(targetMl);
    snprintf(msgJson, sizeof(msgJson), "{\"code\":200,\"msg\":\"Watered %dml\"}",targetMl);
    client.publish(respTopic.c_str(), msgJson);
  }

  // ✅ 设置默认喂食量
  else if (payloadStr.indexOf("\"set_default_feed\"") >= 0) {
    int val = payloadStr.substring(payloadStr.indexOf(":")+1).toInt();
    if(val>0&&val<=100){
      default_feed=val;
      snprintf(msgJson, sizeof(msgJson), "{\"code\":200,\"msg\":\"默认喂食量:%dg\"}",val);
    }else{
      snprintf(msgJson, sizeof(msgJson), "{\"code\":400,\"msg\":\"error\"}");
    }
    client.publish(respTopic.c_str(), msgJson);
  }

  // ✅ 设置默认加水量
  else if (payloadStr.indexOf("\"set_default_water\"") >= 0) {
    int val = payloadStr.substring(payloadStr.indexOf(":")+1).toInt();
    if(val>0&&val<=1000){
      default_water=val;
      snprintf(msgJson, sizeof(msgJson), "{\"code\":200,\"msg\":\"默认加水量:%dml\"}",val);
    }else{
      snprintf(msgJson, sizeof(msgJson), "{\"code\":400,\"msg\":\"error\"}");
    }
    client.publish(respTopic.c_str(), msgJson);
  }

  // ✅ 设置投喂时间 → 自动插入数组（支持多次）
  else if(payloadStr.indexOf("\"set_feed_time\"")>=0){
    int s=payloadStr.indexOf(":")+2;
    int e=payloadStr.indexOf("\"",s);
    String t=payloadStr.substring(s,e);
    int h=t.substring(0,2).toInt();
    int m=t.substring(3,5).toInt();
    
    if(h>=0&&h<24&&m>=0&&m<60){
      // 找空位插入
      for(int i=0;i<MAX_TIMES;i++){
        if(feedHour[i]==-1){
          feedHour[i]=h;
          feedMin[i]=m;
          break;
        }
      }
      snprintf(msgJson,sizeof(msgJson),"{\"code\":200,\"msg\":\"Feed time added %02d:%02d\"}",h,m);
    }else{
      snprintf(msgJson,sizeof(msgJson),"{\"code\":400,\"msg\":\"time err\"}");
    }
    client.publish(respTopic.c_str(),msgJson);
  }

  // ✅ 设置加水时间 → 自动插入数组（支持多次）
  else if(payloadStr.indexOf("\"set_water_time\"")>=0){
    int s=payloadStr.indexOf(":")+2;
    int e=payloadStr.indexOf("\"",s);
    String t=payloadStr.substring(s,e);
    int h=t.substring(0,2).toInt();
    int m=t.substring(3,5).toInt();
    
    if(h>=0&&h<24&&m>=0&&m<60){
      for(int i=0;i<MAX_TIMES;i++){
        if(waterHour[i]==-1){
          waterHour[i]=h;
          waterMin[i]=m;
          break;
        }
      }
      snprintf(msgJson,sizeof(msgJson),"{\"code\":200,\"msg\":\"Water time added %02d:%02d\"}",h,m);
    }else{
      snprintf(msgJson,sizeof(msgJson),"{\"code\":400,\"msg\":\"time err\"}");
    }
    client.publish(respTopic.c_str(),msgJson);
  }
}
/************************************************
**函数名称：✅ MQTT数据流发布函数
**数据流内容：tempature,humi,food_weight
************************************************/
void sendSensorData() {
  if(client.connected()) {
    dht11.dht11Read(&temperature, &humidity);
    food_weight = read_stable_weight();
       Serial.print("温度: ");
    Serial.print(temperature);
    Serial.print(" ℃  |  湿度: ");
    Serial.print(humidity);
    Serial.print(" %  |  剩余食量: ");
    Serial.print(food_weight);
    Serial.println(" g");
    snprintf(msgJson, sizeof(msgJson), dataTemplate, temperature, humidity, food_weight);
    client.publish(dpTopic.c_str(), msgJson);
  }
}
/************************************************
**函数名称：✅ WIFI连接函数
************************************************/
void setupWifi() {
  Serial.println("Connecting WiFi...");
  WiFi.begin(ssid, password);
  while(!WiFi.isConnected()) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
}
/************************************************
**函数名称：✅ MQTT 重连函数
************************************************/
void reconnect() {
  while(!client.connected()) {
    if(client.connect(mqtt_devid, mqtt_pubid, mqtt_password)) {
      client.subscribe(cmdTopic.c_str());
      Serial.println("MQTT connected");
    } else {
      Serial.print("failed, rc=");
      Serial.println(client.state());
      delay(5000);
    }
  }
}
/************************************************
**函数名称：✅ 多时间定时检查
************************************************/
void checkScheduledTasks(){
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return;

  int h = timeinfo.tm_hour;
  int m = timeinfo.tm_min;
  int totalMin = h * 60 + m; // 当天总分钟数，用于防重复

  // 检查所有投喂时间
  for(int i=0;i<MAX_TIMES;i++){
    if(feedHour[i]==-1) continue;
    if(h == feedHour[i] && m == feedMin[i] && totalMin != lastFeedMinute){
      Serial.println("⏰ 定时投喂");
      feed(default_feed);
      lastFeedMinute = totalMin;
      break;
    }
  }

  // 检查所有加水时间
  for(int i=0;i<MAX_TIMES;i++){
    if(waterHour[i]==-1) continue;
    if(h == waterHour[i] && m == waterMin[i] && totalMin != lastWaterMinute){
      Serial.println("⏰ 定时加水");
      controlWater(default_water);
      lastWaterMinute = totalMin;
      break;
    }
  }
}

void Tim_print() {
  unsigned long now = millis();
  if (now - lastPrintTime >= printInterval) {
    lastPrintTime = now;
    struct tm timeinfo;
    if(getLocalTime(&timeinfo)){
      char buf[50];
      strftime(buf,sizeof(buf),"%Y-%m-%d %H:%M:%S",&timeinfo);
    }
  }
}
/************************************************
**函数名称：✅ 按键监测函数
************************************************/
void Key_Scan(){
  unsigned long now = millis();
  int f = digitalRead(BTN_FEED_PIN);
  if(f != lastBtnFeedState){
    lastDebounceTimeFeed = now;
  }
  lastBtnFeedState = f;
  if(now - lastDebounceTimeFeed > debounceDelay){
    static bool ok = false;
    if(f == LOW && !ok){
      Serial.println("按键投喂");
      feed(default_feed);
      ok = true;
    }else if(f == HIGH){
      ok = false;
    }
  }

  int w = digitalRead(BTN_WATER_PIN);
  if(w != lastBtnWaterState){
    lastDebounceTimeWater = now;
  }
  lastBtnWaterState = w;
  if(now - lastDebounceTimeWater > debounceDelay){
    static bool ok = false;
    if(w == LOW && !ok){
      Serial.println("按键加水");
      controlWater(default_water);
      ok = true;
    }else if(w == HIGH){
      ok = false;
    }
  }
}

void handleSerialCommand() {
  if(Serial.available()){
    char c = Serial.read();
    if(c=='\n'||c=='\r'){
      if(serialBuffer.length()>0){
        serialBuffer="";
      }
    }else{
      serialBuffer += c;
    }
  }
}

void processCommand(String cmd){}

void setup() {
  Serial.begin(115200);
  feederServo.attach(SERVO_PIN);
  feederServo.write(SERVO_CLOSE_ANGLE);
  pinMode(CONTROL_PIN,OUTPUT);
  digitalWrite(CONTROL_PIN,HIGH);
  setup_scale();
  setupWifi();
  configTime(gmtOffset_sec,daylightOffset_sec,ntpServer);
  pinMode(BTN_FEED_PIN,INPUT_PULLUP);
  pinMode(BTN_WATER_PIN,INPUT_PULLUP);
  client.setServer(mqtt_server,1883);
  client.setCallback(callback);
  dataTicker.attach(10,sendSensorData);
}

void loop() {
  if(!WiFi.isConnected()) setupWifi();
  if(!client.connected()) reconnect();
  client.loop();
  handleSerialCommand();
  Key_Scan();
  checkScheduledTasks();
  Tim_print();
  delay(10);
}