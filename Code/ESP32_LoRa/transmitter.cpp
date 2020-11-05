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
//Librerias para la gestión del LoRa
#include <SPI.h>
#include <LoRa.h>
//Librerias para la visualizacion el la OLED
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//se definen los pines para el modulo LoRa que incorpora el TTGO
#define SCK 5 //señal de reloj
#define MISO 19 //(Master Input Slave Output) señal de entrada
#define MOSI 27 //(Master Output Slave Input) señal de salida
#define SS 18 //(Slave Select) seleciona el esclavo al que se envían
#define RST 14 //boton de reset
#define DIO0 26 //
#define BAND 866E6 //se definen la banda de transmisión en europa es la 866E6
//Se definen los pines de la OLED
#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16
#define SCREEN_WIDTH 128 // OLED ancho de la pantalla en pixeles
#define SCREEN_HEIGHT 64 // OLED alto de la pantalla en pixeles
int readingID = 0; //variable para el ID del mensaje
int counter = 0; //contador
String LoRaMessage = ""; //en esta string se irá crando el mensaje para enviar
// vector donde se guardarán los niveles de los 5 depositos
float nivel[5]={0,0,0,0,0} ; //0->Plastico ; 1->Papel/Carton ; 2->Metal ;3-> Vidrio ;4->Organico
int material ;
const int EchoPin = 13; // el pin ECHO del sensor de ultrasonidos se conecta al pin 13
const int TriggerPin = 12; //el trigger se conecta al pin 12 
//se crea un objeto display de tipo Adafruit_SSD1306 con las caracteristicas de lapantalla
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

//Inicializa la pantalla OLED
void startOLED(){
//resetea el OLED
    pinMode(OLED_RST, OUTPUT);
    digitalWrite(OLED_RST, LOW);
    delay(20);
    digitalWrite(OLED_RST, HIGH);
    //incializa el OLED
    Wire.begin(OLED_SDA, OLED_SCL);
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) { // Address 0x3C for
        128x32
        Serial.println(F("SSD1306 allocation failed"));
        for(;;); // Don't proceed, loop forever
    }
    display.clearDisplay(); //limpia la pantalla
    display.setTextColor(WHITE); //establece como blanco el color de escritura
    display.setTextSize(1); //establece el tamaño del texto en una fila
    display.setCursor(0,0); //pone el cursor en el origen (parte superior izquierda)
    display.print("Emisor LoRa");
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
//función que devuelve la distancia en cm proporcionada por el sensor de ultrasonidos
int medir (int TriggerPin, int EchoPin) {
    long duracion, distancia_Cm;
    digitalWrite(TriggerPin, LOW); //para generar un pulso limpio pone a LOW 4us
    delayMicroseconds(4);
    digitalWrite(TriggerPin, HIGH); //genera Trigger (disparo) de 10us
    delayMicroseconds(10);
    digitalWrite(TriggerPin, LOW);

    duracion = pulseIn(EchoPin, HIGH); //mide el tiempo entre pulsos, en
    microsegundos
    distancia_Cm = duracion * 10 / 292/ 2; //convierte a distancia, en cm

    return distancia_Cm;
}
// funcion que según el material que se este procesando guarda el nivel medido por el ultrasonidos en su variable correspondiente
void getReadings(int material){
    int cm = medir (TriggerPin, EchoPin); //obtiene la distancia en cm
    Serial.print("Distancia: ");
    Serial.println(cm);
    float nivel_medido = (-2.56)*cm+ (112.82); //nivel maximo 100 % cuando la
    distancia es 5 cm ; nivel minimo 0% cuando distancia es 44 cm
    nivel[material]= nivel_medido;//el nivel medido se guarda en la variable del
    nivel que se este clasificando
    Serial.print("Nivel: ");
    Serial.println(nivel_medido);
}
//función que crea el mensaje y lo envia mediante LoRa tambien visualiza la info en la pantalla
void sendReadings(int material) {
    //se crea el mensaje intercalando los datos con marcadores
    LoRaMessage = String(readingID) + "/" + String(material) + '&' +
    String(nivel[material]);
    // se envía el paquete
    LoRa.beginPacket();
    LoRa.print(LoRaMessage);
    LoRa.endPacket();
    display.clearDisplay(); //se limpia la OLED
    display.setCursor(0,0); //se desplaza el cursor al origen
    display.setTextSize(1); //se establece el tamaño de letra en 1 fila y 1 columna
    display.print("Paquete " + String(readingID) + " enviado"); //se escribe
    display.setCursor(0,10);
    display.print("Plastico:"); //se escribe
    display.setCursor(72,10); //se desplaza el cursor 72 pixeles a la derecha
    display.print(nivel[0] ); //se escribe el nivel de plastico
    display.setCursor(0,20); //se desplaza el cursor 10 pixeles más abajo
    display.print("Carton:");
    display.setCursor(54,20);
    display.print(nivel[1]);
    display.setCursor(0,30);
    display.print("Metal:");
    display.setCursor(54,30);
    display.print(nivel[2]);
    display.setCursor(0,40);
    display.print("Vidrio:");
    display.setCursor(54,40);
    display.print(nivel[3]);
    display.setCursor(0,50);
    display.print("Organico:");
    display.setCursor(54,50);
    display.print(nivel[4]);
    display.display();
    Serial.print("Sending packet: ");
    Serial.println(readingID); //se muestra el ID tambien a traves del monitor serial
    readingID++;
}

void setup() {
    Serial.begin(9600); //inicializa la comunicacion serial a 9600 baudios
    while (!Serial) {
        ; // espera hasta que la comunicación serial se establezca
    }
    pinMode(TriggerPin, OUTPUT); //se configura el pin del trigger como salida
    pinMode(EchoPin, INPUT); //se configura el pin del echo como entrada
    startOLED(); //se llama a la función para inicializar la OLED
    startLoRA(); //se llama a la función para inicializar EL LORA
}
void loop() {
    char buffer[2]; //se crea un buffer de caracteres de 2 posiciones
    if (Serial.available() > 0) { //si el serial esta activo
        int size = Serial.readBytesUntil('\n', buffer, 2); //lee los caracteres que
        entran por el serial hasta la siguiente linea (/n) y los guarda en el buffer (son 2
        bytes)
        if (buffer[0] == 'U') { //si recibe una U ejecuta el proceso
            material = int(buffer[1])-48; //convierte el segundo char recibido a int
            getReadings(material); //lee el dato del ultrasonidos recibido,lo guarda en la
            variable material recibido
            sendReadings(material); //envia los datos
            Serial.write('F'); //envia por el serial una F para indicarle al arduino que ha
            finalizado
            delay(300);
        }
    }
}