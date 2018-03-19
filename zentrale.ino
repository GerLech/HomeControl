#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
//Einbindung von Font Bibliotheken
#include <Fonts/FreeSans9pt7b.h> //Font für Logo
#include <Fonts/FreeSansBold9pt7b.h> //Font für Logo

#include <SPI.h>
#include <SD.h>

//Definition der verwendeten Pins
#define TFT_CS  5  // Chip select line for TFT display
#define TFT_RST  22  // Reset line for TFT 
#define TFT_DC   21  // Data/command line for TFT

#define SD_CS    16  // Chip select line for SD card


//tft instance
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

char *menu[] = {"Eintrag 1", "Eintrag 2", "Eintrag 3", "Eintrag 4", 
        "Eintrag 5", "Eintrag 6", "Eintrag 7", "Eintrag 8", "Eintrag 9", 
        "Eintrag 10", "Eintrag 11", "Eintrag 12", "Eintrag 13", "Eintrag 14"};
        
uint8_t menu_lines = 14; //Anzahl der Menüeinträge

const byte button = 15;
const byte pulse = 14;
const byte pulseDirection = 4 ;

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
volatile int menuUp = 0;
volatile int menuDown = 0;
volatile int menuBtn = 0;

//globale Variablen für das Menü
uint8_t line_len = 127; //Anzahl der pixels pro Zeile
uint8_t start_menu = 32; //Zeile in der das Menü beginnt
uint8_t menu_size = 12; //Anzahl Menü zeilen a 10 Pixel
uint16_t fnt_color = ST7735_WHITE; //Schriftfarbe
uint16_t bg_color = ST7735_BLACK; //Menühintergrund
uint16_t sel_fnt_color = ST7735_BLACK; //Schriftfarbe für Auswahl
uint16_t sel_bg_color = ST7735_GREEN; //Hintergrund für Auswahl
uint8_t line_selected = 0; //Ausgewählte Menüzeile
uint8_t top_line = 0; //erste dargestellte Menüzeile

//Funktion zum Schreiben einer selektierten Zeile
void menuSelLine(uint8_t line, char *txt){
  uint8_t y1 = line * 10 + start_menu; //Oberkante der Zeile
  tft.setCursor(4,y1);
  tft.setTextColor(sel_fnt_color,sel_bg_color);
  tft.print(txt);
}

//Funktion zum Schreiben einer Zeile
void menuLine(uint8_t line, char *txt){
  uint8_t y1 = line * 10 + start_menu; //Oberkante der Zeile
  tft.setCursor(4,y1);
  tft.setTextColor(fnt_color,bg_color);
  tft.print(txt);
}

//Funktion zum Anzeigen des Menüs
void showMenu() {
  //zuerst den Hintergrund löschen
  tft.fillRect(0,(start_menu),line_len,(menu_size * 10),bg_color);
  uint8_t mlin = top_line;
  while ((mlin < menu_lines) && ((mlin - top_line) < menu_size)) {
    if (mlin == line_selected) {
      menuSelLine(mlin-top_line,menu[mlin]);
    } else {
      menuLine(mlin-top_line,menu[mlin]);
    }
    mlin++;
  }
  
}

//Funktion um die Auswahl nach unten zu verschieben
void selectionDown() {
    line_selected++;
    if (line_selected >= menu_lines) line_selected = menu_lines-1;
    if ((line_selected - top_line) >= menu_size) top_line++;
    showMenu();
}


//Funktion um die Auswahl nach oben zu verschieben
void selectionUp() {
    if (line_selected > 0) line_selected--;
    if (line_selected < top_line) top_line--;
    showMenu();
}

//Funktion zum Anzeigen eines Textes am Display in einer bestimmten Farbe 
void displayText(char *text , uint16_t color) {
  tft.setTextColor(color);
  tft.setTextWrap(true); //automatischer Zeilenumbruch wird aktiviert

  tft.print(text);
}

//Funktion zum Anzeigen eines Textes an einem beliebigen Punkt (x,y) auf
//dem Display. Die Farbe kann gewählt werden
void displayText(uint16_t x, uint16_t y, char *text , uint16_t color) {
  tft.setCursor(x, y);
  displayText(text,color);
}

//Funktion zum Anzeigen des Logos ganz oben am Display
void displayLogo() {
  //Hintergrund füllen
  tft.fillRect(0,0,127,30,ST7735_YELLOW);
  //Rahmen zeichnen
  tft.drawRect(1,1,125,28,ST7735_BLUE);
  //Font für das Wort HOME fett
   tft.setFont(&FreeSansBold9pt7b);
   //Cursor positionieren
   tft.setCursor(7,20);
   //Text in schwarz ausgeben
   tft.setTextColor(ST7735_BLACK);
   tft.print("HOME");
   //Font für das Wort Control nicht fett
   tft.setFont(&FreeSans9pt7b);
   //Text in rot ausgeben
   tft.setTextColor(ST7735_RED);
   tft.print("Control");
   //Font auf default zurücksetzen
   tft.setFont(NULL);
}

//interrupt service for switch
//change current color
void IRAM_ATTR btnClick() {
  portENTER_CRITICAL_ISR(&mux);
  menuBtn = 1;
  portEXIT_CRITICAL_ISR(&mux);
}

//interrupt for rotary pulse
void IRAM_ATTR rotaryPulse() {
  byte dir = digitalRead(pulseDirection);
  if ((menuUp == 0) && (menuDown == 0)) {
    portENTER_CRITICAL_ISR(&mux);
    if (dir==0) {
      menuUp = 1;
    } else {
      menuDown = 1;
    }
    portEXIT_CRITICAL_ISR(&mux);
  }
}

  
void setup() {
  Serial.begin(115200);
  //tft initialisieren und schwarzer Hintergrund
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST7735_BLACK);
  //Logo anzeigen
  displayLogo();
  //SD-Karte initialisieren und Ergebnis anzeigen
  if (!SD.begin(SD_CS)) {
    displayText(25,40,"Keine SD-Card",ST7735_YELLOW);
  } else {
    displayText(34,40,"SD-Card OK",ST7735_GREEN);
  }
  delay(1000);
  showMenu();
  //define input pins
  pinMode(button, INPUT_PULLUP);
  pinMode(pulse, INPUT_PULLUP);
  pinMode(pulseDirection, INPUT_PULLUP);
  //define interrupts
  attachInterrupt(digitalPinToInterrupt(pulse),rotaryPulse, FALLING);
  attachInterrupt(digitalPinToInterrupt(button),btnClick, FALLING);
}

void loop() {
  if (menuUp) {
      delay(100); //Zum entprellen
      portENTER_CRITICAL(&mux);
      menuUp = 0;
      menuDown=0;
      portEXIT_CRITICAL(&mux);
      selectionUp();
      Serial.println("Up");
  }
  if (menuDown) {
      delay(100); //Zum entprellen
      portENTER_CRITICAL(&mux);
      menuUp=0;
      menuDown = 0;
      portEXIT_CRITICAL(&mux);
      selectionDown();
      Serial.println("Down!");
  }


}

