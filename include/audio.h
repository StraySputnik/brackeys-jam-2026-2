#ifndef GAME_AUDIO_H
#define GAME_AUDIO_H

#include <raylib.h>

#include <stdlib.h>

typedef struct {
    const char *name;
    Sound       sound;
    float       volume;
} AudioClip;

typedef struct {
    AudioClip *ptr;
    size_t     count;
} AudioStore;

AudioStore make_audio_store();
void       delete_audio_store(AudioStore *store);

void       audio_store_load(AudioStore *store, const char *filename, const char *name, float volume);
AudioClip *get_audio_clip(AudioStore *store, const char *name);

void play_sfx(AudioStore *store, const char *name);

#endif //GAME_AUDIO_H
