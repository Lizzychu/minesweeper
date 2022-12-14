#include <SDL_syswm.h>
#include "sdl_headers.h"
//#include <stdio.h>
#include <iostream>

extern "C" {
    void *menu_create();
    void menu_destroy(void *menu);
    int menu_attach(void *menu, SDL_Window *window);
    void menu_detach(void *menu);
    int menu_get_event_id(void *menu);
}

struct WindowsMenu {
    HWND hwnd;
    HMENU menubar;
    HMENU menu;
    HHOOK hook;
    Uint32 event_id;
};

static struct WindowsMenu the_only_menu;

LRESULT CALLBACK menu_hook(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode < 0) {
        return CallNextHookEx(NULL, nCode, wParam, lParam);
    }

    // convert WM_COMMAND from menu to SDL event accordingly
    MSG *ptr = (MSG *)lParam;
    if (ptr->message == WM_COMMAND) {
        SDL_Event event;
        UINT id;

        id = LOWORD(ptr->wParam);

        // create a event and push to SDL event queue
        SDL_memset(&event, 0, sizeof(event));
        event.type = the_only_menu.event_id;
        event.user.code = id;
        event.user.data1 = 0;
        event.user.data2 = 0;

        SDL_PushEvent(&event);

        // check / uncheck menu item
        for (unsigned int i = 0; i <= 2; i++) {
            if (id == i) {
                CheckMenuItem(the_only_menu.menu, (UINT)i, MF_CHECKED);
            } else {
                CheckMenuItem(the_only_menu.menu, (UINT)i, MF_UNCHECKED);
            }
        }
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

static int menu_attach_menu(struct WindowsMenu *win_menu, SDL_Window *window)
{
    SDL_SysWMinfo info;

    // get HWND from SDL_Windows 
    SDL_VERSION(&info.version);

    if (SDL_GetWindowWMInfo(window, &info) != SDL_TRUE) {
        //printf("SDL_GetWindowWMInfo: %d\n", SDL_GetError());
        std::cout << "SDL_GetWindowWMInfo: " << SDL_GetError() << std::endl;
        return -1;
    }

    // attach menus to HWND
    HWND hwnd  = info.info.win.window;
    HMENU menubar = CreateMenu();
    HMENU menu = CreateMenu();

    AppendMenu(menubar, MF_POPUP, (UINT_PTR)menu, "Level");
    AppendMenu(menu, MF_STRING, 0, "Easy");
    AppendMenu(menu, MF_STRING, 1, "Intermediate");
    AppendMenu(menu, MF_STRING, 2, "Expert");

    SetMenu(hwnd, menubar);

    win_menu->hwnd = hwnd;
    win_menu->menubar = menubar;
    win_menu->menu = menu;

    return 0;
}

static int menu_create_hook(struct WindowsMenu *win_menu)
{
    HHOOK ret;

    // create a hook function to receive WM_COMMAND events
    // - this can also be done in event main loop with SDL_SysWMmsg events
    // - however, we hide the implementation here to simply main window event processing code and 
    ret = SetWindowsHookEx(WH_GETMESSAGE, menu_hook, (HINSTANCE)NULL, GetCurrentThreadId());
    if (ret == NULL) {
        //printf("SetWindowsHookEx: %d\n", GetLastError());
        std::cout << "SetWindowsHookEx: " << GetLastError() << std::endl;
        return -1;
    }

    win_menu->hook = ret;

    return 0;
}

static int menu_register_sdl_event(struct WindowsMenu *win_menu)
{
    Uint32 event_id;

    event_id = SDL_RegisterEvents(1);
    if (event_id == (Uint32)(-1)) {
        //printf("SDL_RegisterEvents failed with error: %d\n", SDL_GetError());
        std::cout << "SDL_RegisterEvents failed with error: " << SDL_GetError() << std::endl;
        return -1;
    }

    win_menu->event_id = event_id;

    return 0;
}

void *menu_create()
{
    return &the_only_menu;
}

void menu_destroy(void *menu)
{
    // do nothing
}

int menu_get_event_id(void *menu)
{
    struct WindowsMenu *win_menu = (struct WindowsMenu *)menu;

    return win_menu->event_id;
}

int menu_attach(void *menu, SDL_Window *window)
{
    struct WindowsMenu *win_menu = (struct WindowsMenu *)menu;

    menu_attach_menu(win_menu, window);
    menu_create_hook(win_menu);
    menu_register_sdl_event(win_menu);

    return 0;
}

void menu_detach(void *menu)
{
    struct WindowsMenu *win_menu = (struct WindowsMenu *)menu;

    // A menu that is assigned to a window is automatically destroyed when the application closes.

    // remove hook
    UnhookWindowsHookEx(win_menu->hook);
}
