#pragma once

enum class slot: uint8_t {
    NONE = 0,
    HEAD = 1,
    OUTFIT = 2,
    WEAPON = 3,
    JEWELRY = 4
};

enum class quality: uint8_t {
    JUNK = 0,
    POOR = 1,
    COMMON = 2,
    UNCOMMON = 3,
    RARE = 4,
    EPIC = 5,
    LEGENDARY = 6
};

enum class bind_on: uint8_t {
    NONE = 0,
    EQUIP = 1,
    ACCOUNT = 2
};

struct stats {
    // Increase max health
    uint32_t stamina = 0;
    // Increase health regeneration
    uint32_t regen = 0;
    // Improved chance of resource encounter
    uint32_t perception = 0;
    // Improved chance of crafting additional items
    uint32_t skill = 0;
    // Improved chance of finding treasure
    uint32_t luck = 0;
};

struct attack {
    uint32_t physical = 0;
    uint32_t spell = 0;
};

struct defense {
    uint32_t physical = 0;
    uint32_t spell = 0;
};
