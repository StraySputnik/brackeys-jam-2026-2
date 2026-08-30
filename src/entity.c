#include "entity.h"

#include <assert.h>
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

const char *strength_to_string(const Strength food) {
    switch (food) {
    case STRENGTH_NONE:
        return "None";
    case STRENGTH_BLOOD:
        return "Blood";
    case STRENGTH_FULL_MOON:
        return "Full Moon";
    case STRENGTH_FEAR:
        return "Fear";
    default:
        return "Unknown";
    }
}

const char *random_vampire_name() {
    const char *names[] = {
        "Dracula",
        "Vlad",
        "Draven",
        "Crimson",
        "Lazarus",
        "Igor",
    };

    return names[GetRandomValue(0, sizeof(names) / sizeof(names[0]) - 1)];
}

const char *random_werewolf_name() {
    const char *names[] = {
        "Fenrir",
        "Remus",
        "Cynric",
        "Wren",
        "Lyra",
        "Fleabag",
        "Wolferine",
    };

    return names[GetRandomValue(0, sizeof(names) / sizeof(names[0]) - 1)];
}

const char *random_ghost_name() {
    const char *names[] = {
        "Liam",
        "Celeste",
        "Mary",
        "Blair",
        "Specter",
        "Agnes",
    };

    return names[GetRandomValue(0, sizeof(names) / sizeof(names[0]) - 1)];
}

bool is_friendly(const MonsterData monster) {
    switch (monster.monster_type) {
    case MONSTER_VAMPIRE:
        return monster.age <= 800 && monster.strength == STRENGTH_BLOOD &&
            monster.weakness & (WEAKNESS_GARLIC | WEAKNESS_SUN) && monster.title != TITLE_DUKE &&
            monster.title != TITLE_SIR;
    case MONSTER_WEREWOLF:
        return monster.age <= 60 && monster.strength == STRENGTH_FULL_MOON &&
            monster.weakness == WEAKNESS_SILVER && monster.title == TITLE_NONE;
    case MONSTER_GHOST:
        return monster.age <= 2000 && monster.strength == STRENGTH_FEAR &&
            monster.weakness & (WEAKNESS_HOLY_WATER | WEAKNESS_SALT);
    default:
        return false;
    }
}

int get_health_for_monster(const MonsterType type) {
    switch (type) {
    case MONSTER_VAMPIRE:
        return 2;
    case MONSTER_WEREWOLF:
        return 4;
    case MONSTER_GHOST:
        return 6;
    default:
        return 0;
    }
}

void randomize_monster_data(MonsterData *monster, const MonsterType type, const bool friendly) {
    monster->monster_type = type;

    if (friendly) {
        switch (type) {
        case MONSTER_WEREWOLF:
            monster->name = random_werewolf_name();
            monster->title    = TITLE_NONE;
            monster->age      = GetRandomValue(18, 60);
            monster->weakness = WEAKNESS_SILVER;
            monster->strength = STRENGTH_FULL_MOON;
            break;
        case MONSTER_VAMPIRE:
            monster->name = random_vampire_name();
            monster->title    = TITLE_NONE + GetRandomValue(0, 1);
            monster->age      = GetRandomValue(18, 800);
            monster->weakness = WEAKNESS_GARLIC + GetRandomValue(0, 1);
            monster->strength = STRENGTH_BLOOD;
            break;
        case MONSTER_GHOST:
            monster->name = random_ghost_name();
            monster->title    = TITLE_NONE;
            monster->age      = GetRandomValue(18, 2000);
            monster->weakness = WEAKNESS_HOLY_WATER + GetRandomValue(0, 1);
            monster->strength = STRENGTH_NONE;
            break;
        default:
            break;
        }
    } else {
        do {
            monster->title    = GetRandomValue(1, TITLE_TYPE_COUNT - 1);
            monster->weakness = GetRandomValue(1, WEAKNESS_TYPE_COUNT - 1);
            monster->strength = GetRandomValue(1, STRENGTH_TYPE_COUNT - 1);

            const int age_group = GetRandomValue(0, 2);
            switch (age_group) {
            case 0:
                monster->age = GetRandomValue(18, 60);
                break;
            case 1:
                monster->age = GetRandomValue(18, 800);
                break;
            default:
                monster->age = GetRandomValue(18, 2000);
                break;
            }

            const int name_group = GetRandomValue(0, 2);
            switch (name_group) {
            case 0:
                monster->name = random_vampire_name();
                break;
            case 1:
                monster->name = random_werewolf_name();
                break;
            default:
                monster->name = random_ghost_name();
                break;
            }
        } while (is_friendly(*monster));
    }
}

void draw_entity(const SpriteStore *sprite_store, const Entity *entity, const float entity_size) {
    if (entity->type == ENTITY_NULL) {
        return;
    }

    Vector2 centered_pos = {
        .x = entity->position.x - 0.5f * entity_size, .y = entity->position.y - 0.5f * entity_size
    };

    const float scale = entity_size / 16.0f;
    if (entity->type == ENTITY_PROJECTILE || entity->type == ENTITY_CANNONBALL) {
        centered_pos.x = centered_pos.x + 0.5f * entity_size - 0.125f * entity_size;
        centered_pos.y = centered_pos.y + 0.5f * entity_size - 0.125f * entity_size;
    }

    Texture sprite;

    switch (entity->type) {
    case ENTITY_MONSTER:
        switch (entity->data.m.monster_type) {
        case MONSTER_VAMPIRE:
            sprite = sprite_store->vampire;
            break;
        case MONSTER_WEREWOLF:
            sprite = sprite_store->werewolf;
            break;
        case MONSTER_GHOST:
            sprite = sprite_store->ghost;
            break;
        default:
            assert(false);
            break;
        }

        break;
    case ENTITY_PROJECTILE:
        sprite = sprite_store->projectile;
        break;
    case ENTITY_CANNONBALL:
        sprite = sprite_store->cannonball;
        break;
    default:
        assert(false);
        break;
    }

    DrawTextureEx(sprite, (Vector2){.x = centered_pos.x, .y = centered_pos.y}, 0.0f, scale, WHITE);
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

void draw_entities(const SpriteStore *sprite_store, const EntityPool *pool, const float entity_size) {
    for (size_t i = 0; i < pool->count; i++) {
        draw_entity(sprite_store, &pool->entities[i], entity_size);
    }
}
