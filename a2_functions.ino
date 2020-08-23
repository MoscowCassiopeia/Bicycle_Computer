
void bars_rpm() {
#define MAX_RPM 400
#define MIN_RPM 0
  uint16_t avrg = 0;

  //  Serial.println("buff_bars[0]: " + String(buff_bars[0]));
  //  Serial.println("buff_bars[1]: " + String(buff_bars[1]));
  //  Serial.println("buff_bars[2]: " + String(buff_bars[2]));

  if (buff_bars[COUNT_AVRG_BARS - 1] != -1) {
    for (int i = 0; i < COUNT_AVRG_BARS; i++) {
      avrg += buff_bars[i];
      buff_bars[i] = -1;
    }
    if (!vector_bars_avrg.full())
      vector_bars_avrg.push_back(avrg / COUNT_AVRG_BARS);
    else {
      vector_bars_avrg.remove(0);
      vector_bars_avrg.push_back(avrg / COUNT_AVRG_BARS);
    }
    //Serial.println("Average: " + String(avrg / COUNT_AVRG_BARS));
  }


  for (int i = 0; i < COUNT_AVRG_BARS; i++) {
    if (buff_bars[i] == -1) {
      buff_bars[i] = map(rpm, MIN_RPM, MAX_RPM, 0, SCREEN_HEIGHT - 1);
      //Serial.println("map(rpm, MIN_RPM, MAX_RPM, 0, SCREEN_HEIGHT - 1); " + String(map(rpm, MIN_RPM, MAX_RPM, 0, SCREEN_HEIGHT - 1)));
      break;
    }
  }

}

void stop_track(String mode = "finish") {
  // останавливает трек, обнуляя нужные переменные

  clear_distance();
  counter_tick_track = 0;
  track_distance_km = 0;
  on_track = false;
  draw_screen();
  if (mode == "finish") {  // если остановка произошла в случае конца трека а не сброса кнопкой, то играем типа мелодию
    play_melody = true;
    melody_timer.setInterval(NOTE_DURATION);
  }

  //play_tone();
}

void play_tone() {
  // играет мелодию

  if (melody_timer.isReady() && play_melody && counter_note != NOTE_COUNT) {
    tone(SPK_PIN, note += NOTE_NEXT, NOTE_DURATION);
    counter_note++;
  }
  else if (counter_note == NOTE_COUNT) {
    counter_note = 0;
    play_melody = false;
    note = NOTE_BEGIN;
  }

}

void screen_bars() {

  display.clearDisplay();
  for (uint8_t i = 0; i < vector_bars_avrg.size(); i++)
    display.drawFastVLine(i + 10, 31 - vector_bars_avrg[i], vector_bars_avrg[i], SSD1306_WHITE);

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("R");
  display.println("P");
  display.println("M");
  display.display();

}


void screen_track() {
  
  // отображает оставшуюся дистанцию трека графически

  display.clearDisplay();
  
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("P");
  display.println("B");
  display.println("A");
  display.println("R");
  
  point_bar = track_distance_km / cost_bar - (LENGTH_PB - INDENT / 2);

  display.fillRect(INDENT, 0, abs(point_bar), 32, SSD1306_WHITE);  // с лева на право

  display.fillRect(127 - abs(point_bar), 0, abs(point_bar), 32, SSD1306_WHITE);  // с права на лево
  display.display();
}

unsigned int get_odometer() {

  // возвращает последнее записанное в EEPROM число

#define MAX_ADDR 1024 - 2 // размер вашего EEPROM, вместо 1024 подставить размер вашего EEPROM в байтах
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

void processing_tick() {

  if (!digitalRead(BICYCLE_PIN) && flag_tick) {

    if (last_tick_time != 0)
      rpm = MILLIS_MIN / (millis() - last_tick_time);
    bars_rpm();

    last_tick_time = millis();

    distance_km = get_distance(counter_tick++); //Serial.println("tik: " + String(counter_tick));
    speed = float(rpm) / TEN_METERS * 10 * MIN_IN_HOUR / METER_IN_KM;

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

void clear_distance() {

  // обнуляет пройденную дистанцию

  put_odometer();
  distance_km = 0;
  counter_tick = 0;
  last_distance = 0;

}

float get_distance(uint32_t counter) {

  // возвращает дистанцию в км, высчитываемую из тиков
  return (counter / float(TEN_METERS * 100));
}

void erase_eeprom() {
  for (int i = 0; i < EEPROM_SIZE; i++) {
    EEPROM.put(i, 0);
  }
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

unsigned int get_last_distance() {

  // получает дистанцию пройденную с момента последней записи в EEPROM

  unsigned int delta_distance = 0;
  delta_distance = distance_km - last_distance;
  last_distance = distance_km;
  return delta_distance;
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
  else if (show_info == SET_TRACK) {
    display_fullsize("Set track:", String(track_distance_km = get_distance(counter_tick_track)));
  }
  else if (show_info == SCREEN_TRACK) {
    screen_track();
  }
  else if (show_info == BARS_TRACK_RPM) {
    screen_bars();
  }

}
