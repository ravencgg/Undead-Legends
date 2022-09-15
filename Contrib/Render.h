#pragma once

#include "SDL.h"

#define USE_GL 1
#define REF(V) ((void)V)

struct R_Texture;

enum R_PixelFormat
{
    R_PixelFormat_Invalid,
    R_PixelFormat_RGBA,
    R_PixelFormat_R,
};

struct R_View
{
    double x, y;
    double sx, sy;
};

struct R_Rect
{
    double x, y;
    double w, h;
};

// Drawing:
//
void        R_RenderClear();
void        R_RenderPresent();
void        R_SetRenderDrawColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);
void        R_SetRenderDrawBlendMode(SDL_BlendMode mode);

void        R_RenderCopy(R_Texture* texture,
                         R_Rect* source,
                         R_Rect* dest);
void        R_RenderCopyEx(R_Texture* texture,
                           R_Rect* source,
                           R_Rect* dest,
                           double angle,
                           SDL_Point* center,
                           SDL_RendererFlip flip);

// Renderer:
void*       R_CreateRenderer(SDL_Window* window);
void        R_DestroyRenderer();

// Textures:
R_Texture*  R_CreateTexture(R_PixelFormat format,
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
void        R_BeginFrame(R_View view);
void        R_RenderClear();
void        R_RenderPresent();
void        R_SetRenderDrawColor(uint8_t red,
                                 uint8_t green,
                                 uint8_t blue,
                                 uint8_t alpha);

void        R_RenderCopy(R_Texture* texture,
                         R_Rect* source,
                         R_Rect* dest);
void        R_RenderCopyEx(R_Texture* texture,
                           R_Rect* source,
                           R_Rect* dest,
                           double angle,
                           SDL_Point* center,
                           SDL_RendererFlip flip);
void        R_RenderDrawLine(double x1,
                             double y1,
                             double x2,
                             double y2);
void        R_RenderFillRect(R_Rect* rect);
void        R_RenderDrawRect(R_Rect* rect);

void        R_BeginWorldDrawing();
void        R_BeginUIDrawing();
