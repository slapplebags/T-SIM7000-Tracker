#include "Adafruit_FONA.h" // https://github.com/botletics/SIM7000-LTE-Shield/tree/master/Code
#define SIMCOM_7000 // SIM7000A/C/E/G
#define FONA_PWRKEY 4
#define FONA_RST 25
#define FONA_TX 26 // ESP32 hardware serial RX2 (GPIO16)
#define FONA_RX 27 // ESP32 hardware serial TX2 (GPIO17)
#include <HardwareSerial.h>
#include <ArduinoJson.h>

HardwareSerial fonaSS(1);
Adafruit_FONA_LTE fona = Adafruit_FONA_LTE();
uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);
uint8_t type;
char replybuffer[255]; // this is a large buffer for replies
char imei[16] = {0}; // MUST use a 16 character buffer for IMEI!
const char* found = "found";
float latitude, longitude, speed_kph, heading, altitude, second;
const char* url = "dweet.io/get/latest/dweet/for/869951031430170";

void setup() {
  
  // put your setup code here, to run once:
pinMode(FONA_RST, OUTPUT);
  digitalWrite(FONA_RST, HIGH); // Default state

  pinMode(FONA_PWRKEY, OUTPUT);

  // Turn on the module by pulsing PWRKEY low for a little bit
  // This amount of time depends on the specific module that's used
  powerOn(); // See function definition at the very end of the sketch

  Serial.begin(9600);
  Serial.println(F("ESP32 Basic Test"));
  Serial.println(F("Initializing....(May take several seconds)"));

  // Note: The SIM7000A baud rate seems to reset after being power cycled (SIMCom firmware thing)
  // SIM7000 takes about 3s to turn on but SIM7500 takes about 15s
  // Press reset button if the module is still turning on and the board doesn't find it.
  // When the module is on it should communicate right after pressing reset
  
  // Start at default SIM7000 shield baud rate
  fonaSS.begin(115200, SERIAL_8N1, FONA_TX, FONA_RX); // baud rate, protocol, ESP32 RX pin, ESP32 TX pin

  Serial.println(F("Configuring to 9600 baud"));
  fonaSS.println("AT+IPR=9600"); // Set baud rate
  delay(100); // Short pause to let the command run
  fonaSS.begin(9600, SERIAL_8N1, FONA_TX, FONA_RX); // Switch to 9600
  if (! fona.begin(fonaSS)) {
    Serial.println(F("Couldn't find FONA"));
    while (1); // Don't proceed if it couldn't find the device
  }
  // Print module IMEI number.
  uint8_t imeiLen = fona.getIMEI(imei);
  if (imeiLen > 0) {
    Serial.print("Module IMEI: "); Serial.println(imei);
  }
fona.setFunctionality(1); // AT+CFUN=1
  fona.setNetworkSettings(F("h2g2"), F("h2g2"));

}

void loop() {

        // read website URL
        uint16_t statuscode;
        int16_t length;
        char url[80];
        char replybuffer[254];
        uint16_t replyidx = 0;
        
        flushSerial();

        Serial.println(F("****"));
        if (!fona.HTTP_GET_start(url, &statuscode, (uint16_t *)&length)) {
          Serial.println("Failed!");

        }
        while (length > 0) {
          while (fona.available()) {
            char c = fona.read();
            replybuffer[replyidx] = c;
            replyidx++;
            // Serial.write is too slow, we'll write directly to Serial register!
            Serial.write(c);             
          }
        }
        Serial.println(F("\n****"));
        fona.HTTP_GET_end();
        Serial.println(replybuffer);
        
        const size_t capacity = JSON_OBJECT_SIZE(1) + 2*JSON_OBJECT_SIZE(4) + 310;
        DynamicJsonDocument doc(capacity);
        const char* json = replybuffer;
deserializeJson(doc, json);

const char* by = doc["by"]; // "getting"
const char* the = doc["the"]; // "dweets"

JsonObject with_0 = doc["with"][0];
const char* with_0_thing = with_0["thing"]; // "869951031430170"
const char* with_0_created = with_0["created"]; // "2020-11-18T07:20:23.410Z"

const char* with_0_content_state = with_0["content"]["state"]; // "found"
        Serial.println(with_0_content_state);
        if (strcmp(with_0_content_state, found) == 0){
          Serial.println("fuck butt poops");
        }
      }



void report() {
  delay(60000);
  //float latitude, longitude, speed_kph, heading, altitude, second;
        uint16_t year;
        uint8_t month, day, hour, minute;
        char URL[150];

        // Use the top line if you want to parse UTC time data as well, the line below it if you don't care
        //        if (fona.getGPS(&latitude, &longitude, &speed_kph, &heading, &altitude, &year, &month, &day, &hour, &minute, &second)) {
        if (fona.getGPS(&latitude, &longitude, &speed_kph, &heading, &altitude)) { // Use this line instead if you don't want UTC time
          Serial.println(F("---------------------"));
          Serial.print(F("Latitude: ")); Serial.println(latitude, 6);
          Serial.print(F("Longitude: ")); Serial.println(longitude, 6);
          Serial.print(F("Speed: ")); Serial.println(speed_kph);
          Serial.print(F("Heading: ")); Serial.println(heading);
          Serial.print(F("Altitude: ")); Serial.println(altitude);
          // Comment out the stuff below if you don't care about UTC time
          char latBuff[12], longBuff[12];
          dtostrf(latitude, 1, 6, latBuff);
          dtostrf(longitude, 1, 6, longBuff);

          sprintf(URL, "dweet.io/dweet/for/%s?lat=%s&long=%s", imei, latBuff, longBuff);

          if (!fona.postData("GET", URL))
          Serial.println(F("Failed to complete HTTP GET..."));

}
}


void flushSerial() {
  while (Serial.available())
    Serial.read();
}

// Power on the module
void powerOn() {
  digitalWrite(FONA_PWRKEY, LOW);
  // See spec sheets for your particular module
  delay(100); // For SIM7000
  digitalWrite(FONA_PWRKEY, HIGH);
}
