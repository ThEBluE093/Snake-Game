#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <ctime>
#include <cstdlib>

extern "C" {
#include "./SDL2-2.0.10/include/SDL.h"
#include "./SDL2-2.0.10/include/SDL_main.h"
}

#define SCREEN_WIDTH 700
#define SCREEN_HEIGHT 700
#define GRID_SIZE 18
#define GRID_LINE_SPACING 30
#define SNAKE_LENGTH 5
#define SPEED_UP 0.20 //in percentage
#define TIME_INTERVAL 10 //in seconds
#define SNAKE_SIZE 17
#define POINTS_FOR_BALLZ 1
#define SNAKE_STARTING_SPEED 0.4 //in seconds
#define SHORTENING 2
#define MOV_SLOWER 0.10 //in percantage
#define WAIT_FOR_DOT 5 //in seconds
#define MAX_NAME_LENGTH 50 // max letters

struct Segment_Snake {
    int x;
    int y;
    int size;
    int kierunek_weza;
    int pkt;
    int dlugosc;
    double speed;
};

struct Blue_dot {
    int x;
    int y;
    int flaga;
};

struct Red_dot {
    int x;
    int y;
    int flaga;
    double czas_od_poj;
    int czas_do_poj;
};

struct Game_Tools {
    int quit;
    double worldTime;
    bool flaga_do_new_game, flaga_do_menu, wstep;
};

struct Score {
  char name[MAX_NAME_LENGTH];
  int score;
};

//Główne narzędzie do rysowania ----------------->

void DrawString(SDL_Surface *screen, int x, int y, const char *text, SDL_Surface *charset) {
    int px, py, c;
    SDL_Rect s, d;
    s.w = 8;
    s.h = 8;
    d.w = 8;
    d.h = 8;
    while(*text) {
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

void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
    int bpp = surface->format->BytesPerPixel;
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
    *(Uint32 *)p = color;
};

void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color) {
    for(int i = 0; i < l; i++) {
        DrawPixel(screen, x, y, color);
        x += dx;
        y += dy;
    };
};

void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k, Uint32 outlineColor, Uint32 fillColor) {
    int i;
    DrawLine(screen, x, y, k, 0, 1, outlineColor);
    DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
    DrawLine(screen, x, y, l, 1, 0, outlineColor);
    DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
    for(i = y + 1; i < y + k - 1; i++)
        DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
};



//----------------->
//Obsługa rankingów----------------->

Score* load_scores_from_file(const char* filename, int* num_scores) {
  FILE* file = fopen(filename, "r");
  if (file == NULL) {
    SDL_Log("Nie można otworzyć pliku: %s", filename);
    return NULL;
  }
  Score* scores = (Score*)malloc(3 * sizeof(Score));
  if (scores == NULL) {
    SDL_Log("Błąd alokacji pamięci.");
    fclose(file);
    return NULL;
  }

  *num_scores = 0;
  while (*num_scores < 3 && fscanf(file, "%s - %d", scores[*num_scores].name, &scores[*num_scores].score) == 2) {
    (*num_scores)++;
  }

  fclose(file);
  return scores;
};

void save_scores_to_file(const char* filename, Score* scores, int num_scores) {
  FILE* file = fopen(filename, "w");
  if (file == NULL) {
    SDL_Log("Nie można otworzyć pliku: %s", filename);
    return;
  }
    for (int i = 0; i < num_scores; i++) {
        if(scores[i].score != 0 && scores[i].name[0] != '\0') fprintf(file, "%s - %d\n", scores[i].name, scores[i].score);
    }

  fclose(file);
}

