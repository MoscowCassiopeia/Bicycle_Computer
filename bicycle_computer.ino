#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>
#include "GyverButton.h"
#include "GyverTimer.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);



#define BICYCLE_PIN 8
#define BTN_PIN 5
#define SPK_PIN 6

#define ALL_INFO 0
#define DISTANCE 1
#define RPM 2
#define SPEED 3
#define ODOMETER 4
#define SCREEN_TRACK 5
#define TRACK 6


#define TEN_METERS 7 // оборотов колеса на 10 метров
#define MIN_IN_HOUR 60
#define METER_IN_KM 1000
#define MILLIS_MIN 60000
#define TICKS_IN_KM 700 // количество оборотов колеса в километре


#define TIME_OUT_TRACK 4000 // таймаут установки дистанции трека
#define IDLE_TIMEOUT 1500 // таймаут простоя колеса

#define LENGTH_PB 64 // длина прогрессбара


bool flag_tick = true;
bool on_track = false;
uint16_t rpm = 0;
float speed = 0;
float distance_km = 0;
float track_distance_km = 0;
uint16_t last_distance = 0;
uint16_t counter_tick = 0;
uint16_t counter_tick_track = 0;
uint32_t last_tick_time = 0;
byte show_info = 0;
int point_bar = 0; // координата для прогрессбара
float cost_bar = 0.0; // цена деления одного бара


GButton butt_1(BTN_PIN);
GTimer track_set_timer(MS);
GTimer idle_timer(MS, IDLE_TIMEOUT);
GTimer snowflakes_timer(MS);

void stop_track(String mode="finish"); 

void setup() {

  Serial.begin(9600);
  pinMode(BICYCLE_PIN, INPUT_PULLUP);

  butt_1.setDebounce(50);        // настройка антидребезга (по умолчанию 80 мс)
  butt_1.setTimeout(500);        // настройка таймаута на удержание (по умолчанию 500 мс)
  butt_1.setClickTimeout(400);   // настройка таймаута между кликами (по умолчанию 300 мс)
  butt_1.setType(HIGH_PULL);
  butt_1.setDirection(NORM_OPEN);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

  // Draw a single pixel in white
  display.drawPixel(10, 10, SSD1306_WHITE);
  display.display();
  

}

void loop() {

  butt_1.tick();


// обрабатываем нажатия кнопок
  if (butt_1.isSingle()) {

    if (show_info == TRACK) {
      counter_tick_track += 350;
      on_track = true;
      track_set_timer.start();

    }
    else if (show_info == ODOMETER && on_track)
      show_info++;      
    else if (show_info == SCREEN_TRACK || show_info == ODOMETER)
      show_info = ALL_INFO;
    else
      show_info++;

    draw_screen();
  }
  if (butt_1.isHold() && show_info == DISTANCE) {
    if (on_track)
      stop_track("reset");
      //stop_track();
    else
      clear_distance();

    draw_screen();

  }
  if (butt_1.isDouble() && show_info == DISTANCE && distance_km == 0) {
    show_info = TRACK;
    track_set_timer.setInterval(TIME_OUT_TRACK);
    draw_screen();
  }


  processing_tick();

// опрашиваем таймеры
  if (track_set_timer.isReady()) {
    show_info = DISTANCE;
    track_set_timer.stop();
    draw_screen();
    cost_bar = track_distance_km / LENGTH_PB;
  }
  if (idle_timer.isReady()) {  
    
      rpm = 0;
      speed = 0;
      put_odometer();
      draw_screen();
      idle_timer.stop();
   
  }

}

void stop_track(String mode="finish") {
//void stop_track() {

  // останавливает трек, обнуляя нужные переменные

  clear_distance();  
  counter_tick_track = 0;
  track_distance_km = 0;    
  on_track = false;  
  draw_screen();
  if (mode == "finish")
    play_tone();
  
}

void clear_distance() {

  // обнуляет пройденную дистанцию

  put_odometer();
  distance_km = 0;
  counter_tick = 0;
  last_distance = 0;
  
}

float get_distance(unsigned int counter) {

  // возвращает дистанцию в км, высчитываемую из тиков
  return (counter / float(700));
}

void processing_tick() {

  //tick_on = !digitalRead(BICYCLE_PIN);

  if (!digitalRead(BICYCLE_PIN) && flag_tick) {

    if (last_tick_time != 0)
      rpm = MILLIS_MIN / (millis() - last_tick_time);

    last_tick_time = millis();

    distance_km = get_distance(counter_tick++);
    speed = rpm / TEN_METERS * 10 * MIN_IN_HOUR / METER_IN_KM;

    if (counter_tick_track > 0)
      track_distance_km = get_distance(counter_tick_track--);
    else if (on_track && counter_tick_track == 0)
      stop_track();

    draw_screen();

    flag_tick = false;
    idle_timer.start();
  }
  //else if (!tick_on && flag_tick == false) {
  else if (digitalRead(BICYCLE_PIN) && flag_tick == false) {

    flag_tick = true;

  }

}

