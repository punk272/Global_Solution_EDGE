//Autor: Fábio Henrique Cabrini
//Resumo: Esse programa possibilita ligar e desligar o led onboard, além de mandar o status para o Broker MQTT possibilitando o Helix saber
//se o led está ligado ou desligado.
//Revisões:
//Rev1: 26-08-2023 Código portado para o ESP32 e para realizar a leitura de luminosidade e publicar o valor em um tópico aprorpiado do broker 
//Autor Rev1: Lucas Demetrius Augusto 
//Rev2: 28-08-2023 Ajustes para o funcionamento no FIWARE Descomplicado
//Autor Rev2: Fábio Henrique Cabrini

#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <PubSubClient.h> // Importa a Biblioteca PubSubClient

//defines:
//defines de id mqtt e tópicos para publicação e subscribe denominado TEF(Telemetria e Monitoramento de Equipamentos)
#define TOPICO_SUBSCRIBE    "/TEF/hosp200/cmd"        //tópico MQTT de escuta
#define TOPICO_PUBLISH      "/TEF/hosp200/attrs"      //tópico MQTT de envio de informações para Broker
#define TOPICO_PUBLISH_2    "/TEF/hosp200/attrs/l"    //tópico MQTT de envio de informações para Broker
#define TOPICO_PUBLISH_3    "/TEF/hosp200/attrs/h"    //tópico MQTT de envio de informações para Broker
#define TOPICO_PUBLISH_4    "/TEF/hosp200/attrs/t"    //tópico MQTT de envio de informações para Broker
                                                      //IMPORTANTE: recomendamos fortemente alterar os nomes
                                                      //            desses tópicos. Caso contrário, há grandes
                                                      //            chances de você controlar e monitorar o ESP32
                                                      //            de outra pessoa.
#define ID_MQTT  "fiware_200"   //id mqtt (para identificação de sessão)
                                 //IMPORTANTE: este deve ser único no broker (ou seja, 
                                 //            se um client MQTT tentar entrar com o mesmo 
                                 //            id de outro já conectado ao broker, o broker 
                                 //            irá fechar a conexão de um deles).
                                 // o valor "n" precisa ser único!

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define DHTPIN 15     // Digital pin connected to the DHT sensor

// Defina os pinos para os LEDs
#define LED_AMARELO 27
#define LED_VERMELHO 26
#define LED_VERDE 25

#define DHTTYPE    DHT22

DHT dht(DHTPIN, DHTTYPE);

// WIFI
const char* SSID = "Wokwi-GUEST"; // SSID / nome da rede WI-FI que deseja se conectar
const char* PASSWORD = ""; // Senha da rede WI-FI que deseja se conectar

// MQTT
const char* BROKER_MQTT = "46.17.108.113"; //URL do broker MQTT que se deseja utilizar
int BROKER_PORT = 1883; // Porta do Broker MQTT

// Saídas Leds
int D4 = 2;       // Led builtin do ESP32 (led na placa)

//Variáveis e objetos globais
WiFiClient espClient; // Cria o objeto espClient
PubSubClient MQTT(espClient); // Instancia o Cliente MQTT passando o objeto espClient
char EstadoSaida = '0';  //variável que armazena o estado atual da saída

//Prototypes
void initWiFi();
void initMQTT();
void reconectWiFi(); 
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void VerificaConexoesWiFIEMQTT(void);
void InitOutput(void);

void setup() {
  Serial.begin(115200);

  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AMARELO, OUTPUT);

  InitOutput();
  initWiFi();
  initMQTT();
  dht.begin();
  MQTT.publish(TOPICO_PUBLISH, "s|off");

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();
  display.setTextColor(WHITE);
}

//Função: inicializa e conecta-se na rede WI-FI desejada
//Parâmetros: nenhum
//Retorno: nenhum
void initWiFi() 
{
    delay(10);
    Serial.println("------Conexao WI-FI------");
    Serial.print("Conectando-se na rede: ");
    Serial.println(SSID);
    Serial.println("Aguarde");
     
    reconectWiFi();
}

//Função: inicializa parâmetros de conexão MQTT(endereço do 
//        broker, porta e seta função de callback)
//Parâmetros: nenhum
//Retorno: nenhum
void initMQTT() 
{
    MQTT.setServer(BROKER_MQTT, BROKER_PORT);   //informa qual broker e porta deve ser conectado
    MQTT.setCallback(mqtt_callback);            //atribui função de callback (função chamada quando qualquer informação de um dos tópicos subescritos chega)
}

//Função: função de callback 
//        esta função é chamada toda vez que uma informação de 
//        um dos tópicos subescritos chega)
//Parâmetros: nenhum
//Retorno: nenhum
void mqtt_callback(char* topic, byte* payload, unsigned int length) 
{
    String msg;
     
    //obtem a string do payload recebido
    for(int i = 0; i < length; i++) 
    {
       char c = (char)payload[i];
       msg += c;
    }
    
    Serial.print("- Mensagem recebida: ");
    Serial.println(msg);
    
    //toma ação dependendo da string recebida:
    //verifica se deve colocar nivel alto de tensão na saída D0:
    //IMPORTANTE: o Led já contido na placa é acionado com lógica invertida (ou seja,
    //enviar HIGH para o output faz o Led apagar / enviar LOW faz o Led acender)
    if (msg.equals("hosp200@on|"))
    {
        digitalWrite(D4, HIGH);
        EstadoSaida = '1';
    }
 
    //verifica se deve colocar nivel alto de tensão na saída D0:
    if (msg.equals("hosp200@off|"))
    {
        digitalWrite(D4, LOW);
        EstadoSaida = '0';
    }
}

