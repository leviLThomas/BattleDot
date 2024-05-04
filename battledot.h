#ifndef BATTLEDOT_DOT_H
#define BATTLEDOT_DOT_H

struct Player {
	char moniker[4]; // Three characters and null char
	int x_ship;
	int y_ship;

	// next and previous player in the list
	struct Player *next;
	struct Player *last;
};

int checkEmpty();
void insertPlayer(char moniker[4], int x_ship, int y_ship);
void removePlayer(struct Player *loser);

#endif
