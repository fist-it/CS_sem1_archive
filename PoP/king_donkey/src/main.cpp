#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>
#include "game_types.h"
#include "support_functions.h"




int in_corner(Barrel* barrel) {
	if (barrel->y + barrel->height / 2 == SCREEN_HEIGHT) {
		if (barrel->x - barrel->width / 2  -2 <= 0) {
			return 1;
		}
		if( barrel->x + barrel->width / 2  +2 >= SCREEN_WIDTH) {
			return 1;
		}
	}
	return 0;
}

//checks all platform, barrel and player collisions, updates player and barrels position, draws everything
void check_collisions_and_update(SDL_Surface* screen, Game* game, Player* player, Dynamic_Barrel_Array* barrels, Dynamic_Platform_Array* platforms, double* timer_1, double delta, SDL_Surface* ladder_bmp, SDL_Surface* platform1, SDL_Surface* barrel_bmp, int* onladder, const int* cheat_ladder, int* platformcol) {
	//adding barrels once every 2 seconds
	if (clock(timer_1, delta, 2)) {
		barrels->addBarrel(screen, barrel_bmp);
	}
	//check player collisions with platforms
	for (int i = 0; i < platforms->size; i++) {
		if (player->checkplatformcollision(platforms->getPlatform(i), *onladder)) {
			*platformcol = 1;
			break;
		}
		else {
			*platformcol = 0;
		}
	};
	//checking platform collisions and freeing barrels in corners of the screen
	for (int i = 0; i < platforms->size; i++) {
		for (int j = 0; j < barrels->size; j++) {
			Barrel* barrel = barrels->getBarrel(j);
			barrel->checkplatformcollision(platforms->getPlatform(i), 0);
			if(in_corner(barrel)) {
				barrels->free_one(j);
			}
		}
	};
	for (int i = 0; i < platforms->size; i++) {
		if (player->on_ladder(platforms->getPlatform(i))) {
			*onladder = 1;
			break;
		}
		else {
			*onladder = 0;
		}
	}
	for (int i = 0; i < barrels->size; i++) {
		if (player->check_barrel_collision(barrels->getBarrel(i))) {
			player->barrelcol = 1;
			game->new_game(screen, platform1, ladder_bmp, platforms, player, barrels);
			break;
		}
		else {
			player->barrelcol = 0;
		}
	}
	player->update(delta, *platformcol, *onladder, *cheat_ladder);

#pragma region drawing everything
	for (int i = 0; i < platforms->size; i++) {
		platforms->getPlatform(i)->draw(screen);

	}
	for (int i = 0; i < platforms->size; i++) {
		platforms->getPlatform(i)->ladder.draw(screen);
	}
	player->draw(screen);
	for (int i = 0; i < barrels->size; i++) {
		barrels->getBarrel(i)->update(delta, screen);
	}
#pragma endregion

}


//Draws an info bar on the top of the screen, based on PG ETI 2023/2024 second project template
void draw_info_bar(SDL_Surface* screen, const double* worldTime, const double* fps, SDL_Surface* charset, int czerwony, int niebieski) {
	//kolory nie sa wykorzystywane nigdzie poza ta funkcja
	char text[128];

	DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 54, czerwony, niebieski);
	sprintf(text, "autor: fist_it, czas trwania = %.1lf s  %.0lf klatek / s", *worldTime, *fps);
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
	sprintf(text, "Esc - wyjscie, \032\030\031\033 - poruszanie sie, spacja - skok");
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);
	sprintf(text, "Restart gry - n, wykonane zadania: A, B, C");
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 42, text, charset);
}

void game_loop(SDL_Event event, int* quit, int* jump, int* onladder, int* left, int* right, int* down, int* cheat_ladder, Game* game, SDL_Surface* screen, SDL_Surface* platform1, SDL_Surface* ladder_bmp, Dynamic_Platform_Array* platforms, Player* player, Dynamic_Barrel_Array* barrels) {
		switch (event.type) {
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
			case SDLK_ESCAPE:
				*quit = 1;
				break;
			case SDLK_UP:
				*jump = 1;
				break;
			case SDLK_SPACE:
				*jump = 1;
				break;
			case SDLK_DOWN:
				if (*onladder) {
					*down = 1;
				}
				break;
			case SDLK_LEFT:
				*left = 1;
				break;
			case SDLK_RIGHT:
				*right = 1;
				break;
			case SDLK_g:
				(*cheat_ladder)++;
				*cheat_ladder %= 2;
				break;
			};
			break;
		case SDL_KEYUP:
			switch (event.key.keysym.sym) {
			case SDLK_UP:
				*jump = 0;
				break;
			case SDLK_SPACE:
				*jump = 0;
				break;
			case SDLK_DOWN:
				*down = 0;
				break;
			case SDLK_LEFT:
				*left = 0;
				break;
			case SDLK_RIGHT:
				*right = 0;
				break;

			case SDLK_n:
				game->new_game(screen, platform1, ladder_bmp, platforms, player, barrels);
				break;
			case SDLK_1:
				game->level1(screen, platform1, ladder_bmp, platforms, player, barrels);
				break;
			case SDLK_2:
				game->level2(screen, platform1, ladder_bmp, platforms, player, barrels);
				break;
			case SDLK_3:
				game->level3(screen, platform1, ladder_bmp, platforms, player, barrels);
				break;
			};
			break;
		case SDL_QUIT:
			*quit = 1;
			break;
		};
}