void draw_screen() {
  if (show_info == ALL_INFO && on_track) {
    display_info("RPM: " + String(rpm) + "\n" +
                 "Distance KM: " + track_distance_km + "\n"
                 "Speed km/h: " + speed + "\n" +
                 "Odometer: " + get_odometer());
  }
  else if (show_info == ALL_INFO) {
    display_info("RPM: " + String(rpm) + "\n" +
                 "Distance KM: " + distance_km + "\n"
                 "Speed km/h: " + speed + "\n" +
                 "Odometer: " + get_odometer());

  }
  else if (show_info == DISTANCE && on_track) {
    display_fullsize("Left km:", String(track_distance_km));
  }
  else if (show_info == DISTANCE) {
    display_fullsize("Distance km:", String(distance_km));
  }
  else if (show_info == RPM) {
    display_fullsize("RPM:", String(rpm));
  }
  else if (show_info == SPEED) {
    display_fullsize("Speed km/h:", String(speed));
  }
  else if (show_info == ODOMETER) {
    display_fullsize("Odometer:", String(get_odometer()));
  }
  else if (show_info == TRACK) {
    display_fullsize("Set track:", String(track_distance_km = get_distance(counter_tick_track)));
  }
  //else if (show_info == SCREEN_TRACK && abs(point_bar) == 63) {
  //  testanimate(logo_bmp, LOGO_WIDTH, LOGO_HEIGHT);
    
  //}
  else if (show_info == SCREEN_TRACK) {
    screen_track();
  }
  
}


void display_info(String s) {

  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0, 0);            // Start at top-left corner
  display.println(s);
  display.display();

}

void display_fullsize(String label, String info) {

  // отображает полную сводную информацию по всем счетчикам  

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(label);
  display.setCursor(0, 10);
  display.setTextSize(3);
  display.println(info);
  display.display();


}

unsigned int get_last_distance() {

  // получает дистанцию пройденную с момента последней записи в EEPROM
  
  unsigned int delta_distance = 0;
  delta_distance = distance_km - last_distance;
  last_distance = distance_km;
  return delta_distance;
}

void put_odometer() {

  // записать дистанцию distance в eeprom равномерно
  unsigned int distance = 0;

  if ((distance = get_last_distance()) == 0)
    return;

#define BEGIN 0
#define STEP 2 // потому что читаем в unsigned int (2 байта)
#define MAX_ADDR 1022

  unsigned int temp = 0;
  unsigned int old_val = 0; 
  int index = -1;

  for (int i = 0; i <= MAX_ADDR; i += STEP) {
    

    // проходим по eeprom с шагом 2, потому что читаем в unsigned int (2 байта)
    // находим наибольшее значение, оно и будет последнее записанное

    EEPROM.get(i, temp);
    

    if (temp > old_val) {
      // если все значения в EEPROM нулевые то переменной index не присвоится значение, присвоим их в блоке else if
      old_val = temp;
      index = i;      
    }
    else if (i == MAX_ADDR && old_val == 0) { 
      // если все значения в EEPROM нулевые то переменной index не присвоится значение, присвоим его здесь
      index = i;      
    }
  }

  if (index >= 0 && (index + STEP) > MAX_ADDR) {
    // если в конец писать некуда, пишем в начало
    // пишем по адресу [адрес последнего значения + STEP]

    EEPROM.put(BEGIN, distance + old_val);
  }
  else if (index >= 0) {
    // пишем по адресу [адрес последнего значения + STEP]

    EEPROM.put(index + STEP, distance + old_val);
  }

}


unsigned int get_odometer() {

  // возвращает последнее записанное в EEPROM число

#define MAX_ADDR 1022
#define STEP 2 // потому что читаем в unsigned int (2 байта)

  unsigned int temp = 0;
  unsigned int old_val = 0;

  for (int i = 0; i <= MAX_ADDR; i += STEP) {

    // проходим по eeprom с шагом 2, потому что читаем в unsigned int (2 байта)
    // находим наибольшее значение, оно и будет последнее записанное

    EEPROM.get(i, temp);

    if (temp > old_val) {

      old_val = temp;

    }
  }

  return old_val;
}

void screen_track() {
  
  display.clearDisplay();
  point_bar = track_distance_km / cost_bar - LENGTH_PB;  
  
  display.fillRect(0, 0, abs(point_bar), 32, SSD1306_WHITE);  // с лева на право
  
  display.fillRect(127 - abs(point_bar), 0, abs(point_bar), 32, SSD1306_WHITE);  // с права на лево
  Serial.println("track_distance_km: " + String(track_distance_km));  
  Serial.println("LEFT: " + String(abs(point_bar)));  
  
  Serial.println("RIGHT: coord: " + String((127 - abs(point_bar))) + "   w: " + String(abs(point_bar)));
  
  display.display();
  
}

void play_tone() {
  tone(SPK_PIN, 900, 5000);
}
