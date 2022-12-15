#import <Cocoa/Cocoa.h>
#include "sdl_headers.h"

@interface MacMenu: NSObject {
    NSMenu *levelMenu;
    NSMenuItem *mainMenuItem;
    NSMenuItem *demo;
    NSMenuItem *easy;
    NSMenuItem *intermediate;
    NSMenuItem *hard;
    Uint32 eventId;
}

// instance method
- (id)init;
- (int)attachMainMenu;
- (void)createSDLEvent:(int)id;
- (void)onClickDEMO;
- (void)onClickEasy;
- (void)onClickIntermediate;
- (void)onClickHard;

@property(nonatomic, readwrite) Uint32 eventId;

@end

@implementation MacMenu
- (id)init
{
    self = [super init];

    if (self) {
        /* create level menu */
        levelMenu = [[NSMenu alloc] initWithTitle:@"Level"];

        /* add menu items */
        demo = [levelMenu addItemWithTitle:@"DEMO" action:nil keyEquivalent:@""];
        easy = [levelMenu addItemWithTitle:@"Easy" action:nil keyEquivalent:@""];
        intermediate = [levelMenu addItemWithTitle:@"Intermediate" action:nil keyEquivalent:@""];
        hard = [levelMenu addItemWithTitle:@"Hard" action:nil keyEquivalent:@""];

        /* setup target and actions */
        [demo setTarget: self];
        [demo setAction:@selector(onClickDEMO)];
        [easy setTarget: self];
        [easy setAction:@selector(onClickEasy)];
        [intermediate setTarget: self];
        [intermediate setAction:@selector(onClickIntermediate)];
        [hard setTarget: self];
        [hard setAction:@selector(onClickHard)];


        /* put menu into the menubar */
        mainMenuItem = [[NSMenuItem alloc] initWithTitle:@"Level" action:nil keyEquivalent:@""];
        [mainMenuItem setSubmenu: levelMenu];

        eventId = (Uint32)-1;
    }

    return self;
}

- (int)attachMainMenu
{
    NSLog(@"attachMainMenu");

    if (NSApp == nil) {
        return -1;
    }

    /* add mainMenuItem to application's main menu */
    [[NSApp mainMenu] addItem: mainMenuItem];

    return 0;
}

- (void)createSDLEvent:(int)id
{
    SDL_Event event;

    // create a event and push to SDL event queue
    SDL_memset(&event, 0, sizeof(event));
    event.type = self.eventId;
    event.user.code = id;
    event.user.data1 = 0;
    event.user.data2 = 0;

    SDL_PushEvent(&event);
}

- (void)onClickDEMO
{
    NSLog(@"onClickDEMO");
    [self createSDLEvent: 0];
}

- (void)onClickEasy
{
    NSLog(@"onClickEasy");
    [self createSDLEvent: 1];
}

- (void)onClickIntermediate
{
    NSLog(@"onClickIntermediate");
    [self createSDLEvent: 2];
}

- (void)onClickHard
{
    NSLog(@"onClickHard");
    [self createSDLEvent: 3];
}

@end

void *menu_create()
{
    return [[MacMenu alloc] init];
}

void menu_destroy(void *menu)
{

}

static int menu_attach_menu(MacMenu *mac_menu)
{
    NSLog(@"menu_attach_menu\n");
    return [mac_menu attachMainMenu];
}

static int menu_register_sdl_event(MacMenu *mac_menu)
{
    Uint32 event_id;

    event_id = SDL_RegisterEvents(1);
    if (event_id == (Uint32)(-1)) {
        NSLog(@"SDL_RegisterEvents failed with error: %s\n", SDL_GetError());
        return -1;
    }

    mac_menu.eventId = event_id;

    return 0;
}

int menu_attach(void *menu, SDL_Window *window)
{
    MacMenu *mac_menu = (MacMenu *)menu;

    menu_attach_menu(mac_menu);
    menu_register_sdl_event(mac_menu);

    return 0;
}

void menu_detach(void *menu)
{
    NSLog(@"detaching menu");
}

int menu_get_event_id(void *menu)
{
    MacMenu *mac_menu = (MacMenu *)menu;
    return (int)mac_menu.eventId;
}