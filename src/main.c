#include <SDL.h>

int main(int argc, char** argv)
{
	(void)argc;
	(void)argv;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		return 1;
	}

	SDL_Window* window = SDL_CreateWindow(
		"Clicker Game",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		640,
		480,
		SDL_WINDOW_SHOWN
	);

	if (!window) {
		SDL_Log("Unable to create window: %s", SDL_GetError());
		goto out;
	}

	SDL_Delay(1000);
	SDL_DestroyWindow(window);

out:
	SDL_Quit();

	return 0;
}
