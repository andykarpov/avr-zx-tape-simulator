#include <unistd.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;


#pragma GCC diagnostic ignored "-Winvalid-source-encoding"


volatile uint8_t DDRA, DDRB, DDRC, DDRD, DDRE, PORTA, PORTB, PORTC, PORTD, PORTE, PINA, PINB, PINC, PIND, PINE;
volatile uint8_t TIMSK, TCNT0, TCNT1, TCCR1A, TCCR1B, OCR3BL, TCCR3A, TCCR0, OCR1A, ADMUX, ADCSRA, OCR2, ADC, TCCR2, TIFR, ICR1, TCCR3B, TWSR, TWBR, TWCR, TWDR;
volatile uint16_t OCR3B;



void _delay_ms(double __ms) {
	usleep(__ms*1000);	
}

void _delay_us(double __us) {
	usleep(__us);	
}



char* itoa(int value, char *str, int radix) {
	char tmp[16];// be careful with the length of the buffer
    char *tp = tmp;
    int i;
    unsigned v;

    int sign = (radix == 10 && value < 0);    
    if (sign)
        v = -value;
    else
        v = (unsigned)value;

    while (v || tp == tmp)
    {
        i = v % radix;
        v /= radix; // v/=radix uses less CPU clocks than v=v/radix does
        if (i < 10)
          *tp++ = i+'0';
        else
          *tp++ = i + 'a' - 10;
    }

    int len = tp - tmp;

    if (sign) {
        *str++ = '-';
        len++;
    }

    while (tp > tmp)
        *str++ = *--tp;

    return str;
}

#define BUFSIZE (sizeof(long) * 8 + 1)

char *ltoa(long N, char *str, int base) {
      register int i = 2;
      long uarg;
      char *tail, *head = str, buf[BUFSIZE];

      if (36 < base || 2 > base)
            base = 10;                    /* can only use 0-9, A-Z        */
      tail = &buf[BUFSIZE - 1];           /* last character position      */
      *tail-- = '\0';

      if (10 == base && N < 0L) {
            *head++ = '-';
            uarg    = -N;
      }
      else  uarg = N;

      if (uarg) {
            for (i = 1; uarg; ++i) {
                  register ldiv_t r;

                  r       = ldiv(uarg, base);
                  *tail-- = (char)(r.rem + ((9L < r.rem) ?
                                  ('A' - 10L) : '0'));
                  uarg    = r.quot;
            }
      }
      else  *tail-- = '0';

      memcpy(head, ++tail, i);
      return str;
}

void wdt_reset() {
	usleep(100);
}
