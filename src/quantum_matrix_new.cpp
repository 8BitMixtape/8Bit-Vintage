#include <Arduino.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <Adafruit_NeoPixel.h>
#include "matrix_helpers.h"
#include "visuals.h"
#include <neolib.h>
#include "neo_lib_quantum.h"

#ifdef PROGMEM
#undef PROGMEM
#define PROGMEM __attribute__((section(".progmem.data")))
#endif

#define CLOCK_PRESCALER 3
#define F_CPU ( 8000000UL >> CLOCK_PRESCALER )

//Half of sinus:
static const unsigned char g_hsin_tab[ 256 ] PROGMEM =
{
  0, 3, 6, 9, 12, 15, 18, 21, 25, 28, 31, 34, 37, 40, 43, 46,
  49, 52, 56, 59, 62, 65, 68, 71, 74, 77, 80, 83, 86, 89, 92, 95,
  97, 100, 103, 106, 109, 112, 115, 117, 120, 123, 126, 128, 131, 134, 136, 139,
  142, 144, 147, 149, 152, 154, 157, 159, 162, 164, 167, 169, 171, 174, 176, 178,
  180, 183, 185, 187, 189, 191, 193, 195, 197, 199, 201, 203, 205, 207, 209, 211,
  212, 214, 216, 217, 219, 221, 222, 224, 225, 227, 228, 229, 231, 232, 233, 235,
  236, 237, 238, 239, 240, 242, 243, 243, 244, 245, 246, 247, 248, 249, 249, 250,
  251, 251, 252, 252, 253, 253, 254, 254, 254, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 254, 254, 254, 253, 253, 252, 252, 251,
  251, 250, 249, 249, 248, 247, 246, 245, 245, 244, 243, 242, 241, 240, 238, 237,
  236, 235, 234, 232, 231, 230, 228, 227, 225, 224, 222, 221, 219, 218, 216, 214,
  213, 211, 209, 207, 205, 203, 201, 200, 198, 196, 194, 191, 189, 187, 185, 183,
  181, 179, 176, 174, 172, 169, 167, 165, 162, 160, 157, 155, 152, 150, 147, 145,
  142, 139, 137, 134, 131, 129, 126, 123, 120, 118, 115, 112, 109, 106, 104, 101,
  98, 95, 92, 89, 86, 83, 80, 77, 74, 71, 68, 65, 62, 59, 56, 53,
  50, 47, 44, 41, 37, 34, 31, 28, 25, 22, 19, 16, 12, 9, 6, 3
};

static int sin8( int phase );
static unsigned char get_options( void );
static int adc_read();
void adc_init();
void adc_start();
int main();

inline static int sin8( int phase )
{
  int rv;
  phase &= 511;
  if ( phase < 256 )
    rv = (int)pgm_read_byte( &g_hsin_tab[ phase & 255 ] );
  else
    rv = -(int)pgm_read_byte( &g_hsin_tab[ phase & 255 ] );
  return rv;
}

enum
{
  MODE_REPEAT = 0,
  MODE_REPEAT2,
  MODE_REPEAT3,
  MODE_REPEAT_MOD, //fast modulations
  MODE_REPEAT_MOD2, //stream modification: add
  MODE_REPEAT_MOD3, //stream modification: xor
  MODE_REPEAT_MOD4, //stream modification: mul
  MODE_WAVE,
  MODE_SINE,
  MODE_SINE2,
  MODE_MAX,
};

volatile uint16_t pot1; // 0...255 (Slider)
volatile uint16_t pot2; // 0...255 (Pot)
volatile uint16_t pot3; // 0...255 (Buttons)

volatile uint8_t adc1 = _BV(ADLAR) | _BV(MUX0); //PB2-ADC1 pot1 (Slider)
volatile uint8_t adc2 = _BV(ADLAR) | _BV(MUX1); //PB4-ADC2 pot2 (Pot)
volatile uint8_t adc3 = _BV(ADLAR) | _BV(MUX0) | _BV(MUX1); //PB3-ADC3 pot3 (Buttons)

#define ENTER_CRIT()    {byte volatile saved_sreg = SREG; cli()
#define LEAVE_CRIT()    SREG = saved_sreg;}

#define SLOWDOWN( len ) \
  { volatile char d = 0; for( volatile int dd = 0; dd < ( len ); dd++ ) d++; }

#define OUT( bit ) \
  { PORTB = ( PORTB & ~( 1 << output_bit_offset ) ) | ( bit ) << output_bit_offset; }

const int frame_len = 1024;
const int output_bit_offset = 1;

