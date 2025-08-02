//#define KILL_NVS 1

#include <Arduino.h>
#include "Jeepify.h"

const int DEBUG_LEVEL = 3; 
const int _LED_SIGNAL = 1;

#define WAIT_ALIVE       15000
#define WAIT_AFTER_SLEEP  3000

uint32_t WaitForContact = WAIT_AFTER_SLEEP;

#pragma region Includes
#include <esp_now.h>
#include <WiFi.h>
#include <nvs_flash.h>
#define u8 unsigned char

#include "Jeepify.h"    
#pragma endregion Includes

#pragma region Globals
u_int8_t    broadcastAddressAll[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}; 
const char *broadCastAddressAllC = "FFFFFFFFFFFF";
#pragma endregion Globals

#pragma region Functions
void OnDataRecv(const esp_now_recv_info *info, const uint8_t* incomingData, int len);
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status); 

void   SetMessageLED(int Color);
void   LEDBlink(int Color, int n, uint8_t ms);
#pragma endregion Functions

void setup()
{
    if (DEBUG_LEVEL > 0)
    {
        #ifdef ARDUINO_USB_CDC_ON_BOOT
            //delay(3000);
        #endif
    }
   
    if (DEBUG_LEVEL > 0)
    {
        Serial.begin(115200);
    }

    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    
    switch (wakeup_reason) {
        case ESP_SLEEP_WAKEUP_TIMER:    
            WaitForContact = WAIT_AFTER_SLEEP; 
            LEDBlink(4, 1, 100);
            break;
        default:                        
            WaitForContact = WAIT_ALIVE; 
            LEDBlink(3, 3, 100);
            break;
    }

    WiFi.mode(WIFI_STA);
    WiFi.STA.begin();
    uint8_t MacTemp[6];
    WiFi.macAddress(MacTemp);
    
    if (esp_now_init() != 0) 
        Serial.printf("Error initializing ESP-NOW\n\r");
    
    esp_now_register_send_cb(OnDataSent);
    esp_now_register_recv_cb(OnDataRecv);    
}
#pragma region System-Things
void SetMessageLED(int Color)
{
    // 0-off, 1-Red, 2-Green, 3-Blue, 4=violett
    if (Color > 0) TSLed = millis();
    else TSLed = 0;

    #if defined(LED_PIN) || defined(RGBLED_PIN)    
        if (_LED_SIGNAL) 
        switch (Color)
        {
            case 0: 
                #ifdef MODULE_TERMINATOR_PRO
                    smartdisplay_led_set_rgb(0, 0, 0);
                #else
                    #ifdef RGBLED_PIN
                        pixels.clear();
                        pixels.show();
                    #endif
                    #ifdef LED_PIN
                        digitalWrite(LED_PIN, LED_OFF);
                    #endif
                #endif
                break;
            case 1:
                #ifdef MODULE_TERMINATOR_PRO
                    smartdisplay_led_set_rgb(1, 0, 0);
                #else
                    #ifdef RGBLED_PIN
                        pixels.clear();
                        pixels.setPixelColor(0, pixels.Color (255,0,0));
                        pixels.show();
                    #endif
                    #ifdef LED_PIN
                        digitalWrite(LED_PIN, LED_ON);
                    #endif
                #endif
                break;
            case 2:
                #ifdef MODULE_TERMINATOR_PRO
                    smartdisplay_led_set_rgb(0, 1, 0);
                #else   
                    #ifdef RGBLED_PIN
                        pixels.clear();
                        pixels.setPixelColor(0, pixels.Color (0,255,0));
                        pixels.show();
                    #endif
                    #ifdef LED_PIN
                        digitalWrite(LED_PIN, LED_ON);
                    #endif
                #endif
                break;
            case 3:
                #ifdef MODULE_TERMINATOR_PRO
                    smartdisplay_led_set_rgb(0, 0, 1);
                #else
                    #ifdef RGBLED_PIN
                        pixels.clear();
                        pixels.setPixelColor(0, pixels.Color (0,0,255));
                        pixels.show();
                    #endif
                    #ifdef LED_PIN
                        digitalWrite(LED_PIN, LED_ON);
                    #endif
                #endif
                break;
            case 4:
                #ifdef MODULE_TERMINATOR_PRO
                    smartdisplay_led_set_rgb(1, 0, 1);
                #else
                    #ifdef RGBLED_PIN
                        pixels.clear();
                        pixels.setPixelColor(0, pixels.Color (255,0,255));
                        pixels.show();
                    #endif
                    #ifdef LED_PIN
                        digitalWrite(LED_PIN, LED_ON);
                    #endif
                #endif
                break;  
        }
    #endif
}
void LEDBlink(int Color, int n, uint8_t ms)
{
    for (int i=0; i<n; i++)
    {
        SetMessageLED(Color);
        delay(ms);
        SetMessageLED(0);
        delay(ms);
    }
}
#pragma endregion System-Things
#pragma region ESP-Things
void OnDataRecv(const esp_now_recv_info *info, const uint8_t* incomingData, int len)
{
    char* buff = (char*) incomingData;        //char buffer
    
    char *_TTLS = strstr(buff, SEND_CMD_JSON_TTL);
    int _TTL = atoi(_TTLS + 4);

    _TTL--;
    if (_TTL == 0) return;

    _TTLS[6] = 48+_TTL;
    
    esp_now_send(broadcastAddressAll, (uint8_t*) buff, 250); 
}
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) 
{ 
    if (DEBUG_LEVEL > 2) 
        if (status == ESP_NOW_SEND_SUCCESS) Serial.println("\r\nLast Packet Send Status: Delivery Success");
        
    if (DEBUG_LEVEL > 0)  
        if (status != ESP_NOW_SEND_SUCCESS) Serial.println("\r\nLast Packet Send Status: Delivery Fail");
}
#pragma endregion ESP-Things
void loop()
{
}
