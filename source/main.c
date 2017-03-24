#include <3ds.h>
#include <stdio.h>

#include "menu.h"
#include "filebrowser/filebrowser.h"

#define GETBIT(var, n) ((var >> n) & 1)
#define BIT_STEP 1
#define BIT_UPDATE_BANKS 2

//Initiate the variables
int  p, r, q, i, PrintBanks=0, i2 = 0, MenuIndex, step=0;
char a[5000], b, o, s[5000], FilePath[262];
PrintConsole topScreen, bottomScreen;
FILE *z;

const char *mode[] =
	{
		"Normal",
		"Step",
		"Update banks",
		"Step + Update banks"
	};

void Wait4key(u32 key)
{
	while(aptMainLoop())
		{
			hidScanInput();
			u32 kDown = (hidKeysDown() & key);
			if(kDown & KEY_A) break;
			if(kDown & KEY_START) break;
			
			gspWaitForVBlank();
		}
}

//Get character input from software keyboard
void getcharinput(char * out)
{
	char mychar[3] = {0};
	static SwkbdState swkbd;
	swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 1, 1);
	swkbdSetValidation(&swkbd, SWKBD_FIXEDLEN, 0, 0);
	swkbdInputText(&swkbd, mychar, 2);
	*out = (mychar[0] & 0x7F);
}

void Step()
{
	if(step)
	{
		consoleSelect(&bottomScreen);
		printf("Press A to continue\n");
		Wait4key(KEY_A);
		consoleSelect(&topScreen);
	}
}

//Shows the value of the banks on the bottom screen (Will probably be replaced by current action later)
void Update_banks(char *in)
{
	if(PrintBanks)
	{
		consoleSelect(&bottomScreen);
		for(i=0;i<5000;i++)
			if(in[i]!=0)
				printf("Bank %u: %u\n", i, in[i]);
		consoleSelect(&bottomScreen);
		for(i=0;i<30;i++)printf("-");
		printf("\n");
		//delay(1000);
		consoleSelect(&topScreen);
		gfxFlushBuffers();
		gfxSwapBuffers();
		gspWaitForVBlank();
	}	
}

//This is all where the magic is done :D
int interpret(char *c)
{
	char *d;
	int update = 0;

	r++;
	while( *c ) {

		switch (update) {
			case BIT_UPDATE_BANKS: Update_banks(a);
			case BIT_STEP: Step(); update = 0; break;
		}

		//if(strchr("<>+-,.[]\n",*c))printf("%c",*c);
		switch(o=1,*c++) {
		
		case '<': p--; update = BIT_STEP; break;
		case '>': p++; update = BIT_STEP; break;
		case '+': a[p]++; update = BIT_UPDATE_BANKS; break;
		case '-': a[p]--; update = BIT_UPDATE_BANKS; break;
		case '.': putchar(a[p]); fflush(stdout); update = BIT_STEP; break;
		case ',': getcharinput(&a[p]); fflush(stdout); update = BIT_UPDATE_BANKS; break;
		case '[':
			for( b=1,d=c; b && *c; c++ )
				b+=*c=='[', b-=*c==']';
			if(!b) {
				c[-1]=0;
				while( a[p] )
					interpret(d);
				c[-1]=']';
				break;
			}
		case ']':
			puts("UNBALANCED BRACKETS"); return 0;
		case '#':
			if(q>2)
				printf("%2d %2d %2d %2d %2d %2d %2d %2d %2d %2d\n%*s\n",
				       *a,a[1],a[2],a[3],a[4],a[5],a[6],a[7],a[8],a[9],3*p+2,"^");
			gfxFlushBuffers();
			gfxSwapBuffers();
			gspWaitForVBlank();
			break;
		default: o=0;
		}
		if( p<0 || p>100) {
			puts("RANGE ERROR");
			return 0;
		}
	}
	r--;
	return 0;
}

int main()
{
	//Basic init stuff
	gfxInitDefault();
	consoleInit(GFX_TOP, &topScreen);
	consoleInit(GFX_BOTTOM, &bottomScreen);
	consoleSelect(&topScreen);
	hidInit();

	MenuIndex = (display_menu(mode, 4, "With which mode do you want to run your script") << 1);
	if (MenuIndex == (-1 << 1)) goto exit;

	// if MenuIndex = 1, only step is set. if = 2, only PrintBanks is set. if = 3, both are set
	step = GETBIT(MenuIndex, BIT_STEP);
	PrintBanks = GETBIT(MenuIndex, BIT_UPDATE_BANKS);

	z=fopen(filebrowser(),"rb"); //let the user go to the file then load it
	//Read and run file
	if(z)
	{
		fseek(z, 0L, SEEK_END);
		size_t size = ftell(z);
		fseek(z, 0L, SEEK_SET);
		fread(s, 1, size, z);
		interpret(s);
	}
	//Prompt the user to exit the app
	consoleSelect(&bottomScreen);
	printf("Press START to exit");
	gfxFlushBuffers();
	gfxSwapBuffers();
	gspWaitForVBlank();
	Wait4key(KEY_START);
	
	exit:
	hidExit();
	gfxExit();
	return 0;
}