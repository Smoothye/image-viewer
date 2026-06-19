#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <stdio.h>
#include <stdlib.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_render.h>

/* We will use this renderer to draw into this window every frame. */
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

typedef enum {
    PPM,
    // JPG,
    // PNG,
    // GIF,
    // BMP,
    UNSUPPORTED
}  img_file_type;

typedef struct {
    uint8_t r, g, b;
} rgb;

typedef struct {
    int32_t width, height;
    rgb *data;

} img_data;

uint8_t *read_file_bytes(FILE* fd, long *file_len) {

    fseek(fd, 0, SEEK_END);
    *file_len = ftell(fd);
    rewind(fd);

    uint8_t *bytes = malloc(*file_len * sizeof(uint8_t));
    if (!bytes) {
        fputs("Could not allocate memory for bytes buffer.\n", stderr);
        *file_len = 0;
        return NULL;
    }

    fread(bytes, 1, *file_len, fd);

    return bytes;
}

img_file_type determine_file_type(const uint8_t *bytes) {

    if (!bytes)
        return UNSUPPORTED;

    if (bytes[0] == 'P' && bytes[1] == '6')
        return PPM;

    return UNSUPPORTED;
}

img_data extract_ppm_data(const uint8_t *bytes, long bytes_count) {

    const uint8_t *end = bytes + bytes_count;

    // skip magic number
    bytes += 2;

    int32_t width = 0, height = 0;
    while (bytes < end && !height) { // extract width and height

        if (*bytes == '#') { // comment, skip bytes until newline '\n'
            while (bytes < end && *bytes != '\n')
                ++bytes;
            ++bytes; // drop newline '\n'
        }

        int32_t *magic = NULL;
        if (*bytes >= '0' && *bytes <= '9') {

            magic = width ? &height : &width;

            while (*bytes >= '0' && *bytes <= '9') {
                *magic = (*magic) * 10 + *bytes - '0';
                ++bytes;
            }
        }

        ++bytes;
    }

    // skip whitespace
    while (bytes < end && (*bytes < '0' || *bytes > '9'))
        ++bytes;

    int32_t max_color_val = 0;
    while (*bytes >= '0' && *bytes <= '9') {
        max_color_val = max_color_val * 10 + *bytes - '0';
        ++bytes;
    }

    // skip last whitespace
    ++bytes;

    // image color raster begins
    rgb *colors = malloc(width * height * sizeof(rgb));
    if (!colors) {
        fputs("Could not allocate memory for color buffer\n", stderr);
        return (img_data) { .width = 0, .height = 0, .data = NULL };
    }

    rgb *colors_itr = colors;

    uint8_t ctr = 0;
    while (bytes < end) {

        ++ctr;
        if (ctr == 1)
            colors_itr->r = *bytes;
        else if (ctr == 2)
            colors_itr->g = *bytes;
        else {
            colors_itr->b = *bytes;
            ++colors_itr;
            ctr = 0;
        }

        ++bytes;
    }

    const img_data result = { .width = width, .height = height, .data = colors };
    return result;
}

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    FILE *img_file = NULL;
    if (argc != 2) {
        fputs("Must give 1 command line argument.\n", stderr);
        return SDL_APP_FAILURE;
    }
    else {
        img_file = fopen(argv[1], "rb");
    }
    if (!img_file) {
        fputs("File could not be opened\n", stderr);
        return SDL_APP_FAILURE;
    }

    long bytes_count;
    uint8_t *bytes = read_file_bytes(img_file, &bytes_count);

    fclose(img_file);

    if (!bytes)
        return SDL_APP_FAILURE;

    img_data img = {0};
    switch (determine_file_type(bytes)) {
        case PPM: {
            img = extract_ppm_data(bytes, bytes_count);
            break;
        }
        case UNSUPPORTED: {
            fputs("Given file type is not supported.\n", stderr);
            free(bytes);
            return SDL_APP_FAILURE;
        }
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