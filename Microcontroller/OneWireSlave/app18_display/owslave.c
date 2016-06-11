#include "owslave.h"
#include "owutils.h"
#include "ow2431.h"

bank1 byte ow_status; // 0 - waiting for reset, ROM_CMD - waiting for rom command, FUNCTION_CMD - waiting for function command
bank1 byte ow_buffer; // buffer for data received on 1-wire line
bank1 byte timeout; 

volatile bank1 byte OW_serial[8];

#define SetState(state) { ow_status=state; ow_error=0; ow_buffer=0; timeout=200; }

extern unsigned char status;
extern unsigned char spos;

void interrupt ISR(void)
{
  static byte i = 0; 
  // we save some instructions for clearing the variable by setting it to zero when leaving this function 
	
  if ( ow_status == CMD_ROM )
  { 
    byte current_byte; // used by macros
    byte ow_buffer = OW_read_byte_lost10();

    if(ow_error)
      goto RST; // nedava zmysel

    if ( ow_buffer == 0xF0 ) // search rom
    {
      _ASSUME( i == 0 );
      do {
        SEARCH_SEND_BYTE( OW_serial[i] );
      } while (++i < 8);

      SetState(CMD_INIT);
    } else
    if ( ow_buffer == 0x55 ) // match rom
    {
      _ASSUME( i == 0 );
      do {
        SEARCH_MATCH_BYTE( OW_serial[i] );
      } while (++i < 8);
      SetState( i==8 ? CMD_FUNCTION : CMD_INIT);
    } else
    if ( ow_buffer == 0x33 ) // send rom
    {
      _ASSUME( i == 0 );
      do {       
        if ( !OW_write_byte(OW_serial[i]) )
          break;
      } while (++i < 8);

      SetState(CMD_INIT);
    } else
    if ( ow_buffer == 0xCC ) // skip rom
    {
      SetState(CMD_FUNCTION);
    } else
    {
      SetState(CMD_INIT);
    }
    i = 0;
    INT0IF = 0;
    return;
  }

  if ( ow_status == CMD_FUNCTION )
  { 
    byte ow_buffer = OW_read_byte_lost10();

    if ( !Emulate2431(ow_buffer) )
      goto RST;

    SetState(CMD_INIT);
    i = 0;
    INT0IF = 0;
    return;
  }

RST:
  if ( OW_reset_pulse() )
  {
    // if reset detected 
    __delay_us(30);
    OW_presence_pulse(); // generate presence pulse    
    SetState(CMD_ROM);
  } else
  {
    SetState(CMD_INIT);
  }

  INT0IF = 0;
//RB3 = 0;
  return;
}

void OW_setup()
{
  OSCCON = 0b01110001;  /// 8 MHz

  // WDT
  PSA = 1; //prescaler assigned to WDT
  PS0 = 1; PS1 = 1; PS2 = 1; //prescale = 128, WDT period = 2.3 s

  // Disable ADC, enable using GPOI2 as INT
  ANSEL = 0; // all digital pins
//  CMCON0 = 0b00000111;

  // enable weak pull up - no pull up resistor necessary
//  PULLUP_OW = 1; // default:1, all pull ups are enabled!
  RBPU = 0;
//  GPPU = 0;
 
  // clear watchdog
  CLRWDT();

  // configure GPIO change interrupt
  INTEDG = 0; //external interrupt on falling edge
  INT0IE = 1;
}

void OW_start(void)
{
  // init state machine
  SetState(CMD_INIT);
  // enable interrupts
  GIE = 1; 
}

void OW_stop(void)
{
  // disable interrupts
  GIE = 0; 
}

void OW_loop()
{
  if ( timeout ) 
  { 
    byte i;
    byte prevTimeout;

    timeout--;
    prevTimeout = timeout;

    CLRWDT();

    // wait 10 ms or until a request was made
    for (i=40; i && (timeout == prevTimeout); i--)
      DelayUs(250);

    return;
  }

  OW_EnterSleep();
  // go to sleep after 2 second of inactivity
  CLRWDT();
  SLEEP();
  NOP();
  OW_LeaveSleep();
}
