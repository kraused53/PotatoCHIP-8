#pragma once

#include <core/CHIP.H>

#include <SDL.h>
#include <string>

class Emulator{
  private:
    // System
    CHIP chip;

    // Emulation
    bool initialized;
    bool running;

    // Graphics
    int video_scale = 10;
    int chip_width = 64;
    int chip_height = 32;
    int window_width = chip_width * video_scale;
    int window_height = chip_height * video_scale;
    int texture_width = chip_width;
    int texture_height = chip_height;
    int video_pitch = sizeof( chip.video[0] ) * chip_width;
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;

  public:
    Emulator();
    ~Emulator();

    // Core functions
    void run( std::string rompath );
    void setup( std::string rompath );
    void key_press( SDL_Event *e );
    void key_release( SDL_Event *e );
    void input();
    void update();
    void render();

    bool is_initialized( void ) { return initialized; }
};