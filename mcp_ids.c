/*
 Authors:
 Copyright 2012-2015 by Eduard Broese <ed.broese@gmx.de>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version
 2 of the License, or (at your option) any later version.
*/

#include "mcp_ids.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <strings.h>

#include "mcp_packet.h"

#define MNAMES_WOOD     { "Oak", "Spruce", "Birch", "Jungle", "Acacia", "Dark Oak" }
#define MNAMES_OLDWOOD  { "Oak", "Spruce", "Birch", "Jungle" }
#define MNAMES_NEWWOOD  { "Acacia", "Dark Oak", NULL, NULL }
#define MNAMES_COLOR    { "White", "Orange", "Magenta", "LBlue", "Yellow", "Lime", "Pink", "Gray", \
                          "LGray", "Cyan", "Purple", "Blue", "Brown", "Green", "Red", "Black" }

const item_id ITEMS[] = {
    ////////////////////////////////////////////////////////////////////////
    // Blocks

    [0x00] = { "Air" },
    [0x01] = { "Stone",                 I_MTYPE|I_OPAQUE,
               { NULL, "Granite", "P. Granite", "Diorite",
                 "P. Diorite", "Andesite", "P. Andesite" } },
    [0x02] = { "Grass",                 I_OPAQUE },
    [0x03] = { "Dirt",                  I_MTYPE|I_OPAQUE,
               { NULL, "Coarse", "Podzol" } },
    [0x04] = { "Cobblestone",           I_OPAQUE },
    [0x05] = { "Planks",                I_MTYPE|I_OPAQUE,
               MNAMES_WOOD },
    [0x06] = { "Sapling",               I_MTYPE|I_STATE_8|I_PLANT,      // S: growth
               MNAMES_WOOD },
    [0x07] = { "Bedrock",               I_OPAQUE },
    [0x08] = { "Flowing Water",         I_STATE_F },                    // S: level/drop
    [0x09] = { "Water",                 I_STATE_F },                    // S: level
    [0x0a] = { "Flowing Lava",          I_STATE_F },                    // S: level/drop
    [0x0b] = { "Lava",                  I_STATE_F },                    // S: level
    [0x0c] = { "Sand",                  I_MTYPE|I_OPAQUE,
               { NULL, "Red" } },
    [0x0d] = { "Gravel",                I_OPAQUE },
    [0x0e] = { "Gold Ore",              I_OPAQUE },
    [0x0f] = { "Iron Ore",              I_OPAQUE },

    [0x10] = { "Coal Ore",              I_OPAQUE },
    [0x11] = { "Log",                   I_MTYPE|I_MPOS|I_LOG|I_OPAQUE,  // P: dir
               MNAMES_OLDWOOD },
    [0x12] = { "Leaves",                I_MTYPE|I_STATE_C,              // S: decay
               MNAMES_OLDWOOD },
    [0x13] = { "Sponge",                I_MTYPE|I_OPAQUE,
               { NULL, "Wet" }, },
    [0x14] = { "Glass" },
    [0x15] = { "Lapis Ore",             I_OPAQUE },
    [0x16] = { "Lapis Block",           I_OPAQUE },
    [0x17] = { "Dispenser",             I_MPOS|I_STATE_8|I_OPAQUE|I_CONT|I_RSDEV },     // P: dir, S: active
    [0x18] = { "Sandstone",             I_MTYPE|I_OPAQUE,
               { NULL, "Chiseled", "Smooth"} },
    [0x19] = { "Noteblock",             I_OPAQUE|I_ADJ },               // A: note (not in meta, but keep I_ADJ for click protection)
    [0x1a] = { "Bed",                   I_MPOS|I_STATE_4 },             // P: dir, S: occupied
    [0x1b] = { "Golden Rail",           I_STATE_F },                    // S: dir/form, active
    [0x1c] = { "Detector Rail",         I_STATE_F },                    // S: dir/form, active
    [0x1d] = { "Sticky Piston",         I_MPOS|I_STATE_8|I_RSDEV },     // P: dir, S: extended
    [0x1e] = { "Web", },
    [0x1f] = { "Tallgrass",             I_MTYPE,
               { "Shrub", NULL, "Fern", }, },

    [0x20] = { "Deadbush" },
    [0x21] = { "Piston",                I_MPOS|I_STATE_8|I_RSDEV },     // P: dir, S: extended
    [0x22] = { "Piston Head",           I_STATE_F },                    // S: dir, extended
    [0x23] = { "Wool",                  I_MTYPE|I_OPAQUE,
               MNAMES_COLOR },
    [0x24] = { "Piston Extension", },
    [0x25] = { "Yellow Flower", },
    [0x26] = { "Red Flower",            I_MTYPE|I_PLANT,
               { "Poppy", "Blue orchid", "Allium", "Azure Bluet",
                 "Red Tulip", "Orange Tulip", "White Tulip", "Pink Tulip",
                 "Oxeye Daisy", }, },
    [0x27] = { "Brown Mushroom",        I_PLANT },
    [0x28] = { "Red Mushroom",          I_PLANT },
    [0x29] = { "Gold Block",            I_OPAQUE },
    [0x2a] = { "Iron Block",            I_OPAQUE },
    [0x2b] = { "Double Stone Slab",     I_MTYPE|I_DSLAB|I_OPAQUE,
               { "Stone", "Sandstone", "Stonewood", "Cobble",
                 "Brick", "Stone Brick", "Netherbrick", "Quartz",
                 "Smooth Stone", "Smooth Sandstone",
                 [15] = "Tile Quartz", }, },
    [0x2c] = { "Stone Slab",            I_MTYPE|I_MPOS|I_SLAB,          // P: U/D
               { NULL, "Sandstone", "Stonewood", "Cobble",
                 "Brick", "Stone Brick", "Netherbrick", "Quartz" } },
    [0x2d] = { "Brick Block",           I_OPAQUE },
    [0x2e] = { "TNT",                   I_OPAQUE },
    [0x2f] = { "Bookshelf",             I_OPAQUE },

    [0x30] = { "Mossy Cobbletone",      I_OPAQUE },
    [0x31] = { "Obsidian",              I_OPAQUE },
    [0x32] = { "Torch",                 I_MPOS|I_TORCH },               // P: dir
    [0x33] = { "Fire",                  I_STATE_F },                    // S: age
    [0x34] = { "Mob Spawner" },
    [0x35] = { "Oak Stairs",            I_MPOS|I_STAIR },               // P: dir
    [0x36] = { "Chest",                 I_MPOS|I_CONT|I_CHEST },        // P: dir
    [0x37] = { "Redstone Wire",         I_STATE_F },                    // S: power level
    [0x38] = { "Diamond Ore",           I_OPAQUE },
    [0x39] = { "Diamond Block",         I_OPAQUE },
    [0x3a] = { "Crafting Table",        I_OPAQUE|I_CONT },
    [0x3b] = { "Wheat",                 I_STATE_F|I_PLANT },            // S: level
    [0x3c] = { "Farmland",              I_STATE_F },                    // S: wetness
    [0x3d] = { "Furnace",               I_MPOS|I_OPAQUE|I_CONT|I_CHEST },       // P: dir
    [0x3e] = { "Lit Furnace",           I_MPOS|I_OPAQUE|I_CONT|I_CHEST },       // P: dir
    [0x3f] = { "Standing Sign",         I_MPOS },                       // P: dir

    [0x40] = { "Wooden Door",           I_MPOS|I_DOOR|I_ADJ },          // P: dir, A: open/close
    [0x41] = { "Ladder",                I_MPOS|I_ONWALL },              // P: dir
    [0x42] = { "Rail",                  I_STATE_F },                    // S: dir/form
    [0x43] = { "Stone Stairs",          I_MPOS|I_STAIR },               // P: dir
    [0x44] = { "Wall Sign",             I_MPOS|I_ONWALL },              // P: dir
    [0x45] = { "Lever",                 I_MPOS|I_ADJ },                 // P: dir, A: on/off
    [0x46] = { "Stone Pressure Plate",  I_STATE_1 },                    // S: pressed
    [0x47] = { "Iron Door",             I_MPOS|I_STATE_8|I_DOOR },      // P: dir, S: open/close
    [0x48] = { "Wooden Pressure Plate", I_STATE_1 },                    // S: pressed
    [0x49] = { "Redstone Ore",          I_OPAQUE },
    [0x4a] = { "Lit Redstone Ore",      I_OPAQUE },
    [0x4b] = { "Redstone Torch",        I_MPOS|I_TORCH },               // P: dir
    [0x4c] = { "Unlit Redstone Torch",  I_MPOS|I_TORCH },               // P: dir
    [0x4d] = { "Stone Button",          I_MPOS|I_STATE_8 },             // P: dir, S: pressed
    [0x4e] = { "Snow Layer",            I_STATE_F },                    // S: level
    [0x4f] = { "Ice" },

    [0x50] = { "Snow",                  I_OPAQUE },
    [0x51] = { "Cactus",                I_STATE_F|I_PLANT },            // S: growth
    [0x52] = { "Clay",                  I_OPAQUE },
    [0x53] = { "Reeds",                 I_STATE_F|I_PLANT },            // S: growth
    [0x54] = { "Jukebox",               I_OPAQUE|I_ADJ },               // A: disk (I_ADJ primarily for the click-protection)
    [0x55] = { "Fence" },
    [0x56] = { "Pumpkin",               I_MPOS|I_OPAQUE },              // P: dir
    [0x57] = { "Netherrack",            I_OPAQUE },
    [0x58] = { "Soul Sand",             I_OPAQUE },
    [0x59] = { "Glowstone",             I_OPAQUE },
    [0x5a] = { "Portal",                I_MPOS },
    [0x5b] = { "Lit Pumpkin",           I_MPOS|I_OPAQUE },
    [0x5c] = { "Cake",                  I_STATE_F },                    // S: level
    [0x5d] = { "Unpowered Repeater",    I_MPOS|I_RSRC|I_ADJ },          // P: dir, A: delay setting
    [0x5e] = { "Powered Repeater",      I_MPOS|I_RSRC|I_ADJ },          // P: dir, A: delay setting
    [0x5f] = { "Stained Glass",         I_MTYPE, MNAMES_COLOR },

    [0x60] = { "Trapdoor",              I_MPOS|I_ADJ|I_TDOOR },         // P: dir,u/d, A: open/close
    [0x61] = { "Monster Egg",           I_MTYPE|I_OPAQUE,
               { "Stone", "Cobblestone", "Stonebrick", "Mossy Stonebrick",
                 "Cracked Stonebrick", "Chiseled Stonebrick" }, },
    [0x62] = { "Stonebrick",            I_MTYPE|I_OPAQUE,
               { NULL, "Mossy", "Cracked", "Chiseled" }, },
    [0x63] = { "Brown Mushroom Block",  I_MPOS|I_OPAQUE },              // P: part
    [0x64] = { "Red Mushroom Block",    I_MPOS|I_OPAQUE },              // P: part
    [0x65] = { "Iron Bars" },
    [0x66] = { "Glass Pane" },
    [0x67] = { "Melon Block",           I_OPAQUE },
    [0x68] = { "Pumpkin Stem",          I_STATE_F|I_PLANT },            // S: level
    [0x69] = { "Melon Stem",            I_STATE_F|I_PLANT },            // S: level
    [0x6a] = { "Vine",                  I_MPOS },                       // P: dir
    [0x6b] = { "Fence Gate",            I_MPOS|I_ADJ|I_GATE },          // P: dir, A: open/close
    [0x6c] = { "Brick Stairs",          I_MPOS|I_STAIR },               // P: dir
    [0x6d] = { "Stone Brick Stairs",    I_MPOS|I_STAIR },               // P: dir
    [0x6e] = { "Mycelium",              I_OPAQUE },
    [0x6f] = { "Waterlily",             I_PLANT },

    [0x70] = { "Nether Brick",          I_OPAQUE },
    [0x71] = { "Nether Brick Fence" },
    [0x72] = { "Nether Brick Stairs",   I_MPOS|I_STAIR },               // P: dir
    [0x73] = { "Nether Wart",           I_STATE_F },                    // S: level
    [0x74] = { "Enchanting Table",      I_CONT },
    [0x75] = { "Brewing Stand",         I_STATE_F|I_CONT },             // S: bottles
    [0x76] = { "Cauldron",              I_STATE_F },                    // S: level
    [0x77] = { "End Portal",            I_MPOS },
    [0x78] = { "End Portal Frame",      I_MPOS },                       // P: dir
    [0x79] = { "End Stone",             I_OPAQUE },
    [0x7a] = { "Dragon Egg" },
    [0x7b] = { "Redstone Lamp",         I_OPAQUE },
    [0x7c] = { "Lit Redstone Lamp",     I_OPAQUE },
    [0x7d] = { "Double Wooden Slab",    I_MTYPE|I_DSLAB|I_OPAQUE,
               MNAMES_WOOD },
    [0x7e] = { "Wooden Slab",           I_MTYPE|I_MPOS|I_SLAB,          // P: up/down
               MNAMES_WOOD },
    [0x7f] = { "Cocoa",                 I_MPOS|I_STATE_C },             // P: dir, S: level

    [0x80] = { "Sandstone Stairs",      I_MPOS|I_STAIR },               // P: dir
    [0x81] = { "Emerald Ore",           I_OPAQUE },
    [0x82] = { "Ender Chest",           I_MPOS|I_CONT|I_CHEST },        // P: dir
    [0x83] = { "Tripwire Hook",         I_MPOS|I_STATE_C },             // P: dir, S: connected,active
    [0x84] = { "Tripwire",              I_STATE_F },                    // S: act/susp/att/arm
    [0x85] = { "Emerald Block",         I_OPAQUE },
    [0x86] = { "Spruce Stairs",         I_MPOS|I_STAIR },               // P: dir
    [0x87] = { "Birch Stairs",          I_MPOS|I_STAIR },               // P: dir
    [0x88] = { "Jungle Stairs",         I_MPOS|I_STAIR },               // P: dir
    [0x89] = { "Command Block",         I_OPAQUE|I_CONT },
    [0x8a] = { "Beacon",                I_CONT },
    [0x8b] = { "Cobblestone Wall",      I_MTYPE,
               { NULL, "Mossy" } },
    [0x8c] = { "Flower Pot" },
    [0x8d] = { "Carrots",               I_STATE_F|I_PLANT },            // S: level
    [0x8e] = { "Potatoes",              I_STATE_F|I_PLANT },            // S: level
    [0x8f] = { "Wooden Button",         I_MPOS|I_STATE_8 },             // P: dir, S: pressed

    [0x90] = { "Skull",                 I_MPOS },
    [0x91] = { "Anvil",                 I_MPOS|I_CONT },
    [0x92] = { "Trapped Chest",         I_MPOS|I_CONT|I_CHEST },        // P: dir
    [0x93] = { "Light Weighted Pressure Plate", I_STATE_F },                    // S: pressed
    [0x94] = { "Heavy Weighted Pressure Plate", I_STATE_F },                    // S: pressed
    [0x95] = { "Unpowered Comparator",  I_MPOS|I_STATE_8|I_RSRC|I_ADJ },// P: dir, S: mode/power
    [0x96] = { "Powered Comparator",    I_MPOS|I_STATE_8|I_RSRC|I_ADJ },// P: dir, S: mode/power
    [0x97] = { "Daylight Detector",     I_ADJ },
    [0x98] = { "Redstone Block",        I_OPAQUE },
    [0x99] = { "Quartz Ore",            I_OPAQUE },
    [0x9a] = { "Hopper",                I_MPOS|I_STATE_8|I_CONT },      // P: dir, S: active
    [0x9b] = { "Quartz Block",          I_MTYPE|I_MPOS|I_OPAQUE,        // P: dir (pillar only)
               { NULL, "Chiseled", "Pillar" } },
    [0x9c] = { "Quartz Stairs",         I_MPOS|I_STAIR },               // P: dir
    [0x9d] = { "Activator Rail",        I_STATE_F },                    // S: dir/form, active
    [0x9e] = { "Dropper",               I_MPOS|I_STATE_8|I_OPAQUE|I_CONT|I_RSDEV },       // P: dir, S: active
    [0x9f] = { "Stained Hardened Clay", I_MTYPE|I_OPAQUE,
               MNAMES_COLOR },

    [0xa0] = { "Stained Glass Pane",    I_MTYPE,
               MNAMES_COLOR },
    [0xa1] = { "Leaves2",               I_MTYPE|I_STATE_C,              // S: decay
               MNAMES_NEWWOOD },
    [0xa2] = { "Log2",                  I_MTYPE|I_MPOS|I_LOG|I_OPAQUE,  // P: dir
               MNAMES_NEWWOOD },
    [0xa3] = { "Acacia Stairs",         I_MPOS|I_STAIR },               // P: dir
    [0xa4] = { "Dark Oak Stairs",       I_MPOS|I_STAIR },               // P: dir
    [0xa5] = { "Slime" },
    [0xa6] = { "Barrier" },
    [0xa7] = { "Iron Trapdoor",         I_MPOS|I_STATE_4|I_TDOOR },     // P: dir,u/d, S: open/close
    [0xa8] = { "Prismarine",            I_MTYPE|I_OPAQUE,
               { NULL, "Brick", "Dark" }, },
    [0xa9] = { "Sea Lantern",           I_OPAQUE },
    [0xaa] = { "Hay Block",             I_MTYPE|I_MPOS|I_LOG|I_OPAQUE },// P: dir
    [0xab] = { "Carpet",                I_MTYPE,
               MNAMES_COLOR },
    [0xac] = { "Hardened Clay",         I_MTYPE|I_OPAQUE },
    [0xad] = { "Coal Block",            I_OPAQUE },
    [0xae] = { "Packed Ice",            I_OPAQUE },
    [0xaf] = { "Double Plant",          I_MTYPE|I_MPOS|I_PLANT,         // P: top/bottom
               { "Sunflower", "Lilac", "DblTallgrass", "Large Fern",
                 "Rose Bush", "Peony", NULL, NULL }, },

    [0xb0] = { "Standing Banner",       I_MPOS },                       // P: dir
    [0xb1] = { "Wall Banner",           I_MPOS|I_ONWALL },              // P: dir
    [0xb2] = { "Daylight Detector Inverted",    I_ADJ },
    [0xb3] = { "Red Sandstone",         I_MTYPE|I_OPAQUE,
               { NULL, "Chiseled", "Smooth"} },
    [0xb4] = { "Red Sandstone Stairs",  I_MPOS|I_STAIR },               // P: dir
    [0xb5] = { "Double Stone Slab2",    I_MTYPE|I_DSLAB|I_OPAQUE,
               { [0]="Red Sandstone", [8]="Smooth Red Sandstone" }, },
    [0xb6] = { "Stone Slab2",           I_MPOS|I_SLAB,                  // P: up/down
               { [0]="Red Sandstone"}, },
    [0xb7] = { "Spruce Fence Gate",     I_MPOS|I_ADJ|I_GATE },          // P: dir, S: open/close
    [0xb8] = { "Birch Fence Gate",      I_MPOS|I_ADJ|I_GATE },          // P: dir, S: open/close
    [0xb9] = { "Jungle Fence Gate",     I_MPOS|I_ADJ|I_GATE },          // P: dir, S: open/close
    [0xba] = { "Dark Oak Fence Gate",   I_MPOS|I_ADJ|I_GATE },          // P: dir, S: open/close
    [0xbb] = { "Acacia Fence Gate",     I_MPOS|I_ADJ|I_GATE },          // P: dir, S: open/close
    [0xbc] = { "Spruce Fence" },
    [0xbd] = { "Birch Fence" },
    [0xbe] = { "Jungle Fence" },
    [0xbf] = { "Dark Oak Fence" },

    [0xc0] = { "Acacia Fence" },
    [0xc1] = { "Spruce Door",           I_MPOS|I_ADJ|I_DOOR },  // P: dir, A: open/close
    [0xc2] = { "Birch Door",            I_MPOS|I_ADJ|I_DOOR },  // P: dir, A: open/close
    [0xc3] = { "Jungle Door",           I_MPOS|I_ADJ|I_DOOR },  // P: dir, A: open/close
    [0xc4] = { "Dark Oak Door",         I_MPOS|I_ADJ|I_DOOR },  // P: dir, A: open/close
    [0xc5] = { "Acacia Door",           I_MPOS|I_ADJ|I_DOOR },  // P: dir, A: open/close
    [0xc6] = { "End Rod",               I_MPOS },               // P: dir
    [0xc7] = { "Chorus Plant" },
    [0xc8] = { "Chorus Flower",         I_STATE_F|I_PLANT },    // S: level
    [0xc9] = { "Purpur Block",          I_OPAQUE },
    [0xca] = { "Purpur Pillar",         I_MTYPE|I_MPOS|I_LOG|I_OPAQUE },// P: dir
    [0xcb] = { "Purpur Stairs",         I_MPOS|I_STAIR },               // P: dir
    [0xcc] = { "Purpur Double Slab",    I_MTYPE|I_DSLAB|I_OPAQUE,
               { }, },
    [0xcd] = { "Purpur Slab",           I_MPOS|I_SLAB },        // P: up/down
    [0xce] = { "End Bricks",            I_OPAQUE },
    [0xcf] = { "Beetroots",             I_STATE_F|I_PLANT },    // S: level

    [0xd0] = { "Grass Path" },
    [0xd1] = { "End Gateway" },
    [0xd2] = { "Repeating Command Block",   I_OPAQUE|I_CONT },
    [0xd3] = { "Chain Command Block",       I_OPAQUE|I_CONT },
    [0xd4] = { "Frosted Ice",           I_STATE_F|I_OPAQUE },   // S: level
    [0xd5] = { "Magma",                 I_OPAQUE },
    [0xd6] = { "Nether Wart Block",     I_OPAQUE },
    [0xd7] = { "Red Nether Brick",      I_OPAQUE },
    [0xd8] = { "Bone Block",            I_OPAQUE|I_LOG },
    [0xd9] = { "Structure Void" },
    [0xda] = { "Observer",              I_OBSERVER },
    [0xdb] = { "White Shulker Box",     I_CHEST },
    [0xdc] = { "Orange Shulker Box",    I_CHEST },
    [0xdd] = { "Magenta Shulker Box",   I_CHEST },
    [0xde] = { "Light Blue Shulker Box",I_CHEST },
    [0xdf] = { "Yellow Shulker Box",    I_CHEST },

    [0xe0] = { "Lime Shulker Box",      I_CHEST },
    [0xe1] = { "Pink Shulker Box",      I_CHEST },
    [0xe2] = { "Gray Shulker Box",      I_CHEST },
    [0xe3] = { "Light Gray Shulker Box",I_CHEST },
    [0xe4] = { "Cyan Shulker Box",      I_CHEST },
    [0xe5] = { "Purple Shulker Box",    I_CHEST },
    [0xe6] = { "Blue Shulker Box",      I_CHEST },
    [0xe7] = { "Brown Shulker Box",     I_CHEST },
    [0xe8] = { "Green Shulker Box",     I_CHEST },
    [0xe9] = { "Red Shulker Box",       I_CHEST },
    [0xea] = { "Black Shulker Box",     I_CHEST },
    [0xeb] = { "White Glazed Terracota",        I_MPOS|I_OPAQUE|I_TERRACOTA },  // P: dir
    [0xec] = { "Orange Glazed Terracota",       I_MPOS|I_OPAQUE|I_TERRACOTA },  // P: dir
    [0xed] = { "Magenta Glazed Terracota",      I_MPOS|I_OPAQUE|I_TERRACOTA },  // P: dir
    [0xee] = { "Light Blue Glazed Terracota",   I_MPOS|I_OPAQUE|I_TERRACOTA },  // P: dir
    [0xef] = { "Yellow Glazed Terracota",       I_MPOS|I_OPAQUE|I_TERRACOTA },  // P: dir

    [0xf0] = { "Lime Glazed Terracota",         I_MPOS|I_OPAQUE|I_TERRACOTA },  // P: dir
    [0xf1] = { "Pink Glazed Terracota",         I_MPOS|I_OPAQUE|I_TERRACOTA },  // P: dir
    [0xf2] = { "Gray Glazed Terracota",         I_MPOS|I_OPAQUE|I_TERRACOTA },  // P: dir
    [0xf3] = { "Light Gray Glazed Terracota",   I_MPOS|I_OPAQUE|I_TERRACOTA },  // P: dir
    [0xf4] = { "Cyan Glazed Terracota",         I_MPOS|I_OPAQUE|I_TERRACOTA },  // P: dir
    [0xf5] = { "Purple Glazed Terracota",       I_MPOS|I_OPAQUE|I_TERRACOTA },  // P: dir
    [0xf6] = { "Blue Glazed Terracota",         I_MPOS|I_OPAQUE|I_TERRACOTA },  // P: dir
    [0xf7] = { "Brown Glazed Terracota",        I_MPOS|I_OPAQUE|I_TERRACOTA },  // P: dir
    [0xf8] = { "Green Glazed Terracota",        I_MPOS|I_OPAQUE|I_TERRACOTA },  // P: dir
    [0xf9] = { "Red Glazed Terracota",          I_MPOS|I_OPAQUE|I_TERRACOTA },  // P: dir
    [0xfa] = { "Black Glazed Terracota",        I_MPOS|I_OPAQUE|I_TERRACOTA },  // P: dir
    [0xfb] = { "Concrete",              I_MTYPE|I_OPAQUE,
                MNAMES_COLOR },
    [0xfc] = { "Concrete Powder",       I_MTYPE|I_OPAQUE,
                MNAMES_COLOR },

    [0xff] = { "Structure Block",       I_OPAQUE },

    ////////////////////////////////////////////////////////////////////////
    // Items

    [0x100] = { "Iron Shovel",          I_ITEM|I_NSTACK },
    [0x101] = { "Iron Pickaxe",         I_ITEM|I_NSTACK },
    [0x102] = { "Iron Axe",             I_ITEM|I_NSTACK },
    [0x103] = { "Flint and Steel",      I_ITEM|I_NSTACK },
    [0x104] = { "Apple",                I_ITEM },
    [0x105] = { "Bow",                  I_ITEM|I_NSTACK },
    [0x106] = { "Arrow",                I_ITEM },
    [0x107] = { "Coal",                 I_ITEM|I_MTYPE,
                { NULL, "Charcoal" }, },
    [0x108] = { "Diamond",              I_ITEM },
    [0x109] = { "Iron Ingot",           I_ITEM },
    [0x10a] = { "Gold Ingot",           I_ITEM },
    [0x10b] = { "Iron Sword",           I_ITEM|I_NSTACK },
    [0x10c] = { "Wooden Sword",         I_ITEM|I_NSTACK },
    [0x10d] = { "Wooden Shovel",        I_ITEM|I_NSTACK },
    [0x10e] = { "Wooden Pickaxe",       I_ITEM|I_NSTACK },
    [0x10f] = { "Wooden Axe",           I_ITEM|I_NSTACK },

    [0x110] = { "Stone Sword",          I_ITEM|I_NSTACK },
    [0x111] = { "Stone Shovel",         I_ITEM|I_NSTACK },
    [0x112] = { "Stone Pickaxe",        I_ITEM|I_NSTACK },
    [0x113] = { "Stone Axe",            I_ITEM|I_NSTACK },
    [0x114] = { "Diamond Sword",        I_ITEM|I_NSTACK },
    [0x115] = { "Diamond Shovel",       I_ITEM|I_NSTACK },
    [0x116] = { "Diamond Pickaxe",      I_ITEM|I_NSTACK },
    [0x117] = { "Diamond Axe",          I_ITEM|I_NSTACK },
    [0x118] = { "Stick",                I_ITEM },
    [0x119] = { "Bowl",                 I_ITEM },
    [0x11a] = { "Mushroom Stew",        I_ITEM|I_NSTACK|I_FOOD },
    [0x11b] = { "Golden Sword",         I_ITEM|I_NSTACK },
    [0x11c] = { "Golden Shovel",        I_ITEM|I_NSTACK },
    [0x11d] = { "Golden Pickaxe",       I_ITEM|I_NSTACK },
    [0x11e] = { "Golden Axe",           I_ITEM|I_NSTACK },
    [0x11f] = { "String",               I_ITEM },

    [0x120] = { "Feather",              I_ITEM },
    [0x121] = { "Gunpowder",            I_ITEM },
    [0x122] = { "Wooden Hoe",           I_ITEM|I_NSTACK },
    [0x123] = { "Stone Hoe",            I_ITEM|I_NSTACK },
    [0x124] = { "Iron Hoe",             I_ITEM|I_NSTACK },
    [0x125] = { "Diamond Hoe",          I_ITEM|I_NSTACK },
    [0x126] = { "Golden Hoe",           I_ITEM|I_NSTACK },
    [0x127] = { "Wheat Seeds",          I_ITEM },
    [0x128] = { "Wheat",                I_ITEM },
    [0x129] = { "Bread",                I_ITEM|I_FOOD },
    [0x12a] = { "Leather Helmet",       I_ITEM|I_NSTACK },
    [0x12b] = { "Leather Chestplate",   I_ITEM|I_NSTACK },
    [0x12c] = { "Leather Leggings",     I_ITEM|I_NSTACK },
    [0x12d] = { "Leather Boots",        I_ITEM|I_NSTACK },
    [0x12e] = { "Chainmail Helmet",     I_ITEM|I_NSTACK },
    [0x12f] = { "Chainmail Chestplate", I_ITEM|I_NSTACK },

    [0x130] = { "Chainmail Leggings",   I_ITEM|I_NSTACK },
    [0x131] = { "Chainmail Boots",      I_ITEM|I_NSTACK },
    [0x132] = { "Iron Helmet",          I_ITEM|I_NSTACK },
    [0x133] = { "Iron Chestplate",      I_ITEM|I_NSTACK },
    [0x134] = { "Iron Leggings",        I_ITEM|I_NSTACK },
    [0x135] = { "Iron Boots",           I_ITEM|I_NSTACK },
    [0x136] = { "Diamond Helmet",       I_ITEM|I_NSTACK },
    [0x137] = { "Diamond Chestplate",   I_ITEM|I_NSTACK },
    [0x138] = { "Diamond Leggings",     I_ITEM|I_NSTACK },
    [0x139] = { "Diamond Boots",        I_ITEM|I_NSTACK },
    [0x13a] = { "Golden Helmet",        I_ITEM|I_NSTACK },
    [0x13b] = { "Golden Chestplate",    I_ITEM|I_NSTACK },
    [0x13c] = { "Golden Leggings",      I_ITEM|I_NSTACK },
    [0x13d] = { "Golden Boots",         I_ITEM|I_NSTACK },
    [0x13e] = { "Flint",                I_ITEM },
    [0x13f] = { "Porkchop",             I_ITEM },

    [0x140] = { "Cooked Porkchop",      I_ITEM|I_FOOD },
    [0x141] = { "Painting",             I_ITEM },
    [0x142] = { "Golden Apple",         I_ITEM,
                { NULL, "Enchanted" }, },
    [0x143] = { "Sign",                 I_ITEM|I_S16 },
    [0x144] = { "Wooden Door",          I_ITEM },
    [0x145] = { "Bucket",               I_ITEM|I_S16 },
    [0x146] = { "Water Bucket",         I_ITEM|I_NSTACK },
    [0x147] = { "Lava Bucket",          I_ITEM|I_NSTACK },
    [0x148] = { "Minecart",             I_ITEM|I_NSTACK },
    [0x149] = { "Saddle",               I_ITEM|I_NSTACK },
    [0x14a] = { "Iron Door",            I_ITEM },
    [0x14b] = { "Redstone",             I_ITEM },
    [0x14c] = { "Snowball",             I_ITEM|I_S16 },
    [0x14d] = { "Boat",                 I_ITEM|I_NSTACK },
    [0x14e] = { "Leather",              I_ITEM },
    [0x14f] = { "Milk Bucket",          I_ITEM|I_NSTACK },

    [0x150] = { "Brick",                I_ITEM },
    [0x151] = { "Clay Ball",            I_ITEM },
    [0x152] = { "Reeds",                I_ITEM },
    [0x153] = { "Paper",                I_ITEM },
    [0x154] = { "Book",                 I_ITEM },
    [0x155] = { "Slime Ball",           I_ITEM },
    [0x156] = { "Chest Minecart",       I_ITEM|I_NSTACK },
    [0x157] = { "Furnace Minecart",     I_ITEM|I_NSTACK },
    [0x158] = { "Egg",                  I_ITEM|I_S16 },
    [0x159] = { "Compass",              I_ITEM },
    [0x15a] = { "Fishing Rod",          I_ITEM|I_NSTACK },
    [0x15b] = { "Clock",                I_ITEM },
    [0x15c] = { "Glowstone Dust",       I_ITEM },
    [0x15d] = { "Fish",                 I_ITEM|I_MTYPE,
                { NULL, "Salmon", "Clownfish", "Pufferfish" }, },
    [0x15e] = { "Cooked Fish",          I_ITEM|I_MTYPE|I_FOOD,
                { NULL, "Salmon", "Clownfish", "Pufferfish" }, },
    [0x15f] = { "Dye",                  I_ITEM|I_MTYPE,
                { "Ink Sac", "Rose Red", "Cactus Green", "Cocoa Beans",
                  "Lapis Lazuli", "Purple", "Cyan", "L.Gray",
                  "Gray", "Pink", "Lime", "Dandelion Yellow",
                  "L.Blue", "Magenta", "Orange", "Bone Meal" }, },

    [0x160] = { "Bone",                 I_ITEM },
    [0x161] = { "Sugar",                I_ITEM },
    [0x162] = { "Cake",                 I_ITEM|I_NSTACK },
    [0x163] = { "Bed",                  I_ITEM|I_NSTACK },
    [0x164] = { "Repeater",             I_ITEM },
    [0x165] = { "Cookie",               I_ITEM },
    [0x166] = { "Filled Map",           I_ITEM },
    [0x167] = { "Shears",               I_ITEM|I_NSTACK },
    [0x168] = { "Melon",                I_ITEM|I_FOOD },
    [0x169] = { "Pumpkin Seeds",        I_ITEM },
    [0x16a] = { "Melon Seeds",          I_ITEM },
    [0x16b] = { "Raw Beef",             I_ITEM },
    [0x16c] = { "Cooked Beef",          I_ITEM|I_FOOD },
    [0x16d] = { "Chicken",              I_ITEM },
    [0x16e] = { "Cooked Chicken",       I_ITEM|I_FOOD },
    [0x16f] = { "Rotten Flesh",         I_ITEM },

    [0x170] = { "Ender Pearl",          I_ITEM|I_S16 },
    [0x171] = { "Blaze Rod",            I_ITEM },
    [0x172] = { "Ghast Tear",           I_ITEM },
    [0x173] = { "Gold Nugget",          I_ITEM },
    [0x174] = { "Nether Wart",          I_ITEM },
    [0x175] = { "Potion",               I_ITEM|I_NSTACK, }, //TODO: potion names
    [0x176] = { "Glass Bottle",         I_ITEM },
    [0x177] = { "Spider Eye",           I_ITEM },
    [0x178] = { "Fermented Spider Eye", I_ITEM },
    [0x179] = { "Blaze Powder",         I_ITEM },
    [0x17a] = { "Magma Cream",          I_ITEM },
    [0x17b] = { "Brewing Stand",        I_ITEM },
    [0x17c] = { "Cauldron",             I_ITEM },
    [0x17d] = { "Ender Eye",            I_ITEM },
    [0x17e] = { "Speckled Melon",       I_ITEM },
    [0x17f] = { "Spawn Egg",            I_ITEM },

    [0x180] = { "Experience Bottle",    I_ITEM },
    [0x181] = { "Fire Charge",          I_ITEM },
    [0x182] = { "Writable Book",        I_ITEM|I_NSTACK },
    [0x183] = { "Written Book",         I_ITEM|I_NSTACK },
    // Written books are actually stackable (16), but only if identical
    // We'll keep it as NSTACK, until we have a better way to handle it
    [0x184] = { "Emerald",              I_ITEM },
    [0x185] = { "Item Frame",           I_ITEM },
    [0x186] = { "Flower Pot",           I_ITEM },
    [0x187] = { "Carrot",               I_ITEM },
    [0x188] = { "Potato",               I_ITEM },
    [0x189] = { "Baked Potato",         I_ITEM|I_FOOD },
    [0x18a] = { "Poisonous Potato",     I_ITEM },
    [0x18b] = { "Map",                  I_ITEM },
    [0x18c] = { "Golden Carrot",        I_ITEM|I_FOOD },
    [0x18d] = { "Skull",                I_ITEM },
    [0x18e] = { "Carrot on a Stick",    I_ITEM|I_NSTACK },
    [0x18f] = { "Nether Star",          I_ITEM },

    [0x190] = { "Pumpkin Pie",          I_ITEM },
    [0x191] = { "Fireworks",            I_ITEM },
    [0x192] = { "Firework Charge",      I_ITEM },
    [0x193] = { "Enchanted Book",       I_ITEM|I_NSTACK },
    [0x194] = { "Comparator",           I_ITEM },
    [0x195] = { "Netherbrick",          I_ITEM },
    [0x196] = { "Quartz",               I_ITEM },
    [0x197] = { "TNT Minecart",         I_ITEM|I_NSTACK },
    [0x198] = { "Hopper Minecart",      I_ITEM|I_NSTACK },
    [0x199] = { "Prismarine Shard",     I_ITEM },
    [0x19a] = { "Prismarine Crystal",   I_ITEM },
    [0x19b] = { "Rabbit",               I_ITEM },
    [0x19c] = { "Cooked Rabbit",        I_ITEM|I_FOOD },
    [0x19d] = { "Rabbit Stew",          I_ITEM|I_NSTACK|I_FOOD },
    [0x19e] = { "Rabbit Foot",          I_ITEM },
    [0x19f] = { "Rabbit Hide",          I_ITEM },

    [0x1a0] = { "Armor Stand",          I_ITEM|I_S16 },
    [0x1a1] = { "Iron Horse Armor",     I_ITEM|I_NSTACK },
    [0x1a2] = { "Golden Horse Armor",   I_ITEM|I_NSTACK },
    [0x1a3] = { "Diamond Horse Armor",  I_ITEM|I_NSTACK },
    [0x1a4] = { "Lead",                 I_ITEM },
    [0x1a5] = { "Name Tag",             I_ITEM },
    [0x1a6] = { "Command Block Minecart",   I_ITEM|I_NSTACK },
    [0x1a7] = { "Mutton",               I_ITEM },
    [0x1a8] = { "Cooked Mutton",        I_ITEM|I_FOOD },
    [0x1a9] = { "Banner",               I_ITEM|I_NSTACK },
    [0x1aa] = { "End Crystal",          I_ITEM },
    [0x1ab] = { "Spruce Door",          I_ITEM },
    [0x1ac] = { "Birch Door",           I_ITEM },
    [0x1ad] = { "Jungle Door",          I_ITEM },
    [0x1ae] = { "Acacia Door",          I_ITEM },
    [0x1af] = { "Dark Oak Door",        I_ITEM },

    [0x1b0] = { "Chorus Fruit",         I_ITEM|I_FOOD },
    [0x1b1] = { "Chorus Fruit Popped",  I_ITEM },
    [0x1b2] = { "Beetroot",             I_ITEM|I_FOOD },
    [0x1b3] = { "Beetroot Seeds",       I_ITEM },
    [0x1b4] = { "Beetroot Soup",        I_ITEM|I_FOOD },
    [0x1b5] = { "Dragon Breath",        I_ITEM },
    [0x1b6] = { "Splash Potion",        I_ITEM },
    [0x1b7] = { "Spectral Arrow",       I_ITEM },
    [0x1b8] = { "Tipped Arrow",         I_ITEM },
    [0x1b9] = { "Lingering Potion",     I_ITEM },
    [0x1ba] = { "Shield",               I_ITEM },
    [0x1bb] = { "Elytra",               I_ITEM },
    [0x1bc] = { "Spruce Boat",          I_ITEM },
    [0x1bd] = { "Birch Boat",           I_ITEM },
    [0x1be] = { "Jungle Boat",          I_ITEM },
    [0x1bf] = { "Acacia Boat",          I_ITEM },

    [0x1c0] = { "Dark Oak Boat",        I_ITEM },
    [0x1c1] = { "Totem of Undying",     I_ITEM },
    [0x1c2] = { "Shulker Shell",        I_ITEM },

    [0x1c4] = { "Iron Nugget",          I_ITEM },

    [0x8d0] = { "Record 13",            I_ITEM|I_NSTACK },
    [0x8d1] = { "Record Cat",           I_ITEM|I_NSTACK },
    [0x8d2] = { "Record Blocks",        I_ITEM|I_NSTACK },
    [0x8d3] = { "Record Chirp",         I_ITEM|I_NSTACK },
    [0x8d4] = { "Record Far",           I_ITEM|I_NSTACK },
    [0x8d5] = { "Record Mall",          I_ITEM|I_NSTACK },
    [0x8d6] = { "Record Mellohi",       I_ITEM|I_NSTACK },
    [0x8d7] = { "Record Stal",          I_ITEM|I_NSTACK },
    [0x8d8] = { "Record Strad",         I_ITEM|I_NSTACK },
    [0x8d9] = { "Record Ward",          I_ITEM|I_NSTACK },
    [0x8da] = { "Record 11",            I_ITEM|I_NSTACK },
    [0x8db] = { "Record Wait",          I_ITEM|I_NSTACK },


    [0x8ff] = { NULL }, //Terminator
};

