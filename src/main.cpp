#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "cpu.h"

struct AppState {
    SDL_Window *window;
    SDL_Renderer *renderer;
    CPU *cpu;
};

void open_file(void *userdata, const char * const *filelist, int filter) {
    if (filelist == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
        return;
    }

    const char * const *cur = filelist;
    while (*cur != NULL) {
        SDL_Log("%s", *cur);
        cur++;
    }
}

void test(AppState *state) {
    uint8_t code[] = {
        0x00, 0xE0, // CLS
        0x12, 0x02, // JMP 0x001
    };

    state->cpu->load_code(code, sizeof(code));

    for (int i = 0; i < 10; ++i) {
        state->cpu->step();
    }
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    AppState *state = new AppState();
    state->cpu = new CPU();

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("Chip8", 1024, 768, 0, &state->window, &state->renderer)) {
        SDL_LogCritical(SDL_LOG_CATEGORY_INPUT, "%s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_SetRenderVSync(state->renderer, 1);

    *appstate = (void *)state;

    // SDL_ShowOpenFileDialog(open_file, NULL, state->window, NULL, 0, NULL, true);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    AppState *state = (AppState *)appstate;
    static bool FIRST_RUN = true;

    if (FIRST_RUN) {
        test(state);

        FIRST_RUN = false;
    }

    SDL_SetRenderDrawColor(state->renderer, 0, 0, 0, 255);
    SDL_RenderClear(state->renderer);

    SDL_FRect rect = {.x = 20, .y = 20, .w = 200, .h = 200};

    SDL_SetRenderDrawColor(state->renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(state->renderer, &rect);

    SDL_SetRenderScale(state->renderer, 4.0f, 4.0f);
    SDL_RenderDebugText(state->renderer, 5.0f, 300.0f / 4.0f, "Test");
    SDL_SetRenderScale(state->renderer, 1.0f, 1.0f);

    SDL_RenderPresent(state->renderer);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    AppState *state = (AppState *)appstate;
    delete state->cpu;
    delete state;
}
