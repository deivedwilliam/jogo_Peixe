/*This source code copyrighted by Lazy Foo' Productions (2004-2015)
and may not be redistributed without written permission.*/

//Using SDL and standard IO
#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include "Agent.h"

#define WIDTH 64
#define HEIGHT 64

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 640;

typedef struct{
	int width;
	int height;
	SDL_Surface** sprite;

} Board;

SDL_Window* window = NULL;
SDL_Surface* screenSurface = NULL;

SDL_Surface* square_sprite = NULL;
SDL_Surface* fish_sprite = NULL;
SDL_Surface* shark_sprite = NULL;

int amountSharks = 0;
int amountFishs = 0;

Board board;
Shark sharks[100];
Fish fishes[100];

int i = 0, j = 0;

SDL_Surface* loadSurface(char path[]);

void close();

int init();

int loadSprites();

void draw();

void update();

int distance(Object a, Object b);

int main( int argc, char* args[] )
{
	//The window we'll be rendering to
	
	if(!init()){
		printf("Failed to initialize!\n");
		return 0;
	}

	if(!loadSprites()){
		printf("Failed to load sprites!\n");
		return 0;
	}


	srand(time(NULL));   // should only be called once

	createGame();

	//Main loop flag
	int quit = 0;

	//Event handler
	SDL_Event e;

	int nextStep = 1; //true for the frist draw

	//While application is running
	while( !quit )
	{

		//Handle events on queue
		while( SDL_PollEvent( &e ) != 0 )
		{
			if( e.type == SDL_Quit )
			{
				quit = 1;
			}
			//User requests quit
			switch( e.type ){
					case SDL_KEYDOWN:
						switch( e.key.keysym.sym ){
	                    case SDLK_ESCAPE:
	                        quit = 1;
	                        break;
	                    case SDLK_RETURN:
	                    	nextStep = 1;
	                    	break;
	                }
				}     				
		}

		if(!nextStep)
			continue;

		nextStep = 0;

		update();
		draw();

		SDL_Delay( 100 );

	}

	close();

	return 0;
}

