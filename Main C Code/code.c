#include <reg51.h>
#include <reg51.h>

// Define port connections for keypad and 7-segment display
sbit ROW1 = P1^0;
sbit ROW2 = P1^1;
sbit ROW3 = P1^2;
sbit ROW4 = P1^3;
sbit COL1 = P1^4;
sbit COL2 = P1^5;
sbit COL3 = P1^6;
sbit SegEn1 = P0^0;
sbit SegEn2 = P0^1;
sbit SegEn3 = P0^2;
sbit SegEn4 = P0^3;
sbit FREQ = P3^7;
sbit EXT_SW = P3^2;

//Signals of selecting the 7-segment
#define SegOne   0xf1
#define SegTwo   0xf2
#define SegThree 0xf4
#define SegFour  0xf8

//Defines needed
int R0,R1,R2,R3,R4,R5,R6,R7;
int temp,overflow_counter,overflow,state=0;
float needturns;
unsigned char th,tl;

// Define 7-segment display patterns for each key
unsigned char patterns[10] =
{
    0xF9,
    0xA4,
    0xB0,
    0x99,
    0x92,
    0x82,
    0xF8,
    0x80,
    0x90,
    0xC0,
};

// Define keypad array
unsigned char keypad[4][3] =
{
    {1, 2, 3},
    {4, 5, 6},
    {7, 8, 9},
    {0, 0, 0}
};


//Delay Function
void Delay()
{
    //0.28s delay timer for the refreshing
    TMOD=0x02;
    TH0=0xFF;
		TL0=0X00;
    TR0=1;
    while (TF0==0);
    TR0=0;
    TF0=0;
}

//7-segment Refreshing Function
void refresh()
{
    P0 = SegFour;
    P2 = patterns[R0];		// Display key value on 7-segment display
    Delay();

    P0 = SegThree;
    P2 = patterns[R1];		// Display key value on 7-segment display
    Delay();

    P0 = SegTwo;
    P2 = patterns[R2];		// Display key value on 7-segment display
    Delay();

    P0 = SegOne;
    P2 = patterns[R3];		// Display key value on 7-segment display
    Delay();
}

//For delaying the by n*0.28s
void DelayNth(int n)
{
    int i=0;
    for(i=0; i<n; i++)
    {
        Delay();
    }
}

//puting the number to the reg. to select its pattern
void Key_put(int num,int key)
{
    P0 = 0x8F;
    R3=R2;
    R7=R6;
    DelayNth(250);
    R2=R1;
    R6=R5;
    DelayNth(250);
    R1=R0;
    R5=R4;
    DelayNth(250);
    R0=num;
    R4=key;
}

//Function for detecting the pressed key
void key_detect()
{
    unsigned char row, col, key, num;
    while (1)
    {
        // Check row 1
        ROW4 = 1;
        ROW3 = 1;
        ROW2 = 1;
        ROW1 = 0;
        if (COL1 == 0)   // Check column 1
        {
            row = 0;
            col = 0;
            num = 0;
            break;
        }
        else if (COL2 == 0)   // Check column 2
        {
            row = 0;
            col = 1;
            num = 1;
            break;
        }
        else if (COL3 == 0)   // Check column 3
        {
            row = 0;
            col = 2;
            num = 2;
            break;
        }

        // Check row 2
        ROW4 = 1;
        ROW3 = 1;
        ROW1 = 1;
        ROW2 = 0;
        if (COL1 == 0)   // Check column 1
        {
            row = 1;
            col = 0;
            num = 3;
            break;
        }
        else if (COL2 == 0)   // Check column 2
        {
            row = 1;
            col = 1;
            num = 4;
            break;
        }
        else if (COL3 == 0)   // Check column 3
        {
            row = 1;
            col = 2;
            num = 5;
            break;
        }

        // Check row 3
        ROW4 = 1;
        ROW2 = 1;
        ROW1 = 1;
        ROW3 = 0;
        if (COL1 == 0)   // Check column 1
        {
            row = 2;
            col = 0;
            num = 6;
            break;
        }
        else if (COL2 == 0)   // Check column 2
        {
            row = 2;
            col = 1;
            num = 7;
            break;
        }
        else if (COL3 == 0)   // Check column 3
        {
            row = 2;
            col = 2;
            num = 8;
            break;
        }

        // Check row 4
        ROW4 = 0;
        ROW3 = 1;
        ROW2 = 1;
        ROW1 = 1;
        if (COL1 == 0)   // Check column 1
        {
            row = 3;
            col = 0;
            num = 9;
            break;
        }
        else if (COL2 == 0)   // Check column 2
        {
            row = 3;
            col = 1;
            num = 9;
            break;
        }
        else if (COL3 == 0)   // Check column 3
        {
            row = 3;
            col = 2;
            num = 9;
            break;
        }

        ROW1 = ROW2 = ROW3 = ROW4 = 1; // Set all rows high
        refresh();
    }
    key = keypad[row][col]; // Get key value from keypad array
    P0 = 0x0F;
    Key_put(num,key);
}

//Function to calculate the value of TH1,TL1
void cal_overflow()
{
    int value = (R4+R5*10+R6*100+R7*1000)/3+50;
    float time = (1.0/value*1.0)/2.0;
    float needturns = time/(1.085/1000000);
    if(needturns >= 65535)
    {
        overflow = 1;
        while(needturns >= 65535)
        {
            overflow*=2;
            needturns /=2;
        }
        temp=(int)65536-needturns;
        th = (unsigned char)((temp>>8)&0xFF);
        tl = (unsigned char)(temp&0x00FF);
    }
    else
    {
        overflow = 0;
        temp=(int)65536-needturns;
        th = (unsigned char)((temp>>8)&0xFF);
        tl = (unsigned char)(temp&0x00FF);
    }
}

//Timer that take Delay of 0.05msec
void Generate (void) interrupt 3
{
    // timer 1 overflow
    TL1 = tl;
    TH1 = th;
    if(overflow_counter == 0)
    {
        FREQ =~ FREQ;
        TR1 = 0;
        overflow_counter = overflow;
        TL1 = tl;
        TH1 = th;
        TR1 = 1;
    }
    else
        overflow_counter--;
}


void debounce()
{
		unsigned char i = 0;
	  while(	EXT_SW == 0 );		
		for( i = 255; i > 0; i--)
			if ( EXT_SW == 0 )
					i = 255;	
}

//Interrupt Functions
void EXT_INT0 (void) interrupt 0
{
	debounce();
	if(state==0){
    cal_overflow();
    overflow_counter= overflow;
    TL1 = tl;
    TH1 = th;
    TR1 = 1;
    FREQ = 0;
		state=1;
	}else{
		TR1=0;
		TF1=0;
		state=0;
		R0=R1=R2=R3=9;
		R4=R5=R6=R7=0;
	}
}


void main()
{
    TMOD =  (TMOD & 0xF3) | 0x10;			// timer 1 in mode 1 (16 bit counter)
    ET1 = 1;
    EX0 = 1			; // enable EXT interrupt 0
    IT1	= 1  		; // activates on falling edge from 1 to 0
    EA	= 1		  ; //enable global interrupts
		state=0;
    R0=R1=R2=R3=9;
    while(1)
    {
        key_detect();
    }
}
