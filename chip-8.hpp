#include "framework.h"

constexpr const uint8_t chip8_fontset[80] = {
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

constexpr const uint8_t keymap[16] = {
    SDLK_x,
    SDLK_1,
    SDLK_2,
    SDLK_3,
    SDLK_q,
    SDLK_w,
    SDLK_e,
    SDLK_a,
    SDLK_s,
    SDLK_d,
    SDLK_z,
    SDLK_c,
    SDLK_4,
    SDLK_r,
    SDLK_f,
    SDLK_v,
};

class Chip8
{
    std::array<uint8_t, 4096> memory; // 4K memory
    std::array<uint8_t, 16> V;        // 16 8-bit general purpose registers
    std::stack<uint16_t> stack;       // 16-bit stack
    uint16_t I = 0;                   // 16-bit index register
    uint16_t pc = 0x200;              // 16-bit program counter
    uint8_t delay_timer = 0;          // 8-bit delay timer
    uint8_t sound_timer = 0;          // 8-bit sound timer

public:
    std::array<uint8_t, 16> key;          // 16-bit keypad
    std::array<uint32_t, 64 * 32> screen; // 64x32 pixels
    bool dirty = false;                   // dirty flag

    std::vector<uint8_t> read_rom(const std::string &path)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open())
        {
            std::cout << "Failed to open file: " << path << std::endl;
            return {};
        }

        std::vector<uint8_t> buffer;
        buffer.resize(file.seekg(0, std::ios::end).tellg());
        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
        file.close();
        return buffer;
    }

    Chip8()
    {
        // Empty the memory
        memory.fill(0);
        screen.fill(0);
        V.fill(0);
        key.fill(0);

        // Load fontset
        for (int i = 0; i < 80; i++)
        {
            memory[i] = chip8_fontset[i];
        }

        auto rom = read_rom("E:\\source\\repos\\CHIP-8-EMU\\roms\\Tic-Tac-Toe [David Winter].ch8");

        if (rom.size() > 0)
        {
            printf("Loaded rom, [%u] bytes.\n", rom.size());

            for (auto &&i : rom)
            {
                memory[pc++] = i;
            }

            // Reset the program counter
            pc = 0x200;
        }
    }

    void updateTimers()
    {
        if (delay_timer > 0)
        {
            delay_timer--;
        }

        if (sound_timer > 0)
        {
            sound_timer--;
        }
    }

    void executeOpcode(uint16_t &opc)
    {
        pc += 2;

        // http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#3.0
        auto x = (opc & 0x0F00) >> 8;
        auto y = (opc & 0x00F0) >> 4;
        auto n = opc & 0x000F;
        auto nn = opc & 0x00FF;
        auto nnn = opc & 0x0FFF;

        switch (opc & 0xF000)
        {
        case 0x0000:
        {
            switch (opc)
            {
            case 0x00E0:
                // CLS
                screen.fill(0);
                dirty = true;
                break;
            case 0x00EE:
            {
                // RET
                pc = stack.top();
                stack.pop();
                break;
            }
            }
            break;
        }

        case 0x1000:
        {
            // JP addr
            pc = nnn;
            break;
        }
        case 0x2000:
        {
            // CALL addr
            stack.push(pc);
            pc = nnn;
            break;
        }
        case 0x3000:
        {
            // SE Vx, byte
            if (V[x] == nn)
            {
                pc += 2;
            }
            break;
        }
        case 0x4000:
        {
            // SNE Vx, byte
            if (V[x] != nn)
            {
                pc += 2;
            }
            break;
        }
        case 0x5000:
        {
            // SE Vx, Vy
            if (V[x] == V[y])
            {
                pc += 2;
            }
            break;
        }
        case 0x6000:
        {
            // LD Vx, byte
            V[x] = nn;
            break;
        }
        case 0x7000:
        {
            // ADD Vx, byte
            V[x] += nn;
            break;
        }
        case 0x8000:
        {
            switch (n)
            {
            case 0x0:
            {
                // LD Vx, Vy
                V[x] = V[y];
                break;
            }
            case 0x1:
            {
                // OR Vx, Vy
                V[x] |= V[y];
                break;
            }
            case 0x2:
            {
                // AND Vx, Vy
                V[x] &= V[y];
                break;
            }
            case 0x3:
            {
                // XOR Vx, Vy
                V[x] ^= V[y];
                break;
            }
            case 0x4:
            {
                // ADD Vx, Vy
                if (V[x] + V[y] > 255)
                {
                    V[0xF] = 1;
                }
                else
                {
                    V[0xF] = 0;
                }

                V[x] += V[y];
                break;
            }
            case 0x5:
            {
                // SUB Vx, Vy
                if (V[x] > V[y])
                {
                    V[0xF] = 1;
                }
                else
                {
                    V[0xF] = 0;
                }

                V[x] -= V[y];
                break;
            }
            case 0x6:
            {
                // SHR Vx {, Vy}
                V[0xF] = V[x] & 0x1;

                V[x] >>= 1;
                break;
            }
            case 0x7:
            {
                // SUBN Vx, Vy
                if (V[y] > V[x])
                {
                    V[0xF] = 1;
                }
                else
                {
                    V[0xF] = 0;
                }

                V[x] = V[y] - V[x];
                break;
            }
            case 0xE:
            {
                // SHL Vx {, Vy}
                V[0xF] = V[x] & 0x80;

                V[x] <<= 1;
                break;
            }
            }
            break;
        }
        case 0x9000:
        {
            // SNE Vx, Vy
            if (V[x] != V[y])
            {
                pc += 2;
            }
            break;
        }
        case 0xA000:
        {
            // LD I, addr
            I = nnn;
            break;
        }
        case 0xB000:
        {
            // JP V0, addr
            pc = nnn + V[0];
            break;
        }
        case 0xC000:
        {
            // RND Vx, byte
            V[x] = (rand() % 256) & nn;
            break;
        }
        case 0xD000:
        {
            // DRW Vx, Vy, nibble
            V[0xF] = 0;

            for (auto row = 0; row < n; row++)
            {
                auto sprite = memory[I + row];

                for (auto col = 0; col < 8; col++)
                {
                    if ((sprite & (0x80 >> col)) != 0)
                    {
                        auto i = V[x] + col + ((V[y] + row) * 64);

                        if (screen[i] == 1)
                        {
                            V[0xF] = 1;
                        }

                        screen[i] ^= 1; // Flip

                        screen[i] = (0x00FFFFFF * screen[i]) | 0xFF000000;
                    }
                }
            }

            dirty = true;
            break;
        }
        case 0xE000:
            switch (nn)
            {
            case 0x9E:
                // SKP Vx
                if (key[V[x]] != 0)
                {
                    pc += 2;
                }
                break;
            case 0xA1:
                // SKNP Vx
                if (key[V[x]] == 0)
                {
                    pc += 2;
                }
                break;
            }
            break;
        case 0xF000:
            switch (nn)
            {
            case 0x07:
                // LD Vx, DT
                V[x] = delay_timer;
                break;
            case 0x0A:
            {
                // LD Vx, K
                bool key_pressed = false;
                for (auto i = 0; i < 16; i++)
                {
                    if (key[i] != 0)
                    {
                        V[x] = i;
                        key_pressed = true;
                        break;
                    }
                }

                if (!key_pressed)
                {
                    pc -= 2;
                    return;
                }
            }
            break;
            case 0x15:
                // LD DT, Vx
                delay_timer = V[x];
                break;
            case 0x18:
                // LD ST, Vx
                sound_timer = V[x];
                break;
            case 0x1E:
                // ADD I, Vx
                if (I + V[x] > 0xFFF)
                {
                    V[0xF] = 1;
                }
                else
                {
                    V[0xF] = 0;
                }

                I += V[x];
                break;
            case 0x29:
                // LD F, Vx
                I = V[x] * 5;
                break;
            case 0x33:
                // LD B, Vx
                memory[I] = V[x] / 100;
                memory[I + 1] = (V[x] / 10) % 10;
                memory[I + 2] = (V[x]) % 10;
                break;
            case 0x55:
                // LD [I], Vx
                for (auto i = 0; i <= x; i++)
                {
                    memory[I + i] = V[i];
                }

                I += x + 1;
                break;
            case 0x65:
                // LD Vx, [I]
                for (auto i = 0; i <= x; i++)
                {
                    V[i] = memory[I + i];
                }

                I += x + 1;
                break;
            }
            break;

        default:
            // UNKNOWN OPCODE
            printf("Unknown opcode: 0x%X\n", opc);
            break;
        }
    }

    void cycle()
    {
        uint16_t curOpcode = (memory[pc] << 8) | memory[pc + 1];

        executeOpcode(curOpcode);
        updateTimers();

        // Play sound
    }
};