#include "include/game.h"
#include <SDL_messagebox.h>

int main() {
	try {
		Game game{"assets/config.json"};
		game.run();
	} catch (std::exception& e) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
								 "Drunken Walk: Error", e.what(),
								 nullptr);
	}
}
