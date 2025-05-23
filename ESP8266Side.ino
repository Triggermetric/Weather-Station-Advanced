#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <FirebaseClient.h>

#include <NTPClient.h>
#include <WifiUdp.h>

// Network and Firebase credentials
#define WIFI_SSID "P 919"
#define WIFI_PASSWORD "123456aA@Aa"

#define Web_API_KEY "AIzaSyDZwUeNkjNGUpvsyz2S3PjqojQG0LozMo8"
#define DATABASE_URL "https://weatherstation-8d3d8-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define USER_EMAIL "lamponhq123@gmail.com"
#define USER_PASS "123456"

// User function
void processData(AsyncResult &aResult);

// Authentication
UserAuth user_auth(Web_API_KEY, USER_EMAIL, USER_PASS);

// Firebase components
FirebaseApp app;
WiFiClientSecure ssl_client;
using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client);
RealtimeDatabase Database;

String values = "0.1";
String sensor_data;

// Timer variables for sending data every 10 seconds
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 30 * 1000ul; // 10 seconds in milliseconds

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

// Variables to send to the Database
int intValue = 0;
float floatValue = 0.01;
String stringValue = "";

void setup(){
  Serial.begin(9600);

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)    {
    Serial.print(".");
    delay(300);
  }
  Serial.println();

  // Configure SSL client
  ssl_client.setInsecure();
  ssl_client.setTimeout(1000); // Set connection timeout
  ssl_client.setBufferSizes(4096, 1024); // Set buffer sizes

  // Initialize Firebase
  initializeApp(aClient, app, getAuth(user_auth), processData, "🔐 authTask");
  app.getApp<RealtimeDatabase>(Database);
  Database.url(DATABASE_URL);
  timeClient.begin();
}

void loop(){
  // Maintain authentication and async tasks
  app.loop();

  bool Sr=false;
 
  while(Serial.available())
  {
    //get sensor data from serial put in sensor_data
    sensor_data=Serial.readString(); 
    Sr=true;    
  }

  // Check if authentication is ready
  if (app.ready() && Sr){
    values = sensor_data;

    int fComma = values.indexOf(',');
    int sComma = values.indexOf(',', fComma+1);
    int thComma = values.indexOf(',', sComma + 1);
  
    String temp = values.substring(0, fComma);
    String humid = values.substring(fComma+1, sComma);
    String pressure = values.substring(sComma+1, thComma);
    String intPredCast = values.substring(thComma+1);

    unsigned long currentTime = millis();
    unsigned long epochTime = getTime();

    if (currentTime - lastSendTime >= sendInterval){
      // Update the last send time
      lastSendTime = currentTime;
      String dir = "/" + String(epochTime) + "/";

      // send a string
      stringValue = "value_" + String(currentTime);

      // send an int
      Database.set<float>(aClient, dir + "temp", temp.toFloat(), processData, "RTDB_Send_Float");
      Database.set<float>(aClient, dir + "humid", humid.toFloat(), processData, "RTDB_Send_Float");
      Database.set<float>(aClient, dir + "pressure", pressure.toFloat(), processData, "RTDB_Send_Float");

      Database.set<int>(aClient, dir + "pred", intPredCast.toInt(), processData, "RTDB_Send_Int");
      Database.set<int>(aClient, dir + "timestamp", epochTime, processData, "RTDB_Send_Int");
    }
  }
}

void processData(AsyncResult &aResult){
  if (!aResult.isResult())
    return;

  if (aResult.isEvent())
    Firebase.printf("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.eventLog().message().c_str(), aResult.eventLog().code());

  if (aResult.isDebug())
    Firebase.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());

  if (aResult.isError())
    Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());

  if (aResult.available())
    Firebase.printf("task: %s, payload: %s\n", aResult.uid().c_str(), aResult.c_str());
}

unsigned long getTime() {
  timeClient.update();
  unsigned long now = timeClient.getEpochTime();
  return now;
}
