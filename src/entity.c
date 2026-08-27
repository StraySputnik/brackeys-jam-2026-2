#include "entity.h"

#include <stdlib.h>

const char *monster_type_to_string(const MonsterType type) {
    switch (type) {
    case MONSTER_VAMPIRE:
        return "Vampire";
    case MONSTER_WEREWOLF:
        return "Werewolf";
    case MONSTER_GHOST:
        return "Ghost";
    default:
        return "Unknown";
    }
}

const char *weakness_to_string(const Weakness weakness) {
    switch (weakness) {
    case WEAKNESS_NONE:
        return "None";
    case WEAKNESS_GARLIC:
        return "Garlic";
    case WEAKNESS_SILVER:
        return "Silver";
    case WEAKNESS_HOLY_WATER:
        return "Holy Water";
    case WEAKNESS_SALT:
        return "Salt";
    case WEAKNESS_SUN:
        return "Sun";
    default:
        return "Unknown";
    }
}

const char *title_to_string(const Title title) {
    switch (title) {
    case TITLE_NONE:
        return "";
    case TITLE_COUNT:
        return "Count ";
    case TITLE_SIR:
        return "Sir ";
    case TITLE_DUKE:
        return "Duke ";
    default:
        return "Unknown ";
    }
}

const char *food_to_string(const Food food) {
    switch (food) {
    case FOOD_NONE:
        return "None";
    case FOOD_BLOOD:
        return "Blood";
    case FOOD_MEAT:
        return "Meat";
    default:
        return "Unknown";
    }
}

bool is_friendly(const MonsterData monster) {
    switch (monster.monster_type) {
    case MONSTER_VAMPIRE:
        return monster.age <= 800 && monster.favored_food == FOOD_BLOOD &&
            monster.weakness & (WEAKNESS_GARLIC | WEAKNESS_SUN) && monster.title != TITLE_DUKE &&
            monster.title != TITLE_SIR;
    case MONSTER_WEREWOLF:
        return monster.age <= 60 && monster.favored_food == FOOD_MEAT &&
            monster.weakness == WEAKNESS_SILVER && monster.title == TITLE_NONE;
    case MONSTER_GHOST:
        return monster.age <= 2000 && monster.favored_food == FOOD_NONE &&
            monster.weakness & (WEAKNESS_HOLY_WATER | WEAKNESS_SALT);
    default:
        return false;
    }
}

void draw_entity(const Entity *entity, float entity_size) {
    if (entity->type == ENTITY_NULL) {
        return;
    }

    Color color = MAGENTA;

    switch (entity->type) {
    case ENTITY_MONSTER:
        color = RED;
        break;
    case ENTITY_PROJECTILE:
        color = YELLOW;
        entity_size *= 0.5f;
        break;
    default:
        break;
    }

    DrawCircleV(entity->position, entity_size, color);
}

EntityPool make_entity_pool() {
    EntityPool pool;
    pool.entities = NULL;
    pool.count    = 0;
    return pool;
}

void delete_entity_pool(EntityPool *pool) {
    free(pool->entities);
}

EntityId spawn_entity(EntityPool *pool, const Vector2 position, const EntityType type) {
    // Look for null entity first
    for (size_t i = 0; i < pool->count; i++) {
        if (pool->entities[i].type != ENTITY_NULL) {
            continue;
        }

        pool->entities[i].position = position;
        pool->entities[i].type     = type;
        pool->entities[i].id       = i;
        return i;
    }

    if (pool->count == 0) {
        pool->entities = (Entity *)malloc(sizeof(Entity));
    } else {
        pool->entities = (Entity *)realloc(pool->entities, (pool->count + 1) * sizeof(Entity));
    }

    pool->count++;
    pool->entities[pool->count - 1].position = position;
    pool->entities[pool->count - 1].type     = type;
    pool->entities[pool->count - 1].id       = pool->count - 1;

    return pool->count - 1;
}

void despawn_entity(const EntityPool *pool, const EntityId id) {
    pool->entities[id].type = ENTITY_NULL;
}

Entity *get_entity(const EntityPool *pool, const EntityId id) {
    if (id >= pool->count) {
        return NULL;
    }

    return &pool->entities[id];
}

void draw_entities(const EntityPool *pool, const float entity_size) {
    for (size_t i = 0; i < pool->count; i++) {
        draw_entity(&pool->entities[i], entity_size);
    }
}
