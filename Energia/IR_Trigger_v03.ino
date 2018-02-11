// IR Trigger
// v03 using BJT driver

// RC-1 Equivalent trigger for Canon EOS
// Frecuency: 32700 Hz (29800 to 35500 Hz)
// Pulses/Burst: 16 (9 to 22)
// Two burst start delay: Immediate 7.82ms 
//                        Delayed (2s) 5.86ms 

// Works Ok 
// Use IR LED TSHF5210

#define LED 2 // Red LED at pin 2 (P1.0)
#define SW1 5 // Button between pin 5 (P1.3) and GND
#define SW2 7 // Button between pin 7 (P1.5) and GND
#define IR  6 // IR LED between pin 6 (P1.4) and VDD

void setup()
{
pinMode(LED,OUTPUT);       // LED pin in output mode
digitalWrite(LED,LOW);     // LED OFF

pinMode(SW1,INPUT_PULLUP); // Pull-Up at SW1
pinMode(SW2,INPUT_PULLUP); // Pull-Up at SW2

pinMode(IR,OUTPUT);        // IR LED pin in output mode
digitalWrite(IR,LOW);      // IR OFF
}

/************** TEST FUNCTIONS ****************/

// Changes blink frequency depending on SW1 and SW2
void blinkTest()
{
  digitalWrite(LED,HIGH);
  if (digitalRead(SW1)) 
      delay(1000);  // Time ON 1s if SW1 not pressed
      else
      delay(200);   // Time ON 0.2s of SW1 pressed
  digitalWrite(LED,LOW);  
  if (digitalRead(SW2))
      delay(1000);  // Time OFF 1s if SW2 not pressed
      else
      delay(200);   // Time OFF 0.2s if SW2 pressed
}

/**********************************************/

// Sends one burst of 16 pulses at 32700 Hz
// 31850 measured value
// Funciona 12-12
void burst()
 {
 int i;
 for(i=0;i<16;i++)
      {
      digitalWrite(IR,HIGH);
      delayMicroseconds(12);  // 3us overhead to 18us
      digitalWrite(IR,LOW);
      delayMicroseconds(12);
      } 
 }

// Send Inmediate (Test OK)
// Measured burst start separation 7.82ms
void immediate()
{
  digitalWrite(LED,HIGH);
  burst();
  delay(6);
  delayMicroseconds(635);
  burst();
  delay(200);
  digitalWrite(LED,LOW);
}

// Send Delayed (Test OK)
// Measured burst start separation 5.77ms
void delayed()
{
  digitalWrite(LED,HIGH);
  burst();
  delay(4);
  delayMicroseconds(635);
  burst();
  delay(200);
  digitalWrite(LED,LOW);
}


// Camera test
void cameraTest()
{
  if (!digitalRead(SW1))
     {
     delay(10);
     immediate();
     while (!digitalRead(SW1));
     delay(10);
     }
     
  if (!digitalRead(SW2))
     {
     delay(10);
     delayed();
     while (!digitalRead(SW2));
     delay(10);
     }     
    
}

void loop()
{
//  blinkTest();
cameraTest();

}


