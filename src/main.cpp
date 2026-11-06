/*
   Inkplate10_Image_Frame_From_Web example for Soldered Inkplate 10
   For this example you will need only USB cable and Inkplate 10.
   Select "e-radionica Inkplate10" or "Soldered Inkplate10" from Tools -> Board menu.
   Don't have "e-radionica Inkplate10" or "Soldered Inkplate10" option? Follow our tutorial and add it:
   https://soldered.com/learn/add-inkplate-6-board-definition-to-arduino-ide/

   Want to learn more about Inkplate? Visit www.inkplate.io
   Looking to get support? Write on our forums: https://forum.soldered.com/
   28 July 2020 by Soldered
*/

const char *UNSPLASH_API_URL = "https://api.unsplash.com/photos/random?orientation=landscape&query=monochrome,silhouette";
char UNSPLASH_API_KEY[65] = ""; // Default API key, can be configured via WiFiManager

// Next 3 lines are a precaution, you can ignore those, and the example would also work without them
// #if !defined(ARDUINO_INKPLATE10) && !defined(ARDUINO_INKPLATE10V2)
// #error "Wrong board selection for this example, please select e-radionica Inkplate10 or Soldered Inkplate10 in the boards menu."
// #endif

#include "Inkplate.h"
#include <ArduinoJson.h>
#include <WiFiManager.h>
#include <Preferences.h>
#include <WiFiClientSecure.h>

#include "Fonts/AcariSans12.h"
#include "Fonts/AcariSansbd12.h"

#include "Utils/DisplayManager.h"


// Inkplate display(INKPLATE_3BIT); // Inkplate 10 in 3-bit mode
Inkplate display(INKPLATE_3BIT); // Inkplate 5 in 4 bit mode
Preferences preferences;

#if defined(ARDUINO_INKPLATE10) || defined(ARDUINO_INKPLATE10V2)
const char *networkName = "Inkplate10 Home Picture Frame";
#elif defined(ARDUINO_INKPLATE5) || defined(ARDUINO_INKPLATE5V2)
const char *networkName = "Inkplate5 Home Picture Frame";
#else
#error "Wrong board selection!"
#endif

void getImageUrl(char *resultUrl, char *resultDescription);
uint8_t *downloadFile(const char *url, int32_t *length);