// given a specific block ID and meta, return the meta value corresponding
// to the basic state of the block
int get_base_meta(int id, int meta) {
    // only accept existing block types
    const item_id * it = &ITEMS[id];
    assert(it->name);

    // if the block has no I_MTYPE subtypes, so base meta is 0
    if (!(it->flags&I_MTYPE)) return 0;

    // block meta is used for I_MTYPE but not used for position/state => base meta as is
    if (!(it->flags&(I_MPOS|I_STATE_MASK))) return meta;

    // unless I_MTYPE flag is specified, item's meta is used for damage only
    if (id>=0x100) return 0;

    // everything else needs to be determined individually
    switch (id) {
        case 0x06: // Sapling
        case 0x2c: // Stone slab
        case 0x7e: // Wooden slab
        case 0xaf: // Large Flower
            return meta&7;

        case 0x11: // Wood
        case 0x12: // Leaves
        case 0xa1: // Leaves2
        case 0xa2: // Wood2
        case 0xaa: // Haybales
        case 0xca: // Purpur Pillar
            return meta&3;

        case 0x9b: // Quartz block
            return (meta>2) ? 2 : meta;

        default:
            printf("id=%d\n",id);
            assert(0);
    }
    return meta;
}

// mapping macro for the block types that have different IDs
// for the block and the item used for placement of such block
//#define BI(block,item) if (mat.bid == block) return BLOCKTYPE(item,0)

