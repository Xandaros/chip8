#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

struct AppState {
    SDL_Window *window;
    SDL_Renderer *renderer;
};

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    AppState *state = new AppState();

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
        SDL_RenderClear(state->renderer);
        SDL_RenderPresent(state->renderer);
        FIRST_RUN = false;
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    delete (AppState *)appstate;
}