void setup()
{
  Serial.begin(115200);
  display.begin();

  // Load saved API key from preferences
  preferences.begin("inkplate", false);
  preferences.getString("apikey", UNSPLASH_API_KEY, 65);
  preferences.end();
  
  Serial.println("Loaded API key from preferences: " + String(UNSPLASH_API_KEY));

  // Connect to WiFi
  WiFiManager wm;
  
  // Add custom parameter for Unsplash API Key
  WiFiManagerParameter custom_api_key("apikey", "Unsplash API Key", UNSPLASH_API_KEY, 64);
  wm.addParameter(&custom_api_key);
  
  // Set save params callback to save the API key
  wm.setSaveParamsCallback([&custom_api_key]() {
    Serial.println("Saving parameters...");
    strcpy(UNSPLASH_API_KEY, custom_api_key.getValue());
    
    // Save to preferences
    preferences.begin("inkplate", false);
    preferences.putString("apikey", UNSPLASH_API_KEY);
    preferences.end();
    
    Serial.println("API Key saved: " + String(UNSPLASH_API_KEY));
  });
  
  bool res = wm.autoConnect(networkName);
  if (!res)
  {
    Serial.println("Failed to connect");
  }
  else
  {
    Serial.println("Connected to WiFi.");
    
    // Update API key from the parameter (in case it was just configured)
    strcpy(UNSPLASH_API_KEY, custom_api_key.getValue());
    Serial.println("Current API Key: " + String(UNSPLASH_API_KEY));
    
    // If no API key is set, switch to config portal mode
    if (strlen(UNSPLASH_API_KEY) == 0)
    {
      Serial.println("No API key set. Starting configuration portal...");
      wm.startConfigPortal(networkName);
      
      // Save the API key after configuration
      // Callback already saves the key to Preferences; just update runtime value
      strcpy(UNSPLASH_API_KEY, custom_api_key.getValue());
      Serial.println("API Key configured: " + String(UNSPLASH_API_KEY));
    }
  }

  char url[256];
  char description[1024];
  getImageUrl(url, description);

  Serial.println("Image url: " + String(url));

  // Download the image ourselves; InkplateLibrary 11.x's downloadFileHTTPS crashes
  // (it destroys its WiFiClientSecure while HTTPClient still references it)
  bool drawImageResult = false;
  int32_t imageLen = 0;
  uint8_t *imageBuf = downloadFile(url, &imageLen);
  if (imageBuf)
  {
    drawImageResult = display.image.drawPngFromBuffer(imageBuf, imageLen, 0, 0, true, false);
    free(imageBuf);
  }

  if (!drawImageResult)
  {
    Serial.println("Error: Failed to draw image - check URL, network connection, or image format");

    display.setCursor(0, 0);
    display.println("Error: Failed to draw image: " + String(url));
  }

  // Draw description text
  int xpos = 10;
  int ypos = 10; // Will be updated below based on text height

  // display.setFont(&AcariSans_Regular20pt8b);

  // GFXfont font = AcariSans_Bold12pt8b;
  GFXfont font = AcariSans_Regular12pt8b;
  display.setFont(&font);
  // display.setCursor(0, display.height() - 50);
  // display.println(description);



  display.setTextColor(0);
  const int outlinewidth = 2;
  const int lineSpacing = 4;

  // Get number lines needed
  int textLines = drawWrappedText(display, font, description, xpos-outlinewidth, ypos, E_INK_WIDTH - 10 -outlinewidth, lineSpacing, true);
  Serial.println("Text lines: " + String(textLines));

  ypos = display.height() - (textLines * (font.yAdvance + lineSpacing)); // 16 is approx line height for 12pt font

  drawWrappedText(display, font, description, xpos, ypos-outlinewidth, E_INK_WIDTH - 10, lineSpacing, false);
  drawWrappedText(display, font, description, xpos, ypos+outlinewidth, E_INK_WIDTH - 10, lineSpacing, false);

  drawWrappedText(display, font, description, xpos-outlinewidth, ypos, E_INK_WIDTH - 10 -outlinewidth, lineSpacing, false);
  drawWrappedText(display, font, description, xpos-outlinewidth, ypos+outlinewidth, E_INK_WIDTH - 10 -outlinewidth, lineSpacing, false);
  drawWrappedText(display, font, description, xpos-outlinewidth, ypos-outlinewidth, E_INK_WIDTH - 10 -outlinewidth, lineSpacing, false);
  
  drawWrappedText(display, font, description, xpos+outlinewidth, ypos, E_INK_WIDTH - 10 +outlinewidth, lineSpacing, false);
  drawWrappedText(display, font, description, xpos+outlinewidth, ypos+outlinewidth, E_INK_WIDTH - 10 +outlinewidth, lineSpacing, false);
  drawWrappedText(display, font, description, xpos+outlinewidth, ypos-outlinewidth, E_INK_WIDTH - 10 +outlinewidth, lineSpacing, false);
  display.setTextColor(7);
  drawWrappedText(display, font, description, xpos, ypos, E_INK_WIDTH - 10, lineSpacing, false);

  display.display();

  Serial.println("Going to sleep");
  delay(100);
  esp_sleep_enable_timer_wakeup(15ll * 60 * 1000 * 1000); // 15 minutes
  esp_deep_sleep_start();
}

void loop()
{
  // Never here, as deepsleep restarts esp32
}

