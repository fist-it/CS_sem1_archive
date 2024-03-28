#ifndef GAME_TYPES_H
#define GAME_TYPES_H
#define _CRT_SECURE_NO_WARNINGS
#include "support_functions.h"

//wlaczenie/wylaczenie konsoli:
// project -> szablon2 properties -> Linker -> System -> Subsystem





class Ladder {
private:
	SDL_Surface* sprite; // sprite platformy


public:
	int x, // srodek drabiny w px
		y, // dol drabiny w px
		height; // wysokosc drabiny w 20*px (ilosc blokow)

	int x1, // lewy bok drabiny w px
		y1, // gorny bok drabiny w px
		x2, // prawy bok drabinyw px
		y2; // dolny bok drabiny w px


	void draw(SDL_Surface* screen) {
		int squares = height;
		for (int i = 0; i < squares; i++) {
			DrawSurface(screen, sprite, x, y - SQUARE_SIZE / 2 - i * SQUARE_SIZE);
		}
	}
	Ladder() {
		x = 0;
		y = 0;
		height = 0;
		x1 = 0;
		y1 = 0;
		x2 = 0;
		y2 = 0;
		sprite = nullptr;
	}
	Ladder(SDL_Surface* screen, SDL_Surface* sprite, int x, int y, int height) {
		this->x = x;
		this->y = y;
		this->height = height;
		this->sprite = sprite;
		this->x1 = x - SQUARE_SIZE/ 2;
		this->y1 = y - height * SQUARE_SIZE;
		this->x2 = x + SQUARE_SIZE / 2;
		this->y2 = y;
		//jesli wysokosc drabiny jest rowna 0, koordynaty nie na planszy
		if (height == 0) {
			this->y1 = -1;
			this->y2 = -1;
		}
	}
	void print_debug() {
		printf("x1: %d, y1: %d, x2: %d, y2: %d\n", x1, y1, x2, y2);
	}
};

class Platform {
private:
	SDL_Surface* sprite; // sprite platformy
public:
	int width;
	Ladder ladder;

	int x; // srodek platformy w px
	int y; // srodek platformy w px
	// (x1, y1) ---- (x2, y1)
	//    |				 |
	//    |              |
	// (x1, y2) ---- (x2, y2)
	int x1, // lewy bok platformy w px
		y1, // gorny bok platformy w px
		x2, // prawy bok platformy w px
		y2; // dolny bok platformy w px


	void draw(SDL_Surface* screen) {
		int squares = width / SQUARE_SIZE;
		for (int i = 0; i < squares; i++) {
			DrawSurface(screen, sprite, x1 + SQUARE_SIZE/2 + i * SQUARE_SIZE, y);
		}
	}
	//konstruktor platformy, przyjmuje srodek platformy w (x, y), 
	Platform(SDL_Surface* screen, SDL_Surface* sprite, float block_x, float block_y, int width, int ladder_block, SDL_Surface* ladder_sprite, int ladder_block_height) {
		this->x =  (int)(SQUARE_SIZE * block_x);
		this->y = SQUARE_SIZE * block_y;
		this->width = width;
		this->x1 = x - width/2;
		this->y1 = y - SQUARE_SIZE / 2;
		this->x2 = x + width/2;
		this->y2 = y + SQUARE_SIZE / 2;
		this->sprite = sprite;
		this->ladder = Ladder(screen, ladder_sprite, x1 + ladder_block * SQUARE_SIZE - SQUARE_SIZE/2, y1, ladder_block_height);
	}

};





//ruszający się obiekt (gracz, beczka, itp.)
class Object {
public:
	int speed = 200; // default: 200
	int height = 20;
	int width = 16;
	double x;
	double y;
	double ySpeed;
	double gravity = 400.0; // default: 400.0
	int direction = 1;
	void left(double delta) {
		x -= speed * delta;
	}
	void right(double delta) {
		x += speed * delta;
	}
	void applygravity(double delta) {
		ySpeed += gravity * delta;
		y += ySpeed * delta;
	}

	void down(double delta) {
		y += speed * delta;
	}

