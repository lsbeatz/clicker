#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SCREEN_WIDTH  1920
#define SCREEN_HEIGHT 1080

// Structure for upgrade buttons
typedef struct {
	SDL_Rect rect;
	const char *name;
	int level;
	long long cost;
	double cost_multiplier;
	Uint32 last_key_press_time;
} Upgrade;

// Structure for dynamic text rendering
typedef struct {
	SDL_Texture *texture;
	SDL_Rect rect;
	char *last_text;
	SDL_Color last_color;
} TextRenderer;

// Global Game State
long long g_money;
int g_money_per_click;
int g_money_per_second;
Uint32 g_last_second_tick;
Upgrade g_upgrades[4];

// Global Text Renderers
TextRenderer g_money_tr;
TextRenderer g_click_power_tr;
TextRenderer g_money_per_second_tr;
TextRenderer g_upgrade_level_tr[4];
TextRenderer g_upgrade_cost_tr[4];

// Global Colors
SDL_Color g_color_white						= { 255, 255, 255, 255 };
SDL_Color g_color_black						= { 0, 0, 0, 255 };
SDL_Color g_color_grey						= { 50, 50, 50, 255 };
SDL_Color g_color_border					= { 20, 20, 20, 255 };
SDL_Color g_color_cost_affordable			= { 100, 255, 100, 255 }; // Light Green
SDL_Color g_color_cost_unaffordable			= { 255, 100, 100, 255 }; // Light Red
SDL_Color g_color_border_affordable_hover	= { 0, 200, 0, 255 }; // Greenish
SDL_Color g_color_border_unaffordable_hover = { 200, 0, 0, 255 }; // Reddish

// Function to initialize TextRenderer
void init_text_renderer(TextRenderer *tr)
{
	tr->texture	   = NULL;
	tr->last_text  = NULL;
	tr->last_color = (SDL_Color){ 0, 0, 0, 0 }; // Initialize with a default color
}

// Function to update and render dynamic text
void update_and_render_text(SDL_Renderer *renderer,
							TTF_Font *font,
							TextRenderer *tr,
							const char *new_text,
							int x,
							int y,
							SDL_Color color)
{
	if (!tr->last_text || strcmp(tr->last_text, new_text) != 0 || tr->last_color.r != color.r ||
		tr->last_color.g != color.g || tr->last_color.b != color.b || tr->last_color.a != color.a) {
		// Text or color has changed, destroy old texture and create new one
		if (tr->texture) {
			SDL_DestroyTexture(tr->texture);
			tr->texture = NULL;
		}
		if (tr->last_text) {
			free(tr->last_text);
			tr->last_text = NULL;
		}

		SDL_Surface *surface = TTF_RenderText_Solid(font, new_text, color);
		if (!surface) {
			SDL_Log("Unable to create text surface: %s", TTF_GetError());
			return;
		}
		tr->texture = SDL_CreateTextureFromSurface(renderer, surface);
		if (!tr->texture) {
			SDL_Log("Unable to create text texture: %s", TTF_GetError());
			SDL_FreeSurface(surface);
			return;
		}

		tr->rect.x = x;
		tr->rect.y = y;
		tr->rect.w = surface->w;
		tr->rect.h = surface->h;

		tr->last_text  = strdup(new_text);
		tr->last_color = color; // Store the new color
		SDL_FreeSurface(surface);
	}

	// Render the texture
	SDL_RenderCopy(renderer, tr->texture, NULL, &tr->rect);
}

// Function to destroy TextRenderer resources
void destroy_text_renderer(TextRenderer *tr)
{
	if (tr->texture) {
		SDL_DestroyTexture(tr->texture);
		tr->texture = NULL;
	}
	if (tr->last_text) {
		free(tr->last_text);
		tr->last_text = NULL;
	}
}

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

// Function Prototypes for refactoring
void handle_keyboard_event(SDL_Event *event);
void handle_mouse_button_down_event(SDL_Event *event);
void process_events();
void update_game_state();
void render_top_info(SDL_Renderer *renderer, TTF_Font *font_large, TTF_Font *font_small);
void render_upgrade_buttons(SDL_Renderer *renderer, TTF_Font *font_large, TTF_Font *font_small, SDL_Point mouse_point);
void render_game(SDL_Renderer *renderer, TTF_Font *font_large, TTF_Font *font_small, SDL_Point mouse_point);

