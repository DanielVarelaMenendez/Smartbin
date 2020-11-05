//Codigo basado en el proyecto de Rui Santos
/*********
Rui Santos
Complete project details at https://RandomNerdTutorials.com/esp32-lora-sensor-webserver/
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files.
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
*********/
#include <Arduino.h>
//Librerias para la conexión WIFI
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
//Librerias para la gestión del LoRa
#include <SPI.h>
#include <LoRa.h>
//Librerias para la visualizacion el la OLED
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// Librerias para el servidor NTP
#include <NTPClient.h>
#include <WiFiUdp.h>
//definimos los pines para el modulo LoRa que incorpora el TTGO
#define SCK 5 //señal de reloj
#define MISO 19 //(Master Input Slave Output) señal de entrada
#define MOSI 27 //(Master Output Slave Input) señal de salida
#define SS 18 //(Slave Select) seleciona el esclavo al que se envían
#define RST 14 //boton de reset
#define DIO0 26 //
#define BAND 866E6 //definimos la banda de transmisión en europa es la 866E6
//Definimos los pines de la OLED
#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16
#define SCREEN_WIDTH 128 // OLED ancho de la pantalla en pixeles
#define SCREEN_HEIGHT 64 // OLED alto de la pantalla en pixeles
//definimos los parametros para la conexión WIFI
const char* ssid = "Yo";
const char* contrasena = "aprueboTFG";
// Definimos el cliente NTP para recibir la fecha y la hora
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Variables para guardar el dia y la fecha
String formattedDate;
String dia;
String hora;
String timestamp;
// Initialize variables to get and save LoRa data
int rssi;
String loRaMessage; //en esta variable se guarda el mensaje recibido
String nivel[5] ; //vector de string donde se guardaran y actualizaran los valores
de los niveles de cada deposito
int material; //se guarda el numero del material (la posicion el vector nivel) que se esta clasificando 0->Plastico ; 1->Papel/Carton ; 2->Metal ;3-> Vidrio ;4- >Organico
String readingID; //ID recibido
// Se crea un servidor en el puerto 80
AsyncWebServer server(80);
//se crea un objeto display de tipo Adafruit_SSD1306 con las caracteristicas de la pantalla
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);
// Esta funcion inserta variable 'var' de tipo String (con los valores de cada deposito) en los placeholder del HTML
String processor(const String& var){
    if(var == "PLASTICO"){
        return nivel[0];//si el placeholder es PLASTICO devuelve el nivel del deposito del plastico 'nivel[0]'
    }
    else if(var == "CARTON/PAPEL"){
        return nivel[1];
    }
    else if(var == "METAL"){
        return nivel[2];
    }
    else if(var == "VIDRIO"){
        return nivel[3];
    }
    else if(var == "ORGANICO"){
        return nivel[4];
    }
    else if(var == "TIMESTAMP"){
        return timestamp;
    }
    else if (var == "RRSI"){
    return String(rssi);
    }
    return String();
    }
    //Inicializa la pantalla OLED

void startOLED(){
    //resetea el OLED
    pinMode(OLED_RST, OUTPUT);
    digitalWrite(OLED_RST, LOW);
    delay(20);
    digitalWrite(OLED_RST, HIGH);
    //incializa el OLED
    Wire.begin(OLED_SDA, OLED_SCL);
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) { // Address 0x3C for128x32
        Serial.println(F("SSD1306 allocation failed"));
        for(;;); // Don't proceed, loop forever
    }
    display.clearDisplay(); //limpia la pantalla
    display.setTextColor(WHITE); //establece como blanco el color de escritura
    display.setTextSize(1); //establece el tamaño del texto en una fila
    display.setCursor(0,0); //pone el cursor en el origen (parte superior izquierda)
    display.print("Receptor LoRa");//una vez inicializado escribe en la pantalla este mensaje
}

//Inicializa el modulo LoRa
void startLoRA(){
    int counter;
    SPI.begin(SCK, MISO, MOSI, SS);//establece los pines correspondientes
    LoRa.setPins(SS, RST, DIO0); //establece el modulo LoRa
    //hace una prueba para comprobar si se establece conexión en un tiempo determinado
    while (!LoRa.begin(BAND) && counter < 10) { //envía hasta 10 paquetes mientras no
        haya establecido conexion
        Serial.print(".");
        counter++;
        delay(500);
    }
    if (counter == 10) {//si se han enviado los 10 paquetes quiere decir que se agoto el tiempo de conexión
        Serial.println("Error: fallo de inicializacion"); //salta un error
    }
    Serial.println("Inicializa LoRa OK!");//si se establecio conexión en la pantalla se muestra este mensaje
    display.setCursor(0,10);//desplaza el curso 10 columnas a la derecha
    display.clearDisplay();//limpia la pantalla
    display.display();//lo muestra
    delay(2000);
}

