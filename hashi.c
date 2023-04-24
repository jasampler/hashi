/**
 * hashi - Console program to find the solutions of any Hashiwokakero puzzle.
 *         The input must be digits and dots separated by newlines or slashes.
 *
 * Copyright 2023 Carlos Rica (jasampler)
 * This file is the main file of the jasampler's hashi project.
 * hashi is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * hashi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with the hashi.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdio.h> /* NULL, printf, fprintf, stderr, getchar, EOF */
#include <stdlib.h> /* exit */
#include <string.h> /* memset */
#include <stdbool.h> /* bool, true, false */
#include <limits.h> /* CHAR_MAX */

typedef enum enum_direction { UP = 0, LEFT, RIGHT, DOWN} direction;

/** Number of directions to move from any island in the bidimensional space. */
#define DIRECTIONS 4

/** Maximum of bridges between two connected islands (2). */
#define MAX_CONNECTION_BRIDGES 2

/** Maximum of expected bridges that an island can specify (8). */
#define MAX_EXPECTED_BRIDGES (MAX_CONNECTION_BRIDGES * DIRECTIONS)

/** CHECK_CONNECTED_SOLUTION is defined to discard not connected solutions. */
#define CHECK_CONNECTED_SOLUTION 1

#define MAX_ISLANDS 150
#define MAX_CONNECTIONS 300
#define MAX_CROSSELEMS 300
#define MAX_VISITED_SIZE 10000

typedef struct st_hcrosselem hcrosselem;

/** Element to compose a linked list of connections crossing a given connection.
 * Only the reference to the number of bridges in the connection is needed. */
struct st_hcrosselem {
	char *pbridges;
	hcrosselem *nextcross;
};

/** Connection shared between two islands where 0, 1 or 2 bridges are built,
 * which also includes a linked list with the other connections crossing it
 * because two crossing connections cannot have bridges at the same time,
 * and the address of to the number of pending bridges of each connected island
 * because a bridge cannot be built in an island with 0 pending bridges. */
typedef struct st_hconnection {
	char bridges;
	hcrosselem *firstcross;
	char *ppendbridges1, *ppendbridges2;
} hconnection;

typedef struct st_hisland hisland;

/** An island has a constant expected number of bridges (1-8) to be built on it,
 * a calculated number of pending bridges that decreases when bridges are built,
 * four islands connected to it in each direction and the connections to them.
 * A connection saves the number of bridges between two islands in any moment.
 * An empty island and an empty connection are used when no connection exists.*/
struct st_hisland {
	char pendbridges, expectbridges, row, col;
	hisland *islands[DIRECTIONS];
	hconnection *connections[DIRECTIONS];
};

/** When another island is added, the number of islands field is incremented
 * and the fields with the total rows and columns can be incremented too. */
typedef struct st_hboard {
	int max_islands, num_islands;
	int max_connections, num_connections;
	int max_crosselems, num_crosselems;
	int rows, cols, max_bridges;
	int max_visited_size, visitedlimit;
	bool *visitedmatrix;
	hisland *islands, out_island_st, *out_island;
	hconnection *connections, out_connection_st, *out_connection;
	hcrosselem *crosselems;
} hboard;

void init_out_island(hisland *out_island) {
	int i;
	out_island->row = -1;
	out_island->col = -1;
	out_island->pendbridges = 0;
	out_island->expectbridges = 0;
	for (i = 0; i < DIRECTIONS; i++) {
		out_island->islands[i] = NULL;
	}
	for (i = 0; i < DIRECTIONS; i++) {
		out_island->connections[i] = NULL;
	}
}

/** The connection to an island out of the board shows */
void init_out_connection(hconnection *out_connection, hisland *out_island) {
	out_connection->bridges = 0;
	out_connection->firstcross = NULL;
	out_connection->ppendbridges1 = &(out_island->pendbridges);
	out_connection->ppendbridges2 = &(out_island->pendbridges);
}

