#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>

#define SCREEN_WIDTH  1920
#define SCREEN_HEIGHT 1080

// Structure for upgrade buttons
typedef struct {
	SDL_Rect rect;
	const char *name;
	int level;
	long long cost;
	double cost_multiplier;
} Upgrade;

// Helper function to render text
void render_text(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y, SDL_Color color)
{
	SDL_Surface *surface = TTF_RenderText_Solid(font, text, color);
	if (!surface) {
		SDL_Log("Unable to create text surface: %s", TTF_GetError());
		return;
	}
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
	if (!texture) {
		SDL_Log("Unable to create text texture: %s", TTF_GetError());
		SDL_FreeSurface(surface);
		return;
	}

	SDL_Rect dest = { x, y, surface->w, surface->h };
	SDL_RenderCopy(renderer, texture, NULL, &dest);

	SDL_FreeSurface(surface);
	SDL_DestroyTexture(texture);
}

int main(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	SDL_Init(SDL_INIT_VIDEO);
	TTF_Init();

	SDL_Window *window	   = SDL_CreateWindow("Clicker Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
											  SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	TTF_Font *font_large = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 28);
	TTF_Font *font_small = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 14);

	// Game state
	long long money			= 1000;
	int money_per_click		= 1;
	int money_per_second	= 0;
	Uint32 last_second_tick = SDL_GetTicks();

	// Colors
	SDL_Color color_white			  = { 255, 255, 255, 255 };
	SDL_Color color_black			  = { 0, 0, 0, 255 };
	SDL_Color color_grey			  = { 50, 50, 50, 255 };
	SDL_Color color_grey_hover		  = { 80, 80, 80, 255 };
	SDL_Color color_border			  = { 20, 20, 20, 255 };
	SDL_Color color_cost_affordable	  = { 100, 255, 100, 255 }; // Light Green
	SDL_Color color_cost_unaffordable = { 255, 100, 100, 255 }; // Light Red

	// Initialize Upgrades
	Upgrade upgrades[4];
	const char *names[] = { "Q", "W", "E", "R" };
	for (int i = 0; i < 4; i++) {
		upgrades[i].name   = names[i];
		upgrades[i].level  = 0;
		upgrades[i].rect.w = 150;
		upgrades[i].rect.h = 60;
		upgrades[i].rect.x = SCREEN_WIDTH - (4 - i) * (upgrades[i].rect.w + 10) - 10;
		upgrades[i].rect.y = SCREEN_HEIGHT - upgrades[i].rect.h - 10;
	}

	upgrades[0].cost			= 10;
	upgrades[0].cost_multiplier = 1.5;

	upgrades[1].cost			= 25;
	upgrades[1].cost_multiplier = 1.6;

	// E and R are disabled for now with a high cost
	upgrades[2].cost			= 999999999;
	upgrades[2].cost_multiplier = 2.0;
	upgrades[3].cost			= 999999999;
	upgrades[3].cost_multiplier = 2.0;

	int running = 1;
	while (running) {
		// EVENT HANDLING
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				running = 0;
			}
			if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.sym == SDLK_ESCAPE) {
					running = 0;
				}
				if (event.key.keysym.sym == SDLK_a) {
					money += money_per_click;
				}
			}
			if (event.type == SDL_MOUSEBUTTONDOWN) {
				if (event.button.button == SDL_BUTTON_LEFT) {
					int mouse_x			  = event.button.x;
					int mouse_y			  = event.button.y;
					SDL_Point mouse_point = { mouse_x, mouse_y };

					for (int i = 0; i < 4; i++) {
						if (SDL_PointInRect(&mouse_point, &upgrades[i].rect)) {
							if (money >= upgrades[i].cost) {
								money -= upgrades[i].cost;
								upgrades[i].level++;
								upgrades[i].cost *= upgrades[i].cost_multiplier;

								if (i == 0) { // Q - Click Power
									money_per_click = 1 + upgrades[i].level * 2;
								} else if (i == 1) { // W - Passive Income
									money_per_second = upgrades[i].level * 1;
								}
							}
						}
					}
				}
			}
		}

		// GAME LOGIC
		if (SDL_GetTicks() - last_second_tick >= 1000) {
			money += money_per_second;
			last_second_tick = SDL_GetTicks();
		}

		// RENDERING
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderClear(renderer);

		// Render top-left info
		char buffer[64];
		snprintf(buffer, sizeof(buffer), "Money: $%lld", money);
		render_text(renderer, font_large, buffer, 10, 10, color_black);

		snprintf(buffer, sizeof(buffer), "Click Power: $%d", money_per_click);
		render_text(renderer, font_small, buffer, 10, 50, color_black);

		snprintf(buffer, sizeof(buffer), "$/sec: %d", money_per_second);
		render_text(renderer, font_small, buffer, 10, 70, color_black);

		// Get mouse state for hover effect
		int mouse_x, mouse_y;
		SDL_GetMouseState(&mouse_x, &mouse_y);
		SDL_Point mouse_point = { mouse_x, mouse_y };

		// Render upgrade buttons
		for (int i = 0; i < 4; i++) {
			SDL_Color current_bg_color = color_grey;
			// Check for hover
			if (SDL_PointInRect(&mouse_point, &upgrades[i].rect)) {
				current_bg_color = color_grey_hover;
			}

			// Draw button border
			SDL_SetRenderDrawColor(renderer, color_border.r, color_border.g, color_border.b, color_border.a);
			SDL_RenderFillRect(renderer, &upgrades[i].rect);

			// Draw button background
			SDL_Rect bg_rect = { upgrades[i].rect.x + 2, upgrades[i].rect.y + 2, upgrades[i].rect.w - 4,
								 upgrades[i].rect.h - 4 };
			SDL_SetRenderDrawColor(renderer, current_bg_color.r, current_bg_color.g, current_bg_color.b,
								   current_bg_color.a);
			SDL_RenderFillRect(renderer, &bg_rect);

			// Draw main name (Q, W, E, R)
			render_text(renderer, font_large, upgrades[i].name, upgrades[i].rect.x + 20,
						upgrades[i].rect.y + (upgrades[i].rect.h / 2) - 16, color_white);

			// Draw Level
			snprintf(buffer, sizeof(buffer), "Lv.%d", upgrades[i].level);
			render_text(renderer, font_small, buffer, upgrades[i].rect.x + 80, upgrades[i].rect.y + 12, color_white);

			// Draw Cost
			SDL_Color cost_color = (money >= upgrades[i].cost) ? color_cost_affordable : color_cost_unaffordable;
			snprintf(buffer, sizeof(buffer), "$%lld", upgrades[i].cost);
			render_text(renderer, font_small, buffer, upgrades[i].rect.x + 80, upgrades[i].rect.y + 32, cost_color);
		}

		SDL_RenderPresent(renderer);
		SDL_Delay(16); // Cap frame rate
	}

	// CLEANUP
	TTF_CloseFont(font_large);
	TTF_CloseFont(font_small);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	TTF_Quit();
	SDL_Quit();

	return 0;
}
