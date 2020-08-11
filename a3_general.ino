


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
