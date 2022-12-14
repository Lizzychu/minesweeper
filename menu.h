#include "sdl_headers.h"

#ifndef __MINE_GAME_MENU_H__
#define __MINE_GAME_MENU_H__

class MineGameMenu {
    public:
        MineGameMenu();
        ~MineGameMenu();

        int AttachMenu(SDL_Window *window);
        void DetachMenu();
        Uint32 GetEventId();

    private:
        void *impl;
};


#endif