#if 0
//moved to db module
int get_base_material(blid_t blk_id) {
    const char *blk_name = db_get_blk_name(blk_id);
    int item_id = db_get_item_id(blk_name);
    assert(item_id > 0);

    //TODO: verify which blocks require a different type of material for building


    //DISABLED: transition to dev_3.0
    assert(mat.bid <0x100);

    // material for doubleslabs is the slab with the next block ID and same meta
    if (ITEMS[mat.bid].flags&I_DSLAB) mat.bid++;

    BI(55,331);   // Redstone Wire -> Redstone
    BI(132,287);  // Tripwire -> String

    BI(92,354);   // Cake
    BI(117,379);  // Brewing Stand
    BI(118,380);  // Cauldron
    BI(140,390);  // Flower pot

    BI(59,295);   // Wheat -> Wheat Seeds
    BI(83,338);   // Sugar Cane
    BI(115,372);  // Nether Wart
    BI(141,391);  // Carrot Plant -> Carrot
    BI(142,392);  // Potato Plant -> Potato
    BI(104,361);  // Pumpkin Stem -> Pumpkin Seeds
    BI(105,362);  // Melon Stem -> Melon Seeds
    BI(207,435);  // Beetroot Plant -> Betroot Seeds
    if (mat.bid == 127) return BLOCKTYPE(351,3);  // Cocoa -> Cocoa Beans (which is also a Brown Dye, duh)

    BI(63,323);   // Standing Sign -> Sign
    BI(68,323);   // Wall Sign -> Sign
    BI(176,425);  // Standing Banner -> Banner
    BI(177,425);  // Wall Banner -> Banner

    BI(26,355);   // Bed

    BI(64,324);   // Oak Door
    BI(193,427);  // Spruce Door
    BI(194,428);  // Birch Door
    BI(195,429);  // Jungle Door
    BI(196,430);  // Acacia Door
    BI(197,431);  // Dark Oak Door
    BI(71,330);   // Iron Door

    BI(74,73);    // Redstone Ore (lit -> unlit)
    BI(75,76);    // Redstone Torch (inactive -> active)
    BI(93,356);   // Redstone Repeater (inactive)
    BI(94,356);   // Redstone Repeater (active)
    BI(124,123);  // Redstone Lamp (lit -> unlit)
    BI(149,404);  // Redstone Comparator
    BI(178,151);  // Daylight Sensor (inverted -> normal)

    BI(8,326);    // Flowing Water -> Water Bucket
    BI(9,326);    // Water -> Water Bucket
    BI(10,327);   // Flowing Lava -> Lava Bucket
    BI(11,327);   // Lava -> Lava Bucket

    mat.meta = get_base_meta(mat.bid, mat.meta);
    return mat;

}
#endif