/** The "outside" island and connection are the first of the given arrays. */
void init_board(hboard *board, hisland *islands, int max_islands,
		hconnection *connections, int max_connections,
		hcrosselem *crosselems, int max_crosselems,
		bool *visitedmatrix, int max_visited_size) {
	board->islands = islands;
	board->max_islands = max_islands;
	board->connections = connections;
	board->max_connections = max_connections;
	board->crosselems = crosselems;
	board->max_crosselems = max_crosselems;
	board->visitedmatrix = visitedmatrix;
	board->max_visited_size = max_visited_size;
	board->num_islands = 0;
	board->num_connections = 0;
	board->num_crosselems = 0;
	board->rows = 0;
	board->cols = 0;
	board->out_island = &(board->out_island_st);
	init_out_island(board->out_island);
	board->out_connection = &(board->out_connection_st);
	init_out_connection(board->out_connection, board->out_island);
	memset(board->visitedmatrix, 0, board->max_visited_size);
}

/** Finds an island from the island with the given index to connect both. */
hisland *find_from_island(hboard *board, int index, direction dir) {
	hisland *start, *island;
	if (index < 0 || index >= board->num_islands) {
		fprintf(stderr, "Invalid index: %d\n", index);
		return NULL;
	}
	start = board->islands + index;
	switch (dir) {
	case LEFT:
		while (--index > -1) {
			island = board->islands + index;
			if (island->row < start->row) {
				break;
			}
			return island;
		}
		break;
	case UP:
		while (--index > -1) {
			island = board->islands + index;
			if (island->col == start->col) {
				return island;
			}
		}
		break;
	default:
		fprintf(stderr, "Unsupported direction: %d\n", dir);
		return NULL;
	}
	return board->out_island;
}

/** Finds an island from the given position to check if a bridge reaches it. */
hisland *find_from_position(hboard *board, int row, int col, direction dir) {
	int index;
	hisland *island;
	switch (dir) {
	case LEFT:
		index = board->num_islands;
		while (--index > -1) {
			island = board->islands + index;
			if (island->row == row) {
				if (island->col < col) {
					return island;
				}
			} else if (island->row < row) {
				break;
			}
		}
		break;
	case UP:
		index = board->num_islands;
		while (--index > -1) {
			island = board->islands + index;
			if (island->col == col && island->row < row) {
				return island;
			}
		}
		break;
	default:
		fprintf(stderr, "Unsupported direction: %d\n", dir);
		return NULL;
	}
	return board->out_island;
}

hisland *next_island(hboard *board) {
	if (board->num_islands >= board->max_islands) {
		fprintf(stderr, "Maximum of islands reached: %d\n",
				board->num_islands);
		return NULL;
	}
	return board->islands + board->num_islands++;
}

hconnection *next_connection(hboard *board) {
	if (board->num_connections >= board->max_connections) {
		fprintf(stderr, "Maximum of connections reached: %d\n",
				board->num_connections);
		return NULL;
	}
	return board->connections + board->num_connections++;
}

hcrosselem *next_crosselem(hboard *board) {
	if (board->num_crosselems >= board->max_crosselems) {
		fprintf(stderr, "Maximum of cross elements reached: %d\n",
				board->num_crosselems);
		return NULL;
	}
	return board->crosselems + board->num_crosselems++;
}

/** Inserts the given cross element in the given connection before the rest. */
void insert_crosselem(hconnection *connection, hcrosselem *crosselem) {
	hcrosselem *tmp = connection->firstcross;
	crosselem->nextcross = tmp;
	connection->firstcross = crosselem;
}

/** Finds the connections crossing the connection between the given islands. */
bool fill_crosses(hboard *board, hisland *up, hisland *down) {
	hisland *island, *right;
	hcrosselem *cross;
	hconnection *conn_vert, *conn_horz;
	int col = up->col;
	conn_vert = up->connections[DOWN];
	for (island = up + 1; island != down; island++) {
		if (island->row > up->row && island->row < down->row
				&& island->col < col) {
			right = island->islands[RIGHT];
			if (right != board->out_island && right->col > col) {
				conn_horz = island->connections[RIGHT];
				if ((cross = next_crosselem(board)) == NULL) {
					return false;
				}
				cross->pbridges = &(conn_vert->bridges);
				insert_crosselem(conn_horz, cross);
				if ((cross = next_crosselem(board)) == NULL) {
					return false;
				}
				cross->pbridges = &(conn_horz->bridges);
				insert_crosselem(conn_vert, cross);
			}
		}
	}
	return true;
}

