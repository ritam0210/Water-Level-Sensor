#include <Adafruit_ADS1X15.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>

Adafruit_ADS1115 ads;
int16_t adc0;
float volts0;
float level = 11.0;

//=======================CREDENTIALS AND FUNCTIONS FOR ESP NOW====================================
// Set your Board ID (ESP32 Sender #1 = BOARD_ID 1, ESP32 Sender #2 = BOARD_ID 2, etc)
#define BOARD_ID 1

//-------------------------------------------------------------------------------------
uint8_t RxMACaddress[] = {0x40, 0x22, 0xD8, 0x7B, 0x64, 0xD8}; //C0:49:EF:E7:C5:70  40:22:D8:7B:64:D8  70:B8:F6:5B:FE:48

//-------------------------------------------------------------------------------------

//Structure example to send data
//Must match the receiver structure
typedef struct struct_message 
{
    int ID;
    float water_lvl;
} struct_message;

//Create a struct_message called myData
struct_message myData;

// Insert your SSID
constexpr char WIFI_SSID[] = "TP-Link_907E";

int32_t getWiFiChannel(const char *ssid) 
{
  if (int32_t n = WiFi.scanNetworks()) 
  {
      for (uint8_t i=0; i<n; i++) 
      {
          if (!strcmp(ssid, WiFi.SSID(i).c_str())) 
          {
              return WiFi.channel(i);
          }
      }
  }
  return 0;
}

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

//================================================================================================

void setup() 
{
  Serial.begin(115200);
  if (!ads.begin())
  {
    Serial.println("Failed to initialize ADS.");
    while (1);
  }

  //===============================SETTING UP FOR ESP NOW==========================================

  // Set device as a Wi-Fi Station and set channel
  WiFi.mode(WIFI_STA);

  int32_t channel = getWiFiChannel(WIFI_SSID);

  WiFi.printDiag(Serial); // Uncomment to verify channel number before
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);
  WiFi.printDiag(Serial); // Uncomment to verify channel change after

  //Init ESP-NOW
  if (esp_now_init() != ESP_OK) 
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  //Register peer
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, RxMACaddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  
  //Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() 
{
  adc0 = ads.readADC_SingleEnded(0);
  volts0 = ads.computeVolts(adc0);
  Serial.print("AIN0: "); Serial.print(adc0); Serial.print("  "); Serial.print(volts0); Serial.println("V");
  
  //=============================================================================================
  //===================================ESP NOW SENDER FUNCTIONS==================================
   myData.ID = BOARD_ID;
   myData.water_lvl = level;

   //-------------------------------------------------------------------------------------
   esp_err_t result = esp_now_send(RxMACaddress, (uint8_t *) &myData, sizeof(myData));
   //-------------------------------------------------------------------------------------
   if (result == ESP_OK) 
   {
      Serial.println("Sent with success");
   }
   else
   {
      Serial.println("Error sending the data");
   }
   //-------------------------------------------------------------------------------------

   delay(500); //main delay of the code

}
