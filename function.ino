void play_tone() {
  // играет мелодию
  tone(SPK_PIN, 900, 5000);
}

void screen_track() {

  // отображает оставшуюся дистанцию трека графически
  
  display.clearDisplay();
  point_bar = track_distance_km / cost_bar - LENGTH_PB;  
  
  display.fillRect(0, 0, abs(point_bar), 32, SSD1306_WHITE);  // с лева на право
  
  display.fillRect(127 - abs(point_bar), 0, abs(point_bar), 32, SSD1306_WHITE);  // с права на лево
  Serial.println("track_distance_km: " + String(track_distance_km));  
  Serial.println("LEFT: " + String(abs(point_bar)));  
  
  Serial.println("RIGHT: coord: " + String((127 - abs(point_bar))) + "   w: " + String(abs(point_bar)));
  
  display.display();
  
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

void processing_tick() {
  
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
