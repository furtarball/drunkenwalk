#include "include/game.h"
#include "include/renderer.h"
#include <SDL2/SDL_filesystem.h>
#include <SDL2/SDL_messagebox.h>
#include <compiletime.h>

int main() {
	try {
		Wrapped<char> path{SDL_GetPrefPath("io.github.furtarball",
										   "io.github.furtarball.drunkenwalk")};
		Game game{std::string{path} + "config.json"};
		game.run();
	} catch (std::exception& e) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Drunken Walk: Error",
								 e.what(), nullptr);
	}
}
