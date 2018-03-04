#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#define STB_IMAGE_IMPLEMENTATION
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "Helper.h"
#include "Matrix.h"
#include "Entity.h"
#include "GameState.h"

#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;
ShaderProgram program;
enum GameMode { Start, Game, Victory, Defeat};
GameMode mode;
const Uint8 *keys = SDL_GetKeyboardState(nullptr);
GameState state;
int fontTextureID;
float lastFrameTicks = 0.0f;
float elapsed = 0.0f;
float bulletCooldown = 1.0f;
float enemyBulletCooldown = 0.0f;

void initEntities() {
	program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	fontTextureID = LoadTexture(RESOURCE_FOLDER"font1.png");
	state.initEntities();
}

void reset() {
	state.reset();
}


void init() {
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 960, 540, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif

	glViewport(0, 0, 960, 540);
	mode = Start;

	initEntities();
}

void processGameState() {
	if (keys[SDL_SCANCODE_A]) {
		state.player.velocity_x = -1.0f;
	}
	else if (keys[SDL_SCANCODE_D]) {
		state.player.velocity_x = 1.0f;
	}
	else {
		state.player.velocity_x = 0.0f;
	}

	if (keys[SDL_SCANCODE_J] && bulletCooldown >= 1.0) {
		state.shootBullet(state.player);
		bulletCooldown = 0.0f;
	}
}

void renderGame() {
	float ticks = (float)SDL_GetTicks() / 1000.0f;
	float elapsed = ticks - lastFrameTicks;
	lastFrameTicks = ticks;
	bulletCooldown += elapsed;
	enemyBulletCooldown += elapsed;
	processGameState();
	state.updateGameState(elapsed);
	state.player.Draw(program);
	DrawMessage(program, fontTextureID, "LIVES:", -3.4, -1.85, 0.2, 0.0);
	for (int i = 0; i < state.playerLives.size(); ++i) {
		state.playerLives[i].Draw(program);
	}
	for (int i = 0; i < state.enemies.size(); ++i) {
		for (int j = 0; j < state.enemies[i].size(); ++j) {
			state.enemies[i][j].Draw(program);
		}
	}

	srand(SDL_GetTicks());
	int randEnemyShoot = rand();
	if (enemyBulletCooldown > 2.0f / state.enemies.size()) {
		state.shootBullet(state.enemies[randEnemyShoot%state.enemies.size()].back());
		enemyBulletCooldown = 0.0f;
	}

	for (int i = 0; i < state.bullets.size(); ++i) {
		state.bullets[i].Draw(program);
	}
	if (state.player.health < 1) {
		mode = Defeat;
	}
	if (state.enemies.empty()) {
		mode = Victory;
	}
	for (int i = 0; i < state.enemies.size(); ++i) {
		if (state.enemies[i].back().y - state.enemies[i].back().height/2 < -1.75) {
			mode = Defeat;
		}
	}
}

void renderState() {
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgram(program.programID);
	switch (mode) {
	case Start:
		DrawMessage(program, fontTextureID, "PRESS SPACE TO START", -0.3 * 19/2.0, 0.5, 0.3f, 0.0f);
		DrawMessage(program, fontTextureID, "INSTRUCTIONS", -0.2*12 /2.0, 0.0f, 0.2f, 0.0f);
		DrawMessage(program, fontTextureID, "USE A AND D TO MOVE", -0.15 * 19/ 2.0, -0.2, 0.15f, 0.0f);
		DrawMessage(program, fontTextureID, "USE J TO SHOOT", -0.15 * 14 / 2.0, -0.35, 0.15f, 0.0f);
		if (keys[SDL_SCANCODE_SPACE]) {
			mode = Game;
		}
		break;
	case Game:
		renderGame();
		break;
	case Victory:
		DrawMessage(program, fontTextureID, "YOU WIN - PRESS SPACE", -0.3 * 21 / 2.0, 0.5, 0.3f, 0.0f);
		DrawMessage(program, fontTextureID, "TO REPLAY THE GAME", -0.3 * 18 / 2.0, -0.5, 0.3f, 0.0f);
		if (keys[SDL_SCANCODE_SPACE]) {
			reset();
			mode = Start;
		}
		break;
	case Defeat:
		DrawMessage(program, fontTextureID, "YOU LOSE - PRESS SPACE", -0.3 * 22 / 2.0, 0.5, 0.3f, 0.0f);
		DrawMessage(program, fontTextureID, "TO REPLAY THE GAME", -0.3 * 18 / 2.0, -0.5, 0.3f, 0.0f);
		if (keys[SDL_SCANCODE_SPACE]) {
			reset();
			mode = Start;
		}
		break;
	}
}

int main(int argc, char *argv[])
{
	init();

	SDL_Event event;
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}
		renderState();
		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