	void up(double delta) {
		y -= speed * delta;
	}
	//kolizja z obiektem
	int check_border_collision() {
		if (x + width / 2 >= SCREEN_WIDTH) {
			x = SCREEN_WIDTH - width / 2;
		}
		else if (x - width / 2 <= 0) {
			x = width / 2;
		}
		if (y + height / 2 >= SCREEN_HEIGHT) {
			y = SCREEN_HEIGHT - height / 2;
			return 1;
		}
		else if (y - height / 2 <= 0) {
			y = height / 2;
			return 1;
		}
		return 0;
	}
	//kolizja z platforma
	int checkplatformcollision(const Platform* platform, int onladder) {
		if (platform->x1 <= x && platform->x2 >= x) {
			if (y + height / 2 >= platform->y1 && y + height / 2 <= platform->y2) {
				if (!onladder) {
					y = platform->y1 - height / 2;
				}
				return 1;
			}
			else if (y + height / 2 <= platform->y2 && y - height / 2 >= platform->y1) {
				if (!onladder) {
				y = platform->y2 + height / 2;
				}
				return 1;
			}
		}
		return 0;
	}

	//rysowanie obiektu w miejscu (x, y)
	virtual void draw(SDL_Surface* screen, SDL_Surface* sprite) {
		DrawSurface(screen, sprite, (int)x, (int)y);
	}
	Object(SDL_Surface* screen, SDL_Surface* sprite, int startX, int startY) {
		this->x = startX;
		this->y = startY;
		ySpeed = 0.0;
		draw(screen, sprite);
	}
};


class Barrel : public Object {
private:
	int direction = 0;
	void check_wall_collision() {
		if (x + width / 2 >= SCREEN_WIDTH && y != SCREEN_HEIGHT - height / 2) {
			direction = 1;
		}
		else if (x - width / 2 <= 0 && y != SCREEN_HEIGHT - height / 2) {
			direction = 0;
		}
	}
public:
	int platformcol = 0;
	SDL_Surface* default_sprite_L;
	SDL_Surface* default_sprite_R;

	void update(double delta, SDL_Surface* screen) {
		check_wall_collision();
		if (check_border_collision() || platformcol) {
			ySpeed = 0;
		}
		else {
			applygravity(delta);
		}
		if (direction) {
			left(delta);
		}
		else {
			right(delta);
		}
		draw(screen, default_sprite_L);
	}
	Barrel(SDL_Surface* screen, SDL_Surface* sprite, int startX, int startY) : Object(screen, sprite, startX, startY) {
		this->x = startX;
		this->y = startY;
		ySpeed = 0;
		this->default_sprite_L = sprite;
		this->default_sprite_R = sprite;
		speed = 100;
		gravity = 50;
		height = 16;
		draw(screen, sprite);
	}
};


class Player : public Object {
private:
	double jumpSpeed = 150; // Adjust the jump speed as needed, default: 150
	double initialY;
	double jumpHeight = 5; // Adjust the jump height as needed, default: 5
	int level = 1;
	SDL_Surface* default_sprite_R;
	SDL_Surface* default_sprite_L;
public:
	int ismoving = 0; // flag for animation
	int isJumping = 0; // flag for jumping animation
	int animation_phase = 0; // animation phase
	int barrelcol = 0;

	// Override the draw method to add any additional drawing logic for the player
	void draw(SDL_Surface* screen) {
		// Add any additional drawing logic for the player
		if (direction) {
			Object::draw(screen, default_sprite_R);
		}
		else {
			Object::draw(screen, default_sprite_L);
		}
	}

	// Constructor for the Player class
	Player(SDL_Surface* screen, SDL_Surface* sprite1, SDL_Surface* sprite2, int x, int y) : Object(screen, sprite1, x, y) {
		isJumping = 0;
		initialY = 0;
		this->default_sprite_R = sprite1;
		this->default_sprite_L = sprite2;
	}
	~Player() {
		SDL_FreeSurface(default_sprite_L);
		SDL_FreeSurface(default_sprite_R);
	}

	// Function to handle jumping
	// During the period when player jumped less than jumpHeight, player is moving at speed -jumpSpeed without gravity
	// after reaching jumpHeight, player is being slowed down from -jumpSpeed by gravity, starting to fall
	void jump(double delta, int platformcol, int onladder, int cheatladder) {
		if ((!isJumping && (check_border_collision() || platformcol)) && !onladder && !cheatladder) {
			initialY = y;
			// Only jump if not already jumping
			isJumping = 1;
			y -= jumpSpeed * delta;
		}
		if (onladder || cheatladder) {
			y -= jumpSpeed * delta;
		}
	}

