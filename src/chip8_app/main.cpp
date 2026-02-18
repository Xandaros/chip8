#include <ctime>
#include <fstream>
#include <ios>
#include <iosfwd>
#include <iostream>
#include <mutex>

#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_thread.h>

#include "cpu.h"

struct AppState {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_AudioStream *audio;
    CPU *cpu;
    bool running;
};

void open_file(void *appstate, const char * const *filelist, int filter) {
    AppState *state = *(AppState **)appstate;

    if (filelist == NULL) {
        // filelist being NULL indicates an error
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
        return;
    }

    const char * const *file = filelist;
    if (*file == NULL) {
        // No file selected
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "No ROM selected.");
        return;
    }

    std::ifstream stream;
    stream.open(*file, std::ios::in | std::ios::binary | std::ios::ate);

    if (!stream.is_open()) {
        return;
    }

    std::streampos size = stream.tellg();
    char *buffer = new char[size];
    stream.seekg(0, std::ios::beg);

    stream.read(buffer, size);

    state->cpu->load_code((uint8_t *)buffer, size);

    stream.close();
    delete[] buffer;

    state->running = true;
}

void test(AppState *state) {
    uint8_t code[] = {
        // Initialisation
        0x60, 0x00, // LD V0, 0
        0x61, 0x00, // LD V1, 0
        0x62, 0x05, // LD V2, 5
        0xA0, 0x00, // LD I, 0

        // Loop over X
        // loop: (8)
        0xD0, 0x15, // DRW V0, V1, 5
        0x70, 0x06, // ADD V0, 6
        0xF2, 0x1E, // ADD I, V2
        0x30, 48, // SE V0, 54
        0x12, 0x08, // JP loop

        // Loop over Y
        0x60, 0x00, // LD V0, 0
        0x71, 0x08, // ADD V1, 0x08
        0x31, 0x10, // SE V1, 0x10
        0x12, 0x08, // JP loop

        // Halt
        // end: (0x20)
        0x12, 0x20, // JP end
    };

    state->cpu->load_code(code, sizeof(code));
}

int cpu_thread(void *appstate) {
    AppState *state = (AppState *)appstate;

    while (true) {
        if (state->running) {
            state->cpu->step();

            // 1ms gives us somewhere between 500 and 1000 Hz clock frequency (depending on the time actually waited)
            SDL_Delay(1);
        }
    }
    return 0;
}

int timer_thread(void *appstate) {
    AppState *state = (AppState *)appstate;

    while (true) {
        if (state->running) {
            state->cpu->tick_timers();

            // Timers should run at 60 Hz
            SDL_Delay(1000 / 60);
        }
    }
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    AppState *state = new AppState();
    state->cpu = new CPU();
    state->running = false;

    std::srand(std::time(NULL));

    // Initialise SDL
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Create window
    if (!SDL_CreateWindowAndRenderer("Chip8", 1024, 768, 0, &state->window, &state->renderer)) {
        SDL_LogCritical(SDL_LOG_CATEGORY_INPUT, "%s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_SetRenderVSync(state->renderer, 1);

    // Create audio stream
    SDL_AudioSpec spec {
        .format = SDL_AUDIO_F32,
        .channels = 1,
        .freq = 8000,
    };
    state->audio = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);

    if (!state->audio) {
        SDL_LogCritical(SDL_LOG_CATEGORY_AUDIO, "Failed to create audio stream: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_SetAudioStreamGain(state->audio, 0.1);

    // Show file dialogue
    *appstate = (void *)state;

    SDL_ShowOpenFileDialog(open_file, appstate, state->window, NULL, 0, NULL, false);

    // Create threads
    SDL_CreateThread(cpu_thread, "CPU Thread", (void *)state);
    SDL_CreateThread(timer_thread, "Timer Thread", (void *)state);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    AppState *state = (AppState *)appstate;

    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }

    if (event->type == SDL_EVENT_KEY_DOWN || event->type == SDL_EVENT_KEY_UP) {
        if (event->key.repeat) {
            // Ignore key repeats on held keys
            return SDL_APP_CONTINUE;
        }
        SDL_Keycode keycode = event->key.key;

        uint8_t key;

        if (keycode >= '0' && keycode <= '9') {
            key = keycode - '0';
        } else if (keycode >= 'a' && keycode <= 'f') {
            key = keycode - 'a' + 0xA;
        } else {
            // Unused key
            return SDL_APP_CONTINUE;
        }

        state->cpu->set_key_down(key, event->key.down);
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult draw_frame(AppState *state) {
    std::lock_guard<std::mutex> lock(state->cpu->display->lock);

    auto vram = state->cpu->display->vram;

    int window_w, window_h;

    if (!SDL_GetWindowSize(state->window, &window_w, &window_h)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to get window size: %s", SDL_GetError());

        return SDL_APP_FAILURE;
    }

    float pixel_w = (float)window_w / (float)Display::WIDTH;
    float pixel_h = (float)window_h / (float)Display::HEIGHT;

    for (int y = 0; y < Display::HEIGHT; ++y) {
        for (int x = 0; x < Display::WIDTH; ++x) {
            uint8_t pixel = vram[y * Display::WIDTH + x];

            SDL_FRect rect = {
                .x = x * pixel_w,
                .y = y * pixel_h,
                .w = pixel_w,
                .h = pixel_h,
            };

            if (pixel != 0) {
                SDL_SetRenderDrawColor(state->renderer, 255, 255, 255, 255);
            } else {
                SDL_SetRenderDrawColor(state->renderer, 0, 0, 0, 255);
            }

            SDL_RenderFillRect(state->renderer, &rect);
        }
    }

    return SDL_APP_CONTINUE;
}

/// Generate audio samples for the beep function
/// @param stream Audio stream for which to generate samples
/// @param current_sample Index of the next sample to be generated
void generate_audio(SDL_AudioStream *stream, int &current_sample) {
    const int MINIMUM_SAMPLES = 4096;
    if (SDL_GetAudioStreamQueued(stream) < MINIMUM_SAMPLES) {
        float samples[MINIMUM_SAMPLES];

        for (int i = 0; i < MINIMUM_SAMPLES; ++i) {
            const int freq = 440;
            const float phase = current_sample * freq / 8000.0f;
            samples[i] = SDL_sinf(phase * 2 * SDL_PI_F);
            current_sample = (current_sample + 1) % 8000;
        }

        SDL_PutAudioStreamData(stream, samples, MINIMUM_SAMPLES);
    }
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    AppState *state = (AppState *)appstate;
    static bool FIRST_RUN = true;
    static int current_audio_sample = 0;

    if (!state->running) {
        return SDL_APP_CONTINUE;
    }

    if (FIRST_RUN) {
        // test(state);

        FIRST_RUN = false;
    }

    generate_audio(state->audio, current_audio_sample);

    // Play audio if ST > 0
    if (state->cpu->st > 0) {
        SDL_ResumeAudioStreamDevice(state->audio);
    } else {
        SDL_PauseAudioStreamDevice(state->audio);
    }

    SDL_SetRenderDrawColor(state->renderer, 0, 0, 0, 255);
    SDL_RenderClear(state->renderer);

    SDL_AppResult result = draw_frame(state);
    if (result != SDL_APP_CONTINUE) {
        return result;
    }

    SDL_RenderPresent(state->renderer);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    AppState *state = (AppState *)appstate;
    state->running = false;
    delete state->cpu;
    delete state;
}
