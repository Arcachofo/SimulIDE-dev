//#include "sim_avr.h"
#include "sim_core_declare.h"
#include "avr_eeprom.h"
#include "avr_flash.h"
#include "avr_watchdog.h"
#include "avr_extint.h"
#include "avr_ioport.h"
#include "avr_uart.h"
//#include "avr_adc.h"
#include "avr_timer.h"
#include "avr_spi.h"
#include "avr_acomp.h"

void m8515_init(struct avr_t * avr);
void m8515_reset(struct avr_t * avr);

#define _AVR_IO_H_
#define __ASSEMBLER__
#include "avr/iom8515.h"

#ifndef EFUSE_DEFAULT
#define EFUSE_DEFAULT 0xff
#endif
#define EICRA MCUCR
#define EIMSK GICR
#define EIFR GIFR

const struct mcu_t {
	avr_t          core;
	avr_eeprom_t 	eeprom;
	avr_flash_t 	selfprog;
	avr_watchdog_t	watchdog;
	avr_extint_t	extint;
	avr_ioport_t	porta, portb, portc, portd, porte;
	avr_uart_t		uart;
	avr_acomp_t		acomp;
	avr_timer_t		timer0,timer1;
	avr_spi_t		spi;
} mcu_mega8515 = {
	.core = {
		.mmcu = "atmega8515",
		DEFAULT_CORE(2),

		.init = m8515_init,
		.reset = m8515_reset,

	},
	AVR_EEPROM_DECLARE_NOEEPM(EE_RDY_vect),
	AVR_SELFPROG_DECLARE(SPMCR, SPMEN, SPM_RDY_vect),
	AVR_WATCHDOG_DECLARE_128(WDTCR, _VECTOR(0)),
	.extint = {
		AVR_EXTINT_DECLARE(0, 'D', PD2),
		AVR_EXTINT_DECLARE(1, 'D', PD3),
		AVR_ASYNC_EXTINT_DECLARE(2, 'E', PE0),
	},
	AVR_IOPORT_DECLARE(a, 'A', A),
	AVR_IOPORT_DECLARE(b, 'B', B),
	AVR_IOPORT_DECLARE(c, 'C', C),
	AVR_IOPORT_DECLARE(d, 'D', D),
	AVR_IOPORT_DECLARE(e, 'E', E),

	// no PRUSART
	AVR_UART_DECLARE(0, 0, PE, , ),

	.acomp = {
/*		.mux_inputs = 0,
		.mux = { AVR_IO_REGBIT(0, 0), AVR_IO_REGBIT(0, 0),
				AVR_IO_REGBIT(0, 0) },
		.aden = AVR_IO_REGBIT(0, 0),
		.acme = AVR_IO_REGBIT(0, 0),*/

		.r_acsr = ACSR,
		.acis = { AVR_IO_REGBIT(ACSR, ACIS0), AVR_IO_REGBIT(ACSR, ACIS1) },
		.acic = AVR_IO_REGBIT(ACSR, ACIC),
		.aco = AVR_IO_REGBIT(ACSR, ACO),
		.acbg = AVR_IO_REGBIT(ACSR, ACBG),
		.disabled = AVR_IO_REGBIT(ACSR, ACD),

		.timer_name = '1',

		.ac = {
			.enable = AVR_IO_REGBIT(ACSR, ACIE),
			.raised = AVR_IO_REGBIT(ACSR, ACI),
			.vector = ANA_COMP_vect,
		}
	},

	.timer0 = {
		.name = '0',
        .wgm = { AVR_IO_REGBIT( TCCR0, WGM00 )
               , AVR_IO_REGBIT( TCCR0, WGM01 ) },
		.wgm_op = {
            [0] = WGM_NORMAL8(),
            [1] = WGM_FCPWM_8(),
            [2] = WGM_CTC_OC(),
            [3] = WGM_FASTPWM_8(),
		},
		.cs = { AVR_IO_REGBIT(TCCR0, CS00), AVR_IO_REGBIT(TCCR0, CS01), AVR_IO_REGBIT(TCCR0, CS02) },
		//		.cs_div = { 0, 0, 3 /* 8 */, 6 /* 64 */, 8 /* 256 */, 10 /* 1024 */ },
		.cs_div = { 0, 0, 3 /* 8 */, 5 /* 32 */, 6 /* 64 */, 7 /* 128 */, 8 /* 256 */, 10 /* 1024 */},

		.r_tcnt = TCNT0,

		.overflow = {
			.enable = AVR_IO_REGBIT(TIMSK, TOIE0),
			.raised = AVR_IO_REGBIT(TIFR, TOV0),
			.vector = TIMER0_OVF_vect,
		},
		.comp = {
			[AVR_TIMER_COMPA] = {
				.r_ocr = OCR0,
				.com = AVR_IO_REGBITS(TCCR0, COM00, 0x3),
				.com_pin = AVR_IO_REGBIT(PORTB, PB0),
				.interrupt = {
					.enable = AVR_IO_REGBIT(TIMSK, OCIE0),
					.raised = AVR_IO_REGBIT(TIFR, OCF0),
					.vector = TIMER0_COMP_vect,
				},
			},
		},
	},
	.timer1 = {
		.name = '1',
		.wgm = { AVR_IO_REGBIT(TCCR1A, WGM10), AVR_IO_REGBIT(TCCR1A, WGM11),
					AVR_IO_REGBIT(TCCR1B, WGM12), AVR_IO_REGBIT(TCCR1B, WGM13) },
		.wgm_op = {
            [0]  = WGM_NORMAL16(),
            [1]  = WGM_FCPWM_8(),
            [2]  = WGM_FCPWM_9(),
            [3]  = WGM_FCPWM_10(),
            [4]  = WGM_CTC_OC(),
            [5]  = WGM_FASTPWM_8(),
            [6]  = WGM_FASTPWM_9(),
            [7]  = WGM_FASTPWM_10(),
            [8]  = WGM_FCPWM_IC(),
            [9]  = WGM_FCPWM_OC(),
            [10] = WGM_FCPWM_IC(),
            [11] = WGM_FCPWM_OC(),
            [12] = WGM_CTC_IC(),
            [14] = WGM_FASTPWM_IC(),
            [15] = WGM_FASTPWM_OC(),
		},
		.cs = { AVR_IO_REGBIT(TCCR1B, CS10), AVR_IO_REGBIT(TCCR1B, CS11), AVR_IO_REGBIT(TCCR1B, CS12) },
        .cs_div = { 0, 0, 3 /* 8 */, 6 /* 64 */, 8 /* 256 */, 10 /* 1024 */, EXTCLK_CHOOSE, EXTCLK_CHOOSE },
		.ext_clock_pin = AVR_IO_REGBIT(PORTB, 1), /* External clock pin */

		.r_tcnt = TCNT1L,
		.r_icr = ICR1L,
		.r_icrh = ICR1H,
		.r_tcnth = TCNT1H,

		.ices = AVR_IO_REGBIT(TCCR1B, ICES1),
		.icp = AVR_IO_REGBIT(PORTD, 4),

		.overflow = {
			.enable = AVR_IO_REGBIT(TIMSK, TOIE1),
			.raised = AVR_IO_REGBIT(TIFR, TOV1),
			.vector = TIMER1_OVF_vect,
		},
		.icr = {
			.enable = AVR_IO_REGBIT(TIMSK, TICIE1),
			.raised = AVR_IO_REGBIT(TIFR, ICF1),
			.vector = TIMER1_CAPT_vect,
		},
		.comp = {
			[AVR_TIMER_COMPA] = {
				.r_ocr = OCR1AL,
				.r_ocrh = OCR1AH,	// 16 bits timers have two bytes of it
				.com = AVR_IO_REGBITS(TCCR1A, COM1A0, 0x3),
				.com_pin = AVR_IO_REGBIT(PORTB, PB5),
				.interrupt = {
					.enable = AVR_IO_REGBIT(TIMSK, OCIE1A),
					.raised = AVR_IO_REGBIT(TIFR, OCF1A),
					.vector = TIMER1_COMPA_vect,
				},
			},
			[AVR_TIMER_COMPB] = {
				.r_ocr = OCR1BL,
				.r_ocrh = OCR1BH,
				.com = AVR_IO_REGBITS(TCCR1A, COM1B0, 0x3),
				.com_pin = AVR_IO_REGBIT(PORTB, PB6),
				.interrupt = {
					.enable = AVR_IO_REGBIT(TIMSK, OCIE1B),
					.raised = AVR_IO_REGBIT(TIFR, OCF1B),
					.vector = TIMER1_COMPB_vect,
				},
			},
		},

	},
	AVR_SPI_DECLARE(0, 0, 'B', 1, 3, 2, 0),

};

static avr_t * make()
{
	return avr_core_allocate(&mcu_mega8515.core, sizeof(struct mcu_t));
}

avr_kind_t mega8515 = {
        .names = { "atmega8515", "atmega8515L" },
        .make = make
};

void m8515_init(struct avr_t * avr)
{
	struct mcu_t * mcu = (struct mcu_t*)avr;
	
	avr_eeprom_init(avr, &mcu->eeprom);
	avr_flash_init(avr, &mcu->selfprog);
	avr_extint_init(avr, &mcu->extint);
	avr_watchdog_init(avr, &mcu->watchdog);
	avr_ioport_init(avr, &mcu->porta);
	avr_ioport_init(avr, &mcu->portb);
	avr_ioport_init(avr, &mcu->portc);
	avr_ioport_init(avr, &mcu->portd);
	avr_ioport_init(avr, &mcu->porte);
	avr_uart_init(avr, &mcu->uart);
	avr_acomp_init(avr, &mcu->acomp);
	avr_timer_init(avr, &mcu->timer0);
	avr_timer_init(avr, &mcu->timer1);
	avr_spi_init(avr, &mcu->spi);
}

void m8515_reset(struct avr_t * avr)
{
//	struct mcu_t * mcu = (struct mcu_t*)avr;
}
