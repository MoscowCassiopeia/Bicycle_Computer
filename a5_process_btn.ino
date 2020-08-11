void process_buttons() {
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

}
