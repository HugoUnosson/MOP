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

#define PORT_D 0x40020C00 //För att enkelt nå Port D.

#define GPIO_MODER ((volatile unsigned int *)PORT_D) //Alla register + deras offset.

#define GPIO_PUPDR ((volatile unsigned int *)PORT_D + 0x0C)

#define GPIO_OSPEEDR ((volatile unsigned int *)PORT_D + 0x08)

#define GPIO_OTYPER ((volatile unsigned short *)PORT_D + 0x04)

#define GPIO_ODR_HIGH ((volatile unsigned char *)PORT_D + 0x15)

#define GPIO_ODR_LOW ((volatile unsigned char *)PORT_D + 0x14)

#define GPIO_IDR_HIGH ((volatile unsigned char *)PORT_D + 0x11)
}

void app_init(void)
{
	* ( (unsigned long *) 0x40023830) &= 0x18; //Startar klockor port D och E.
	* GPIO_MODER = 0x55005555; //Pinnar 15-8 (15-12 utgångar) är för Keyboard, 7-0 som utgångar för 7-segmentsdisplay.
	* GPIO_PUPDR = 0x00AA0000; // 11-8 som pull-down för Keyboard,
	* GPIO_OTYPER = 0x00000000; //15-12 som push-pull.
	* GPIO_OSPEEDR = 0x55555555;  //Port D medium speed.
	* GPIO_ODR_HIGH = 0;
	* GPIO_ODR_LOW = 0;
} 

void ActivateRow(unsigned char row){
	switch(row){
		case 1: 
			* GPIO_ODR_HIGH = 0x10; //Sätter pinne 12 till 1 (hur sätter jag till 0??)
			break;
			
		case 2: 
			* GPIO_ODR_HIGH = 0x20; //Sätter pinne 13 till 1
			break;
			
		case 3: 
			* GPIO_ODR_HIGH = 0x40; //Sätter pinne 14 till 1
			break;
	}
}

unsigned int ReadColumn(void){
	unsigned int c = * GPIO_IDR_HIGH; //c = värdet av GPIO_IDR_HIGH.
	if (c & 0x01){ //Kollar om bit 1 har ström.
		return 1;
	}
	if (c & 0x02){ //Kollar om bit 2 har ström.
		return 2;
	} 
	if (c & 0x04){ //Kollar om bit 3 har ström.
		return 3;
	} 
	return 0;
}

unsigned char keyb(void)
{
	unsigned char row;
	unsigned char column;
	
	for (row = 1; row < 4; ++row){ //Itererar över varje rad
		ActivateRow(row); //Aktiverar den valda raden, dvs ger ström till raden.
		column = ReadColumn(); //Läser av vilken kolumn som är nedtryckt.
		if (column > 0 && column < 4){ //Kolumner 1-3 godtas.
			return (3 * (row-1) + column); //Retunerar Keypad värdet.
		}
	}
	return 0xFF;
}

void out7seg(unsigned char c){
	unsigned char segCodes[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x67};
	if (c > 0 && c < 10){
		* GPIO_ODR_LOW = segCodes[c];
	}
	else{
		* GPIO_ODR_LOW = 0;
	}
}

void main(void)
{
	unsigned char c;
	app_init();
	while(1){
		c = keyb();
		out7seg(c);
	}
}

