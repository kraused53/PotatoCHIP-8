#include "core/CHIP.h"

#include <CLogg.h>
#include <format>
#include <fstream>
#include <vector>

uint8_t fontset[FONTSET_SIZE] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

CHIP::CHIP() {
  //CLogg::enable_file_output( "logs/log.txt" );
  CLogg::Debug( "Setting up CHIP..." );
  srand( time( nullptr ) );
}

CHIP::~CHIP() {
  CLogg::Debug( "Tearing down CHIP..." );
}

void CHIP::load_font() {
  for( int i = 0; i < FONTSET_SIZE; i++ ) {
    memory[FONT_ADDR + i] = fontset[i];
  }
}

void CHIP::clear_video() { video.fill( 0x00000000 ); }

bool CHIP::load_rom( std::string rompath ) {

  CLogg::Debug( std::format( "Reading file: {:s}", rompath ).c_str() );

  std::ifstream file( rompath, std::ios::binary | std::ios::ate );

  if( !file.is_open() ) {
    CLogg::Error( "Could not open file!" );
    return false;
  }

  std::streampos size = file.tellg();
  file.seekg(0, std::ios::beg);

  if (START_ADDR + size > memory.size()) { 
    CLogg::Error( "Rom is too big for memory" );
    return false;
  }
  
  std::vector<uint8_t> buffer(size);

  if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
    CLogg::Error("Failed to read ROM");
    return false;
  }
  
  for( int i = 0; i < size; i++ ) {
    memory[ START_ADDR + i ] = buffer[i];
  }

  CLogg::Debug( "ROM Loaded into memory" );

  return true;
}

void CHIP::reset() {
  CLogg::Debug( "Resetting chip 8" );

  memory.fill( 0x00 );
  load_font();

  PC = START_ADDR;
  IP = FONT_ADDR;
  regs.fill( 0x00 );
  opcode = 0x0000;

  DT = 0x00;
  ST = 0x00;

  SP = 0x00;
  stack.fill( 0x0000 );

  keypad.fill( false );

  clear_video();
}

void CHIP::set_key( uint8_t reg ) { keypad[reg] = true; }
void CHIP::clear_key( uint8_t reg ) { keypad[reg] = false; }

void CHIP::step() {
  CLogg::Debug( diss( PC ).c_str() );

  opcode = ( memory[PC] << 8 ) | memory[PC + 1];
  PC += 2;

  switch( get_S() ) {
    case 0x0: sys_calls(); break;
    case 0x1:   OP_1nnn(); break;
    case 0x2:   OP_2nnn(); break;
    case 0x3:   OP_3xnn(); break;
    case 0x4:   OP_4xnn(); break;
    case 0x5:   OP_5xy0(); break;
    case 0x6:   OP_6xnn(); break;
    case 0x7:   OP_7xnn(); break;
    case 0x8:       ALU(); break;
    case 0x9:   OP_9xy0(); break;
    case 0xA:   OP_Annn(); break;
    case 0xC:   OP_Cxnn(); break;
    case 0xD:   OP_Dxyn(); break;
    case 0xE:   OP_Exnn(); break;
    case 0xF:      Fxnn(); break;
    default: CLogg::Error( "Unhandled OpCode!" ); break;
  }

  
}

void CHIP::timers() {
  if( DT > 0 ) { DT--; }
  if( ST > 0 ) { ST--; }
}

// Opcodes
void CHIP::sys_calls() {
  switch( get_NNN() ) {
    case 0x00E0: OP_00E0(); break;
    case 0x00EE: OP_00EE(); break;
    default: CLogg::Error( "Unhandled OpCode!" );  break;
  }
}

void CHIP::ALU() {
  switch( get_N() ) {
    case 0x0: OP_8xy0(); break;
    case 0x1: OP_8xy1(); break;
    case 0x2: OP_8xy2(); break;
    case 0x3: OP_8xy3(); break;
    case 0x4: OP_8xy4(); break;
    case 0x5: OP_8xy5(); break;
    case 0x6: OP_8xy6(); break;
    case 0x7: OP_8xy7(); break;
    case 0xE: OP_8xyE(); break;
    default: CLogg::Error( "Unhandled OpCode!" );  break;
  }
}

