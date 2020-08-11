void process_timers() {
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
