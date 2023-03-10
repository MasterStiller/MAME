// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
    Atmel 8-bit AVR simulator

*/

#ifndef MAME_CPU_AVR8_AVR8_H
#define MAME_CPU_AVR8_AVR8_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> avr8_device

// Used by core CPU interface
class avr8_device : public cpu_device
{
public:
	// inline configuration helpers
	void set_eeprom_tag(const char *tag) { m_eeprom.set_tag(tag); }

	// fuse configs
	void set_low_fuses(uint8_t byte);
	void set_high_fuses(uint8_t byte);
	void set_extended_fuses(uint8_t byte);
	void set_lock_bits(uint8_t byte);

	// public interfaces
	virtual void update_interrupt(int source);

	// register handling
	void regs_w(offs_t offset, uint8_t data);
	uint8_t regs_r(offs_t offset);
	uint32_t m_shifted_pc;

	// GPIO
	template<uint8_t Port> auto gpio_out() { return m_gpio_out_cb[Port].bind(); }
	template<uint8_t Port> auto gpio_in() { return m_gpio_in_cb[Port].bind(); }

	// ADC
	template<uint8_t Pin> auto adc_in() { return m_adc_in_cb[Pin].bind(); }

protected:
	avr8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, const device_type type, uint32_t address_mask, address_map_constructor internal_map, int32_t num_timers);

	typedef void (avr8_device::*op_func) (uint16_t op);

	op_func m_op_funcs[0x10000];
	int m_op_cycles[0x10000];
	int m_opcycles;
	std::unique_ptr<uint8_t[]> m_add_flag_cache;
	std::unique_ptr<uint8_t[]> m_adc_flag_cache;
	std::unique_ptr<uint8_t[]> m_sub_flag_cache;
	std::unique_ptr<uint8_t[]> m_sbc_flag_cache;
	std::unique_ptr<uint8_t[]> m_bool_flag_cache;
	std::unique_ptr<uint8_t[]> m_shift_flag_cache;

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override;
	virtual uint32_t execute_max_cycles() const noexcept override;
	virtual uint32_t execute_input_lines() const noexcept override;
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// address spaces
	const address_space_config m_program_config;
	const address_space_config m_data_config;
	required_region_ptr<uint8_t> m_eeprom;

	// bootloader
	uint16_t m_boot_size;

	// Fuses
	uint8_t m_lfuses;
	uint8_t m_hfuses;
	uint8_t m_efuses;
	uint8_t m_lock_bits;

	// CPU registers
	uint32_t m_pc;
	uint8_t m_r[0x200];

	// internal timers
	int32_t m_num_timers;
	int32_t m_timer_top[6];
	uint8_t m_timer_increment[6];
	uint16_t m_timer_prescale[6];
	uint16_t m_timer_prescale_count[6];
	int32_t m_wgm1;
	int32_t m_timer1_compare_mode[2];
	uint16_t m_ocr1[3];
	uint16_t m_timer1_count;
	bool m_ocr2_not_reached_yet;

	// GPIO
	void write_gpio(const uint8_t port, const uint8_t data);
	uint8_t read_gpio(const uint8_t port);
	devcb_write8::array<11> m_gpio_out_cb;
	devcb_read8::array<11> m_gpio_in_cb;

	// ADC
	devcb_read16::array<8> m_adc_in_cb;
	emu_timer *m_adc_timer;
	uint16_t m_adc_sample;
	uint16_t m_adc_result;
	uint16_t m_adc_data;
	bool m_adc_first;
	bool m_adc_hold;
	void adc_start_conversion();
	TIMER_CALLBACK_MEMBER(adc_conversion_complete);
	void change_adcsra(uint8_t data);
	void change_adcsrb(uint8_t data);

	// SPI
	bool m_spi_active;
	uint8_t m_spi_prescale;
	uint8_t m_spi_prescale_count;
	int8_t m_spi_prescale_countdown;
	void enable_spi();
	void disable_spi();
	void spi_update_masterslave_select();
	void spi_update_clock_polarity();
	void spi_update_clock_phase();
	void spi_update_clock_rate();
	void change_spcr(uint8_t data);
	void change_spsr(uint8_t data);

	// internal CPU state
	uint32_t m_addr_mask;
	bool m_interrupt_pending;

	// other internal states
	int m_icount;

	// memory access
	inline void push(uint8_t val);
	inline uint8_t pop();
	inline bool is_long_opcode(uint16_t op);

	// utility
	void unimplemented_opcode(uint32_t op);

	// interrupts
	void set_irq_line(uint16_t vector, int state);

	// timers
	void timer_tick();
	void update_timer_clock_source(uint8_t timer, uint8_t selection);
	void update_timer_waveform_gen_mode(uint8_t timer, uint8_t mode);

	// timer 0
	void timer0_tick();
	void changed_tccr0a(uint8_t data);
	void changed_tccr0b(uint8_t data);
	void update_ocr0(uint8_t newval, uint8_t reg);
	void timer0_force_output_compare(int reg);

	// timer 1
	inline void timer1_tick();
	void changed_tccr1a(uint8_t data);
	void changed_tccr1b(uint8_t data);
	void update_timer1_input_noise_canceler();
	void update_timer1_input_edge_select();
	void update_ocr1(uint16_t newval, uint8_t reg);

	// timer 2
	void timer2_tick();
	void changed_tccr2a(uint8_t data);
	void changed_tccr2b(uint8_t data);
	void update_ocr2(uint8_t newval, uint8_t reg);
	void timer2_force_output_compare(int reg);

	// timer 3
	void timer3_tick();
	void changed_tccr3a(uint8_t data);
	void changed_tccr3b(uint8_t data);
	void changed_tccr3c(uint8_t data);
