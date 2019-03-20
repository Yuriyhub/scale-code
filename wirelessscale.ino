

// Loading the ESP8266WiFi library and the PubSubClient library
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <HX711.h>

// Setting definitions for scale

#define calibration_factor  -9526.0//-8437.0//-22795.0 //-7050.0 // -9826.0//This value is obtained using the SparkFun_HX711_Calibration sketch

#define DOUT 0  //Pin connected to HX711 (DT) data output pin
#define CLK 4 // Pin connected to HX711 (SCK)clk pin
#define NUM_MEASUREMENTS 10 // number of measurements
#define THRESHOLD 5 // measures only if the wight is greater than 2kg. convert this value to pounds if you're not using SI units.
#define THRESHOLD1 0.2 //restart averaging if the weight changes more than 0.2 kg
char result[8];
char data[80];

// Change the credentials below, so your ESP8266 connects to your router
const char* ssid = "fitmirror";
const char* password = "rpipotato";

// Change the variable to your Raspberry Pi IP address, so it connects to your MQTT broker
const char* mqtt_server = "192.168.1.101";

// Initializes the espClient
WiFiClient espClient;
PubSubClient client(espClient);


// Connect an LED to each GPIO of your ESP8266
const int ledGPIO5 = 5;

HX711 scale(DOUT, CLK); // Scale initialization
float weight = 0.0;
float prev_wight = 0.0;



// Timers auxiliar variables

long now = millis();
long lastMeasure = 0;

// Don't change the function below. This functions connects your ESP8266 to your router
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}

// This functions is executed when some device publishes a message to a topic that your ESP8266 is subscribed to
// Change the function below to add logic to your program, so when a device publishes a message to a topic that 
// your ESP8266 is subscribed you can actually do something
void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

}

// This functions reconnects your ESP8266 to your MQTT broker
// Change the function below if you want to subscribe to more topics with your ESP8266 
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");  
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// The setup function sets your ESP GPIOs to Outputs, starts the serial communication at a baud rate of 115200
// Sets your mqtt broker and sets the callback function
// The callback function is what receives messages and actually controls the LEDs
void setup() {
  
  // To output wifi information for debugging
   Serial.setDebugOutput(true);
    WiFi.printDiag(Serial);
  scale.set_scale(calibration_factor); //This value is obtained by using the SparkFun_HX711_Calibration sketch
  scale.tare(); //Assuming there is no weight on the scale at start up, reset the scale to 0
  ESP.wdtFeed();
  pinMode(ledGPIO5, OUTPUT);
  
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

// For this project, you don't need to change anything in the loop function. 
// Basically it ensures that you ESP is connected to your broker
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  if(!client.loop())
    client.connect("ESP8266Client");
    ESP.wdtFeed();


digitalWrite(ledGPIO5, LOW);

    // weight avg function
weight = scale.get_units();
float checkWeight = 0;
  float avgweight = 0;
  if (weight > THRESHOLD) { // Takes measures if the weight is greater than the threshold
    float weight0 = scale.get_units();
    delay(1000);
    for (int i = 0; i <= NUM_MEASUREMENTS; i++) {  // Takes several measurements
      weight = scale.get_units();
      
      digitalWrite(ledGPIO5, LOW);
        
      delay(100);
      Serial.println(weight);
      avgweight = avgweight + weight;
      if (abs(weight - weight0) > THRESHOLD1) {
        avgweight = 0;
        i = 0;
      }
      weight0 = weight;
      digitalWrite(ledGPIO5, HIGH);
    }
    avgweight = avgweight / NUM_MEASUREMENTS; // Calculate average weight
    Serial.print("Measured weight: ");
    Serial.print(avgweight, 1);
    Serial.println(" kg");
    dtostrf(avgweight, 6, 2, result);
//Jason string sent to raspberrypi
    String dhtReadings = "{ \"weight\": \"" + String(result) + "\"}";
    dhtReadings.toCharArray(data, (dhtReadings.length() + 1));

    
    while (scale.get_units() > THRESHOLD) {
      ESP.wdtFeed();

     now = millis();
  // Publishes new temperature and humidity every 10 seconds
  if (now - lastMeasure > 20000) {
    lastMeasure = now;
    
            // Now we can publish stuff!
         Serial.print(F("\nSending myValue "));
         Serial.print((result));
         Serial.print("...");
         //delay(5000);
         if (! client.publish("/esp8266/dhtreadings", data)) {
         Serial.println(F("Failed"));
         digitalWrite(ledGPIO5, LOW);
          } else {
            Serial.println(F("OK!"));
            delay(3000);
            digitalWrite(ledGPIO5, HIGH);
           }
  }   
    }
    delay(1000);   
  }
}
