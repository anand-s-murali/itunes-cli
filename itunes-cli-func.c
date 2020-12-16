#include "itunes-cli.h"

/* Function error
 * ------------------------------
 *		Prints the error message passed in and exits the program.
 *
 *		msg: The error message to display to the user.
 */
void error(char *msg) {
	// print out the message
	perror(msg);

	// stop any background music still playing
	stopPlayingTrack();

	// exit program
	exit(errno);
}

/* Function showListeningWindow
 * ------------------------------
 * 		This function is responsible for displaying information about the currently
 * 		playing track.
 *
 * 		win: The window to display (somewhat redundant, but necessary).
 * 		yMax: The maximum number of columns of the terminal screen. Needed for if we have to create the window.
 * 		xMax: The maximum number of rows of the terminal screen. Needed for if we have to create the window.
 * 		track: The track struct we need to display information about.
 *
 * 		This function returns a pointer to win back to the caller. This is needed when win is originally null.
 */
WINDOW * showListeningWindow(WINDOW *win, int yMax, int xMax, Track *track) {
	int lwRows = 10;
	int lwCols = 75;
	int lwStartRow = yMax-15;
	int lwStartCol = 5;

	// clear the window if it is non-null
	if(win != NULL) {
		wclear(win);
	}

	// if the window is null, create it, otherwise do nothing
	win = (win == NULL) ? newwin(lwRows, lwCols, lwStartRow, lwStartCol): win;

	// re-draw border and title
	box(win, 0, 0); // border

	char lwTitleTextBuff[] = " Currently Listening To: ";
	mvwprintw(win, 0, 1, lwTitleTextBuff); // title

	// now display artist, album and title
	int n;
	char artistBuffer[1024];
	char albumBuffer[1024];
	char titleBuffer[1024];

	n = sprintf(artistBuffer, "Artist: %s", (track == NULL) ? "": track->artist);
	if(n < 0) {
		error("(showListeningWindow -> artist) Unable to format string");
	}
	n = sprintf(albumBuffer, "Album: %s", (track == NULL) ? "": track->album);
	if(n < 0) {
		error("(showListeningWindow -> album) Unable to format string");
	}
	n = sprintf(titleBuffer, "Title: %s", (track == NULL) ? "": track->title);
	if(n < 0) {
		error("(showListeningWindow -> title) Unable to format string");
	}

	mvwprintw(win, 2, 2, artistBuffer);
	mvwprintw(win, 4, 2, albumBuffer);
	mvwprintw(win, 6, 2, titleBuffer);

	wrefresh(win);
	return win;
}

/* Function playSong
 * ------------------------------
 *      Attempts the play the requested track whose pathName is given.
 *
 *      pathName: the escaped absolute path of the requested track.
 */
void playSong(char *pathName) {
	char cmdBuffer[1024];
	/* DON'T FORGET THE & AT THE END -- THIS ALLOWS US TO PLAY MUSIC IN THE BACKGROUND */
	int n = sprintf(cmdBuffer, "afplay %s/Music/iTunes/\"iTunes Media\"/Music/%s &", getenv("HOME"), pathName+2);
	if(n < 0) {
		error("(playSong) Error occurred while formatting string");
	}
	system(cmdBuffer);
}

/* Function stopPlayingTrack
 * ------------------------------
 *      Stops the currently running track running on the
 *      given process id.
 */
void stopPlayingTrack() {
	system("killall afplay");
}

/* Function getTrackPid
 * ------------------------------
 *      Gets the process ID of the currently running afplay process use pgrep.
 */
int getTrackPid() {
	FILE *fp;
	char pidBuffer[1024];
	int trackPid;

	char *cmd = "pgrep afplay";
	fp = popen(cmd, "r");
	if(fp == NULL) {
		error("(getTrackPid) Unable to perform pope");
	}

	char *res = fgets(pidBuffer, sizeof(pidBuffer), fp);
	if(res == NULL) {
		error("(getTrackPid) Unable to find PID for currently playing track");
	}

	// close the pipe
	if(pclose(fp) == -1) {
		error("(getTrackPid) Error while closing pipe");
	}

	// convert the buffer to an int
	trackPid = atoi(pidBuffer);
	return trackPid;
}

/* Function pauseTrack
 * ------------------------------
 *      Depending on the state of the track, this function will either
 *      pause the currently running track, or resume it.
 *
 *      trackId: The process ID of the afplay process (ie. pid of currently running track)
 *      paused: If false, the track is playing in the background (will pause). If true, track has been previously paused (will resume).
 */
void pauseTrack(int trackId, bool paused) {
	int sig = (paused) ? SIGCONT: SIGSTOP;
	if(kill(trackId, sig) == -1) {
		char buffer[1024];
		sprintf(buffer, "(pauseTrack) Unable to %s track", (paused) ? "resume": "pause");
		error(buffer);
	}
}

/* Function getArtist
 * ------------------------------
 *		Using the path name specified to the function, get the artist.
 *
 *		pathName: The path of the track to parse.
 */