	// Function to check if the player won the game
	int did_win() {
		if (x < WINNING_X && y < WINNING_Y) {
			return 1;
		}
		else {
			return 0;
		}
	}

	// Function to check if the player got hit by a barrel
	int check_barrel_collision(Barrel* barrel) {
		if (barrel->x + barrel->width / 2 >= x - width / 2 && barrel->x - barrel->width / 2 <= x + width / 2) {
			if (barrel->y + barrel->height / 2 >= y - height / 2 && barrel->y - barrel->height / 2 <= y + height / 2) {
				return 1;
			}
		}
		return 0;
	}

	// Function to update the player state (e.g., called in the game loop)
	void update(double delta, int platformcol, int onladder, int cheatladder) {


		if ((check_border_collision() || platformcol) && !onladder && !cheatladder) {
			ySpeed = 0;
			isJumping = 0;
		}
		else  if (!onladder && !cheatladder) {
			applygravity(delta);
		}
		if (isJumping && (!check_border_collision() && !platformcol)) {
			if (ySpeed >= 0) {
				ySpeed = -jumpSpeed;
			}
			// If jumping, continue moving upwards until reaching the apex
			if (y <= initialY - jumpHeight) {
				// Reached the jumpHeight, start gravity
				isJumping = 0;
			}
		}
	}
	
	//return 1 if player is on ladder
	int on_ladder(const Platform* platform) {
		if (platform->ladder.y2 == y + SQUARE_SIZE/2) {
			return 0;
		}
		if (platform->ladder.x1 <= x && platform->ladder.x2 >= x) {
			if ((int)y >= platform->ladder.y1 && (int)y <= platform->ladder.y2) {
				ySpeed = 0;
				return 1;
			}
			else if ((int)y <= platform->ladder.y2 && (int)y + SQUARE_SIZE/2 >= platform->ladder.y1) {
				ySpeed = 0;
				return 1;
			};
		}
		return 0;
	}



	void restart() {
		this->x = SCREEN_WIDTH - SQUARE_SIZE;
		this->y = SCREEN_HEIGHT - height/2;
		ySpeed = 0;
		isJumping = 0;
	}
};


//TODO - klasa Kong, animacje antagonisty powiazane z rzuconymi beczkami
class Kong {
private:
	SDL_Surface* pickup_sprite;
	SDL_Surface* holding_sprite;
	SDL_Surface* throwing_sprite;
public:
	int throwing_speed;
	int throwing_phase = 0;
	int timer;
	void update(double delta) {
	}
};


//klasa przechowujaca dynamiczna tablice platform
class Dynamic_Platform_Array {
private:
	Platform** platforms;  // Dynamic array of pointers to Platform objects
	int capacity;

public:
	int size;
	Dynamic_Platform_Array() : platforms(nullptr), capacity(0), size(0) {}

	~Dynamic_Platform_Array() {
		clear();
	}

	// Add a platform to the array
	void addPlatform(SDL_Surface* screen, SDL_Surface* sprite, float block_x, float block_y, int width, int ladder_block, SDL_Surface* ladder_sprite, int ladder_height) {
		if (size == capacity) {
			resize();
		}

		platforms[size++] = new Platform(screen, sprite, block_x, block_y, width, ladder_block, ladder_sprite, ladder_height);
	}

	// Access the platform at the given index
	Platform* getPlatform(int index) {
		if (index >= 0 && index < size) {
			return platforms[index];
		}
		return nullptr;  // Return nullptr for out-of-bounds access
	}

	// Remove all platforms from the array
	void clear() {
		for (int i = 0; i < size; ++i) {
			delete platforms[i];
		}
		delete[] platforms;
		platforms = nullptr;
		size = 0;
		capacity = 0;
	}

private:
	// Resize the array when full
	void resize() {
		int newCapacity = (capacity == 0) ? 1 : capacity * 2;
		Platform** newPlatforms = new Platform * [newCapacity];

		for (int i = 0; i < size; ++i) {
			// gdy newPlatforms ma jeden element, to obecny size jest rowny 0, wiec petla nie wykona sie
			newPlatforms[i] = platforms[i];
		}

		delete[] platforms;
		platforms = newPlatforms;
		capacity = newCapacity;
	}
};

