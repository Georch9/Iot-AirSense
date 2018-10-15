// Esta es la librería para utilizar las funciones de red del ESP8266
#include <ESP8266WiFi.h> 

const char* ssid = "iot"; // Rellena con el nombre de tu red WiFi
const char* password = "12345678"; // Rellena con la contraseña de tu red WiFi

//const char* host = "api.thingspeak.com";
const char* host = "mfufif7r84.execute-api.us-west-2.amazonaws.com";
//const char* apiKey = "/update?api_key=KPIV8WQLT57PSQ0C&field1="; 
const char* apiKey = "/Airsense/01/NH3/";
//https://mfufif7r84.execute-api.us-west-2.amazonaws.com/Airsense/userID/type/value

//Funciones
double getCO2();

void setup() 
{
  Serial.begin(115200);
  delay(10);

  // Conectamos a la red WiFi

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
 
  /* Configuramos el ESP8266 como cliente WiFi. Si no lo hacemos 
     se configurará como cliente y punto de acceso al mismo tiempo */
  WiFi.mode(WIFI_STA); // Modo cliente WiFi
  WiFi.begin(ssid, password);

  // Esperamos a que estemos conectados a la red WiFi
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected"); 
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP()); // Mostramos la IP
}

void loop() 
{

  Serial.print("connecting to ");
  Serial.println(host);
 
  // Creamos el cliente
  WiFiClient client;
  const int httpPort = 80; // Puerto HTTP
   // Creamos la URL para la petición
  String url;
 
  while(1)
  {


    if (!client.connect(host, httpPort)) 
    {
      // ¿hay algún error al conectar?
      Serial.println("Ha fallado la conexión");
      return;
    }
    Serial.println("Conexión establecida");
    url = "";
    url += apiKey + String(getCO2());

    Serial.print("URL de la petición: http://");
    Serial.print(host);
    Serial.println(url);
    // Enviamos la petición
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
         "Host: " + host + "\r\n" + 
         "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    while (client.available() == 0) 
    {
      if (millis() - timeout > 50000) 
        {
          Serial.println(">>> Superado el tiempo de espera !");
          client.stop();
          return;
        }
    }

  // Consutar la memoria libre
  // Quedan un poco más de 40 kB
  Serial.printf("\nMemoria libre en el ESP8266: %d Bytes\n\n",ESP.getFreeHeap());
 
  // Leemos la respuesta y la enviamos al monitor serie
  while(client.available())
  {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
 
  Serial.println();
  Serial.println("Cerrando la conexión");
  delay(50000);
}
  while(1){
    delay(0); // Siempre que hay un bucle que pueda durar mucho tiempo
              // hay que llamar a la función delay() para atender a los 
              // procesos de la conexión WiFi. Si no se hace el ESP8266
              // generará un error y se reiniciará a los pocos segundos
  }
}


double getCO2()
{
  int adc_MQ = analogRead(A0); //Lemos la salida analógica  del MQ
  float voltaje = adc_MQ * (5.0 / 1023.0); //Convertimos la lectura en un valor de voltaje
  float Rs=1000*((5-voltaje)/voltaje);  //Calculamos Rs con un RL de 1k
  double alcohol=0.4091*pow(Rs/5463, -1.497); // calculamos la concentración  de alcohol con la ecuación obtenida.
  //-------Enviamos los valores por el puerto serial------------
  Serial.print("adc:");
  Serial.print(adc_MQ);
  Serial.print("    voltaje:");
  Serial.print(voltaje);
  Serial.print("    Rs:");
  Serial.print(Rs);
  Serial.print("    alcohol:");
  Serial.print(alcohol);
  Serial.println("mg/L");  
}


