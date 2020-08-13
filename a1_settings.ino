#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>
#include "GyverButton.h"
#include "GyverTimer.h"


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
#define NOTE_DURATION 500 // пауза между нотами
#define NOTE_BEGIN 100 // нота с которой начинаем играть
#define NOTE_NEXT 100 // приращение следующей ноты 
#define NOTE_COUNT 50 // сколько всего нот проигрываем в мелодии
 
#define TEN_METERS 6.3 // оборотов колеса на 10 метров
#define MIN_IN_HOUR 60
#define METER_IN_KM 1000
#define MILLIS_MIN 60000
#define TICKS_IN_KM 700 // количество оборотов колеса в километре


#define TIME_OUT_TRACK 4000 // таймаут установки дистанции трека
#define TIME_INC_TRACK 200 // таймаут между инкрементами дистанции трека при удержании кнопки
#define IDLE_TIMEOUT 1500 // таймаут простоя колеса

#define LENGTH_PB 64 // длина прогрессбара


GButton butt_1(BTN_PIN);
GTimer track_set_timer(MS);
GTimer idle_timer(MS, IDLE_TIMEOUT);
GTimer snowflakes_timer(MS);
GTimer melody_timer(MS);
GTimer hold_btn_track_timer(MS); // для пауз при удержании кнопки установки дистанции трека


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


bool play_melody = false; // флаг начать проигрывать мелодию
bool flag_tick = true;
bool on_track = false;
uint16_t rpm = 0;
float speed = 0;
float distance_km = 0;
float track_distance_km = 0;
uint16_t last_distance = 0;
uint16_t counter_tick = 0;
uint16_t counter_tick_track = 0;
uint16_t note = 100; // нота с которой начнется мелодия
uint32_t last_tick_time = 0;
byte show_info = 0;
byte counter_note = 0; // счетчик для проигранных нот
int point_bar = 0; // координата для прогрессбара
float cost_bar = 0.0; // цена деления одного бара