//klasa przechowujaca dynamiczna tablice beczek
 class Dynamic_Barrel_Array {
private:
	Barrel** barrels;  // Dynamic array of pointers to Platform objects
	int capacity;

public:

	int size;
	Dynamic_Barrel_Array() : barrels(nullptr), capacity(0), size(0) {}

	~Dynamic_Barrel_Array() {
		clear();
	}

	void free_one(int index) {
		if (index >= 0 && index < size) {
			delete barrels[index];
			// Shift remaining elements to fill the gap
			for (int i = index; i < size - 1; ++i) {
				barrels[i] = barrels[i + 1];
			}
			size--;
		}
	}

	// Add a platform to the array
	void addBarrel(SDL_Surface* screen, SDL_Surface* sprite) {
		if (size == capacity) {
			resize();
		}

		barrels[size++] = new Barrel(screen, sprite, 30, 150);
	}

	// Access the platform at the given index
	Barrel* getBarrel(int index) {
		if (index >= 0 && index < size) {
			return barrels[index];
		}
		return nullptr;  // Return nullptr for out-of-bounds access
	}

	// Remove all platforms from the array
	void clear() {
		for (int i = 0; i < size; ++i) {
			
			delete barrels[i];
		}
		delete[] barrels;
		barrels = nullptr;
		size = 0;
		capacity = 0;
	}

private:
	// Resize the array when full
	void resize() {
		int newCapacity = (capacity == 0) ? 1 : capacity * 2; // if capacity == 0, newCapacity = 1, else newCapacity = capacity * 2
		Barrel** newBarrels = new Barrel * [newCapacity];
		// gdy newBarrels ma jeden element, to obecny size jest rowny 0, wiec petla nie wykona sie
		for (int i = 0; i < size; ++i) {
			newBarrels[i] = barrels[i];
		}

		delete[] barrels;
		barrels = newBarrels;
		capacity = newCapacity;
	}
};



class Game {
private:
	int score;
	int lives;
public:
	int current_level;
	Game() {
		score = 0;
		lives = 3;
		current_level = 1;
	}