void CHIP::Fxnn() {
  switch ( get_NN() ) {
    case 0x07: OP_Fx07(); break;
    case 0x0A: OP_Fx0A(); break;
    case 0x15: OP_Fx15(); break;
    case 0x18: OP_Fx18(); break;
    case 0x1E: OP_Fx1E(); break;
    case 0x29: OP_Fx29(); break;
    case 0x33: OP_Fx33(); break;
    case 0x55: OP_Fx55(); break;
    case 0x65: OP_Fx65(); break;
    default: CLogg::Error( "Unhandled OpCode!" );  break;
  }
}

void CHIP::OP_Exnn() {
  switch ( get_NN() ) {
    case 0x9E: OP_Ex9E(); break;
    case 0xA1: OP_ExA1(); break;
    default: CLogg::Error( "Unhandled OpCode!" );  break;
  }
}

void CHIP::OP_00E0() { clear_video(); }

// Return from subroutine
void CHIP::OP_00EE() { 
  SP--;
  PC = stack[SP];
}

// Jump to NNN
void CHIP::OP_1nnn() { PC = get_NNN(); }

// Go to subroutine
void CHIP::OP_2nnn() { 
  stack[SP] = PC;
  SP++;

  PC = get_NNN();
}

// Skip if v[x] == nn
void CHIP::OP_3xnn() {
  if( regs[get_X()] == get_NN() ) {
    PC += 2;
  }
}

// Skip if v[x] != nn
void CHIP::OP_4xnn() {
  if( regs[get_X()] != get_NN() ) {
    PC += 2;
  }
}

// Skip if v[x] == v[y]
void CHIP::OP_5xy0() {
if( regs[get_X()] == regs[get_Y()] ) {
    PC += 2;
  }
}

void CHIP::OP_6xnn() { regs[get_X()] = get_NN(); }

void CHIP::OP_7xnn() { regs[get_X()] += get_NN(); }

// ALU Sections
// Set V[x] = V[y]
void CHIP::OP_8xy0() { regs[get_X()] = regs[get_Y()]; }

// V[X] |= V[Y]
void CHIP::OP_8xy1() { regs[get_X()] |= regs[get_Y()]; }

// V[X] &= V[Y]
void CHIP::OP_8xy2() { regs[get_X()] &= regs[get_Y()]; }

// V[X] ^= V[Y]
void CHIP::OP_8xy3() { regs[get_X()] ^= regs[get_Y()]; }

// V[X] += V[Y]
void CHIP::OP_8xy4() {
  uint16_t sum = regs[get_X()] + regs[get_Y()];

  regs[get_X()] = sum & 0xFF;

  if( sum > 255 ) {
    regs[0xF] = 0x01;
  }else {
    regs[0xF] = 0x00;
  }
}

// V[X] -= V[Y]
void CHIP::OP_8xy5() {
  uint8_t x = regs[get_X()];
  uint8_t y = regs[get_Y()];
  
  regs[get_X()] -= regs[get_Y()];
  
  if( x >= y ) {
    regs[0xF] = 0x01;
  }else {
    regs[0xF] = 0x00;
  }
}

// SHR V[X]
void CHIP::OP_8xy6() {
  uint8_t x = regs[get_X()];

  regs[get_X()] = x >> 1;
  
  if( x & 0x01 ) {
    regs[0xF] = 0x01;
  }else {
    regs[0xF] = 0x00;
  }
}

// SUBN V[x], V[Y]
void CHIP::OP_8xy7() {
  uint8_t x = regs[get_X()];
  uint8_t y = regs[get_Y()];

  regs[get_X()] = y - x;

  if( y >= x ) {
    regs[0xF] = 0x01;
  }else {
    regs[0xF] = 0x00;
  }
}

// SHL V[X]
void CHIP::OP_8xyE() {
  uint8_t x = regs[get_X()];

  regs[get_X()] = x << 1;
  
  if( x & 0x80 ) {
    regs[0xF] = 0x01;
  }else {
    regs[0xF] = 0x00;
  }
}

// Skip if v[x] != v[y]
void CHIP::OP_9xy0() {
  if( regs[get_X()] != regs[get_Y()] ) {
    PC += 2;
  }
}

void CHIP::OP_Annn() { IP = get_NNN(); }

void CHIP::OP_Cxnn() {
  uint8_t rand_gen = (uint8_t) rand() % 256;
  regs[get_X()] = rand_gen & get_NN();
}

