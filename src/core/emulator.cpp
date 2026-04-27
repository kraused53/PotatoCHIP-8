#include <core/emulator.h>

#include <CLogg.h>
#include <fstream>
#include <format>

Emulator::Emulator() {
  CLogg::set_priority( TRACE );
  initialized = false;
  running = false;

  if( SDL_Init( SDL_INIT_EVERYTHING ) ) {
    CLogg::Fatal( "SDL Subsystems Failed To Initialize" );
    return;
  }
  CLogg::Debug( "SDL Subsystems Initialized" );

  window = SDL_CreateWindow( 
    "CHIP 8",
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    window_width, window_height,
    SDL_WINDOW_SHOWN
  );
  if( !window ) {
    CLogg::Fatal( "Could not create sdl window" );
    return;
  }
  CLogg::Debug( "SDL window created" );

  renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED );
  if( !renderer ) {
    CLogg::Fatal( "Could not create sdl renderer" );
    return;
  }
  CLogg::Debug( "SDL renderer created" );

  texture = SDL_CreateTexture(
    renderer,
    SDL_PIXELFORMAT_RGBA8888, 
    SDL_TEXTUREACCESS_STREAMING, 
    texture_width, texture_height
  );
  if( !texture ) {
    CLogg::Fatal( "Could not create sdl texture" );
    return;
  }
  CLogg::Debug( "SDL texture created" );

  CLogg::Debug( "Emulator core established..." );
  initialized = true;
}

Emulator::~Emulator() {
  SDL_DestroyTexture( texture );
  SDL_DestroyRenderer( renderer );
  SDL_DestroyWindow( window );
  SDL_Quit();
  CLogg::Debug( "Emulator core destroyed..." );
}

void Emulator::run( std::string rompath ) {
  // Set up the emulator
  setup( rompath );

  // If setup does not fail, running will be true
  while( running ) {
    input();
    update();
    render();
  }
}

// Load rom and initialize emulator state
void Emulator::setup( std::string rompath ) {
  // Stop emulator
  running = false;

  // Reset chip 8
  chip.reset();

  // Load rom into memory
  if( !chip.load_rom( rompath ) ) {
    return;
  }

  // Start emulator
  running = true;
}


void Emulator::key_press( SDL_Event *e ) {
  switch( e->key.keysym.sym ) {
    case SDLK_ESCAPE: running = false; break;
    case SDLK_1: chip.set_key( 0x1 ); break;
    case SDLK_2: chip.set_key( 0x2 ); break;
    case SDLK_3: chip.set_key( 0x3 ); break;
    case SDLK_4: chip.set_key( 0xC ); break;
    case SDLK_q: chip.set_key( 0x4 ); break;
    case SDLK_w: chip.set_key( 0x5 ); break;
    case SDLK_e: chip.set_key( 0x6 ); break;
    case SDLK_r: chip.set_key( 0xD ); break;
    case SDLK_a: chip.set_key( 0x7 ); break;
    case SDLK_s: chip.set_key( 0x8 ); break;
    case SDLK_d: chip.set_key( 0x9 ); break;
    case SDLK_f: chip.set_key( 0xE ); break;
    case SDLK_z: chip.set_key( 0xA ); break;
    case SDLK_x: chip.set_key( 0x0 ); break;
    case SDLK_c: chip.set_key( 0xB ); break;
    case SDLK_v: chip.set_key( 0xF ); break;
  }
}

void Emulator::key_release( SDL_Event *e ) {
  switch( e->key.keysym.sym ) {
    case SDLK_1: chip.clear_key( 0x1 ); break;
    case SDLK_2: chip.clear_key( 0x2 ); break;
    case SDLK_3: chip.clear_key( 0x3 ); break;
    case SDLK_4: chip.clear_key( 0xC ); break;
    case SDLK_q: chip.clear_key( 0x4 ); break;
    case SDLK_w: chip.clear_key( 0x5 ); break;
    case SDLK_e: chip.clear_key( 0x6 ); break;
    case SDLK_r: chip.clear_key( 0xD ); break;
    case SDLK_a: chip.clear_key( 0x7 ); break;
    case SDLK_s: chip.clear_key( 0x8 ); break;
    case SDLK_d: chip.clear_key( 0x9 ); break;
    case SDLK_f: chip.clear_key( 0xE ); break;
    case SDLK_z: chip.clear_key( 0xA ); break;
    case SDLK_x: chip.clear_key( 0x0 ); break;
    case SDLK_c: chip.clear_key( 0xB ); break;
    case SDLK_v: chip.clear_key( 0xF ); break;
  }
}

// Gather user input
void Emulator::input() {
  SDL_Event e;
  while( SDL_PollEvent( &e ) ) {
    switch( e.type ) {
      case    SDL_QUIT:   running = false; break;
      case SDL_KEYDOWN:   key_press( &e ); break;
      case   SDL_KEYUP: key_release( &e ); break;
    }
  }
}

// Step the emulator
void Emulator::update() {

  const uint64_t start_time = SDL_GetPerformanceCounter();
  
  // ~11 opcodes per 60FPS frame
  for( int i = 0; i < 11; i++ ) {
    chip.step();
  }

  chip.timers();
  
  const uint64_t end_time = SDL_GetPerformanceCounter();

  // 60 FPS ~16.6ms
  const double dt = (double) (( end_time - start_time ) * 1000 ) / SDL_GetPerformanceFrequency();
  
  if( 16.67f > dt ) {
    SDL_Delay( 16.67f - dt );
  }
}

// Render emulator state
void Emulator::render() {
  SDL_UpdateTexture(texture, nullptr, &chip.video, video_pitch);
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, nullptr, nullptr);
  SDL_RenderPresent(renderer);
}