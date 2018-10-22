#include <ESP8266WiFi.h> 
#define MEDICIONES 6
#define TIEMPO_ENTRE_MUESTRAS 50000 //50 segundos
#define TIEMPO_ENTRE_ENVIOS  10000 //10 segundos
//#define DEBUG
/******************************************          Wifi                        *************************************/
const char* ssid = "iot"; // Rellena con el nombre de tu red WiFi
const char* password = "12345678"; // Rellena con la contraseña de tu red WiFi

/******************************************          Get Urls                    *************************************/
const char* host = "api.thingspeak.com";
//const char* host = "mfufif7r84.execute-api.us-west-2.amazonaws.com";
const char* apiKey = "/update?api_key=BPHU8TA1BBFR0N8Q&field"; 
//const char* apiKey = "/Airsense/01/NH3/";
//https://mfufif7r84.execute-api.us-west-2.amazonaws.com/Airsense/userID/type/value

/******************************************          Variables globales          *************************************/
int adc_MQ;
float voltaje;
float Rs ;
String url;
int n;
/******************************************          Funciones                   *************************************/
double analog_benceno(float Rs);
double analog_tolueno(float Rs);
double analog_fenol(float Rs);
double analog_amonio(float Rs);
double analog_monoxDeCarbono(float Rs);
double analog_dioxidoDeCarbono(float Rs);
double (*analog_lectur[6])(float) = {analog_benceno, analog_tolueno, analog_fenol, analog_amonio, analog_monoxDeCarbono, analog_dioxidoDeCarbono};

/******************************************          Configuracion inicial      *************************************/
void setup() 
{
  Serial.begin(115200);
  delay(10);

  // Conectamos a la red WiFi
  #ifdef DEBUG 
    randomSeed(analogRead(A0));
  #endif
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
/******************************************          Main                    *************************************/
void loop() 
{

  Serial.print("connecting to ");
  Serial.println(host);
 
  // Creamos el cliente
  WiFiClient client;
  const int httpPort = 80; // Puerto HTTP

 
  while(1)
  {


  #ifdef DEBUG  
    adc_MQ = random(1023);
  #else 
    adc_MQ  = analogRead(A0); //Leemos la salida analógica del MQ
  #endif
    voltaje = adc_MQ * (5.0 / 1023.0); //Convertimos la lectura en un valor de voltaje
    Rs = 22000 * ((5 - voltaje)/voltaje); //Calculamos Rs con un RL de 1k


    
    for(n = 1; n <= MEDICIONES; n++)
    {
      if (!client.connect(host, httpPort)) 
      {
        // ¿hay algún error al conectar?
        Serial.println("Ha fallado la conexión");
        return;
      }
      Serial.println("Conexión establecida");

      url = "";
      url += apiKey;
      url += String(n) + "=";
      url += String((*analog_lectur[n -1])(Rs));
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
      delay(TIEMPO_ENTRE_ENVIOS);
    }
    delay(TIEMPO_ENTRE_MUESTRAS);
  }
}

double analog_benceno(float Rs)
{
  double benceno = 37.89*pow(Rs/5463, -3.165); //Calculamos la concentración del metano
  return benceno;
}

double analog_tolueno(float Rs)
{
  double tolueno = 47.36*pow(Rs/5463, -3.292); //Calculamos la concentración del propano
  return tolueno;
}

double analog_fenol(float Rs)
{
  double fenol = 79.77*pow(Rs/5463, -3.005);
  return fenol;
}

double analog_amonio(float Rs)
{
  double amonio = 101*pow(Rs/5463, -2.495); //Calculamos la concentración del amoniaco
  return amonio; 
}

double analog_monoxDeCarbono(float Rs)
{
  double monoxDeCarbono = 763.7*pow(Rs/5463, -4.541);
  return monoxDeCarbono;
}

double analog_dioxidoDeCarbono(float Rs)
{
  double dioxidoDeCarbono = 110.8*pow(Rs/5463, -2.729); 
  return dioxidoDeCarbono;
}