// select a block material that matches best a block derived from it,
// e.g. Oak Woodplanks for Oak stairs or slabs
bid_t get_base_block_material(bid_t mat) {
    switch (mat.bid) {
        case 0x2c: // Stone Slab
        case 0x2b: // Double Slab
            switch (mat.meta&0x7) {
                case  0: return BLOCKTYPE(0x01,0); // Stone (plain)
                case  1: return BLOCKTYPE(0x18,0); // Sandstone (plain)
                case  2: return BLOCKTYPE(0x05,0); // Woodplanks (oak)
                case  3: return BLOCKTYPE(0x04,0); // Cobblestone
                case  4: return BLOCKTYPE(0x2d,0); // Brick
                case  5: return BLOCKTYPE(0x62,0); // Stonebrick (plain)
                case  6: return BLOCKTYPE(0x70,0); // Netherbrick
                case  7: return BLOCKTYPE(0x9b,0); // Quartz block (plain)
                default: assert(0);
            }
        case 0x7d: // Double Wooden Slab
        case 0x7e: // Wooden Slab
            return BLOCKTYPE(0x05,(mat.meta&0x7)); // Woodplanks of the same type

        // wooden stairs
        case 0x35: return BLOCKTYPE(0x05,0); // Woodplanks (oak)
        case 0x86: return BLOCKTYPE(0x05,1); // Woodplanks (spruce)
        case 0x87: return BLOCKTYPE(0x05,2); // Woodplanks (birch)
        case 0x88: return BLOCKTYPE(0x05,3); // Woodplanks (jungle)
        case 0xa3: return BLOCKTYPE(0x05,4); // Woodplanks (acacia)
        case 0xa4: return BLOCKTYPE(0x05,5); // Woodplanks (dark oak)

        // stone stairs
        case 0x43: return BLOCKTYPE(0x04,0); // Cobblestone
        case 0x6c: return BLOCKTYPE(0x2d,0); // Brick
        case 0x6d: return BLOCKTYPE(0x62,0); // Stonebrick (plain)
        case 0x72: return BLOCKTYPE(0x70,0); // Netherbrick
        case 0x80: return BLOCKTYPE(0x18,0); // Sandstone (plain)
        case 0x9c: return BLOCKTYPE(0x9b,0); // Quartz block (plain)

        // red sandstone stairs, d-slabs, slabs
        case 0xb4:
        case 0xb5:
        case 0xb6:
            return BLOCKTYPE(0xb3,0); // Red Sandstone (plain)

        // purpur stairs, d-slabs, slabs
        case 0xcb:
        case 0xcc:
        case 0xcd:
            return BLOCKTYPE(0xc9,0); // Purpur Block
    }
    return mat;
}