/** Creates the connections of the last added island. */
bool fill_connections(hboard *board) {
	hisland *island, *left, *up;
	hconnection *connection;
	int i;
	int index = board->num_islands - 1;
	island = board->islands + index;
	island->islands[RIGHT] = board->out_island;
	island->islands[DOWN] = board->out_island;
	island->islands[LEFT] = left = find_from_island(board, index, LEFT);
	island->islands[UP] = up = find_from_island(board, index, UP);
	connection = board->out_connection;
	for (i = 0; i < DIRECTIONS; i++) {
		island->connections[i] = connection;
	}
	if (left != board->out_island) {
		if ((connection = next_connection(board)) == NULL) {
			return false;
		}
		connection->bridges = 0;
		connection->firstcross = NULL;
		connection->ppendbridges1 = &(left->pendbridges);
		connection->ppendbridges2 = &(island->pendbridges);
		island->connections[LEFT] = connection;
		left->connections[RIGHT] = connection;
		left->islands[RIGHT] = island;
	}
	if (up != board->out_island) {
		if ((connection = next_connection(board)) == NULL) {
			return false;
		}
		connection->bridges = 0;
		connection->firstcross = NULL;
		connection->ppendbridges1 = &(up->pendbridges);
		connection->ppendbridges2 = &(island->pendbridges);
		island->connections[UP] = connection;
		up->connections[DOWN] = connection;
		up->islands[DOWN] = island;
		if (! fill_crosses(board, up, island)) {
			return false;
		}
	}
	return true;
}

/** Adds a new island that must have a row/colum bigger than previous island. */
bool add_island(hboard *board, int row, int col, int expectbridges) {
	hisland *island;
	if (row < 0 || col < 0) {
		fprintf(stderr, "Negative position: %d,%d\n", row, col);
		return false;
	}
	if (row >= CHAR_MAX) {
		fprintf(stderr, "Maximum of rows reached: %d\n", row);
		return false;
	}
	if (col >= CHAR_MAX) {
		fprintf(stderr, "Maximum of columns reached: %d\n", col);
		return false;
	}
	if (board->num_islands > 0) {
		hisland *prev = board->islands + (board->num_islands - 1);
		if (row < prev->row || (row == prev->row && col <= prev->col)) {
			fprintf(stderr, "Invalid position: %d,%d\n", row, col);
			return false;
		}
	}
	if (expectbridges < 1 || expectbridges > MAX_EXPECTED_BRIDGES) {
		fprintf(stderr, "Bad number of bridges: %d\n", expectbridges);
		return false;
	}
	if ((island = next_island(board)) == NULL) {
		return false;
	}
	island->expectbridges = expectbridges;
	island->pendbridges = expectbridges;
	island->row = row;
	island->col = col;
	if (board->rows <= row) {
		board->rows = row + 1;
	}
	if (board->cols <= col) {
		board->cols = col + 1;
	}
	if (! fill_connections(board)) {
		return false;
	}
	board->max_bridges += expectbridges;
	return true;
}

