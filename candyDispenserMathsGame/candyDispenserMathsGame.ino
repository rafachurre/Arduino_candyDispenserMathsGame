/*
 * Candy dispenser with a maths game
 * 
 * This is a maths game with different levels. It uses a 4x4 keypad.
 * 
 * Game:  It shows an operation ('+', '-' or '*') and wait for the result
 *        There are diferent levels:
 *        Level 1 to 5: Only sums of 1 digit numbers
 *        Level 5 to 10: Only subtractions of 1 digit numbers
 *        Level 10 to 15: Only sums of 1 or 2 digit numbers
 *        Level 15 to 20: Only subtraction of 1 or 2 digit numbers
 *        Level 20 to 25: Only multiplication of 1 digit numbers
 *        Level 25 to 30: Only multiplication of 1 or 2 digit numbers
 *        Level 30 or greater: Random operation of 1 or 2 digit numbers
 *        
 *        'A', 'B', 'C', 'D', '#', '*' keys enter the player result
 * 
 * Wiring:
 *    Keypad rows: pins 2 to 5;
 *    Keypad columns: pins 6 to 8 and A1;
 *    Servo: pin 9;
 *    Nokia5110 LCD: 10 to 13 and A0;
 *    
 * 
 * @author Rafael Lopez Martinez
 * @date 19 March 2016
 */

#include <Keypad.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <avr/pgmspace.h>
#include <Servo.h>


Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

#define PINSERVO 9

// Software SPI (slower updates, more flexible pin options):
// pin 7 - Serial clock out (SCLK)
// pin 6 - Serial data out (DIN)
// pin 5 - Data/Command select (D/C)
// pin 4 - LCD chip select (CS)
// pin 3 - LCD reset (RST)
// Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 4, 3);

// Hardware SPI (faster, but must use certain hardware pins):
// SCK is LCD serial clock (SCLK) - this is pin 13 on Arduino Uno
// MOSI is LCD DIN - this is pin 11 on an Arduino Uno
// pin 10 - Data/Command select (D/C)
// pin 12 - LCD chip select (CS)
// pin A0 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(10, 12, A0);
// Note with hardware SPI MISO and SS pins aren't used but will still be read
// and written to during SPI transfer.  Be careful sharing these pins!

#define NUMFLAKES 5
#define XPOS 0
#define YPOS 1
#define DELTAY 2


#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  24

static const unsigned char PROGMEM logoCaramelo[] =
{ B00000000, B00000000, B00000000,
  B00000000, B00000000, B00000000,
  B00000000, B00000000, B00000000,
  B00000000, B00111100, B00000000,
  B00000000, B11101111, B00000000,
  B10000001, B11011101, B10000001,
  B11100011, B10111011, B11000111,
  B11111011, B01110111, B01011111,
  B11111010, B11101110, B11011111,
  B11100011, B11011101, B11000111,
  B10000001, B10111011, B10000001,
  B00000000, B11110111, B00000000,
  B00000000, B00111100, B00000000,
  B00000000, B00000000, B00000000,
  B00000000, B00000000, B00000000,
  B00000000, B00000000, B00000000 };


const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {5, 4, 3, 2}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {A1, 8, 7, 6}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

//vars
bool waitResult = false; //flag: el jugador tiene que introducir el resultado
int numberOfErrors = 0; //contador: errores cometidos
int level = 1; //contador: nivel alcanzado
int number1 = 0; //primer numero de la operacion
int number2 = 0; //segundo numero de la operacion
int operation = 1; //1='+'; 2='-'; 3='x';
String resultString = ""; //variable que va almacenando los numero que va pulsando
int servoPos = 5;    // variable to store the servo position

void setup(){
  Serial.begin(9600);
  display.begin();
  randomSeed(analogRead(A5));
  myservo.attach(PINSERVO);  // attaches the servo on pin 9 to the servo object
  myservo.write(servoPos);              // tell servo to go to position in variable 'pos'
  // init done

  // you can change the contrast around to adapt the display
  // for the best viewing!
  display.setContrast(50);
     
  // display WelcomeScreen
  display.clearDisplay();
  display.setRotation(2);  // rotate 90 degrees counter clockwise, can also use values of 2 and 3 to go further.
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0,0);
  display.println(" ------------");
  display.println(" SumaCaramelo");
  display.println(" ------------");
  candyFall(logoCaramelo, LOGO16_GLCD_WIDTH, LOGO16_GLCD_HEIGHT);

}