void getImageUrl(char *resultUrl, char* resultDescription)
{
  Serial.println("Getting random photo from Unsplash...");

  String url;
  HTTPClient http;
  http.begin(UNSPLASH_API_URL);
  http.addHeader("Authorization", String("Client-ID ") + UNSPLASH_API_KEY);

  int httpCode = http.GET();
  if (httpCode > 0)
  {
    Serial.println("Getting image URL from Unsplash response");

    // url = http.getString();

    JsonDocument doc;
    deserializeJson(doc, http.getStream());
    url = doc["urls"]["small"].as<String>();

    Serial.println("Image URL obtained: " + url);

    // Replace format to png and set size
    // url = url.substring(0, url.indexOf("?")) + "?crop=entropy&fit=crop&fm=png&h=820&w=1200"; // Crop: https://docs.imgix.com/en-US/apis/rendering/size/crop-mode#entropy
    url = url.substring(0, url.indexOf("?")) + "?crop=entropy&fit=crop&fm=png&h=" + String(E_INK_HEIGHT) + "&w=" + String(E_INK_WIDTH);

    // int urlStart = url.indexOf("href=\"") + 6;
    // int urlEnd = url.indexOf("\">", urlStart);

    // url = url.substring(urlStart, urlEnd);
    // url = url.substring(0, url.indexOf("?")) + "?crop=entropy&fit=crop&fm=png&h=800&w=1200";

    strcpy(resultUrl, url.c_str());

    String description = doc["description"].as<String>();
    if (description.length() == 0 || description == "null") {
      description = doc["alt_description"].as<String>();
    }
    if (description == "null" || description.length() == 0) {
      description = "";
    }
    Serial.println("Image description obtained: " + description);

    strcpy(resultDescription, description.c_str());
  }
  else
  {
    display.println("HTTP error:" + String(httpCode));
    display.display();
  }
  http.end();
}

// Downloads url into a PSRAM buffer. Caller must free() the result.
uint8_t *downloadFile(const char *url, int32_t *length)
{
  WiFiClientSecure client;
  client.setInsecure(); // Use HTTPS but don't check cert

  // Declared after client so it's destroyed first, while client is still valid
  HTTPClient http;
  http.useHTTP10(true); // Avoid chunked transfer encoding
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  if (!http.begin(client, url))
  {
    Serial.println("Error: Failed to begin image download");
    return nullptr;
  }

  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK)
  {
    Serial.println("Error: Image download HTTP code: " + String(httpCode));
    http.end();
    return nullptr;
  }

  int32_t size = http.getSize();
  int32_t capacity = size > 0 ? size : 256 * 1024;
  uint8_t *buffer = (uint8_t *)ps_malloc(capacity);
  if (!buffer)
  {
    Serial.println("Error: Failed to allocate image buffer");
    http.end();
    return nullptr;
  }

  WiFiClient *stream = http.getStreamPtr();
  int32_t total = 0;
  unsigned long lastData = millis();
  while ((size < 0 || total < size) && (http.connected() || stream->available()))
  {
    size_t available = stream->available();
    if (!available)
    {
      if (millis() - lastData > 10000)
        break; // Timed out waiting for data
      delay(1);
      continue;
    }

    if (total + (int32_t)available > capacity)
    {
      capacity *= 2;
      uint8_t *grown = (uint8_t *)ps_realloc(buffer, capacity);
      if (!grown)
      {
        Serial.println("Error: Failed to grow image buffer");
        free(buffer);
        http.end();
        return nullptr;
      }
      buffer = grown;
    }

    total += stream->readBytes(buffer + total, min((int32_t)available, capacity - total));
    lastData = millis();
  }
  http.end();

  if (size > 0 && total != size)
  {
    Serial.println("Error: Incomplete image download (" + String(total) + "/" + String(size) + " bytes)");
    free(buffer);
    return nullptr;
  }

  Serial.println("Downloaded image: " + String(total) + " bytes");
  *length = total;
  return buffer;
}