	//wywolanie poziomu 1
	void level1(SDL_Surface* screen, SDL_Surface* platform1, SDL_Surface* ladder_sprite, Dynamic_Platform_Array* platforms, Player* player, Dynamic_Barrel_Array* barrels) {
		this->current_level = 1;
		if (platforms->size > 0) {
			platforms->clear();
		}
		if (barrels->size > 0) {
			barrels->clear();
		}
		player->restart();
		#pragma region adding platforms
		platforms->addPlatform(screen, platform1, SCREEN_WIDTH_BLOCKS - 3.0/2, SCREEN_HEIGHT_BLOCKS+1.0/2, 100, 2, ladder_sprite, 4); //starting ladder
		platforms->addPlatform(screen, platform1, SCREEN_WIDTH_BLOCKS - 3, SCREEN_HEIGHT_BLOCKS-3 - 1.0/2, 120, 5, ladder_sprite, 4); // 1st floor right platform with ladder
		platforms->addPlatform(screen, platform1, 9, SCREEN_HEIGHT_BLOCKS -  3 - 1.0 / 2, 400, 10, ladder_sprite, 4); // 1st floor left platform
		platforms->addPlatform(screen, platform1, SCREEN_WIDTH_BLOCKS - 5, SCREEN_HEIGHT_BLOCKS - 7 - 1.0/2, SQUARE_SIZE * 17, 1, ladder_sprite, 0); // 2nd floor right platform
		platforms->addPlatform(screen, platform1, 5, SCREEN_HEIGHT_BLOCKS - 7 - 1.0 / 2, 10 * SQUARE_SIZE, 3, ladder_sprite, 4); // 2nd floor left platform with ladder
		platforms->addPlatform(screen, platform1, 14, SCREEN_HEIGHT_BLOCKS - 7 - 1.0/2, SQUARE_SIZE, 1, ladder_sprite, 0); // 2nd floor middle floating platform
		platforms->addPlatform(screen, platform1, 10, SCREEN_HEIGHT_BLOCKS - 11 - 1.0 / 2, 16 * SQUARE_SIZE, 1, ladder_sprite, 0); // 3rd floor left platform
		platforms->addPlatform(screen, platform1, SCREEN_WIDTH_BLOCKS - 5, SCREEN_HEIGHT_BLOCKS - 11 - 1.0 / 2, 10 * SQUARE_SIZE, 9, ladder_sprite, 4); // 3rd floor right platform)
		platforms->addPlatform(screen, platform1, SCREEN_WIDTH_BLOCKS - 8, SCREEN_HEIGHT_BLOCKS - 15 - 1.0/2, 14 * SQUARE_SIZE, 1, ladder_sprite, 0); // 4th floor right platform
		platforms->addPlatform(screen, platform1, 5, SCREEN_HEIGHT_BLOCKS - 15 - 1.0 / 2, 10 * SQUARE_SIZE, 5, ladder_sprite, 4); // 4th floor left platform
		platforms->addPlatform(screen, platform1, 3.5, SCREEN_HEIGHT_BLOCKS - 19 - 1.0 / 2, 3 * SQUARE_SIZE, 1, ladder_sprite, 0); // final platform
	#pragma endregion
	}
	//wywolanie poziomu 2
	void level2(SDL_Surface* screen, SDL_Surface* platform1, SDL_Surface* ladder_sprite, Dynamic_Platform_Array* platforms, Player* player, Dynamic_Barrel_Array* barrels) {
		this->current_level = 2;
		if (platforms->size > 0) {
			platforms->clear();
		}
		if(barrels->size > 0) {
			barrels->clear();
		}
		player->restart();

		#pragma region adding platforms
		platforms->addPlatform(screen, platform1, SCREEN_WIDTH_BLOCKS - 3.0 / 2, SCREEN_HEIGHT_BLOCKS + 1.0 / 2, 100, 2, ladder_sprite, 4); // starting ladder
		platforms->addPlatform(screen, platform1, SCREEN_WIDTH_BLOCKS - 3, SCREEN_HEIGHT_BLOCKS - 3 - 1.0 / 2, 120, 5, ladder_sprite, 4); // 1st floor right platform with ladder
		platforms->addPlatform(screen, platform1, SCREEN_WIDTH_BLOCKS - 10.5, SCREEN_HEIGHT_BLOCKS - 7 - 1.0 / 2, SQUARE_SIZE * 23, 1, ladder_sprite, 0); // 2nd floor right platform
		platforms->addPlatform(screen, platform1, 2, SCREEN_HEIGHT_BLOCKS - 7 - 1.0 / 2, 4 * SQUARE_SIZE, 3, ladder_sprite, 4); // 2nd floor left platform with ladder)
		platforms->addPlatform(screen, platform1, 10, SCREEN_HEIGHT_BLOCKS - 11 - 1.0 / 2, 20 * SQUARE_SIZE, 1, ladder_sprite, 0); // 3rd floor left platform)
		platforms->addPlatform(screen, platform1, SCREEN_WIDTH_BLOCKS - 3, SCREEN_HEIGHT_BLOCKS - 11 - 1.0 / 2, 10 * SQUARE_SIZE, 3, ladder_sprite, 4); // 3rd floor right platform)
		platforms->addPlatform(screen, platform1, 3, SCREEN_HEIGHT_BLOCKS - 15 - 1.0 / 2, 6 * SQUARE_SIZE, 5, ladder_sprite, 4); // 4th floor left platform
		platforms->addPlatform(screen, platform1, SCREEN_WIDTH_BLOCKS - 12, SCREEN_HEIGHT_BLOCKS - 15 - 1.0 / 2, 18 * SQUARE_SIZE, 5, ladder_sprite, 0); // 4th floor right platform
		platforms->addPlatform(screen, platform1, 3.5, SCREEN_HEIGHT_BLOCKS - 19 - 1.0 / 2, 3 * SQUARE_SIZE, 1, ladder_sprite, 0); // final platform
#pragma endregion
	}

