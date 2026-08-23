#ifndef GAME_ENTITY_H
#define GAME_ENTITY_H

#include <raylib.h>

#include <stddef.h>

typedef size_t EntityId;

typedef enum {
    NULL_ENTITY,
    ENEMY,
    FRIEND,
    ENTITY_TYPE_COUNT,
} EntityType;

typedef struct {
    Vector2 position;
    EntityType type;
} Entity;

void draw_entity(const Entity* entity, float entity_size);

typedef struct {
    Entity* entities;
    size_t count;
} EntityPool;

EntityPool make_entity_pool(size_t initial_count);
void delete_entity_pool(EntityPool* pool);

EntityId spawn_entity(EntityPool* pool, Vector2 position, EntityType type);
void despawn_entity(const EntityPool* pool, EntityId id);

Entity* get_entity(const EntityPool* pool, EntityId id);
void draw_entities(const EntityPool* pool, float entity_size);

#endif //GAME_ENTITY_H