//  void update_ocr3(uint8_t newval, uint8_t reg);
//  void timer3_force_output_compare(int reg);

	// timer 4
	void timer4_tick();
	void changed_tccr4a(uint8_t data);
	void changed_tccr4b(uint8_t data);
	void changed_tccr4c(uint8_t data);
	//void update_ocr4(uint8_t newval, uint8_t reg);
	//void timer4_force_output_compare(int reg);

	// timer 5
	void timer5_tick();
	void changed_tccr5a(uint8_t data);
	void changed_tccr5b(uint8_t data);
//  void update_ocr5(uint8_t newval, uint8_t reg);
//  void timer5_force_output_compare(int reg);

	// ops
	void populate_ops();
	void populate_add_flag_cache();
	void populate_adc_flag_cache();
	void populate_sub_flag_cache();
	void populate_sbc_flag_cache();
	void populate_bool_flag_cache();
	void populate_shift_flag_cache();
	void op_nop(uint16_t op);
	void op_movw(uint16_t op);
	void op_muls(uint16_t op);
	void op_mulsu(uint16_t op);
	void op_fmul(uint16_t op);
	void op_fmuls(uint16_t op);
	void op_fmulsu(uint16_t op);
	void op_cpc(uint16_t op);
	void op_sbc(uint16_t op);
	void op_add(uint16_t op);
	void op_cpse(uint16_t op);
	void op_cp(uint16_t op);
	void op_sub(uint16_t op);
	void op_adc(uint16_t op);
	void op_and(uint16_t op);
	void op_eor(uint16_t op);
	void op_or(uint16_t op);
	void op_mov(uint16_t op);
	void op_cpi(uint16_t op);
	void op_sbci(uint16_t op);
	void op_subi(uint16_t op);
	void op_ori(uint16_t op);
	void op_andi(uint16_t op);
	void op_lddz(uint16_t op);
	void op_lddy(uint16_t op);
	void op_stdz(uint16_t op);
	void op_stdy(uint16_t op);
	void op_lds(uint16_t op);
	void op_ldzi(uint16_t op);
	void op_ldzd(uint16_t op);
	void op_lpmz(uint16_t op);
	void op_lpmzi(uint16_t op);
	void op_elpmz(uint16_t op);
	void op_elpmzi(uint16_t op);
	void op_ldyi(uint16_t op);
	void op_ldyd(uint16_t op);
	void op_ldx(uint16_t op);
	void op_ldxi(uint16_t op);
	void op_ldxd(uint16_t op);
	void op_pop(uint16_t op);
	void op_sts(uint16_t op);
	void op_stzi(uint16_t op);
	void op_stzd(uint16_t op);
	void op_styi(uint16_t op);
	void op_styd(uint16_t op);
	void op_stx(uint16_t op);
	void op_stxi(uint16_t op);
	void op_stxd(uint16_t op);
	void op_push(uint16_t op);
	void op_com(uint16_t op);
	void op_neg(uint16_t op);
	void op_swap(uint16_t op);
	void op_inc(uint16_t op);
	void op_asr(uint16_t op);
	void op_lsr(uint16_t op);
	void op_ror(uint16_t op);
	void op_setf(uint16_t op);
	void op_clrf(uint16_t op);
	void op_ijmp(uint16_t op);
	void op_eijmp(uint16_t op);
	void op_dec(uint16_t op);
	void op_jmp(uint16_t op);
	void op_call(uint16_t op);
	void op_ret(uint16_t op);
	void op_reti(uint16_t op);
	void op_sleep(uint16_t op);
	void op_break(uint16_t op);
	void op_wdr(uint16_t op);
	void op_lpm(uint16_t op);
	void op_elpm(uint16_t op);
	void op_spm(uint16_t op);
	void op_spmzi(uint16_t op);
	void op_icall(uint16_t op);
	void op_eicall(uint16_t op);
	void op_adiw(uint16_t op);
	void op_sbiw(uint16_t op);
	void op_cbi(uint16_t op);
	void op_sbic(uint16_t op);
	void op_sbi(uint16_t op);
	void op_sbis(uint16_t op);
	void op_mul(uint16_t op);
	void op_out(uint16_t op);
	void op_in(uint16_t op);
	void op_rjmp(uint16_t op);
	void op_rcall(uint16_t op);
	void op_ldi(uint16_t op);
	void op_brset(uint16_t op);
	void op_brclr(uint16_t op);
	void op_bst(uint16_t op);
	void op_bld(uint16_t op);
	void op_sbrs(uint16_t op);
	void op_sbrc(uint16_t op);
	void op_unimpl(uint16_t op);

	// address spaces
	address_space *m_program;
	address_space *m_data;
};

