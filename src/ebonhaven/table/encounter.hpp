#pragma once

enum class mob_type: uint8_t {
    CREATURE = 0,
    HUMANOID = 1,
    BEAST = 2,
    GIANT = 3,
    ELEMENTAL = 4,
    ARMORED = 5,
    SPECTRAL = 6,
    SPELLCASTER = 7,
    MINION = 8
};

struct mob_drop {
    float_t percentage = 0.00;
    uint64_t item_id;
};


