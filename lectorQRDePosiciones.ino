#include <LiquidCrystal_I2C.h>
#include <Wire.h>
LiquidCrystal_I2C lcd(0x27,20,4); 


//LA MAYORIA DE VALORES 16 ES PORQUE EN LAS PRUEBAS, ESTABA USANDO 16 LEDS, RECUERDA QUE TENDREMOS QUE USAR 130 LEDS


int SER_Pin = 9;  // Pin de datos (DS)
int SRCLK_Pin = 12; // Pin de reloj de desplazamiento (SHCP)
int RCLK_Pin = 11; // Pin de reloj de almacenamiento (STCP)

const int numChips = 16;  // Cambia esto según la cantidad de chips que estés usando  13 x 8 = 104 13 tarjetas
  
int A[9]; //posiciones
int B[9];
int POS[16];

String variante;
String UBICACION[16];  // Un arreglo para almacenar los valores de UBICACION1 a UBICACION15
//String ledPositionsA[130];

int finish = 4;
int valorFinish;
int sensor = 13;
int valorSensor;
int boton2 = 2;
int valorBoton2;

int buzzer = 3;

int numPosiciones = 0;

bool posicionesFlag = false;

String ledPositionsA[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16"};





void setup() {
  lcd.init();
  lcd.backlight();
  lcd.clear();

  Serial.begin(9600);

  pinMode(SER_Pin, OUTPUT);
  pinMode(SRCLK_Pin, OUTPUT);
  pinMode(RCLK_Pin, OUTPUT);

  pinMode(finish, INPUT);
  pinMode(sensor, INPUT);
  pinMode(boton2, INPUT);
  pinMode(buzzer, OUTPUT);

  A[0] = 0;
  B[0] = 0;
  POS[0]=200;
  UBICACION[0] = "NA";

  barridoLEDS();
  //apaga todo al inicio
  apagarLEDS();
  BuzzerInicio();

}



void loop() {

  barridoManual();
  LCDinicio();
  if (Serial.available()){
    
    leerQR();
    for (int i = 1; i < 16; i++) {
      asignarPosiciones(UBICACION[i], i);
    }

    LCDmostrarvalores();  
    BuzzerLectura();


    int ledsToTurnOn[] = { POS[1], POS[2], POS[3], POS[4], POS[5], POS[6], POS[7], POS[8], POS[9], POS[10], POS[11], POS[12], POS[13], POS[14], POS[15] };  // Lista de LEDs que deseas encender simultáneamente    Lista de LED empieza desde 0
    // para valores sin posicion, colocar numero mayores a 104 (cantidad de LEDS); si ponemos 13 chips, la posicion 103 (que es la ultima)
    // comenzara desde el led 8 del primer chip conectado, eso quiere decir que el LED103 corresponde a a la salida 8 del chip, la salida 7 sera la posicion 102
    int numLEDs = sizeof(ledsToTurnOn) / sizeof(ledsToTurnOn[0]);
    encenderLEDs(ledsToTurnOn, numLEDs);

    esperandoFinish();
    buzzerProcesoOK();
    apagarLEDS();
    vaciadosBasura();

  }

}



void encenderLEDs(int leds[], int numLEDs) {
  // Asegurarse de que haya al menos un LED y no más de 104 LEDs

  if (numLEDs > 0 && numLEDs <= 130) {
    byte shiftRegisterData[numChips]; // Almacenar datos para cada chip

    // Inicializar el arreglo de datos del registro de desplazamiento
    for (int i = 0; i < numChips; i++) {
      shiftRegisterData[i] = 0;
    }

    // Configurar los bits correspondientes en el arreglo de datos
    for (int i = 0; i < numLEDs; i++) {
      int numeroLED = leds[i];
      if (numeroLED >= 0 && numeroLED <= 130) {
        int chipIndex = numChips - 1 - (numeroLED / 8);  // Invertir la asignación de chips
        int bitIndex = numeroLED % 8;
        shiftRegisterData[chipIndex] |= (1 << bitIndex);
      }
    }

    // Cargar los datos en los registros de desplazamiento
    for (int i = 0; i < numChips; i++) {
      shiftOut(SER_Pin, SRCLK_Pin, MSBFIRST, shiftRegisterData[i]);
    }

    // Almacenar cambios en los registros de almacenamiento
    digitalWrite(RCLK_Pin, LOW);
    digitalWrite(RCLK_Pin, HIGH);
  }
}



void leerQR(){
  
  String valor = Serial.readStringUntil('\n');
  int numComas = contarComas(valor);
  numPosiciones = numComas;

  int commaIndex = valor.indexOf(',');

  variante = valor.substring(0, commaIndex);

  valor = valor.substring(commaIndex + 1);

  // Usar un bucle for para asignar los valores a POS1 a POS15
  for (int i = 1; i < 16; i++) {
    commaIndex = valor.indexOf(',');

    if (commaIndex == -1) {
      // Si no se encuentra una coma, asumimos que hemos llegado al final de la cadena
      UBICACION[i] = valor;
      for (int j = i + 1; j < 15; j++) {
        UBICACION[j] = "NA";
      }
      break;
    }

    UBICACION[i] = valor.substring(0, commaIndex);
    valor = valor.substring(commaIndex + 1);
  }

}

void barridoLEDS(){
  lcd.clear();
  LCDFirma();
  for(int i=0;i<17;i++){
    int ledsToTurnOn[] = {i};  // Lista de LEDs que deseas encender simultáneamente    Lista de LED empieza desde 0
    int numLEDs = sizeof(ledsToTurnOn) / sizeof(ledsToTurnOn[0]);
    encenderLEDs(ledsToTurnOn, numLEDs);

    lcd.setCursor(0,0);
    lcd.print("VERSION 2-PROTOTIPO");
    lcd.setCursor(0,1);
    lcd.print("INICIANDO-LEDS");
    lcd.setCursor(9,2);
    lcd.print(i);

    delay(100);
    apagarLEDS();
  }
  lcd.clear();
}


void asignarPosiciones(const String& UBICACION, const int& numPos){
  //String ledPositionsA[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16"};

  for (int i = 0; i<20 ; i++){
    if(UBICACION == "NA"){
      POS[numPos] = 200;
      break;
    }

    if (UBICACION == ledPositionsA[i]){
      POS[numPos] = i; 
      break;
    }

    /*
    fOr (int i = 0; i < 130; i++) {
    ledPositionsA[i] = String(i + 1);
    }
    */

    
  }
}

void apagarLEDS(){
  digitalWrite(RCLK_Pin, LOW); 
  for (int i = 0; i < numChips; i++) {
    shiftOut(SER_Pin, SRCLK_Pin, MSBFIRST, 0);
  }
  digitalWrite(RCLK_Pin, HIGH); 
}

void LCDinicio(){
  lcd.setCursor(0,0);
  lcd.print("VERSION 2-PROTOTIPO");
  lcd.setCursor(0,1);
  lcd.print("Esperando datos...");
  lcd.setCursor(0,2);
  lcd.print("Escanee QR");
  LCDFujikuraLogo();
}

void LCDmostrarvalores(){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("VARIANTE:");
    lcd.setCursor(0,1);
    lcd.print(variante);
    lcd.setCursor(0,2);
    lcd.print("NUM DE POSICIONES: ");
    lcd.print(numPosiciones);
    lcd.setCursor(0,3);
    lcd.print("POSICIONES EN -SEL-");
}

void esperandoFinish(){
  while(true){
    valorFinish=digitalRead(finish);
    if  (valorFinish==LOW){
        break;
    }
    verPosiciones();
  }
  lcd.clear();
}

void verPosiciones(){

  if( !(valorBoton2=digitalRead(boton2)) && !posicionesFlag){
    posicionesFlag = true;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("POSICIONES:");
    lcd.setCursor(0,1);
    for(int i = 1; i<16;i++){
      if (POS[i] == 200){
        continue;
      }
      lcd.print( ledPositionsA[ POS[i] ] );
      lcd.print(" ");
    }
    delay(1000);
  }
    
  if( !(valorBoton2=digitalRead(boton2)) && posicionesFlag){
    posicionesFlag = false;
    LCDmostrarvalores();
    delay(1000);
  }



}



void barridoManual(){
  while(!(valorBoton2=digitalRead(boton2))){
    bool botonSoltado=false;
    for(int i=1; i<2000; i++){
      delay(1);
      if((valorBoton2=digitalRead(boton2))){
        botonSoltado=true;
        break;
      }  
    }
    if (botonSoltado==false){
      barridoLEDS();
    }
  }
}

void buzzerProcesoOK(){
  for (int i = 1; i < 6; i++) {
    digitalWrite(buzzer, HIGH);
    delay(50);
    digitalWrite(buzzer, LOW);
    delay(50);
  }
}

void BuzzerInicio(){
  digitalWrite(buzzer, HIGH);
  delay(40);
  digitalWrite(buzzer, LOW);
  delay(40);
  digitalWrite(buzzer, HIGH);
  delay(400);
  digitalWrite(buzzer, LOW);
  delay(40);

}


void BuzzerLectura(){
  for (int i = 1; i < 4; i++) {
    digitalWrite(buzzer, HIGH);
    delay(50);
    digitalWrite(buzzer, LOW);
    delay(50);
  }
}

int contarComas(const String& str) {
  int count = 0;
  for (size_t i = 0; i < str.length(); i++) {
    if (str.charAt(i) == ',') {
      count++;
    }
  }
  return count;
}


void LCDFirma(){
  byte A[] = {
  B00000,
  B00000,
  B01110,
  B01010,
  B01110,
  B01010,
  B00000,
  B00000
};

byte G[] = {
  B00000,
  B00000,
  B01110,
  B01000,
  B01010,
  B01110,
  B00000,
  B00000
};

byte U[] = {
  B00000,
  B00000,
  B01010,
  B01010,
  B01010,
  B01110,
  B00000,
  B00000
};

byte E[] = {
  B00000,
  B00000,
  B01110,
  B01100,
  B01000,
  B01110,
  B00000,
  B00000
};

byte R[] = {
  B00000,
  B00000,
  B01110,
  B01010,
  B01100,
  B01010,
  B00000,
  B00000
};

byte O[] = {
  B00000,
  B00000,
  B01110,
  B01010,
  B01010,
  B01110,
  B00000,
  B00000
};


  lcd.createChar(0, A);
  lcd.createChar(1, G);
  lcd.createChar(2, U);
  lcd.createChar(3, E);
  lcd.createChar(4, R);
  lcd.createChar(5, O);         
  lcd.setCursor(7,3);
  lcd.write(0);
  lcd.setCursor(8,3);
  lcd.write(1);
  lcd.setCursor(9,3);
  lcd.write(2);
  lcd.setCursor(10,3);
  lcd.write(3);
  lcd.setCursor(11,3);
  lcd.write(4);
  lcd.setCursor(12,3);
  lcd.write(5);
}


void LCDFujikuraLogo(){

  byte FA[] = {
    B00111,
    B00111,
    B01111,
    B01110,
    B11100,
    B11100,
    B11100,
    B11100
  };

  byte FB[] = {
    B11100,
    B11100,
    B11100,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000
  };

  byte FC[] = {
    B00000,
    B00000,
    B00000,
    B00000,
    B01111,
    B11111,
    B11111,
    B10000
  };

  byte FD[] = {
    B00000,
    B00000,
    B00000,
    B00000,
    B11100,
    B11100,
    B11100,
    B00000
  };
  lcd.createChar(6, FA);
  lcd.createChar(7, FB);
  lcd.createChar(8, FC);
  lcd.createChar(9, FD);

  lcd.setCursor(18,3);
  lcd.write(6);
  lcd.setCursor(19,3);
  lcd.write(7);
  lcd.setCursor(18,2);
  lcd.write(8);
  lcd.setCursor(19,2);
  lcd.write(9);   

}

void vaciadosBasura(){
  if (Serial.available()){
    String basura = Serial.readStringUntil('\n');

  }




}