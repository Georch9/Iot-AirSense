/*
  AirSense 
*/

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#define MEDICIONES 6
#define TIEMPO_ENTRE_MUESTRAS 50000 //50 segundos
#define TIEMPO_ENTRE_ENVIOS  10000 //10 segundos
#define RO 41763
#define RL 1350
#define SMOKE_D 1
//#define DEBUG
/******************************************          Wifi                        *************************************/
const char* ssid = "iot";
const char* password = "12345678";
/******************************************          Get Urls                    *************************************/
const char* host = "mfufif7r84.execute-api.us-west-2.amazonaws.com";
const int httpsPort = 443;
const String apiKey[7] = {"/Airsense/03/Benzene/", 
                          "/Airsense/03/Toluene/", 
                          "/Airsense/03/Phenol/", 
                          "/Airsense/03/Ammonium/",
                          "/Airsense/03/Carbon_monoxide/",
                          "/Airsense/03/Carbon_dioxide/",
                          "/Airsense/notification/"};

// Usa un navegador para obener el certificado 
// Fingerprint 
const char* fingerprint = "‎C9 D2 6D 9A ED D7 D8 EB A6 F7 71 3D 70 5F 95 BA 97 B9 E6 DD";

/******************************************          Variables globales          *************************************/
int adc_MQ;
float voltaje;
float Rs ;
String url;
int n;
int smoke_detected;
int extra_data;
int notify;
int no_net;
/******************************************          Funciones                   *************************************/
double analog_benceno(float Rs);
double analog_tolueno(float Rs);
double analog_fenol(float Rs);
double analog_amonio(float Rs);
double analog_monoxDeCarbono(float Rs);
double analog_dioxidoDeCarbono(float Rs);
double humoDetectado(float Rs);
double (*analog_lectur[7])(float) = {analog_benceno, analog_tolueno, analog_fenol, analog_amonio, analog_monoxDeCarbono, analog_dioxidoDeCarbono, humoDetectado};


void setup() {
  pinMode(D0, INPUT_PULLUP);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT); //rojo
  pinMode(D4, OUTPUT); //verde
  Serial.begin(115200);
  Serial.println();
  //Para debug, generación de valores random
  #ifdef DEBUG 
    randomSeed(analogRead(A0));
  #endif
  /*Conexión con WIFI*/
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    /*Indicador de conexión WIFI, encciende led rojo*/
    digitalWrite(D3, HIGH);
    digitalWrite(D4, LOW); 
    Serial.print(".");
  }
  /*Configuración periféricos*/
  /*Indicador de conexión WIFI, encciende led verde, conexión activa*/
  digitalWrite(D3, LOW);
  digitalWrite(D4, HIGH); 
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {

  #ifdef DEBUG  
    adc_MQ = random(1023);
  #else 
    /*lectura sensor*/
    adc_MQ  = analogRead(A0);
     /*Se detectó humo*/
    smoke_detected = digitalRead(D0);
     /*Se notifica solo una vez al usuario*/
    if(!smoke_detected)
    {
      if(notify)
      {
       extra_data = 0;
      }else{
       extra_data = 1;
       notify = 1; 
      }
    }else
    {
      extra_data = 0;
      notify = 0;
    }
  #endif
    /*Cálculo del valor reistivo del sensor*/ 
    voltaje = (adc_MQ * (3.3 / 1023.0))*2; //Convertimos la lectura en un valor de voltaje
    Rs = 1000.0*((1023.0 - adc_MQ)/(adc_MQ));
    Serial.println("Rs: ");
    Serial.println(Rs);

   /*Envío de datos a la nube*/
  for(n = 1; n <= MEDICIONES + extra_data; n++)
  {
    /*Enciende filtros si se detecta humo durante el envío*/
    smoke_detected = digitalRead(D0);
    if(!smoke_detected)
    {
       digitalWrite(D2, HIGH);  
    }else
    {
      digitalWrite(D2, LOW);
    }
    // Crea la conexión con la página
    WiFiClientSecure client;
    Serial.print("connecting to ");
    Serial.println(host);
    if (!client.connect(host, httpsPort)) 
    {
      Serial.println("connection failed");
      /*Indicador de conexión de internet*/
      digitalWrite(D3, HIGH);
      digitalWrite(D4, LOW);
      if(no_net == 0 && notify == 1)
      {
        no_net = 1;
      }       
      return;
    }
     /*Indicador de conexión de internet*/
    digitalWrite(D3, LOW);
    digitalWrite(D4, HIGH);
    /*Validación de credenciales*/
    if (client.verify(fingerprint, host)) {
      Serial.println("certificate matches");
    } else {
      Serial.println("certificate doesn't match");
    }
    /*Construcción del link para hacer GET*/
    url = "";
    if(no_net == 1 && extra_data == 1)
    {
      no_net = 0;  
    }
    /*Obtención del valor del contaminante a maandar o notificación si es el caso*/
    url += apiKey[n-1 + no_net] + String((*analog_lectur[n-1 + no_net])(Rs));
    Serial.print("requesting URL: ");
    Serial.println(url);
    /*Método GET del dato leído*/ 
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "User-Agent: BuildFailureDetectorESP8266\r\n" +
                 "Connection: close\r\n\r\n");
  
    Serial.println("request sent");
    /*Termina conexión con el servidor*/
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") {
        Serial.println("headers received");
        break;
      }
    }
    /*Valores recibidos de la página*/
    String line = client.readStringUntil('\n');
    if (line.startsWith("{\"state\":\"success\"")) {
      Serial.println("esp8266/Arduino CI successfull!");
    } else {
      Serial.println("esp8266/Arduino CI has failed");
    }
    Serial.println("reply was:");
    Serial.println("==========");
    Serial.println(line);
    Serial.println("==========");
    Serial.println("closing connection");
    Serial.printf("\nMemoria libre en el ESP8266: %d Bytes\n\n",ESP.getFreeHeap());
    delay(TIEMPO_ENTRE_ENVIOS);
  }
    no_net = 0;
    delay(TIEMPO_ENTRE_MUESTRAS);
}

/*Función para lectura del benceno*/
double analog_benceno(float Rs)
{
  double benceno = 37.89*pow(Rs/2677.6997, -3.165); //Calculamos la concentración del metano
  return benceno;
}
/*Función para lectura del tolueno*/
double analog_tolueno(float Rs)
{
  double tolueno = 47.36*pow(Rs/2749.7649, -3.292); //Calculamos la concentración del propano
  return tolueno;
}
/*Función para lectura del fenol*/
double analog_fenol(float Rs)
{
  double fenol = 79.77*pow(Rs/5664.9976, -3.005);
  return fenol;
}
/*Función para lectura del amonio*/
double analog_amonio(float Rs)
{
  double amonio = 101*pow(Rs/462.2764, -2.495); //Calculamos la concentración del amoniaco
  return amonio; 
}
/*Función para lectura del monoxido de carbono*/
double analog_monoxDeCarbono(float Rs)
{
  double monoxDeCarbono = 763.7*pow(Rs/3430.9353, -4.541);
  return monoxDeCarbono;
}
/*Función para lectura del dióxido de carbono*/
double analog_dioxidoDeCarbono(float Rs)
{
  double dioxidoDeCarbono = 110.8*pow(Rs/39335.8215, -2.729); 
  return dioxidoDeCarbono;
}
/*Función para lectura de humo detectado*/
double humoDetectado(float Rs)
{ 
  return SMOKE_D;
}


