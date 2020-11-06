#include <Arduino.h>
//Pines del motor del selector NEMA17
int pasos = 12; //definimos como «entero» pin digital 12 para dar los pasos al servo
int direccion = 11; //definimos como «entero» pin digital 11 para dar el sentido de giro
int reset = 10;
//Pines de los motores de las compuertas 28BY J-48
int Motores[2][4]={{22,24,26,28} , // Fila 1--> Motor de la puerta 1 (4 pines) ;
                    {5,4,3,2} }; // Fila 2--> Motor de la puerta 2 (4 pines) ;
#define vel_sel 40000
#define vel_comp 600
int FC[3] ={52,50,48} ; //finales de carrera 0--> SELECTOR 1-->PUERTA 1 2--> PUERTA 2
#define L_AZUL 36
#define L_AMAR 40
#define L_ROJO 42
#define L_VERDE 38
#define L_BLANCO 34
int posic_actual[3];//VECTOR DE POSICIONES ACTUALES 0->SELECTOR 1->PUERTA 1 2->PUERTA 2
int posic_plastico=560;
int posic_vidrio=420;
int posic_carton=280;
int posic_metal=140;
int posic_organico=0; //=700
int Steps[3];
char entrada;
boolean Direction[3] ;
int NormalPaso [4][4] =
{
    {1, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 1},
    {1, 0, 0, 1}
};
int Completopaso [4][4] =
{
    {1, 0, 0, 0},
    {0, 1, 0, 0},
    {0, 0, 1, 0},
    {0, 0, 0, 1}
};
int MedioPaso [ 8 ][ 4 ] =
    { {1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {1, 0, 0, 1}
};
void setup() {
    // start serial port at 9600 bps:
    Serial.begin(9600);
    pinMode(pasos, OUTPUT); //definimos pasos como salida digital
    pinMode(direccion, OUTPUT); //definimos direccion como salida digital
    pinMode(reset, OUTPUT); //definimos reset como salida digital
    for (int fil=0;fil<2;fil++){
        for (int col=0;col<5;col++){
            pinMode(Motores[fil][col], OUTPUT);
        }
    }
    pinMode(L_AZUL, OUTPUT);
    pinMode(L_AMAR, OUTPUT);
    pinMode(L_ROJO, OUTPUT);
    pinMode(L_VERDE, OUTPUT);
    pinMode(L_BLANCO, OUTPUT);
    pinMode(FC[0], INPUT);
    pinMode(FC[1], INPUT);
    pinMode(FC[2], INPUT);
    while (!Serial) {
        ; // wait for serial port to connect.
    }
}
void SetDirection(int x)
{
    if(Direction[x])
        Steps[x]++;
    else {
        Steps[x]--;
    }
    Steps[x] = ( Steps[x] + 4 ) % 4 ;
}

void stepper(int fil ) //Avanza un paso
{
    digitalWrite(Motores[fil][0], NormalPaso[Steps[fil]][ 0] );
    digitalWrite(Motores[fil][1], NormalPaso[Steps[fil]][ 1] );
    digitalWrite(Motores[fil][2], NormalPaso[Steps[fil]][ 2] );
    digitalWrite(Motores[fil][3], NormalPaso[Steps[fil]][ 3] );
    SetDirection(fil);
}

void avanza (int x){
    Direction[x]=true;
    posic_actual[x+1]++;
    stepper(x) ; // Avanza un paso
    delayMicroseconds (vel_comp) ;
}
void retrocede (int x){
    Direction[x]=false;
    posic_actual[x+1]--;
    stepper(x) ; // retrocede un paso
    delayMicroseconds (vel_comp) ;
}
void reset_nema(){
    digitalWrite(reset, LOW); //Mientras reset este en LOW, el motor permanece apagado
    delay(2000); //Retardo en la instruccion
    digitalWrite(reset, HIGH); //Cuando reset se encuentre en HIGH el motor arranca
}
void homingsel(){
    reset_nema();
    digitalWrite(direccion, HIGH); //mandamos direccion al servo
    while ( digitalRead(FC[0])==LOW ) //normalmente cerrado
    {
        digitalWrite(pasos, HIGH); // ponemos a high «pasos»
        digitalWrite(pasos, LOW); // ponemos a low «pasos»
        delayMicroseconds(vel_sel); // leemos la referencia de velocidad
    }
    digitalWrite(reset, LOW); //Mientras reset este en LOW, el motor
    permanece apagado
    posic_actual[0]=0;
    delay(300);
}
void homing_comp(){
    while (not ( digitalRead(FC[1])==HIGH && digitalRead(FC[2])==HIGH) )
//normalmente cerrado
    {
        if (digitalRead(FC[1]) ==LOW) {
            retrocede(0);
        }
        if (digitalRead(FC[2]) ==LOW) {
            retrocede(1);
        }
    }
    posic_actual[1,2]=0;
    Serial.write('F');
    delay(300);
}
void loop() {
    digitalWrite(L_AMAR, LOW);
    digitalWrite(L_ROJO, LOW);
    digitalWrite(L_VERDE, LOW);
    digitalWrite(L_AZUL, LOW);
    if (Serial.available() > 0) {
        int entrada = Serial.read();
        if (entrada=='P'){
            digitalWrite(L_AMAR, HIGH);
            reset_nema();
            while(not (posic_actual[0]==posic_plastico))
            {
                //Serial.write(posic_actual);
                if (posic_actual[0]<posic_plastico){
                    digitalWrite(direccion, LOW); //mandamos direccion al servo
                    posic_actual[0]++;
                }
                else{
                    digitalWrite(direccion, HIGH); //mandamos direccion al servo
                    posic_actual[0]--;
                }
                digitalWrite(pasos, HIGH); // ponemos a high «pasos»
                digitalWrite(pasos, LOW); // ponemos a low «pasos»
                delayMicroseconds(vel_sel); // leemos la referencia de velocidad
            }
            digitalWrite(reset, LOW); //Mientras reset este en LOW, el motor permanece apagado
            //Serial.write(posic_actual);
            Serial.write('F');
            delay(300);
        }
        if (entrada=='C'){
            digitalWrite(L_AZUL, HIGH);
            reset_nema();
            while(not (posic_actual[0]==posic_carton))
            {
                if (posic_actual[0]<posic_carton){
                    digitalWrite(direccion, LOW); //mandamos direccion al servo
                    posic_actual[0]++;
                }
                else{
                    digitalWrite(direccion, HIGH); //mandamos direccion al servo
                    posic_actual[0]--;
                }
                digitalWrite(pasos, HIGH); // ponemos a high «pasos»
                digitalWrite(pasos, LOW); // ponemos a low «pasos»
                delayMicroseconds(vel_sel); // leemos la referencia de velocidad
            }
            digitalWrite(reset, LOW); //Mientras reset este en LOW, el
            motor permanece apagado
            Serial.write('F');
            delay(300);
        }
        if (entrada=='M'){
            digitalWrite(L_ROJO, HIGH);
            reset_nema();
            while(not (posic_actual[0]==posic_metal))
            {
                if (posic_actual[0]<posic_metal){
                    digitalWrite(direccion, LOW); //mandamos direccion al servo
                    posic_actual[0]++;
                }
                else{
                    digitalWrite(direccion, HIGH); //mandamos direccion al servo
                    posic_actual[0]--;
                }
                digitalWrite(pasos, HIGH); // ponemos a high «pasos»
                digitalWrite(pasos, LOW); // ponemos a low «pasos»
                delayMicroseconds(vel_sel); // leemos la referencia de velocidad
            }
            digitalWrite(reset, LOW); //Mientras reset este en LOW, el motor permanece apagado
            Serial.write('F');
            delay(300);
        }
        if (entrada=='V'){
            digitalWrite(L_VERDE, HIGH);
            reset_nema();
            while(not (posic_actual[0]==posic_vidrio))
            {
                if (posic_actual[0]<posic_vidrio){
                    digitalWrite(direccion, LOW); //mandamos direccion al servo
                    posic_actual[0]++;
                }
                else{
                    digitalWrite(direccion, HIGH); //mandamos direccion al servo
                    posic_actual[0]--;
                }
                digitalWrite(pasos, HIGH); // ponemos a high «pasos»
                digitalWrite(pasos, LOW); // ponemos a low «pasos»
                delayMicroseconds(vel_sel); // leemos la referencia de velocidad
            }
            digitalWrite(reset, LOW); //Mientras reset este en LOW, el motor permanece apagado
            Serial.write('F');
            delay(300);
        }
        if (entrada=='O'){
            digitalWrite(L_BLANCO, HIGH);
            reset_nema();
            while(not (posic_actual[0]==posic_organico))
            {
                if (posic_actual[0]<posic_organico){
                    digitalWrite(direccion, LOW); //mandamos direccion al servo
                    posic_actual[0]++;
                }
                else{
                    digitalWrite(direccion, HIGH); //mandamos direccion al servo
                    posic_actual[0]--;
                }
                digitalWrite(pasos, HIGH); // ponemos a high «pasos»
                digitalWrite(pasos, LOW); // ponemos a low «pasos»
                delayMicroseconds(vel_sel); // leemos la referencia de velocidad
            }
            digitalWrite(reset, LOW); //Mientras reset este en LOW, el motor permanece apagado
            Serial.write('F');
            delay(300);
        }
        if (entrada=='H'){ //hacer homing
            digitalWrite(L_AMAR, HIGH);
            digitalWrite(L_ROJO, HIGH);
            digitalWrite(L_VERDE, HIGH);
            digitalWrite(L_AZUL, HIGH);
            homingsel();
            homing_comp();
            Serial.write('F');
            delay(300);
        }
        if (entrada=='B'){
            digitalWrite(L_AMAR, HIGH);
            digitalWrite(L_AZUL, HIGH);
            while (posic_actual[1]<3400 and posic_actual[2]<3400 ){
                avanza(0);
                avanza(1);
            }
            Serial.write('F');
            delay(300);
        }
        if (entrada=='S' ){ //subir compuertas
            homing_comp();
        }

    }
}