// Function to handle keyboard events
void handle_keyboard_event(SDL_Event *event)
{
	if (event->key.keysym.sym == SDLK_a) {
		g_money += g_money_per_click;
	} else if (event->key.keysym.sym == SDLK_q) {
		// Simulate click for Q upgrade
		if (g_money >= g_upgrades[0].cost) {
			g_money -= g_upgrades[0].cost;
			g_upgrades[0].level++;
			g_upgrades[0].cost *= g_upgrades[0].cost_multiplier;
			g_money_per_click				  = 1 + g_upgrades[0].level * 2;
			g_upgrades[0].last_key_press_time = SDL_GetTicks();
		}
	} else if (event->key.keysym.sym == SDLK_w) {
		// Simulate click for W upgrade
		if (g_money >= g_upgrades[1].cost) {
			g_money -= g_upgrades[1].cost;
			g_upgrades[1].level++;
			g_upgrades[1].cost *= g_upgrades[1].cost_multiplier;
			g_money_per_second				  = g_upgrades[1].level * 1;
			g_upgrades[1].last_key_press_time = SDL_GetTicks();
		}
	} else if (event->key.keysym.sym == SDLK_e) {
		// Simulate click for E upgrade
		if (g_money >= g_upgrades[2].cost) {
			g_money -= g_upgrades[2].cost;
			g_upgrades[2].level++;
			g_upgrades[2].cost *= g_upgrades[2].cost_multiplier;
			g_upgrades[2].last_key_press_time = SDL_GetTicks();
		}
	} else if (event->key.keysym.sym == SDLK_r) {
		// Simulate click for R upgrade
		if (g_money >= g_upgrades[3].cost) {
			g_money -= g_upgrades[3].cost;
			g_upgrades[3].level++;
			g_upgrades[3].cost *= g_upgrades[3].cost_multiplier;
			g_upgrades[3].last_key_press_time = SDL_GetTicks();
		}
	}
}

// Function to handle mouse button down events
void handle_mouse_button_down_event(SDL_Event *event)
{
	if (event->button.button == SDL_BUTTON_LEFT) {
		int mouse_x			  = event->button.x;
		int mouse_y			  = event->button.y;
		SDL_Point mouse_point = { mouse_x, mouse_y };

		for (int i = 0; i < 4; i++) {
			if (SDL_PointInRect(&mouse_point, &g_upgrades[i].rect)) {
				if (g_money >= g_upgrades[i].cost) {
					g_money -= g_upgrades[i].cost;
					g_upgrades[i].level++;
					g_upgrades[i].cost *= g_upgrades[i].cost_multiplier;

					if (i == 0) { // Q - Click Power
						g_money_per_click = 1 + g_upgrades[i].level * 2;
					} else if (i == 1) { // W - Passive Income
						g_money_per_second = g_upgrades[i].level * 1;
					}
				}
			}
		}
	}
}

// Function to process all events
void process_events()
{
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {
			exit(0); // Exit the game
		}
		if (event.type == SDL_KEYDOWN) {
			if (event.key.keysym.sym == SDLK_ESCAPE) {
				exit(0); // Exit the game
			}
			handle_keyboard_event(&event);
		}
		if (event.type == SDL_MOUSEBUTTONDOWN) {
			handle_mouse_button_down_event(&event);
		}
	}
}

// Function to update game state
void update_game_state()
{
	if (SDL_GetTicks() - g_last_second_tick >= 1000) {
		g_money += g_money_per_second;
		g_last_second_tick = SDL_GetTicks();
	}
}

// Function to render top-left info
void render_top_info(SDL_Renderer *renderer, TTF_Font *font_large, TTF_Font *font_small)
{
	char buffer[64];
	snprintf(buffer, sizeof(buffer), "Money: $%lld", g_money);
	update_and_render_text(renderer, font_large, &g_money_tr, buffer, 10, 10, g_color_black);

	snprintf(buffer, sizeof(buffer), "Click Power: $%d", g_money_per_click);
	update_and_render_text(renderer, font_small, &g_click_power_tr, buffer, 10, 50, g_color_black);

	snprintf(buffer, sizeof(buffer), "$/sec: %d", g_money_per_second);
	update_and_render_text(renderer, font_small, &g_money_per_second_tr, buffer, 10, 70, g_color_black);
}

