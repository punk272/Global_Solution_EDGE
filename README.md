<h1 align="center"> Sensor DHT22 + OLED + LEDs no ESP32 | Global Solution </h1> <br>
<p align="center">
  <a href="https://github.com/punk272/Global_Solution_EDGE">
    <img alt="Circuito" title="ESP32DHT22" src="https://i.imgur.com/GvD1Cp1.png" width="1000">
  </a>
</p>



## Saúde e Tecnologia

O desafio apresentado é: "Como podemos inovar a área da saúde usando a tecnologia?"
Nesta entrega, com um circuito utilizando ESP32, podemos medir a temperatura e umidade de um local para melhor armazenamento de remédios e vacinas que precisam ser guardados em condições específicas, oferecendo um meio simples de monitorar o armazenamento de medicações tanto para pequenas farmácias, centros médicos e postos de vacinação, quanto para o paciente em casa.

Para facilitar o monitoramento de dados, os valores de temperatura e umidade ficarão slavos em um servidor IoT por meio de FIWARE, através do protocolo MQTT.

## Componentes

Peças usadas na construção deste circuito:

* 1 x Placa ESP32
* 1 x Breadboard
* 1 x Sensor DHT22
* 1 x Display OLED
* 1 x LED Verde
* 1 x LED Vermelho
* 1 x LED Amarelo
* 3 x Resistores de 330 Ohms
* 1 x Resistor de 10k Ohms
* Jumpercables

## Configuração

<img alt="Config" title="Temperatura&Umidade" src="https://i.imgur.com/dnYjlmW.png" width="1000">

Nesta parte do código podemos fazer o ajuste para a temperatura e umidade que deseja monitorar.

## Simulação

(Clique <a href=https://wokwi.com/projects/381409774330712065/> aqui</a> para ver a simulação completa do projeto.

## Requisitos para conectar-se com o servidor IoT

* Collection FIWARE Postman (FIWARE Descomplicado.postman_collection.json): Recebe e armazena dos dados do ESP32.
* Código Fonte IDE Arduino (gs_edge.ino): Código principal do circuito, faz todo o processo de captura, leitura de dados e comunicação do microcontrolador com o servidor IoT através de Wi-Fi.
* Docker e Docker-Compose Ubuntu Server LTS (docker-compose.yml): Software onde o servidor será instalado (Clique <a href=https://docs.docker.com/engine/install/ubuntu/> aqui </a> para ver as instruções de instalação.

## Autor

- **Leonardo Mansur - RM:551659**