// device type definition
DECLARE_DEVICE_TYPE(ATMEGA88,   atmega88_device)
DECLARE_DEVICE_TYPE(ATMEGA168,  atmega168_device)
DECLARE_DEVICE_TYPE(ATMEGA328,  atmega328_device)
DECLARE_DEVICE_TYPE(ATMEGA644,  atmega644_device)
DECLARE_DEVICE_TYPE(ATMEGA1280, atmega1280_device)
DECLARE_DEVICE_TYPE(ATMEGA2560, atmega2560_device)
DECLARE_DEVICE_TYPE(ATTINY15,   attiny15_device)

// ======================> atmega88_device

class atmega88_device : public avr8_device
{
public:
	// construction/destruction
	atmega88_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void atmega88_internal_map(address_map &map);
};

// ======================> atmega168_device

class atmega168_device : public avr8_device
{
public:
	// construction/destruction
	atmega168_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void update_interrupt(int source) override;
	void atmega168_internal_map(address_map &map);
};

// ======================> atmega328_device

class atmega328_device : public avr8_device
{
public:
	// construction/destruction
	atmega328_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void update_interrupt(int source) override;
	void atmega328_internal_map(address_map &map);
};

// ======================> atmega644_device

class atmega644_device : public avr8_device
{
public:
	// construction/destruction
	atmega644_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void update_interrupt(int source) override;
	void atmega644_internal_map(address_map &map);
};

// ======================> atmega1280_device

class atmega1280_device : public avr8_device
{
public:
	// construction/destruction
	atmega1280_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void update_interrupt(int source) override;
	void atmega1280_internal_map(address_map &map);
};

// ======================> atmega2560_device

class atmega2560_device : public avr8_device
{
public:
	// construction/destruction
	atmega2560_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void update_interrupt(int source) override;
	void atmega2560_internal_map(address_map &map);
};

// ======================> atmega88_device

class attiny15_device : public avr8_device
{
public:
	// construction/destruction
	attiny15_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void attiny15_internal_map(address_map &map);
};

/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