void CHIP::OP_Dxyn() {
  uint8_t Vx = get_X();
	uint8_t Vy = get_Y();
	uint8_t height = get_N();
  int VIDEO_WIDTH = 64;
  int VIDEO_HEIGHT = 32;

	// Wrap if going beyond screen boundaries
	uint8_t xPos = regs[Vx] % VIDEO_WIDTH;
	uint8_t yPos = regs[Vy] % VIDEO_HEIGHT;

	regs[0xF] = 0;

	for (unsigned int row = 0; row < height; ++row) {
		uint8_t spriteByte = memory[IP + row];

		for (unsigned int col = 0; col < 8; ++col) {
			uint8_t spritePixel = spriteByte & (0x80u >> col);
			uint32_t* screenPixel = &video[
        ( ( yPos + row ) % VIDEO_HEIGHT ) * VIDEO_WIDTH +
        ( ( xPos + col ) % VIDEO_WIDTH )
      ];

			// Sprite pixel is on
			if (spritePixel) {
				// Screen pixel also on - collision
				if (*screenPixel == 0xFFFFFFFF) {
					regs[0xF] = 1;
				}

				// Effectively XOR with the sprite pixel
				*screenPixel ^= 0xFFFFFFFF;
			}
		}
	}
}

// Skip key pressed
void CHIP::OP_Ex9E() {
  if( keypad[regs[get_X()]] ) { 
    PC += 2;
  }
}

// Skip key not pressed
void CHIP::OP_ExA1() {
  if( !keypad[regs[get_X()]] ) { 
    PC += 2;
  }
}

// LD V[x], DT
void CHIP::OP_Fx07() { regs[get_X()] = DT; }

// If a key is pressed, put it in V[X], else redo op
void CHIP::OP_Fx0A() { 
  for( int i = 0; i < 16; i++ ) {
    if( keypad[i] ) {
      regs[get_X()] = i;
      return;
    }
  }

  PC -= 2;
}

// Load DT with V[x]
void CHIP::OP_Fx15() { DT = regs[ get_X() ]; }

// Load ST with V[x]
void CHIP::OP_Fx18() { ST = regs[ get_X() ]; }

// IP += V[x]
void CHIP::OP_Fx1E() { IP += regs[get_X()]; }

// IP = FONT[ V[ x ] ]
void CHIP::OP_Fx29() { IP = FONT_ADDR + ( 5 * regs[get_X()] ); }

// Load BCD of V[x] into memory
void CHIP::OP_Fx33() { 
  uint8_t value = regs[get_X()];
  
  memory[ IP + 2 ] = value % 10;
  value /= 10;
  
  memory[ IP + 1 ] = value % 10;
  value /= 10;
  
  memory[ IP + 0 ] = value % 10;
}

// Load registers into memory
void CHIP::OP_Fx55() {
  for( int i = 0; i <= get_X(); i++ ) {
    memory[IP + i] = regs[i];
  }
}

// Load registers from memory
void CHIP::OP_Fx65() {
  for( int i = 0; i <= get_X(); i++ ) {
    regs[i] = memory[IP + i];
  }
}