void loop(){
  //if (!waitResult && !pressKey){ //si no estoy esperando por el resultado o por una tecla cualquiera
  if (!waitResult){ //si no estoy esperando por el resultado o por una tecla cualquiera
    if(numberOfErrors < 3){ // si aun me quedan vidas
      
      //Calculo los numeros y la operacion
      generateOperation();
      generateNumbers();

      printOperation();
      
      //fijo los flags: esperando por resultado
      waitResult = true;
    }
    else{  //si no me quedan vidas
      display.clearDisplay();
      display.setRotation(2);  // rotate 90 degrees counter clockwise, can also use values of 2 and 3 to go further.
      display.setTextSize(1);
      display.setTextColor(BLACK);
      display.setCursor(0,0);
      display.println("Has fallado 3");
      display.setCursor(20,9);
      display.println("veces!!");
      display.setCursor(5,30);
      display.println("No te quedan");
      display.setCursor(20,39);
      display.println("vidas");
      waitResult = false;
    }
  }
  else{ //si estoy esperando por resultado o por una tecla cualquiera
    char key = keypad.getKey();
    if (key != NO_KEY){//si presiono una tecla

      // Si estoy esperando el resultado
      if (key == '*' ||key == '#' ||key == 'A' ||key == 'B' ||key == 'C' ||key == 'D'){ //si presiono una tecla que no sea numero, confirmo el resultado introducido
        Serial.println();
        //Serial.println(resultString); //debugging
        checkResult(resultString);
        resultString = ""; //limpio la variable para la siguiente operacion
      }

      else{ // Mientras introduzca numeros, los almaceno e imprimo
        resultString += key;
        display.clearDisplay();
        printOperation();        
        display.println(resultString);
      }
    }
  }
  display.display();
}

/*
 * Comprueba si el resultado es correcto
 */
void checkResult(String resultString){  
  int resultInteger = resultString.toInt();
  int trueResult = 0;

  //calculate trueResult
  switch(operation){ // calcula el resultado correcto dependiendo de la operacion
    case 1: 
      trueResult = number1+number2;
      break;
    case 2: 
      trueResult = number1-number2;
      break;
    case 3:
      trueResult = number1*number2;
      break;
    default:
      trueResult = number1*number2;
      break;
  }
  if(trueResult == resultInteger){ //si es correcto
    for(int i=0; i<3; i++){
      display.clearDisplay();
      display.println();
      display.display();
      delay(250);
      display.clearDisplay();
      display.setRotation(2);  // rotate 90 degrees counter clockwise, can also use values of 2 and 3 to go further.
      display.setTextSize(1);
      display.setTextColor(BLACK);
      display.setTextSize(1);
      display.setCursor(15,20);
      display.println("Correcto!");
      display.display();
      delay(500);
    }
    
    // draw a bitmap icon and 'animate' movement
    display.clearDisplay();
    display.setRotation(2);  // rotate 90 degrees counter clockwise, can also use values of 2 and 3 to go further.
    display.setTextSize(1);
    display.setTextColor(BLACK);
    display.setCursor(0,0);
    display.println(" ------------");
    display.println("  Caramelos!");
    display.println(" ------------");

    activateServo();
    candyFall(logoCaramelo, LOGO16_GLCD_WIDTH, LOGO16_GLCD_HEIGHT);
    
    waitResult = false; //fija el flag: no esta esperando un resultado
    level += 1; //sube un nivel
  }
  else{ //si es incorrecto
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(BLACK);
    display.setCursor(0,0);
    display.println("OOH! Fallaste");
    display.setCursor(0,20);
    display.print("Era: ");
    display.setCursor(30,20);
    display.setTextSize(2);
    display.println(String(trueResult));
    display.display();
    delay(5000);
    waitResult = false; //fija el flag: no esta esperando un resultado
    numberOfErrors += 1; //incrementa el contador de errores
  }
}

/*
 * Genera los numeros segun el nivel alcanzado
 */
void generateNumbers(){
  int maxRandom = 10; //en los 10 primeros niveles: solo numeros de 1 digito (sumas y restas)
  if (level>=10 && level<20){ //del nivel 10 al 20: numeros de 1 o 2 digitos (sumas y restas)
    maxRandom = 100;
  }
  else if (level>=20 && level<25){ //del nivel 20 al 25: solo numeros de 1 (multiplicacion)
    maxRandom = 10;
  }
  else if (level>=25){ //del nivel 25 en adelante: numeros de 1 o 2 digitos (del 25 al 30 multiplicaciones; del 30 en adelante, cualquier operacion)
    maxRandom = 100;
  }
  number1 = random(maxRandom); //genera el numero 1
  number2 = random(maxRandom); //genera el numero 2
  
  if(operation == 2){ // si es una resta, evitamos resultados negativos obligando a que number2 < number1
    int temp = min(number1,number2);
    number1 = max(number1, number2);
    number2 = temp;
  }
}

