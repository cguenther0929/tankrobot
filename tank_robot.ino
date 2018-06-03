/*

Note that timing functions like delay uses timer0, while the servo library 
uses timer1.  Also, the PWM on pins 9 and 10 will be controlled by timer 1.
Therefore, the interrupt routine must use timer 2.  Timers 3, 4, and 5 are only
available on Arduino Mega boards.  

You can’t use PWM on Pin 9, 10 when you use the Servo Library on an Arduino.
For Arduino Mega it is a bit more difficult. The timer needed depends on the number of servos.
Each timer can handle 12 servos. For the first 12 servos timer 5 will be used 
(losing PWM on Pin 44,45,46). For 24 Servos timer 5 and 1 will be used 
(losing PWM on Pin 11,12,44,45,46).. For 36 servos timer 5, 1 and 3 will be used
(losing PWM on Pin 2,3,5,11,12,44,45,46).. For 48 servos all 16bit timers 5,1,3 and 4 will be used (losing all PWM pins).

For defining pins, we must use the Arduino pin identifiers.  The best thing to do here is to look up 
the pinout online.

The arduino pro min -- at least according to a few online resources -- is supposed to have an 8MHz oscillator 
and this seems to jive based on testing I've done in the lab.  
Generally, this is a good resource for timers http://maxembedded.com/2011/06/avr-timers-timer2/   

TODO: Latch "robot is charging" until power cycle! It needs to be this way because the charging status goes HI-Z
upon the cell reaching a full charge. 
  
*/


/* DEFINE CONSTANTS */
#define MIN_DISTANCE      8           // Distance in CM when robot is considered to be too close to a wall
#define MIN_ALLOWED_BATV  3.5         // Mimimum allowed battery voltage
#define DESIRED_BAUD      (57600*2)   // For whatever reason, need to multiply by two to get the correct BAUD.  Oscillator actually 16MHz
#define LEDPIN            13
#define TRIGGER_PIN       3  
#define ECHO_PIN          2 
#define n_BAT_CHG         4           // When this is low, the battery is charging 
#define BATSEN            6

#define MC_AIN1           17        // Pins related to motor control  
#define MC_AIN2           16
#define MC_BIN1           15
#define MC_BIN2           14
#define n_MC_STDBY        10
#define MCPWM             9 

#define TURN_90_TIME      (2/.02)     //How many 20ms ticks are needed to make this turn
#define TURN_180_TIME     (4/.02)     //How many 20ms ticks are needed to make this turn
#define BACKUP_TIME       (4/.02)     //How many 20ms ticks are needed to make this turn

#define IND_SLOW_BLINK    0           //Associate the three different blink rates with actual numbers
#define IND_MED_BLINK     1
#define IND_FAST_BLINK    2
#define PWM_VAL           80        // Hardcoded motor PWM value
#define RT_POLARITY       0         // Options here are 0 or 1
#define LT_POLARITY       0         

// FSM states
#define FWD_MOVEMENT    1                         // Normal robot movement as it searches for a wall
#define REV_MOVEMENT    (FWD_MOVEMENT + 1)        // Robot backing up
#define CW_90_TURN      (REV_MOVEMENT + 1)        // Robot making a clockwise turn to the right by 90 deg
#define CCW_180_TURN    (CW_90_TURN + 1)          // Robot making a counter-clockwise turn to the left by 180 deg
#define CCW_90_TURN     (CCW_180_TURN + 1)        // Robot making a counter-clockwise turn to the left by 90 deg
#define HALT_MOVEMENT   (CCW_90_TURN + 1)         // Robot is not allowed to move

//TURN MANEUVERS
#define CCW_TURN        1
#define CW_TURN         (CCW_TURN + 1)      

/* CONTAINERS FOR DATA COMPONENTS */
// Time keeping containers
unsigned int  ticks_20ms        = 0;
unsigned int  ticks_100ms       = 0;
unsigned int  ticks_500ms       = 0;
bool          Time20msFlag      = false;
bool          Time100msFlag     = false;
bool          Time500msFlag     = false;
bool          Time1000msFlag    = false;
bool          Timing20ms        = false;
unsigned int  Timer20ms_Counts  = 0;

// Other data containers
unsigned char current_state       = 0;
unsigned int  last_state          = 0;
unsigned char last_turn_maneuver  = CCW_TURN;
float         anval               = 0;                // variable to read the value from the analog pin
float         bat_voltage         = 0;                // Calculated battery voltage 
int           ctr                 = 0;
long          distance            = 0;
long          temp_distance       = 0;
unsigned char blink_rate          = 0;   
bool          wall_detected       = false;            // Flag is set when a wall is detected 
bool          battery_dead        = false;            // Flag is set when the battery voltage falls below MIN_ALLOWED_BATV
bool          charging            = false;            // This flag will indicate if the battery is charging

