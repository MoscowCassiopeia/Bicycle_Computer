void setup() {

  Serial.begin(9600);
  pinMode(BICYCLE_PIN, INPUT_PULLUP);

  butt_1.setDebounce(50);        // настройка антидребезга (по умолчанию 80 мс)
  butt_1.setTimeout(500);        // настройка таймаута на удержание (по умолчанию 500 мс)
  butt_1.setClickTimeout(400);   // настройка таймаута между кликами (по умолчанию 300 мс)
  butt_1.setType(HIGH_PULL);
  butt_1.setDirection(NORM_OPEN);
  //vector_bars.setStorage(bars);
  vector_bars_avrg.setStorage(bars_avrg);

// инициализируем массив
  for (uint16_t i = 0; i < COUNT_AVRG_BARS; i++)
    buff_bars[i] = -1;  

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
  //display.drawPixel(10, 10, SSD1306_WHITE);
  //display.display();
  Serial.println("Possible command:");
  Serial.println("\terase - Clear EEPROM");  
  Serial.println("Send me command");
  delay(4000);
  if (Serial.available() > 0) {    
    String incomingByte = Serial.readString();
    if (incomingByte == "erase") {
      erase_eeprom();
    }    
  }

}
