#pragma once

#include <string>
#include <array>
#include <algorithm>
#include <cstdint>
#include <cstdint>

const uint16_t START_ADDR = 0x0200;
const uint16_t  FONT_ADDR = 0x0050;

const unsigned int FONTSET_SIZE = 80;

class CHIP {
  private:
    std::array<uint8_t, 4 * 1024> memory;
    void load_font();
    void clear_video();

    // System variables
    uint16_t PC;
    uint16_t IP;
    uint16_t opcode;
    std::array<uint8_t, 16> regs;

    // Timers
    uint8_t DT;
    uint8_t ST;
    
    // Stack
    uint8_t SP;
    std::array<uint16_t, 16> stack;

    // Keypad
    std::array<bool, 16> keypad;

    uint16_t get_NNN() { return   opcode & 0x0FFF; }
    uint8_t   get_NN() { return   opcode & 0x00FF; }
    uint8_t    get_N() { return   opcode & 0x000F; }
    uint8_t    get_Y() { return ( opcode & 0x00F0 ) >>  4; }
    uint8_t    get_X() { return ( opcode & 0x0F00 ) >>  8; }
    uint8_t    get_S() { return ( opcode & 0xF000 ) >> 12; }

    // Opcodes
    void sys_calls();
    void ALU();
    void OP_Exnn();
    void Fxnn();
    

    void   OP_00E0();
    void   OP_00EE();
    void   OP_1nnn();
    void   OP_2nnn();
    void   OP_3xnn();
    void   OP_4xnn();
    void   OP_5xy0();
    void   OP_6xnn();
    void   OP_7xnn();
    void   OP_8xy0();
    void   OP_8xy1();
    void   OP_8xy2();
    void   OP_8xy3();
    void   OP_8xy4();
    void   OP_8xy5();
    void   OP_8xy6();
    void   OP_8xy7();
    void   OP_8xyE();
    void   OP_9xy0();
    void   OP_Annn();
    void   OP_Cxnn();
    void   OP_Dxyn();
    void   OP_Ex9E();
    void   OP_ExA1();
    void   OP_Fx07();
    void   OP_Fx0A();
    void   OP_Fx15();
    void   OP_Fx18();
    void   OP_Fx1E();
    void   OP_Fx29();
    void   OP_Fx33();
    void   OP_Fx55();
    void   OP_Fx65();

  public:
    CHIP();
    ~CHIP();

    bool load_rom( std::string rompath );
    void reset();
    void step();
    void timers();
    std::string diss( uint16_t addr );

    void set_key( uint8_t reg );
    void clear_key( uint8_t reg );
    
    // GFX
    std::array<uint32_t, 64 * 32> video;
};