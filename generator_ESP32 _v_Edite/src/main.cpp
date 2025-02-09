#include <Arduino.h>
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
float voltage = 0.0;
bool start_machine_success = false;

int start_machine_round = 0;

bool debuger_test = false;

String serial_command = "";
bool sub_string = false;  
int running_state = 0;
int signal_gride = 0;
unsigned long machine_timer = 0;

void run_machine(void);
void interprete_command(String serial_command);
bool check_serial_command(void);
String get_serial_command(void);
void reset_serial_variables(void);
void off_machine(void);
void read_pzem(void);

void setup() 
{
  Serial.begin(115200);
  pinMode(R1,OUTPUT);
  pinMode(R2,OUTPUT);
  pinMode(R3,OUTPUT);
  pinMode(R4,OUTPUT);
  pinMode(gride,INPUT_PULLUP);
  pinMode(generator,INPUT_PULLUP);
  reset_serial_variables();
  Serial.println("setup done");
}

void loop() 
{
  voltage = pzem.voltage();
  signal_gride = digitalRead(gride);
  if (check_serial_command() == true)
    {
      String serial_command = get_serial_command();
      interprete_command(serial_command);
    }
  if (debuger_test == false)
  {
    if (signal_gride == 0)
    {
      if(start_machine_success == false)
      {
        digitalWrite(R3,HIGH);
        run_machine();
      }
    }
    if (signal_gride == 1)
    {
      start_machine_round = 0;
      digitalWrite(R3,LOW);
      running_state = 0;
      off_machine();
      start_machine_success = false;
    }
  }
  if(running_state>0)
    {
      run_machine();
    }
}

void run_machine(void)
{
  switch (running_state)
  {
    case 0:
    {
      machine_timer = millis();
      running_state = 1;
      break;
    }
    
    case 1: //เช็คสถานะอีก 1 รอบ
    {
      if(start_machine_round < 5)
      {
        if ((millis() - machine_timer) > 5000) //ใช้จริง 60000 1 นาที
        {
          if (signal_gride == 0) //เริ่ม ออนคอยเพื่อเผาหัว
          {
            digitalWrite(R1,HIGH);
            digitalWrite(R2,HIGH);
            machine_timer = millis();
            running_state = 2;
          }
        }
      }
      if (debuger_test == true) //function debuger //เริ่ม ออนคอยเพื่อเผาหัว
      {
        digitalWrite(R1,HIGH);
        digitalWrite(R2,HIGH);
        machine_timer = millis();
        running_state = 2;
      }
      break;
    }

    case 2: // รอเผาหัว 5 วินาทีแล้วสาร์ทเครื่อง
    {
      if ((millis() - machine_timer) > 5000)//ใช้จริง 5000 5 วินาที
      {
        digitalWrite(R4,HIGH);
        machine_timer = millis();
        running_state = 3;
      }
      break;
    }

    case 3: // สตาร์ทเครื่อง 5 วินาทีแลล้วเช็คสถานะไฟจากเครื่องปั่นไฟ
    {
      if ((millis() - machine_timer) > 5000)
      {
        digitalWrite(R4,LOW);
        if (isnan(voltage)) // ถ้าสตาร์ทเครื่องแล้วไฟยังไม่ออกจาก generator
        {
          running_state = 5;
          machine_timer = millis();
        }
        if(voltage >= 200) // ถ้าสตาร์ทเครื่องแล้วไฟออกจาก generator
        {
          running_state = 4;
          machine_timer = millis();
        }
      }
      break;
    }

    case 4: // ถ้าไฟออกจาก generator ให้ปิดคอยสตาร์ท แล้วส่งค่าเพื่อนล็อคไม่ให้เข้า loop นี้
    {
      if ((millis() - machine_timer) > 1000)
        {
          digitalWrite(R4,LOW);
          start_machine_success = true;
        }
      break;
    }

    case 5: // ถ้าไฟยังไม่ออกจาก generator ให้ปิด 2 คอนเผาหัว รอ 3 วินาทีแล้วปิดคอยเผาหัว
    {
      if ((millis() - machine_timer) > 1000)
      {
        digitalWrite(R1,LOW);
        digitalWrite(R2,LOW);
        machine_timer = millis();
        running_state = 6;
      }
      break;
    }
    case 6: // รอเวลา 5 วินาทีแล้วเริ่มกระบวนการใหม่
    {
      if ((millis() - machine_timer) > 5000)
      {
        running_state = 1;
        start_machine_round++;
      }
      break;
    }
    default:
      off_machine();
      running_state = 0;
      break;
  }
}

void off_machine(void)
{
  digitalWrite(R1,LOW);
  digitalWrite(R2,LOW);
  digitalWrite(R3,LOW);
  digitalWrite(R4,LOW);
}

void interprete_command(String serial_command)
{
  switch(serial_command[0])
  {
    case 'd': // debug
      {
        switch (serial_command[1])
          {
            case '1':
              {
                Serial.println("debug R1");
                digitalWrite(R1,HIGH);
                break;
              }
            case '2':
              {
                Serial.println("debug R2");
                digitalWrite(R2,HIGH);
                break;
              }
            case '3':
              {
                Serial.println("debug R3");
                digitalWrite(R3,HIGH);
                break;
              }
            case '4':
              {
                Serial.println("debug R4");
                digitalWrite(R4,HIGH);
                break;
              }
            case '0':
              {
                Serial.println("debug off");
                digitalWrite(R1,LOW);
                digitalWrite(R2,LOW);
                digitalWrite(R3,LOW);
                digitalWrite(R4,LOW);
                break;
              }
            default:
              {
                break;
              }
          }
        break;
      }
    case 'c': // debug
      {
        read_pzem();
      }
    case 's': // start debuger
      {
        switch (serial_command[1])
        {
          case '1': // start debuger machine
            {
              Serial.println("start");
              debuger_test = true;
              running_state = 1;
              break;
            }
          case '0': // stop debuger machine
            {
              Serial.println("stop");
              debuger_test = false;
              running_state = 0;
              break;
            }
          default:
            {
              break;
            }
        }
        break;
      }
    case 'r': // reset mcu
      {
        Serial.println("reset command");
        off_machine();
        delay(100);
        ESP.restart();
        break;
      }
    default:
      {

      }
  }
}

bool check_serial_command(void)
{
    bool interprete_status = false;
    if(Serial.available()){
        char temp_command = Serial.read();
        if(temp_command == '\n')
        {
            interprete_status = true;
        }
        else
        {
            serial_command = serial_command + String(temp_command);
        }  
    }
    return interprete_status;
}

String get_serial_command(void)
{
    String temp_command = serial_command;
    serial_command = "";
    return temp_command;
}

void reset_serial_variables(void){
    String serial_command = "";
    String serial_response = "";
}

void read_pzem(void)
{
  float voltage = pzem.voltage();
  float current = pzem.current();
  float power = pzem.power();
  float energy = pzem.energy();
  float frequency = pzem.frequency();
  float powerFactor = pzem.pf();
  if (isnan(voltage) || isnan(current) || isnan(power) || isnan(energy) || isnan(frequency) || isnan(powerFactor)) {
      Serial.println("Could not read from sensor");
      return;
    }
  else
  {
    Serial.print("Voltage: ");
    Serial.print(voltage);
    Serial.print("V; Current: ");
    Serial.print(current);
    Serial.print("A; Power: ");
    Serial.print(power);
    Serial.print("W; Energy: ");
    Serial.print(energy);
    Serial.print("Wh; Frequency: ");
    Serial.print(frequency);
    Serial.print("Hz; Power Factor: ");
    Serial.println(powerFactor);
  }
}