const char * get_mat_name(char *buf, int id, int meta) {
    if (id<0 || id>=db_num_items) {
        sprintf(buf, "-");
        return buf;
    }

    int bmeta = get_base_meta(id, meta);

    const item_id *it = &ITEMS[id];
    if (it->name) {
        int pos = sprintf(buf, "%s", it->name);

        if ((it->flags&I_MTYPE) && it->mname[bmeta])
            pos += sprintf(buf+pos, " (%s)",it->mname[bmeta]);
#if 0
        //TODO: support other block types with I_MPOS
        if (it->flags&I_SLAB)
            sprintf(buf+pos, " (%s)",(meta&8)?"upper":"lower");
#endif
    }
    else {
        printf(buf, "???");
    }
    return buf;

}

////////////////////////////////////////////////////////////////////////////////
// Orientation

static const char * _DIRNAME[] = { "any", "up", "down", "south", "north", "east", "west" };
const char ** DIRNAME = _DIRNAME+1;

// Meta mappings for the rotation-dependent blocks
/*
  How to interpret:

  The array is selected by the functions responsible for meta rotation/flipping,
  according to the block's flags (i.e. I_STAIR)

  The array is a 4*16 int8_t elements. The outer index defines block's original
  meta value (0..15). The inner index defines the 4 meta values corresponding
  to the orientations South, North, East, West, in this order (this index can be
  determined using the standard direction constant DIR_* and substracting 2).
  The int8_t value determines the meta value of this block in this orientation.

  1. Take the current meta of the block and look up the 4-element row.
  This defines a "group" of meta values related to the original value, with
  different azimuth orientations, but sharing all other orientation or type.
  E.g. a meta value corresponding to an upside-down stairs block pointing north
  will reference a group of values for all upside-down stair blocks, pointing
  in the 4 worlds directions.
  If the row pointer is NULL - this meta does not exist or does not specify
  rotational position. E.g. meta value 5 for torches corresponds to "on ground"
  position which is not rotable.

  2. Locate the original meta value in the row.
  The obtained index will tell you the current azimuth orientation of the block.
  E.g. a stairs block with a meta value 5 can be found on index 3 - i.e. it is
  "west-oriented"

  3. Select the desired rotation of the block.
  E.g. if we want to rotate the block counter-clockwise, west should become south.

  4. Look up the meta value at the index corresponding to the new selected direction
  in the same group row.
  E.g. the direction "south" corresponds to the index 0 in the row. Therefore,
  the meta value for the new orientation will be 6
*/

