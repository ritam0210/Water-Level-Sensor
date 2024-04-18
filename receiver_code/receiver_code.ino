#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HTTPClient.h>
#include "time.h"

LiquidCrystal_I2C lcd(0x27, 20, 4);

//===================================================================================================
#define led 2
int ledState = LOW;   // ledState used to set the LED
unsigned long previousMillis = 0;   // will store last time LED was updated

//---------------------------------------------------------------------------------------------------
float fixed_lvl = 6.0;
float level_1 = 6.0;
float level_2 = 0.0;
float level_3 = 0.0;
float level_4 = 0.0;

//==================================GOOGLE SHEET CREDENTIALS========================================
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 19800;
const int   daylightOffset_sec = 0;
// Google script ID and required credentials
String GOOGLE_SCRIPT_ID = "AKfycbwlRdysYQKbNlD31cDTYsl9q_Qvyt-4GWD4sdGU2K3KDi0AoTbUgClvtZVfC9Oq_EYM";
int count = 0;
//===================================================================================================

//---------------------------------------------------------------------------------------------------
// Replace with your network credentials (STATION)
const char* ssid = "Accio_Guest";
const char* password = "Accio@Guest";

//==================================ESP NOW RECEIVER STRUCTURE=======================================

typedef struct RxStruct
{
    int ID;
    float water_lvl;
}RxStruct;

// Create a struct_message called myData
RxStruct receivedData;

// Create a structure to hold the readings from each board
RxStruct board1;
RxStruct board2;
RxStruct board3;
RxStruct board4;

// Create an array with all the structures
RxStruct boardsStruct[4] = {board1, board2, board3, board4};

//===================================================================================================

//=================================ESP NOW RECEIVER FUNCTION=========================================

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len)
{
  memcpy(&receivedData, incomingData, sizeof(receivedData));
//  blinkit();
  
  // Update the structures with the new incoming data
  boardsStruct[receivedData.ID-1].water_lvl = receivedData.water_lvl;
}

//===================================================================================================

//==============================-=====FUNCTIONS FOR LED BLINKING=====================================

//void blinkit()
//{
//  unsigned long currentMillis = millis();
//  if (currentMillis - previousMillis >= 750) 
//  {
//    // save the last time you blinked the LED
//    previousMillis = currentMillis;  
//    if(ledState == LOW)
//    {
//      ledState = HIGH;                        
//    }    
//    else
//    {
//      ledState = LOW;
//    }
//  }
//  digitalWrite(led, ledState);
//}
//===================================================================================================

void updateDisplay()
{
    lcd.setCursor(0, 0);
    lcd.print("Const level ");
    lcd.print(fixed_lvl);
    lcd.print("cm");
    lcd.setCursor(0, 1);
    lcd.print("Node 1: ");
//    lcd.print(level_1);
    lcd.print("6.0 cm");
    lcd.setCursor(0,2);
    lcd.print("Node 2: ");
//    lcd.print(level_2);
    lcd.print(" NA");
    lcd.setCursor(5,3);
    lcd.print("Pumps OFF");
    delay(5000);
  
    lcd.setCursor(0, 0);
    lcd.print("Const level ");
    lcd.print(fixed_lvl);
    lcd.print("cm");
    lcd.setCursor(0,1);
    lcd.print("Node 3: ");
//    lcd.print(level_3);
    lcd.print(" NA        ");
    lcd.setCursor(0,2);
    lcd.print("Node 4: ");
//    lcd.print(level_4);
    lcd.print(" NA        ");
    lcd.setCursor(5,3);
    lcd.print("Pumps OFF");
    delay(2500);

}
//===================================================================================================

void setup() 
{
  Serial.begin(9600);
  lcd.init();       
  lcd.backlight();            // Make sure backlight is on
  lcd.clear();
  
  pinMode(led, OUTPUT);

  // Set the device as a Station and Soft Access Point simultaneously
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) 
  {
    Serial.println("Setting as a Wi-Fi Station..");
    delay(1000);
  }
  Serial.print("Station IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Wi-Fi Channel: ");
  Serial.println(WiFi.channel());
  //--------------------------------------------------------------------------------------------------
  
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);
  
  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.flush();

}

//===================================================================================================

void loop() 
{
//  level_1 = boardsStruct[0].water_lvl;
//  level_2 = boardsStruct[1].water_lvl;
//  level_3 = boardsStruct[2].water_lvl;
//  level_4 = boardsStruct[3].water_lvl;
//
//  updateDisplay();

  if (WiFi.status() == WL_CONNECTED) 
  {
      static bool flag = false;
      struct tm timeinfo;
      if (!getLocalTime(&timeinfo)) 
      {
        Serial.println("Failed to obtain time");
        return;
      }
      
//      String level_1 = String(boardsStruct[0].water_lvl);
//      String level_2 = String(boardsStruct[1].water_lvl);
//      String level_3 = String(boardsStruct[2].water_lvl);
//      String level_4 = String(boardsStruct[3].water_lvl);

      String level_1 = String(level_1);
      String level_2 = String(level_2);
      String level_3 = String(level_3);
      String level_4 = String(level_4);

      String urlFinal = "https://script.google.com/macros/s/"+GOOGLE_SCRIPT_ID+"/exec?node1=" + level_1 + "&node2=" + level_2 + "&node3=" + level_3 + "&node4=" + level_4;
      
      Serial.print("POST data to spreadsheet:");
      Serial.println(urlFinal);
      HTTPClient http;
      http.begin(urlFinal.c_str());
      http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
      int httpCode = http.GET(); 
      Serial.print("HTTP Status Code: ");
      Serial.println(httpCode);
      //---------------------------------------------------------------------
      //getting response from google sheet
      String payload;
      if (httpCode > 0) {
          payload = http.getString();
          Serial.println("Payload: "+payload);    
      }
      //---------------------------------------------------------------------
      http.end();
  }
  count++;
  updateDisplay();
  delay(52000);
}