void setup() {
  
  Serial.begin(DESIRED_BAUD);             //For the serial monitor
  
  analogReference(DEFAULT);   
  // pinMode(BATSEN,INPUT);   //TODO might need to put this back in
  
  pinMode(MCPWM,OUTPUT);
  pinMode(TRIGGER_PIN,OUTPUT);
  pinMode(ECHO_PIN,INPUT);
  pinMode(n_BAT_CHG,INPUT);
  
  
  pinMode(MC_AIN1,OUTPUT);
  pinMode(MC_AIN2,OUTPUT);
  pinMode(MC_BIN1,OUTPUT);
  pinMode(MC_BIN2,OUTPUT);

  digitalWrite(TRIGGER_PIN, LOW);
    
  // set timer2 interrupt
  // to get this math to work out, I had to use 4MHz as the oscillator frequency
  noInterrupts();                           //Disable interrupts
  TCCR2A = 0;                               // set entire TCCR2A register to 0
  TCCR2B = 0;                               // same for TCCR2B
  TCNT2  = 0;                               //initialize counter value to 0

  // set compare match register for 50Hz increments (20ms interrupt)
  // As mentioned above, to get the math to work out properly, I needed to use 4MHz 
  OCR2A = 78;                                        // = (OSC Freq) / [(Interrupt Freq * Prescaler) - 1] --> (must be < 256)

  TCCR2A |= (1 << WGM21);                            // Turn on CTC mode    
  TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20);  // Yields a prescaler of 1024
  TIMSK2 |= (1 << OCIE2A);                           // Enable timer compare interrupt
  interrupts();                                      // Enable interrupts


  MotorStop ();
  analogWrite(MCPWM,PWM_VAL);
  current_state = FWD_MOVEMENT;                     // Initialize state
  last_turn_maneuver = CCW_TURN;                     // Keep track of the last turn maneuver 
  blink_rate = IND_SLOW_BLINK;                      // Initialize blink rate for LED

}

void loop() {
  
  if(Time20msFlag == true) {
    (Timing20ms == true)?(Timer20ms_Counts++):(Timer20ms_Counts = 0);

    Time20msFlag = false;
    StateEvaluation();
  }

  if(Time100msFlag == true) {
    Time100msFlag = false;
    
    if(blink_rate == IND_FAST_BLINK) {
      digitalWrite(LEDPIN,!digitalRead(LEDPIN));
    }
    
  }

  if(Time500msFlag == true) {
    Time500msFlag = false;
    
    if(blink_rate == IND_MED_BLINK) 
      digitalWrite(LEDPIN,!digitalRead(LEDPIN));

    distance = Get_Distance();
    if (distance < MIN_DISTANCE) {
      wall_detected = true;
      StateEvaluation();
    }
  
  }
  
  if(Time1000msFlag == true) {
    Time1000msFlag = false;
    
    if(blink_rate == IND_SLOW_BLINK) 
      digitalWrite(LEDPIN,!digitalRead(LEDPIN));
    
    anval = analogRead(BATSEN);             //Read battery input voltage
    bat_voltage = anval / 205.298;
    Serial.print("Battery Votlage: "); Serial.println(bat_voltage);
    // if (bat_voltage <= MIN_ALLOWED_BATV && battery_dead != true) {
    //   Serial.println("we dead.");
    //   blink_rate = IND_FAST_BLINK;        
    //   battery_dead = true;                
    //   current_state = HALT_MOVEMENT;

    // }
    // else if (battery_dead == true){
    //   blink_rate = IND_SLOW_BLINK;        
    //   battery_dead = false;                
    //   current_state = new_state = FWD_MOVEMENT;         // Initialize state
    // }

  }

}

long Get_Distance(void) {
  long duration;
  long distance;
  
  noInterrupts();                           //Disable interrupts
  
  // The PING))) is triggered by a HIGH pulse of ~50 microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  //pinMode(pingPin, OUTPUT);
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(75);     
  digitalWrite(TRIGGER_PIN, LOW);

  // The same pin is used to read the signal from the PING): a HIGH pulse
  // whose duration is the time (in microseconds) from the sending of the ping
  // to the reception of its echo off of an object.
  //pinMode(ECHO_PIN, INPUT);

  duration = pulseIn(ECHO_PIN, HIGH, 100000);  //Stopped testing at 100ms
  distance = duration * 0.017;         // This value is in CM
  interrupts();

  return distance;
}

ISR(TIMER2_COMPA_vect){    //timer2 interrupt 
  
  Time20msFlag = true;        //Enter this ISR every 20ms
  
  if(ticks_20ms == 5) {
    ticks_20ms = 0;               //Reset centi-seconds
    Time100msFlag = true;
    
    if(ticks_100ms == 5) {         //See if we need to roll over
      Time500msFlag = true;  
      ticks_100ms = 0;           //Reset the seconds counter

      if(ticks_500ms == 2) {
        Time1000msFlag = true;
        ticks_500ms = 0;
      }
      else {
        ticks_500ms++;
      }

    }
    else {
      ticks_100ms++;            //Increase seconds timeer
    }
  }
  
  else {
      ticks_20ms++;
  }

}




