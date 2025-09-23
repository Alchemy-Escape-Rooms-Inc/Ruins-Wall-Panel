#define MATRIX_INPUT 2
#define MOSFET_OUTPUT 3 
#define PWM_PIN 11


uint8_t brightness;

void setup() {
  Serial.begin(9600);
  Serial.println("Program started.");

  pinMode(MATRIX_INPUT, INPUT);
  pinMode(MOSFET_OUTPUT, OUTPUT);
  pinMode(PWM_PIN,OUTPUT);

  brightness = 128;
}


void turnOnLED(){
  digitalWrite(MOSFET_OUTPUT,HIGH);  
}

void turnOffLED(){
  digitalWrite(MOSFET_OUTPUT,LOW);    
}

void drivePWM(){
  analogWrite(PWM_PIN,brightness);  
}
void loop() {}
