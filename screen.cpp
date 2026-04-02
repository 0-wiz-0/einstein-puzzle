// This file is part of Einstein Puzzle

// Einstein Puzzle
// Copyright (C) 2003-2005  Flowix Games

// Modified 2012-05-06 by Jordan Evens <jordan.evens@gmail.com>

// Einstein Puzzle is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// Einstein Puzzle is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "screen.h"
#include <SDL.h>
#include <SDL_video.h>

#include "exceptions.h"
#include "unicode.h"
#include "utils.h"

#include <sstream>

#define UNSCALED_WIDTH 800
#define UNSCALED_HEIGHT 600

int DESKTOP_WIDTH = 0;
int DESKTOP_HEIGHT = 0;

Screen::Screen()
    : screen(nullptr), window(nullptr), renderer(nullptr), texture(nullptr),
      scale(1.0), fullScreen(false), screenSize(0), mouseCursor(nullptr),
      mouseVisible(false), saveX(0), saveY(0), niceCursor(false),
      cursor(nullptr) {
}

Screen::~Screen() {
    SDL_SetCursor(cursor);
    SDL_ShowCursor(SDL_ENABLE);
    if (mouseCursor) {
        SDL_FreeCursor(mouseCursor);
        mouseCursor = nullptr;
    }
}

void Screen::setMode(bool isFullScreen) {
    if (!screen || fullScreen != isFullScreen) {
        fullScreen = isFullScreen;
        applyMode();
    }
}

void Screen::applyMode() {
    int flags
        = (fullScreen) ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_RESIZABLE;
    if (texture) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (screen) {
        SDL_FreeSurface(screen);
        screen = nullptr;
    }
    if (window) {
        SDL_SetWindowSize(window, UNSCALED_WIDTH, UNSCALED_HEIGHT);
        SDL_SetWindowFullscreen(window, flags);
    }
    else {
        window = SDL_CreateWindow("Einstein", SDL_WINDOWPOS_UNDEFINED,
                                  SDL_WINDOWPOS_UNDEFINED, UNSCALED_WIDTH,
                                  UNSCALED_HEIGHT, flags | SDL_WINDOW_HIDDEN);
    }
    if (!window) {
        throw Exception(L"Couldn't create window: " + fromMbcs(SDL_GetError()));
    }

    renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        throw Exception(L"Couldn't create renderer: "
                        + fromMbcs(SDL_GetError()));
    }
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                SDL_TEXTUREACCESS_STREAMING, UNSCALED_WIDTH,
                                UNSCALED_HEIGHT);
    if (!texture) {
        throw Exception(L"Couldn't create texture: "
                        + fromMbcs(SDL_GetError()));
    }

    screen
        = SDL_CreateRGBSurface(0, UNSCALED_WIDTH, UNSCALED_HEIGHT, 32,
                               0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);

    if (!screen) {
        throw Exception(L"Couldn't create screen: "
                        + fromMbcs((SDL_GetError())));
    }
}

int Screen::getWidth() const {
    return UNSCALED_WIDTH;
}

int Screen::getHeight() const {
    return UNSCALED_HEIGHT;
}

void Screen::setMouseImage(SDL_Surface *image) {
    if (mouseCursor) {
        SDL_FreeCursor(mouseCursor);
        mouseCursor = nullptr;
    }
    if (!image) {
        return;
    }

    SDL_Surface *mouseImage
        = SDL_ConvertSurfaceFormat(image, SDL_PIXELFORMAT_ARGB8888, 0);
    mouseCursor = SDL_CreateColorCursor(mouseImage, 0, 0);
}

void Screen::hideMouse() {
    if (!mouseVisible) {
        return;
    }

    if (!niceCursor) {
        mouseVisible = false;
        return;
    }

    SDL_ShowCursor(SDL_DISABLE);
    mouseVisible = false;
}

void Screen::showMouse() {
    if (mouseVisible) {
        return;
    }

    if (!niceCursor) {
        mouseVisible = true;
        return;
    }

    if (mouseCursor != NULL) {
        SDL_SetCursor(mouseCursor);
    }
    SDL_ShowCursor(SDL_ENABLE);
    mouseVisible = true;
}

void Screen::updateMouse() {
    // TODO: remove
}

void Screen::flush() {
    static uint32_t last_flush = 0;
    uint32_t now = SDL_GetTicks();
    if (now - last_flush > 30) // cap at 33 fps
    {
        last_flush = now;
        // do the actual swap...
        SDL_UpdateTexture(texture, NULL, screen->pixels, screen->pitch);
        // SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        SDL_ShowWindow(window);
    }
}

void Screen::draw(int x, int y, SDL_Surface *tile) {
    blitDraw(x, y, tile, screen);
}

void Screen::drawScaled(int x, int y, SDL_Surface *tile) {
    SDL_Surface *s = scaleUp(tile);
    blitDraw(scaleUp(x), scaleUp(y), s, screen);
    SDL_FreeSurface(s);
}

void Screen::setCursor(bool nice) {
    if (nice == niceCursor) {
        return;
    }

    if (niceCursor) {
        SDL_SetCursor(mouseCursor);
    }
    else {
        SDL_SetCursor(cursor);
    }
    niceCursor = nice;
}

void Screen::initCursors() {
    cursor = SDL_GetCursor();
}

void Screen::doneCursors() {
    if (niceCursor) {
        SDL_SetCursor(cursor);
    }
}

SDL_Surface *Screen::createSubimage(int x, int y, int width, int height) {
    SDL_Surface *s = SDL_CreateRGBSurface(
        0, scaleUp(width), scaleUp(height), screen->format->BitsPerPixel,
        screen->format->Rmask, screen->format->Gmask, screen->format->Bmask,
        screen->format->Amask);
    if (!s) {
        throw Exception(L"Error creating buffer surface");
    }
    SDL_Rect src = {scaleUp(x), scaleUp(y), scaleUp(width), scaleUp(height)};
    SDL_Rect dst = {0, 0, src.w, src.h};
    SDL_BlitSurface(screen, &src, s, &dst);
    return s;
}

void Screen::drawWallpaper(const std::wstring &name) {
    drawTiled(name, screen);
}

SDL_PixelFormat *Screen::getFormat() {
    return screen->format;
}

void Screen::setClipRect(SDL_Rect *rect) {
    if (rect) {
        SDL_Rect sRect = {scaleUp(rect->x), scaleUp(rect->y), scaleUp(rect->w),
                          scaleUp(rect->h)};
        SDL_SetClipRect(screen, &sRect);
    }
    else {
        SDL_SetClipRect(screen, rect);
    }
}

void Screen::setSize(int size) {
    if (screenSize != size) {
        screenSize = size;
        if (!fullScreen) {
            applyMode();
        }
    }
}

float Screen::getScale() {
    return scale;
}

void Screen::getMouse(int *x, int *y) {
    SDL_GetMouseState(x, y);
    convertMouse(x, y);
}

void Screen::convertMouse(int *x, int *y) {
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    (*x) = ((*x) * screen->w) / w;
    (*y) = ((*y) * screen->h) / h;
}
