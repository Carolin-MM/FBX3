#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <iostream>
#include <string>
#include <vector>
#include <time.h>
#include <windows.h>

using namespace std;

//Screen dimension constants
const int SCREEN_WIDTH = 512;
const int SCREEN_HEIGHT = 512;

#define PI 3.14159265

#define DISTANCE_MULTIPLIER 23

//GLOBAL
SDL_Renderer* gMyRenderer = NULL;
//Audios
vector<Mix_Chunk*> audios;
bool exitGame = false, playing = true, first = true, superFirst = true;
int channelWaterfall, channelMonster;
Uint8* eco0, * eco1, * eco2, * eco3, * eco4, * eco5, * eco6, * eco7, * eco8, * eco9;
int ecoIndex = 0;

struct PlayerPosition {
    int row;
    int column;
	int rotation;
};
PlayerPosition player = { 7, 2, 0 };

struct Position {
    int row;
    int column;
};
Position monster = { 4, 5 };
Position waterfall = { 8 , 8 };

int map[10][10] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 1, 1, 0, 1, 1, 1, 1, 0},
    {0, 1, 1, 1, 0, 1, 1, 1, 1, 0},
    {0, 1, 0, 1, 0, 1, 1, 1, 1, 0},
    {0, 1, 1, 1, 1, 1, 1, 0, 1, 0},
    {0, 1, 1, 1, 1, 1, 1, 0, 1, 0},
    {0, 1, 1, 0, 1, 1, 1, 0, 1, 0},
    {0, 1, 1, 0, 1, 1, 1, 0, 1, 0},
    {0, 1, 1, 0, 1, 1, 1, 0, 2, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

void MonsterEat() {
    playing = false;

    Mix_Pause(-1);
    Mix_SetPostMix(NULL, NULL);
    int channel = Mix_PlayChannel(-1, audios[4], 0);
    Mix_SetPosition(channel, 0, 0);
    Sleep(1000);

    Mix_Pause(-1);
    channel = Mix_PlayChannel(-1, audios[5], -1);
    Mix_SetPosition(channel, 0, 0);
}

void MoveMonster() {
    int direction = rand() % 4;
    bool posible = false;
    int auxRow, auxColumn;

    do {
        auxRow = monster.row;
        auxColumn = monster.column;

        switch (direction) {
            case 0:
                auxRow += -1;
                break;

            case 1:
                auxColumn += 1;
                break;

            case 2:
                auxRow += 1;
                break;

            case 3:
                auxColumn += -1;
                break;
        }

        posible = map[auxRow][auxColumn] == 1;
        direction = (direction + 1) % 4;
    } while (!posible);

    monster.column = auxColumn;
    monster.row = auxRow;
    std::cout << "Monster moves to " << auxRow << ", " << auxColumn << endl;

    float distance = sqrt(pow(monster.column - player.column, 2) +
                          pow(monster.row - player.row, 2) * 1.0);
    distance *= DISTANCE_MULTIPLIER;
    if (distance > 255) distance = 255;

    int soundDirection = (atan2(monster.row - player.row, monster.column - player.column)) * 180 / PI;
    soundDirection -= 90;
    soundDirection -= player.rotation;
    while (soundDirection < 0) soundDirection += 360;
    soundDirection = soundDirection % 360;

    int channel = Mix_PlayChannel(-1, audios[7], 0);
    Mix_SetPosition(channel, soundDirection, distance);
}

void MovePlayer() {
    int auxRow = player.row;
    int auxColumn = player.column;

    switch (player.rotation) {
        case 0:
            auxRow += -1;
            break;

        case 90:
            auxColumn += 1;
            break;

        case 180:
            auxRow += 1;
            break;

        case 270:
            auxColumn += -1;
            break;
    }

    int channel;
    switch (map[auxRow][auxColumn]) {
        case 0:
            channel = Mix_PlayChannel(-1, audios[3], 0);
            Mix_SetPosition(channel, 0, 0);
            std::cout << "Player hits the wall" << endl;

            MoveMonster();
            if (monster.column == auxColumn && monster.row == auxRow)
                MonsterEat();

            break;

        case 1:
            if (monster.column == auxColumn && monster.row == auxRow) {
                MonsterEat();
                return;
            }
            channel = Mix_PlayChannel(-1, audios[2], 0);
            Mix_SetPosition(channel, 0, 0);

            player.column = auxColumn;
            player.row = auxRow;
            std::cout << "Player moves to " << auxRow << ", " << auxColumn << endl;
            break;

        case 2:
            playing = false;

            Mix_Pause(-1);
            Mix_SetPostMix(NULL, NULL);
            channel = Mix_PlayChannel(-1, audios[6], -1);
            Mix_SetPosition(channel, 0, 0);
            break;
    }
}

void SetSoundsPositions() {
    float distance = sqrt(pow(waterfall.column - player.column, 2) +
                          pow(waterfall.row - player.row, 2) * 1.0);
    distance *= DISTANCE_MULTIPLIER;
    if (distance > 255) distance = 255;

    int direction = (atan2(waterfall.row - player.row, waterfall.column - player.column)) * 180 / PI;
    direction -= 90;
    direction -= player.rotation;
    while (direction < 0) direction += 360;
    direction = direction % 360;

    std::cout << "Waterfall sound " << distance << ", " << direction << endl;
    Mix_SetPosition(channelWaterfall, direction, distance);

    distance = sqrt(pow(monster.column - player.column, 2) +
                    pow(monster.row - player.row, 2) * 1.0);
    distance *= DISTANCE_MULTIPLIER;
    if (distance > 255) distance = 255;

    direction = (atan2(monster.row - player.row, monster.column - player.column)) * 180 / PI;
    direction -= 90;
    direction -= player.rotation;
    while (direction < 0) direction += 360;
    direction = direction % 360;

    std::cout << "Monster sound " << distance << ", " << direction << endl;
    Mix_SetPosition(channelMonster, direction, distance);
}

void echoEffect(void* udata, Uint8* stream, int len) {
    Uint8* aux = (Uint8*)malloc(len), * eco;

    if (superFirst) {
        superFirst = false;
        eco0 = (Uint8*)malloc(len);
        eco1 = (Uint8*)malloc(len);
        eco2 = (Uint8*)malloc(len);
        eco3 = (Uint8*)malloc(len);
        eco4 = (Uint8*)malloc(len);
        eco5 = (Uint8*)malloc(len);
        eco6 = (Uint8*)malloc(len);
        eco7 = (Uint8*)malloc(len);
        eco8 = (Uint8*)malloc(len);
        eco9 = (Uint8*)malloc(len);
    }

    switch (ecoIndex) {
    case 0: eco = eco0;
        break;

    case 1: eco = eco1;
        break;

    case 2: eco = eco2;
        break;

    case 3: eco = eco3;
        break;

    case 4: eco = eco4;
        break;

    case 5: eco = eco5;
        break;

    case 6: eco = eco6;
        break;

    case 7: eco = eco7;
        break;

    case 8: eco = eco8;
        break;

    case 9: eco = eco9;
        break;

    default:
        return;
    }
     
    // Guardar salida de audio para proximos ecos en variable auxiliar
    for (int i = 0; i < len; i++) {
        *(aux + i) = *(stream + i);
    }

    // Sumar eco a salida de audio
    if (!first) {
        for (int i = 0; i < len; i++) {
            *(stream + i) = *(stream + i) + *(eco + i);
        }
    }

    if (first && ecoIndex == 9) first = false;

    // Guardar salida para proximos ecos en variabe definitiva
    int pos = 0;
    for (int i = 0; i < len; i++) {
        *(eco + i) = *(aux + i);
    }

    ecoIndex = (++ecoIndex) % 10;
}

void InitSounds() {
    Mix_SetPostMix(echoEffect, NULL);
    channelMonster = Mix_PlayChannel(-1, audios[0], -1);
    channelWaterfall = Mix_PlayChannel(-1, audios[1], -1);
    SetSoundsPositions();
}

int main(int argc, char* args[]) {
    SDL_Window *gWindow = NULL;

    //Mouse
    int mouseX = 0, mouseY = 0;

    //INIT

    srand(time(0));
    //Initialize SDL
    SDL_Init(SDL_INIT_EVERYTHING);
    //Create window
    gWindow = SDL_CreateWindow("PEC3 - Efectes visuals i sonors ", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                               SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    //Initialize PNG loading
    IMG_Init(IMG_INIT_PNG);
    //Get window renderer
    gMyRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_SetHint("SDL_HINT_RENDER_VSYNC", "1");
    //Sound audio active
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);

    //Load Audios
    Mix_Chunk *loadSound;
    loadSound = Mix_LoadWAV("Assets/snore.wav");
    audios.push_back(loadSound);
    loadSound = Mix_LoadWAV("Assets/water.wav");
    audios.push_back(loadSound);
    loadSound = Mix_LoadWAV("Assets/walk.wav");
    audios.push_back(loadSound);
    loadSound = Mix_LoadWAV("Assets/hurt.wav");
    audios.push_back(loadSound);
    loadSound = Mix_LoadWAV("Assets/death.wav");
    audios.push_back(loadSound);
    loadSound = Mix_LoadWAV("Assets/gameover_music.wav");
    audios.push_back(loadSound);
    loadSound = Mix_LoadWAV("Assets/victory.wav");
    audios.push_back(loadSound);
    loadSound = Mix_LoadWAV("Assets/monster.wav");
    audios.push_back(loadSound);

    InitSounds();

    while (!exitGame) {
        // UPDATE
        SDL_Event test_event;
        while (SDL_PollEvent(&test_event)) {
            switch (test_event.type) {
                case SDL_KEYDOWN:
                    if (!playing) continue;

                    switch (test_event.key.keysym.scancode) {
                        case SDL_SCANCODE_ESCAPE:
                            exitGame = true;
                            break;

                        case SDL_SCANCODE_UP:

                        case SDL_SCANCODE_W:
                            MovePlayer();
                            break;

                        case SDL_SCANCODE_RIGHT:

                        case SDL_SCANCODE_D:
                            player.rotation = (player.rotation + 90) % 360;
                            std::cout << "Player turns right" << endl;
                            break;

                        case SDL_SCANCODE_LEFT:

                        case SDL_SCANCODE_A:
                            player.rotation = (player.rotation - 90) % 360;
                            if (player.rotation < 0) player.rotation += 360;
                            std::cout << "Player turns left" << endl;
                            break;
                    }
                    SetSoundsPositions();
                    break;

                case SDL_QUIT:
                    exitGame = true;
                    break;
            }
        }

        // UPDATE SOUNDS


        //Update the surface
        SDL_RenderPresent(gMyRenderer);
    }

    //Destroy window
    SDL_DestroyRenderer(gMyRenderer);
    gMyRenderer = NULL;
    SDL_DestroyWindow(gWindow);
    gWindow = NULL;

    //Quit SDL subsystems
    free(eco0);
    free(eco1);
    free(eco2);
    free(eco3);
    free(eco4);
    free(eco5);
    free(eco6);
    free(eco7);
    free(eco8);
    free(eco9);
    Mix_CloseAudio();
    IMG_Quit();
    SDL_Quit();

    return 0;
}