#include "audio.h"

#include <raylib.h>
#include <stdio.h>

#include <stdlib.h>
#include <string.h>

AudioStore make_audio_store() {
    AudioStore store;
    store.ptr   = NULL;
    store.count = 0;
    return store;
}

void delete_audio_store(AudioStore *store) {
    for (int i = 0; i < store->count; i++) {
        UnloadSound(store->ptr[i].sound);
    }

    free(store->ptr);
}

void audio_store_load(AudioStore *store, const char *filename, const char *name, const float volume) {
    AudioClip clip;
    clip.name   = name;
    clip.sound  = LoadSound(filename);
    clip.volume = volume;

    if (store->ptr) {
        store->ptr = realloc(store->ptr, (store->count + 1) * sizeof(AudioClip));
    } else {
        store->ptr = malloc(sizeof(AudioClip));
    }

    store->count++;
    store->ptr[store->count - 1] = clip;
}

AudioClip *get_audio_clip(AudioStore *store, const char *name) {
    for (int i = 0; i < store->count; i++) {
        AudioClip *clip = &store->ptr[i];
        if (strcmp(clip->name, name) != 0) {
            continue;
        }

        return clip;
    }

    return NULL;
}

void play_sfx(AudioStore *store, const char *name) {
    AudioClip *clip = get_audio_clip(store, name);
    if (!clip) {
        fprintf(stderr, "Audio clip not found: %s", name);
        return;
    }

    SetSoundVolume(clip->sound, clip->volume);
    PlaySound(clip->sound);
}