// main
#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char **argv) {

#pragma region inicjalizacja zmiennych, inicjalizacja SDL, wczytanie bitmap
	//deklaracje
	int t1, t2, quit, frames, rc;
	double delta, worldTime, fpsTimer, speed, fps;
	SDL_Event event;
	SDL_Surface *screen, *charset;
	SDL_Surface *wario1, *wario2, *barrel_bmp, *platform1, *ladder_bmp;
	SDL_Texture *scrtex;
	SDL_Window *window;
	SDL_Renderer *renderer;


	initialize_SDL(&rc, &window, &renderer, &screen, &scrtex, &charset);

	//wczytanie obrazka Wario
	wario1 = load_bitmap("./bmp/wario16x20.bmp", screen, scrtex, window, renderer);
	wario2 = load_bitmap("./bmp/wario16x202.bmp", screen, scrtex, window, renderer);

	//wczytanie platformy
	platform1 = load_bitmap("./bmp/platform1.bmp", screen, scrtex, window, renderer);
	
	//wczytywanie obrazu beczki i drabiny
	barrel_bmp = load_bitmap("./bmp/barrel.bmp", screen, scrtex, window, renderer);
	ladder_bmp = load_bitmap("./bmp/ladder.bmp", screen, scrtex, window, renderer);

	//wczytanie title
	SDL_Surface* title = load_bitmap("./bmp/title.bmp", screen, scrtex, window, renderer);

	//kolory, zrodlo: PG ETI 2023/2024 second project template
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);
	int pomaranczowy = SDL_MapRGB(screen->format, 0xFF, 0x66, 0x00);


	t1 = SDL_GetTicks();

#pragma region inicjalizajca zmiennych z ustawieniem wartosci poczatkowych
	frames = 0;
	fpsTimer = 0;
	fps = 0;
	quit = 0;
	worldTime = 0;
	speed = 1.0;
	Game game;
	Player player(screen, wario1, wario2, 100, 100);
	Dynamic_Platform_Array platforms;
	Dynamic_Barrel_Array barrels;
	int right = 0;
	int left = 0;
	int jump = 0;
	int down = 0;
	int onladder = 0;
	int platformcol = 0;
	int cheat_ladder = 0;
	double timer_1 = 1.5;
#pragma endregion


#pragma endregion

	game.level1(screen, platform1, ladder_bmp, &platforms, &player, &barrels);
	while(!quit) {
		
#pragma region translacja tickow na czas delta, zrodlo PG ETI 2023/2024 second project template
		t2 = SDL_GetTicks();

		// w tym momencie t2-t1 to czas w milisekundach,
		// jaki uplynal od ostatniego narysowania ekranu
		// delta to ten sam czas w sekundach
		delta = (t2 - t1) * 0.001;
		t1 = t2;

		worldTime += delta;
		SDL_FillRect(screen, NULL, czarny);
		
		fpsTimer += delta;
		if(fpsTimer > 0.5) {
			fps = frames * 2;
			frames = 0;
			fpsTimer -= 0.5;
			};
#pragma endregion
		
		check_collisions_and_update(screen, &game, &player, &barrels, &platforms, &timer_1, delta, ladder_bmp, platform1, barrel_bmp, &onladder, &cheat_ladder, &platformcol);
		//infobar
		draw_info_bar(screen, &worldTime, &fps, charset, czerwony, pomaranczowy);
		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		//SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);
		while (SDL_PollEvent(&event)) {
			game_loop(event, &quit, &jump, &onladder, &left, &right, &down, &cheat_ladder, &game, screen, platform1, ladder_bmp, &platforms, &player, &barrels);
		}
		frames++;
		if (right) {
			player.right(delta);
			player.direction = 1;
		}
		if (left) {
			player.left(delta);
			player.direction = 0;
		}
		if (jump) {
			player.jump(delta, platformcol, onladder, cheat_ladder);
		}
		if (down) {
			player.down(delta);
		}
		if (player.did_win()) {
			print_winning_screen(screen, title, charset, wario1, scrtex, window, renderer, &worldTime, &timer_1);
			game.current_level++;
			game.new_game(screen, platform1, ladder_bmp, &platforms, &player, &barrels);
		}
	};
	// zwolnienie powierzchni / freeing all surfaces
	SDL_FreeSurface(charset);
	SDL_FreeSurface(barrel_bmp);
	SDL_FreeSurface(platform1);
	return proper_quit_SDL(screen, scrtex, window, renderer);
};
