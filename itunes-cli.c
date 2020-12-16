#include "itunes-cli.h"

int NUMBER_OF_TRACKS = 0;

int main(int argc, char **argv) {
	/* DECLARE MENU VARIABLES */
	ITEM **trackItems;
	MENU *trackMenu;
	WINDOW *trackWindow;

	/* START NCURSES */
	initscr(); // create screen
	curs_set(0); // hide cursor
	noecho(); // don't show the characters the user is typing
	cbreak(); // allow user to quit with ctrl-c; may want to remove this later

	// get screen size
	int yMax, xMax;
	getmaxyx(stdscr, yMax, xMax);

	/* CREATE THE MENU ITEMS */
	Track **tracks = getUserTracks();
	trackItems = (ITEM **)calloc(NUMBER_OF_TRACKS+1, sizeof(ITEM *));
	int index = 0;
	for(int i = 0; i < NUMBER_OF_TRACKS; i++) {
		char *trackBuffer = (char *)malloc(1024 * sizeof(char));
		int n = sprintf(trackBuffer, "%s - %s", tracks[i]->artist, tracks[i]->title);
		if(n < 0) {
			error("(main) Unable to format string");
		}

		ITEM *currTrack = new_item(trackBuffer, "");
		//printw("(%s --> %d)\n", tracks[i]->origin, i);
		if(currTrack == NULL) {
			//printw("(%s --> %d)\n", tracks[i]->origin, i);
			if(errno == E_BAD_ARGUMENT) {
				//printw("(bad argument --> %d)\n", i);
			}
			else {
				//printw("(system error --> %d)\n", i);
			}
		}
		else {
			trackItems[index++] = currTrack;
		}
	}

	// null-terminate list!
	trackItems[index] = (ITEM *)NULL;

	/* CREATE OUR MENU */
	trackMenu = new_menu((ITEM **)trackItems);
	printw("iTunes CLI brought to you by Anand Murali and ncurses", item_count(trackMenu));
	refresh();

	// create window associated with menu
	trackWindow = newwin(yMax/2 + 5, xMax, 2, 0);
	keypad(trackWindow, TRUE);

	// set main window and sub menus
	set_menu_win(trackMenu, trackWindow);
	set_menu_sub(trackMenu, derwin(trackWindow, yMax/2 + 4, xMax-1, 1, 1));
	set_menu_format(trackMenu, 25, 1);

	/* SET SELECTED ITEM SYMBOL */
	set_menu_mark(trackMenu, " * ");

	/* ADD SOME INSTRUCTIONS */
	box(trackWindow, 0, 0);
	//mvprintw(LINES - 2, 0, "Use PageUp and PageDown to scroll down or up a page of items");
	//mvprintw(LINES - 1, 0, "Arrow Keys to navigate (q to Exit)");

	/* DISPLAY OUR MENU */
	post_menu(trackMenu);
	refresh();
	wrefresh(trackWindow);

	/* Currently Listening To Window */
	WINDOW *listeningWindow = NULL;
	listeningWindow = showListeningWindow(listeningWindow, yMax, xMax, NULL);

	wchar_t choice; // will store the user's choice upon hitting arrow keys/enter
	int musicIndex = 0; // stores the index in the track list the user is requesting
	bool currentlyListening = false; // sentinel for if we are currently playing a song
	int trackPid = -1; // process ID for currently running track
	bool paused = false;
	Track *selectedTrack = NULL;

	while((choice = wgetch(trackWindow)) != 'q') {
		switch(choice) {
			case KEY_DOWN:
				menu_driver(trackMenu, REQ_DOWN_ITEM);
				musicIndex++;
				musicIndex = (musicIndex == NUMBER_OF_TRACKS) ? NUMBER_OF_TRACKS-1: musicIndex;
				break;
			case KEY_UP:
				menu_driver(trackMenu, REQ_UP_ITEM);
				musicIndex--;
				musicIndex = (musicIndex < 0) ? 0: musicIndex;
				break;
		}
		wrefresh(trackWindow);

		// if user hits enter on a song
		if(choice == 10) {
			// update selected track
			selectedTrack = tracks[musicIndex];
			// if we are listening to a song, we must kill the process
			if(currentlyListening) {
				stopPlayingTrack();
				currentlyListening = false;
			}
			// play the requested song
			playSong(selectedTrack->pathName);
			trackPid = getTrackPid();
			currentlyListening = true;
			paused = false;

			/* UPDATE Currently Listening TO REFLECT NEW SELECTED TRACK */
			listeningWindow = showListeningWindow(listeningWindow, yMax, xMax, selectedTrack);

		}
		else if (choice == ' ') {
			if(currentlyListening) {
				// if the user hits space, simply pause/resume track
				pauseTrack(trackPid, paused);
				paused = !paused;
			}
		}
	}

	// stop playing track if we are listening
	if(currentlyListening) {
		stopPlayingTrack();
		currentlyListening = false; // technically not needed but its fine :)
	}

	/* CLEANUP */
	unpost_menu(trackMenu);
	free_menu(trackMenu);
	for(int i = 0; i < NUMBER_OF_TRACKS; i++) {
		free_item(trackItems[i]);
	}
	endwin();

	for(int i = 0; i < NUMBER_OF_TRACKS; i++) {
		free(tracks[i]);
	}

	/* END NCURSES */
	return 0;
}

