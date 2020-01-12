#include <math.h> 
#include <stdio.h> 
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const int refresh=3;//3 seconds
// DHT settings starts
#include "DHT.h"
#define DHTPIN 12   
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);
float tValue;//
//_______________________________LCD___________________________
#include <Adafruit_GFX.h>    // Core graphics library
#include <XTronical_ST7735.h> // Hardware-specific library
#include <SPI.h>

// set up pins we are going to use to talk to the screen
#define TFT_DC     D4       // register select (stands for Data Control perhaps!)
#define TFT_RST   D3         // Display reset pin, you can also connect this to the ESP8266 reset
                            // in which case, set this #define pin to -1!
#define TFT_CS   D8       // Display enable (Chip select), if not enabled will not talk on SPI bus

// initialise the routine to talk to this display with these pin connections (as we've missed off
// TFT_SCLK and TFT_MOSI the routine presumes we are using hardware SPI and internally uses 13 and 11
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);  


//______________________________________________________________



#include "Wire.h"
#define TMP102_I2C_ADDRESS 72 /* This is the I2C address for our chip. This value is correct if you tie the ADD0 pin to ground. See the datasheet for some other values. */

// Reverses a string 'str' of length 'len' 
void reverse(char* str, int len) 
{ 
    int i = 0, j = len - 1, temp; 
    while (i < j) { 
        temp = str[i]; 
        str[i] = str[j]; 
        str[j] = temp; 
        i++; 
        j--; 
    } 
} 
  
// Converts a given integer x to string str[].  
// d is the number of digits required in the output.  
// If d is more than the number of digits in x,  
// then 0s are added at the beginning. 
int intToStr(int x, char str[], int d) 
{ 
    int i = 0; 
    while (x) { 
        str[i++] = (x % 10) + '0'; 
        x = x / 10; 
    } 
  
    // If number of digits required is more, then 
    // add 0s at the beginning 
    while (i < d) 
        str[i++] = '0'; 
  
    reverse(str, i); 
    str[i] = '\0'; 
    return i; 
} 
  
// Converts a floating-point/double number to a string. 
void ftoa(float n, char* res, int afterpoint) 
{ 
    // Extract integer part 
    int ipart = (int)n; 
  
    // Extract floating part 
    float fpart = n - (float)ipart; 
  
    // convert integer part to string 
    int i = intToStr(ipart, res, 0); 
  
    // check for display option after point 
    if (afterpoint != 0) { 
        res[i] = '.'; // add dot 
  
        // Get the value of fraction part upto given no. 
        // of points after dot. The third parameter  
        // is needed to handle cases like 233.007 
        fpart = fpart * pow(10, afterpoint); 
  
        intToStr((int)fpart, res + i + 1, afterpoint); 
    } 
} 

//_______________________________MQTT-WIF CONFIG___________________________________


#define temperaturepub "home/NodeMcu/temperature"
#define temperaturesub "home/NodeMcu/gettemperature"

#define humiditypub "home/NodeMcu/humidity"
#define humiditysub "home/NodeMcu/gethumidity"




const char* ssid = "HUAWEI P9"; // Enter your WiFi name
const char* password =  "11111111"; // Enter WiFi password
const char* mqttServer = "192.168.43.189";
const int mqttPort = 1883;
const char* mqttUser = "otfxknod";
const char* mqttPassword = "nSuUc1dDLygF";



WiFiClient espClient;
PubSubClient client(espClient);
//___________________________________________________________________________________



