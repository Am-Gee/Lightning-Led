#ifndef SOFTPWM_H
#define SOFTPWM_H

#define MAX_PULSE 255

#define DUTY_CYCLE_PORTB_PIN(pulse, bright, pin) if(pulse >= bright)\
 PORTB &= ~(1 << pin);\
    else\
        PORTB |= (1 << pin)

#define DUTY_CYCLE_PORTA_PIN(pulse, bright, pin) if(pulse >= bright)\
 PORTA &= ~(1 << pin);\
    else\
        PORTA |= (1 << pin)

#define DUTY_CYCLE_PORTL_PIN(pulse, bright, pin) if(pulse >= bright)\
 PORTL &= ~(1 << pin);\
    else\
        PORTL |= (1 << pin)

class SoftPwm
{
    public:
        SoftPwm();
        virtual ~SoftPwm();

        void Setup();
        void Update();
        void SetBrightness(byte pinNumber, uint16_t brightness);

    protected:
    private:
        unsigned long m_lLastMillis;

};

#endif // SOFTPWM_H