#define OPTION1_BIT ( 1 << 1 )
#define OPTION2_BIT ( 1 << 2 )

static unsigned char get_options( void )
{
  unsigned char rv = 0;
  uint8_t b = wasButtonPressed(pot3);

  if ( b == BUTTON_LEFT                )   rv |= 1;
  if ( b == BUTTON_RIGHT               )    rv |= 2;

  return rv;
}

#define adc_read_begin() ADCSRA |= ( 1 << ADSC )
#define adc_read_finished() ( ( ADCSRA & ( 1 << ADSC ) ) == 0 )
#define adc_get() ADC
static int adc_read() //Get 0...1023 or -512...+511 value from ADC channel
{
  return pot1 + pot2 + pot3;
}

void adc_init()
{
  ADCSRA |= _BV(ADIE); //adc interrupt enable
  ADCSRA |= _BV(ADEN); //adc enable
  ADCSRA |= _BV(ADATE); //auto trigger
  ADCSRA |= _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); //prescale 128
  ADMUX  = adc3;
  ADCSRB = 0;
}

void adc_start()
{
  ADCSRA |= _BV(ADSC); //start adc conversion
}

uint8_t globalColorScheme = 0;

void chaoticPinkVisual(uint8_t intensity, uint8_t potBase) {
  // Update pixels based on audio intensity
  int num_updates = 2 + (intensity >> 5); // 2 to 9 pixels
  
  for(int i=0; i<num_updates; i++) {
      int pixel = rand() % 20;
      
      uint8_t r = 0, g = 0, b = 0;
      uint8_t peak = (intensity > 220) ? (intensity - 220) * 2 : 0;
      
      // Color schemes
      if (globalColorScheme == 0) {
          // Classic Pink/Blue
          r = intensity; 
          g = peak;
          b = potBase; 
      } else if (globalColorScheme == 1) {
          // Matrix Green
          g = intensity;
          r = peak;
          b = peak;
      } else if (globalColorScheme == 2) {
          // Cyberpunk Blue/Cyan
          b = intensity;
          g = potBase;
          r = peak;
      } else if (globalColorScheme == 3) {
          // Hellfire Red/Orange
          r = intensity;
          g = (intensity >> 1) + peak;
          b = 0;
      } else {
          // Neolib ColorWheel based on intensity + pot
          uint32_t c = colorWheel(intensity + potBase);
          r = (c >> 16); g = (c >> 8); b = c;
      }
      
      pixels.setPixelColor(pixel, pixels.Color(r, g, b));
  }
  
  // Clear pixels to create blinking
  int clear_chance = (intensity < 100) ? 2 : 4; 
  for(int i=0; i<5; i++) {
      if ((rand() % clear_chance) == 0) {
         pixels.setPixelColor(rand() % 20, 0);
      }
  }
  
  pixels.show();
}

