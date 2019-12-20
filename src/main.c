/**@file main.c
 * @brief Contains main program for holo_display_chr2019
 *
 * @author Joseph Yankel (jpyankel@gmail.com)
 * @date 2019/12/18
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <util/delay.h>
#include "../external/include/light_ws2812.h"

// TODO Move these defines into a header for cleanliness
/**@brief PORTB ID belonging to the WS2812B data pin*/
#define PORTB_DATA ((char)(1 << PB1))

/**@brief PORTB ID belonging to the sleep button input pin*/
#define PORTB_SLEEP ((char)(1 << PB0))

/**@brief Bit vector with 1s representing unused PORTB pins*/
#define PORTB_UNUSED ((char)(~(PORTB_DATA | PORTB_SLEEP)))

/**@brief Pin-Change Interrupt bit corresponding to sleep button input pin
 * @details Setting configures the IO pin to cause a pin-change interrupt when
 *          the pin's value changes.
 */
#define PCINT_SLEEP ((char)(1 << PCINT0))

/**@brief Pin-Change Interrupt Enable bit in General Interrupt Mask
 * @details Setting enables all configured pin-change interrupts
 */
#define GIMSK_PCIE ((char)(1 << PCIE))

/**@brief Number of WS2812B LEDs*/
#define NUM_LEDS 4

/**@brief Maximum color value of any color component in a single LED*/
#define MAX_COMP_VAL 128

/**@brief Delay between color updates in ms*/
#define COLOR_DELAY 256

/**@brief Program state enum
 * @details SLEEP indicates the program is in sleep mode;
 *          ACTIVE indicates the program is running.
 */
typedef enum prog_state {SLEEP, ACTIVE}prog_state;

/**@brief LED light show next target enum
 * @details SLEEP indicates the program is in sleep mode;
 *          ACTIVE indicates the program is running.
 */
typedef enum next_color_target {RED, GREEN, BLUE}next_color_target;

/**@brief Current mode of operation (SLEEP or ACTIVE)
 * @details When 1, the device is ACTIVE; When 0, the device is in SLEEP mode.
 *          We use this to determine when the program should save power.
 */
prog_state volatile state = ACTIVE;

/**@brief pin-change-interrupt 0 (PCINT0) connected to sleep button input
 *
 * toggles program state between "sleep" and "on"
 */
ISR(PCINT0_vect)
{
  // toggle mcu state
  state = !state;

  // the above will apply when we return to our run-loop
}

// TODO Document
int main (void)
{
  // DDRB is already initialized to 0, meaning all pins are defaulted to input

  // to save power, we enable pull-up resistors on all unused inputs
  // in addition, the sleep button uses a pull-up resistor
  PORTB |= PORTB_UNUSED | PORTB_SLEEP;

  // initialize sleep button pin-change-interrupt (PCINT0)
  PCMSK |= PCINT_SLEEP;

  // enable pin-change-interrupts (required for sleep button interrupt)
  GIMSK |= GIMSK_PCIE;

  // configure device for power-saving
  // first, configure sleep mode to "Power-down", which allows only interrupts
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

  // next, disable the ADC since we aren't using it
  power_adc_disable();

  // disable universal serial interface
  power_usi_disable();

  // disable unecessary timers
  power_timer0_disable();
  power_timer1_disable();

  // initialize each LED's intial color to be offset from each other:
  struct cRGB colors[NUM_LEDS];
  next_color_target color_targs[NUM_LEDS];
  for (unsigned char i = 0; i < NUM_LEDS; i++)
  {
    // red is the first target color
    color_targs[i] = RED;

    // offset red color by the maximum LED value / number of leds
    colors[i].r = MAX_COMP_VAL / NUM_LEDS * i;
    colors[i].g = 0;
    colors[i].b = 0;
  }

  // loop indefinitely
  for (;;)
  {
    // disable interrupts until after we have performed one loop's work
    cli();

    if (state == ACTIVE)
    {
      // all LEDs get sent an identical color
      ws2812_setleds(colors, NUM_LEDS);

      // delay a certain time before sending the next color
      _delay_ms(COLOR_DELAY);

      // rotate colors around using a fade
      for (unsigned char i = 0; i < NUM_LEDS; i++)
      {
        switch (color_targs[i])
        {
          case RED:
            if (colors[i].r < MAX_COMP_VAL)
              colors[i].r++;
            if (colors[i].b > 0)
              colors[i].b--;
            if (colors[i].r == MAX_COMP_VAL && colors[i].b == 0)
              color_targs[i] = GREEN;
            break;
          case GREEN:
            if (colors[i].g < MAX_COMP_VAL)
              colors[i].g++;
            if (colors[i].r > 0)
              colors[i].r--;
            if (colors[i].g == MAX_COMP_VAL && colors[i].r == 0)
              color_targs[i] = BLUE;
            break;
          case BLUE:
            if (colors[i].b < MAX_COMP_VAL)
              colors[i].b++;
            if (colors[i].g > 0)
              colors[i].g--;
            if (colors[i].b == MAX_COMP_VAL && colors[i].g == 0)
              color_targs[i] = RED;
            break;
        }
      }
    }
    else
    {
      // code sequence sourced from:
      // http://www.nongnu.org/avr-libc/user-manual/group__avr__sleep.html

      // this code has a very particular timing sequence which sleeps the
      //   processor with brown-out-detection disabled to save power.
      sleep_enable();
      sleep_bod_disable();
      sei();
      sleep_cpu();
      sleep_disable();
    }

    // manually set the status reg I-bit to allow interrupts
    sei();
  }

  return 0;
}