// meta value group - orientation mapping
typedef struct {
    int inuse;
    int8_t meta[4];
} metagroup;

#define METAO(m,s,n,e,w) [m] = {1,{s,n,e,w}}

//          S N E W

// I_STAIR
static metagroup MM_STAIR[16] = {
    METAO(0,2,3,0,1),
    METAO(1,2,3,0,1),
    METAO(2,2,3,0,1),
    METAO(3,2,3,0,1),
    METAO(4,6,7,4,5),
    METAO(5,6,7,4,5),
    METAO(6,6,7,4,5),
    METAO(7,6,7,4,5),
};

// I_TORCH
static metagroup MM_TORCH[16] = {
    METAO(1,3,4,1,2),
    METAO(2,3,4,1,2),
    METAO(3,3,4,1,2),
    METAO(4,3,4,1,2),
};

// I_LOG
// Wood/Wood2 logs - N/S,N/S,E/W,E/W orientation
static metagroup MM_LOG[16] = {
    // Oak/Acacia
    METAO(4,8,8,4,4),
    METAO(8,8,8,4,4),
    // Spruce/Dark Oak
    METAO(5,9,9,5,5),
    METAO(9,9,9,5,5),
    // Birch
    METAO(6,10,10,6,6),
    METAO(10,10,10,6,6),
    //Jungle
    METAO(7,11,11,7,7),
    METAO(11,11,11,7,7),
};

