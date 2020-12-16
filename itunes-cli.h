#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <menu.h>
#include <libgen.h>

/* Variable NUMBER_OF_TRACKS
 * ------------------------------
 *      Represents the total number of tracks (songs) the user has available
 *      on their system.
 */
extern int NUMBER_OF_TRACKS;

/* Struct Track
 * ------------------------------
 *      Every song on the user's machine, if any exist, are encapsulated via a
 *      Track. A Track is composed of five things:
 *
 *		artist: Stores the artist of the track.
 *		album: Stores the album of the track.
 *		title: Stores the title of the track only
 *      origin: Stores the ORIGINAL path string retrieved (ie. the --likely-- invalid path).
 *          eg: ./Jay-Z & Kanye West/Watch the Throne/01 Primetime.m4a
 *      pathName: Stores the *escaped* path name
 *          eg: ./"Jay-Z & Kanye West"/"Watch the Throne"/"01 Primetime.m4a"
 */
struct Track {
	char *artist;
	char *album;
	char *title;
	char *origin;
	char *pathName;
}; typedef struct Track Track;

/* Function error
 * ------------------------------
 *		Prints the error message passed in and exits the program.
 *
 *		msg: The error message to display to the user.
 */
void error(char *msg);

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
WINDOW * showListeningWindow(WINDOW *win, int yMax, int xMax, Track *track);

/* Function playSong
 * ------------------------------
 *      Attempts the play the requested track whose pathName is given.
 *
 *      pathName: the escaped absolute path of the requested track.
 */
void playSong(char *pathName);

/* Function stopPlayingTrack
 * ------------------------------
 *      Stops the currently running track running on the
 *      given process id.
 */
void stopPlayingTrack();

/* Function getTrackPid
 * ------------------------------
 *      Gets the process ID of the currently running afplay process use pgrep.
 */
int getTrackPid();

/* Function pauseTrack
 * ------------------------------
 *      Depending on the state of the track, this function will either
 *      pause the currently running track, or resume it.
 *
 *      trackId: The process ID of the afplay process (ie. pid of currently running track)
 *      paused: If false, the track is playing in the background (will pause). If true, track has been previously paused (will resume).
 */
void pauseTrack(int trackId, bool paused);

/* Function getArtist
 * ------------------------------
 *		Using the path name specified to the function, get the artist.
 *
 *		pathName: The path of the track to parse.
 */
char * getArtist(char *pathName);

/* Function getAlbum
 * ------------------------------
 *		Using the path name specified to the function, get the album.
 *
 *		pathName: The path of the track to parse.
 */
char * getAlbum(char *pathName);

/* Function getTitle
 * ------------------------------
 *		Using the path name specified to the function, get the title.
 *
 *		pathName: The path of the track to parse.
 */
char * getTitle(char *pathName);

/* Function containsUnicode
 * ------------------------------
 *      Checks if the passed in track (and its path name) contains any non-ASCII characters.
 *      Unfortunately, these tracks will not be available for the user as the ncurses library is unable
 *      to genenrate menu items from strings containing these characters.
 *
 *      pathName: The path of the track to check.
 */
bool containsUnicode(char *pathName);

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
char * escapeString(char *pathName);

/* Function getUserTracks
 * ------------------------------
 *      Retrieves all of the user's iTunes tracks stored on their system and returns a list
 *      of pointers to structures containing information about the retrieved songs.
 *      This function ONLY looks at the directory ~/Music/iTunes/iTunes Media/Music/.
 */
Track ** getUserTracks();