void setup() {
  
  tft.init();
  Serial.begin(115200);
  Wire.begin();
  WiFi.begin(ssid, password);
  
  tft.setCursor(0, 0);
  tft.fillScreen(ST7735_BLACK);
 
  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(1);
   tft.println("            ");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
    tft.println("Connecting to WiFi..");
    
  }

  Serial.println("Connected to the WiFi network");
  tft.setCursor(0, 0);
  tft.fillScreen(ST7735_BLACK);
  //tft.setCursor(0, 0);
  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(1);
  tft.println("            ");
   
  tft.println("Connected to the WiFi network");
  
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(1);
  tft.println("            ");
  while (!client.connected()) {
    
    Serial.println("Connecting to MQTT...");
    tft.println("Connecting to MQTT...");
    if (client.connect("ESP8266Client")) {
       tft.fillScreen(ST7735_BLACK);
     tft.setCursor(0, 0);
    tft.setTextColor(ST7735_WHITE);
    tft.setTextSize(1);
    tft.println("            ");
      Serial.println("connected");  
      tft.println("connected");
 
    } else {
      tft.println("            ");
      Serial.print("failed with state ");
      Serial.print(client.state());
       tft.println("failed with state");
        tft.println("client.state()");
      delay(2000);
 
    }
  }

 }



//_____________________________DECODIFICARE COMENZI_________________________________
void callback(char* topic, byte* payload, unsigned int length) {
 
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
 
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  
  Serial.println();
  Serial.println("-----------------------");
}


//_______________________________SENZOR TEMPERATURA___________________________________
float getTemp102()
{
  byte firstbyte, secondbyte; //these are the bytes we read from the TMP102 temperature registers
  int val; /* an int is capable of storing two bytes, this is where we "chuck" the two bytes together. */ 
  float convertedtemp; /* We then need to multiply our two bytes by a scaling factor, mentioned in the datasheet. */ 
  float correctedtemp; 
 
  /* Reset the register pointer (by default it is ready to read temperatures)
You can alter it to a writeable register and alter some of the configuration - 
the sensor is capable of alerting you if the temperature is above or below a specified threshold. */
 
  Wire.beginTransmission(TMP102_I2C_ADDRESS); //Say hi to the sensor. 
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(TMP102_I2C_ADDRESS, 2);
  Wire.endTransmission();
 
 
  firstbyte      = (Wire.read()); 
  /*read the TMP102 datasheet - here we read one byte from
   each of the temperature registers on the TMP102*/
  secondbyte     = (Wire.read()); 
  /*The first byte contains the most significant bits, and 
   the second the less significant */
    val = firstbyte;
    if ((firstbyte & 0x80) > 0) 
    {
      val |= 0x0F00;
    } 
    val <<= 4; 
 /* MSB */
    val |= (secondbyte >> 4);    
/* LSB is ORed into the second 4 bits of our byte.
Bitwise maths is a bit funky, but there's a good tutorial on the playground*/
    convertedtemp = val*0.0625;
    correctedtemp = convertedtemp - 5; 
    /* See the above note on overreading */
 
 
  Serial.println(correctedtemp);
  Serial.println();
  
return correctedtemp;
}

void display_data(float temp,float h)
{
 tft.fillScreen(ST7735_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(2);
  tft.println("       ");
  tft.println("Temp:");
  tft.setTextSize(2);
  tft.print(temp);
  tft.println(" C");
  tft.println("       ");
  tft.println("Humidity:");
  tft.setTextSize(2);
  tft.print(h);
  tft.println(" %");
  
}

void loop() {
  client.loop();

//_______________________________READ_TEMPERATURE_AND_HUMIDITY_____________________
 int temp= getTemp102();
  float h = dht.readHumidity();// Reading humidity 
//_________________________________________________________________________________

  
   Serial.print("humidity: ");
   Serial.println(h);

//_______________________________CONVERT HUMIDITY/TEMPERATURE TO STR_____________________________
  char humidity[20];
  ftoa(h,  humidity, 4);
  char temperature[20];
  ftoa(temp,temperature , 4);
  
//___________________________________________________________________________________

  display_data(temp,h);
//_________________________________SEND DATA_________________________________________
  client.publish(humiditypub, humidity);
  client.publish(temperaturepub,temperature);
//_________________________________________________________________________________

  delay(4000);
   
}