enum : uint8_t
{
	AVR8_SREG = 1,
	AVR8_PC,
	AVR8_R0,
	AVR8_R1,
	AVR8_R2,
	AVR8_R3,
	AVR8_R4,
	AVR8_R5,
	AVR8_R6,
	AVR8_R7,
	AVR8_R8,
	AVR8_R9,
	AVR8_R10,
	AVR8_R11,
	AVR8_R12,
	AVR8_R13,
	AVR8_R14,
	AVR8_R15,
	AVR8_R16,
	AVR8_R17,
	AVR8_R18,
	AVR8_R19,
	AVR8_R20,
	AVR8_R21,
	AVR8_R22,
	AVR8_R23,
	AVR8_R24,
	AVR8_R25,
	AVR8_R26,
	AVR8_R27,
	AVR8_R28,
	AVR8_R29,
	AVR8_R30,
	AVR8_R31,
	AVR8_X,
	AVR8_Y,
	AVR8_Z,
	AVR8_SPH,
	AVR8_SPL
};

enum : uint8_t
{
	AVR8_INT_RESET = 0,
	AVR8_INT_INT0,
	AVR8_INT_INT1,
	AVR8_INT_PCINT0,
	AVR8_INT_PCINT1,
	AVR8_INT_PCINT2,
	AVR8_INT_WDT,
	AVR8_INT_T2COMPA,
	AVR8_INT_T2COMPB,
	AVR8_INT_T2OVF,
	AVR8_INT_T1CAPT,
	AVR8_INT_T1COMPA,
	AVR8_INT_T1COMPB,
	AVR8_INT_T1OVF,
	AVR8_INT_T0COMPA,
	AVR8_INT_T0COMPB,
	AVR8_INT_T0OVF,
	AVR8_INT_SPI_STC,
	AVR8_INT_USART_RX,
	AVR8_INT_USART_UDRE,
	AVR8_INT_USART_TX,
	AVR8_INT_ADC,
	AVR8_INT_EE_RDY,
	AVR8_INT_ANALOG_COMP,
	AVR8_INT_TWI,
	AVR8_INT_SPM_RDY,

	// ATMEGA644
	ATMEGA644_INT_RESET = 0,
	ATMEGA644_INT_INT0,
	ATMEGA644_INT_INT1,
	ATMEGA644_INT_INT2,
	ATMEGA644_INT_PCINT0,
	ATMEGA644_INT_PCINT1,
	ATMEGA644_INT_PCINT2,
	ATMEGA644_INT_PCINT3,
	ATMEGA644_INT_WDT,
	ATMEGA644_INT_T2COMPA,
	ATMEGA644_INT_T2COMPB,
	ATMEGA644_INT_T2OVF,
	ATMEGA644_INT_T1CAPT,
	ATMEGA644_INT_T1COMPA,
	ATMEGA644_INT_T1COMPB,
	ATMEGA644_INT_T1OVF,
	ATMEGA644_INT_T0COMPA,
	ATMEGA644_INT_T0COMPB,
	ATMEGA644_INT_T0OVF,
	ATMEGA644_INT_SPI_STC,
	ATMEGA644_INT_USART_RX,
	ATMEGA644_INT_USART_UDRE,
	ATMEGA644_INT_USART_TX,
	ATMEGA644_INT_ADC,
	ATMEGA644_INT_EE_RDY,
	ATMEGA644_INT_ANALOG_COMP,
	ATMEGA644_INT_TWI,
	ATMEGA644_INT_SPM_RDY
};

