#define LED 8

void setup() 
{
  pinMode(LED, OUTPUT);
  Serial.begin(115200);
  Serial.println("ESP32-C3 LED Blink Pattern Started!");
}

void loop() 
{
  digitalWrite(LED, LOW);  
  delay(100);  
  digitalWrite(LED, HIGH);
  delay(100);  

  digitalWrite(LED, LOW);  
  delay(300);  
  digitalWrite(LED, HIGH);
  delay(300);  

  digitalWrite(LED, LOW);  
  delay(500);  
  digitalWrite(LED, HIGH);
  delay(500);  

  digitalWrite(LED, LOW);  
  delay(700);  
  digitalWrite(LED, HIGH);
  delay(700);  

  digitalWrite(LED, LOW);  
  delay(900);  
  digitalWrite(LED, HIGH);
  delay(900);  

  digitalWrite(LED, LOW);  
  delay(1100);  
  digitalWrite(LED, HIGH);
  delay(1000);  

  digitalWrite(LED, LOW);  
  delay(1100);  
  digitalWrite(LED, HIGH);
  delay(1100);  

  digitalWrite(LED, LOW);  
  delay(1300);  
  digitalWrite(LED, HIGH);
  delay(1300);   
}




