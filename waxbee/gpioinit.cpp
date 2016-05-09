#include <avr/io.h>
#include <avr/pgmspace.h>

#include "console.h"
#include "extdata.h"
#include "frequency.h"
#include "strings.h"

namespace GpioInit
{
	static uint16_t parseNumber(uint8_t* str, uint8_t len)
	{
		uint8_t* last = str + len - 1;
	
		uint16_t number = 0;
	
		while(str <= last)
		{
			number = (number * 10) + ((*str)-'0');
			str++;
		}

		return number;
	}

#ifdef DEBUG_SUPPORT
#define REPORTGPIOERRORP(x) reportGpioErrorP(x)
#else
#define REPORTGPIOERRORP(x)
#endif

#ifdef DEBUG_SUPPORT
	static void reportGpioErrorP(const prog_char* progmem_str)
	{
		CONSOLE_PRINTP(STR_GPIO_INIT);
		CONSOLE_PRINTP(progmem_str);
	}
#endif

	static void reportGpioInvalidCommand()
	{
		REPORTGPIOERRORP(STR_INVALID_PORT_COMMAND);
	}
	
	/** The 3 character commands are of the form:  
	 *	D4+
	 *	C7~
	 *	F3^
	 */
	static bool processPortCommand(uint8_t* cmd, uint8_t len, bool errors_only)
	{
		if(len != 3)
		{
			reportGpioInvalidCommand();
			return false;
		}
	
		volatile uint8_t* portData;
		volatile uint8_t* portDir;
		
		switch(cmd[0])
		{
			case 'B':
				portData = &PORTB;
				portDir = &DDRB;
				break;
			case 'C':
				portData = &PORTC;
				portDir = &DDRC;
				break;
			case 'D':
				portData = &PORTD;
				portDir = &DDRD;
				break;
			case 'E':
				portData = &PORTE;
				portDir = &DDRE;
				break;
			case 'F':
				portData = &PORTF;
				portDir = &DDRF;
				break;
			default:
				reportGpioInvalidCommand();
				return false;
		}
	
		uint8_t bitmask = (1 << (cmd[1]-'0'));
		
		if(errors_only)
		{
			switch(cmd[2])
			{
				case '+': // drive high
				case '-': // drive low
				case '~': // input -- no pullup (floating)
				case '^': // input -- with pullup (this will work only if PUD (in MCUCR) is 1)
					return true;
				default:
					reportGpioInvalidCommand();
					return false;
			}
		}

		switch(cmd[2])
		{
			case '+': // drive high
				*portData |= bitmask;
				*portDir |= bitmask;
				return true;
			case '-': // drive low
				*portData &= ~bitmask;
				*portDir |= bitmask;
				return true;
			case '~': // input -- no pullup (floating)
				*portData &= ~bitmask;
				*portDir &= ~bitmask;
				return true;
			case '^': // input -- with pullup (this will work only if PUD (in MCUCR) is 1)
				*portData |= bitmask;
				*portDir &= ~bitmask;
				return true;
			default:
				reportGpioInvalidCommand();
				return false;
		}
	}
	
	static bool processGpioCommand(uint8_t* cmd, uint8_t len, bool errors_only)
	{
		console::print("\nCmd(");
		console::printNumber(len);
		console::print(")='");
		for(uint8_t i = 0; i<len; i++)
			console::print(cmd[i]);
		console::println("'");

		switch(cmd[0])
		{
			case 'P':
			{
				if(len < 2)
				{
					REPORTGPIOERRORP(STR_INVALID_PAUSE_COMMAND);
					return false;
				}
				
				// pause for n milliseconds
				if(!errors_only)
				{
					uint16_t n = parseNumber(cmd+1, len-1);
					Frequency::delay_ms(n);
				}
				return true;
			}
			case 'B':
			case 'C':
			case 'D':
			case 'E':
			case 'F':
				return processPortCommand(cmd, len, errors_only);
			default:
				REPORTGPIOERRORP(STR_ERROR_PARSING_INIT_STRING);
				CONSOLE_PRINTP(STR_BAD_CHAR);
				return false; // error, abort
		}
	
		return true;
	}
	
	bool customGpioInit(bool errors_only)
	{
		uint16_t gpioinit = extdata_getAddress(EXTDATA_GPIO_INIT);
		uint8_t length = extdata_getLength8(EXTDATA_GPIO_INIT);
	
		uint8_t command[8];
		uint8_t cmdlen = 0;
	
		for(uint8_t index = 0 ; index < length; index++)
		{
			uint8_t c = pgm_read_byte(gpioinit + index);
	
			if(c != ',')
			{
				command[cmdlen] = c;
				cmdlen++;
				if(cmdlen > 8)
				{
					REPORTGPIOERRORP(STR_ERROR_PARSING_INIT_STRING);
					CONSOLE_PRINTP(STR_CMD_TOO_LONG_POS);
					console::printNumber(index);
					console::println(")");
					return false;
				}
			}

			if(c == ',' || index+1 == length)
			{
				if(!processGpioCommand(command, cmdlen, errors_only))
				{
					console::print(" (pos = ");
					console::printNumber(index);
					console::println(")");
					return false; // fatal error
				}
				cmdlen = 0;
				continue;
			}
		}

		return true;
	}
}