// Used by I/O register handling
enum : uint16_t
{
	AVR8_REGIDX_R0 = 0x00,
	AVR8_REGIDX_R1,
	AVR8_REGIDX_R2,
	AVR8_REGIDX_R3,
	AVR8_REGIDX_R4,
	AVR8_REGIDX_R5,
	AVR8_REGIDX_R6,
	AVR8_REGIDX_R7,
	AVR8_REGIDX_R8,
	AVR8_REGIDX_R9,
	AVR8_REGIDX_R10,
	AVR8_REGIDX_R11,
	AVR8_REGIDX_R12,
	AVR8_REGIDX_R13,
	AVR8_REGIDX_R14,
	AVR8_REGIDX_R15,
	AVR8_REGIDX_R16,
	AVR8_REGIDX_R17,
	AVR8_REGIDX_R18,
	AVR8_REGIDX_R19,
	AVR8_REGIDX_R20,
	AVR8_REGIDX_R21,
	AVR8_REGIDX_R22,
	AVR8_REGIDX_R23,
	AVR8_REGIDX_R24,
	AVR8_REGIDX_R25,
	AVR8_REGIDX_R26,
	AVR8_REGIDX_R27,
	AVR8_REGIDX_R28,
	AVR8_REGIDX_R29,
	AVR8_REGIDX_R30,
	AVR8_REGIDX_R31,
	AVR8_REGIDX_PINA = 0x20,
	AVR8_REGIDX_DDRA,
	AVR8_REGIDX_PORTA,
	AVR8_REGIDX_PINB,
	AVR8_REGIDX_DDRB,
	AVR8_REGIDX_PORTB,
	AVR8_REGIDX_PINC,
	AVR8_REGIDX_DDRC,
	AVR8_REGIDX_PORTC,
	AVR8_REGIDX_PIND,
	AVR8_REGIDX_DDRD,
	AVR8_REGIDX_PORTD,
	AVR8_REGIDX_PINE,
	AVR8_REGIDX_DDRE,
	AVR8_REGIDX_PORTE,
	AVR8_REGIDX_PINF,
	AVR8_REGIDX_DDRF,
	AVR8_REGIDX_PORTF,
	AVR8_REGIDX_PING,
	AVR8_REGIDX_DDRG,
	AVR8_REGIDX_PORTG,
	AVR8_REGIDX_TIFR0 = 0x35,
	AVR8_REGIDX_TIFR1,
	AVR8_REGIDX_TIFR2,
	AVR8_REGIDX_TIFR3,
	AVR8_REGIDX_TIFR4,
	AVR8_REGIDX_TIFR5,
	AVR8_REGIDX_PCIFR = 0x3B,
	AVR8_REGIDX_EIFR,
	AVR8_REGIDX_EIMSK,
	AVR8_REGIDX_GPIOR0,
	AVR8_REGIDX_EECR,
	AVR8_REGIDX_EEDR,
	AVR8_REGIDX_EEARL,
	AVR8_REGIDX_EEARH,
	AVR8_REGIDX_GTCCR,
	AVR8_REGIDX_TCCR0A,
	AVR8_REGIDX_TCCR0B,
	AVR8_REGIDX_TCNT0,
	AVR8_REGIDX_OCR0A,
	AVR8_REGIDX_OCR0B,
	//0x49: Reserved
	AVR8_REGIDX_GPIOR1 = 0x4A,
	AVR8_REGIDX_GPIOR2,
	AVR8_REGIDX_SPCR,
	AVR8_REGIDX_SPSR,
	AVR8_REGIDX_SPDR,
	//0x4F: Reserved
	AVR8_REGIDX_ACSR = 0x50,
	AVR8_REGIDX_OCDR,
	//0x52: Reserved
	AVR8_REGIDX_SMCR = 0x53,
	AVR8_REGIDX_MCUSR,
	AVR8_REGIDX_MCUCR,
	//0x56: Reserved
	AVR8_REGIDX_SPMCSR = 0x57,
	//0x58: Reserved
	//0x59: Reserved
	//0x5A: Reserved
	AVR8_REGIDX_RAMPZ = 0x5B,
	AVR8_REGIDX_EIND,
	AVR8_REGIDX_SPL,
	AVR8_REGIDX_SPH,
	AVR8_REGIDX_SREG,
//--------------------------
	AVR8_REGIDX_WDTCSR = 0x60,
	AVR8_REGIDX_CLKPR,
	//0x62: Reserved
	//0x63: Reserved
	AVR8_REGIDX_PRR0 = 0x64,
	AVR8_REGIDX_PRR1,
	AVR8_REGIDX_OSCCAL,
	//0x67: Reserved
	AVR8_REGIDX_PCICR = 0x68,
	AVR8_REGIDX_EICRA,
	AVR8_REGIDX_EICRB,
	AVR8_REGIDX_PCMSK0,
	AVR8_REGIDX_PCMSK1,
	AVR8_REGIDX_PCMSK2,
	AVR8_REGIDX_TIMSK0,
	AVR8_REGIDX_TIMSK1,
	AVR8_REGIDX_TIMSK2,
	AVR8_REGIDX_TIMSK3,
	AVR8_REGIDX_TIMSK4,
	AVR8_REGIDX_TIMSK5,
	AVR8_REGIDX_XMCRA,
	AVR8_REGIDX_XMCRB,
	//0x76: Reserved
	//0x77: Reserved
	AVR8_REGIDX_ADCL = 0x78,
	AVR8_REGIDX_ADCH,
	AVR8_REGIDX_ADCSRA,
	AVR8_REGIDX_ADCSRB,
	AVR8_REGIDX_ADMUX,
	AVR8_REGIDX_DIDR2,
	AVR8_REGIDX_DIDR0,
	AVR8_REGIDX_DIDR1,
	AVR8_REGIDX_TCCR1A,
	AVR8_REGIDX_TCCR1B,
	AVR8_REGIDX_TCCR1C,
	//0x83: Reserved
	AVR8_REGIDX_TCNT1L = 0x84,
	AVR8_REGIDX_TCNT1H,
	AVR8_REGIDX_ICR1L,
	AVR8_REGIDX_ICR1H,
	AVR8_REGIDX_OCR1AL,
	AVR8_REGIDX_OCR1AH,
	AVR8_REGIDX_OCR1BL,
	AVR8_REGIDX_OCR1BH,
	AVR8_REGIDX_OCR1CL,
	AVR8_REGIDX_OCR1CH,
	//0x8E: Reserved
	//0x8F: Reserved
	AVR8_REGIDX_TCCR3A = 0x90,
	AVR8_REGIDX_TCCR3B,
	AVR8_REGIDX_TCCR3C,
	//0x93: Reserved
	AVR8_REGIDX_TCNT3L = 0x94,
	AVR8_REGIDX_TCNT3H,
	AVR8_REGIDX_ICR3L,
	AVR8_REGIDX_ICR3H,
	AVR8_REGIDX_OCR3AL,
	AVR8_REGIDX_OCR3AH,
	AVR8_REGIDX_OCR3BL,
	AVR8_REGIDX_OCR3BH,
	AVR8_REGIDX_OCR3CL,
	AVR8_REGIDX_OCR3CH,
	//0x9E: Reserved
	//0x9F: Reserved
	AVR8_REGIDX_TCCR4A = 0xA0,
	AVR8_REGIDX_TCCR4B,
	AVR8_REGIDX_TCCR4C,
	//0xA3: Reserved
	AVR8_REGIDX_TCNT4L = 0xA4,
	AVR8_REGIDX_TCNT4H,
	AVR8_REGIDX_ICR4L,
	AVR8_REGIDX_ICR4H,
	AVR8_REGIDX_OCR4AL,
	AVR8_REGIDX_OCR4AH,
	AVR8_REGIDX_OCR4BL,
	AVR8_REGIDX_OCR4BH,
	AVR8_REGIDX_OCR4CL,
	AVR8_REGIDX_OCR4CH,
	//0xAE: Reserved
	//0xAF: Reserved
	AVR8_REGIDX_TCCR2A = 0xB0,
	AVR8_REGIDX_TCCR2B,
	AVR8_REGIDX_TCNT2,
	AVR8_REGIDX_OCR2A,
	AVR8_REGIDX_OCR2B,
	//0xB5: Reserved
	AVR8_REGIDX_ASSR = 0xB6,
	//0xB7: Reserved
	AVR8_REGIDX_TWBR = 0xB8,
	AVR8_REGIDX_TWSR,
	AVR8_REGIDX_TWAR,
	AVR8_REGIDX_TWDR,
	AVR8_REGIDX_TWCR,
	AVR8_REGIDX_TWAMR,
	//0xBE: Reserved
	//0xBF: Reserved
	AVR8_REGIDX_UCSR0A = 0xC0,
	AVR8_REGIDX_UCSR0B,
	AVR8_REGIDX_UCSR0C,
	//0xC3: Reserved
	AVR8_REGIDX_UBRR0L = 0xC4,
	AVR8_REGIDX_UBRR0H,
	AVR8_REGIDX_UDR0,
	//0xC7: Reserved
	AVR8_REGIDX_UCSR1A = 0xC8,
	AVR8_REGIDX_UCSR1B,
	AVR8_REGIDX_UCSR1C,
	//0xCB: Reserved
	AVR8_REGIDX_UBRR1L = 0xCC,
	AVR8_REGIDX_UBRR1H,
	AVR8_REGIDX_UDR1,
	//0xCF: Reserved
	AVR8_REGIDX_UCSR2A = 0xD0,
	AVR8_REGIDX_UCSR2B,
	AVR8_REGIDX_UCSR2C,
	//0xD3: Reserved
	AVR8_REGIDX_UBRR2L = 0xD4,
	AVR8_REGIDX_UBRR2H,
	AVR8_REGIDX_UDR2,
	//0xD7: Reserved
	//0xD8: Reserved
	//0xD9: Reserved
	//0xDA: Reserved
	//0xDB: Reserved
	//0xDC: Reserved
	//0xDD: Reserved
	//0xDE: Reserved
	//0xDF: Reserved
	//0xE0: Reserved
	//0xE1: Reserved
	//0xE2: Reserved
	//0xE3: Reserved
	//0xE4: Reserved
	//0xE5: Reserved
	//0xE6: Reserved
	//0xE7: Reserved
	//0xE8: Reserved
	//0xE9: Reserved
	//0xEA: Reserved
	//0xEB: Reserved
	//0xEC: Reserved
	//0xED: Reserved
	//0xEE: Reserved
	//0xEF: Reserved
	//0xF0: Reserved
	//0xF1: Reserved
	//0xF2: Reserved
	//0xF3: Reserved
	//0xF4: Reserved
	//0xF5: Reserved
	//0xF6: Reserved
	//0xF7: Reserved
	//0xF8: Reserved
	//0xF9: Reserved
	//0xFA: Reserved
	//0xFB: Reserved
	//0xFC: Reserved
	//0xFD: Reserved
	//0xFE: Reserved
	//0xFF: Reserved
	AVR8_REGIDX_PINH = 0x100,
	AVR8_REGIDX_DDRH,
	AVR8_REGIDX_PORTH,
	AVR8_REGIDX_PINJ,
	AVR8_REGIDX_DDRJ,
	AVR8_REGIDX_PORTJ,
	AVR8_REGIDX_PINK,
	AVR8_REGIDX_DDRK,
	AVR8_REGIDX_PORTK,
	AVR8_REGIDX_PINL,
	AVR8_REGIDX_DDRL,
	AVR8_REGIDX_PORTL,
	//0x10C: Reserved
	//0x10D: Reserved
	//0x10E: Reserved
	//0x10F: Reserved
	//0x110: Reserved
	//0x111: Reserved
	//0x112: Reserved
	//0x113: Reserved
	//0x114: Reserved
	//0x115: Reserved
	//0x116: Reserved
	//0x117: Reserved
	//0x118: Reserved
	//0x119: Reserved
	//0x11A: Reserved
	//0x11B: Reserved
	//0x11C: Reserved
	//0x11D: Reserved
	//0x11E: Reserved
	//0x11F: Reserved
	AVR8_REGIDX_TCCR5A = 0x120,
	AVR8_REGIDX_TCCR5B,
	AVR8_REGIDX_TCCR5C,
	//0x123: Reserved
	AVR8_REGIDX_TCNT5L = 0x124,
	AVR8_REGIDX_TCNT5H,
	AVR8_REGIDX_ICR5L,
	AVR8_REGIDX_ICR5H,
	AVR8_REGIDX_OCR5AL,
	AVR8_REGIDX_OCR5AH,
	AVR8_REGIDX_OCR5BL,
	AVR8_REGIDX_OCR5BH,
	AVR8_REGIDX_OCR5CL,
	AVR8_REGIDX_OCR5CH,
	//0x12E: Reserved
	//0x12F: Reserved
	AVR8_REGIDX_UCSR3A = 0x130,
	AVR8_REGIDX_UCSR3B,
	AVR8_REGIDX_UCSR3C,
	//0x133: Reserved
	AVR8_REGIDX_UBRR3L = 0x134,
	AVR8_REGIDX_UBRR3H,
	AVR8_REGIDX_UDR3
	//0x137: Reserved
	//  .
	//  . up to
	//  .
	//0x1FF: Reserved
};

