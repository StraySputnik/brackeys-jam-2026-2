#include "entity.h"

#include <stdlib.h>

void draw_entity(const Entity* entity, const float entity_size) {
    if (entity->type == NULL_ENTITY) {
        return;
    }

    Color color = MAGENTA;

    switch (entity->type) {
    case ENEMY:
        color = RED;
        break;
    case FRIEND:
        color = GREEN;
        break;
    default:
        break;
    }

    DrawCircleV(entity->position, entity_size, color);
}

EntityPool make_entity_pool() {
    EntityPool pool;
    pool.entities = NULL;
    pool.count = 0;
    return pool;
}

void delete_entity_pool(EntityPool* pool) {
    free(pool->entities);
}

EntityId spawn_entity(EntityPool* pool, const Vector2 position, const EntityType type) {
    // Look for null entity first
    for (size_t i = 0; i < pool->count; i++) {
        if (pool->entities[i].type != NULL_ENTITY) {
            continue;
        }

        pool->entities[i].position = position;
        pool->entities[i].type = type;
        return i;
    }

    if (pool->count == 0) {
        pool->entities = (Entity*)malloc(sizeof(Entity));
    } else {
        pool->entities = (Entity*)realloc(pool->entities, (pool->count + 1) * sizeof(Entity));
    }

    pool->count++;
    pool->entities[pool->count - 1].position = position;
    pool->entities[pool->count - 1].type = type;

    return pool->count - 1;
}

void despawn_entity(const EntityPool* pool, const EntityId id) {
    pool->entities[id].type = NULL_ENTITY;
}

Entity* get_entity(const EntityPool* pool, const EntityId id) {
    if (id >= pool->count) {
        return NULL;
    }

    return &pool->entities[id];
}

void draw_entities(const EntityPool* pool, const float entity_size) {
    for (size_t i = 0; i < pool->count; i++) {
        draw_entity(&pool->entities[i], entity_size);
    }
}
