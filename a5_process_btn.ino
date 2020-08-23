void process_buttons() {
  // обрабатываем нажатия кнопок
  if (butt_1.isSingle()) {

    if (show_info == SET_TRACK) {
      counter_tick_track += TICKS_IN_KM / 2;
      on_track = true;
      track_set_timer.start();
    }
    else if (on_track) {
      if (show_info == SCREEN_TRACK)
        show_info = ALL_INFO;
      else
        show_info++;
    }
    else {
      if (show_info >= BARS_TRACK_RPM)
        show_info = ALL_INFO;
      else
        show_info++;
    }

    draw_screen();
  }
  if (butt_1.isHold()) {
    if (on_track && show_info == DISTANCE)
      stop_track("reset");
    else if (show_info == SET_TRACK) {

      if (hold_btn_track_timer.isReady()) {

        counter_tick_track += TICKS_IN_KM / 2;
        on_track = true;
        track_set_timer.start();
        //draw_screen();
      }
    }
    else
      clear_distance();

    draw_screen();

  }


  if (butt_1.isDouble() && show_info == DISTANCE && distance_km == 0) {
    show_info = SET_TRACK;
    track_set_timer.setInterval(TIME_OUT_TRACK);
    hold_btn_track_timer.setInterval(TIME_INC_TRACK);
    draw_screen();
  }

}
