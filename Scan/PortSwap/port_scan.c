/* Copyright (C) 2015-2016 by Jacob Alexander
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this file.  If not, see <http://www.gnu.org/licenses/>.
 */

// ----- Includes -----

// Compiler Includes
#include <Lib/ScanLib.h>

// Project Includes
#include <cli.h>
#include <kll_defs.h>
#include <led.h>
#include <print.h>

// USB Includes
#if defined(_at90usb162_) || defined(_atmega32u4_) || defined(_at90usb646_) || defined(_at90usb1286_)
#include <avr/usb_keyboard_serial.h>
#elif defined(_mk20dx128_) || defined(_mk20dx128vlf5_) || defined(_mk20dx256_) || defined(_mk20dx256vlh7_)
#include <arm/usb_dev.h>
#endif

// Interconnect Includes
#include <connect_scan.h>

// Local Includes
#include "port_scan.h"



// ----- Defines -----



// ----- Structs -----



// ----- Function Declarations -----

// CLI Functions
void cliFunc_portCross( char* args );
void cliFunc_portUART ( char* args );
void cliFunc_portUSB  ( char* args );



// ----- Variables -----

uint32_t Port_lastcheck_ms;

// Scan Module command dictionary
CLIDict_Entry( portCross, "Cross interconnect pins." );
CLIDict_Entry( portUSB,   "Swap USB ports manually, forces usb and interconnect to re-negotiate if necessary." );
CLIDict_Entry( portUART,  "Swap interconnect ports." );

CLIDict_Def( portCLIDict, "Port Swap Module Commands" ) = {
	CLIDict_Item( portCross ),
	CLIDict_Item( portUART ),
	CLIDict_Item( portUSB ),
	{ 0, 0, 0 } // Null entry for dictionary end
};



// ----- Functions -----

void Port_usb_swap()
{
	info_print("USB Port Swap");

	// PTA4 - USB Swap
	GPIOA_PTOR |= (1<<4);

	// Re-initialize usb
	// Call usb_configured() to check if usb is ready
	usb_init();
}

void Port_uart_swap()
{
	info_print("Interconnect Line Swap");

	// PTA13 - UART Swap
	GPIOA_PTOR |= (1<<13);
}

void Port_cross()
{
	info_print("Interconnect Line Cross");

	// PTA12 - UART Tx/Rx cross-over
	GPIOA_PTOR |= (1<<12);

	// Reset interconnects
	Connect_reset();
}

// Setup
inline void Port_setup()
{
	// Register Scan CLI dictionary
	CLI_registerDictionary( portCLIDict, portCLIDictName );

	// PTA4 - USB Swap
	// Start, disabled
	GPIOA_PDDR |= (1<<4);
	PORTA_PCR4 = PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(1);
	GPIOA_PCOR |= (1<<4);

	// PTA12 - UART Tx/Rx cross-over
	// Start, disabled
	GPIOA_PDDR |= (1<<12);
	PORTA_PCR12 = PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(1);
	GPIOA_PCOR |= (1<<12);

	// PTA13 - UART Swap
	// Start, disabled
	GPIOA_PDDR |= (1<<13);
	PORTA_PCR13 = PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(1);
	GPIOA_PCOR |= (1<<13);

	// Starting point for automatic port swapping
	Port_lastcheck_ms = systick_millis_count;
}

// Port State processing loop
inline uint8_t Port_scan()
{
	// TODO Add in interconnect line cross

	#define USBPortSwapDelay_ms 1000
	// Wait 1000 ms before checking
	// Only check for swapping after delay
	uint32_t wait_ms = systick_millis_count - Port_lastcheck_ms;
	if ( wait_ms > USBPortSwapDelay_ms )
	{
		// Update timeout
		Port_lastcheck_ms = systick_millis_count;

		// USB not initialized, attempt to swap
		if ( !usb_configured() )
		{
			Port_usb_swap();
		}
	}

	return 0;
}


// Signal from parent Scan Module that available current has changed
// current - mA
void Port_currentChange( unsigned int current )
{
	// TODO - Power savings?
}



// ----- Capabilities -----

void Port_uart_capability( uint8_t state, uint8_t stateType, uint8_t *args )
{
	// Display capability name
	if ( stateType == 0xFF && state == 0xFF )
	{
		print("Port_uart_capability()");
		return;
	}

	// Only only release
	// TODO Analog
	if ( state != 0x03 )
		return;

	Port_uart_swap();
}

void Port_usb_capability( uint8_t state, uint8_t stateType, uint8_t *args )
{
	// Display capability name
	if ( stateType == 0xFF && state == 0xFF )
	{
		print("Port_usb_capability()");
		return;
	}

	// Only only release
	// TODO Analog
	if ( state != 0x03 )
		return;

	Port_usb_swap();
}

void Port_cross_capability( uint8_t state, uint8_t stateType, uint8_t *args )
{
	// Display capability name
	if ( stateType == 0xFF && state == 0xFF )
	{
		print("Port_cross_capability()");
		return;
	}

	// Only only release
	// TODO Analog
	if ( state != 0x03 )
		return;

	Port_cross();
}



// ----- CLI Command Functions -----

void cliFunc_portUART( char* args )
{
	print( NL );
	Port_uart_swap();
}

void cliFunc_portUSB( char* args )
{
	print( NL );
	Port_usb_swap();
}

void cliFunc_portCross( char* args )
{
	print( NL );
	Port_cross();
}