void check_and_update_ranking(const char* filename, Segment_Snake* snake, SDL_Surface* screen, SDL_Event event, SDL_Surface* charset, SDL_Texture* scrtex, SDL_Renderer* renderer) {
    int num_scores;
    int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
    Score* scores = load_scores_from_file(filename, &num_scores);
    if (scores == NULL) {
        return;
    }

    int new_score_index = -1;

    if (num_scores < 3) {
        for (int i = 0; i < num_scores; i++) {
            if (snake[0].pkt > scores[i].score) {
            new_score_index = i;
            break;
            }
        }
        if(new_score_index == -1) new_score_index = num_scores;
    } else {
        for (int i = 0; i < num_scores; i++) {
            if (snake[0].pkt > scores[i].score) {
            new_score_index = i;
            break;
            }
        }
    }

    if (new_score_index != -1) {
        char text1[128];
        char text2[128];
        sprintf(text1, "You've beat one of the best scores!!!!");
        DrawString(screen, screen->w / 2 - strlen(text1) * 8 / 2, 200, text1, charset);
        sprintf(text2, "Enter your nickname (max 20 letters) and confirm by pressing the 'Enter' key");
        DrawString(screen, screen->w / 2 - strlen(text2) * 8 / 2, 220, text2, charset);
        SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
        SDL_RenderCopy(renderer, scrtex, NULL, NULL);
        SDL_RenderPresent(renderer);

        char inputText[21]; 
        int size = 0;
        bool running = true;

        SDL_StartTextInput();
        SDL_Event e;
        while(running){
            while(SDL_PollEvent(&e)){
                if(e.type == SDL_TEXTINPUT){
                if(size<20){
                    inputText[size] = e.text.text[0];
                    size++;
                    inputText[size] = '\0';
                }
                }
                else if(e.type == SDL_KEYDOWN){
                if(e.key.keysym.sym == SDLK_BACKSPACE && size>0){
                    size--;
                    inputText[size] = '\0'; 
                }else if(e.key.keysym.sym == SDLK_KP_ENTER || e.key.keysym.sym == SDLK_RETURN){
                    running = false;
                    break;
                }
                }
            };
            SDL_FillRect(screen, NULL, czarny);
            DrawString(screen, screen->w / 2 - strlen(text1) * 8 / 2, 200, text1, charset);
            DrawString(screen, screen->w / 2 - strlen(text2) * 8 / 2, 220, text2, charset); 
            DrawString(screen, screen->w / 2 - strlen(inputText) * 8 / 2, 300, inputText, charset);
            SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
            SDL_RenderCopy(renderer, scrtex, NULL, NULL);
            SDL_RenderPresent(renderer);
        };
        SDL_StopTextInput();
            
        if (num_scores == 3 && new_score_index < num_scores) {
            for (int i = num_scores - 1; i > new_score_index; i--) scores[i] = scores[i - 1];
        }
        else if (num_scores == 1 && new_score_index == 0) scores[1] = scores[0];
        else if(num_scores == 2 && new_score_index == 0) {
            scores[2] = scores[1];
            scores[1] = scores[0];
        }
        else if(num_scores == 2 && new_score_index == 1) scores[2] = scores[1];

        scores[new_score_index].score = snake[0].pkt;
        strcpy(scores[new_score_index].name, inputText);

        if(num_scores < 3) num_scores++;

        save_scores_to_file(filename, scores, num_scores);
    };
  free(scores);
};

void show_ranking(SDL_Surface* screen, SDL_Surface* charset) {
    char text[128];
    int num_scores;
    int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
    int bialy = SDL_MapRGB(screen->format, 0xFF, 0xFF, 0xFF);

    Score* scores = load_scores_from_file("wyniki.txt", &num_scores);
    if (scores == NULL) {
        return;
    }

    DrawRectangle(screen, SCREEN_WIDTH/2 - 104, SCREEN_HEIGHT/ 2 + 10, 140, 90, bialy, czarny);
    sprintf(text, "BEST SCORES!");
    DrawString(screen, SCREEN_WIDTH/2 - 99, SCREEN_HEIGHT/ 2 + 20, text, charset);
    sprintf(text, "1.");
    DrawString(screen, SCREEN_WIDTH/2 - 99, SCREEN_HEIGHT/ 2 + 40, text, charset);
    sprintf(text, "2.");
    DrawString(screen, SCREEN_WIDTH/2 - 99, SCREEN_HEIGHT/ 2 + 60, text, charset);
    sprintf(text, "3.");
    DrawString(screen, SCREEN_WIDTH/2 - 99, SCREEN_HEIGHT/ 2 + 80, text, charset);

    for (int i = 0; i < num_scores; i++) {
        sprintf(text, "%s - %d", scores[i].name, scores[i].score);
        DrawString(screen, SCREEN_WIDTH/2 - 84, SCREEN_HEIGHT/ 2 + 40 + i * 20, text, charset);
    }
    free(scores);
};