void update(){
	
	//Movimentação shark
	for(i = 0; i < amountSharks; i++){

		if(sharks[i].state == SHARK_IDLE){
		
			int nextInX[4] = { sharks[i].obj.x - 1, sharks[i].obj.x, sharks[i].obj.x + 1, sharks[i].obj.x };
			int nextInY[4] = { sharks[i].obj.y, sharks[i].obj.y - 1, sharks[i].obj.y, sharks[i].obj.y + 1 };

			int myNextX;
			int myNextY;
			int emptyNext = 1;
			do{
				myNextX = nextInX[rand() % 4];
				myNextY = nextInY[rand() % 4];
				emptyNext = 1;

				if(myNextX < 0 || myNextX >= board.width || myNextY < 0 || myNextY >= board.height){
					emptyNext = 0;
					continue;
				}

				for(j = 0; j < amountSharks; j++){
					//Nao estou conferindo comigo msm
					if(i != j){
						if(myNextX == sharks[j].obj.x && myNextY == sharks[j].obj.y)
							emptyNext = 0;
					}
				}

				for(j = 0; j < amountFishs; j++){
					if(myNextX == fishes[j].obj.x && myNextY == fishes[j].obj.y)
						emptyNext = 0;
				}

			} while(!emptyNext);

			sharks[i].obj.x = myNextX;
			sharks[i].obj.y = myNextY;
		}
		else if(sharks[i].state == SHARK_CHASE){
			//Segue o inimigo
			Fish fish = fishes[sharks[i].indexFish];
			Shark shark = sharks[i];
			if(fish.obj.x > shark.obj.x){
				sharks[i].obj.x += 1;
			}
			else if(fish.obj.x < shark.obj.x){
				sharks[i].obj.x += -1;
			}
			else{
				if(fish.obj.y > shark.obj.y){
					sharks[i].obj.y += 1;
				}
				if(fish.obj.y < shark.obj.y){
					sharks[i].obj.y += -1;
				}
			}
		}
		/*else if(sharks[i].state == SHARK_ATTACK){
			if(sharks[i].indexFish != -1){
				fishes[sharks[i].indexFish] = fishes[amountFishs - 1];
				amountFishs--;
			}
			sharks[i].indexFish = -1;
			sharks[i].state = SHARK_IDLE;
		}*/




		//Se não estiver seguindo nenhum peixe ele vai ver se tem algum
		if(sharks[i].indexFish == -1){
			int betterDistance = 99;
			for(j = 0; j < amountFishs; j++){
				int distance = abs(sharks[i].obj.x - fishes[j].obj.x) + abs(sharks[i].obj.y - fishes[j].obj.y);
				if(distance <= sharks[i].captureRange && distance < betterDistance){
					sharks[i].indexFish = j;
					sharks[i].state = SHARK_CHASE;
					betterDistance = distance;
				}
			}
		}
		else{
			int distance = abs(sharks[i].obj.x - fishes[sharks[i].indexFish].obj.x) + 
							abs(sharks[i].obj.y - fishes[sharks[i].indexFish].obj.y);

			if(distance <= 1){
				sharks[i].state = SHARK_ATTACK;
				if(sharks[i].indexFish != -1){
					fishes[sharks[i].indexFish] = fishes[amountFishs - 1];
					amountFishs--;
				}
			sharks[i].indexFish = -1;
			sharks[i].state = SHARK_IDLE;
			} 
		}
	}

	//movimentação peixe
	for(i = 0; i < amountFishs; i++){

		Fish fish = fishes[i];
		if(fish.state == FISH_IDLE){
			int nextInX[4] = { fishes[i].obj.x - 1, fishes[i].obj.x, fishes[i].obj.x + 1, fishes[i].obj.x };
			int nextInY[4] = { fishes[i].obj.y, fishes[i].obj.y - 1, fishes[i].obj.y, fishes[i].obj.y + 1 };

			int myNextX;
			int myNextY;
			int emptyNext = 1;
			do{
				int sorted = rand() % 4;
				myNextX = nextInX[sorted];
				myNextY = nextInY[sorted];
				emptyNext = 1;

				if(myNextX < 0 || myNextX >= board.width || myNextY < 0 || myNextY >= board.height){
					emptyNext = 0;
					continue;
				}

				for(j = 0; j < amountFishs; j++){
					//Nao estou conferindo comigo msm
					if(i != j){
						if(myNextX == fishes[j].obj.x && myNextY == fishes[j].obj.y)
							emptyNext = 0;
					}
				}

				for(j = 0; j < amountSharks; j++){
					if(myNextX == sharks[j].obj.x && myNextY == sharks[j].obj.y)
						emptyNext = 0;
				}
				
			} while(!emptyNext);

			fishes[i].obj.x = myNextX;
			fishes[i].obj.y = myNextY;
		}
		else if(fish.state == FISH_RUNNING){

			if(fish.indexShark != -1){

				Shark shark = sharks[fish.indexShark];

				if(fish.obj.x > shark.obj.x && fish.obj.x < board.width - 1){
					fishes[i].obj.x += 1;
				}
				else if(fish.obj.x < shark.obj.x && fish.obj.x > 0){
					fishes[i].obj.x += -1;
				}
				else{
					if(fish.obj.y > shark.obj.y && fish.obj.y < board.height - 1){
						fishes[i].obj.y += 1;
					}
					if(fish.obj.y < shark.obj.y && fish.obj.y > 0){
						fishes[i].obj.y += -1;
					}
				}	
			}

			fishes[i].state = FISH_IDLE;
			fishes[i].indexShark = -1;
		}

		int betterDistance = 99;
		for(j = 0; j < amountSharks; j++){
			
			Shark shark = sharks[j];

			int distance = abs(shark.obj.x - fish.obj.x) + abs(shark.obj.y - fish.obj.y);
			if(distance <= fish.perceptionRange && distance < betterDistance){
				betterDistance = distance;
				fishes[i].indexShark = j;
				fishes[i].state = FISH_RUNNING;
			}
		}

		for(j = 0; j < amountFishs; j++){

			Fish fish = fishes[j];
			if(fishes[i].timeAfterReproduction < 3 || fish.timeAfterReproduction < 3){
				continue;
			}

			if(j != i){
				int distanceA = distance(fish.obj, fishes[i].obj);

				if(distanceA <= 1){
					printf("distancia sendo menor que 1\n");
					//int nextInX[8] = { -1, -1, -1, 0,  0, 1, 1,  1};
					//int nextInY[8] = {  1,  0, -1, 1, -1, 1, 0, -1};
					
					int nextInX[4] = { -1, -1, 1, 1,};
					int nextInY[4] = {  1, -1, 1, -1};
					
					int sorted = rand() % 4;
					int posX = fishes[i].obj.x + nextInX[sorted];
					int posY = fishes[i].obj.y + nextInY[sorted];
					while(abs(posX - fish.obj.x) + abs(posY - fish.obj.y) < 2){
						sorted = rand() % 4;
						posX = fishes[i].obj.x + nextInX[sorted];
						posY = fishes[i].obj.y + nextInY[sorted];
					}

					fish.timeAfterReproduction = 0;
					fishes[i].timeAfterReproduction = 0;

					fishes[amountFishs].obj.x = posX;
					fishes[amountFishs].obj.y = posY;
					fishes[amountFishs].obj.life = 10;
					fishes[amountFishs].obj.sprite = fish_sprite;

					fishes[amountFishs].state = FISH_IDLE;
					fishes[amountFishs].perceptionRange = 5;
					fishes[amountFishs].indexShark = -1;
					fishes[amountFishs].timeAfterReproduction = 0;

					amountFishs++;
				}
			}
		}

		fishes[i].timeAfterReproduction++;
	}
}

