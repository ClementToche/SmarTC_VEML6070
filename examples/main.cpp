#include <Arduino.h>
#include <SmarTC_VEML6070.h>

/**
 * @brief Example for ESP8266 Module
 */

// Example with Generic VEML6070 Module (https://fr.aliexpress.com/item/32839082701.html?spm=a2g0s.9042311.0.0.16f16c37XftlSx)
// Let's take Standard IT (1 T) with 270KOhm RSet
SmarTC_VEML6070 uvs = SmarTC_VEML6070(VEML6070_4_T, 270);

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;

  Serial.println("SmarTC VEML6070 Sensor Library Test");

  if (uvs.launch())
    Serial.println("VEML Launch succeed");
  else
    Serial.println("/!\\ VEML Launch failure");
}

void loop()
{
  Serial.print("UV Value: ");
  Serial.println(uvs.getUV());

  delay(500);
}