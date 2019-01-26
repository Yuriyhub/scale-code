#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <HX711.h>
/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "fitmirror"
#define WLAN_PASS       "rpipotato"

/************************* Information to Connect to Adafruit IO *********************************/
/*  You will need and account - Its FREE  */

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "YuriyD"
#define AIO_KEY         "13f2bbf99eb74adda6859b1216ac9835"

/************ Setting up your WiFi Client and MQTT Client ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.

WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);



/************ Setting definitions for scale ******************/

#define calibration_factor -22795.0 //-7050.0 //This value is obtained using the SparkFun_HX711_Calibration sketch

#define DOUT 0  //Pin connected to HX711 (DT) data output pin
#define CLK 12 // Pin connected to HX711 (SCK)clk pin
#define NUM_MEASUREMENTS 10 // number of measurements
#define THRESHOLD 5 // measures only if the wight is greater than 2kg. convert this value to pounds if you're not using SI units.
#define THRESHOLD1 0.2 //restart averaging if the weight changes more than 0.2 kg

//int* myWeight;

HX711 scale(DOUT, CLK); // Scale initialization
float weight = 0.0;
float prev_wight = 0.0;




/****************************** Set Up a Feed to Publish To ***************************************/

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>

Adafruit_MQTT_Publish my_weight = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/my_weight");


/********************* Values ******************************/
// Need a changing value to send.  We will increment this value
// in the getVal function.  
//we start at zero and when it gets to 10 we start over.

//uint32_t xVal=-1;

/*************************** Sketch Code ***********************************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();

void setup() {
  Serial.begin(115200);
  WiFi.disconnect();
  delay(3000);
//  delay(10);
// To output wifi information for debugging
  Serial.setDebugOutput(true);
  WiFi.printDiag(Serial);
  
  

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  //Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while ((!(WiFi.status() == WL_CONNECTED))){
    delay(300);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP().toString());
 
 
  scale.set_scale(calibration_factor); //This value is obtained by using the SparkFun_HX711_Calibration sketch
  scale.tare(); //Assuming there is no weight on the scale at start up, reset the scale to 0

}


void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  
  ESP.wdtFeed();

// weight avg function

weight = scale.get_units();
float checkWeight = 0;
  float avgweight = 0;
  if (weight > THRESHOLD) { // Takes measures if the weight is greater than the threshold
    float weight0 = scale.get_units();
    delay(1000);
    for (int i = 0; i <= NUM_MEASUREMENTS; i++) {  // Takes several measurements
      weight = scale.get_units();
      delay(100);
      Serial.println(weight);
      avgweight = avgweight + weight;
      if (abs(weight - weight0) > THRESHOLD1) {
        avgweight = 0;
        i = 0;
      }
      weight0 = weight;
    }
    avgweight = avgweight / NUM_MEASUREMENTS; // Calculate average weight
    Serial.print("Measured weight: ");
    Serial.print(avgweight, 1);
    Serial.println(" kg");
    
    char result[8]; // Buffer big enough for 7-character float
    dtostrf(avgweight, 6, 1, result);

     //save myWeight to Adafruit.io
     //myWeight->save(avgweight);
     bool check = true;
    while (scale.get_units() > THRESHOLD) {
      ESP.wdtFeed();
        // Now we can publish stuff!
         Serial.print(F("\nSending myValue "));
         Serial.print((avgweight));
         Serial.print("...");
         delay(5000);
         if (! my_weight.publish(avgweight)) {
         Serial.println(F("Failed"));
          } else {
            Serial.println(F("OK!"));
            
           }
      
    }
    delay(2000);
   
  }
  



  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  /*
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
  */
  delay(1000);
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }
  

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
         //ESP.restart();
       }
  }
  Serial.println();
  Serial.println("MQTT Connected!");
}

