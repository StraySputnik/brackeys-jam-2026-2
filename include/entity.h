#ifndef GAME_ENTITY_H
#define GAME_ENTITY_H

#include <raylib.h>

#include <stddef.h>

typedef size_t EntityId;

typedef enum {
    ENTITY_NULL,
    ENTITY_MONSTER,
    ENTITY_PROJECTILE,
    ENTITY_CANNONBALL,
    ENTITY_TYPE_COUNT,
} EntityType;

typedef enum {
    MONSTER_NONE,
    MONSTER_VAMPIRE,
    MONSTER_WEREWOLF,
    MONSTER_GHOST,
    MONSTER_TYPE_COUNT,
} MonsterType;

const char *monster_type_to_string(MonsterType type);

typedef enum {
    WEAKNESS_NONE,
    WEAKNESS_GARLIC,
    WEAKNESS_SUN,
    WEAKNESS_HOLY_WATER,
    WEAKNESS_SALT,
    WEAKNESS_SILVER,
    WEAKNESS_TYPE_COUNT,
} Weakness;

const char *weakness_to_string(Weakness weakness);

typedef enum {
    TITLE_NONE,
    TITLE_COUNT,
    TITLE_SIR,
    TITLE_DUKE,
    TITLE_TYPE_COUNT,
} Title;

const char *title_to_string(Title title);

typedef enum {
    STRENGTH_NONE,
    STRENGTH_BLOOD,
    STRENGTH_FULL_MOON,
    STRENGTH_FEAR,
    STRENGTH_TYPE_COUNT,
} Strength;

const char *strength_to_string(Strength food);

typedef struct {
    const char *name;
    Vector2     direction;
    MonsterType monster_type;
    Title       title;
    Weakness    weakness;
    Strength    strength;
    int         age;
    int         health;
    bool        marked;
    int         target_count;
} MonsterData;

const char *random_vampire_name();
const char *random_werewolf_name();
const char *random_ghost_name();

bool is_friendly(MonsterData monster);
int  get_health_for_monster(MonsterType type);

void randomize_monster_data(MonsterData *monster, MonsterType type, bool friendly);

typedef struct {
    Vector2 direction;
} ProjectileData;

typedef struct {
    Vector2 direction;
} CannonballData;

typedef struct {
    EntityId   id;
    Vector2    position;
    EntityType type;

    union {
        MonsterData    m;
        ProjectileData p;
        CannonballData c;
    } data;
} Entity;

void draw_entity(const Entity *entity, float entity_size);

typedef struct {
    Entity *entities;
    size_t  count;
} EntityPool;

EntityPool make_entity_pool();
void       delete_entity_pool(EntityPool *pool);

EntityId spawn_entity(EntityPool *pool, Vector2 position, EntityType type);
void     despawn_entity(const EntityPool *pool, EntityId id);

Entity *get_entity(const EntityPool *pool, EntityId id);
void    draw_entities(const EntityPool *pool, float entity_size);

#endif //GAME_ENTITY_H
