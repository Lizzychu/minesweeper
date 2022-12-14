#include "menu.h"

// real platform specific menu implementation
extern "C" {
    void *menu_create();
    void menu_destroy(void *menu);
    int menu_attach(void *menu, SDL_Window *window);
    void menu_detach(void *menu);
    int menu_get_event_id(void *menu);
}

MineGameMenu::MineGameMenu()
{
    this->impl = menu_create();
}

MineGameMenu::~MineGameMenu()
{
    if (this->impl != NULL) {
        menu_destroy(this->impl);
    }
}

int MineGameMenu::AttachMenu(SDL_Window *window)
{
    if (this->impl != NULL) {
        return menu_attach(this->impl, window);
    } else {
        return -1;
    }
}

void MineGameMenu::DetachMenu()
{
    if (this->impl != NULL) {
        menu_detach(this->impl);
    }
}

Uint32 MineGameMenu::GetEventId()
{
    return menu_get_event_id(this->impl);
}