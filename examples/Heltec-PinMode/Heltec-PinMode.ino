#define PIN GPIO0

void setup() {
  Serial.begin(115200);
  delay(500);

//  Serial.println("INPUT floating");
//  pinMode(PIN, INPUT);
  
  Serial.println("INPUT_PULLUP");
  pinMode(PIN, INPUT_PULLUP);

//  Serial.println("INPUT_PULLDOWN");
//  pinMode(PIN, INPUT_PULLDOWN);
}

void loop() {
  Serial.println(digitalRead(PIN));
  delay(1000);
}