//----------------->
//Konfiguracja począkowa mapy ----------------->

int displayMenu(SDL_Surface *screen, SDL_Surface *charset, SDL_Texture *scrtex, SDL_Renderer *renderer) {
    int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
    SDL_Event event;

    DrawString(screen, screen->w / 2 - 64, screen->h / 2 - 20, "1. Press ESC to leave", charset);
    DrawString(screen, screen->w / 2 - 104, screen->h / 2, "2. Press N to start new game", charset);
    show_ranking(screen, charset);

    SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
    SDL_RenderCopy(renderer, scrtex, NULL, NULL);
    SDL_RenderPresent(renderer);

    while(SDL_PollEvent(&event)) {
        switch(event.type) {
            case SDL_KEYDOWN:
                if(event.key.keysym.sym == SDLK_ESCAPE) return 1;
                else if(event.key.keysym.sym == SDLK_n) return 2;
                break;
            case SDL_QUIT:
                return 1;
        };
    }

    return 0;
};

int menu (SDL_Surface *screen,  SDL_Surface *charset, SDL_Texture *scrtex, SDL_Renderer *renderer) {
    int menu_result = displayMenu(screen, charset, scrtex, renderer);
    if (menu_result == 1) {
        return 1;
    }
    else if (menu_result == 2) {
        return 2;
    }
    return 0;
};

void drawGrid(SDL_Surface *screen) {
    int bialy = SDL_MapRGB(screen->format, 0xFF, 0xFF, 0xFF);
    int i;

    int startX = (SCREEN_WIDTH - GRID_SIZE * GRID_LINE_SPACING) / 2;
    int startY = (SCREEN_HEIGHT - GRID_SIZE * GRID_LINE_SPACING) / 2;

    for (i = 0; i <= GRID_SIZE; i++) {
        DrawLine(screen, startX + i * GRID_LINE_SPACING, startY, GRID_SIZE * GRID_LINE_SPACING, 0, 1, bialy);
    }

    for (i = 0; i <= GRID_SIZE; i++) {
        DrawLine(screen, startX, startY + i * GRID_LINE_SPACING, GRID_SIZE * GRID_LINE_SPACING, 1, 0, bialy);
    }
};

double getTime(double t1) {
    static double czas1;
    double delta, czas2 = SDL_GetTicks();
    if(czas2 == t1) {
        delta = 0;
        czas1 = 0;
        czas1 = t1;
        delta = (czas2 - czas1) * 0.001;
        czas1 = czas2;
    }
    else {
        delta = (czas2 - czas1) * 0.001;
        czas1 = czas2;
    }
    return delta;
};