void print_empty_position(hboard *board, int i, int j) {
	hisland *left, *up;
	hconnection *conn;
	bool emptyleft = false, emptyup = false, printed = false;
	left = find_from_position(board, i, j, LEFT);
	if (left != board->out_island) {
		conn = left->connections[RIGHT];
		if (conn != board->out_connection) {
			if (conn->bridges == 0) {
				emptyleft = true;
			} else if (conn->bridges == 1) {
				printf("---");
				printed = true;
			} else if (conn->bridges == 2) {
				printf("===");
				printed = true;
			}
		}
	}
	up = find_from_position(board, i, j, UP);
	if (up != board->out_island) {
		conn = up->connections[DOWN];
		if (conn != board->out_connection) {
			if (conn->bridges == 0) {
				emptyup = true;
			} else if (conn->bridges == 1) {
				printf(" ! ");
				printed = true;
			} else if (conn->bridges == 2) {
				printf(" !!");
				printed = true;
			}
		}
	}
	if (! printed) {
		if (emptyleft && emptyup) {
			printf(" + ");
		} else if (emptyleft) {
			printf(" - ");
		} else if (emptyup) {
			printf(" ' ");
		} else {
			printf(" . ");
		}
	}
}

void print_space_right(hboard *board, int i, int j) {
	hisland *left;
	hconnection *conn;
	bool printed = false;
	left = find_from_position(board, i, j, LEFT);
	if (left != board->out_island) {
		conn = left->connections[RIGHT];
		if (conn != board->out_connection) {
			if (conn->bridges == 1) {
				printf("--");
				printed = true;
			} else if (conn->bridges == 2) {
				printf("==");
				printed = true;
			}
		}
	}
	if (! printed) {
		printf("  ");
	}
}

void print_space_down(hboard *board, int i, int j) {
	hisland *up;
	hconnection *conn;
	bool printed = false;
	up = find_from_position(board, i, j, UP);
	if (up != board->out_island) {
		conn = up->connections[DOWN];
		if (conn != board->out_connection) {
			if (conn->bridges == 1) {
				printf(" ! ");
				printed = true;
			} else if (conn->bridges == 2) {
				printf(" !!");
				printed = true;
			}
		}
	}
	if (! printed) {
		printf("   ");
	}
}

void print_board(hboard *board) {
	int i, j, index = 0;
	hisland *island;
	for (i = 0; i < board->rows; i++) {
		for (j = 0; j < board->cols; j++) {
			if (index < board->num_islands) {
				island = board->islands + index;
				if (island->row == i && island->col == j) {
					printf("(%d)",island->expectbridges);
					index++;
				} else {
					print_empty_position(board, i, j);
				}
			} else {
				printf(" . ");
			}
			print_space_right(board, i, j + 1);
		}
		printf("\n");
		for (j = 0; j < board->cols; j++) {
			print_space_down(board, i + 1, j);
			printf("  ");
		}
		printf("\n");
	}	
	printf("\n");
}

/** Reads the islands from the standard input and adds them to the board.
 * Supported format: 02/000/1001/35/0202 (or '.' and '\n' for '0' and '/'). */
bool read_islands(hboard *board) {
	int c, row, col;
	row = col = 0;
	while ((c = getchar()) != EOF) {
		if (c == '/' || c == '\n') {
			row++;
			col = 0;
		} else if (c == '.' || c == '0') {
			col++;
		} else if (c > '0' && c <= '9') {
			if (! add_island(board, row, col, c - '0')) {
				return false;
			}
			col++;
		}
	}
	return true;
}

/** Adds a bridge to the given connection or returns false if cannot be done. */
bool add_bridge(hconnection *connection) {
	if (connection->bridges >= MAX_CONNECTION_BRIDGES) {
		return false;
	}
	if (*(connection->ppendbridges1) && *(connection->ppendbridges2)) {
		hcrosselem *cross;
		for (cross = connection->firstcross; cross != NULL;
						cross = cross->nextcross) {
			if (*(cross->pbridges)) {
				return false;
			}
		}
		connection->bridges++;
		(*(connection->ppendbridges1))--;
		(*(connection->ppendbridges2))--;
		return true;
	}
	return false;
}

/** Deletes a bridge from the given connection or returns false if cannot. */
bool del_bridge(hconnection *connection) {
	if (connection->bridges) {
		connection->bridges--;
		(*(connection->ppendbridges1))++;
		(*(connection->ppendbridges2))++;
		return true;
	}
	return false;
}

