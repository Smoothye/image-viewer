#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_render.h>

#include "image_viewer_utils.h"
#include "netpbm_utils.h"

/* We will use this renderer to draw into this window every frame. */
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    if (argc != 2) {
        fputs("Must give 1 command line argument.\n", stderr);
        return SDL_APP_FAILURE;
    }

    FILE *img_file = fopen(argv[1], "rb");
    if (!img_file) {
        fprintf(stderr, "File `%s` could not be opened\n", argv[1]);
        return SDL_APP_FAILURE;
    }

    long bytes_count;
    uint8_t *bytes = read_file_bytes(img_file, &bytes_count);

    fclose(img_file);

    if (!bytes)
        return SDL_APP_FAILURE;

    img_data img = {0};
    switch (determine_file_type(bytes, bytes_count)) {
        case PBM:
            img = load_pbm_from_memory(bytes, bytes_count);
            break;
        case PGM:
        case PPM:
            img = load_ppm_or_pgm_from_memory(bytes, bytes_count);
            break;
        case UNSUPPORTED:
            fputs("Given file type is not supported.\n", stderr);
            free(bytes);
            return SDL_APP_FAILURE;
    }

    free(bytes);

    if (!img.data)
        return SDL_APP_FAILURE;

    SDL_SetAppMetadata("Image viewer", "1.0", NULL);

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        free(img.data);
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("image viewer", img.width, img.height, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        free(img.data);
        return SDL_APP_FAILURE;
    }
    SDL_SetRenderLogicalPresentation(renderer, img.width, img.height, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    for (int32_t y = 0; y < img.height; ++y) {
        for (int32_t x = 0; x < img.width; ++x) {

            const uint32_t idx = y * img.width + x;
            SDL_SetRenderDrawColor(renderer, img.data[idx].r, img.data[idx].g, img.data[idx].b, SDL_ALPHA_OPAQUE);
            SDL_RenderPoint(renderer, (float)x, (float)y);
        }
    }
    free(img.data);

    SDL_RenderPresent(renderer);

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc.) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }
    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once per frame and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    /* SDL will clean up the window/renderer for us. */
}