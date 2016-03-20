
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <avr/pgmspace.h>

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

static const unsigned char PROGMEM logo16_glcd_bmp[] =
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


//vars
int count = 0;
bool acierto = false;

void setup()   {
  Serial.begin(9600);

  display.begin();
  // init done

  // you can change the contrast around to adapt the display
  // for the best viewing!
  display.setContrast(50);

  
  randomSeed(analogRead(A5));
 
    
  // display WelcomeScreen
  display.clearDisplay();
  display.setRotation(2);  // rotate 90 degrees counter clockwise, can also use values of 2 and 3 to go further.
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0,0);
  display.println(" ------------");
  display.println(" SumaCaramelo");
  display.println(" ------------");
  testdrawbitmap(logo16_glcd_bmp, LOGO16_GLCD_WIDTH, LOGO16_GLCD_HEIGHT);
  //display.drawBitmap(29, 25,  logo16_glcd_bmp, LOGO16_GLCD_WIDTH, LOGO16_GLCD_HEIGHT, BLACK);
  //display.display();
  //delay(2000);

}

void loop() {
  if(!acierto){
    // text display tests
    display.clearDisplay();
    display.setRotation(2);  // rotate 90 degrees counter clockwise, can also use values of 2 and 3 to go further.
    display.setTextSize(1);
    display.setTextColor(BLACK);
    display.setCursor(0,0);
    display.print("Nivel 25");
    display.setTextColor(WHITE, BLACK); // 'inverted' text
    display.setCursor(53,0);
    display.println(" XXX ");
    display.setTextColor(BLACK);
    display.setCursor(3,7);
    display.println("-------------");
    display.setTextSize(2);
    display.println("55+60=");
    if(count == 0){
      count += 1;
    }
    else if(count == 1){
      display.print("1");
      count += 1;
    }
    else if(count == 2){
      display.print("11");
      count += 1;
    }
    else{
      display.print("115");
      acierto = true;
      count = 0;
    }
    display.display();
    delay(1000);
  }
  else{
    for(int i=0; i<3; i++){
      display.clearDisplay();
      display.println();
      display.display();
      delay(400);
      display.clearDisplay();
      display.setRotation(2);  // rotate 90 degrees counter clockwise, can also use values of 2 and 3 to go further.
      display.setTextSize(1);
      display.setTextColor(BLACK);
      display.setTextSize(1);
      display.setCursor(15,20);
      display.println("Correcto!");
      display.display();
      delay(800);
    }
    
    // draw a bitmap icon and 'animate' movement
    display.clearDisplay();
    display.clearDisplay();
    display.setRotation(2);  // rotate 90 degrees counter clockwise, can also use values of 2 and 3 to go further.
    display.setTextSize(1);
    display.setTextColor(BLACK);
    display.setCursor(0,0);
    display.println(" ------------");
    display.println("  Caramelos!");
    display.println(" ------------");
    testdrawbitmap(logo16_glcd_bmp, LOGO16_GLCD_WIDTH, LOGO16_GLCD_HEIGHT);
    acierto = false;
  }
}


void testdrawbitmap(const uint8_t *bitmap, uint8_t w, uint8_t h) {
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
      display.drawBitmap(icons[f][XPOS], icons[f][YPOS], logo16_glcd_bmp, w, h, BLACK);
    }
    display.display();
    delay(200);
    
    // then erase it + move it
    for (uint8_t f=0; f< NUMFLAKES; f++) {
      display.drawBitmap(icons[f][XPOS], icons[f][YPOS],  logo16_glcd_bmp, w, h, WHITE);
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
