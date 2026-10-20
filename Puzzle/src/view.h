#ifndef PORTAL_VIEW_H
#define PORTAL_VIEW_H
#include "game.h"
#include "raylib.h"

#define VIEW_W 1280
#define VIEW_H 820
#define BOARD_X 32
#define BOARD_Y 160
#define TILE 64

typedef enum { UI_NONE, UI_CONTINUE, UI_NEW, UI_QUIT, UI_RESUME, UI_PAUSE, UI_NEXT,
    UI_UNDO, UI_RESET, UI_CONSOLE, UI_RUN, UI_BACKSPACE, UI_CLEAR,
    UI_COMMAND, UI_DUEL_START, UI_BLOCK, UI_SUBMIT, UI_COUNTER_CLEAR, UI_COUNTER_BACKSPACE,
    UI_FINISH, UI_HINT, UI_MUTE, UI_DIAGNOSIS_HINT, UI_GUIDE,
    UI_STORY, UI_STORY_BEGIN, UI_STORY_PAUSE, UI_STORY_RESTART } UiKind;
typedef struct { UiKind kind; int arg; } UiAction;
typedef enum { SCREEN_TITLE, SCREEN_PLAY, SCREEN_PAUSE, SCREEN_STORY } Screen;
typedef struct { Vector2 position, velocity; float life; Color color; } Particle;
typedef struct {
    Font font, mono;
    Sound sounds[5];
    bool audio, muted, custom_font, custom_mono, guide;
    Texture2D story_images[3];
    RenderTexture2D story_text;
    float story_elapsed, story_height, story_duration;
    int story_room;
    bool story_paused;
    Vector2 mouse, hero_pos, bot_pos;
    float time;
    int visual_room;
    unsigned seen_event;
    Particle particles[72];
} View;

void view_init(View *view, bool audio);
void view_destroy(View *view);
void view_update(View *view, const Game *game, Screen screen, float dt);
void view_start_story(View *view, int room);
UiAction view_draw(View *view, const Game *game, Screen screen, int checkpoint);

#endif
