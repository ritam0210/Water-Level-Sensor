#include <WiFi.h>
#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "Accio_Guest"
#define WIFI_PASSWORD "Accio@Guest"

// Insert Firebase project API Key
#define API_KEY "AIzaSyAlfJXPAqI06yIZZpI6C3Rr2GbSYPlLLfQ"
// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://waterlevel-a582d-default-rtdb.asia-southeast1.firebasedatabase.app/" 

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis1 = 0;
unsigned long sendDataPrevMillis2 = 0;
bool signupOK = false;

float node_1 = 6.0;
float node_2 = 0;
float node_3 = 0;
float node_4 = 0;
float level = 6.0;
String pump = "OFF";


//---------------------------------------------------------------------------------------------------

void setup()
{
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", ""))
  {
    Serial.println("ok");
    signupOK = true;
  }
  else
  {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop()
{
  fb_send();
  delay(500);
}

void fb_send()
{
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis1 > 4000 || sendDataPrevMillis1 == 0))
  {
    sendDataPrevMillis1 = millis();
    
    if (Firebase.RTDB.setFloat(&fbdo, "Recv_data/Node_1", node_1))
    Serial.println("PASSED");

    else 
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    
    if (Firebase.RTDB.setFloat(&fbdo, "Recv_data/Node_2", node_2))
    Serial.println("PASSED");

    else 
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    
    if (Firebase.RTDB.setFloat(&fbdo, "Recv_data/Node_3", node_3))
    Serial.println("PASSED");

    else 
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setFloat(&fbdo, "Recv_data/Node_4", node_4))
    Serial.println("PASSED");
      
    else 
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setString(&fbdo, "Recv_data/Pump", pump))
    Serial.println("PASSED");
      
    else 
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    //*******************************************************************************************
    if (Firebase.RTDB.setFloat(&fbdo, "App data/Level", level))
    Serial.println("PASSED");

    else 
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }


  }
}