enum : uint8_t
{
	AVR8_IO_PORTA = 0,
	AVR8_IO_PORTB,
	AVR8_IO_PORTC,
	AVR8_IO_PORTD,
	AVR8_IO_PORTE,
	AVR8_IO_PORTF,
	AVR8_IO_PORTG,
	AVR8_IO_PORTH,
	AVR8_IO_PORTJ,
	AVR8_IO_PORTK,
	AVR8_IO_PORTL
};

enum : uint8_t
{
	AVR8_ADC_ADC0 = 0,
	AVR8_ADC_ADC1,
	AVR8_ADC_ADC2,
	AVR8_ADC_ADC3,
	AVR8_ADC_ADC4,
	AVR8_ADC_ADC5,
	AVR8_ADC_ADC6,
	AVR8_ADC_ADC7
};

enum : uint8_t
{
	AVR8_REG_A = 0,
	AVR8_REG_B,
	AVR8_REG_C,
	AVR8_REG_D,
	AVR8_REG_E,
	AVR8_REG_F,
	AVR8_REG_G,
	AVR8_REG_H,
	AVR8_REG_J,
	AVR8_REG_K,
	AVR8_REG_L
};

enum : uint8_t
{
	AVR8_INTIDX_SPI,

	AVR8_INTIDX_OCF0B,
	AVR8_INTIDX_OCF0A,
	AVR8_INTIDX_TOV0,

