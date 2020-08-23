#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>
#include "GyverButton.h"
#include "GyverTimer.h"
#include <Vector.h>


#define EEPROM_SIZE 1024

#define BICYCLE_PIN 8
#define BTN_PIN 5
#define SPK_PIN 6

// ---------- коды экранов -----------
#define ALL_INFO 0
#define DISTANCE 1
#define RPM 2
#define SPEED 3
#define ODOMETER 4
#define BARS_TRACK_RPM 5 // график rpm
#define SCREEN_TRACK 6 // прогрессбар трека
#define SET_TRACK 7  // меню установки трека
// -----------------------------------

#define NOTE_DURATION 500 // пауза между нотами
#define NOTE_BEGIN 100 // нота с которой начинаем играть
#define NOTE_NEXT 100 // приращение следующей ноты 
#define NOTE_COUNT 50 // сколько всего нот проигрываем в мелодии
 
#define TEN_METERS 6.3 // оборотов колеса на 10 метров
#define MIN_IN_HOUR 60
#define METER_IN_KM 1000
#define MILLIS_MIN 60000
#define TICKS_IN_KM 630 // количество оборотов колеса в километре


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)

#define TIME_OUT_TRACK 4000 // таймаут установки дистанции трека
#define TIME_INC_TRACK 100 // таймаут между инкрементами дистанции трека при удержании кнопки
#define IDLE_TIMEOUT 1500 // таймаут простоя колеса
#define TIME_PROC_BAR 1000 // раз в период расчитываем бар

#define LENGTH_PB 64 // длина прогрессбара
#define INDENT 10 // отступ слева для текста в прогрессбаре
#define COUNT_AVRG_BARS 10 // количество баров из которых считаем среднюю


GButton butt_1(BTN_PIN);
GTimer track_set_timer(MS);
GTimer idle_timer(MS, IDLE_TIMEOUT);
GTimer snowflakes_timer(MS);
GTimer melody_timer(MS);
GTimer hold_btn_track_timer(MS); // для пауз при удержании кнопки установки дистанции трека
GTimer bars_proc_timer(MS, TIME_PROC_BAR); // таймер для расчета баров



Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


bool play_melody = false; // флаг начать проигрывать мелодию
bool flag_tick = true;
bool on_track = false;
uint16_t rpm = 0;
float speed = 0;
float distance_km = 0;
float track_distance_km = 0;
uint16_t last_distance = 0;
uint32_t counter_tick = 0;
uint32_t counter_tick_track = 0;
uint16_t note = 100; // нота с которой начнется мелодия
uint32_t last_tick_time = 0;
byte show_info = 0;
byte counter_note = 0; // счетчик для проигранных нот
int point_bar = 0; // координата для прогрессбара
float cost_bar = 0.0; // цена деления одного бара

int buff_bars[COUNT_AVRG_BARS]; // буфер для промежуточных значений
byte bars_avrg[SCREEN_WIDTH]; // тут будем хранить средние значения
Vector<byte> vector_bars_avrg; // вектор с средними значениями баров для отрисовки