//Função: reconecta-se ao broker MQTT (caso ainda não esteja conectado ou em caso de a conexão cair)
//        em caso de sucesso na conexão ou reconexão, o subscribe dos tópicos é refeito.
//Parâmetros: nenhum
//Retorno: nenhum
void reconnectMQTT() 
{
    while (!MQTT.connected()) 
    {
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT)) 
        {
            Serial.println("Conectado com sucesso ao broker MQTT!");
            MQTT.subscribe(TOPICO_SUBSCRIBE); 
        } 
        else
        {
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Havera nova tentatica de conexao em 2s");
            delay(2000);
        }
    }
}

//Função: reconecta-se ao WiFi
//Parâmetros: nenhum
//Retorno: nenhum
void reconectWiFi() 
{
    //se já está conectado a rede WI-FI, nada é feito. 
    //Caso contrário, são efetuadas tentativas de conexão
    if (WiFi.status() == WL_CONNECTED)
        return;
         
    WiFi.begin(SSID, PASSWORD); // Conecta na rede WI-FI
     
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(100);
        Serial.print(".");
    }
   
    Serial.println();
    Serial.print("Conectado com sucesso na rede ");
    Serial.print(SSID);
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());
}

//Função: verifica o estado das conexões WiFI e ao broker MQTT. 
//        Em caso de desconexão (qualquer uma das duas), a conexão
//        é refeita.
//Parâmetros: nenhum
//Retorno: nenhum
void VerificaConexoesWiFIEMQTT(void)
{
    if (!MQTT.connected()) 
        reconnectMQTT(); //se não há conexão com o Broker, a conexão é refeita
     
     reconectWiFi(); //se não há conexão com o WiFI, a conexão é refeita
}

//Função: envia ao Broker o estado atual do output 
//Parâmetros: nenhum
//Retorno: nenhum
void EnviaEstadoOutputMQTT(void)
{
    if (EstadoSaida == '1')
    {
      MQTT.publish(TOPICO_PUBLISH, "s|on");
      Serial.println("- Led Ligado");
    }
    if (EstadoSaida == '0')
    {
      MQTT.publish(TOPICO_PUBLISH, "s|off");
      Serial.println("- Led Desligado");
    }
    Serial.println("- Estado do LED onboard enviado ao broker!");
    delay(1000);
}

//Função: inicializa o output em nível lógico baixo
//Parâmetros: nenhum
//Retorno: nenhum
void InitOutput(void)
{
    //IMPORTANTE: o Led já contido na placa é acionado com lógica invertida (ou seja,
    //enviar HIGH para o output faz o Led apagar / enviar LOW faz o Led acender)
    pinMode(D4, OUTPUT);
    digitalWrite(D4, HIGH);
    
    boolean toggle = false;

    for(int i = 0; i <= 10; i++)
    {
        toggle = !toggle;
        digitalWrite(D4,toggle);
        delay(200);           
    }

    digitalWrite(D4, LOW);
}

void loop() {
  delay(5000);

  char msgBuffer[4];
  //garante funcionamento das conexões WiFi e ao broker MQTT
  VerificaConexoesWiFIEMQTT();
 
  //envia o status de todos os outputs para o Broker no protocolo esperado
  EnviaEstadoOutputMQTT();


  //read temperature and humidity
  float temperatura = dht.readTemperature();
  float umidade = dht.readHumidity();

  if (isnan(umidade) || isnan(temperatura)) {
    Serial.println("Failed to read from DHT sensor!");
  }

  // clear display
  display.clearDisplay();
  
  // display temperature
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("Temperatura: ");
  display.setTextSize(2);
  display.setCursor(0,10);
  display.print(temperatura);
  display.print(" ");
  display.setTextSize(1);
  display.cp437(true);
  display.write(167);
  display.setTextSize(2);
  display.print("C");
  
  // display humidity
  display.setTextSize(1);
  display.setCursor(0, 35);
  display.print("Umidade: ");
  display.setTextSize(2);
  display.setCursor(0, 45);
  display.print(umidade);
  display.print(" %");
  
  // Control LEDs based on temperature and humidity
  if (temperatura >= -40 && temperatura <= 0) {
    digitalWrite(LED_VERDE, HIGH);   // Verde aceso
    digitalWrite(LED_VERMELHO, LOW); // Vermelho apagado
  } else if (temperatura > 0 && temperatura <= 80) {
    digitalWrite(LED_VERMELHO, HIGH);  // Vermelho aceso
    digitalWrite(LED_VERDE, LOW);      // Verde apagado
  } else {
    // Caso a temperatura não esteja em nenhum dos intervalos acima,
    // todos os LEDs são apagados
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_VERMELHO, LOW);
  }

  // Controle do LED amarelo baseado na umidade
  if (umidade >= 50 && umidade <= 100) {
    digitalWrite(LED_AMARELO, HIGH);  // Amarelo aceso
  } else {
    digitalWrite(LED_AMARELO, LOW);   // Amarelo apagado
  }
  
  Serial.println("Temperatura: " + String(temperatura, 2) + "°C");
  Serial.println("Umidade: " + String(umidade, 1) + "%");
  Serial.println("---");
  delay(2000); // Wait for a new reading from the sensor (DHT22 has ~0.5Hz sample rate)

  //Publicação tópico MQTT Umidade
  dtostrf(umidade, 4, 1, msgBuffer);
  MQTT.publish(TOPICO_PUBLISH_3,msgBuffer);
    
  //Publicação tópico MQTT Temperatura
  dtostrf(temperatura, 4, 1, msgBuffer);
  MQTT.publish(TOPICO_PUBLISH_4,msgBuffer);
    
  //keep-alive da comunicação com broker MQTT
  MQTT.loop();
  
  display.display();
}