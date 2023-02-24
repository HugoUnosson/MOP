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

#define GPIO_ODR_LOW ((volatile unsigned char *)PORT_E + 0x14)

#define GPIO_OTYPER ((volatile unsigned short *)PORT_E + 0x04)

#define STK 0xE000E010 //För att enkelt nå SysTick

#define STK_CTRL ((volatile unsigned int *)STK)

#define STK_LOAD ((volatile unsigned int *)STK + 0x04)

#define STK_VAL ((volatile unsigned int *)STK + 0x08)
}

void app_init()
{
	* ( (unsigned long *) 0x40023830) &= 0x18; //Startar klockor port D och E.
	* GPIO_MODER = 0x00005555; //Pinnar 7-0 som utgångar för Bargraph.
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

void main(void)
{
	unsigned int bargraph;
	app_init();
	while(1){
		* GPIO_ODR_LOW = 0;
		delay_milli(1);
		* GPIO_ODR_LOW = 0xFF;
		delay_milli(1);
	}
}