//funcion para la conexión wifi
void connectWiFi(){
    Serial.print("Conectandose a "); //indica a que WIFI se conecta
    Serial.println(ssid);
    WiFi.begin(ssid, contrasena);//inicia el intento de conexión al wifi con los datos proporcionados
    delay(2000);
    while (WiFi.status() != WL_CONNECTED) {//realiza prueba de conexión al wifi
        delay(500);
        Serial.print(".");
    }

    // Print local IP address and start web server
    Serial.println("");
    Serial.println("WiFi conectado.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());//muestra el IP que se le asignó al ESP32 a traves del monitor serial
    display.setCursor(0,20); //mueve el cursor a la derecha
    display.print("Accede a la pagina web: ");
    display.setCursor(0,30);
    display.print(WiFi.localIP());//muestra el IP que se le asignó al ESP32 para que el usuario lo introduzca en la APP
    display.display();
}
// Recibe el paquete del emisor LoRa
void getLoRaData() {
    Serial.print("Paquete LoRa recibido: "); //indica que ha recibido un paquete
    // lee el paquete
    while (LoRa.available()) {
        String LoRaData = LoRa.readString();
        Serial.print(LoRaData);
        //extrae la posición del string LoRaData en el que se encuentra cada uno de los marcadores que dividen los diferentes datos dentro del mensaje
        int pos1 = LoRaData.indexOf('/');
        int pos2 = LoRaData.indexOf('&');
        //extrae los datos del mensaje que estan divididos por los marcadores anteriormente explicados
        readingID = LoRaData.substring(0, pos1); //lee desde el origen a la posición 1 y lo guarda en la variable readingID
        material = LoRaData.substring(pos1 +1, pos2).toInt(); //recibe el residuo que se esta clasificando (su posicion en el vector nivel)
        nivel[material] = LoRaData.substring(pos2+1, LoRaData.length() );
    }
    //muestra el RSSI (received signal strength indicator)
    rssi = LoRa.packetRssi();
    Serial.print(" with RSSI ");
    Serial.println(rssi);
}

// funcion para obtener la fecha y la hora
void getTimeStamp() {
    while(!timeClient.update()) {
        timeClient.forceUpdate();
    }
    formattedDate = timeClient.getFormattedDate();//formatea la fecha y hora
    Serial.println(formattedDate); //lo muestra
    // Extrae la fecha
    int splitT = formattedDate.indexOf("T"); //dividimos el string hasta la letra T
    dia = formattedDate.substring(0, splitT); //guarda el dia
    Serial.println(dia);

    // Extrae el tiempo
    hora = formattedDate.substring(splitT+1, formattedDate.length()-1);
    Serial.println(hora);
    timestamp = dia + " " + hora;
}

void setup() {
    // Initialize Serial Monitor
    Serial.begin(115200); //inicia la comunicacion serial a una tasa de 115200 baudios
    startOLED(); //inicializa el OLED
    startLoRA(); //inicializa el LoRa
    connectWiFi(); //conecta a la red WIFI
    if(!SPIFFS.begin()){ //inicia el SPIFFS donde se almacena el html y la imagen
        Serial.println("An Error has occurred while mounting SPIFFS"); //si hay
        cualquier error se indica
        return;
    }
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){//en la pagina principal
    request->send(SPIFFS, "/index.html", String(), false, processor); // se muestra el index.html y se llama a la función processor para obtener los datos
    });
    server.on("/nivel_carton", HTTP_GET, [](AsyncWebServerRequest *request){ //en el nivel_carton del HTML
    request->send_P(200, "text/plain", nivel[1].c_str()); //se introducel el valor del deposito del carton
    });
    server.on("/nivel_metal", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", nivel[2].c_str());
    });
    server.on("/nivel_organico", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", nivel[4].c_str());
    });
    server.on("/nivel_plastico", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", nivel[0].c_str());
    });
    server.on("/nivel_vidrio", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", nivel[3].c_str());
    });
    server.on("/timestamp", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", timestamp.c_str());
    });
    server.on("/rssi", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(rssi).c_str());
    });
    server.on("/smartbin", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/smartbin.jpg", "image/jpg");
    });
    server.begin(); //inicia el servidor web
    timeClient.begin();// Inicia NTPClient para obtener el tiempo
    timeClient.setTimeOffset(3600); //para ajustar el tiempo a la zona horaria de  españa (GMT +1) se aplica un offset de 3600
}

void loop() {
    //analiza si hay paquetes por recibir
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
        getLoRaData();//obtiene los datos del mensaje
        getTimeStamp(); //obtiene la fecha y la hora
    }
}