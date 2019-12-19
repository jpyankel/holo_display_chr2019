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

/**@brief Program state enum
 * @details SLEEP indicates the program is in sleep mode;
 *          ACTIVE indicates the program is running.
 */
typedef enum prog_state {SLEEP, ACTIVE}prog_state;

/**@brief Current mode of operation (SLEEP or ACTIVE)
 * @details When 1, the device is ACTIVE; When 0, the device is in SLEEP mode.
 *          We use this to determine when the program should save power.
 */
prog_state volatile state = 1;

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

  // initialize WS2812B support
  // TODO

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


  // loop indefinitely
  for (;;)
  {
    // disable interrupts until after we have performed one loop's work
    cli();

    if (state == ACTIVE)
    {
      // TODO
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