int main()
{
  unsigned int stream = 0;
  unsigned int stream2 = 0;
  unsigned int emotion_counter = 0;

  clock_prescale_set(clock_div_1); // 16MHz for NeoPixels
  adc_init();
  sei(); //enable global interrupt
  adc_start(); //start adc conversion

  // Initialize NeoPixels
  pixels.begin();
  pixels.setBrightness(255); // Full brightness

  DDRB |= (1 << output_bit_offset); // Set Port B output bit for audio, preserving NeoPixels
  PORTB &= ~(1 << output_bit_offset); // Turn off audio output pin
  
  int btn_timer = 0;  
  int opt_btn = 0;
  uint8_t fq = 4; // Start at 1MHz (clock_div_16)

  while ( 1 )
  {
    // Switch to 16MHz for reliable NeoPixel communication
    clock_prescale_set(clock_div_1);
    chaoticPinkVisual(stream & 255, pot2); // React to audio stream, use Pot 2 for blue base
    // Restore the user-selected clock speed for the audio engine
    CLKPR = 1 << CLKPCE;
    CLKPR = fq;
    
    btn_timer++;

    {
      uint8_t b = getButton(pot3 << 2);
      if ( b == BUTTON_LEFT)   opt_btn |= 1;
      if ( b == BUTTON_NONE)   opt_btn = 0;

      uint8_t c = wasButtonPressed(pot3 << 2);

      if (c == BUTTON_RIGHT){
        globalColorScheme = (globalColorScheme + 1) % 5;
      } else if (c == BUTTON_LEFT) {
        globalColorScheme = (globalColorScheme == 0) ? 4 : globalColorScheme - 1;
      }
      btn_timer = 0;
    }

    unsigned char opt = opt_btn;
    stream2 <<= 1;
    stream2 |= adc_read() & 1;
    
    // OUR NEW INTERFACE: explicitly map the hardware inputs to sound parameters!
    // Slider (pot1) controls slowdown
    unsigned int slowdown = pot1 >> 1; // 0 to 127
    
    // Pot (pot2) controls mode
    unsigned int mode = (pot2 * MODE_MAX) / 256; 
    
    // frame_len2 is influenced by base length + emotion counter sine wave
    unsigned int frame_len2 = ( frame_len + ( sin8( emotion_counter << 3 ) + 255 ) ) / ( ( slowdown >> 1 ) + 1 );
    if ( opt & 1 ) frame_len2 <<= 2;
    
    // Let button left explicitly set to MODE_REPEAT3 if held
    if ( opt & 2 ) mode = MODE_REPEAT3;
    
    emotion_counter++;
    
    switch ( mode )
    {
      case MODE_REPEAT:
        {
          for ( int i = 0; i < frame_len2; i++ )
          {
            stream <<= 1;
            stream |= adc_read() & 1;
            OUT( stream & 1 );
            SLOWDOWN( slowdown );
          }
        }
        break;
      case MODE_REPEAT2:
        {
          for ( int i = 0; i < frame_len2; i++ )
          {
            stream <<= 1;
            stream |= ( adc_read() >> 1 ) & 1;
            OUT( stream & 1 );
            SLOWDOWN( slowdown );
          }
        }
        break;
      case MODE_REPEAT3:
        {
          // We can use pot1 to influence the wave intensity here
          char bb = (pot1 & 7);
          for ( int i = 0; i < frame_len2; i++ )
          {
            stream <<= 1;
            stream |= ( adc_read() >> 1 ) & 1;
            OUT( stream & 1 );
            SLOWDOWN( slowdown + ( ( sin8( i << bb ) + 255 ) >> 6 ) );
          }
        }
        break;
      case MODE_REPEAT_MOD:
        {
          for ( int i = 0; i < frame_len; i++ )
          {
            stream <<= 1;
            stream |= adc_read() & 1;
            OUT( stream & 1 );
            SLOWDOWN( stream & 15 );
          }
        }
        break;
      case MODE_REPEAT_MOD2:
        {
          for ( int i = 0; i < frame_len2; i++ )
          {
            stream <<= 1;
            stream |= adc_read() & 1;
            stream += i;
            OUT( stream & 1 );
            SLOWDOWN( slowdown );
          }
        }
        break;
      case MODE_REPEAT_MOD3:
        {
          for ( int i = 0; i < frame_len2; i++ )
          {
            stream <<= 1;
            stream |= adc_read() & 1;
            stream ^= ( stream >> 8 );
            OUT( stream & 1 );
            SLOWDOWN( slowdown );
          }
        }
        break;
      case MODE_REPEAT_MOD4:
        {
          for ( int i = 0; i < frame_len2; i++ )
          {
            stream <<= 1;
            stream |= adc_read() & 1;
            stream += ( stream >> 8 );
            OUT( stream & 1 );
            SLOWDOWN( slowdown );
          }
        }
        break;
      case MODE_WAVE:
        {
          for ( int i = 0; i < frame_len2 >> 2; i++ )
          {
            OUT( ( stream << ( i & 15 ) ) & 1 );
            SLOWDOWN( slowdown );
          }
        }
        break;
      case MODE_SINE:
        {
          for ( int i = 0; i < frame_len2; i++ )
          {
            stream <<= 1;
            stream |= adc_read() & 1;
            stream += sin8( i );
            OUT( stream & 1 );
            SLOWDOWN( slowdown );
          }
        }
        break;
      case MODE_SINE2:
        {
          for ( int i = 0; i < frame_len2; i++ )
          {
            stream <<= 1;
            stream |= adc_read() & 1;
            stream ^= sin8( i );
            OUT( stream & 1 );
            SLOWDOWN( slowdown );
          }
        }
        break;
    }
  }
}

ISR(ADC_vect)
{
  static uint8_t firstTime = 1;
  static uint8_t val;

  val = ADCH;

  if (firstTime == 1) {
    firstTime = 0;
  }
  else if (ADMUX  == adc1) {
    pot1 = val;
    ADMUX = adc2;
  }
  else if ( ADMUX == adc2) {
    pot2  = val;
    ADMUX = adc3;
  }
  else if ( ADMUX == adc3) {
    pot3  = val;
    ADMUX = adc1;
  }
}
