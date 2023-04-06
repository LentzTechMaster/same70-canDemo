#include "asf.h"
#include "stdio_serial.h"
#include "conf_board.h"
#include "conf_clock.h"
#include "conf_mcan.h"
#include "pmc.h"
#include "same70_can_driver.h"

volatile uint64_t unix_timestamp_ms = 0;
volatile uint32_t counter = 0;

void SysTick_Handler(void)
{
	counter++;
	unix_timestamp_ms++;
}

static void mdelay(uint32_t delay)
{
	uint32_t ticks;
	
	ticks = counter;
	while ((counter - ticks) < delay);
}
/**
 * \brief Configure UART console.
 */
static void configure_console(void)
{
	const usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_UART_BAUDRATE,
#ifdef CONF_UART_CHAR_LENGTH
		.charlength = CONF_UART_CHAR_LENGTH,
#endif
		.paritytype = CONF_UART_PARITY,
#ifdef CONF_UART_STOP_BITS
		.stopbits = CONF_UART_STOP_BITS,
#endif
	};

	/* Configure console UART. */
	sysclk_enable_peripheral_clock(CONSOLE_UART_ID);
	stdio_serial_init(CONF_UART, &uart_serial_options);
}

static uint8_t tx_message[8] = {0b00000000, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
int8_t status_code = 0;

void mcan0_get_message_available()
{
	uint8_t i = 0;
	while(circ_buf_flex_available_elements_to_read(&mcan0_buffer.buffer_rx) > 0)
	{
		mcan_timestamped_rx_message_t time_message;

		circ_buf_flex_pop(&mcan0_buffer.buffer_rx, &time_message);

		printf("[%i][%llu]", i, time_message.timestamp);
		i++;
		printf("(%X)", time_message.rx_message.id);

		for (uint16_t j = 0; j < time_message.rx_message.dlc; j++)
		{
			printf("|0x%2X", time_message.rx_message.data[j]);
		}
		printf("|\r\n");
		
	}
}

void mcan1_get_message_available()
{
	uint8_t i = 0;
	while(circ_buf_flex_available_elements_to_read(&mcan1_buffer.buffer_rx) > 0)
	{
		mcan_timestamped_rx_message_t time_message;

		circ_buf_flex_pop(&mcan1_buffer.buffer_rx, &time_message);

		printf("[%i][%llu]", i, time_message.timestamp);
		i++;
		printf("(%X)", time_message.rx_message.id);

		for (uint16_t j = 0; j < time_message.rx_message.dlc; j++)
		{
			printf("|0x%2X", time_message.rx_message.data[j]);
		}
		printf("|\r\n");
		
	}
}

uint8_t a = 0;

typedef struct test_t {
	void*					pointer;
	uint16_t 				a;
}test_t;

typedef struct a_t {
	uint16_t 				a;
	uint16_t 				a2[16];
}a_t;


int main(void)
{
	sysclk_init();
	board_init();

	configure_console();
    configure_mcan0();
    configure_mcan1();
	
	SysTick_Config(sysclk_get_cpu_hz() / 1000);
	printf("________________________START_____________________________________\r\n");
	//mcan_tx_transfer_request(&mcan1_instance, 1<<1);
	test_t t;
	while (1)
	{
		printf("---------------------------------%i--------------------------------\r\n", a);
		a++;
		
		printf("[%i][%i]\r\n", mcan0_buffer.buffer_being_emptied_by_interruption, mcan0_buffer.interruption_occured_while_adding_in_tx_buffer);
		uint32_t reg;
		
		//reg = mcan_tx_get_fifo_queue_status(&mcan1_instance);
		//printf("1FREE %i[%i]: %i:%i\r\n", reg&0b111111, GET_BITS(reg, 21, 21),GET_BITS(reg, 8, 12), GET_BITS(reg, 16, 20));
		mdelay(50);
		uint32_t status = mcan_tx_get_fifo_queue_status(&mcan1_instance);
		bool is_full = (status& (0x1u<<25))>>25;
		uint32_t fifo_put_index = (status & MCAN_TXEFS_EFPI_Msk)>>MCAN_TXEFS_EFPI_Pos;
		uint32_t fifo_get_index = (status & MCAN_TXEFS_EFGI_Msk)>>MCAN_TXEFS_EFGI_Pos;
		uint32_t fifo_fill_level = (status & MCAN_TXEFS_EFFL_Msk)>>MCAN_TXEFS_EFFL_Pos;
		
		reg = mcan_tx_get_fifo_queue_status(&mcan1_instance);
		//printf("2FREE %i[%i]: %i:%i\r\n", reg&0b111111, GET_BITS(reg, 21, 21), GET_BITS(reg, 8, 12), GET_BITS(reg, 16, 20));
		//printf("FIFO %iG  %iP %iF\r\n", fifo_get_index, fifo_put_index, mcan0_message_transmited);
		mcan1_get_message_available();
		
		

		#define data_len 4
		for (uint8_t i = 0; i < 64; i++)
		{
			mcan0_send_message(i, tx_message, data_len, true, false);
		}
		
		//_mcan_send_standard_message(&mcan1_instance, 0b00000010100, tx_message, data_len);
		
		reg = mcan_tx_get_fifo_queue_status(&mcan1_instance);
		//printf("3FREE %i[%i]: %i:%i\r\n", reg&0b111111, GET_BITS(reg, 21, 21), GET_BITS(reg, 8, 12), GET_BITS(reg, 16, 20));
		for (uint32_t i = 0; i < data_len; i++)
		{
			tx_message[i]++;
		}
		



 	}
}