// I_ONWALL (ladders/banners/signs), but also
// Hoppers, Pistons, Droppers and Dispensers
static metagroup MM_ONWALL[16] = {
    METAO(2,3,2,5,4),
    METAO(3,3,2,5,4),
    METAO(4,3,2,5,4),
    METAO(5,3,2,5,4),
    METAO(10,11,10,13,12),
    METAO(11,11,10,13,12),
    METAO(12,11,10,13,12),
    METAO(13,11,10,13,12),
};

// Redstone repeaters and comparators
static metagroup MM_RSRC[16] = {
    METAO(0,2,0,1,3),
    METAO(1,2,0,1,3),
    METAO(2,2,0,1,3),
    METAO(3,2,0,1,3),
    METAO(4,6,4,5,7),
    METAO(5,6,4,5,7),
    METAO(6,6,4,5,7),
    METAO(7,6,4,5,7),
    METAO(8,10,8,9,11),
    METAO(9,10,8,9,11),
    METAO(10,10,8,9,11),
    METAO(11,10,8,9,11),
    METAO(12,14,12,13,15),
    METAO(13,14,12,13,15),
    METAO(14,14,12,13,15),
    METAO(15,14,12,13,15),
};

static metagroup MM_DOOR[16] = {
    METAO(0,3,1,2,0), // bottom half, closed doors
    METAO(1,3,1,2,0),
    METAO(2,3,1,2,0),
    METAO(3,3,1,2,0),
    METAO(4,7,5,6,4), // bootom half, opened doors
    METAO(5,7,5,6,4),
    METAO(6,7,5,6,4),
    METAO(7,7,5,6,4),
    METAO(8,8,8,8,8), // top half, hinges on the left
    METAO(9,9,9,9,9), // top half, hinges on the right
};

static metagroup MM_TRAPDOOR[16] = {
    METAO(0,0,1,2,3),
    METAO(1,0,1,2,3),
    METAO(2,0,1,2,3),
    METAO(3,0,1,2,3),
    METAO(4,4,5,6,7),
    METAO(5,4,5,6,7),
    METAO(6,4,5,6,7),
    METAO(7,4,5,6,7),
    METAO(8,8,9,10,11),
    METAO(9,8,9,10,11),
    METAO(10,8,9,10,11),
    METAO(11,8,9,10,11),
    METAO(12,12,13,14,15),
    METAO(13,12,13,14,15),
    METAO(14,12,13,14,15),
    METAO(15,12,13,14,15),
};

static metagroup MM_BED[16] = {
    METAO(0,0,2,3,1),
    METAO(1,0,2,3,1),
    METAO(2,0,2,3,1),
    METAO(3,0,2,3,1),
    METAO(4,4,6,7,5),
    METAO(5,4,6,7,5),
    METAO(6,4,6,7,5),
    METAO(7,4,6,7,5),
    METAO(8,8,10,11,9),
    METAO(9,8,10,11,9),
    METAO(10,8,10,11,9),
    METAO(11,8,10,11,9),
    METAO(12,12,14,15,13),
    METAO(13,12,14,15,13),
    METAO(14,12,14,15,13),
    METAO(15,12,14,15,13),
};

static metagroup MM_LEVER[16] = {
    METAO(1,3,4,1,2), // lever on the side
    METAO(2,3,4,1,2),
    METAO(3,3,4,1,2),
    METAO(4,3,4,1,2),
    METAO(0,7,7,0,0), // lever on the bottom
    METAO(7,7,7,0,0),
    METAO(5,5,5,6,6), // lever on the top
    METAO(6,5,5,6,6),
};

static metagroup MM_QUARTZ[16] = {
    METAO(3,4,4,3,3), // East-West
    METAO(4,4,4,3,3), // North-South
};

// I_OBSERVER - it's almost same as droppers/dispensers, but nooo,
// Mojang had to use different codes for direction again
static metagroup MM_OBSERVER[16] = {
    METAO(2,2,3,4,5),
    METAO(3,2,3,4,5),
    METAO(4,2,3,4,5),
    METAO(5,2,3,4,5),
};

#define GETMETAGROUP(mmname) mmname[b.meta].inuse ? mmname[b.meta].meta : NULL
static inline int8_t *get_metagroup(bid_t b) {
    uint64_t flags = ITEMS[b.bid].flags;
    if (flags&I_STAIR)  return GETMETAGROUP(MM_STAIR);
    if (flags&I_TORCH)  return GETMETAGROUP(MM_TORCH);
    if (flags&I_LOG)    return GETMETAGROUP(MM_LOG);
    if (flags&I_ONWALL) return GETMETAGROUP(MM_ONWALL);
    if (flags&I_RSRC)   return GETMETAGROUP(MM_RSRC);
    if (flags&I_DOOR)   return GETMETAGROUP(MM_DOOR);
    if (flags&I_TDOOR)  return GETMETAGROUP(MM_TRAPDOOR);
    if (b.bid == 26)    return GETMETAGROUP(MM_BED);
    if (flags&I_CHEST)  return GETMETAGROUP(MM_ONWALL);
    if (b.bid == 69)    return GETMETAGROUP(MM_LEVER);
    if (b.bid==77 || b.bid==143) return GETMETAGROUP(MM_TORCH);
    if (b.bid==155)     return GETMETAGROUP(MM_QUARTZ);
    if (flags&I_GATE)   return GETMETAGROUP(MM_BED);
    if (flags&I_OBSERVER)   return GETMETAGROUP(MM_OBSERVER);
    if (flags&I_TERRACOTA)  return GETMETAGROUP(MM_BED);

    // Redstone devices, hoppers and end rods can use ONWALL set
    if ((flags&I_RSDEV) || b.bid==154 || b.bid==198)
        return GETMETAGROUP(MM_ONWALL);

    return NULL; // default - no orientation mapping
}

