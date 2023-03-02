
#define MY_BLUE_LED_PIN 2 // for esp8266 nodemcu 1.0 and the wemos 

void setup() {
  Serial.begin(115200);
  delay(2000); 

  pinMode(MY_BLUE_LED_PIN, OUTPUT);     // Initialise the LED_BUILTIN pin as an output
}

// the loop function runs over and over again forever
void loop() {
  Serial.println("loop");
  digitalWrite(MY_BLUE_LED_PIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
  // but actually the LED is on; this is because
  // it is active low on the ESP-01)
  delay(1000);                      // Wait for a second
  digitalWrite(MY_BLUE_LED_PIN, HIGH);  // Turn the LED off by making the voltage HIGH
  delay(2000);  
                      // Wait for two seconds (to demonstrate the active low LED)
}