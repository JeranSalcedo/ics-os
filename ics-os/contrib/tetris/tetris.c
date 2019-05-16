#include "../../sdk/dexsdk.h"
#include "../../sdk/time.h"


#define not	 !=
#define over 1
#define bool int
#define true 0
#define false 1

#define queue_size	 4
#define board_width	 10
#define board_height 20
#define box_size 9

#define empty		0
#define occupied	1


void init();
void start();
void erase_block(int x_start, int x_end, int y_start, int y_end);
void renderScreen(bool init);
void renderQueue();
int dequeue();
bool stopPiece();
void changePiece();
void rowsClear();

/**************************************
 *	PIECES 							  *
 *		1:	** 			5:	*		  *
 *			**				*0*		  *
 *									  *
 *		2:	** 			6:	  *		  *
 *			 0*				*0*		  *
 *									  *
 *		3:	 ** 		7:	 *		  *
 *			*0				*0*		  *
 *									  *
 *		4:	****					  *
 *									  *
 **************************************/


int game, speed, playerPiece, previous_piece, pieceOrientation, previous_orientation, playerX, playerY, previous_x, previous_y, queueHead, queueTail, *pieceQueue, **board, padding;

int main(){
	init();
	start();

	return 0;
}

//	initializes game values
void init(){
	int i, j;

	//	for allowing generation of random numbers
	srand(time(NULL));

	set_graphics(VGA_320X200X256);

	game = 0;
	speed = 25;
	padding = box_size * (board_width + 2) + 20;
	queueHead = 0;
	queueTail = queue_size - 1;
	//	initializes player piece to null
	playerPiece = previous_piece = 0;
	pieceOrientation = previous_orientation = 0;
	playerX = 4;
	playerY = 0;
	previous_x = previous_y = -1;

	//	allocates memory for the queue
	pieceQueue = (int *) malloc(queue_size * sizeof(int));
	//	initializes the queue with random pieces
	for(i = 0; i < queue_size; i++){
		//	fills the array with an integer from 1 to 7
		pieceQueue[i] = 4;
	}

	//	allocates memory for the game board
	board = (int **) malloc(board_height * sizeof(int *));
	for(i = 0; i < board_height; i++){
		board[i] = (int *) malloc(board_width * sizeof(int));
		for(j = 0; j < board_width; j++){
			//	initializes the board as empty
			board[i][j] = empty;
		}
	}
}

//	starts the game
void start(){
	char command;

	renderScreen(true);
	while(game not over){
		//	resets speed
		speed = speed != 0? 25 : 0;

		//	if player has no current piece
		if(playerPiece == 0){
			//	gets a new piece from the queue
			playerPiece = dequeue();
			//	sets initial position to middle top part of play area
			playerX = 4;
			playerY = 0;
			speed = 25;
		}

		//	reads command
		if(kb_ready()){
			command = (char) getch();

			switch(command){
				case 'w':
					pieceOrientation = pieceOrientation == 3? 0 : pieceOrientation + 1;
					break;
				case 'a':
				//	need to check colisions from the left
					playerX--;
					break;
				case 'd':
				//	check from the right
					playerX++;
					break;
				case 's':
					speed = 7;
					break;
				case ' ':
					speed = 0;
			}
		}

		//	checks for filled rows and adjusts the game
		rowsClear();

		//	displays the current status of the board
		renderScreen(false);

		//	checks if player piece should be stopped
		if(stopPiece() == false){
			//	if not, keep making piece fall
			playerY++;
		} else {
			changePiece();
		}

		// waits for a while
		delay(speed);
	}
}

//	erases a block from the screen
void erase_block(int x_start, int x_end, int y_start, int y_end){
	int x, y;

	for(y = y_start; y < y_end; y++){
		for(x = x_start; x < x_end; x++){
			write_pixel(x, y, BLACK);
		}
	}
}

