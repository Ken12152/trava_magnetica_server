#include <ESP8266WiFi.h>


// Troque pelos dados da sua rede
const char* ssid = "SSID";
const char* password = "12345678";
const char* ip = "192.168.0.177";

// Timeout da conexão
#define TIMEOUT 500
#define MAX_BUFFER 300

// Server na porta 80 (padrão http)
WiFiServer server(80);

// Buffer onde serão gravados os bytes da comunicação
uint8_t buffer[MAX_BUFFER];



void setup() 
{
  Serial.begin(9600);
  //Serial.begin(115200);

  // Envia a informação da rede para conectar
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Espera a conexão com o access point
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  // Configura o IP
  IPAddress ipAddress;
  ipAddress.fromString(ip);
  WiFi.config(ipAddress, WiFi.gatewayIP(), WiFi.subnetMask());

  // Inicializa o server
  server.begin();
}



void loop() 
{
  // Verifica se alguem se conectou
  WiFiClient client = server.available();
  if(!client) {
    // Se ninguém conectou apenas retorna sem fazer nada
    return;
  }


  // Marca o tempo que o cliente se conectou
  // e a quantidade de bytes lidos
  uint32_t connectedTime = millis();
  int bytesRead = 0;

  
  // Enquanto o cliente estiver conectado
  while(client.connected()) {

    /////////////// Verifica timeout /////////////// 
    // Tempo agora
    uint32_t now = millis();
    // Quanto tempo passou desde a conexão com o cliente
    uint32_t ellapsed = now - connectedTime;

    // Se o tempo passou do tempo máximo e não leu nenhum byte
    if(ellapsed > TIMEOUT && bytesRead == 0) {
      // Fecha a conexão com o cliente
      client.stop();
      break;
    }


    /////////////// Se o "Cliente" possui bytes a serem lidos ///////////////
    int available = client.available();
    
    if(available) {
      int bufferSize = available < MAX_BUFFER ? available : MAX_BUFFER;
      int readCount = client.read(buffer, bufferSize);
      
      // Envia os bytes pela serial e aumenta o contador de bytes lidos
      Serial.write(buffer, readCount);
      Serial.flush();
      
      bytesRead += readCount;
    }


    /////////////// Se a "Serial" possui bytes a serem lidos ///////////////
    available = Serial.available();
    
    if(available) {
      int bufferSize = available < MAX_BUFFER ? available : MAX_BUFFER;
      // Lê os bytes
      Serial.readBytes(buffer, bufferSize);

      // Se for o byte que define a finalização da conexão
      if(buffer[bufferSize - 1] == 127) {
        client.write(buffer, bufferSize-1);
        // Envia o que ainda não tenha sido enviado
        client.flush();
        // Espera um tempo para o cliente receber
        delay(100);
        // Fecha a conexão com o cliente e sai do 'while'
        client.stop();
        break;
      }

      // Envia os bytes para o cliente
      client.write(buffer, bufferSize);
    }
  } // while(client.connected())
}