/** Fills the expected bridges in the given island or returns false if it
 * cannot be done. Only adds bridges in the directions of islands not already
 * visited (note that only the RIGHT and DOWN directions are filled).
 * This function uses the greedy algorithm and if it fails considers that the
 * island cannot be completed, and any added bridge will be deleted. */
bool fill_bridges(hisland *island) {
	int dir = RIGHT;
	while (island->pendbridges) {
		if (! add_bridge(island->connections[dir])) {
			if (++dir == DIRECTIONS) {
				break;
			}
		}
	}
	if (island->pendbridges) {
		while (del_bridge(island->connections[RIGHT]));
		while (del_bridge(island->connections[DOWN]));
		return false;
	}
	return true;
}

/** Reorders the expected bridges in the given completed island
 * or returns false if it cannot find another ordering for the bridges.
 * Only reorders bridges in the directions of islands not already visited
 * (note that only the RIGHT and DOWN directions are used to reorder bridges).
 * If a new ordering cannot be found, the previous added bridges are deleted. */
bool reorder_bridges(hisland *island) {
	if (del_bridge(island->connections[RIGHT])) {
		if (add_bridge(island->connections[DOWN])) {
			return true;
		}
		while (del_bridge(island->connections[RIGHT]));
	}
	while (del_bridge(island->connections[DOWN]));
	return false;
}

/** Visits recursively the given island and all its connected islands,
 * if not visited already, returning the total number of visited islands,
 * setting to true the positions of the matrix of the visited islands and
 * updating the limit of positions true in the matrix of visited islands. */
int visit_islands(hboard *board, hisland *island) {
	int total = 0, pos, i;
	if (island != board->out_island) {
		pos = island->row * board->cols + island->col;
		if (! board->visitedmatrix[pos]) {
			board->visitedmatrix[pos] = true;
			total++;
			if (pos + 1 > board->visitedlimit) {
				board->visitedlimit = pos + 1;
			}
			for (i = 0; i < DIRECTIONS; i++) {
				if (island->connections[i]->bridges) {
					total += visit_islands(board,
							island->islands[i]);
				}
			}
		}
	}
	return total;
}

/** Returns true if the given solution forms only one connected group. */
bool check_connected_solution(hboard *board) {
#ifdef CHECK_CONNECTED_SOLUTION
	int total;
	board->visitedlimit = 0;
	total = visit_islands(board, board->islands);
	if (board->visitedlimit) {
		memset(board->visitedmatrix, 0, board->visitedlimit);
		board->visitedlimit = 0;
	}
	if (total != board->num_islands) {
		return false;
	}
#endif
	return true;
}

/** Finds all solutions by brute force without mandatory bridges. */
void find_solutions_from_island(hboard* board, int idx) {
	if (idx >= board->num_islands) {
		if (check_connected_solution(board)) {
			print_board(board);
		}
		return;
	}
	if (fill_bridges(board->islands + idx)) {
		find_solutions_from_island(board, idx + 1);
		while (reorder_bridges(board->islands + idx)) {
			find_solutions_from_island(board, idx + 1);
		}
	}
}

bool valid_visited_matrix_size(hboard *board) {
	if (board->max_visited_size < board->rows * board->cols) {
		fprintf(stderr, "Maximum visited islands size too small: %d\n",
				board->max_visited_size);
		return false;
	}
	return true;
}

int main(void) {
	hisland islands[MAX_ISLANDS];
	hconnection connections[MAX_CONNECTIONS];
	hcrosselem crosselems[MAX_CROSSELEMS];
	bool visitedmatrix[MAX_VISITED_SIZE];
	hboard board;
	init_board(&board, islands, MAX_ISLANDS, connections, MAX_CONNECTIONS,
		crosselems, MAX_CROSSELEMS, visitedmatrix, MAX_VISITED_SIZE);
	if (! read_islands(&board)) {
		exit(-1);
	}
	print_board(&board);
	if (board.num_islands) {
		if (! valid_visited_matrix_size(&board)) {
			exit(-1);
		}
		find_solutions_from_island(&board, 0);
	}
	return 0;
}