	//wywolanie poziomu 3
	void level3(SDL_Surface* screen, SDL_Surface* platform1, SDL_Surface* ladder_sprite, Dynamic_Platform_Array* platforms, Player* player, Dynamic_Barrel_Array* barrels) {
		this->current_level = 3;
		if (platforms->size > 0) {
			platforms->clear();
		}
		if(barrels->size > 0) {
			barrels->clear();
		}
		player->restart();

		#pragma region adding platforms
		platforms->addPlatform(screen, platform1, 2.5, SCREEN_HEIGHT_BLOCKS+1.0/2, 20, 1, ladder_sprite, 4); //starting ladder
		platforms->addPlatform(screen, platform1, SCREEN_WIDTH_BLOCKS - 16.5, SCREEN_HEIGHT_BLOCKS-1, 3 * SQUARE_SIZE, 1, ladder_sprite, 0);
		platforms->addPlatform(screen, platform1, SCREEN_WIDTH_BLOCKS - 12, SCREEN_HEIGHT_BLOCKS-1.0/2 - 1.75, 3 * SQUARE_SIZE, 1, ladder_sprite, 0);
		platforms->addPlatform(screen, platform1, 4.5, SCREEN_HEIGHT_BLOCKS - 1.0 / 2 - 3, 9 * SQUARE_SIZE, 3, ladder_sprite, 4); // 1st floor left platform with ladder
		platforms->addPlatform(screen, platform1, SCREEN_WIDTH_BLOCKS - 4.5, SCREEN_HEIGHT_BLOCKS - 1.0 / 2 - 3, 9 * SQUARE_SIZE, 4, ladder_sprite, 4); // 1st floor right platform with ladder
		platforms->addPlatform(screen, platform1, 8.5, SCREEN_HEIGHT_BLOCKS - 1.0 / 2 - 7, 19 * SQUARE_SIZE, 3, ladder_sprite, 0); // 2nd floor left platform
		platforms->addPlatform(screen, platform1, SCREEN_WIDTH_BLOCKS - 3.5, SCREEN_HEIGHT_BLOCKS - 1.0 / 2 - 7, 5 * SQUARE_SIZE, 4, ladder_sprite, 4); // 2nd floor right platform with ladder
		platforms->addPlatform(screen, platform1, 14.5, SCREEN_HEIGHT_BLOCKS - 1.0 / 2 - 11, 5 * SQUARE_SIZE, 4, ladder_sprite, 4); // 3rd floor middle platform
        platforms->addPlatform(screen, platform1, SCREEN_WIDTH_BLOCKS - 5.5, SCREEN_HEIGHT_BLOCKS - 1.0 / 2 - 11, 10 * SQUARE_SIZE, 4, ladder_sprite, 0); // 3rd floor right platform
		platforms->addPlatform(screen, platform1, 4.5, SCREEN_HEIGHT_BLOCKS - 1.0 / 2 - 11, 5 * SQUARE_SIZE, 2, ladder_sprite, 0); // 3rd floor left platform with ladder
		platforms->addPlatform(screen, platform1, 8, SCREEN_HEIGHT_BLOCKS - 1.0 / 2 - 15, 16 * SQUARE_SIZE, 5, ladder_sprite, 4); // 4th floor left platform)
		platforms->addPlatform(screen, platform1, SCREEN_WIDTH_BLOCKS - 2.5, SCREEN_HEIGHT_BLOCKS - 1.0 / 2 - 15, 5 * SQUARE_SIZE, 1, ladder_sprite, 0); // 4th floor right platform
		platforms->addPlatform(screen, platform1, 3.5, SCREEN_HEIGHT_BLOCKS - 19 - 1.0 / 2, 3 * SQUARE_SIZE, 1, ladder_sprite, 0); // final platform
#pragma endregion
	}
	void new_game(SDL_Surface* screen, SDL_Surface* platform1, SDL_Surface* ladder_sprite, Dynamic_Platform_Array* platforms, Player* player, Dynamic_Barrel_Array* barrels) {
		if (current_level == 1) {
			level1(screen, platform1, ladder_sprite, platforms, player, barrels);
		}
		else if (current_level == 2) {
			level2(screen, platform1, ladder_sprite, platforms, player, barrels);
		}
		else if (current_level == 3) {
			level3(screen, platform1, ladder_sprite, platforms, player, barrels);
		}
		else {
			//Screen after ending level 3
			while (SDL_PollEvent) {
				SDL_Event event;
				SDL_PollEvent(&event);
				if (event.type == SDL_KEYDOWN) {
					if (event.key.keysym.sym == SDLK_ESCAPE) {
						exit(0);
					}
					else if (event.key.keysym.sym == SDLK_n) {
						level1(screen, platform1, ladder_sprite, platforms, player, barrels);
					}
				}
			}
		}
	}
	};

#endif