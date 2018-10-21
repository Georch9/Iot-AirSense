#include <ESP8266WiFi.h> 
#define MEDICIONES 6
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
double getCO2();
double analog_alcohol(float Rs);
double analog_amoniaco(float Rs);
double analog_metano(float Rs);
double analog_propano(float Rs);
double analog_monoxDeCarbono(float Rs);
double analog_hidrogeno(float Rs);
double analog_dioxidoDeCarbono(float Rs);
double (*analog_lectur[6])(float) = {analog_amoniaco, analog_metano, analog_propano, analog_monoxDeCarbono, analog_hidrogeno, analog_dioxidoDeCarbono};

/******************************************          Configuracion inicial      *************************************/
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

    adc_MQ  = analogRead(A0); //Leemos la salida analógica del MQ
    voltaje = adc_MQ * (5.0 / 1023.0); //Convertimos la lectura en un valor de voltaje
    Rs = 1000 * ((5 - voltaje)/voltaje); //Calculamos Rs con un RL de 1k


    
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
      delay(10000);
    }
    delay(50000);
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

double analog_alcohol(float Rs)
{
  double alcohol = 1.108*pow(Rs/5463, -1.41); //Calculamos la concentración del alcohol
  return alcohol; 
}

double analog_amoniaco(float Rs)
{
  double amoniaco = 161.7*pow(Rs/5463, -2.26); //Calculamos la concentración del amoniaco
  return amoniaco; 
}

double analog_metano(float Rs)
{
  double metano = 6922*pow(Rs/5463, -1.91); //Calculamos la concentración del metano
  return metano;
}

double analog_propano(float Rs)
{
  double propano = 2738*pow(Rs/5463, -1.81); //Calculamos la concentración del propano
  return propano;
}

double analog_monoxDeCarbono(float Rs)
{
  double monoxDeCarbono = 233.9*pow(Rs/5463, -1.40);
  return monoxDeCarbono;
}

double analog_hidrogeno(float Rs)
{
  double hidrogeno = 1803*pow(Rs/5463, -0.66);
  return hidrogeno;
}

double analog_dioxidoDeCarbono(float Rs)
{
  double dioxidoDeCarbono = 245*pow(Rs/5463, 2.26); 
  return dioxidoDeCarbono;
}



