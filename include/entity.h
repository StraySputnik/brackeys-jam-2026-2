#ifndef GAME_ENTITY_H
#define GAME_ENTITY_H

#include <raylib.h>

#include <stddef.h>

typedef size_t EntityId;

typedef enum {
    NULL_ENTITY,
    ENEMY,
    FRIEND,
    PROJECTILE,
    ENTITY_TYPE_COUNT,
} EntityType;

typedef struct {
    Vector2 direction;
    bool marked;
} EnemyData;

typedef struct {
    Vector2 direction;
    bool marked;
} FriendData;

typedef struct {
    Vector2 direction;
} ProjectileData;

typedef struct {
    EntityId id;
    Vector2 position;
    EntityType type;

    union {
        EnemyData enemy;
        FriendData friend;
        ProjectileData projectile;
    } data;
} Entity;

void draw_entity(const Entity* entity, float entity_size);

typedef struct {
    Entity* entities;
    size_t count;
} EntityPool;

EntityPool make_entity_pool();
void delete_entity_pool(EntityPool* pool);

EntityId spawn_entity(EntityPool* pool, Vector2 position, EntityType type);
void despawn_entity(const EntityPool* pool, EntityId id);

Entity* get_entity(const EntityPool* pool, EntityId id);
void draw_entities(const EntityPool* pool, float entity_size);

#endif //GAME_ENTITY_H
