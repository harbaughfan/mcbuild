/*
 Authors:
 Copyright 2012-2016 by Eduard Broese <ed.broese@gmx.de>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version
 2 of the License, or (at your option) any later version.
*/

#pragma once

/*
  entity : entities and entity metadata handling
*/

#include <stdint.h>

#include "mcp_types.h"
#include "slot.h"

////////////////////////////////////////////////////////////////////////////////

typedef enum {
    //// Superclasses
    //// Superclasses
    IllegalEntityType   = -1,
    Entity              = 256-1,
    Living              = 256-2,
    Ageable             = 256-3,
    Hanging             = 256-9,
    Insentinent         = 256-10,
    Ambient             = 256-11,
    Creature            = 256-12,
    Animal              = 256-13,
    Golem               = 256-14,
    Flying              = 256-15,
    Monster             = 256-17,
    TameableAnimal      = 256-18,
    AbstractFireball    = 256-19,
    AbstractFish        = 256-20,
    AbstractHorse       = 256-21,
    AbstractIllager     = 256-22,
    AbstractSkeleton    = 256-23,
    Arrow               = 256-24,
    ChestedHorse        = 256-25,
    MinecartContainer   = 256-26,
    SpellcasterIllager  = 256-27,
    Throwable           = 256-28,
    WaterMob            = 256-29,
    Weather             = 256-30,
    Raider              = 256-31,
    BasePiglin          = 256-32,
    AbstractVillager    = 256-33,


    //// Mobs (SP_SpawnMob)
    Bat = 3,
    Bee = 4,
    Blaze = 5,
    Cat = 7,
    CaveSpider = 8,
    Chicken = 9,
    Cod = 10,
    Cow = 11,
    Creeper = 12,
    Dolphin = 13,
    Donkey = 14,
    Drowned = 16,
    ElderGuardian = 17,
    EnderDragon = 19,
    Enderman = 20,
    Endermite = 21,
    EvocationIllager = 22,
    ExperienceOrb = 24,
    Fox = 28,
    Ghast = 29,
    Giant = 30,
    Guardian = 31,
    Hoglin = 32,
    Horse = 33,
    Husk = 34,
    IllusionIllager = 35,
    IronGolem = 36,
    Llama = 42,
    MagmaCube = 44,
    Mule = 52,
    Mooshroom = 53,
    Ocelot = 54,
    Painting = 55,
    Panda = 56,
    Parrot = 57,
    Phantom = 58,
    Pig = 59,
    Piglin = 60,
    PiglinBrute = 61,
    Pillager = 62,
    PolarBear = 63,
    Pufferfish = 65,
    Rabbit = 66,
    Ravager = 67,
    Salmon = 68,
    Sheep = 69,
    Shulker = 70,
    Silverfish = 72,
    Skeleton = 73,
    SkeletonHorse = 74,
    Slime = 75,
    Snowman = 77,
    Spider = 80,
    Squid = 81,
    Stray = 82,
    Strider = 83,
    TraderLlama = 89,
    TropicalFish = 90,
    Turtle = 91,
    Vex = 92,
    Villager = 93,
    VindicationIllager = 94,
    WanderingTrader = 95,
    Witch = 96,
    Wither = 97,
    WitherSkeleton = 98,
    Wolf = 100,
    Zoglin = 101,
    Zombie = 102,
    ZombieHorse = 103,
    ZombieVillager = 104,
    ZombiePigman = 105,
    Player = 106,



    //// Objects (SP_SpawnObject)
    // Update: I think SpawnObject is using the entity id now
    Boat                = 6 ,
    Item                = 37 ,
    AreaEffectCloud     = 0 ,

    Minecart            = 45,
    ChestMinecart       = 46, // deprecated since 1.6
    MinecartFurnace     = 48, // deprecated since 1.6
    MinecartCommandBlock= 47, // deprecated

    LightningBolt       = 41,

    ActivatedTNT        = 64,
    EnderCrystal        = 18,

    TippedArrow         = 2,
    Snowball            = 78,
    Egg                 = 84,
    Fireball            = 76,
    FireCharge          = 39,
    ThrownEnderpearl    = 85,
    WitherSkull         = 99,
    ShulkerBullet       = 71,
    LlamaSpit           = 43, // moved from mob to object

    FallingBlock        = 26,
    ItemFrame           = 38,
    EyeOfEnder          = 24,
    Potion              = 87,
    FallingDragonEgg    = 255, // deprecated
    ThrownExpBottle     = 86,
    Fireworks           = 27,
    LeashKnot           = 40,
    ArmorStand          = 1,
    EvocationFangs      = 23, // moved from mob to object

    FishingFloat        = 107,
    SpectralArrow       = 79,
    DragonFireball      = 15,
    Trident             = 88, // added


    //// Terminating ID
    MaxEntityType       = 512,
} EntityType;

extern const char * METANAME[][32];
extern const EntityType ENTITY_HIERARCHY[];
extern const char * ENTITY_NAMES[];
const char * get_entity_name(char *buf, EntityType type);

////////////////////////////////////////////////////////////////////////////////

#define META_BYTE       0
#define META_VARINT     1
#define META_FLOAT      2
#define META_STRING     3
#define META_CHAT       4
#define META_OPTCHAT    5
#define META_SLOT       6
#define META_BOOL       7
#define META_VEC3       8
#define META_POS        9
#define META_OPTPOS     10
#define META_DIR        11
#define META_OPTUUID    12
#define META_BID        13
#define META_NBT        14
#define META_PARTICLE   15
#define META_VILLAGER   16
#define META_OPTVARINT  17
#define META_POSE       18
#define META_NONE       255

// single metadata key-value pair
typedef struct {
    uint8_t         key;
    uint32_t        type;
    union {
        uint8_t     b;
        int32_t     i;
        float       f;
        char        *str;
        slot_t      slot;
        uint8_t     bool;
        struct {
            float   fx;
            float   fy;
            float   fz;
        };
        pos_t       pos;    // OPTPOS with missing position is encoded as (int64_t)-1
        int32_t     dir;
        uuid_t      uuid;   // missing UUID is encoded in all-zeros
        uint32_t    block;
        nbt_t*      nbt;
        struct {
            uint32_t   vil1;
            uint32_t   vil2;
            uint32_t   vil3;
        };
        uint32_t    pose;
        //FIXME: particle metadata is ignored and discarded at the moment
    };
} metadata;

extern const char * METATYPES[];

metadata * clone_metadata(metadata *meta);
metadata * update_metadata(metadata *meta, metadata *upd);
void free_metadata(metadata *meta);
uint8_t * read_metadata(uint8_t *p, metadata **meta);
uint8_t * write_metadata(uint8_t *w, metadata *meta);
void dump_metadata(metadata *meta, EntityType et);
