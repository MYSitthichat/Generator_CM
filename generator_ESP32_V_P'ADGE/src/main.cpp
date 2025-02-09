#include <PZEM004Tv30.h>
#if !defined(PZEM_RX_PIN) && !defined(PZEM_TX_PIN)
  #define PZEM_RX_PIN 16
  #define PZEM_TX_PIN 17
#endif

#if !defined(PZEM_SERIAL)
  #define PZEM_SERIAL Serial2
#endif
#if defined(ESP32)
  PZEM004Tv30 pzem(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN);
#elif defined(ESP8266)
#else
  PZEM004Tv30 pzem(PZEM_SERIAL);
#endif

const int R1 = 32;
const int R2 = 33;
const int R3 = 25;
const int R4 = 26;

const int gride = 19;
const int generator = 21;
int signal_gride = 0;
int signal_gennerator = 0;
int state_start = 0;
bool start_machine = false;
void stop_machine(void);
void off_coil_start(void);
void setup()
{
  Serial.begin(115200);
  pinMode(R1,OUTPUT);
  pinMode(R2,OUTPUT);
  pinMode(R3,OUTPUT);
  pinMode(R4,OUTPUT);
  pinMode(gride,INPUT_PULLUP);
  pinMode(generator,INPUT_PULLUP);
  delay(1000);
}

void loop() {
  float voltage = pzem.voltage();
  signal_gride = digitalRead(gride);
  signal_gennerator = digitalRead(generator);
  // grid == 0 == ไฟดับ
  // grid == 1 == ไฟมา
  Serial.println(signal_gride);
  switch (signal_gride) 
  {
    case 1:
    {
      start_machine = false;
      digitalWrite(R1,LOW);
      digitalWrite(R2,LOW);
      digitalWrite(R4,LOW);
      stop_machine();
      delay(0);
      break;
    }
    case 0:
    {
      if (signal_gride == 0)  //ไฟดับ
            delay(5000);
            if(isnan(voltage)) //ไฟยังไม่ออกจาก generator
              {
                if (signal_gride == 0) 
                  {
                    start_machine = true;
                    if(start_machine == true)
                      { 
                        digitalWrite(R1,HIGH);
                        digitalWrite(R2,HIGH);
                        delay(3000);
                        if(signal_gride == 0)
                          {
                            digitalWrite(R4,HIGH);
                            delay(5000);
                            if(isnan(voltage))
                              {
                                off_coil_start();
                              }
                            else 
                              {
                                off_coil_start();
                                start_machine = false;
                              }         
                          }
                        else
                          {
                            start_machine = false;
                            off_coil_start();
                          }
    
                      }
                  }
      else //ไฟออกจาก generator
        {
          start_machine = false;
          digitalWrite(R4,LOW);
        }
    }
      break;
    }
    default:
    {
      start_machine = false;
      stop_machine();
      Serial.println("ERROR");
      break;
    }
  }
}

void stop_machine(void)
{
  digitalWrite(R1,LOW);
  digitalWrite(R2,LOW);
  digitalWrite(R4,LOW);
}
void off_coil_start(void)
{
  digitalWrite(R4,LOW);
}