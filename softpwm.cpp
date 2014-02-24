#include <Arduino.h>
#include "softpwm.h"

volatile uint16_t COLORS_PORTB[8] = {0, 0, 0, 0,0,0,0,0}; // pin 50,51,52,53

//volatile int ERRORS_PORTB[4] = {128, 128, 128, 128}; // pin 50,51,52,53

//volatile uint16_t COLORS_PORTL[1] = {0}; //pin 49


volatile uint16_t _gPulse = 0;

volatile uint16_t _debug_count = 0;

SoftPwm::SoftPwm()
{
    //ctor
}

SoftPwm::~SoftPwm()
{
    //dtor
}

ISR(TIMER2_COMPA_vect)
{  //change the 0 to 1 for timer1 and 2 for timer2
   //interrupt commands here
    //_debug_count++;

    uint16_t pulse = _gPulse;

    uint16_t volatile* pColors = COLORS_PORTB; // starting on port 50 here

    DUTY_CYCLE_PORTB_PIN(pulse, *(pColors++), 3);

    DUTY_CYCLE_PORTB_PIN(pulse, *(pColors++), 2);

    DUTY_CYCLE_PORTB_PIN(pulse, *(pColors++), 1);

    DUTY_CYCLE_PORTB_PIN(pulse, *(pColors++), 0);

//    pColors = COLORS_PORTL;

//    DUTY_CYCLE_PORTL_PIN(pulse, *(pColors++), 0);

    pulse++;

    if(pulse > MAX_PULSE)
    {
        pulse = 0;
    }
   _gPulse = pulse;
}

void SoftPwm::Setup()
{
    m_lLastMillis = millis();
/*
    // Timer2 490 hz
    TCCR2A = 0;
    TCCR2B = 0;
    TCNT2 = 0;
    OCR2A = 0;   // toggle after counting to 8
    TCCR2A |= (1 << COM2A0);   // Toggle OC1A on Compare Match.

    TCCR2B = TCCR2B & 0b11111000 | 0x05;

    TCCR2B |= (1 << WGM22);    // CTC mode
    //TCCR2B |= (1 << CS21);     // clock on, no pre-scaler

    TIMSK2 |= (1 << OCIE2A);
*/

    TCCR2A = 0;// set entire TCCR2A register to 0
    //TCCR2B = 0;// same for TCCR2B
    TCNT2  = 0;//initialize counter value to 0
    // set compare match register for 8khz increments
    //OCR2A = 0;// = (1 6*10^6) / (500 * 1024) - 1 (must be <256)
    // turn on CTC mode
    TCCR2A |= (1 << WGM21);
    // Set CS21 bit for 8 prescaler
    TCCR2B = TCCR2B & 0b11111000 | 0x00;

    TCCR2B |= (1 << CS12) | (1 << CS10);
    //TCCR2B = TCCR2B & 0b11111000 | 0x04;
    // enable timer compare interrupt
    TIMSK2 |= (1 << OCIE2A);


    DDRB = DDRB | B00001111; // pin 50 as output pin 50 , 51 is PB3 (port b3)

    PORTB &= ~(B00001111);

    //DDRL = DDRL | B00001111; // pin 50 as output pin 50 , 51 is PB3 (port b3)

    //PORTL &= ~(B00001111);

    /*PORTB &= ~(1 << 3); // set bit 3 on portB to zero
    PORTB &= ~(1 << 2); // set bit 3 on portB to zero
    //sei();*/


}

void SoftPwm::SetBrightness(byte pinNumber, uint16_t brightness)
{
    if(brightness > MAX_PULSE)
    {
        brightness = MAX_PULSE;
    }
    COLORS_PORTB[pinNumber] = brightness;
}

void SoftPwm::Update()
{
    if(millis() - m_lLastMillis > 1000UL)
    {
        m_lLastMillis = millis();

        Serial.print("softpwm: ");
        Serial.println(_debug_count);
        _debug_count = 0;
    }
    //delay(500);

    /*
    Serial.println("hello");
    delay(2000);
    COLORS_PORTB[0] = 0;
    COLORS_PORTB[1] = 0;
    COLORS_PORTB[2] = 0;
    delay(2000);

    for(int i = 0, j = MAX_PULSE; i < MAX_PULSE, j > 0; i++, j--)
    {
        COLORS_PORTB[0] = i;
        COLORS_PORTB[1] = j;
        COLORS_PORTB[2] = i;
        delay(20);
    }


    Serial.print("next loooooop.....");

    for(int i = MAX_PULSE, j = 0; i > 0, j < MAX_PULSE; i--, j++)
    {
        COLORS_PORTB[0] = i;
        COLORS_PORTB[1] = j;
        COLORS_PORTB[2] = i;
        delay(20);

    }
*/
    /*for(int i = 0; i < 500; i++)
    {
        brightness = i;
        delay(50);
    }*/
}
