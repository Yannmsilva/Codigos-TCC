#include "DHT.h"
#include "heltec.h" 

#define BAND 433E6

//Sensores e Atuadores
#define DHTPIN 0    // Settings do sensor de Umidade e Temperatura do Ar
#define DHTTYPE DHT11 // Settings do sensor de Umidade e Temperatura do Ar

int s_UmidS = 34;
int s_Nivel = 38;

int bomba = 21;
int motor_IN1 = 12;
int motor_IN2 = 13;
int motor_ENA = 25;


//Propriedades do PWM do motor
const int freq = 30000;
const int pwmChannel = 8;
const int resolution = 8;
#define dutyCycle 150


//Variáveis auxiliares
int aux = 0;
int UmidAr = 0;
int UmidSolo = 0;
int TempAr = 0;
int Nivel = 0;

//Inicialização do sensor de Temperaturae Umidade do Ar
DHT dht(DHTPIN, DHTTYPE);

//Parâmetros para setagem do timer
unsigned long previousMillis = 0;
unsigned long intervalo = 5000; //Tempo de espera para enviar os dados em millisegundos
unsigned long tempoLigado = 60000;

//Parâmetros do deep sleep
#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  10770/* Time ESP32 will go to sleep (in seconds) */
RTC_DATA_ATTR int bootCount = 0;

void ativar_Bomba(){
  //Função específica para a ativação do Relé que vai acionar a bomba
  digitalWrite(bomba, HIGH);
  delay(5000);
  digitalWrite(bomba,LOW);
  }

void girar_Motor(int sentido, int tempo){
  //Função específica para a ativação do motor que vai subir a janela. A variável sentido indica qual o sentido de rotação e a variável tempo quanto tempo o motor vai ficar rodando
  //Sentido horário = 1, Sentido antihorário = -1
  //Tempo de rotação em milisegundos
  //Parâmetro velocidade é referente ao dutycycle do PWM. Valores de 0 a 255
  if (sentido == 1){
    digitalWrite(motor_IN1,HIGH);
    digitalWrite(motor_IN2,LOW);
    ledcWrite(pwmChannel, dutyCycle);
    }
    else if (sentido == -1){
      digitalWrite(motor_IN1,LOW);
      digitalWrite(motor_IN2,HIGH);
      ledcWrite(pwmChannel, dutyCycle);
      }
      
  delay(tempo);
  
  digitalWrite(motor_IN1,LOW);
  digitalWrite(motor_IN2,LOW);
 }


float leitura_TemperaturaAr(){
  float t = dht.readTemperature();
  return t;
  }

float leitura_UmidadeAr(){
  float h = dht.readHumidity();
  return h;
  }

int leitura_UmidadeSolo(){
  int x = analogRead(s_UmidS);
  int z = map(x, 2500,4100,100,0);
  return z;
  }

int leitura_NivelReservatorio(){
  int y = digitalRead(s_Nivel);  
  return y;
  }


void enviar_Dados(){
  //Função responsável por enviar as leituras dos sensores pro roteador

  Nivel = leitura_NivelReservatorio();
  UmidSolo = leitura_UmidadeSolo();
  UmidAr = leitura_UmidadeAr();
  TempAr = leitura_TemperaturaAr();
  
  LoRa.beginPacket();
  LoRa.setTxPower(14,RF_PACONFIG_PASELECT_PABOOST);
  LoRa.print(Nivel);
  //Serial.println(Nivel);
  LoRa.print(UmidSolo);
  //Serial.println(UmidSolo);
  LoRa.print(UmidAr);
  //Serial.println(UmidAr);
  LoRa.print(TempAr);
  //Serial.println(TempAr);
  LoRa.endPacket();
  } 


void Controle(){

  int x = leitura_UmidadeSolo();
  int y = leitura_UmidadeAr();
  int z = leitura_NivelReservatorio();

  if (x < 20 and z != 1){
    ativar_Bomba();
    }

  if (y > 95){
    girar_Motor(-1,340);
    aux = 1;
    } else if (y < 95 and aux == 1){
      girar_Motor(1, 200);
      aux = 0;
      }
  }

void setup() {
  // put your setup code here, to run once:
  dht.begin();
  Heltec.begin(false /*DisplayEnable Enable*/, true /*Heltec.LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);

  
  pinMode(s_UmidS, INPUT);
  pinMode(s_Nivel, INPUT);

  pinMode(bomba, OUTPUT);
  pinMode(motor_IN1, OUTPUT);
  pinMode(motor_IN2, OUTPUT);
  pinMode(motor_ENA,OUTPUT);
  
  ledcSetup(pwmChannel, freq, resolution);
  ledcAttachPin(motor_ENA, pwmChannel);

  
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  delay(10);

  while(millis() < tempoLigado){
    if (millis() - previousMillis >= intervalo){
      previousMillis += intervalo;
      enviar_Dados();
      Controle();
    }
  }  
  delay(50);
  
  LoRa.end();
  LoRa.sleep();
  delay(100);

  esp_deep_sleep_start();
  
}




void loop() {
  // put your main code here, to run repeatedly:
}
