
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <fcntl.h>

/* Define OS-specific constants */
#if defined(_WIN32)

#include "../include/curses.h"
#include <io.h>
#define KBD_ENTER 13
#define KBD_BACKSPACE 8

#elif defined(unix) || defined(__unix__) || defined(__unix) || defined(__APPLE__)

#include <ncurses.h>
#include <unistd.h>
#define KBD_ENTER 10
#define KBD_BACKSPACE 127

/* Include custom headers */
#include "../include/network.h"

#endif

/* Define application constants */
// Max default scrollback
#define SCROLLBACK 9999

// Prompt to show
#define SEASHELL "seashell>"

// Current command string
char *cmd = NULL;
// Length of current command
size_t cmdLen = 0;

// I/O buffer used in displaying text
struct buffer
{
	char **input;
	char **output;
	int n;
} * buffer;

size_t curLine = 0;

// Screen size
size_t maxRow, maxCol = 0;
// Cursor position
size_t x, y = 0;
// Last keypress
int currCh;

void updateStatusbar(char *str, signed int offset, bool redrawBorders)
{
	init_pair(1, COLOR_CYAN, COLOR_BLACK);
	// attron(COLOR_PAIR(1));

	// Redraw border
	if (redrawBorders)
	{
		border(L'│', L'│', L'─', L'─', L'╔', L'╗', L'╚', L'╝');
	}

	// Go to end, where statusbar is
	if (offset >= 0)
	{
		move(maxRow - 1, 2 + offset);
	}
	else
	{
		move(maxRow - 1, maxCol - 2 + offset - strlen(str));
	}

	// Print string
	printw(str);

	// attroff(COLOR_PAIR(1));

	// Go back to where (y, x) were before
	move(y, x);
}

// Prints information in statusbar
void printInfo()
{
	// Get string of 'y' coordinates
	int coordsLenY = snprintf(NULL, 0, " Row %3.d ", (int)y);
	char *coordsY = malloc(coordsLenY + 1);
	snprintf(coordsY, coordsLenY + 1, " Row %3.d ", (int)y);

	// Print in statusbar
	updateStatusbar(coordsY, -30, TRUE);

	free(coordsY);

	// Get string of 'x' coordinates
	int coordsLenX = snprintf(NULL, 0, " Col %3.d ", (int)x);
	char *coordsX = malloc(coordsLenX + 1);
	snprintf(coordsX, coordsLenX + 1, " Col %3.d ", (int)x);

	// Print in statusbar
	updateStatusbar(coordsX, -17, FALSE);

	free(coordsX);

	// Get currCh value
	int charkeyLen = snprintf(NULL, 0, " (key %d) ", currCh);
	char *charkey = malloc(charkeyLen + 1);
	snprintf(charkey, charkeyLen + 1, " (key %d) ", currCh);

	// Print in statusbar
	updateStatusbar(charkey, -46, FALSE);

	free(charkey);
}

// Append or remove a character at the end of string cmd
void cmdAlter(char pCh, int mode)
{

	if (mode) // Adding character - TODO: Make this a util in a different file
	{
		// Display the character on stdscr
		addch(pCh);
		x++;

		// New string size: +1 for new char, +1 for nullbyte
		size_t newStrLen = (cmdLen + 2) * sizeof(char);

		// Allocate array for tmp string
		char strtmp[newStrLen];

		// Copy cmd to tmp string
		strncpy(strtmp, cmd, cmdLen);

		// Add character to end of cmd where nullbyte is, put back nullbyte at end
		strtmp[cmdLen] = pCh;
		strtmp[++cmdLen] = '\0';

		// Free cmd str from heap, then malloc again for new str size
		free(cmd);
		cmd = malloc(newStrLen);

		// Copy over new data to cmd
		strcpy(cmd, strtmp);

		// Move cursor to next line if at end of row
		if (x == maxCol - 1)
		{
			move(++y, x = 1);
		}
		else
		{
			move(y, x);
		}
	}
	else // Backspacing a previously entered character
	{
		if (cmdLen)
		{ // Checks if strlen is != 0
			cmdLen--;

			// Move back to end of prev line if backspacing
			if (x == 1)
			{
				move(--y, x = (maxCol - 1));
			}

			// Move back, overwrite previous char, then move back
			printw("\b \b");
			x--;

			cmd[cmdLen - 1] = '\0'; // Simply overwrite prev char with '\0'
		}
		else
		{
			// Pressed backspace, but nothing to delete, do nothing
		}
	}
	return;
}

