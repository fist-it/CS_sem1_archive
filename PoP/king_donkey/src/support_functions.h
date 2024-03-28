#ifndef SUPPORT_H
#define SUPPORT_H



extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}



#define SQUARE_SIZE		20 // rozmiar boku jednego bloku, z ktorych zbudowana jest mapa
#define WINNING_Y 60 // granica Y finalnej platformy
#define WINNING_X 4*SQUARE_SIZE // granica X finalnej platformy
#define SCREEN_WIDTH	640 //szerokosc ekranu w pikselach
#define SCREEN_HEIGHT	480 //wysokosc ekranu w pikselach
#define SCREEN_WIDTH_BLOCKS		SCREEN_WIDTH/SQUARE_SIZE // szerokosc ekranu w blokach
#define SCREEN_HEIGHT_BLOCKS	SCREEN_HEIGHT/SQUARE_SIZE // wysokosc ekranu w blokach

#pragma region SDL2 QOL functions from PG ETI 2023/2024 second project template
//wypisanie napisu na ekranie zaczynajac od punktu (x,y),
//charset to bitmapa 128x128 zawierajaca znaki
void DrawString(SDL_Surface* screen, int x, int y, const char* text,
	SDL_Surface* charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while (*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
	};
};


//rysowanie powierzchni z bitmapy
void DrawSurface(SDL_Surface* screen, SDL_Surface* sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
};

// rysowanie pojedynczego pixela
void DrawPixel(SDL_Surface* surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32*)p = color;
};


// rysowanie linii o dlugosci l w pionie (gdy dx = 0, dy = 1) 
// badz w poziomie (gdy dx = 1, dy = 0)
void DrawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for (int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
	};
};


// rysowanie prostokata o dlugosci bokow l i k
void DrawRectangle(SDL_Surface* screen, int x, int y, int l, int k,
	Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for (i = y + 1; i < y + k - 1; i++) {
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
	}
};

#pragma endregion


//funkcja wczytujaca bitmapy z plikow, przyjmuje path i obiekty SDL w razie porazki
SDL_Surface* load_bitmap(char* path, SDL_Surface* screen, SDL_Texture* scrtex, SDL_Window* window, SDL_Renderer* renderer) {
	SDL_Surface* bitmap = SDL_LoadBMP(path);
	if (bitmap == NULL) {
		printf("SDL_LoadBMP(%s) error: %s\n", path, SDL_GetError());
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		return NULL;
	}
	SDL_SetColorKey(bitmap, true, 0x000000);
	return bitmap;
}

//funkcja rysujaca ekran wygranej
void print_winning_screen(SDL_Surface* screen, SDL_Surface* title, SDL_Surface* charset, SDL_Surface* winning_screen, SDL_Texture* scrtex, SDL_Window* window, SDL_Renderer* renderer, double* worldtime, double* timer_1) {
	SDL_FillRect(screen, NULL, 0x000000);
	DrawSurface(screen, winning_screen, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
	char text[128];
	sprintf(text, "You Won in %.0f seconds!", *worldtime);
	DrawString(screen, WINNING_X, WINNING_Y, text, charset);
	DrawSurface(screen, title, SCREEN_WIDTH/2, SCREEN_HEIGHT / 2);
	SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, scrtex, NULL, NULL);
	SDL_RenderPresent(renderer);
	SDL_Delay(3000);
	*worldtime = -3.0;
	*timer_1 = -3.0;
}

//zegar dla timera timer, n to liczba sekund do odliczenia
//nalezy uzywac INNYCH zmiennych do odmierzania roznych przedzialow czasu
int clock(double* timer, double delta, double n) {
	*timer += delta;
	if (*timer >= n) {
		*timer -= n;
		return 1;
	}
	return 0;
}

//funkcja inicjalizujaca SDL, przyjmuje wskazniki na zmienne, ktore chcemy zainicjalizowac
void initialize_SDL(int* rc, SDL_Window** window, SDL_Renderer** renderer, SDL_Surface** screen, SDL_Texture** scrtex, SDL_Surface** charset) {
	// inicjalizacja SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		exit(1);
	};

//	*rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
	//	window, renderer);

	// tryb pelnoekranowy
	*rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP, window, renderer);


	if (*rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		exit(1);
	};

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(*renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(*renderer, 0, 0, 0, 255);

	SDL_SetWindowTitle(*window, "King Donkey, Franciszek Fabinski");


	*screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
		0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	*scrtex = SDL_CreateTexture(*renderer, SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		SCREEN_WIDTH, SCREEN_HEIGHT);
	// wczytanie obrazka cs8x8.bmp
	*charset = load_bitmap("./bmp/cs8x8.bmp", *screen, *scrtex, *window, *renderer);
}

//funkcja niszczaca obiekty SDL
int proper_quit_SDL(SDL_Surface* screen, SDL_Texture* scrtex, SDL_Window* window, SDL_Renderer* renderer) {
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}

#endif