//	prints the screen
void renderScreen(bool init){
	int x, y;

	//	if first time calling
	if(init == true){
		//	renders the borders of the game board
		for(y = 0; y < box_size * (board_height + 2); y++){
			for(x = 0; x < box_size * (board_width + 2); x++){
				//	skips the middle parts of the board
				if(y >= box_size && y < box_size * board_height + box_size && x >= box_size && x < box_size * board_width + box_size){
					continue;
				}

				write_pixel(x, y, LIGHTGRAY);
			}
		}

		//	renders the side ui
		write_text("NEXT", padding, 10, WHITE, 0);
		renderQueue();

		return;
	}

	//	erases the previous render
	if(previous_y != -1){
		switch(previous_piece){
			case 1:
				erase_block(
					(previous_x + 1) * box_size,
					(previous_x + 3) * box_size, 
					previous_y * box_size < box_size? box_size : previous_y * box_size,
					(previous_y + 2) * box_size
				);
				break;
			case 2:
				switch(previous_orientation){
					case 0:
						erase_block(
							previous_x * box_size, (previous_x + 2) * box_size,
							previous_y * box_size < box_size? box_size : previous_y * box_size,
							(previous_y + 1) * box_size
						);
						erase_block(
							(previous_x + 1) * box_size, (previous_x + 3) * box_size,
							(previous_y + 1) * box_size,
							(previous_y + 2) * box_size
						);
						break;
					case 1:
						erase_block(
							(previous_x + 2) * box_size, (previous_x + 3) * box_size,
							previous_y * box_size < box_size? box_size : previous_y * box_size,
							(previous_y + 2) * box_size
						);
						erase_block(
							(previous_x + 1) * box_size, (previous_x + 2) * box_size,
							(previous_y + 1) * box_size,
							(previous_y + 3) * box_size
						);
						break;
					case 2:
						erase_block(
							previous_x * box_size, (previous_x + 2) * box_size,
							(previous_y + 1) * box_size,
							(previous_y + 2) * box_size
						);
						erase_block(
							(previous_x + 1) * box_size, (previous_x + 3) * box_size,
							(previous_y + 2) * box_size,
							(previous_y + 3) * box_size
						);
						break;
					case 3:
						erase_block(
							(previous_x + 1) * box_size, (previous_x + 2) * box_size,
							previous_y * box_size < box_size? box_size : previous_y * box_size,
							(previous_y + 2) * box_size
						);
						erase_block(
							previous_x * box_size, (previous_x + 1) * box_size,
							(previous_y + 1) * box_size,
							(previous_y + 3) * box_size
						);
				}
		}
	}

	//	renders the current position of the player's piece
	switch(playerPiece){
		case 1:
			for(y = playerY * box_size; y < (playerY + 2) * box_size; y++){
				if(y < box_size){
					continue;
				}
				for(x = (playerX + 1) * box_size; x < (playerX + 3) * box_size; x++){
					write_pixel(x, y, YELLOW);
				}
			}
			break;
		case 2:
			switch(pieceOrientation){
				case 0:
					for(y = playerY * box_size; y < (playerY + 2) * box_size; y++){
						if(y < box_size){
							continue;
						}
						for(x = playerX * box_size; x < (playerX + 3) * box_size; x++){
							if(y < (playerY + 1) * box_size && x >= (playerX + 2) * box_size || y >= (playerY + 1) * box_size && x < (playerX + 1) * box_size){
								continue;
							}
							write_pixel(x, y, RED);
						}
					}
					break;
				case 1:
					for(y = playerY * box_size; y < (playerY + 3) * box_size; y++){
						if(y < box_size){
							continue;
						}
						for(x = (playerX + 1) * box_size; x < (playerX + 3) * box_size; x++){
							if(y < (playerY + 1) * box_size && x < (playerX + 2) * box_size || y >= (playerY + 2) * box_size && x >= (playerX + 2) * box_size){
								continue;
							}
							write_pixel(x, y, RED);
						}
					}
					break;
				case 2:
					for(y = (playerY + 1) * box_size; y < (playerY + 3) * box_size; y++){
						for(x = playerX * box_size; x < (playerX + 3) * box_size; x++){
							if(y < (playerY + 2) * box_size && x >= (playerX + 2) * box_size || y >= (playerY + 2) * box_size && x < (playerX + 1) * box_size){
								continue;
							}
							write_pixel(x, y, RED);
						}
					}
					break;
				case 3:
					for(y = playerY * box_size; y < (playerY + 3) * box_size; y++){
						if(y < box_size){
							continue;
						}
						for(x = playerX * box_size; x < (playerX + 2) * box_size; x++){
							if(y < (playerY + 1) * box_size && x < (playerX + 1) * box_size || y >= (playerY + 2) * box_size && x >= (playerX + 1) * box_size){
								continue;
							}
							write_pixel(x, y, RED);
						}
					}
			}
			break;
		case 3:
			switch(pieceOrientation){
				case 0:
					for(y = playerY * box_size; y < (playerY + 2) * box_size; y++){
						if(y < box_size){
							continue;
						}
						for(x = playerX * box_size; x < (playerX + 3) * box_size; x++){
							if(y < (playerY + 1) * box_size && x < (playerX + 1) * box_size  || y >= (playerY + 1) * box_size && x >= (playerX + 2) * box_size){
								continue;
							}
							write_pixel(x, y, GREEN);
						}
					}
					break;
				case 1:
					for(y = playerY * box_size; y < (playerY + 3) * box_size; y++){
						if(y < box_size){
							continue;
						}
						for(x = (playerX + 1) * box_size; x < (playerX + 3) * box_size; x++){
							if(y < (playerY + 1) * box_size && x >= (playerX + 2) * box_size || y >= (playerY + 2) * box_size && x < (playerX + 2) * box_size){
								continue;
							}
							write_pixel(x, y, GREEN);
						}
					}
					break;
				case 2:
					for(y = (playerY + 1) * box_size; y < (playerY + 3) * box_size; y++){
						for(x = playerX * box_size; x < (playerX + 3) * box_size; x++){
							if(y < (playerY + 2) * box_size && x < (playerX + 1) * box_size  || y >= (playerY + 2) * box_size && x >= (playerX + 2) * box_size){
								continue;
							}
							write_pixel(x, y, GREEN);
						}
					}
					break;
				case 3:
					for(y = playerY * box_size; y < (playerY + 3) * box_size; y++){
						if(y < box_size){
							continue;
						}
						for(x = playerX * box_size; x < (playerX + 2) * box_size; x++){
							if(y < (playerY + 1) * box_size && x >= (playerX + 1) * box_size || y >= (playerY + 2) * box_size && x < (playerX + 1) * box_size){
								continue;
							}
							write_pixel(x, y, GREEN);
						}
					}
					break;

			}
			break;
		case 4:
			switch(pieceOrientation){
				case 0:
					for(y = playerY * box_size; y < (playerY + 1) * box_size; y++){
						if(y < box_size){
							continue;
						}
						for(x = (playerX + 1) * box_size; x < (playerX + 4) * box_size; x++){	
							write_pixel(x, y, CYAN);
						}
					}
				break;
				case 1:
					for(y = (playerY + 1) * box_size; y < (playerY + 4) * box_size; y++){
						if(y < box_size){
							continue;
						}
						for(x = playerX * box_size; x < (playerX + 1) * box_size; x++){	
							write_pixel(x, y, CYAN);
						}
					}
				break;
				case 2:
					for(y = playerY * box_size; y < (playerY + 1) * box_size; y++){
						if(y < box_size){
							continue;
						}
						for(x = (playerX + 2) * box_size; x < (playerX + 4) * box_size; x++){	
							write_pixel(x, y, CYAN);
						}
					}
				break;
				case 3:
					for(y = (playerY + 2) * box_size; y < (playerY + 4) * box_size; y++){
						if(y < box_size){
							continue;
						}
						for(x = playerX * box_size; x < (playerX + 1) * box_size; x++){	
							write_pixel(x, y, CYAN);
						}
					}
				break;
			}
			// break;
		// case 5:
			// break;
		// case 6:
			// break;
		// case 7:
	}
	previous_x = playerX;
	previous_y = playerY;
	previous_piece = playerPiece;
	previous_orientation = pieceOrientation;
}

