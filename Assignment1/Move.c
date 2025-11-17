#include <ncurses.h>
#include <string.h> 
#define KEY_ESC 27

int main()
{
	initscr();
	keypad(stdscr, true);
	// don't print out the text the user enters
	noecho();
	// remove the cursor
	curs_set(0);

	// get the size of the terminal
	int maxX, maxY;
	getmaxyx(stdscr, maxY, maxX);

	int curX = 0, curY = 0;

	int curChar;
	while ((curChar = getch()) != KEY_ESC)
	{
		// remove the current x from the screen
		clear();
		
		move(curY, curX);
		printw("X");
		refresh();	
		
		// choose the appropriate direction to move based on the input
		// it wraps around, so go far enough up and you end up on the bottom
		switch (curChar)
		{
			case 'w':
				curY = (curY == 0) ? maxY - 1 : curY - 1;
				break;
			case 'a':
				curX = (curX == 0) ? maxX - 1 : curX - 1;
				break;
			case 's':
				curY = (curY + 1 == maxY) ? 0 : curY + 1;
				break;
			case 'd':
				curX = (curX + 1 == maxX) ? 0 : curX + 1;
				break;
		}
	}

	endwin();
	return 0;
}