void MotorFWD ( void ) {
    digitalWrite(n_MC_STDBY,1);     //Enable motor driver

    //Right Motor FWD
    if(RT_POLARITY) {           
        digitalWrite(MC_AIN1,1);
        digitalWrite(MC_AIN2,0);
    }   
    else {
        digitalWrite(MC_AIN1,0);
        digitalWrite(MC_AIN2,1);
    }  

    //Left Motor FWD
    if(LT_POLARITY) {           
        digitalWrite(MC_BIN1,1);
        digitalWrite(MC_BIN2,0);
    }   
    else {
        digitalWrite(MC_BIN1,0);
        digitalWrite(MC_BIN2,1);
    } 
}

void MotorREV ( void ) {
    digitalWrite(n_MC_STDBY,1);     //Enable motor driver

    //Right Motor REV
    if(RT_POLARITY) {           
        digitalWrite(MC_AIN1,0);
        digitalWrite(MC_AIN2,1);
    }   
    else {
        digitalWrite(MC_AIN1,1);
        digitalWrite(MC_AIN2,0);
    }  

    //Left Motor REV
    if(LT_POLARITY) {           
        digitalWrite(MC_BIN1,0);
        digitalWrite(MC_BIN2,1);
    }   
    else {
        digitalWrite(MC_BIN1,1);
        digitalWrite(MC_BIN2,0);
    } 
}

void MotorCWTurn ( void ) {
    digitalWrite(n_MC_STDBY,1);     //Enable motor driver

    //Right Motor REV
    if(RT_POLARITY) {           
        digitalWrite(MC_AIN1,0);
        digitalWrite(MC_AIN2,1);
    }   
    else {
        digitalWrite(MC_AIN1,1);
        digitalWrite(MC_AIN2,0);
    }  

    //Left Motor FWD
    if(LT_POLARITY) {           
        digitalWrite(MC_BIN1,1);
        digitalWrite(MC_BIN2,0);
    }   
    else {
        digitalWrite(MC_BIN1,0);
        digitalWrite(MC_BIN2,1);
    } 
}

void MotorCCWTurn ( void ) {
    digitalWrite(n_MC_STDBY,1);     // Enable motor driver

    //Right Motor FWD
    if(RT_POLARITY) {           
        digitalWrite(MC_AIN1,1);
        digitalWrite(MC_AIN2,0);
    }   
    else {
        digitalWrite(MC_AIN1,0);
        digitalWrite(MC_AIN2,1);
    }  
    
    //Left Motor REV
    if(LT_POLARITY) {           
        digitalWrite(MC_BIN1,0);
        digitalWrite(MC_BIN2,1);
    }   
    else {
        digitalWrite(MC_BIN1,1);
        digitalWrite(MC_BIN2,0);
    } 
}

void MotorStop ( void ) {
    digitalWrite(n_MC_STDBY,0);     // Disable motor driver
    digitalWrite(MC_AIN1,0);        // Set all drivers to brake 
    digitalWrite(MC_AIN2,0);
    digitalWrite(MC_BIN1,0);
    digitalWrite(MC_BIN2,0);


}
