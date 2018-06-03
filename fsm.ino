void StateEvaluation() {
  switch (current_state) {
    case FWD_MOVEMENT:
      if(wall_detected) {
        wall_detected = false;
        current_state = REV_MOVEMENT;
        last_state = FWD_MOVEMENT;
        Timing20ms = true;
      }
      else {
        MotorFWD();
      }
    break;

    case REV_MOVEMENT:
      if(Timer20ms_Counts >= BACKUP_TIME) {
        Timer20ms_Counts = 0; Timing20ms = true;      // Going to turn now, so need timer running
        last_state = REV_MOVEMENT;
        if(last_turn_maneuver == CW_TURN) {
          current_state = CCW_90_TURN;
        }
        else {
          current_state = CW_90_TURN;
        }  
      }
      else {
        wall_detected = false;
        MotorREV();
      }
    break;

    case CW_90_TURN:
      if(Timer20ms_Counts >= TURN_90_TIME) {
        Timer20ms_Counts = 0; Timing20ms = false;      // Going to turn now, so need timer running
        temp_distance = Get_Distance();
        if(temp_distance <= MIN_DISTANCE) {
          current_state = CCW_180_TURN;
          last_state = CW_90_TURN;
          Timer20ms_Counts = 0; Timing20ms = true;      // Going to turn now, so need timer running
        }
        else {                                            //Okay to continue going forward
          current_state = FWD_MOVEMENT;
          last_state = CW_90_TURN;
        }
      }
      
      else {
        last_turn_maneuver = CW_TURN;
        wall_detected = false;
        MotorCWTurn();
      }     
    break;

    case CCW_180_TURN:
      if(Timer20ms_Counts >= TURN_180_TIME) {
        Timer20ms_Counts = 0; Timing20ms = false;       // Going to turn now, so need timer running
        temp_distance = Get_Distance();
        
        if(temp_distance <= MIN_DISTANCE) {
          current_state = CCW_90_TURN;
          last_state = CCW_180_TURN;
          Timer20ms_Counts = 0; Timing20ms = true;      // Going to turn now, so need timer running
        }
      
        else {                                          //Okay to continue forward
          current_state = FWD_MOVEMENT;
          last_state = CCW_180_TURN;
        }
      }
    
      else {
        wall_detected = false;
        MotorCCWTurn();
      }
    break;

    case CCW_90_TURN:
      if(Timer20ms_Counts >= TURN_90_TIME) {
        Timer20ms_Counts = 0; Timing20ms = false;       // Going to turn now, so need timer running
        current_state = FWD_MOVEMENT;
        last_state = CCW_90_TURN;
      }
      
      else {
        wall_detected = false;
        last_turn_maneuver = CCW_TURN;
        MotorCCWTurn();
      }

    break;
    
    case HALT_MOVEMENT:
      last_state = HALT_MOVEMENT;
      MotorStop();
    break;

    default:
      MotorFWD(); 
    break;
  }
}

