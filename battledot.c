#include "battledot.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// initializing first and current player before building list
struct Player *front = NULL;
struct Player *current = NULL;

/*
Description:
Checks if the player list is empty.

Arguments:
	none

Returns:
	front == NULL : if true then empty if not then not empty
*/
int checkEmpty() {
	return front == NULL;
}

/*
Description:
Inserts a player to the front of the player list and inserts the coordinates of their ship into a Player
class.

Arguments:
	char moniker[4]		: Three letter name of player + terminating character
	int x_ship			: x coordinate of player ship
	int y_ship 			: y coordinate of player ship

Returns:
	none
*/
void insertPlayer(char moniker[4], int x_ship, int y_ship) {
	struct Player *newplayer = (struct Player *)malloc(sizeof(struct Player));

	strcpy(newplayer->moniker, moniker);
	newplayer->x_ship = x_ship;
	newplayer->y_ship = y_ship;

	// if list is not empty then 
	if(!checkEmpty()) {
		
		// newplayer replaces front
		newplayer->next = front; 
		newplayer->last = front->last;
		
		// the player before front now points to newplayer as next
		front->last->next = newplayer; 

		// newplayer is now the player before front
		front->last = newplayer;
	}

	else {
		// if list is empty then newplayer is now the head.
		front = newplayer;

		// since only one player the next, last and current player are the same
		front->next = front;
		front->last = front;
	}
}


/*
Description:
Removes a player from the player list and appropriately pairs up their neighbours. There are different cases
of removal like when the loser is at the front of the list that require a different approach. This is why
there are several if else statements.

Arguments:
	struct Player *loser : the player who was just eliminated from the game.

Returns:
	none
*/
void removePlayer(struct Player *loser) {
	// if loser is last player
	if (loser == front && loser->next == NULL) {
		free(front);
	}

	// if loser is second last player then set players besides front to NULL
	// signalling that if they are removed then game is empty
	else if (loser == front && front->next->next == loser) {
		front = loser->next;
		front->next = NULL;
		front->last = NULL;
		free(loser);
	}
	// if loser is at front of list
	else if (loser == front) {
		front = loser->next;
		loser->last->next = front;
		front->last = loser->last;
		free(loser);
	}

	else {
		loser->next->last = loser->last;
		loser->last->next = loser->next;
		free(loser);
	}
}