char * getArtist(char *pathName) {
	char *artist = (char *)malloc(1024 * sizeof(char));
	int i = 2;
	int index = 0;
	while(pathName[i] != '/') {
		artist[index++] = pathName[i];
		i++;
	}
	artist[index] = '\0';

	return artist;
}

/* Function getAlbum
 * ------------------------------
 *		Using the path name specified to the function, get the album.
 *
 *		pathName: The path of the track to parse.
 */
char * getAlbum(char *pathName) {
	char *album = (char *)malloc(1024 * sizeof(char));
	int i = 2;
	int index = 0;

	while(pathName[i] != '/') {
		i++;
	}
	i++;

	while(pathName[i] != '/') {
		album[index++] = pathName[i];
		i++;
	}
	album[index] = '\0';

	return album;
}

/* Function getTitle
 * ------------------------------
 *		Using the path name specified to the function, get the title.
 *
 *		pathName: The path of the track to parse.
 */
char * getTitle(char *pathName) {
	char *title = strdup(basename(pathName));
	// move title pointer to beginning of title
	while(*title != ' ')  {
		title++;
	}
	title++;

	// remove the .m4a extension
	title[strlen(title)-5] = '\0';

	return title;
}
/* Function containsUnicode
 * ------------------------------
 *      Checks if the passed in track (and its path name) contains any non-ASCII characters.
 *      Unfortunately, these tracks will not be available for the user as the ncurses library is unable
 *      to genenrate menu items from strings containing these characters.
 *
 *      pathName: The path of the track to check.
 */
bool containsUnicode(char *pathName) {
	unsigned int c;
	for(int i = 0; pathName[i] != '\0'; i++) {
		c = (int)pathName[i];
		if(c > 127) {
			return true;
		}
	}
	return false;
}

/* Function escapeString
 * ------------------------------
 *      Upon retrieval, path names are not always valid (directory names may contain spaces!)
 *      This function takes in a path name and adds quotation marks around every portion of the
 *      path so that it can be later used in system calls.
 *
 *      Example:
 *          ./this/is/a simple/example --> ./"this"/"is"/"a simple"/"example"
 *
 *      pathName: The path name we need to add escape characters to.
 */
char * escapeString(char *pathName) {
	char *escape = (char *)malloc(1024 * sizeof(char));
	if(escape == NULL) {
		error("(escapeString) Not enough memory available");
	}

	int index = 0;
	for(int i = 0; pathName[i] != '\n'; i++) {
		if(pathName[i] == '/') {
			if(i == 1) {
				// we don't need to doubly pad the first '/' with quotes
				escape[index++] = pathName[i];
				escape[index++] = '\"';
			}
			else {
				escape[index++] = '\"';
				escape[index++] = pathName[i];
				escape[index++] = '\"';
			}
		}
		else {
			escape[index++] = pathName[i];
		}
	}
	// don't forget to add last quote to end the song
	escape[index++] = '\"';
	// don't forget null!
	escape[index] = '\0';

	return escape;
}

/* Function getUserTracks
 * ------------------------------
 *      Retrieves all of the user's iTunes tracks stored on their system and returns a list
 *      of pointers to structures containing information about the retrieved songs.
 *      This function ONLY looks at the directory ~/Music/iTunes/iTunes Media/Music/.
 */
Track ** getUserTracks() {
	FILE *fp;
	char pathName[300]; // may need to be longer?

	// use fp with popen
	char *cmd = "cd ~/Music/iTunes/\"iTunes Media\"/Music/ && find . -name \"*.m4a\"";
	fp = popen(cmd, "r");
	if(fp == NULL) {
		error("(getUserTracks) Unable to find your tracks");
	}

	// create our list
	Track **trackList = (Track **)malloc(1024 * sizeof(Track *));

	// read the output (i.e) the tracks
	while(fgets(pathName, sizeof(pathName), fp) != NULL) {
		// allocate memory for track
		if(!containsUnicode(pathName)) {
			/* ALLOCATE MEMORY FOR TRACK */
			trackList[NUMBER_OF_TRACKS] = (Track *)malloc(sizeof(Track));

			/* UPDATE ATTRIBUTES FOR THE TRACK */
			trackList[NUMBER_OF_TRACKS]->artist = strdup(getArtist(pathName));
			trackList[NUMBER_OF_TRACKS]->album = strdup(getAlbum(pathName));
			trackList[NUMBER_OF_TRACKS]->title = strdup(getTitle(pathName));
			trackList[NUMBER_OF_TRACKS]->origin = strdup(pathName);

			/* ESCAPE PATHNAME FOR LATER SYSTEM CALL */
			char *escapedPath = strdup(escapeString(pathName));
			trackList[NUMBER_OF_TRACKS]->pathName = strdup(escapedPath);

			// update number of tracks
			NUMBER_OF_TRACKS++;
		}
	}

	// null terminate list
	trackList[NUMBER_OF_TRACKS] = (Track *)NULL;

	// close the pipe
	if(pclose(fp) == -1) {
		error("(getUserTracks) Error while closing pipe");
	}

	// return our list
	return trackList;
}
