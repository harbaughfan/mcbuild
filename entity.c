/*
 Authors:
 Copyright 2012-2016 by Eduard Broese <ed.broese@gmx.de>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version
 2 of the License, or (at your option) any later version.
*/

/*
  entity : entities and entity metadata handling
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define LH_DECLARE_SHORT_NAMES 1
#include <lh_buffers.h>
#include <lh_bytes.h>
#include <lh_debug.h>

#include "entity.h"
#include "mcp_packet.h"

// Entities

const char * METANAME[][32] = {
    [AbstractFireball] = { },
    [AbstractFish] = {
        [15] = "From bucket",
    },
    [AbstractHorse] = {
        [16] = "flags",
        [17] = "owner",
    },
    [AbstractIllager] = { },
    [AbstractMinecart] = {
        [7]  = "Shaking power",
        [8]  = "Shaking direction",
        [9]  = "Shaking multiplier",
        [10] = "Block id/data",
        [11] = "Block y",
        [12] = "Show block",
    },
    [AbstractMinecartCont] = { },
    [AbstractSkeleton] = { },
    [AbstractVillager] = {
        [16] = "Head shake timer",
    },
    [ActivatedTNT] = {
        [7]  = "Fuse time",
    },
    [Ageable] = {
        [15] = "Is baby",
    },
    [Ambient] = {
    },
    [Animal] = {
    },
    [AreaEffectCloud] = {
        [7]  = "Radius",
        [8]  = "Color",
        [9]  = "Single point",
        [10]  = "Particle ID",
    },
    [ArmorStand] = {
        [14] = "Armor stand flags",
        [15] = "Head position",
        [16] = "Body position",
        [17] = "L arm position",
        [18] = "R arm position",
        [19] = "L leg position",
        [20] = "R leg position",
    },
    [Arrow] = {
        [7]  = "Is critical",
        [8]  = "piercing level",
    },
    [BasePiglin] = {
        [15] = "Immune",
    },
    [Bat] = {
        [15] = "Is hanging",
    },
    [Bee] = {
        [16] = "flags",
        [17] = "Anger Time",
    },
    [Blaze] = {
        [15] = "On fire",
    },
    [Boat] = {
        [7]  = "Time since hit",
        [8]  = "Forward direction",
        [9]  = "Damage taken",
        [10]  = "Type",
        [11] = "Right paddle turning",
        [12] = "Left paddle turning",
        [13] = "Splash Timer",
    },
    [Cat] = {
        [18]  = "Variant",
        [19]  = "Lying",
        [20]  = "Relaxed",
        [21]  = "Collar Color",
    },
    [CaveSpider] = { },
    [ChestedHorse] = {
        [18] = "Has Chest",
    },
    [Chicken] = { },
    [Cod] = { },
    [Cow] = { },
    [Creature] = {
    },
    [Creeper] = {
        [15] = "Creeper state",
        [16] = "Charged",
        [17] = "Ignited",
    },
    [Dolphin] = {
        [15] = "Treasure position",
        [16] = "Can find treasure",
        [17] = "Has fish",
    },
    [Donkey] = { },
    [DragonFireball] = { },
    [Drowned] = { },
    [Egg] = {
        [7] = "Item",
    },
    [ElderGuardian] = { },
    [EnderCrystal] = {
        [7]  = "Beam target",
        [8]  = "Show bottom",
    },
    [EnderDragon] = {
        [15] = "Phase",
    },
    [Enderman] = {
        [15] = "Carried block",
        [16] = "Screaming",
        [17] = "Staring",
    },
    [Endermite] = { },
    [Entity] = {
        [0]  = "Flags",
        [1]  = "Air",
        [2]  = "Custom name",
        [3]  = "Name visible",
        [4]  = "Is silent",
        [5]  = "No gravity",
        [6]  = "Pose",
    },
    [EvocationFangs] = { },
    [EvocationIllager] = { },
    [ExperienceOrb] = { },
    [EyeOfEnder] = {
        [7] = "Item",
     },
    [FallingBlock] = {
        [7]  = "Position",
    },
    [Fireball] = {
        [7]  = "item",
    },
    [FireCharge] = {
        [7]  = "item",
    },
    [Fireworks] = {
        [7] = "Firework info",
        [8] = "Boosted entity ID",
        [9] = "shot at angle",
    },
    [FishingFloat] = {
        [7]  = "Hooked entity",
        [8]  = "Catchable",
    },
    [Flying] = {
    },
    [Fox] = {
        [16]  = "Type",
        [17]  = "flags",
        [18]  = "UUID1",
        [19]  = "UUID2",
    },
    [Ghast] = {
        [15] = "Attacking",
    },
    [Giant] = { },
    [Golem] = { },
    [Guardian] = {
        [15] = "Flags",
        [16] = "Target EID",
    },
    [Hanging] = {
    },
    [Hoglin] = {
        [16] = "Immune",
    },
    [Horse] = {
        [18] = "Variant",
    },
    [Husk] = { },
    [IllusionIllager] = { },
    [Insentinent] = {
        [12] = "Insentinent flags",
    },
    [IronGolem] = {
        [15] = "created by player",
    },
    [Item] = {
        [7] = "Item",
    },
    [ItemFrame] = {
        [7] = "Item",
        [8] = "Rotation",
    },
    [LeashKnot] = { },
    [LightningBolt] = { },
    [Living] = {
        [7]  = "Active hand",
        [8]  = "Health",
        [9]  = "Potion effect color",
        [10]  = "Potion effect ambient",
        [11] = "Number of arrows",
        [12] = "AbsorptionHealth",
        [13] = "Sleeping Location",
    },
    [Llama] = {
        [19] = "Strength (inventory size)",
        [20] = "Carpet color",
        [21] = "Variant",
    },
    [LlamaSpit] = { },
    [MagmaCube] = { },
    [Minecart] = { },
    [MinecartChest] = { },
    [MinecartCommandBlock] = {
        [12] = "Command",
        [14] = "Last Output",
    },
    [MinecartFurnace] = {
        [13] = "Powered",
    },
    [MinecartHopper] = { },
    [MinecartSpawner] = { },
    [MinecartTNT] = { },
    [Monster] = {
        [14] = "flags",
    },
    [Mooshroom] = {
        [16] = "Variant",
    },
    [Mule] = { },
    [Ocelot] = {
        [16] = "Ocelot type",
    },
    [Painting] = { },
    [Panda] = {
        [16]  = "Breed timer",
        [17]  = "Sneeze timer",
        [18]  = "Eat timer",
        [19]  = "Main gene",
        [20]  = "Hidden gene",
        [21]  = "flags",
    },
    [Parrot] = {
        [18] = "Variant",
    },
    [Phantom] = {
        [15] = "Size of hitbox",
    },
    [Pig] = {
        [16] = "Has saddle",
        [17] = "Carrot boost time",
    },
    [Piglin] = {
        [16] = "Baby",
        [17] = "Charging Crossbow",
        [18] = "Dancing",
    },
    [PiglinBrute] = { },
    [Pillager] = { },
    [Player] = {
        [14] = "Additional hearts",
        [15] = "Score",
        [16] = "Skin flags",
        [17] = "Main hand",
        [18] = "Left shoulder",
        [19] = "Right shoulder",
    },
    [PolarBear] = {
        [16] = "Standing",
    },
    [Potion] = {
        [7]  = "Slot",
    },
    [Pufferfish] = {
        [16] = "Puffstate",
    },
    [Rabbit] = {
        [16] = "Rabbit type",
    },
    [Raider] = {
        [15] = "Celebrating",
    },
    [Ravager] = { },
    [Salmon] = { },
    [Sheep] = {
        [16] = "Sheep color",
    },
    [Shulker] = {
        [15] = "Direction",
        [16] = "Attachment position",
        [17] = "Shield height",
        [18] = "Color",
    },
    [ShulkerBullet] = { },
    [Silverfish] = { },
    [Skeleton] = { },
    [SkeletonHorse] = { },
    [Slime] = {
        [15] = "Size",
    },
    [Snowball] = {
        [7] = "Item",
     },
    [Snowman] = {
        [15] = "Flags",
    },
    [SpectralArrow] = { },
    [SpellcasterIllager] = {
        [16] = "Spell enumerator",
    },
    [Spider] = {
        [15] = "Climbing",
    },
    [Squid] = { },
    [Stray] = { },
    [Strider] = {
        [16] = "Boost time",
        [17] = "Shaking",
        [18] = "Saddle",
    },
    [TameableAnimal] = {
        [16] = "Tameable flags",
        [17] = "Owner UUID",
    },
    [Throwable] = { },
    [ThrownEnderpearl] = {
        [7] = "Item",
     },
    [ThrownExpBottle] = {
        [7] = "Item",
    },
    [TippedArrow] = {
        [9]  = "Color",
    },
    [Trident] = {
        [9] = "Loyalty level",
        [10] = "has glint",
    },
    [TraderLlama] = { },
    [TropicalFish] = {
        [16] = "Variant",
    },
    [Turtle] = {
        [16] = "Home position",
        [17] = "Has egg",
        [18] = "Laying egg",
        [19] = "Travel pos",
        [20] = "Going home",
        [21] = "Traveling",
    },
    [Vex] = {
        [15] = "In attack mode",
    },
    [Villager] = {
        [14] = "Profession",
    },
    [VindicationIllager] = { },
    [WanderingTrader] = { },
    [WaterMob] = { },
    [Weather] = { },
    [Witch] = {
        [16] = "Drinking",
    },
    [Wither] = {
        [15] = "Target 1",
        [16] = "Target 2",
        [17] = "Target 3",
        [18] = "Invulnerable time",
    },
    [WitherSkeleton] = { },
    [WitherSkull] = {
        [7]  = "Invulnerable",
    },
    [Wolf] = {
        [18] = "Begging",
        [19] = "Collar color",
        [20] = "Anger time",
    },
    [Zoglin] = {
        [15] = "Baby",
    },
    [Zombie] = {
        [15] = "Baby zombie",
        [16] = "Unused",
        [17] = "Converting zombie",
    },
    [ZombieHorse] = { },
    [ZombiePigman] = { },
    [ZombieVillager] = {
        [18] = "Is converting",
        [19] = "Profession",
    },


};

const EntityType ENTITY_HIERARCHY[] = {
    //// Superclasses
    [Entity]                   = IllegalEntityType,
    [AbstractFireball]         = Entity,
    [AbstractFish]             = WaterMob,
    [AbstractHorse]            = Animal,
    [AbstractIllager]          = Monster,
    [AbstractMinecart]         = Entity,
    [AbstractMinecartCont]     = AbstractMinecart,
    [AbstractSkeleton]         = Monster,
    [ActivatedTNT]             = Entity,
    [Ageable]                  = Creature,
    [Ambient]                  = Insentinent,
    [Animal]                   = Ageable,
    [AreaEffectCloud]          = Entity,
    [ArmorStand]               = Living,
    [Arrow]                    = Entity,
    [Bat]                      = Ambient,
    [Blaze]                    = Monster,
    [Boat]                     = Entity,
    [CaveSpider]               = Spider,
    [ChestedHorse]             = AbstractHorse,
    [Chicken]                  = Animal,
    [Cod]                      = AbstractFish,
    [Cow]                      = Animal,
    [Creature]                 = Insentinent,
    [Creeper]                  = Monster,
    [Dolphin]                  = WaterMob,
    [Donkey]                   = ChestedHorse,
    [DragonFireball]           = AbstractFireball,
    [Drowned]                  = Zombie,
    [Egg]                      = Throwable,
    [ElderGuardian]            = Guardian,
    [EnderCrystal]             = Entity,
    [EnderDragon]              = Insentinent,
    [Enderman]                 = Monster,
    [Endermite]                = Monster,
    [EvocationFangs]           = Entity,
    [EvocationIllager]         = SpellcasterIllager,
    [ExperienceOrb]            = Entity,
    [EyeOfEnder]               = Entity,
    [FallingBlock]             = Entity,
    [Fireball]                 = AbstractFireball,
    [FireCharge]               = AbstractFireball,
    [Fireworks]                = Entity,
    [FishingFloat]             = Entity,
    [Flying]                   = Insentinent,
    [Ghast]                    = Flying,
    [Giant]                    = Monster,
    [Golem]                    = Creature,
    [Guardian]                 = Monster,
    [Hanging]                  = Entity,
    [Horse]                    = AbstractHorse,
    [Husk]                     = Zombie,
    [IllusionIllager]          = SpellcasterIllager,
    [Insentinent]              = Living,
    [IronGolem]                = Golem,
    [Item]                     = Entity,
    [ItemFrame]                = Hanging,
    [LeashKnot]                = Hanging,
    [LightningBolt]            = Weather,
    [Living]                   = Entity,
    [Llama]                    = ChestedHorse,
    [LlamaSpit]                = Entity,
    [MagmaCube]                = Slime,
    [Minecart]                 = AbstractMinecart,
    [MinecartChest]            = AbstractMinecartCont,
    [MinecartCommandBlock]     = AbstractMinecart,
    [MinecartFurnace]          = AbstractMinecart,
    [MinecartHopper]           = AbstractMinecartCont,
    [MinecartSpawner]          = AbstractMinecart,
    [MinecartTNT]              = AbstractMinecart,
    [Monster]                  = Creature,
    [Mooshroom]                = Cow,
    [Mule]                     = ChestedHorse,
    [Ocelot]                   = TameableAnimal,
    [Painting]                 = Hanging,
    [Parrot]                   = TameableAnimal,
    [Phantom]                  = Flying,
    [Pig]                      = Animal,
    [Player]                   = Living,
    [PolarBear]                = Animal,
    [Potion]                   = Throwable,
    [Pufferfish]               = AbstractFish,
    [Rabbit]                   = Animal,
    [Salmon]                   = AbstractFish,
    [Sheep]                    = Animal,
    [Shulker]                  = Golem,
    [ShulkerBullet]            = Entity,
    [Silverfish]               = Monster,
    [Skeleton]                 = AbstractSkeleton,
    [SkeletonHorse]            = AbstractHorse,
    [Slime]                    = Insentinent,
    [Snowball]                 = Throwable,
    [Snowman]                  = Golem,
    [SpectralArrow]            = Arrow,
    [SpellcasterIllager]       = AbstractIllager,
    [Spider]                   = Monster,
    [Squid]                    = WaterMob,
    [Stray]                    = AbstractSkeleton,
    [TameableAnimal]           = Animal,
    [Throwable]                = Entity,
    [ThrownEnderpearl]         = Throwable,
    [ThrownExpBottle]          = Throwable,
    [TippedArrow]              = Arrow,
    [Trident]                  = Arrow,
    [TropicalFish]             = AbstractFish,
    [Turtle]                   = Animal,
    [Vex]                      = Monster,
    [Villager]                 = AbstractVillager,
    [VindicationIllager]       = AbstractIllager,
    [WaterMob]                 = Creature,
    [Weather]                  = Entity,
    [Witch]                    = Monster,
    [Wither]                   = Monster,
    [WitherSkeleton]           = AbstractSkeleton,
    [WitherSkull]              = AbstractFireball,
    [Wolf]                     = TameableAnimal,
    [Zombie]                   = Monster,
    [ZombieHorse]              = AbstractHorse,
    [ZombiePigman]             = Zombie,
    [ZombieVillager]           = Zombie,
    [Bee]                      = Animal,
    [Cat]                      = TameableAnimal,
    [Fox]                      = Animal,
    [Hoglin]                   = Animal,
    [Panda]                    = Animal,
    [Piglin]                   = BasePiglin,
    [PiglinBrute]              = BasePiglin,
    [Pillager]                 = AbstractIllager,
    [Ravager]                  = Raider,
    [Strider]                  = Animal,
    [TraderLlama]              = IllegalEntityType,
    [Zoglin]                   = Monster,
    [Raider]                   = Monster,
    [BasePiglin]               = Monster,
    [AbstractVillager]         = Ageable,
    [WanderingTrader]          = AbstractVillager,

};

#define ENUMNAME(name) [name] = #name

const char * ENTITY_NAMES[MaxEntityType] = {
    ENUMNAME(Entity),
    ENUMNAME(AbstractFireball),
    ENUMNAME(AbstractFish),
    ENUMNAME(AbstractHorse),
    ENUMNAME(AbstractIllager),
    ENUMNAME(AbstractMinecart),
    ENUMNAME(AbstractMinecartCont),
    ENUMNAME(AbstractSkeleton),
    ENUMNAME(ActivatedTNT),
    ENUMNAME(Ageable),
    ENUMNAME(Ambient),
    ENUMNAME(Animal),
    ENUMNAME(AreaEffectCloud),
    ENUMNAME(ArmorStand),
    ENUMNAME(Arrow),
    ENUMNAME(Bat),
    ENUMNAME(Blaze),
    ENUMNAME(Boat),
    ENUMNAME(CaveSpider),
    ENUMNAME(ChestedHorse),
    ENUMNAME(Chicken),
    ENUMNAME(Cod),
    ENUMNAME(Cow),
    ENUMNAME(Creature),
    ENUMNAME(Creeper),
    ENUMNAME(Dolphin),
    ENUMNAME(Donkey),
    ENUMNAME(DragonFireball),
    ENUMNAME(Drowned),
    ENUMNAME(Egg),
    ENUMNAME(ElderGuardian),
    ENUMNAME(EnderCrystal),
    ENUMNAME(EnderDragon),
    ENUMNAME(Enderman),
    ENUMNAME(Endermite),
    ENUMNAME(EvocationFangs),
    ENUMNAME(EvocationIllager),
    ENUMNAME(ExperienceOrb),
    ENUMNAME(EyeOfEnder),
    ENUMNAME(FallingBlock),
    ENUMNAME(Fireball),
    ENUMNAME(FireCharge),
    ENUMNAME(Fireworks),
    ENUMNAME(FishingFloat),
    ENUMNAME(Flying),
    ENUMNAME(Ghast),
    ENUMNAME(Giant),
    ENUMNAME(Golem),
    ENUMNAME(Guardian),
    ENUMNAME(Hanging),
    ENUMNAME(Horse),
    ENUMNAME(Husk),
    ENUMNAME(IllusionIllager),
    ENUMNAME(Insentinent),
    ENUMNAME(IronGolem),
    ENUMNAME(Item),
    ENUMNAME(ItemFrame),
    ENUMNAME(LeashKnot),
    ENUMNAME(LightningBolt),
    ENUMNAME(Living),
    ENUMNAME(Llama),
    ENUMNAME(LlamaSpit),
    ENUMNAME(MagmaCube),
    ENUMNAME(Minecart),
    ENUMNAME(MinecartChest),
    ENUMNAME(MinecartCommandBlock),
    ENUMNAME(MinecartFurnace),
    ENUMNAME(MinecartHopper),
    ENUMNAME(MinecartSpawner),
    ENUMNAME(MinecartTNT),
    ENUMNAME(Monster),
    ENUMNAME(Mooshroom),
    ENUMNAME(Mule),
    ENUMNAME(Ocelot),
    ENUMNAME(Painting),
    ENUMNAME(Parrot),
    ENUMNAME(Phantom),
    ENUMNAME(Pig),
    ENUMNAME(Player),
    ENUMNAME(PolarBear),
    ENUMNAME(Potion),
    ENUMNAME(Pufferfish),
    ENUMNAME(Rabbit),
    ENUMNAME(Salmon),
    ENUMNAME(Sheep),
    ENUMNAME(Shulker),
    ENUMNAME(ShulkerBullet),
    ENUMNAME(Silverfish),
    ENUMNAME(Skeleton),
    ENUMNAME(SkeletonHorse),
    ENUMNAME(Slime),
    ENUMNAME(Snowball),
    ENUMNAME(Snowman),
    ENUMNAME(SpectralArrow),
    ENUMNAME(SpellcasterIllager),
    ENUMNAME(Spider),
    ENUMNAME(Squid),
    ENUMNAME(Stray),
    ENUMNAME(TameableAnimal),
    ENUMNAME(Throwable),
    ENUMNAME(ThrownEnderpearl),
    ENUMNAME(ThrownExpBottle),
    ENUMNAME(TippedArrow),
    ENUMNAME(Trident),
    ENUMNAME(TropicalFish),
    ENUMNAME(Turtle),
    ENUMNAME(Vex),
    ENUMNAME(Villager),
    ENUMNAME(VindicationIllager),
    ENUMNAME(WaterMob),
    ENUMNAME(Weather),
    ENUMNAME(Witch),
    ENUMNAME(Wither),
    ENUMNAME(WitherSkeleton),
    ENUMNAME(WitherSkull),
    ENUMNAME(Wolf),
    ENUMNAME(Zombie),
    ENUMNAME(ZombieHorse),
    ENUMNAME(ZombiePigman),
    ENUMNAME(ZombieVillager),
    ENUMNAME(Bee),
    ENUMNAME(Cat),
    ENUMNAME(Fox),
    ENUMNAME(Hoglin),
    ENUMNAME(Panda),
    ENUMNAME(Piglin),
    ENUMNAME(PiglinBrute),
    ENUMNAME(Pillager),
    ENUMNAME(Ravager),
    ENUMNAME(Strider),
    ENUMNAME(TraderLlama),
    ENUMNAME(Zoglin),
    ENUMNAME(Raider),
    ENUMNAME(BasePiglin),
    ENUMNAME(AbstractVillager),
    ENUMNAME(WanderingTrader),

};

const char * get_entity_name(char *buf, EntityType type) {
    if (type < 0 || type >= MaxEntityType ) {
        sprintf(buf, "%s", "IllegalEntityType");
    }
    else if ( ENTITY_NAMES[type] ) {
        sprintf(buf, "%s", ENTITY_NAMES[type]);
    }
    else {
        sprintf(buf, "%s", "UnknownEntity");
    }
    return buf;
}

////////////////////////////////////////////////////////////////////////////////
// Entity Metadata

const char * METATYPES[] = {
    [META_BYTE]     = "byte",
    [META_VARINT]   = "varint",
    [META_FLOAT]    = "float",
    [META_STRING]   = "string",
    [META_CHAT]     = "chat",
    [META_OPTCHAT]  = "optchat",
    [META_SLOT]     = "slot",
    [META_BOOL]     = "bool",
    [META_VEC3]     = "vector3f",
    [META_POS]      = "position",
    [META_OPTPOS]   = "optional_position",
    [META_DIR]      = "direction",
    [META_OPTUUID]  = "optional_uuid",
    [META_BID]      = "block_id",
    [META_NBT]      = "nbt",
    [META_PARTICLE] = "particle",
    [META_VILLAGER] = "villager",
    [META_OPTVARINT]= "optvarint",
    [META_POSE]     = "pose",
    [META_NONE]     = "-"
};

metadata * clone_metadata(metadata *meta) {
    if (!meta) return NULL;
    lh_create_num(metadata, newmeta, 32);
    memmove(newmeta, meta, 32*sizeof(metadata));
    int i;
    for(i=0; i<32; i++) {
        switch (newmeta[i].type) {
            case META_SLOT:
                clone_slot(&meta[i].slot, &newmeta[i].slot);
                break;
            case META_NBT:
                newmeta[i].nbt = nbt_clone(meta[i].nbt);
                break;
            case META_STRING:
            case META_CHAT:
            case META_OPTCHAT:
                newmeta[i].str = meta[i].str ? strdup(meta[i].str) : NULL;
                break;
        }
    }
    return newmeta;
}

metadata * update_metadata(metadata *meta, metadata *upd) {
    if (!meta) return NULL;
    if (!upd)  return meta;

    int i;
    for(i=0; i<32; i++) {
        if (upd[i].type != 0xff) {
            if (meta[i].type != upd[i].type) {
                printf("update_metadata : incompatible metadata types at index %d : old=%d vs new=%d\n",
                       i, meta[i].type, upd[i].type);
                continue;
            }

            // replace stored metadata with the one from the packet
            switch (meta[i].type) {
                case META_SLOT:
                    clear_slot(&meta[i].slot);
                    clone_slot(&upd[i].slot, &meta[i].slot);
                    break;
                case META_NBT:
                    nbt_free(meta[i].nbt);
                    meta[i].nbt = nbt_clone(upd[i].nbt);
                    break;
                case META_STRING:
                case META_CHAT:
                case META_OPTCHAT:
                    lh_free(meta[i].str);
                    meta[i].str = upd[i].str ? strdup(upd[i].str) : NULL;
                    break;
                default:
                    meta[i] = upd[i];
            }
        }
    }
    return meta;
}


void free_metadata(metadata *meta) {
    if (!meta) return;
    int i;
    for(i=0; i<32; i++) {
        switch (meta[i].type) {
            case META_SLOT:
                clear_slot(&meta[i].slot);
                break;
            case META_NBT:
                nbt_free(meta[i].nbt);
                break;
            case META_STRING:
            case META_CHAT:
            case META_OPTCHAT:
                lh_free(meta[i].str);
                break;
        }
    }
    free(meta);
}

uint8_t * read_metadata(uint8_t *p, metadata **meta) {
    assert(meta);
    assert(!*meta);
    ssize_t mc = 0;

    // allocate a whole set of 32 values
    lh_alloc_num(*meta, 32);
    metadata *m = *meta;

    int i;
    int bool;
    uint32_t di;
    float df;
    slot_t ds;

    // debug
    // hexdump(p,64);

    // mark all entries as not present - we use the same 0xff value
    // that Mojang uses as terminator
    for(i=0; i<32; i++) m[i].type = META_NONE;

    char sbuf[MCP_MAXSTR];

    while (1) {
        uint8_t key = read_char(p);
        if (key == 0xff) break; // terminator
        assert(key < 32); //VERIFY: if this legacy limit still valid

        metadata *mm = &m[key];
        mm->key = key;
        mm->type = read_varint(p);

        switch (mm->type) {
            case META_BYTE:     mm->b = read_char(p);    break;
            case META_VARINT:   mm->i = read_varint(p);  break;
            case META_FLOAT:    mm->f = read_float(p);   break;
            case META_STRING:
            case META_CHAT:     lh_free(mm->str);
                                p = read_string(p,sbuf);
                                mm->str = strdup(sbuf);
                                break;
            case META_OPTCHAT:  lh_free(mm->str);
                                bool = read_char(p);
                                if (bool) {
                                    p = read_string(p,sbuf);
                                    mm->str = strdup(sbuf);
                                }
                                break;
            case META_SLOT:     clear_slot(&mm->slot);
                                p = read_slot(p,&mm->slot);
                                break;
            case META_BOOL:     mm->bool = read_char(p);  break;
            case META_VEC3:     mm->fx=read_float(p);
                                mm->fy=read_float(p);
                                mm->fz=read_float(p); break;
            case META_POS:      mm->pos.p = read_long(p); break;
            case META_OPTPOS:   bool = read_char(p);
                                if (bool) {
                                    mm->pos.p = read_long(p);
                                }
                                else {
                                    mm->pos.p = (uint64_t)-1;
                                }
                                break;
            case META_DIR:      mm->dir = read_varint(p);  break;
            case META_OPTUUID:  bool = read_char(p);
                                if (bool) {
                                    memmove(mm->uuid,p,sizeof(uuid_t));
                                    p+=sizeof(uuid_t);
                                }
                                break;
            case META_BID:      mm->block = read_varint(p); break;
            case META_NBT:      nbt_free(mm->nbt);
                                mm->nbt = nbt_parse(&p);
                                break;
            case META_PARTICLE: // discard particle meta and any following data
                                mm->type = META_NONE;
                                di = read_varint(p);
                                switch (di) {
                                    case 3:
                                    case 20:
                                        di = read_varint(p);
                                        break;
                                    case 11:
                                        df = read_float(p);
                                        df = read_float(p);
                                        df = read_float(p);
                                        df = read_float(p);
                                        break;
                                    case 27:
                                        p = read_slot(p,&ds);
                                        clear_slot(&ds);
                                        break;
                                }
                                break;
            case META_VILLAGER:     mm->vil1 = read_varint(p);
                                    mm->vil2 = read_varint(p);
                                    mm->vil3 = read_varint(p); break;
            case META_OPTVARINT:    mm->i = read_varint(p); break;
            case META_POSE:         mm->i = read_varint(p); break;
        }
    }

    return p;
}

uint8_t * write_metadata(uint8_t *w, metadata *meta) {
    assert(meta);

    int i,j;
    char bool;
    for (i=0; i<32; i++) {
        metadata *mm = meta+i;
        if (mm->type==META_NONE) continue;

        lh_write_char(w, mm->key);
        lh_write_varint(w, mm->type);
        switch (mm->type) {
            case META_BYTE:     write_char(w, mm->b);    break;
            case META_VARINT:   write_varint(w, mm->i);  break;
            case META_FLOAT:    write_float(w, mm->f);   break;
            case META_STRING:
            case META_CHAT:     w = write_string(w, mm->str); break;
            case META_OPTCHAT:  bool = (mm->str != NULL);
                                write_char(w, bool);
                                if (bool) w = write_string(w, mm->str);
                                break;
            case META_SLOT:     w = write_slot(w, &mm->slot); break;
            case META_BOOL:     write_char(w, mm->bool);  break;
            case META_VEC3:     write_float(w, mm->fx);
                                write_float(w, mm->fy);
                                write_float(w, mm->fz); break;
            case META_POS:      write_long(w, mm->pos.p); break;
            case META_OPTPOS:   bool = (mm->pos.p == -1);
                                write_char(w, bool);
                                if (bool) {
                                    write_long(w, mm->pos.p);
                                }
                                break;
            case META_DIR:      write_varint(w, mm->dir);  break;
            case META_OPTUUID:  bool = 0;
                                for(j=0; j<16; j++)
                                    if (mm->uuid[j])
                                        bool = 1;
                                write_char(w, bool);
                                if (bool) {
                                    memmove(w, mm->uuid, sizeof(mm->uuid));
                                    w+=16;
                                }
                                break;
            case META_BID:      write_char(w, mm->block); break;
            case META_NBT:      nbt_write(&w, mm->nbt); break;
        }
    }
    lh_write_char(w, 0xff);

    return w;
}

void dump_metadata(metadata *meta, EntityType et) {
    if (!meta) return;

    int i;
    for (i=0; i<32; i++) {
        metadata *mm = meta+i;
        if (mm->type==META_NONE) continue;

        printf("\n    ");

        const char * name = NULL;
        EntityType ett = et;
        while ((!name) && (ett!=IllegalEntityType)) {
            name = METANAME[ett][mm->key];
            ett = ENTITY_HIERARCHY[ett];
        }

        printf("%2d %-24s [%-6s] = ", mm->key, name?name:"Unknown",METATYPES[mm->type]);
        switch (mm->type) {
            case META_BYTE:     printf("%d",  mm->b);   break;
            case META_VARINT:   printf("%d",  mm->i);   break;
            case META_FLOAT:    printf("%.1f",mm->f);   break;
            case META_STRING:
            case META_CHAT:
            case META_OPTCHAT:  printf("\"%s\"", mm->str ? mm->str : ""); break;
            case META_SLOT:     dump_slot(&mm->slot); break;
            case META_BOOL:     printf("%s", mm->bool?"true":"false"); break; //VERIFY
            case META_VEC3:     printf("%.1f,%.1f,%.1f", mm->fx, mm->fy, mm->fz); break;
            case META_POS:
            case META_OPTPOS:   printf("%d,%d,%d", mm->pos.x, mm->pos.y, mm->pos.z); break;
            case META_DIR:      printf("%d",mm->dir);  break;
            case META_OPTUUID:  hexprint(mm->uuid, sizeof(uuid_t)); break;
            case META_BID:      printf("%2x (%d)", mm->block, mm->block); break; //TODO: print material name
            case META_NBT:      printf("NBT data %p", mm->nbt); break;
            case META_VILLAGER: printf("Villager: %i %i %i",mm->vil1,mm->vil2,mm->vil3); break;
            case META_OPTVARINT: printf("%d",  mm->i);   break;
            case META_POSE:     printf("%d",  mm->i);   break;
            default:            printf("<unknown type>"); break;
        }
    }
}