	AVR8_INTIDX_ICF1,

	AVR8_INTIDX_OCF1B,
	AVR8_INTIDX_OCF1A,
	AVR8_INTIDX_TOV1,

	AVR8_INTIDX_OCF2B,
	AVR8_INTIDX_OCF2A,
	AVR8_INTIDX_TOV2,

//------ TODO: review this --------
	AVR8_INTIDX_OCF3B,
	AVR8_INTIDX_OCF3A,
	AVR8_INTIDX_TOV3,

	AVR8_INTIDX_OCF4B,
	AVR8_INTIDX_OCF4A,
	AVR8_INTIDX_TOV4,

	AVR8_INTIDX_OCF5B,
	AVR8_INTIDX_OCF5A,
	AVR8_INTIDX_TOV5,
//---------------------------------

	AVR8_INTIDX_COUNT
};

// lock bit masks
enum : uint8_t
{
	LB1 = (1 << 0),
	LB2 = (1 << 1),
	BLB01 = (1 << 2),
	BLB02 = (1 << 3),
	BLB11 = (1 << 4),
	BLB12 = (1 << 5)
};

// extended fuses bit masks
enum : uint8_t
{
	BODLEVEL0 = (1 << 0),
	BODLEVEL1 = (1 << 1),
	BODLEVEL2 = (1 << 2)
};

