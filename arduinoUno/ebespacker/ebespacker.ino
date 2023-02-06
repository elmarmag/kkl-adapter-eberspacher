#include <Arduino.h>

#define LOOP_DELAY_US 1000
#define WAIT_DELAY_MS 1600
#define RESET_TIMEOUT_CYCLES 8000
#define WAIT_TIMER_INCREMENT_MS 400
#define OUTPUT_PIN LED_BUILTIN //19
#define RTS_INPUT_PIN 3

unsigned int assertion_timer = 0;
unsigned int wait_timer = 0;
unsigned int transition_cnt = 0;
unsigned int last_rts = 0;

volatile unsigned long timer_count = 0;

void timer_isr() {
  loop1ms();
  //timer_count++;
          //Serial.println(  digitalRead(RTS_INPUT_PIN));

}

void setup() {
  
  pinMode(OUTPUT_PIN, OUTPUT);
  pinMode(8, OUTPUT);
  digitalWrite(8, HIGH);
  pinMode(RTS_INPUT_PIN, INPUT_PULLUP);
  //pinMode(RTS_INPUT_PIN, INPUT);
  
  unsigned int current_rts = digitalRead(RTS_INPUT_PIN);
  last_rts = digitalRead(RTS_INPUT_PIN);
//  Serial.begin(9600);
  
  // Set up Timer1 to run at 1kHz
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;

  OCR1A = 14;            // compare match register 16MHz/1024/1000Hz
  TCCR1B |= (1 << WGM12);   // CTC mode
  TCCR1B |= (1 << CS12) | (1 << CS10);    // 64 prescaler 
  TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
  interrupts();             // enable all interrupts
}
void loop(){


//        Serial.println(  digitalRead(RTS_INPUT_PIN));

}

void doOutput()
{
    if (wait_timer > 0)
    {
        --wait_timer;
    }
    else if (assertion_timer > 0)
    {
        digitalWrite(OUTPUT_PIN, HIGH);
        digitalWrite(8, LOW);
        if (--assertion_timer == 0)
          transition_cnt = 0;
    }
    else
    {
      digitalWrite(OUTPUT_PIN, LOW);
              digitalWrite(8, HIGH);

    }

}



void loop1ms() {
  doOutput();
    
  
  // timer_count++;
  // if (timer_count >= 8000) {
  //   Serial.println("8 second passed");
  //   timer_count = 0;
  // }

  unsigned int current_rts = digitalRead(RTS_INPUT_PIN);
  static unsigned int reset_cnt = 0;
  
  
  
  if (current_rts != last_rts) {
    last_rts = current_rts;
    reset_cnt = 0;
    noInterrupts(); 
    transition_cnt++;
    interrupts(); 
    
    switch (transition_cnt) {
      case 1:
    //    Serial.println("case 1");
        noInterrupts(); 
        wait_timer = WAIT_DELAY_MS;  //1600
        interrupts(); 
        break;
      case 2:
      //Serial.println("case 2");
        assertion_timer = 2400;
        break;
      case 4:
      //Serial.println("case 4");
        noInterrupts(); 
        wait_timer += WAIT_TIMER_INCREMENT_MS; //400
        interrupts(); 
        assertion_timer = 1900;
        break;
      case 6:
      //Serial.println("case 6");
        noInterrupts(); 
        wait_timer += WAIT_TIMER_INCREMENT_MS; //400
        interrupts(); 
        assertion_timer = 3000;
        break;
      default:
        assertion_timer = 0;
        break;
    }
  }
  
  if (reset_cnt++ > RESET_TIMEOUT_CYCLES || transition_cnt > 6) {
    //Serial.println("resetting " + reset_cnt);
    transition_cnt = 0;    
    reset_cnt = 0;
    noInterrupts();
    wait_timer = 0;
    assertion_timer = 0;
    interrupts();
  }

}

ISR(TIMER1_COMPA_vect) {
  timer_isr();
}