/*
 * Genera la operacion segun el nivel alcanzado
 */
void generateOperation(){
  if(level<5){ //del nivel 1 al 5: suma (1 digito)
    operation = 1;
  }else if (level>=5 && level<10){ //del nivel 5 al 10: resta (1 digito)
    operation = 2;
  }
  else if (level>=10 && level<15){ //del nivel 10 al 15: suma (1 o 2 digitos)
    operation = 1;
  }
  else if (level>=15 && level<20){ //del nivel 15 al 20: resta (1 o 2 digitos)
    operation = 2;
  }
  else if (level>=20 && level<30){ //del nivel 20 al 30: multiplicacion (del 20 al 25: 1 digito; del 25 al 30: 1 o 2 digitos)
    operation = 3;
  }
  else if (level > 30){// a partir del nivel 30: operacion random
    operation = random(1,4);
  }
}

void printOperation(){
      //Imprimo la siguiente operacion
      display.clearDisplay();
      display.setRotation(2);  // rotate 90 degrees counter clockwise, can also use values of 2 and 3 to go further.
      display.setTextSize(1);
      display.setTextColor(BLACK);
      display.setCursor(0,0);
      display.print("Nivel ");// imprimo el nivel
      display.print(String(level));
      display.setTextColor(WHITE, BLACK); // 'inverted' text
      display.setCursor(53,0);
      switch(numberOfErrors){ // imprimo las vidas que quedan
        case 0: 
          display.println(" XXX ");
          break;
        case 1: 
          display.println("  XX ");
          break;
        case 2: 
          display.println("   X ");
          break;
        default:
          display.println(" XXX ");
          break;
      }
      display.setTextColor(BLACK);
      display.setCursor(3,7);
      display.println("-------------");
      display.setTextSize(2);
      
      //imprimo la operacion
      display.print(String(number1));
      switch(operation){
        case 1: 
          display.print("+");
          break;
        case 2: 
          display.print("-");
          break;
        case 3: 
          display.print("x");
          break;
        default:
          display.print("x");
          break;
      }
      display.print(String(number2));
      display.println("=");
}

void candyFall(const uint8_t *bitmap, uint8_t w, uint8_t h) {
  uint8_t icons[NUMFLAKES][3];
 
  // initialize
  for (uint8_t f=0; f< NUMFLAKES; f++) {
    icons[f][XPOS] = random(display.width());
    icons[f][YPOS] = 20;
    icons[f][DELTAY] = random(5) + 1;
    
    Serial.print("x: ");
    Serial.print(icons[f][XPOS], DEC);
    Serial.print(" y: ");
    Serial.print(icons[f][YPOS], DEC);
    Serial.print(" dy: ");
    Serial.println(icons[f][DELTAY], DEC);
  }
  int timeVar = 0;
  while (timeVar < 30) {
    timeVar +=1;
    // draw each icon
    for (uint8_t f=0; f< NUMFLAKES; f++) {
      display.drawBitmap(icons[f][XPOS], icons[f][YPOS], logoCaramelo, w, h, BLACK);
    }
    display.display();
    delay(200);
    
    // then erase it + move it
    for (uint8_t f=0; f< NUMFLAKES; f++) {
      display.drawBitmap(icons[f][XPOS], icons[f][YPOS],  logoCaramelo, w, h, WHITE);
      // move it
      icons[f][YPOS] += icons[f][DELTAY];
      // if its gone, reinit
      if (icons[f][YPOS] > display.height()) {
        icons[f][XPOS] = random(display.width());
        icons[f][YPOS] = 20;
        icons[f][DELTAY] = random(5) + 1;
      }
    }
  }
}

void activateServo(){
  for (servoPos = 5; servoPos <= 180; servoPos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(servoPos);              // tell servo to go to position in variable 'pos'
    delay(5);                       // waits 15ms for the servo to reach the position
  }
  for (servoPos = 180; servoPos >= 5; servoPos -= 1) { // goes from 180 degrees to 0 degrees
    myservo.write(servoPos);              // tell servo to go to position in variable 'pos'
    delay(5);                       // waits 15ms for the servo to reach the position
  }
}
