#include <3ds.h>
#include <stdio.h>

int display_menu(const char *menu_entries[], const int entries, const char *headerstr)
{
	bool redraw = true;
	u32 kDown;
	int ctr = 0;

	consoleClear();

	while(aptMainLoop())
	{

		hidScanInput();
		kDown = hidKeysDown();

		if (redraw)
		{
			printf("\x1b[0;0H"); //places cursor at top left of screen
			printf("%s\n\n", headerstr);
			for(int i = 0; i < entries; i++)
			{
				printf("%s %s\n", (ctr == i) ? "#" : " ", menu_entries[i]);
			}
			redraw = false;
		}

		if (kDown) redraw = true;

		if (kDown & KEY_A) return ctr;
		else if (kDown & (KEY_B | KEY_START)) break; // Exit

		else if (kDown & KEY_Y) return -2;
		else if (kDown & KEY_X) return -3;

		else if (kDown & KEY_DOWN) ctr++;
		else if (kDown & KEY_UP) ctr--;

		if (ctr >= entries)
		{
			ctr = 0;
			redraw = true;
		}

		else if (ctr < 0)
		{
			ctr = entries-1;
			redraw = true;
		}

		gspWaitForVBlank();
	}

	return -1;
}