int distance(Object a, Object b){

	return abs(a.x - b.x) + abs(a.y - b.y);
}

void draw(){

	SDL_Rect rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = SCREEN_WIDTH / board.width;
	rect.h = SCREEN_HEIGHT / board.height;

	for(i = 0; i < board.width; i++){
		for(j = 0; j < board.height; j++){
			rect.x = i * rect.w;
			rect.y = j * rect.h;

			SDL_BlitSurface( board.sprite, NULL, screenSurface, &rect );
		}
	}

	for(i = 0; i < amountSharks; i++){
		rect.x = sharks[i].obj.x * rect.w;;
		rect.y = sharks[i].obj.y * rect.h;

		SDL_BlitSurface( sharks[i].obj.sprite, NULL, screenSurface, &rect );
	}

	for(i = 0; i < amountFishs; i++){
		rect.x = fishes[i].obj.x * rect.w;;
		rect.y = fishes[i].obj.y * rect.h;

		SDL_BlitSurface( fishes[i].obj.sprite, NULL, screenSurface, &rect );
	}

	SDL_UpdateWindowSurface( window );
}

void createGame(){

	//FAZER SÓ OS TUBARÕES
	printf("Insira a quantidade de tubaroes: ");
	scanf("%i", &amountSharks);

	printf("Insira a quantidade de peixes: ");
	scanf("%i", &amountFishs);

	board.width = WIDTH;
	board.height = HEIGHT;
	board.sprite = square_sprite;

	for(i = 0; i < amountSharks; i++){
		
		sharks[i].obj.x = rand() % 64;
		sharks[i].obj.y = rand() % 64;
		sharks[i].obj.life = 10;
		sharks[i].obj.sprite = shark_sprite;

		sharks[i].state = SHARK_IDLE;
		sharks[i].captureRange = 10;
		sharks[i].indexFish = -1;
	}

	for(i = 0; i < amountFishs; i++){
		
		fishes[i].obj.x = rand() % 64;
		fishes[i].obj.y = rand() % 64;
		fishes[i].obj.life = 10;
		fishes[i].obj.sprite = fish_sprite;
		fishes[i].timeAfterReproduction = 0;

		fishes[i].state = FISH_IDLE;
		fishes[i].perceptionRange = 5;
		fishes[i].indexShark = -1;
	}

}

SDL_Surface* loadSurface(char path[])
{
    //The final optimized image
    SDL_Surface* optimizedSurface = NULL;

    //Load image at specified path
    SDL_Surface* loadedSurface = IMG_Load( path );
    if( loadedSurface == NULL )
    {
        printf( "Unable to load image %s! SDL_image Error: %s\n", path, IMG_GetError() );
    }
    else
    {
        //Convert surface to screen format
        optimizedSurface = SDL_ConvertSurface( loadedSurface, screenSurface->format, NULL );
        if( optimizedSurface == NULL )
        {
            printf( "Unable to optimize image %s! SDeL Error: %s\n", path, SDL_GetError() );
        }

        //Get rid of old loaded surface
        SDL_FreeSurface( loadedSurface );
    }

    return optimizedSurface;
}

void close()
{
	//Free loaded image
	SDL_FreeSurface( square_sprite );
	SDL_FreeSurface( fish_sprite );
	SDL_FreeSurface( shark_sprite );
	square_sprite = NULL;
	fish_sprite = NULL;
	shark_sprite = NULL;

	//Destroy window
	SDL_DestroyWindow( window );
	window = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

int init(){
	int sucess = 1;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
		sucess = 0;
	}
	else
	{
		//Create window
		window = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		if( window == NULL )
		{
			printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
			sucess = 0;
		}
	}

	//Get window surface
	screenSurface = SDL_GetWindowSurface( window );

	//Fill the surface white
	SDL_FillRect( screenSurface, NULL, SDL_MapRGB( screenSurface->format, 0xFF, 0xFF, 0xFF ) );

	SDL_UpdateWindowSurface( window );

	return sucess;
}

int loadSprites(){

	square_sprite = loadSurface("square.png");
	fish_sprite = loadSurface("fish.png");
	shark_sprite = loadSurface("shark.png");

	if(square_sprite == NULL || fish_sprite == NULL || shark_sprite == NULL)
		return 0;

	return 1;
}