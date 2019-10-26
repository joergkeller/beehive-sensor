void setup() {
  Serial.begin(115200);
}

// see https://forum.arduino.cc/index.php?topic=120693.msg908179#msg908179
long readVcc() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  return result;
}

void loop() {
  Serial.print(readVcc() / 1023.0);
  Serial.println(" V");
  delay(1000);
}