void renderQueue(){
	int x, y, i, pointer;

	for(i = 0, pointer = queueHead; i < queue_size; i++, pointer = pointer + 1 == queue_size? 0 : pointer + 1){
		switch(pieceQueue[pointer]){
			case 1:
				for(y = 10; y < 20; y++){
					for(x = padding + 45 + i * 20; x < padding + 55 + i * 20; x++){
						write_pixel(x, y, YELLOW);
					}
				}
				break;
			case 2:
				for(y = 10; y < 20; y++){
					for(x = padding + 45 + i * 20; x < padding + 60 + i * 20; x++){
						if(y < 15 && x >= padding + 55 + i * 20 || y >= 15 && x < padding + 50 + i * 20){
							continue;
						}
						write_pixel(x, y, RED);
					}
				}
				break;
			case 3:
				for(y = 10; y < 20; y++){
					for(x = padding + 45 + i * 20; x < padding + 60 + i * 20; x++){
						if(y >= 15 && x >= padding + 55 + i * 20 || y < 15 && x < padding + 50 + i * 20){
							continue;
						}
						write_pixel(x, y, GREEN);
					}
				}
				break;
			case 4:
				for(y = 15; y < 20; y++){
					for(x = padding + 43 + i * 20; x < padding + 62 + i * 20; x++){
						write_pixel(x, y, /*LIGHTBLUE*/ CYAN);
					}
				}
				break;
			case 5:
				for(y = 10; y < 20; y++){
					for(x = padding + 45 + i * 20; x < padding + 60 + i * 20; x++){
						if(y < 15 && x >= padding + 50 + i * 20){
							continue;
						}
						write_pixel(x, y, BLUE);
					}
				}
				break;
			case 6:
				for(y = 10; y < 20; y++){
					for(x = padding + 45 + i * 20; x < padding + 60 + i * 20; x++){
						if(y < 15 && x < padding + 55 + i * 20){
							continue;
						}
						write_pixel(x, y, BROWN);
					}
				}
				break;
			case 7:
				for(y = 10; y < 20; y++){
					for(x = padding + 45 + i * 20; x < padding + 60 + i * 20; x++){
						if(y < 15 && (x < padding + 50 + i * 20 || x >= padding + 55 + i * 20)){
							continue;
						}
						write_pixel(x, y, MAGENTA);
					}
				}
		}
	}
}

