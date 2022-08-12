#pragma once

#include "SDL.h"

#define USE_GL 1

struct R_Texture;

#if USE_GL

#define SDL_Texture R_Texture

#define SDL_PIXELFORMAT_RGBA32 R_PixelFormat_RGBA

#define SDL_CreateRenderer(window, ...) (SDL_Renderer*)R_CreateRenderer(window)
#define SDL_RenderClear(...) R_RenderClear()
#define SDL_RenderPresent(...) R_RenderPresent()
#define SDL_RenderCopy(renderer, ...) R_RenderCopy(__VA_ARGS__)
#define SDL_RenderCopyEx(renderer, ...) R_RenderCopyEx(__VA_ARGS__)
#define SDL_RenderDrawLine(renderer, ...) R_RenderDrawLine(__VA_ARGS__);
#define SDL_RenderFillRect(renderer, ...) R_RenderFillRect(__VA_ARGS__);
#define SDL_RenderDrawRect(renderer, ...) R_RenderDrawRect(__VA_ARGS__);

#define SDL_SetRenderDrawColor(renderer, ...) R_SetRenderDrawColor(__VA_ARGS__)
#define SDL_SetRenderDrawBlendMode(renderer, ...) R_SetRenderDrawBlendMode(__VA_ARGS__)
#define SDL_SetTextureColorMod(...) R_SetTextureColorMod(__VA_ARGS__)
#define SDL_SetTextureBlendMode(...) R_SetTextureBlendMode(__VA_ARGS__)

// Textures:
#define SDL_CreateTexture(renderer, ...) R_CreateTexture(__VA_ARGS__)
#define SDL_DestroyTexture(...) R_DestroyTexture(__VA_ARGS__)
#define SDL_LockTexture(...) R_LockTexture(__VA_ARGS__)
#define SDL_UnlockTexture(...) R_UnlockTexture(__VA_ARGS__)
#define SDL_QueryTexture(...) R_QueryTexture(__VA_ARGS__)

#endif

// Drawing:
//
void        R_RenderClear();
void        R_RenderPresent();
void        R_SetRenderDrawColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);
void        R_SetRenderDrawBlendMode(SDL_BlendMode mode);

void        R_RenderCopy(R_Texture* texture,
                         SDL_Rect* source,
                         SDL_Rect* dest);
void        R_RenderCopyEx(R_Texture* texture,
                           SDL_Rect* source,
                           SDL_Rect* dest,
                           double angle,
                           SDL_Point* center,
                           SDL_RendererFlip flip);


enum R_PixelFormat
{
    R_PixelFormat_Invalid,
    R_PixelFormat_RGBA,
    R_PixelFormat_R,
};


// Renderer:
void*       R_CreateRenderer(SDL_Window* window);
void        R_DestroyRenderer();

// Textures:
R_Texture*  R_CreateTexture(R_PixelFormat format,
                            int access,
                            int width,
                            int height);
void        R_DestroyTexture(R_Texture* texture);
bool        R_LockTexture(R_Texture* texture,
                          SDL_Rect* region,
                          void** pixels,
                          int* pitch);
void        R_UnlockTexture(R_Texture* texture);
void        R_QueryTexture(R_Texture* texture,
                           uint32_t* format,
                           int* access,
                           int* width,
                           int* height);
void        R_SetTextureColorMod(R_Texture* texture,
                                 uint8_t red,
                                 uint8_t green,
                                 uint8_t blue);
void        R_SetTextureBlendMode(R_Texture* texture,
                                  SDL_BlendMode mode);

// Drawing:
//
void        R_RenderClear();
void        R_RenderPresent();
void        R_SetRenderDrawColor(uint8_t red,
                                 uint8_t green,
                                 uint8_t blue,
                                 uint8_t alpha);

void        R_RenderCopy(R_Texture* texture,
                         SDL_Rect* source,
                         SDL_Rect* dest);
void        R_RenderCopyEx(R_Texture* texture,
                           SDL_Rect* source,
                           SDL_Rect* dest,
                           double angle,
                           SDL_Point* center,
                           SDL_RendererFlip flip);
void        R_RenderDrawLine(int x1,
                             int y1,
                             int x2,
                             int y2);
void        R_RenderFillRect(SDL_Rect* rect);
void        R_RenderDrawRect(SDL_Rect* rect);