void draw_mandatory_opt(SDL_Surface *screen, SDL_Surface *charset) {
    char text[128];
    int ciemnozielony = SDL_MapRGB(screen->format, 0x00, 0x64, 0x00); 
    DrawRectangle(screen, SCREEN_WIDTH - 70, (SCREEN_HEIGHT - GRID_SIZE * GRID_LINE_SPACING) / 2, 60, GRID_LINE_SPACING*GRID_SIZE, ciemnozielony, ciemnozielony);
    sprintf(text, "1");
    DrawString(screen, SCREEN_WIDTH - 45, (SCREEN_HEIGHT - GRID_SIZE * GRID_LINE_SPACING) / 2 + 20, text, charset);
    sprintf(text, "2");
    DrawString(screen, SCREEN_WIDTH - 45, (SCREEN_HEIGHT - GRID_SIZE * GRID_LINE_SPACING) / 2 + 40, text, charset);
    sprintf(text, "3");
    DrawString(screen, SCREEN_WIDTH - 45, (SCREEN_HEIGHT - GRID_SIZE * GRID_LINE_SPACING) / 2 + 60, text, charset);
    sprintf(text, "4");
    DrawString(screen, SCREEN_WIDTH - 45, (SCREEN_HEIGHT - GRID_SIZE * GRID_LINE_SPACING) / 2 + 80, text, charset);
    sprintf(text, "A");
    DrawString(screen, SCREEN_WIDTH - 45, (SCREEN_HEIGHT - GRID_SIZE * GRID_LINE_SPACING) / 2 + 100, text, charset);
    sprintf(text, "B");
    DrawString(screen, SCREEN_WIDTH - 45, (SCREEN_HEIGHT - GRID_SIZE * GRID_LINE_SPACING) / 2 + 120, text, charset);
    sprintf(text, "C");
    DrawString(screen, SCREEN_WIDTH - 45, (SCREEN_HEIGHT - GRID_SIZE * GRID_LINE_SPACING) / 2 + 140, text, charset);
    sprintf(text, "D");
    DrawString(screen, SCREEN_WIDTH - 45, (SCREEN_HEIGHT - GRID_SIZE * GRID_LINE_SPACING) / 2 + 160, text, charset);
    sprintf(text, "F");
    DrawString(screen, SCREEN_WIDTH - 45, (SCREEN_HEIGHT - GRID_SIZE * GRID_LINE_SPACING) / 2 + 180, text, charset);
};

void drawinfo(SDL_Surface *screen, SDL_Surface *charset, Game_Tools* narzedzia, Segment_Snake* snake) {
    double czas_do_wyswietlenia = narzedzia->worldTime;
    char text[128];
    int ciemnozielony = SDL_MapRGB(screen->format, 0x00, 0x64, 0x00); 

    if (screen != NULL) { 
        DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 52, ciemnozielony, ciemnozielony);
        sprintf(text, "Gra Snake, , czas trwania = %.1lf s | %d PKT", czas_do_wyswietlenia, snake[0].pkt);
        DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
        sprintf(text, "Esc - Exit, N - New Game, \030 - Go Up");
        DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);
        sprintf(text, "\031 - Go Down, \033 - Go Right, \032 - Go Left");
        DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 42, text, charset);
        draw_mandatory_opt(screen, charset);
    } else {
        printf("Błąd: screen nie jest zainicjalizowany!\n");
    }
};

void initSnake(SDL_Surface* screen, Segment_Snake* snake) {
    for (int i = 0; i < SNAKE_LENGTH; i++) {
        snake[i].x = ((SCREEN_WIDTH )/2) + ((GRID_LINE_SPACING - SNAKE_SIZE)/2) + 1;
        snake[i].y = (SCREEN_HEIGHT + 60*i)/2 + ((GRID_LINE_SPACING - SNAKE_SIZE)/2) + 1;
        snake[i].size = SNAKE_SIZE;
        snake[i].kierunek_weza = 1;
        snake[i].dlugosc = SNAKE_LENGTH;
        snake[i].pkt = 0;
        snake[i].speed = SNAKE_STARTING_SPEED;
    }
};

void inicjalizacja_snake(SDL_Surface *screen, SDL_Texture *scrtex, SDL_Renderer *renderer, Segment_Snake* snake) {
    initSnake(screen, snake);
    SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
    SDL_RenderCopy(renderer, scrtex, NULL, NULL);
    SDL_RenderPresent(renderer);
};

//----------------->
//ruch_weża ----------------->