int dequeue(){
	//	saves the next piece to be returned
	int returnVal = pieceQueue[queueHead]; 
	
	//	queue head and tail are adjusted
	queueTail = queueHead;
	queueHead = queueHead + 1 == queue_size? 0 : queueHead + 1;

	//	enqueues a new piece
	pieceQueue[queueTail] = 4;

	// erases the queued pieces
	erase_block(padding + 43, padding + 127, 10, 20);
	renderQueue();

	return returnVal;
}

bool stopPiece(){
	if(
		//	checks origin
		(
			playerPiece == 1 ||
			playerPiece == 2 &&
			(
				pieceOrientation == 0 ||
				pieceOrientation == 3
			)
			||
			playerPiece == 3 &&
			(
				pieceOrientation == 0 ||
				pieceOrientation == 1
			)
			||
			playerPiece == 4 &&
			(
				pieceOrientation == 0 ||
				pieceOrientation == 2
			)
			||
			playerPiece == 5 &&
			(
				pieceOrientation == 0 ||
				pieceOrientation == 2
			)
			||
			playerPiece == 6 &&
			(
				pieceOrientation == 0 ||
				pieceOrientation == 3
			)
			||
			playerPiece == 7 && pieceOrientation == 0
		)
		&&
		(
			playerY == board_height - 1 ||
			playerY < board_height - 1 && board[playerY + 1][playerX] != empty
		)
		||
		//	checks left
		(
			playerPiece == 2 && pieceOrientation == 2 ||
			playerPiece == 3 &&
			(
				pieceOrientation == 0 ||
				pieceOrientation == 3
			)
			||
			playerPiece == 4 &&
			(
				pieceOrientation == 0 ||
				pieceOrientation == 2
			)
			||
			playerPiece == 5 &&
			(
				pieceOrientation == 0 ||
				pieceOrientation == 2
			)
			||
			playerPiece == 6 && pieceOrientation == 0 ||
			playerPiece == 7 && pieceOrientation != 1
		)
		&&
		(
			playerY == board_height - 1 ||
			playerY < board_height - 1 && board[playerY + 1][playerX - 1] != empty
		)
		||
		//	checks left left
		playerPiece == 4 && pieceOrientation == 2 &&
		(
			playerY == board_height - 1 ||
			playerY < board_height - 1 && board[playerY + 1][playerX - 2] != empty
		)
		||
		//	checks right
		(
			playerPiece == 1 ||
			playerPiece == 2 &&
			(
				pieceOrientation == 0 ||
				pieceOrientation == 1
			)
			||
			playerPiece == 3 && pieceOrientation == 2 ||
			playerPiece == 4 &&
			(
				pieceOrientation == 0 ||
				pieceOrientation == 2
			)
			||
			playerPiece == 5 && pieceOrientation == 0 ||
			playerPiece == 6 &&
			(
				pieceOrientation == 0 ||
				pieceOrientation == 2
			)
			||
			playerPiece == 7 && pieceOrientation != 3
		)
		&&
		(
			playerY == board_height - 1 ||
			playerY < board_height - 1 && board[playerY + 1][playerX + 1] != empty
		)
		||
		//	checks right right
		playerPiece == 4 && pieceOrientation == 0 &&
		(
			playerY == board_height - 1 ||
			playerY < board_height - 1 && board[playerY + 1][playerX + 2] != empty
		)
		||
		//	checks down
		(
			playerPiece == 2 &&
			(
				pieceOrientation == 1 ||
				pieceOrientation == 2
			)
			||
			playerPiece == 3 &&
			(
				pieceOrientation == 2 ||
				pieceOrientation == 3
			)
			||
			playerPiece == 4 &&
			(
				pieceOrientation == 1 ||
				pieceOrientation == 3
			)
			||
			(
				playerPiece == 5 ||
				playerPiece == 6
			)
			&&
			(
				pieceOrientation == 1 ||
				pieceOrientation == 3
			)
			||
			playerPiece == 7 && pieceOrientation != 0
		)
		&&
		(
			playerY + 1 == board_height - 1 ||
			playerY + 1 < board_height - 1 && board[playerY + 2][playerX] != empty
		)
		||
		//	checks down down
		playerPiece == 4 && pieceOrientation == 1 &&
		(
			playerY + 2 == board_height - 1 ||
			playerY + 2 < board_height - 1 && board[playerY + 3][playerX] != empty
		)
		||
		//	checks upper left
		(
			playerPiece == 2 && pieceOrientation == 0 ||
			playerPiece == 6 && pieceOrientation == 3
		)
		&&
		(
			board[playerY][playerX - 1] != empty
		)
		||
		//	checks upper right
		(
			playerPiece == 3 && pieceOrientation == 0 ||
			playerPiece == 5 && pieceOrientation == 1
		)
		&&
		(
			board[playerY][playerX + 1] != empty
		)
		||
		//	checks lower left
		(
			(playerPiece == 2 || playerPiece == 5) && pieceOrientation == 3 ||
			(playerPiece == 3 || playerPiece == 6) && pieceOrientation == 2
		)
		&&
		(
			playerY + 1 == board_height - 1 ||
			playerY + 1 < board_height - 1 && board[playerY + 2][playerX - 1] != empty
		)
		||
		//	checks lower right
		(
			(playerPiece == 2 || playerPiece == 5) && pieceOrientation == 2 ||
			(playerPiece == 3 || playerPiece == 6) && pieceOrientation == 1
		)
		&&
		(
			playerY + 1 == board_height - 1 ||
			playerY + 1 < board_height - 1 && board[playerY + 2][playerX + 1] != empty
		)
	){
		return true;
	}

	return false;
}