// get the orientation of the block by its meta - as one of the DIR_xxx constants
static inline int get_orientation(bid_t b) {
    int8_t *group = get_metagroup(b);
    if (!group) return DIR_ANY;

    int i;
    for(i=0; i<4; i++)
        if (group[i] == b.meta)
            return i+2;
    assert(0);
}

// generic rotation map to calculate clockwise rotation for a direction
int ROT_MAP[][4] = {
    [DIR_UP]    = { DIR_UP,    DIR_UP,    DIR_UP,    DIR_UP,    },
    [DIR_DOWN]  = { DIR_DOWN,  DIR_DOWN,  DIR_DOWN,  DIR_DOWN,  },
    [DIR_NORTH] = { DIR_NORTH, DIR_EAST,  DIR_SOUTH, DIR_WEST,  },
    [DIR_EAST]  = { DIR_EAST,  DIR_SOUTH, DIR_WEST,  DIR_NORTH, },
    [DIR_SOUTH] = { DIR_SOUTH, DIR_WEST,  DIR_NORTH, DIR_EAST,  },
    [DIR_WEST]  = { DIR_WEST,  DIR_NORTH, DIR_EAST,  DIR_SOUTH, },
};

// rotate a block's meta clockwise given number of times
bid_t rotate_meta(bid_t b, int times) {
    int8_t * mg = get_metagroup(b);
    if (!mg) return b;

    int mo = get_orientation(b);
    int rmo = ROT_MAP[mo][times&3];
    b.meta = mg[rmo-2];
    return b;
}

// get the number of clockwise 90-degree rotations to get from one direction to another
int numrot(int from_dir, int to_dir) {
    int i;
    for(i=0; i<4; i++)
        if (ROT_MAP[from_dir][i] == to_dir)
            return i;
    return -1;
}

// calculate metas for a flipped version of a block

int FLIP_MAP[][2] = {
    [DIR_UP]    = { DIR_UP,    DIR_UP,    },
    [DIR_DOWN]  = { DIR_DOWN,  DIR_DOWN,  },
    [DIR_NORTH] = { DIR_NORTH, DIR_SOUTH, },
    [DIR_SOUTH] = { DIR_SOUTH, DIR_NORTH, },
    [DIR_EAST]  = { DIR_WEST,  DIR_EAST,  },
    [DIR_WEST]  = { DIR_EAST,  DIR_WEST,  },
};

bid_t flip_meta(bid_t b, char mode) {
    assert(mode=='x' || mode=='z');

    int8_t * mg = get_metagroup(b);
    if (!mg) return b;

    int mo = get_orientation(b);
    int fmo = FLIP_MAP[mo][mode=='z'];
    b.meta = mg[fmo-2];
    return b;
}

bid_t flip_meta_y(bid_t b) {
    uint64_t flags = ITEMS[b.bid].flags;
    if (flags&I_STAIR) b.meta^=4; // flip the up/down bit
    if (flags&I_SLAB)  b.meta^=8; // flip the up/down bit
    //TODO: support other blocks with up/down orientation, like Droppers
    return b;
}

////////////////////////////////////////////////////////////////////////////////
// Biomes info

// Color values courtesy of AMIDST
const biome_id BIOMES[256] = {
    [  0] = { "Ocean", 0x000070 },
    [  1] = { "Plains", 0x8db360 },
    [  2] = { "Desert", 0xfa9418 },
    [  3] = { "Extreme Hills", 0x606060 },
    [  4] = { "Forest", 0x056621 },
    [  5] = { "Taiga", 0x0b6659 },
    [  6] = { "Swampland", 0x07f9b2 },
    [  7] = { "River", 0x0000ff },
    [  8] = { "Hell", 0xff0000 },
    [  9] = { "The End", 0x0a000a },
    [ 10] = { "Frozen Ocean", 0x9090a0 },
    [ 11] = { "Frozen River", 0xa0a0ff },
    [ 12] = { "Ice Plains", 0xffffff },
    [ 13] = { "Ice Mountains", 0xa0a0a0 },
    [ 14] = { "Mushroom Island", 0xff00ff },
    [ 15] = { "Mushroom Island Shore", 0xa000ff },
    [ 16] = { "Beach", 0xfade55 },
    [ 17] = { "Desert Hills", 0xd25f12 },
    [ 18] = { "Forest Hills", 0x22551c },
    [ 19] = { "Taiga Hills", 0x163933 },
    [ 20] = { "Extreme Hills Edge", 0x72789a },
    [ 21] = { "Jungle", 0x537b09 },
    [ 22] = { "Jungle Hills", 0x2c4205 },
    [ 23] = { "Jungle Edge", 0x628b17 },
    [ 24] = { "Deep Ocean", 0x000030 },
    [ 25] = { "Stone Beach", 0xa2a284 },
    [ 26] = { "Cold Beach", 0xfaf0c0 },
    [ 27] = { "Birch Forest", 0x307444 },
    [ 28] = { "Birch Forest Hills", 0x1f5f32 },
    [ 29] = { "Roofed Forest", 0x40511a },
    [ 30] = { "Cold Taiga", 0x31554a },
    [ 31] = { "Cold Taiga Hills", 0x243f36 },
    [ 32] = { "Mega Taiga", 0x596651 },
    [ 33] = { "Mega Taiga Hills", 0x454f3e },
    [ 34] = { "Extreme Hills+", 0x507050 },
    [ 35] = { "Savanna", 0xbdb25f },
    [ 36] = { "Savanna Plateau", 0xa79d64 },
    [ 37] = { "Mesa", 0xd94515 },
    [ 38] = { "Mesa Plateau F", 0xb09765 },
    [ 39] = { "Mesa Plateau", 0xca8c65 },
    [127] = { "Sky", 0x8080ff },
    [128] = { "Plains M", 0x8db360 },
    [129] = { "Sunflower Plains", 0xb5db88 },
    [130] = { "Desert M", 0xffbc40 },
    [131] = { "Extreme Hills M", 0x888888 },
    [132] = { "Flower Forest", 0x2d8e49 },
    [133] = { "Taiga M", 0x338e81 },
    [134] = { "Swampland M", 0x2fffda },
    [135] = { "River M", 0x2828ff },
    [136] = { "Hell M", 0xff2828 },
    [138] = { "Frozen Ocean M", 0xb8b8c8 },
    [139] = { "Frozen River M", 0xc8c8ff },
    [140] = { "Ice Plains Spikes", 0xb4dcdc },
    [141] = { "Ice Mountains M", 0xc8c8c8 },
    [142] = { "Mushroom Island M", 0xff28ff },
    [143] = { "Mushroom Island Shore M", 0xc828ff },
    [144] = { "Beach M", 0xffff7d },
    [145] = { "Desert Hills M", 0xfa873a },
    [146] = { "Forest Hills M", 0x4a7d44 },
    [147] = { "Taiga Hills M", 0x3e615b },
    [148] = { "Extreme Hills Edge M", 0x9aa0c2 },
    [149] = { "Jungle M", 0x7ba331 },
    [150] = { "Jungle Hills M", 0x546a2d },
    [151] = { "Jungle Edge M", 0x8ab33f },
    [152] = { "Deep Ocean M", 0x282858 },
    [153] = { "Stone Beach M", 0xcacaac },
    [154] = { "Cold Beach M", 0xffffe8 },
    [155] = { "Birch Forest M", 0x589c6c },
    [156] = { "Birch Forest Hills M", 0x47875a },
    [157] = { "Roofed Forest M", 0x687942 },
    [158] = { "Cold Taiga M", 0x597d72 },
    [159] = { "Cold Taiga Hills M", 0x4c675e },
    [160] = { "Mega Spruce Taiga", 0x818e79 },
    [161] = { "Mega Spruce Taiga (Hills)", 0x6d7766 },
    [162] = { "Extreme Hills+ M", 0x789878 },
    [163] = { "Savanna M", 0xe5da87 },
    [164] = { "Savanna Plateau M", 0xcfc58c },
    [165] = { "Mesa (Bryce)", 0xff6d3d },
    [166] = { "Mesa Plateau F M", 0xd8bf8d },
    [167] = { "Mesa Plateau M", 0xf2b48d },
    [255] = { "Sky M", 0xa8a8ff },
};