void auto_pelzanko(Segment_Snake* snake, SDL_Surface* screen) {
    for(int i = snake[0].dlugosc; i>=1; i--) {
        snake[i].x = snake[i-1].x;
        snake[i].y = snake[i-1].y;
    };
    if(snake[0].kierunek_weza == 1) { //gora
        if(snake[0].y - 30 < (SCREEN_HEIGHT - GRID_SIZE * GRID_LINE_SPACING) / 2) {
            snake[0].kierunek_weza = 3;
            if(snake[0].x + 30 > (SCREEN_WIDTH - GRID_SIZE * GRID_LINE_SPACING) / 2 + GRID_LINE_SPACING*GRID_SIZE) snake[0].kierunek_weza = 4;
        }
    }
    else if(snake[0].kierunek_weza == 2) { //dol
        if(snake[0].y + 30 > (SCREEN_HEIGHT - GRID_SIZE * GRID_LINE_SPACING) / 2 + GRID_LINE_SPACING*GRID_SIZE) {
            snake[0].kierunek_weza = 4;
            if(snake[0].x - 30 < (SCREEN_WIDTH - GRID_SIZE * GRID_LINE_SPACING) / 2 ) snake[0].kierunek_weza = 3;
        }
    }
    else if(snake[0].kierunek_weza == 3) { //prawo
        if(snake[0].x + 30 > (SCREEN_WIDTH - GRID_SIZE * GRID_LINE_SPACING) / 2 + GRID_LINE_SPACING*GRID_SIZE) {
            snake[0].kierunek_weza = 2;
            if(snake[0].y + 30 > (SCREEN_HEIGHT - GRID_SIZE * GRID_LINE_SPACING) / 2 + GRID_LINE_SPACING*GRID_SIZE) snake[0].kierunek_weza = 1;
        }
    }
    else if(snake[0].kierunek_weza == 4) { //lewo
        if(snake[0].x - 30 < (SCREEN_WIDTH - GRID_SIZE * GRID_LINE_SPACING) / 2) {
            snake[0].kierunek_weza = 1;
            if(snake[0].y - 30 < (SCREEN_HEIGHT - GRID_SIZE * GRID_LINE_SPACING) / 2) snake[0].kierunek_weza = 2;
        }
    }
    if(snake[0].kierunek_weza == 1) snake[0].y -= 30;
    if(snake[0].kierunek_weza == 2) snake[0].y += 30;
    if(snake[0].kierunek_weza == 3) snake[0].x += 30;
    if(snake[0].kierunek_weza == 4) snake[0].x -= 30;
};

bool check_collision(Segment_Snake* snake) {
    for(int i = 2; i< snake[0].dlugosc; i++) {
        if(snake[0].x == snake[i].x && snake[0].y == snake[i].y) return true;
    }
    return false;
};

int auto_ruch(Segment_Snake* snake, SDL_Surface* screen) {
    switch (snake[0].kierunek_weza)
    {
    case 1:
        auto_pelzanko(snake, screen);
        if(check_collision(snake) == true) return 2;
        break;
    case 2:
        auto_pelzanko(snake, screen);
        if(check_collision(snake) == true) return 2;
        break;
    case 3:
        auto_pelzanko(snake, screen);
        if(check_collision(snake) == true) return 2;
        break;
    case 4:
        auto_pelzanko(snake, screen);
        if(check_collision(snake) == true) return 2;
        break;
    }
    return 0; 
};

int ruch_snake(SDL_Surface* screen, SDL_Event event, Segment_Snake* snake, Game_Tools* narzedzia) {
    static double czas_do_ruchu = narzedzia->worldTime;
    static double czas_do_zwieksz = narzedzia->worldTime;
    double czas_aktualny = SDL_GetTicks(), czas_aktualnej_pred = SDL_GetTicks();
    while(SDL_PollEvent(&event)){
        switch (event.type) {
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE) return 2;
                if (event.key.keysym.sym == SDLK_n) return 3;
                else if (event.key.keysym.sym == SDLK_UP && snake[0].kierunek_weza != 2) {
                    snake[0].kierunek_weza = 1;
                    break;
                }
                else if (event.key.keysym.sym == SDLK_DOWN && snake[0].kierunek_weza != 1) {
                    snake[0].kierunek_weza = 2;
                    break;
                }
                else if (event.key.keysym.sym == SDLK_RIGHT && snake[0].kierunek_weza != 4) {
                    snake[0].kierunek_weza = 3;
                    break;
                }
                else if (event.key.keysym.sym == SDLK_LEFT && snake[0].kierunek_weza != 3) {
                    snake[0].kierunek_weza = 4;
                    break;
                };
            case SDL_KEYUP:
                break;
            case SDL_QUIT:
                return 2;
        };
    };
    if((czas_aktualnej_pred - czas_do_zwieksz) * 0.001 >= TIME_INTERVAL) {
        czas_do_zwieksz = czas_aktualnej_pred;
        snake[0].speed -= SPEED_UP*snake[0].speed;
    }
    if((czas_aktualny - czas_do_ruchu) * 0.001 >= snake[0].speed) {
        czas_do_ruchu = czas_aktualny;
        if(auto_ruch(snake, screen) == 2) return 4;
    }
    return 0;
};

