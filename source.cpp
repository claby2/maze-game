#include "C:/MinGW/include/SDL2/SDL.h"
#include <iostream>
#include <algorithm>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <vector>
#include <utility>

const int SCREEN_HEIGHT = 1000;
const int SCREEN_WIDTH = 1000;

const int height = 50;
const int width = 50;

const int FRAME_RATE = std::min(height, width) * 10;

const uint32_t WHITE = 0xffffffff;
const uint32_t BLACK = 0xff000000;
const uint32_t GREEN = 0xff00ff00;
const uint32_t RED = 0xff0000ff;
const uint32_t BLUE = 0xffff0000;

int dx[] = {0, 0, 1, -1, 1, 1, -1, -1};
int dy[] = {1, -1, 0, 0, 1, -1, 1, -1};

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;

class Player {
    public:
        int x = 0;
        int y = 0;

        void handleEvent(SDL_Event& e, std::vector<uint32_t>& pixels) {
            if(e.type == SDL_KEYDOWN && e.key.repeat == 0) {
                switch(e.key.keysym.sym){
                    case SDLK_w: up = true; break;
                    case SDLK_s: down = true; break;
                    case SDLK_a: right = true; break;
                    case SDLK_d: left = true; break;
                } 
            } else if (e.type == SDL_KEYUP) {
                switch(e.key.keysym.sym){
                    case SDLK_w: up = false; break;
                    case SDLK_s: down = false; break;
                    case SDLK_a: right = false; break;
                    case SDLK_d: left = false; break;
                } 
            }
        }

        void move(std::vector<uint32_t>& pixels) {
            if(pixels[(y-1)*width + x] != BLACK && y - 1 >= 0 && up) y -= 1;
            if(pixels[(y+1)*width + x] != BLACK && y + 1 < height && down) y += 1;
            if(pixels[y*width + x - 1] != BLACK && x - 1 >= 0 && right) x -= 1;
            if(pixels[y*width + x + 1] != BLACK && x + 1 < width && left) x += 1;
        }


        void render(std::vector<uint32_t>& pixels, std::vector<bool>& visited) {
            visited[y*width + x] = true;
            pixels[y*width + x] = RED;
        }

    private:
        bool up = false;
        bool down = false;
        bool right = false;
        bool left = false;
};

bool init() {
	bool success = true;
	if(SDL_Init( SDL_INIT_VIDEO ) < 0) {
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		success = false;
	}
	else {
		if(!SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
			printf( "Warning: Linear texture filtering not enabled!" );
		}
		gWindow = SDL_CreateWindow( "Maze", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, 0 );
		if(gWindow == NULL) {
			printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			success = false;
		}
		else {
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
			if(gRenderer == NULL) {
				printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
			}
			else {
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
			}
		}
	}
	return success;
}

void generateMaze(std::vector<uint32_t>& pixels, int sx, int sy) {
    std::vector<std::pair<int, int> > walls;
    std::vector<std::pair<int, int> > unvisited;

    for(int i = 0; i < height; i++) {
        for(int j = 0; j < width; j++) {
            pixels[i*width + j] = BLACK;
        }
    }

    pixels[sy*width + sx] = WHITE;

    if(sx + 1 < width) walls.push_back(std::make_pair(sx + 1, sy));
    if(sx - 1 >= 0) walls.push_back(std::make_pair(sx - 1, sy));
    if(sy + 1 < height) walls.push_back(std::make_pair(sx, sy + 1));
    if(sy - 1 >= 0) walls.push_back(std::make_pair(sx, sy - 1));

    while(walls.size() > 0) {
        int randomIndex = rand()%walls.size();
        int nx = walls[randomIndex].first;
        int ny = walls[randomIndex].second;

        unvisited.clear();
        if(nx + 1 < width && pixels[ny*width + nx + 1] == WHITE) unvisited.push_back(std::make_pair(nx-1, ny));
        if(nx - 1 >= 0 && pixels[ny*width + nx - 1] == WHITE) unvisited.push_back(std::make_pair(nx + 1, ny));
        if(ny + 1 < height && pixels[(ny+1)*width + nx] == WHITE) unvisited.push_back(std::make_pair(nx, ny -1));
        if(ny - 1 >= 0 && pixels[(ny-1)*width + nx] == WHITE) unvisited.push_back(std::make_pair(nx, ny + 1));

        if(unvisited.size() == 1) {
            int ux = unvisited[0].first;
            int uy = unvisited[0].second;

            pixels[ny*width + nx] = WHITE;
            if(ux >= 0 && ux < width && uy >= 0 && uy < height) {
                pixels[uy*width + ux] = WHITE;

                if(ux + 1 < width && pixels[uy*width + ux + 1] == BLACK) walls.push_back(std::make_pair(ux + 1, uy));
                if(ux - 1 >= 0 && pixels[uy*width + ux - 1] == BLACK) walls.push_back(std::make_pair(ux - 1, uy));
                if(uy + 1 < height && pixels[(uy+1)*width + ux] == BLACK) walls.push_back(std::make_pair(ux, uy + 1));
                if(uy - 1 >= 0 && pixels[(uy-1)*width + ux] == BLACK) walls.push_back(std::make_pair(ux, uy - 1));

            }
        }

        walls.erase(walls.begin()+randomIndex);
    }
}

void resetGame(std::vector<uint32_t>& pixels, std::vector<bool>& visited, Player& player) {
    for(int i = 0; i < height; i++) {
        for(int j = 0; j < width; j++) {
            visited[i*width + j] = false;
        }
    }
    visited[0] = true;
    generateMaze(pixels, 0, 0);
    pixels[(height-1)*width + width-1] = BLUE;
    player.x = 0;
    player.y = 0;
}

int main(int argc, char* args[]){
    srand((unsigned)time(NULL));
    if(!init()){
        printf("Failed to initialize!\n");
    } else {
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
        SDL_Texture* gTexture = SDL_CreateTexture(
            gRenderer,
            SDL_PIXELFORMAT_ABGR8888,
            SDL_TEXTUREACCESS_STATIC,
            width,
            height
        );
        bool quit = false;

        std::vector<uint32_t> pixels(height*width, BLACK);
        std::vector<bool> visited(height*width, false);
        Player player;

        resetGame(pixels, visited, player);

        SDL_Event e;

        int frame = 0;

        while(!quit){
            while(SDL_PollEvent( &e ) != 0) {
                if( e.type == SDL_QUIT ) {
                    quit = true;
                } else if(e.type == SDL_KEYDOWN){
                    switch(e.key.keysym.sym){
                        case SDLK_ESCAPE:
                            quit = true;
                            break;
                    }
                }

                player.handleEvent(e, pixels);
            }
            SDL_UpdateTexture(gTexture, NULL, &pixels[0], width * sizeof(uint32_t));
            SDL_RenderClear(gRenderer);
            SDL_RenderCopy(gRenderer, gTexture, NULL, NULL);
            frame++;
            if(frame == FRAME_RATE) {
                player.move(pixels);
                frame = 0;
            }

            for(int i = 0; i < height; i++) {
                for(int j = 0; j < width; j++) {
                    if(visited[i*width + j]) {
                        pixels[i*width + j] = GREEN;
                    }
                }
            }

            if(visited[(height-1)*width + (width-1)]) {
                resetGame(pixels, visited, player);
            }

            player.render(pixels, visited);


            SDL_RenderPresent(gRenderer);
        }
        SDL_DestroyTexture(gTexture);
    }
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    gWindow = NULL;
    gRenderer = NULL;
    SDL_Quit();
    return 0;
}