#include "asf.h"
#include "stdio_serial.h"
#include "conf_board.h"
#include "conf_clock.h"
#include "conf_mcan.h"
#include "pmc.h"
#include "same70_can_driver.h"

void can0_get_message_available(void);
void can1_get_message_available(void);

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

void can0_get_message_available(void)
{
	uint8_t i = 0;
	while(can_available_message(CAN_LINE_0) > 0)
	{
		can_timestamped_rx_message_t time_message;

		can_get_message(CAN_LINE_0, &time_message);

		printf("[%i][%llu]", i, time_message.timestamp);
		i++;
		printf("(%lX)", time_message.rx_message.id);

		for (uint16_t j = 0; j < time_message.rx_message.dlc; j++)
		{
			printf("|0x%02X", time_message.rx_message.data[j]);
		}
		printf("|\r\n");
		
	}
}

void can1_get_message_available(void)
{
	uint8_t i = 0;
	while(can_available_message(CAN_LINE_1) > 0)
	{
		can_timestamped_rx_message_t time_message;

		can_get_message(CAN_LINE_1, &time_message);

		printf("[%i][%llu]", i, time_message.timestamp);
		i++;
		printf("(%lX)", time_message.rx_message.id);

		for (uint16_t j = 0; j < time_message.rx_message.dlc; j++)
		{
			printf("|0x%2X", time_message.rx_message.data[j]);
		}
		printf("|\r\n");
		
	}
}
uint8_t a = 0;

int main(void)
{
	sysclk_init();
	board_init();

	configure_console();
	#define speed CAN_BR_1_Mbps
    can_configure(CAN_LINE_0, speed, 64, 64);
    can_configure(CAN_LINE_1, speed, 64, 64);
	
	SysTick_Config(sysclk_get_cpu_hz() / 1000);
	printf("________________________START_____________________________________\r\n");
	while (1)
	{
		printf("---------------------------------%i--------------------------------\r\n", a);
		a++;
		
		mdelay(500);
		
		can0_get_message_available();
		
		#define data_len 4
		
		for (uint8_t i = 0; i < 64; i++)
		{
			uint8_t st = can_send_message(CAN_LINE_1, i, tx_message, data_len, false, false);
			if(st == CBF_BUFFER_FULL)
			{
				printf("SEND CANCELED BUFFER FULL\r\n");
			}
		}
	
		for (uint32_t i = 0; i < data_len; i++)
		{
			tx_message[i]++;
		}
 	}
}