std::string CHIP::diss( uint16_t addr ) {
  // Parse opcode at address
  uint16_t tmp_oc  = ( memory[addr] << 8 ) | memory[addr + 1];
  uint16_t tmp_nnn = tmp_oc & 0x0FFF;
  uint8_t   tmp_nn = tmp_oc & 0x00FF;
  uint8_t    tmp_n = tmp_oc & 0x000F;
  uint8_t    tmp_x = ( tmp_oc & 0x0F00 ) >>  8;
  uint8_t    tmp_y = ( tmp_oc & 0x00F0 ) >>  4;
  uint8_t selector = ( tmp_oc & 0xF000 ) >> 12;

  std::string dbg = "";
  
  dbg += std::format( "${:04X} ", addr );
  dbg += std::format( "[ {:04X} ] - ", tmp_oc );

  switch(selector) {
    // Sys calls
    case 0x0:
      if(      tmp_nnn == 0x00E0 ) { dbg +=       "Clear the screen"; }
      else if( tmp_nnn == 0x00EE ) { dbg += "Return from subroutine"; }
      else                         { dbg +=         "Unknown Opcode"; }
      break;
    case 0x1: dbg += std::format( "JUMP ${:03X}", tmp_nnn ); break;
    case 0x2: dbg += std::format( "GOTO ${:03X}", tmp_nnn ); break;
    case 0x3: dbg += std::format( "SE  V{:01X}, #{:02X}", tmp_x, tmp_nn ); break;
    case 0x4: dbg += std::format( "SNE V{:01X}, #{:02X}", tmp_x, tmp_nn ); break;
    case 0x5: dbg += std::format( "SE  V{:01X}, V{:01X}", tmp_x, tmp_y ); break;
    case 0x6: dbg += std::format( "LD  V{:01X}, #{:02X}", tmp_x, tmp_nn ); break;
    case 0x7: dbg += std::format( "ADD V{:01X}, #{:02X}", tmp_x, tmp_nn ); break;
    case 0x8:
      if(       tmp_n == 0x0 )     { dbg += std::format( "LD  V{:01X}, V{:01X}", tmp_x, tmp_y ); }
      else if(  tmp_n == 0x1 )     { dbg += std::format( "OR  V{:01X}, V{:01X}", tmp_x, tmp_y ); }
      else if(  tmp_n == 0x2 )     { dbg += std::format( "AND V{:01X}, V{:01X}", tmp_x, tmp_y ); }
      else if(  tmp_n == 0x3 )     { dbg += std::format( "XOR V{:01X}, V{:01X}", tmp_x, tmp_y ); }
      else if(  tmp_n == 0x4 )     { dbg += std::format( "ADD V{:01X}, V{:01X}", tmp_x, tmp_y ); }
      else if(  tmp_n == 0x5 )     { dbg += std::format( "SUB V{:01X}, V{:01X}", tmp_x, tmp_y ); }
      else if(  tmp_n == 0x6 )     { dbg += std::format( "SHR V{:01X}", tmp_x ); }
      else if(  tmp_n == 0x7 )     { dbg += std::format( "SUB V{:01X}, V{:01X}", tmp_y, tmp_x ); }
      else if(  tmp_n == 0xE )     { dbg += std::format( "SHL V{:01X}", tmp_x ); }
      else                         { dbg +=         "Unknown Opcode"; }
      break;
    case 0x9: dbg += std::format( "SNE V{:01X},  V{:01X}", tmp_x, tmp_y ); break;
    case 0xA: dbg += std::format( "LD I,   ${:03X}", tmp_nnn ); break;
    case 0xD: dbg += std::format( 
      "Draw {:2d}-byte sprite from $IP at ( V{:01X}, V{:01X} )",  
      tmp_n,
      tmp_x,
      tmp_y
    ); break;
    case 0xe:
      if      ( tmp_nn == 0xA1 ) { dbg += std::format( "SKP  V{:01X}", tmp_x ); }
      else if ( tmp_nn == 0xA1 ) { dbg += std::format( "SNKP V{:01X}", tmp_x ); }
      else                       { dbg +=         "Unknown Opcode"; }
      break;
    case 0xf:
      if(       tmp_nn == 0x07 )     { dbg += std::format( "LD  V{:01X}, DT", tmp_x ); }
      else if(  tmp_nn == 0x0A )     { dbg += std::format( "LD  V{:01X}, KEY", tmp_x ); }
      else if(  tmp_nn == 0x15 )     { dbg += std::format( "LD  DT, V{:01X}", tmp_x ); }
      else if(  tmp_nn == 0x18 )     { dbg += std::format( "LD  ST, V{:01X}", tmp_x ); }
      else if(  tmp_nn == 0x1E )     { dbg += std::format( "ADD IP, V{:01X}", tmp_x ); }
      else if(  tmp_nn == 0x29 )     { dbg += std::format( "Point to Font V{:01X}", tmp_x ); }
      else if(  tmp_nn == 0x33 )     { dbg += std::format( "BCD For V{:01X}", tmp_x ); }
      else if(  tmp_nn == 0x55 )     { dbg += std::format( "LD [I], V{:01X}", tmp_x ); }
      else if(  tmp_nn == 0x65 )     { dbg += std::format( "LD V{:01X}, [I]", tmp_x ); }
      else                           { dbg +=         "Unknown Opcode"; }
      break;
    default: dbg += "Unknown Opcode"; break;
  }

  return dbg;
}