void cmdEnter(int prompt)
{
	cmd = calloc(4, sizeof(char)); // allocate one byte for string
	currCh = '\0';				   // string for input char

	if (prompt)
	{
		printw(SEASHELL);
		x += strlen(SEASHELL);
	}

	while (1)
	{
		// printw("currCh addr = [%d], value = [%s]", &currCh, currCh);

		printInfo(); // Show X,Y pos in statusbar border
		currCh = getch();

		// clear();
		move(y, x);

		if (currCh == '\0' || currCh == KEY_ENTER || currCh == KBD_ENTER)
		{
			// Enter key
			break;
		}
		else if (currCh == KEY_BACKSPACE || currCh == KBD_BACKSPACE)
		{
			// Backspace
			cmdAlter(currCh, 0);
		}
		else if (currCh == 3)
		{
			// Ctrl-C
			endwin();
			printf("Pressed Ctrl-C, exiting...\n");
			exit(0);
		}
		else
		{
			// printw("\ncmdAlter with char %c", currCh);
			// Add char to cmd
			cmdAlter(currCh, 1);
		}
	}
}

int main()
{
	// Holds previous I/O
	buffer = malloc(sizeof(struct buffer));
	buffer->input = malloc(sizeof(char *) * SCROLLBACK);
	buffer->output = malloc(sizeof(char *) * SCROLLBACK);
	buffer->n = 0;

	// _setmode(_fileno(stdout), 0x00020000);
	initscr();			  /* Start curses mode */
	cbreak();			  /* Line buffering disabled, Pass on everything to me */
	start_color();		  /* Start the color functionality */
	keypad(stdscr, TRUE); /* We get F1, F2 etc... */
	noecho();			  /* Don't echo() while we do getch */
	timeout(-1);		  /* Pause on input and don't timeout */
	raw();				  /* Line buffering disabled */

	getmaxyx(stdscr, maxRow, maxCol);
	// box(stdscr, 0, 0);
	// wborder(stdscr, '|', '|', '-', '-', '+', '+', '+', '+');
	// wborder(stdscr, L'║', L'║', L'═', L'═', L'╔', L'╗', L'╚', L'╝');
	// wborder(stdscr, L'│', L'│', L'─', L'─', L'┌', L'┐', L'└', L'┘');
	// wborder(stdscr, L'│', L'│', L'─', L'─', L'╔', L'╗', L'╚', L'╝');

	// Move 1 block away from the edges to not overwrite them
	move(++y, ++x);

	while (1)
	{
		// printInfo(); // Show X,Y pos in statusbar border
		cmdEnter(1);
		buffer->input[curLine] = malloc(sizeof(char) * cmdLen);
		strncpy(buffer->input[curLine], cmd, cmdLen);
		curLine++;

		// refresh();
		getmaxyx(stdscr, maxRow, maxCol);
		getyx(stdscr, y, x);
		// clear(); //Needs to be run while waiting for input, halfwait

		// Print out command after done
		move(++y, x = 1);

		if (strcmp(cmd, "connect") == 0)
		{
			// Attempt connection
			char *res = malloc(256);
			int sockStat = create_socket("127.0.0.1", 4444, res);
			if (sockStat != 0)
			{
				printw("There was an error communicating with the server (connect() return status: %u)", sockStat);
			}
			else
			{
				printw("The server responded with: ");
				printw("%s", res);
			}

			free(res);
		}
		else
		{
			// Just echo that the command is unknown
			printw("Unknown seashell command: %s", cmd);
		}

		// Redraw border and print command length in bottom of screen
		// int tmpLen = snprintf( NULL, 0, "%d", x );

		// Clear variables and move to back, going to next line for new command
		// TODO: Store into command history
		cmdLen = 0;
		move(++y, x = 1);
	}

	return 0;
}