int mov(SDL_Surface* screen, SDL_Event event, Segment_Snake* snake, Game_Tools* narzedzia) {
    int ruch = ruch_snake(screen, event, snake, narzedzia);
    if(ruch == 2) return 1;
    else if(ruch == 3) {
        narzedzia->flaga_do_new_game = true;
    }
    else if(ruch == 4) {
        narzedzia->flaga_do_menu = true;
        narzedzia->flaga_do_new_game = true;
    };
    return 0;
};

//----------------->
//dodatki do gry----------------->

void blue_dot(Blue_dot* jablko, SDL_Surface* screen, Red_dot* power_up) {
    int x = rand()%GRID_SIZE;
    int y = rand()%GRID_SIZE;
    do {
        x = rand()%GRID_SIZE;
        y = rand()%GRID_SIZE;
    } while (jablko->x == x && jablko->y == y);
    jablko->x = ((SCREEN_WIDTH - GRID_SIZE * GRID_LINE_SPACING) / 2) + x*GRID_LINE_SPACING + 1;
    jablko->y = ((SCREEN_HEIGHT - GRID_SIZE * GRID_LINE_SPACING) / 2) + y*GRID_LINE_SPACING + 1;
    jablko->flaga = 1;
};

void red_dot(Red_dot* power_up, SDL_Surface* screen, Blue_dot* jablko, Segment_Snake* snake) {
    int x = rand()%GRID_SIZE;
    int y = rand()%GRID_SIZE;
    bool flaga = false;
    do {
        x = rand()%GRID_SIZE;
        y = rand()%GRID_SIZE;
    } while (jablko->x == x && jablko->y == y);
    while(flaga == false) {
        int i = 0;
        while(i < snake[0].dlugosc) {
            if(snake[i].x == x && snake[i].y == y) {
                x = rand()%GRID_SIZE;
                y = rand()%GRID_SIZE;
                break;
            };
            i++;
            if(i == snake[0].dlugosc - 1) flaga = true;
        };
    };
    power_up->x = ((SCREEN_WIDTH - GRID_SIZE * GRID_LINE_SPACING) / 2) + x*GRID_LINE_SPACING + 1;
    power_up->y = ((SCREEN_HEIGHT - GRID_SIZE * GRID_LINE_SPACING) / 2) + y*GRID_LINE_SPACING + 1;
    power_up->flaga = 1;
    power_up->czas_od_poj = SDL_GetTicks();
    power_up->czas_do_poj = 0;
};

void sprawdz_kolizje_jablko(Segment_Snake* snake, Blue_dot* jablko) {
    int i = 0;
    while(i < snake[0].dlugosc) {
        if (snake[i].x - 6 == jablko->x && snake[i].y - 6 == jablko->y) break;
        if(i == snake[0].dlugosc - 1) return;
        i++;
    };
    snake[0].pkt = snake[0].pkt + POINTS_FOR_BALLZ;
    int NewSize = snake[0].dlugosc + 1;
    snake[0].dlugosc = NewSize; 
    jablko->flaga = 0;
    int LastIndex = snake[0].dlugosc-1;
    snake[LastIndex].size = SNAKE_SIZE;
    snake[LastIndex].kierunek_weza = 1;
    if(snake[LastIndex].x - snake[LastIndex-2].x < 0) snake[LastIndex].x = snake[LastIndex-1].x - 30;
    if(snake[LastIndex-1].x - snake[LastIndex-2].x > 0) snake[LastIndex].x = snake[LastIndex-1].x + 30;
    if(snake[LastIndex-1].x == snake[LastIndex-2].x) snake[LastIndex].x = snake[LastIndex-1].x;
    if(snake[LastIndex-1].y - snake[LastIndex-2].y < 0) snake[LastIndex].y = snake[LastIndex-1].y - 30;
    if(snake[LastIndex-1].y - snake[LastIndex-2].y > 0) snake[LastIndex].y = snake[LastIndex-1].y + 30;
    if(snake[LastIndex-1].y == snake[LastIndex-2].y) snake[LastIndex].y = snake[LastIndex-1].y; 
};

