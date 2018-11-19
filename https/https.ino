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
const String apiKey[7] = {"/Airsense/00/Benzene/", 
                          "/Airsense/00/Toluene/", 
                          "/Airsense/00/Phenol/", 
                          "/Airsense/00/Ammonium/",
                          "/Airsense/00/Carbon_monoxide/",
                          "/Airsense/00/Carbon_dioxide/",
                          "/Airsense/notification/"};

// Use un navegador para obener el certificado 
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
  Serial.begin(115200);
  Serial.println();
  #ifdef DEBUG 
    randomSeed(analogRead(A0));
  #endif
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  pinMode(D0, INPUT_PULLUP);
  pinMode(D2, OUTPUT);
}

void loop() {

  #ifdef DEBUG  
    adc_MQ = random(1023);
  #else 
    adc_MQ  = analogRead(A0); //Leemos la salida analógica del MQ
    smoke_detected = digitalRead(D0);
    if(!smoke_detected)
    {
      if(notify)
      {
       extra_data = 0;
      }else{
       digitalWrite(D2, HIGH);  
       extra_data = 1;
       notify = 1; 
      }

    }else
    {
      digitalWrite(D2, LOW);
      extra_data = 0;
      notify = 0;
    }
  #endif
 //Rs = ((1024.0 * RL) / adc_MQ) - RL;
    voltaje = (adc_MQ * (3.3 / 1023.0))*2; //Convertimos la lectura en un valor de voltaje
    //Rs = RL * ((5 - voltaje)/voltaje); //Calculamos Rs con un RL de 1k
    Rs = (1000.0*adc_MQ)/(1023.0 - adc_MQ);
    Serial.println("Rs: ");
    Serial.println(Rs);
  
  for(n = 1; n <= MEDICIONES + extra_data; n++)
  {
    // Use WiFiClientSecure class to create TLS connection
    WiFiClientSecure client;
    Serial.print("connecting to ");
    Serial.println(host);
    if (!client.connect(host, httpsPort)) 
    {
      Serial.println("connection failed");
      return;
    }
  
    if (client.verify(fingerprint, host)) {
      Serial.println("certificate matches");
    } else {
      Serial.println("certificate doesn't match");
    }
    
    url = "";
    url += apiKey[n-1] + String((*analog_lectur[n-1])(Rs));
    Serial.print("requesting URL: ");
    Serial.println(url);
  
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "User-Agent: BuildFailureDetectorESP8266\r\n" +
                 "Connection: close\r\n\r\n");
  
    Serial.println("request sent");
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") {
        Serial.println("headers received");
        break;
      }
    }
    
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
    delay(TIEMPO_ENTRE_MUESTRAS);
}

double analog_benceno(float Rs)
{
  double benceno = 37.89*pow(Rs/4.6646, -3.165); //Calculamos la concentración del metano
  return benceno;
}

double analog_tolueno(float Rs)
{
  double tolueno = 47.36*pow(Rs/4.7901, -3.292); //Calculamos la concentración del propano
  return tolueno;
}

double analog_fenol(float Rs)
{
  double fenol = 79.77*pow(Rs/9.8685, -3.005);
  return fenol;
}

double analog_amonio(float Rs)
{
  double amonio = 101*pow(Rs/0.8053, -2.495); //Calculamos la concentración del amoniaco
  return amonio; 
}

double analog_monoxDeCarbono(float Rs)
{
  double monoxDeCarbono = 763.7*pow(Rs/5.9767, -4.541);
  return monoxDeCarbono;
}

double analog_dioxidoDeCarbono(float Rs)
{
  double dioxidoDeCarbono = 110.8*pow(Rs/68.5236, -2.729); 
  return dioxidoDeCarbono;
}
double humoDetectado(float Rs)
{ 
  return SMOKE_D;
}


