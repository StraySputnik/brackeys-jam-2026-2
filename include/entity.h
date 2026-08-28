#ifndef GAME_ENTITY_H
#define GAME_ENTITY_H

#include <raylib.h>

#include <stddef.h>

typedef size_t EntityId;

typedef enum {
    ENTITY_NULL,
    ENTITY_MONSTER,
    ENTITY_PROJECTILE,
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
    FOOD_NONE,
    FOOD_BLOOD,
    FOOD_MEAT,
    FOOD_TYPE_COUNT,
} Food;

const char *food_to_string(Food food);

typedef struct {
    const char *name;
    Vector2     direction;
    MonsterType monster_type;
    Title       title;
    Weakness    weakness;
    Food        favored_food;
    int         age;
    bool        marked;
} MonsterData;

bool is_friendly(MonsterData monster);

typedef struct {
    Vector2 direction;
} ProjectileData;

typedef struct {
    EntityId   id;
    Vector2    position;
    EntityType type;

    union {
        MonsterData    m;
        ProjectileData p;
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