void sprawdz_kolizje_mocy(Segment_Snake* snake, Red_dot* power_up) {
    int i = 0, chance;
    while(i < snake[0].dlugosc) {
        if (snake[i].x - 6 == power_up->x && snake[i].y - 6 == power_up->y) break;
        if(i == snake[0].dlugosc - 1) return;
        i++;
    };
    snake[0].pkt = snake[0].pkt + POINTS_FOR_BALLZ;
    power_up->x = 0;
    power_up->y = 0;
    power_up->flaga = 0;
    chance = rand()%2 + 1;
    if(snake[0].dlugosc <= SNAKE_LENGTH) chance = 1;
    if(snake[0].speed >= SNAKE_STARTING_SPEED) chance = 2;
    if(snake[0].dlugosc <= SNAKE_LENGTH && snake[0].speed >= SNAKE_STARTING_SPEED) return;
    if(chance == 2) snake[0].dlugosc -= SHORTENING; 
    else snake[0].speed += snake[0].speed*MOV_SLOWER;
};

void wspomagacze(Segment_Snake* snake, Blue_dot* jablko, SDL_Surface* screen, Red_dot* power_up) {
    static double procent = 0;
    double czas_aktualny = SDL_GetTicks();
    int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);
    int czerwony = SDL_MapRGB(screen->format, 0xCC, 0x11, 0x11);
    int bialy = SDL_MapRGB(screen->format, 0xFF, 0xFF, 0xFF);
    if(power_up->czas_do_poj == 0) {
        int losowosc = rand()%8 + 1;
        power_up->czas_do_poj = losowosc + WAIT_FOR_DOT;
    }
    if(jablko->flaga == 0) blue_dot(jablko, screen, power_up);
    if(power_up->flaga == 0) {
        if((czas_aktualny - power_up->czas_od_poj) * 0.001 > power_up->czas_do_poj)  red_dot(power_up, screen, jablko, snake); 
    }
    DrawRectangle(screen, jablko->x + 10, jablko->y + 10, 9, 9, niebieski, niebieski);
    if(power_up->flaga == 1) DrawRectangle(screen, power_up->x + 10, power_up->y + 10, 9, 9, czerwony, czerwony);
    sprawdz_kolizje_jablko(snake, jablko);
    sprawdz_kolizje_mocy(snake, power_up);
    if(power_up->flaga == 1) {
        if((czas_aktualny - power_up->czas_od_poj)*0.001 <= WAIT_FOR_DOT) {
            int pocz_x = (SCREEN_WIDTH - GRID_SIZE * GRID_LINE_SPACING) / 2 + 20;
            int pocz_y = (SCREEN_HEIGHT - GRID_SIZE * GRID_LINE_SPACING) / 2 + GRID_LINE_SPACING*GRID_SIZE + 20;
            int dlugosc = (SCREEN_WIDTH - GRID_SIZE * GRID_LINE_SPACING) / 2 + GRID_LINE_SPACING*GRID_SIZE - (SCREEN_WIDTH - GRID_SIZE * GRID_LINE_SPACING) / 2 - 40;
            procent = (czas_aktualny - power_up->czas_od_poj)/10;
            DrawRectangle(screen, pocz_x, pocz_y, dlugosc, 20, bialy, bialy);
            DrawRectangle(screen, pocz_x + 1, pocz_y + 1, dlugosc - 2 - procent, 18, czerwony, czerwony);
        }
        if((czas_aktualny - power_up->czas_od_poj)*0.001 > WAIT_FOR_DOT) {
            power_up->flaga = 0;
            procent = 0;
            power_up->x = 0;
            power_up->y = 0;
        }
    }
};


//----------------->
//Main Game----------------->