// high fuses bit masks
enum : uint8_t
{
	BOOTRST = (1 << 0),
	BOOTSZ0 = (1 << 1),
	BOOTSZ1 = (1 << 2),
	EESAVE = (1 << 3),
	WDTON = (1 << 4),
	SPIEN = (1 << 5),
	JTAGEN = (1 << 6),
	OCDEN = (1 << 7)
};

// low fuses bit masks
enum : uint8_t
{
	CKSEL0 = (1 << 0),
	CKSEL1 = (1 << 1),
	CKSEL2 = (1 << 2),
	CKSEL3 = (1 << 3),
	SUT0 = (1 << 4),
	SUT1 = (1 << 5),
	CKOUT = (1 << 6),
	CKDIV8 = (1 << 7)
};

#define AVR8_EEARH_MASK         0x01

#define AVR8_SPSR_SPIF_MASK     0x80
#define AVR8_SPSR_SPIF_SHIFT    7
#define AVR8_SPSR_SPR2X_MASK    0x01

#define AVR8_SPCR_SPIE_MASK     0x80
#define AVR8_SPCR_SPE_MASK      0x40
#define AVR8_SPCR_DORD_MASK     0x20
#define AVR8_SPCR_MSTR_MASK     0x10
#define AVR8_SPCR_CPOL_MASK     0x08
#define AVR8_SPCR_CPHA_MASK     0x04
#define AVR8_SPCR_SPR_MASK      0x03

#endif /* MAME_CPU_AVR8_AVR8_H */
