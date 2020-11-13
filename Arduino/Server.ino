//////////////////////////// ------ ////////////////////////////



// Faz a leitura da primeira linha da requisição
char* readRequest()
{
  bool currentLineIsBlank = true;
  char request[50];
  int i = 0;
  bool firstLine = true;

  while(true) {
    while(!Serial.available());
    char c = Serial.read();
    
    // Apenas a primeira linha da requisição nos interessa
    if(firstLine) {
      request[i] = c;
      i++;
    }
    
    if(c == '\n') {
        // A última linha da requisição é um \r\n sozinho, após o \r\n da linha anterior
        if(currentLineIsBlank){
            // Se chegou aqui é porque a requisição foi lida por completo
            break;
        }
        currentLineIsBlank = true;
        firstLine = false;
    }
    else if(c != '\r') {
        // Se leu qualquer caracter que não for \n e \r significa que a linha não está em branco
        currentLineIsBlank = false;
    }
  }
  
  return request;
}



// Envia o HTML para o cliente
void sendResponse()
{
    // Envia o cabeçalho HTTP
    Serial.print(
      "HTTP/1.0 200 OK\r\n"
      "Content-Type: text/html; charset=UTF-8\r\n"
      "Connection: close\r\n"
      "\r\n"
    );

    Serial.println("<!DOCTYPE HTML>");
    Serial.println("<html>");
    head();  //Envia o cabeçalho do HTML
    body();  //Envia o corpo do HTML
    Serial.println("</html>");
}


// Envia o CSS para modificar a aparência da página
void head()
{
  Serial.println(F(
    "<head>"
    "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
    "<style>"
        "body{"
            "text-align: center;"
            "font-family: sans-serif;"
            "font-size: 14px;"
        "}"
        "p{"
            "color:#555;"
            "font-size: 12px;"
        "}"
        ".button{"
            "outline: none;"
            "display: block;"
            "border: 1px solid #555;"
            "border-radius:18px;"
            "width: 150px;"
            "height: 30px;"
            "margin: 10px;"
            "margin-left: auto;"
            "margin-right: auto;"
            "cursor: pointer;"
        "}"
        ".button_off{"
            "background-color:#FFF;"
            "color: #555;"
        "}"
        ".button_on{"
            "background-color:#2C5;"
            "color: #fff;"
        "}"
    "</style>"
  "</head>"
  ));
}


// Cria o body e os botões
void body()
{
    Serial.println("<body>");

    String buttons = "";

    // Cria um botão para cada pino que possui um relê
    for(int i = 0; i < PINS_COUNT; i++) {
        buttons.concat(button(i));
    }
           
    Serial.println(buttons);
    Serial.println("</body>");
}


// Cria um botão com a aparência e ação correspondente ao estado atual do relê
String button(int number)
{
    String label = String(number + 1);
    
    String className = "button button_on";
    //className += pinsStatus[number] == HIGH ? "button_on" : "button_off";
    
    //String action = pinsStatus[number] == HIGH ? "off" : "on";
    String action = "on";
    
    return "<button class=\"" + className + "\"onclick=\"location.href='?" + action + "=" + label + "'\">" + "DESBLOQUEAR" + "</button>";
    //return "<button class=\"" + className + "\"onclick=\"location.href='?" + action + "=" + label + "'\">" + label + "</button>";
}


// Retorna a ação que o cliente deseja executar (on off)
String getAction(char *request)
{
  return getStringBetween(request, '?', '=');
}


// Retorna o valor (numero do relê) que a ação será executada
String getValue(char *request)
{
  return getStringBetween(request, '=', ' ');
}


// Retorna a string que fica entre o primeiro caractere 'start' e o primeiro caractere 'end'
String getStringBetween(char* input, char start, char end)
{
  String str = "";
  
  // retorna o endereço de memória do caractere 'start'
  char* c = strchr(input, start);

  // Se não achou o caractere
  if(c == NULL) {
      return "";
  }

  // Vai para o próximo caractere
  c++;

  // Enquanto não chegar ao caractere 'end' ou ao final da string
  while(*c != end && *c!='\0') {
      str += *c;
      c++;
  }

  return str;
}


// Executada a ação junto ao valor (número do relê)
void execute(String action, String value)
{
  // Se é uma das duas ações que esperamos
  if(action == "on" || action == "off") {
    // Os relês são numerados a partir do 1, max o array começa do 0
    // então tiramos 1
    int index = value.toInt() - 1;
    
    // O número do pino será o índice mais o número do pino onde os relês
    // começam. Os relês devem estar em sequência a partir do pino inicial (FIRST_PIN)
    int pinNumber = FIRST_PIN + index;
    
    int status = action == "on" ? HIGH : LOW;
    
    digitalWrite(pinNumber, status);
    pinsStatus[index] = status;
  }
}
