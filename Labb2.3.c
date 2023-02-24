/*
 * 	startup.c
 *
 */
__attribute__((naked)) __attribute__((section (".start_section")) )
void startup ( void )
{
__asm__ volatile(" LDR R0,=0x2001C000\n");		/* set stack */
__asm__ volatile(" MOV SP,R0\n");
__asm__ volatile(" BL main\n");					/* call main */
__asm__ volatile(".L1: B .L1\n");				/* never return */



#define PORT_E 0x40021000 //För att enkelt nå Port E.

#define GPIO_MODER ((volatile unsigned int *)PORT_E) //Alla register + deras offset.

#define GPIO_OTYPER ((volatile unsigned short *)PORT_E + 0x04)

//#define GPIO_PUPDR ((volatile unsigned int *)PORT_E + 0x0C)

#define GPIO_IDR_LOW ((volatile unsigned char *)PORT_E + 0x10)

#define GPIO_IDR_HIGH ((volatile unsigned char *)PORT_E + 0x11)

#define GPIO_ODR_LOW ((volatile unsigned char *)PORT_E + 0x14)

#define GPIO_ODR_HIGH ((volatile unsigned char *)PORT_E + 0x15)



#define B_E 0x40 //Enable

#define B_SELECT 4 //Välj ASCII-display (ska alltid vara 1)

#define B_RW 2 //0=Write, 1=Read

#define B_RS 1	//0=Control, 1=Data (styrregister eller dataregister)



#define STK 0xE000E010 //För att enkelt nå SysTick

#define STK_CTRL ((volatile unsigned int *)STK)

#define STK_LOAD ((volatile unsigned int *)STK + 0x04)

#define STK_VAL ((volatile unsigned int *)STK + 0x08)
}

void app_init()
{
	* ( (unsigned long *) 0x40023830) &= 0x18; //Startar klockor port D och E.
	* GPIO_MODER = 0x55555555; //Pinnar 15-0 som utgångar.
	* GPIO_OTYPER = 0x00000000; //Push-pull
}

void delay_250ns(void) //10^-9 * 250 sekunder. MD407:s systemklocka = 168000000.
{
	unsigned short countValue = ((168 / 4) - 1); //-1 behövs enligt boken, s91.
	* STK_CTRL = 0; //Återställ SysTick
	* STK_LOAD = countValue;
	* STK_VAL = 0; //Nollställ räknarregistret.
	* STK_CTRL = 5; //Starta om räknaren, aktiverar bit 2 och 0.
	while((* STK_CTRL & 0x10000) == 1) {} //Bit 16 är 1 om räknaren räknat ned till 0.
	* STK_CTRL = 0; //Återställ SysTick.
}

void delay_mikro(unsigned int us)
{
	while(us--){
		delay_250ns(); //1000ns = 1us
		delay_250ns();
		delay_250ns();
		delay_250ns();
	}
}

void delay_milli(unsigned int ms)
{
	while(ms--){
		delay_mikro(1000); //1000us = 1ms
	}
}

// --------------------- 2.3 --------------------
void ascii_ctrl_bit_set(unsigned char x) //Ettställer bitar.
{
	*GPIO_ODR_LOW |= ( B_SELECT | x ); //
}

void ascii_ctrl_bit_clear(unsigned char x) //Nollställer bitar.
{
	*GPIO_ODR_LOW &= ( B_SELECT | ~x); //
}

void ascii_write_controller(unsigned char byte) //Skriver till displayen
{
	ascii_ctrl_bit_set(B_E);
	*GPIO_ODR_HIGH = byte;
	delay_250ns();
	ascii_ctrl_bit_clear(B_E);
}

void ascii_write_cmd(unsigned char command)
{
	ascii_ctrl_bit_clear(B_RS);
	ascii_ctrl_bit_clear(B_RW);
	ascii_write_controller(command);
}

void ascii_write_data(unsigned char data)
{
	ascii_ctrl_bit_set(B_RS);
	ascii_ctrl_bit_clear(B_RW);
	ascii_write_controller(data);
}

unsigned char ascii_read_controller(void) //Läser från displayen
{
	char c;
	*GPIO_MODER = 0x00005555;
	ascii_ctrl_bit_set(B_E);
	delay_250ns();
	delay_250ns();
	c = *GPIO_IDR_HIGH;
	ascii_ctrl_bit_clear(B_E);
	*GPIO_MODER = 0x55555555;
	return c;
}

unsigned char ascii_read_status(void)
{
	char c;
	ascii_ctrl_bit_set(B_RW); //Sätter RW=1, dvs read.
	ascii_ctrl_bit_clear(B_RS); //Sätter RS=0, dvs control.
	c = ascii_read_controller();
	return c;
}

unsigned char ascii_read_data(void)
{
	char c;
	ascii_ctrl_bit_set(B_RW); //Sätter RW=1, dvs read.
	ascii_ctrl_bit_clear(B_RS); //Sätter RS=1, dvs data.
	c = ascii_read_controller();
	return c;
}

void ascii_init(void){
	ascii_command(56); //Funtion Set, 2 rader, 5x8 punkter
	delay_mikro(100); //Specifik exeveringstid.
	ascii_command(14); //Display Control
	delay_mikro(100);
	ascii_command(1); //Clear Display
	delay_milli(2);
	ascii_command(6); //Entry Mode Set
	delay_mikro(100);
}

void ascii_command(unsigned char command)
{
	while((ascii_read_status() & 0x80)== 0x80 ){} //Väntar tills displayen är redo att emot kommando
	delay_mikro(8); //Latenstid för kommando.
	ascii_write_cmd(command); //Det specifika kommandot.
}

void ascii_gotoxy(int x, int y) //Positionerar markören.
{
	int adress = x-1;
	if(y == 2){
		adress += 0x40;
	}
	ascii_write_cmd(0x80 | adress);
}

void ascii_write_char(char c)
{
	while((ascii_read_status() & 0x80)== 0x80 ){} //Väntar tills displayen är redo att emot kommando
	delay_mikro(8); //Latenstid för kommando.
	ascii_write_data(c);
	delay_mikro(100);
}

int main(void)
{
	char *s;
	char test1[] = "Alfanumerisk";
	char test2[] = "Display - test";

	app_init();
	ascii_init();
	ascii_gotoxy(1, 1);
	s = test1;
	while(*s){
		ascii_write_char(*s++);
	}
	ascii_gotoxy(1, 2);
	s = test2;
	while(*s){
		ascii_write_char(*s++);
	}
	return 0;
}