void tools_ini(Game_Tools* narzedzia) {
    narzedzia->quit = 0;
    narzedzia->worldTime = 0;
    narzedzia->flaga_do_menu = true;
    narzedzia->flaga_do_new_game = true;
};

void main_game(SDL_Surface* screen, SDL_Event event, SDL_Surface* charset, SDL_Texture* scrtex, SDL_Window* window, SDL_Renderer* renderer, Blue_dot* jablko, Segment_Snake* snake, Game_Tools* narzedzia, Red_dot* power_up) {
    int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
    int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
    int t1, wstep;
    bool czy_pierwszy_raz = true, juz_zrobione = false;
    narzedzia = (struct Game_Tools*)malloc(sizeof(struct Game_Tools));
    tools_ini(narzedzia);
    while (narzedzia->quit == 0) {
        SDL_FillRect(screen, NULL, czarny);
        while(narzedzia->flaga_do_menu == true) {
            if(czy_pierwszy_raz == false && juz_zrobione == false) {
                check_and_update_ranking("wyniki.txt", snake, screen, event, charset, scrtex, renderer);
                juz_zrobione = true;
                SDL_FillRect(screen, NULL, czarny);
            }
            wstep = menu(screen, charset, scrtex, renderer);
            if(wstep == 1) return;
            else if(wstep == 2 ) narzedzia->flaga_do_menu = false;
        };
        if(narzedzia->flaga_do_new_game == true) {
            if(juz_zrobione == false && czy_pierwszy_raz == false) {
                check_and_update_ranking("wyniki.txt", snake, screen, event, charset, scrtex, renderer);
            }
            if(snake != NULL) free(snake);
            if(jablko != NULL) free(jablko);
            snake = (struct Segment_Snake*)malloc( (GRID_SIZE*GRID_SIZE) * sizeof(struct Segment_Snake));
            jablko = (struct Blue_dot*)malloc(sizeof(struct Blue_dot));
            power_up = (struct Red_dot*)malloc(sizeof(struct Red_dot));
            SDL_FillRect(screen, NULL, czarny);
            inicjalizacja_snake(screen, scrtex, renderer, snake);
            blue_dot(jablko,screen, power_up);
            red_dot(power_up, screen, jablko, snake);
            narzedzia->flaga_do_new_game = false;
            narzedzia->worldTime = 0;
            t1 = SDL_GetTicks();
            czy_pierwszy_raz = false;
            juz_zrobione = false;
        }
        narzedzia->worldTime += getTime(t1);
        drawinfo(screen, charset, narzedzia, snake);
        drawGrid(screen);
        if(mov(screen ,event, snake, narzedzia) == 1) return;
        wspomagacze(snake, jablko, screen, power_up);
        for(int j = 0; j < snake[0].dlugosc; j++) {
            DrawRectangle(screen, snake[j].x, snake[j].y, snake[j].size, snake[j].size, zielony, zielony);
        };
        SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);
    };
};

//----------------->
//funkcja główna----------------->


int main(int argc, char **argv) {
    int rc;
    SDL_Event event;
    SDL_Surface *screen, *charset;
    SDL_Texture *scrtex;
    SDL_Window *window;
    SDL_Renderer *renderer;
    srand(time(NULL));

    if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("SDL_Init error: %s\n", SDL_GetError());
        return 1;
    };

    charset = SDL_LoadBMP("./cs8x8.bmp");
    if(charset == NULL) {
        printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    };
    SDL_SetColorKey(charset, true, 0x000000);
    
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    SDL_SetWindowTitle(window, "Snaker");

    rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);
    screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

    struct Segment_Snake* snake = NULL;
    struct Blue_dot* jablko = NULL;
    struct Game_Tools* narzedzia = NULL;
    struct Red_dot* power_up = NULL;
    

    main_game(screen, event, charset, scrtex, window, renderer, jablko, snake, narzedzia, power_up);

    free(narzedzia);
    free(jablko);
    free(snake);
    SDL_FreeSurface(charset);
    SDL_FreeSurface(screen);
    SDL_DestroyTexture(scrtex);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
};