void changePiece(){
	int x, y;

	//	origin
	for(y = (playerY + 1) * box_size; y < (playerY + 2) * box_size; y++){
		for(x = (playerX + 1) * box_size; x < (playerX + 2) * box_size; x++){
			write_pixel(x, y, DARKGRAY);
		}
	}
	board[playerY][playerX] = occupied;
	//	up
	if(
		playerY > 0 &&
		(
			playerPiece == 1 ||
			playerPiece == 2 &&
			(
				pieceOrientation == 0 ||
				pieceOrientation == 3
			)
			||
			playerPiece == 3 &&
			(
				pieceOrientation == 0 ||
				pieceOrientation == 1
			)
			||
			(
				playerPiece == 4 ||
				playerPiece == 5 ||
				playerPiece == 6
			)
			&&
			(
				pieceOrientation == 1 ||
				pieceOrientation == 3
			)
			||
			playerPiece == 7 && pieceOrientation != 2
		)
	){
		for(y = playerY * box_size; y < (playerY + 1) * box_size; y++){
			for(x = (playerX + 1) * box_size; x < (playerX + 2) * box_size; x++){
				write_pixel(x, y, DARKGRAY);
			}
		}
		board[playerY - 1][playerX] = occupied;
	}
	//	up up
	if(playerY > 1 && playerPiece == 4 && pieceOrientation == 3){
		for(y = (playerY - 1) * box_size; y < playerY * box_size; y++){
			for(x = (playerX + 1) * box_size; x < (playerX + 2) * box_size; x++){
				write_pixel(x, y, DARKGRAY);
			}
		}
		board[playerY - 2][playerX] = occupied;
	}
	//	left
	if(
		playerPiece == 2 &&
		(
			pieceOrientation == 2 ||
			pieceOrientation == 3
		)
		||
		playerPiece == 3 &&
		(
			pieceOrientation == 0 ||
			pieceOrientation == 3
		)
		||
		playerPiece == 4 &&
		(
			pieceOrientation == 0 ||
			pieceOrientation == 2
		)
		||
		(
			playerPiece == 5 ||
			playerPiece == 6
		)
		&&
		(
			pieceOrientation == 0 ||
			pieceOrientation == 2
		)
		||
		playerPiece == 7 && pieceOrientation != 1
	){
		for(y = (playerY + 1) * box_size; y < (playerY + 2) * box_size; y++){
			for(x = playerX * box_size; x < (playerX + 1) * box_size; x++){
				write_pixel(x, y, DARKGRAY);
			}
		}
		board[playerY][playerX - 1] = occupied;
	}
	//	left left
	if(playerPiece == 4 && pieceOrientation == 2){
		for(y = (playerY + 1) * box_size; y < (playerY + 2) * box_size; y++){
			for(x = (playerX - 1) * box_size; x < playerX * box_size; x++){
				write_pixel(x, y, DARKGRAY);
			}
		}
		board[playerY][playerX - 2] = occupied;
	}
	//	right
	if(
		playerPiece == 1 ||
		playerPiece == 2 &&
		(
			pieceOrientation == 0 ||
			pieceOrientation == 1
		)
		||
		playerPiece == 3 &&
		(
			pieceOrientation == 1 ||
			pieceOrientation == 2
		)
		||
		playerPiece == 4 &&
		(
			pieceOrientation == 0 ||
			pieceOrientation == 2
		)
		||
		(
			playerPiece == 5 ||
			playerPiece == 6
		)
		&&
		(
			pieceOrientation == 0 ||
			pieceOrientation == 2
		)
		||
		playerPiece == 7 && pieceOrientation != 3
	){
		for(y = (playerY + 1) * box_size; y < (playerY + 2) * box_size; y++){
			for(x = (playerX + 2) * box_size; x < (playerX + 3) * box_size; x++){
				write_pixel(x, y, DARKGRAY);
			}
		}
		board[playerY][playerX + 1] = occupied;
	}
	//	right right
	if(playerPiece == 4 && pieceOrientation == 0){
		for(y = (playerY + 1) * box_size; y < (playerY + 2) * box_size; y++){
			for(x = (playerX + 3) * box_size; x < (playerX + 4) * box_size; x++){
				write_pixel(x, y, DARKGRAY);
			}
		}
		board[playerY][playerX + 2] = occupied;
	}
	//	down
	if(
		playerPiece == 2 &&
		(
			pieceOrientation == 1 ||
			pieceOrientation == 2
		)
		||
		playerPiece == 3 &&
		(
			pieceOrientation == 2 ||
			pieceOrientation == 3
		)
		||
		(
			playerPiece == 4 ||
			playerPiece == 5 ||
			playerPiece == 6
		)
		&&
		(
			pieceOrientation == 1 ||
			pieceOrientation == 3
		)
		||
		playerPiece == 7 && pieceOrientation != 0
	){
		for(y = (playerY + 2) * box_size; y < (playerY + 3) * box_size; y++){
			for(x = (playerX + 1) * box_size; x < (playerX + 2) * box_size; x++){
				write_pixel(x, y, DARKGRAY);
			}
		}
		board[playerY + 1][playerX] = occupied;
	}
	//	down down
	if(playerPiece == 4 && pieceOrientation == 1){
		for(y = (playerY + 3) * box_size; y < (playerY + 4) * box_size; y++){
			for(x = (playerX + 1) * box_size; x < (playerX + 2) * box_size; x++){
				write_pixel(x, y, DARKGRAY);
			}
		}
		board[playerY + 2][playerX] = occupied;
	}
	//	upper left
	if(
		playerY > 0 &&
		(
			(playerPiece == 2 || playerPiece == 5) && pieceOrientation == 0 ||
			(playerPiece == 3 || playerPiece == 6) && pieceOrientation == 3
		)
	){
		for(y = playerY * box_size; y < (playerY + 1) * box_size; y++){
			for(x = playerX * box_size; x < (playerX + 1) * box_size; x++){
				write_pixel(x, y, DARKGRAY);
			}
		}
		board[playerY - 1][playerX - 1] = occupied;
	}
	//	upper right
	if(
		playerY > 0 &&
		(
			playerPiece == 1 ||
			(playerPiece == 2 || playerPiece == 5) && pieceOrientation == 1 ||
			(playerPiece == 3 || playerPiece == 6) && pieceOrientation == 0
		)
	){
		for(y = playerY * box_size; y < (playerY + 1) * box_size; y++){
			for(x = (playerX + 2) * box_size; x < (playerX + 3) * box_size; x++){
				write_pixel(x, y, DARKGRAY);
			}
		}
		board[playerY - 1][playerX + 1] = occupied;
	}
	//	lower left
	if(
		(playerPiece == 2 || playerPiece == 5) && pieceOrientation == 3 ||
		(playerPiece == 3 || playerPiece == 6) && pieceOrientation == 2
	){
		for(y = (playerY + 2) * box_size; y < (playerY + 3) * box_size; y++){
			for(x = playerX * box_size; x < (playerX + 1) * box_size; x++){
				write_pixel(x, y, DARKGRAY);
			}
		}
		board[playerY + 1][playerX - 1] = occupied;
	}
	//	lower right
	if(
		(playerPiece == 2 || playerPiece == 5) && pieceOrientation == 2 ||
		(playerPiece == 3 || playerPiece == 6) && pieceOrientation == 1
	){
		for(y = (playerY + 2) * box_size; y < (playerY + 3) * box_size; y++){
			for(x = (playerX + 1) * box_size; x < (playerX + 2) * box_size; x++){
				write_pixel(x, y, DARKGRAY);
			}
		}
		board[playerY + 1][playerX + 1] = occupied;
	}

	//	removes current player piece
	playerPiece = pieceOrientation = 0;
	previous_x = previous_y = -1;
}

