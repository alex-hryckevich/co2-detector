//#include <SdsDustSensor.h>

#include "FirebaseESP8266.h"

#include <ESP8266WiFi.h>
#include <DHT.h>

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <TFT_ILI9163C.h>


// Set these to run example.
#define WIFI_SSID "WIFI_SSID"
#define WIFI_PASSWORD "WIFI_PASSWORD"

#define WIFI_SSID_ALTERNATE "WIFI_SSID_ALTERNATE"
#define WIFI_PASSWORD_ALTERNATE "WIFI_PASSWORD"

#define FIREBASE_DOMAIN "FIREBASE_DOMAIN"
#define FIREBASE_AUTH "FIREBASE_AUTH"

//#define LED_PIN 3
//#define DHT11_PIN 2

#define SLEEP_DELAY 15000
#define CALC_CYCLES 4

#include <MHZ.h>

// pin for CO2 uart reading
#define MH_Z19_RX 2
#define MH_Z19_TX 5 

#define __CS 12
#define __DC 4

//int rxPin = D1;
//int txPin = D2;
//SdsDustSensor sds(rxPin, txPin);

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0  
#define WHITE   0xFFFF


//DHT dht(DHT11_PIN, DHT11, 11);

TFT_ILI9163C display = TFT_ILI9163C(__CS, __DC);
MHZ co2(MH_Z19_RX, MH_Z19_TX, MHZ19B);
FirebaseData firebaseData;

float pi = 3.1415926;
int i; 
int i2;
int cx;
int cy;
bool isFBAlive;

void setup() {
  Serial.begin(115200);
  //dht.begin();  
  display.begin();
  co2.setDebug(true);

  // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  //WiFi.begin(WIFI_SSID_ALTERNATE, WIFI_PASSWORD_ALTERNATE);
  Serial.println(""); 
  Serial.println("connecting to WI-FI");
  int wifiAttemptCnt = 0;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    wifiAttemptCnt++;
    delay(500);

    if (wifiAttemptCnt == 60) {
      Serial.println("switch to alternative wi-fi");
      WiFi.disconnect();
      delay(300);
      WiFi.begin(WIFI_SSID_ALTERNATE, WIFI_PASSWORD_ALTERNATE);
    }

    if (wifiAttemptCnt == 120) {
      Serial.println("cannot connect to the wi-fi");
      WiFi.disconnect();
      break;
    }
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
  
  Firebase.begin(FIREBASE_DOMAIN, FIREBASE_AUTH);
  //Firebase.stream("/led");  
}


// a_ - for average
float a_co2 = 0;
float a_co2raw = 0;
float a_temp = 0;
//float a_hum = 0;
int a_cnt = 0;


void getValues() {
//  float hum = dht.readHumidity();
  //float temp = dht.readTemperature();
  float temp = co2.getLastTemperature();

  int co2_val = co2.readCO2UART();
  if (co2_val > 400) {
    a_co2 = averagify(a_co2, co2_val); 
  }
  a_temp = averagify(a_temp, temp); 
  //a_hum = averagify(a_hum, hum); 


  Serial.println("ACTUAL: ");
  Serial.println("Temperature: " + String(temp));
  //Serial.println("Humidity: " + String(hum));
  Serial.println("CO2: " + String(co2_val));

  if (a_cnt == CALC_CYCLES) {
    a_cnt = 0;
  } else {
    a_cnt++;
  }
}

float averagify(float a, float v) {
  if (isnan(v)) {
    return a;
  }
  return (a * a_cnt + v) / (a_cnt+1);
}

void printValues() {
  Serial.println();
  Serial.println("Cycle count: " + String(a_cnt));
  Serial.println("Temperature: " + String(a_temp));
  //Serial.println("Humidity: " + String(a_hum));
  Serial.println("CO2: " + String(a_co2));
  Serial.println("-------------");
  Serial.println();
}

void displayValues() {
  display.setRotation(-pi/2);
  display.fillScreen(BLACK);
  display.setCursor(0,0);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  
  display.println(" ");
  display.println(" Uptime: " + uptime());

  display.println(" Cycle count: " + String(a_cnt));
  display.println(" Temperature: " + String(a_temp));
  display.println(" CO2: " + String(a_co2));

  display.println(" ");
  display.println(" ");
  display.print(" Connection: ");
  display.setTextColor(isFBAlive ? GREEN : RED);
  display.print(isFBAlive ? "Alive" : "Dead");
  display.println(" ");

}


void sendValues() {
  a_temp += (rand() % 100 - 50) / 100;
  //  Firebase.setFloat("triggers/harry/humidity/now", a_hum);

  isFBAlive = true;  
  if (Firebase.setFloat(firebaseData, "triggers/harry/temperature/now", a_temp)) {
    Serial.println("- - - temperature was sent");
  } else {
    isFBAlive = false;
    Serial.print("Firebase sending failed:");
    Serial.println(firebaseData.errorReason());  
  }
  if (Firebase.setFloat(firebaseData, "triggers/harry/co2/now", a_co2)) {
   Serial.println("- - - CO2 was sent");
  } else {
    isFBAlive = false;
    Serial.print("Firebase sending failed:");
    Serial.println(firebaseData.errorReason());  
  }
  
  printValues();
  displayValues();
}

void loop() {
  getValues();
  if (a_cnt == 0) {
    sendValues();
  }

  displayValues();
  delay(SLEEP_DELAY);
}


//-----------------------------
void testText(){
  display.setRotation(pi/2);
  display.fillScreen(WHITE);
  display.setCursor(0,0);
  display.setTextColor(WHITE);
  display.setTextColor(YELLOW);
  display.setTextSize(2);
  display.setTextColor(RED);
  display.setTextSize(3);
  display.println("RED");
  display.setTextColor(GREEN);
  display.setTextSize(4);
  display.println("GREEN");
}

//-----------------------------
void testtriangles() {
  display.clearScreen();
  int color = 0xF800;
  int t;
  int w = display.width()/2;
  int x = display.height();
  int y = 0;
  int z = display.width();
  for(t = 0 ; t <= 15; t+=1) {
    display.drawTriangle(w, y, y, x, z, x, color);
    x-=4;
    y+=4;
    z-=4;
    color+=10;
  }
}
//-----------------------------
void testroundrects() {
  display.clearScreen();
  int color = 100;
  int i;
  int t;
  for(t = 0 ; t <= 4; t+=1) {
    int x = 0;
    int y = 0;
    int w = display.width();
    int h = display.height();
    for(i = 0 ; i <= 24; i+=1) {
      display.drawRoundRect(x, y, w, h, 5, color);
      x+=2;
      y+=3;
      w-=4;
      h-=6;
      color+=10;
    }
    color+=10;
  }
}

//############################################ UPTIME vvvvv  #################################
String uptime(){
 long days=0;
 long hours=0;
 long mins=0;
 long secs=0;
 long currentmillis=millis();
 secs = currentmillis/1000; //convect milliseconds to seconds
 mins=secs/60; //convert seconds to minutes
 hours=mins/60; //convert minutes to hours
 days=hours/24; //convert hours to days
 secs=secs-(mins*60); //subtract the coverted seconds to minutes in order to display 59 secs max
 mins=mins-(hours*60); //subtract the coverted minutes to hours in order to display 59 minutes max
 hours=hours-(days*24); //subtract the coverted hours to days in order to display 23 hours max
 return  String(days) +"d  " + String(hours) + ":" + String(mins) + ":" + String(secs);
}
//############################################ UPTIME ^^^^  #################################