// Function to render upgrade buttons
void render_upgrade_buttons(SDL_Renderer *renderer, TTF_Font *font_large, TTF_Font *font_small, SDL_Point mouse_point)
{
	char buffer[64];
	for (int i = 0; i < 4; i++) {
		int border_thickness = 2; // Default border thickness
		SDL_Color current_border_color; // This will be determined based on affordability or hover

		// Determine the base border color based on affordability
		if (g_money >= g_upgrades[i].cost) {
			current_border_color = g_color_cost_affordable; // Light Green for affordable
		} else {
			current_border_color = g_color_cost_unaffordable; // Light Red for unaffordable
		}

		// Check for hover or recent key press, and override border color if applicable
		Uint32 current_time						  = SDL_GetTicks();
		const Uint32 KEY_PRESS_HIGHLIGHT_DURATION = 200; // milliseconds

		if (SDL_PointInRect(&mouse_point, &g_upgrades[i].rect) ||
			current_time - g_upgrades[i].last_key_press_time < KEY_PRESS_HIGHLIGHT_DURATION) {
			border_thickness = 4; // Thicker border on hover or key press
			if (g_money >= g_upgrades[i].cost) {
				current_border_color = g_color_border_affordable_hover;
			} else {
				current_border_color = g_color_border_unaffordable_hover;
			}
		}

		// Draw button border
		SDL_SetRenderDrawColor(renderer, current_border_color.r, current_border_color.g, current_border_color.b,
							   current_border_color.a);
		SDL_RenderFillRect(renderer, &g_upgrades[i].rect);

		// Draw button background (inner rectangle)
		SDL_Rect bg_rect = { g_upgrades[i].rect.x + border_thickness, g_upgrades[i].rect.y + border_thickness,
							 g_upgrades[i].rect.w - (border_thickness * 2),
							 g_upgrades[i].rect.h - (border_thickness * 2) };
		SDL_SetRenderDrawColor(renderer, g_color_grey.r, g_color_grey.g, g_color_grey.b, g_color_grey.a);
		SDL_RenderFillRect(renderer, &bg_rect);

		// Draw main name (Q, W, E, R)
		render_text(renderer, font_large, g_upgrades[i].name, g_upgrades[i].rect.x + 10,
					g_upgrades[i].rect.y + (g_upgrades[i].rect.h / 2) - 16, g_color_white);

		// Draw Level
		snprintf(buffer, sizeof(buffer), "Lv.%d", g_upgrades[i].level);
		update_and_render_text(renderer, font_small, &g_upgrade_level_tr[i], buffer,
							   g_upgrades[i].rect.x + g_upgrades[i].rect.w - 50, g_upgrades[i].rect.y + 12,
							   g_color_white);

		// Draw Cost
		SDL_Color cost_color = (g_money >= g_upgrades[i].cost) ? g_color_cost_affordable : g_color_cost_unaffordable;
		snprintf(buffer, sizeof(buffer), "$%lld", g_upgrades[i].cost);
		update_and_render_text(renderer, font_small, &g_upgrade_cost_tr[i], buffer,
							   g_upgrades[i].rect.x + g_upgrades[i].rect.w - 50, g_upgrades[i].rect.y + 32, cost_color);
	}
}

// Function to render the entire game scene
void render_game(SDL_Renderer *renderer, TTF_Font *font_large, TTF_Font *font_small, SDL_Point mouse_point)
{
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderClear(renderer);

	render_top_info(renderer, font_large, font_small);
	render_upgrade_buttons(renderer, font_large, font_small, mouse_point);
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

	init_text_renderer(&g_money_tr);
	init_text_renderer(&g_click_power_tr);
	init_text_renderer(&g_money_per_second_tr);
	for (int i = 0; i < 4; i++) {
		init_text_renderer(&g_upgrade_level_tr[i]);
		init_text_renderer(&g_upgrade_cost_tr[i]);
	}

	g_money			   = 1000;
	g_money_per_click  = 1;
	g_money_per_second = 0;
	g_last_second_tick = SDL_GetTicks();

	// Initialize Upgrades
	const char *names[] = { "Q", "W", "E", "R" };
	for (int i = 0; i < 4; i++) {
		g_upgrades[i].name				  = names[i];
		g_upgrades[i].level				  = 0;
		g_upgrades[i].rect.w			  = 150;
		g_upgrades[i].rect.h			  = 60;
		g_upgrades[i].rect.x			  = SCREEN_WIDTH - (4 - i) * (g_upgrades[i].rect.w + 10) - 10;
		g_upgrades[i].rect.y			  = SCREEN_HEIGHT - g_upgrades[i].rect.h - 10;
		g_upgrades[i].last_key_press_time = 0;
	}

	g_upgrades[0].cost			  = 10;
	g_upgrades[0].cost_multiplier = 1.5;

	g_upgrades[1].cost			  = 25;
	g_upgrades[1].cost_multiplier = 1.6;

	// E and R are disabled for now with a high cost
	g_upgrades[2].cost			  = 999999999;
	g_upgrades[2].cost_multiplier = 2.0;
	g_upgrades[3].cost			  = 999999999;
	g_upgrades[3].cost_multiplier = 2.0;

	int running = 1;
	while (running) {
		Uint32 frame_start_time = SDL_GetTicks();
		// EVENT HANDLING
		process_events();

		// GAME LOGIC
		update_game_state();

		// RENDERING
		int mouse_x, mouse_y;
		SDL_GetMouseState(&mouse_x, &mouse_y);
		SDL_Point mouse_point = { mouse_x, mouse_y };
		render_game(renderer, font_large, font_small, mouse_point);

		SDL_RenderPresent(renderer);

		// Cap frame rate
		const int TARGET_FPS	 = 60;
		const int FRAME_DELAY_MS = 1000 / TARGET_FPS;

		Uint32 frame_time = SDL_GetTicks() - frame_start_time;
		if (frame_time < (Uint32)FRAME_DELAY_MS) {
			SDL_Delay(FRAME_DELAY_MS - frame_time);
		}
	}

	// CLEANUP
	TTF_CloseFont(font_large);
	TTF_CloseFont(font_small);

	destroy_text_renderer(&g_money_tr);
	destroy_text_renderer(&g_click_power_tr);
	destroy_text_renderer(&g_money_per_second_tr);
	for (int i = 0; i < 4; i++) {
		destroy_text_renderer(&g_upgrade_level_tr[i]);
		destroy_text_renderer(&g_upgrade_cost_tr[i]);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	TTF_Quit();
	SDL_Quit();

	return 0;
}
