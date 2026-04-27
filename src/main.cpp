#include <core/emulator.H>

int main( int argc, char** argv ) {


  Emulator system;

  if( !system.is_initialized() ) {
    return EXIT_FAILURE;
  }

  //system.run( "roms/tests/1-chip8-logo.ch8" );
  //system.run( "roms/tests/2-ibm-logo.ch8" );
  //system.run( "roms/tests/3-corax+.ch8" );
  //system.run( "roms/tests/4-flags.ch8" );
  //system.run( "roms/tests/6-keypad.ch8" );
  //system.run( "roms/tests/8-beep.ch8" );

  system.run( "roms/Tetris.ch8" );
  
  return EXIT_SUCCESS;
}