void rowsClear(){
	int x, y, i, j, k;
	bool isClear;

	//	from top -> bottom, checks for filled rows
	for(y = 0; y < board_height; y++){
		//	initializes the flag to true
		isClear = true;
		//	if even one cell is empty, skips the row
		for(x = 0; x < board_width; x++){
			if(board[y][x] == empty){
				isClear = false;
				break;
			}
		}

		//	when a filled row is found
		if(isClear == true){
			//	from current row -> 2nd row from the top
			for(i = y; i > 0; i--){
				//	erases whole row
				erase_block(box_size, (board_width + 1) * box_size, (i + 1) * box_size, (i + 2) * box_size);
				//	for each cell in the row
				for(x = 0; x < board_width; x++){
					//	transfers the values from the row above to current row
					board[i][x] = board[i - 1][x];

					//	if cell is occupied
					if(board[i][x] == occupied){
						//	recolors the cell to display it being occupied
						for(j = (i + 1) * box_size; j < (i + 2) * box_size; j++){
							for(k = (x + 1) * box_size; k < (x + 2) * box_size; k++){
								write_pixel(j, k, DARKGRAY);
							}
						}
					}
				}
			}
			//	erases topmost row
			erase_block(box_size, (board_width + 1) * box_size, box_size, 2 * box_size);
			for(x = 0; x < board_width; x++){
				board[0][x] = empty;
			}
		}
	}
}

/*
	PIECE_TYPE(1-7), ORIENTATION(0-3):

	1,x:		5,0:
	 **			*
	 **			*0*
	
	2,0:		5,1:
	**			 **
	 0*			 0
	 			 *

	2,1:	 			 
	  *			5,2:
	 0*			*0*
	 *			  *

	2,2:		5,3:
	*0			 *
	 **			 0
	 			**
	2,3:
	 *			6,0:
	*0			  *
	*			*0*

	3,0:		6,1:
	 **			 *
	*0			 0
				 **
	3,1:
	 *			6,2:
	 0*			*0*
	  *			*

	3,2:		6,3:
	 0*			**
	**			 0
				 *
	3,3:
	*			7,0:
	*0			 *
	 *			*0*

	4,0:		7,1:
	*0**		 *
				 0*
	4,1:		 *
	 *			 
	 0			7,2:
	 *			*0*
	 *			 *

	4,2:		7,3:
	**0*		 *
				*0
	4,3:		 *
	  *	
	  *
	  0
	  *
*/