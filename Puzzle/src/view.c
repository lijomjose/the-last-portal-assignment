#include "view.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "story.h"

static const Color PAL_BG = {12,19,33,255}, PAL_PANEL = {20,31,49,255};
static const Color PAL_FLOOR = {34,48,65,255}, PAL_LINE = {56,75,93,255};
static const Color PAL_INK = {235,241,239,255}, PAL_MUTED = {160,181,192,255};
static const Color PAL_TEAL = {104,231,205,255}, PAL_GOLD = {247,199,114,255};
static const Color PAL_RED = {237,130,131,255}, PAL_BLUE = {111,166,228,255};
static const char *room_titles[] = { "A memory of a mistake", "A route to remember", "Clash of Queue and Stack" };
static const char *room_short[] = { "THE MEMORY ROOM", "THE MEMORY RUN", "CLASH OF QUEUE AND STACK" };

static Vector2 centre(Cell c) { return (Vector2){BOARD_X + c.x*TILE + TILE/2, BOARD_Y + c.y*TILE + TILE/2}; }
static void text(View *v, const char *s, float x, float y, float size, Color color)
{ DrawTextEx(v->font, s, (Vector2){x,y}, size, .2f, color); }
static void mono(View *v, const char *s, float x, float y, float size, Color color)
{ DrawTextEx(v->mono, s, (Vector2){x,y}, size, .1f, color); }
static float text_width(View *v, const char *s, float size)
{ return MeasureTextEx(v->font,s,size,.2f).x; }

static float wrapped(View *v, const char *content, float x, float y, float width, float size, Color color)
{
    char line[256] = "", word[160];
    const char *p = content;
    while (*p) {
        if (*p == '\n') { text(v,line,x,y,size,color); y += size*1.4f; line[0]=0; p++; continue; }
        while (*p == ' ') p++;
        int n=0;
        while (*p && *p != ' ' && *p != '\n') { if(n<158) word[n++]=*p; p++; }
        word[n]=0;
        if (!n) continue;
        char candidate[420];
        snprintf(candidate,sizeof(candidate),"%s%s%s",line,line[0]?" ":"",word);
        if (text_width(v,candidate,size)>width && line[0]) {
            text(v,line,x,y,size,color); y += size*1.4f;
            snprintf(line,sizeof(line),"%s",word);
        } else {
            size_t len = strlen(line);
            if (len && len < sizeof(line)-1) line[len++]=' ';
            snprintf(line+len,sizeof(line)-len,"%s",word);
        }
    }
    if (line[0]) { text(v,line,x,y,size,color); y += size*1.4f; }
    return y;
}

void view_start_story(View *v, int room)
{
    if(room<0||room>2) return;
    v->story_room=room; v->story_elapsed=0; v->story_paused=false;
    if(v->story_text.id) {
        BeginTextureMode(v->story_text);
        ClearBackground(BLANK);
        v->story_height=wrapped(v,room_stories[room].narrative,24,26,720,32,PAL_GOLD)+24;
        EndTextureMode();
    } else v->story_height=700;
    /* Finish only the animation, never auto-start play. The fixed mission
     * card remains until Enter so slower readers do not lose instructions. */
    v->story_duration=(v->story_height+360)/30.0f;
}

static bool button(View *v, Rectangle rect, const char *label, bool enabled, bool primary)
{
    bool hover = enabled && CheckCollisionPointRec(v->mouse,rect);
    Color fill = primary ? PAL_TEAL : (Color){35,51,70,255};
    if (hover) fill = primary ? (Color){148,247,224,255} : (Color){51,73,94,255};
    if (!enabled) fill = (Color){25,37,54,255};
    DrawRectangleRounded(rect,.17f,5,fill);
    float size=18;
    Color ink = !enabled ? (Color){88,110,127,255} : primary ? PAL_BG : PAL_INK;
    text(v,label,rect.x+(rect.width-text_width(v,label,size))/2,rect.y+(rect.height-size)/2-1,size,ink);
    return hover && IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
}

static void key(View *v, const char *label, float x, float y, const char *meaning)
{
    float w = MeasureTextEx(v->mono,label,16,.1f).x+18;
    DrawRectangleRounded((Rectangle){x,y,w,29},.2f,4,(Color){32,48,65,255});
    mono(v,label,x+9,y+6,16,PAL_INK);
    text(v,meaning,x+w+7,y+5,17,PAL_MUTED);
}

static void arrow(Vector2 from, Vector2 to, Color color, float width)
{
    DrawLineEx(from,to,width,color);
    Vector2 d={to.x-from.x,to.y-from.y};
    float length=sqrtf(d.x*d.x+d.y*d.y);
    if(length<1) return;
    d.x/=length; d.y/=length;
    Vector2 a={to.x-d.x*9+d.y*5,to.y-d.y*9-d.x*5};
    Vector2 b={to.x-d.x*9-d.y*5,to.y-d.y*9+d.x*5};
    DrawTriangle(to,a,b,color);
}

static void hero(Vector2 pos, float time, int pixel)
{
    /* Match the course Caitlyn: tousled brown hair, orange field jacket,
     * red scarf, leather boots and a turquoise command device. */
    static const char sprite[][25]={
        ".......hh..hh...........",
        ".....hhHHhhHHh..........",
        "....hHHHHHHHHhh.........",
        "...hHHhHHHhhHHHh........",
        "....hHffffhffhHHh.......",
        "...hHfffewffewhHh.......",
        "....hsffefsfefhH........",
        ".....hfffffffsh.........",
        "......ssffffsh..........",
        ".......sRRRrs...........",
        ".....brRRRRRRrb..RRr....",
        "....boOrRRRrOobRRrr.....",
        "...boOOoottooOobr.......",
        "...boOOotgtooOob........",
        "..bsoOOotgtooOob........",
        "..bsboObtgtoObos........",
        "...sbooogggooboss.......",
        "...sboooddddooocds......",
        "....booocccccccds.......",
        ".....bbooddddooob.......",
        "......bppgpppbb.........",
        "......pPPppPPp..........",
        "......pPPppPPp..........",
        "......pPp..pPp..........",
        "......pPp..pPp..........",
        ".....kggk..kggk.........",
        ".....kBBk..kBBk.........",
        "....kkkkk..kkkkk........"};
    DrawEllipse((int)pos.x,(int)pos.y+12*pixel,10*pixel,2*pixel,Fade(PAL_BG,.65f));
    int bob=(int)(sinf(time*3)*1.1f);
    for(int y=0;y<28;y++) for(int x=0;x<24;x++) {
        char c=sprite[y][x]; Color color;
        switch(c) {
            case 'h': color=(Color){38,27,27,255}; break;
            case 'H': color=(Color){94,59,38,255}; break;
            case 'f': color=(Color){246,188,130,255}; break;
            case 's': color=(Color){196,128,81,255}; break;
            case 'e': color=(Color){22,39,37,255}; break;
            case 'w': color=(Color){255,235,192,255}; break;
            case 'r': color=(Color){177,42,38,255}; break;
            case 'R': color=(Color){234,76,48,255}; break;
            case 'o': color=(Color){216,109,37,255}; break;
            case 'O': color=(Color){251,161,66,255}; break;
            case 'b': color=(Color){130,63,32,255}; break;
            case 't': color=(Color){66,42,37,255}; break;
            case 'g': color=(Color){168,121,62,255}; break;
            case 'p': color=(Color){44,37,44,255}; break;
            case 'P': color=(Color){86,61,62,255}; break;
            case 'k': color=(Color){20,23,28,255}; break;
            case 'B': color=(Color){85,47,32,255}; break;
            case 'd': color=(Color){29,87,87,255}; break;
            case 'c': color=(Color){82,235,213,255}; break;
            default: continue;
        }
        DrawRectangle((int)pos.x-12*pixel+x*pixel,(int)pos.y-16*pixel+y*pixel+bob,pixel,pixel,color);
    }
}

static void bot(Vector2 pos, float time, bool delivered)
{
    DrawEllipse((int)pos.x,(int)pos.y+20,22,5,Fade(PAL_BG,.7f));
    DrawRectangle((int)pos.x-23,(int)pos.y+7,10,14,PAL_BG);
    DrawRectangle((int)pos.x+13,(int)pos.y+7,10,14,PAL_BG);
    DrawRectangle((int)pos.x-21,(int)pos.y-14,42,30,(Color){92,119,146,255});
    DrawRectangle((int)pos.x-16,(int)pos.y-10,32,17,PAL_BG);
    DrawRectangle((int)pos.x-10,(int)pos.y-4,5,5,PAL_TEAL);
    DrawRectangle((int)pos.x+5,(int)pos.y-4,5,5,PAL_TEAL);
    DrawRectangle((int)pos.x-2,(int)pos.y-24,4,10,PAL_LINE);
    DrawCircle((int)pos.x,(int)pos.y-26,4,delivered?PAL_TEAL:PAL_GOLD);
    if(!delivered) {
        DrawRectangle((int)pos.x-12,(int)pos.y+9,24,12,PAL_GOLD);
        DrawCircle((int)pos.x,(int)pos.y+15,3,Fade(PAL_INK,.8f+.2f*sinf(time*4)));
    }
}

static void crate(Cell c)
{
    Vector2 p=centre(c);
    DrawRectangle((int)p.x-24,(int)p.y-19,48,45,(Color){62,47,37,255});
    DrawRectangle((int)p.x-24,(int)p.y-24,48,44,PAL_GOLD);
    DrawRectangle((int)p.x-19,(int)p.y-19,38,34,(Color){154,114,68,255});
    DrawLineEx((Vector2){p.x-17,p.y-17},(Vector2){p.x+17,p.y+13},4,PAL_GOLD);
    DrawLineEx((Vector2){p.x+17,p.y-17},(Vector2){p.x-17,p.y+13},4,PAL_GOLD);
}

static void portal(View *v, Cell cell, bool online, bool home)
{
    Vector2 p=centre(cell);
    Color color=online?PAL_TEAL:PAL_LINE;
    for(int i=3;i>=1;i--) DrawEllipse((int)p.x,(int)p.y,18+i*5,25+i*3,Fade(color,.035f));
    DrawEllipse((int)p.x,(int)p.y,21,29,(Color){7,15,27,255});
    DrawEllipseLines((int)p.x,(int)p.y,22,30,color);
    DrawEllipseLines((int)p.x,(int)p.y,18,26,Fade(color,.6f));
    if(online) {
        for(int i=0;i<5;i++) {
            float t=v->time*1.1f+i*1.2566f;
            DrawCircle((int)(p.x+17*cosf(t)),(int)(p.y+24*sinf(t)),2,PAL_GOLD);
        }
    }
    text(v,home?"HOME":"EXIT",p.x-22,p.y+37,14,online?PAL_TEAL:PAL_MUTED);
}

static void console(View *v, bool active)
{
    Vector2 p=centre((Cell){2,6});
    DrawRectangle((int)p.x-23,(int)p.y-25,46,34,(Color){67,87,109,255});
    DrawRectangle((int)p.x-18,(int)p.y-21,36,20,PAL_BG);
    DrawRectangle((int)p.x-13,(int)p.y-15,active?25:13,4,active?PAL_TEAL:PAL_BLUE);
    DrawRectangle((int)p.x-13,(int)p.y-8,18,3,Fade(PAL_TEAL,.5f));
    DrawRectangle((int)p.x-10,(int)p.y+9,20,12,PAL_LINE);
    text(v,active?"LINKED":"CONSOLE",p.x+28,p.y-14,11,PAL_GOLD);
}

static void floor_map(View *v, const Game *g)
{
    DrawRectangleRounded((Rectangle){BOARD_X-3,BOARD_Y-3,MAP_W*TILE+6,MAP_H*TILE+6},.025f,4,PAL_LINE);
    for(int y=0;y<MAP_H;y++) for(int x=0;x<MAP_W;x++) {
        int px=BOARD_X+x*TILE, py=BOARD_Y+y*TILE;
        char tile=game_tile(g->room,x,y);
        if(tile=='#') {
            DrawRectangle(px,py,TILE,TILE,(Color){18,29,46,255});
            DrawRectangle(px+3,py+4,TILE-6,TILE-8,(Color){26,39,57,255});
            DrawRectangle(px+5,py+5,TILE-10,3,(Color){43,59,77,255});
            if((x+y)%3==0) DrawRectangle(px+12,py+29,15,3,(Color){41,57,73,255});
        } else {
            DrawRectangle(px,py,TILE,TILE,PAL_FLOOR);
            DrawRectangle(px+1,py+1,TILE-2,TILE-2,((x+y)&1)?(Color){33,47,63,255}:(Color){36,50,67,255});
            DrawCircle(px+6,py+6,1,(Color){67,83,98,255});
            DrawCircle(px+TILE-7,py+TILE-7,1,(Color){67,83,98,255});
            if(tile=='=') {
                Vector2 p=centre((Cell){x,y});
                DrawRectangle(px+7,py+7,TILE-14,TILE-14,(Color){40,64,79,255});
                for(int d=0;d<4;d++) {
                    int dx=(d==1)-(d==3), dy=(d==2)-(d==0);
                    if(game_tile(g->room,x+dx,y+dy)=='=')
                        DrawLineEx(p,(Vector2){p.x+dx*TILE/2,p.y+dy*TILE/2},4,(Color){81,109,123,255});
                }
            }
        }
    }
    portal(v,game_exit(g->room),game_ready(g),g->room==2);
    if(g->room>0) console(v,g->interacting);
}

#include "duel_view.h"
#include "memory_view.h"

static void header(View *v, const Game *g)
{
    text(v,"THE LAST PORTAL",32,24,30,PAL_INK);
    text(v,"CAITLYN'S WAY HOME",34,66,14,PAL_MUTED);
    text(v,PORTAL_BUILD_LABEL,930,34,13,PAL_MUTED);
    for(int i=0;i<3;i++) {
        float x=36+i*282;
        Color color=i<g->room?PAL_TEAL:i==g->room?PAL_GOLD:PAL_LINE;
        DrawCircle((int)x+9,121,9,color);
        if(i<g->room) {
            DrawLineEx((Vector2){x+4,121},(Vector2){x+8,125},2,PAL_BG);
            DrawLineEx((Vector2){x+8,125},(Vector2){x+15,117},2,PAL_BG);
        }
        text(v,room_short[i],x+28,112,16,i==g->room?PAL_INK:PAL_MUTED);
        if(i<2) DrawLineEx((Vector2){x+228,121},(Vector2){x+255,121},1,PAL_LINE);
    }
}

static const char *next_action(const Game *g)
{
    if(g->room==0) {
        if(game_ready(g)) return "Walk Caitlyn into EXIT at the lower right.";
        if(g->crate.y==1) return "Press Z to undo the trapped push. Then push the crate onto the green plate.";
        return "Push the crate onto the green plate. H shows a route; Z undoes one action.";
    }
    if(g->room==1) {
        switch(g->courier.phase) {
            case COURIER_WAITING: return "Space reveals a RANDOM path for 30 seconds. Memorize the arrows from S to D.";
            case COURIER_REVEAL: return "Memorize the arrows. Space or Start planning now hides the path early; otherwise it stays for 30 seconds.";
            case COURIER_PLAN: return "Queue the ENTIRE remembered route. Arrows add, Backspace edits, C clears. Space launches.";
            case COURIER_RUN: return "Your whole queue is running. One wrong step triggers the laser; no edits or rewind.";
            case COURIER_CRASH: return "The bot was destroyed. Press R for a new random path and another 30-second preview.";
            case COURIER_DELIVERED: return "Perfect memory! Walk Caitlyn right into the glowing EXIT.";
        }
    }
    switch(g->duel.phase) {
        case DUEL_WAITING: return "Space starts the duel. Enemy openings enter a STACK; your response enters a QUEUE.";
        case DUEL_INCOMING: return "Watch the whole opening sequence arrive. Both sequences are read left to right.";
        case DUEL_BLOCK: return "Type the WHOLE response: 1 = ), 2 = ], 3 = }. Enter launches; C clears; Backspace edits.";
        case DUEL_COUNTER: return "Sequence matched! Each shot pops the enemy TOP and dequeues your FRONT.";
        case DUEL_GAP: return "Wave cleared. The alien is charging a longer nested pattern.";
        case DUEL_READY: return "All three sequences blocked. Press F for HOMEBOUND, the finishing move.";
        case DUEL_FINISH: return "HOMEBOUND! The charged beam breaks the alien's last defence.";
        case DUEL_CLEARED: return "Hold Right / D to walk Caitlyn through the cleared alley into HOME.";
    }
    return "F1 explains this encounter.";
}

static UiAction side_panel(View *v, const Game *g)
{
    UiAction action={UI_NONE,0};
    float x=832, y=160, w=416;
    DrawRectangleRounded((Rectangle){x,y,w,512},.035f,6,PAL_PANEL);
    text(v,TextFormat("0%d / 03",g->room+1),x+24,y+22,15,PAL_GOLD);
    text(v,room_titles[g->room],x+24,y+52,23,PAL_INK);
    text(v,"NEXT ACTION",x+24,y+85,13,PAL_TEAL);
    wrapped(v,next_action(g),x+24,y+108,w-48,17,PAL_INK);
    float section=y+191;
    DrawLine((int)x+24,(int)section-13,(int)(x+w-24),(int)section-13,PAL_LINE);
    if(g->room==0) {
        text(v,"MOVEMENT MEMORY",x+24,section,14,PAL_TEAL);
        mono(v,TextFormat("%d snapshot%s saved",g->history.count,g->history.count==1?"":"s"),x+24,section+32,23,PAL_INK);
        text(v,TextFormat("CAPACITY: %d complete snapshots",HISTORY_CAPACITY),x+24,section+72,16,PAL_GOLD);
        wrapped(v,"Each entry saves the WHOLE board before an action. Z restores the newest entry. The stack grows until its actual capacity is reached.",x+24,section+103,w-48,16,PAL_MUTED);
        if(button(v,(Rectangle){x+24,section+190,w-48,44},"Z   Rewind one step",g->history.count>0,true)) action.kind=UI_UNDO;
        if(button(v,(Rectangle){x+24,section+247,w-48,42},"R   Restart this room",true,false)) action.kind=UI_RESET;
        text(v,game_ready(g)?"Door powered. Walk into the light.":"The crate can be pushed, never pulled.",x+24,section+299,14,game_ready(g)?PAL_TEAL:PAL_GOLD);
    } else if(g->room==1) {
        const CourierState *c=&g->courier;
        text(v,"YOUR FULL ROUTE / FRONT TO REAR",x+24,section,14,PAL_TEAL);
        for(int i=0;i<c->length;i++) {
            Rectangle slot={x+24+(i%7)*53,section+28+(i/7)*48,47,43};
            bool used=i<g->queue.count;
            DrawRectangleRounded(slot,.1f,4,used?(Color){47,77,87,255}:(Color){29,43,60,255});
            if(used) {
                Command cmd=g->queue.items[(g->queue.front+i)%COMMAND_CAPACITY];
                Vector2 p={slot.x+27,slot.y+23};
                int dx=(cmd==CMD_RIGHT)-(cmd==CMD_LEFT),dy=(cmd==CMD_DOWN)-(cmd==CMD_UP);
                arrow((Vector2){p.x-dx*9,p.y-dy*9},(Vector2){p.x+dx*9,p.y+dy*9},PAL_TEAL,2);
            }
            mono(v,TextFormat("%d",i+1),slot.x+3,slot.y+3,10,PAL_MUTED);
        }
        if(c->phase==COURIER_RUN||c->phase==COURIER_CRASH)
            mono(v,TextFormat("%d left / %d of %d safe",g->queue.count,c->step,c->length),x+24,section+130,15,PAL_GOLD);
        else if(c->phase==COURIER_DELIVERED) mono(v,TextFormat("PERFECT: %d / %d correct",c->step,c->length),x+24,section+130,15,PAL_TEAL);
        else mono(v,TextFormat("%d / %d planned   buffer: %d",g->queue.count,c->length,COMMAND_CAPACITY),x+24,section+130,15,PAL_GOLD);
        bool usable=c->phase==COURIER_PLAN;
        const char *directions[]={"Up","Right","Down","Left"};
        for(int i=0;i<4;i++) if(button(v,(Rectangle){x+24+i*94,section+163,82,34},directions[i],usable,false)) action=(UiAction){UI_COMMAND,i};
        bool retry=c->phase==COURIER_CRASH;
        if(button(v,(Rectangle){x+24,section+210,w-48,40},retry?"R   New random route":c->phase==COURIER_WAITING?"Space   Reveal for 30 seconds":c->phase==COURIER_REVEAL?"Space   Start planning now":"Space   Launch whole route",retry||c->phase==COURIER_WAITING||c->phase==COURIER_REVEAL||(usable&&g->queue.count==c->length),true)) action.kind=retry?UI_RESET:UI_RUN;
        if(button(v,(Rectangle){x+24,section+263,136,36},"C   Clear",usable&&g->queue.count>0,false)) action.kind=UI_CLEAR;
        if(button(v,(Rectangle){x+172,section+263,196,36},"Backspace   Edit",usable&&g->queue.count>0,false)) action.kind=UI_BACKSPACE;

    } else {
        const DuelState *d=&g->duel;
        text(v,"YOUR COUNTER QUEUE / LEFT TO RIGHT",x+24,section,14,PAL_TEAL);
        for(int i=0;i<DUEL_SEQUENCE_MAX;i++) {
            Rectangle slot={x+24+i*94,section+29,82,51};
            bool occupied=i<g->counter_queue.count;
            DrawRectangleRounded(slot,.1f,4,occupied?(Color){47,77,87,255}:(Color){29,43,60,255});
            if(occupied) {
                char c=counter_symbol(g->counter_queue.items[(g->counter_queue.front+i)%COMMAND_CAPACITY]);
                bracket_sprite(c,(Vector2){slot.x+41,slot.y+22},3,attack_color(c));
            } else text(v,"_",slot.x+33,slot.y+10,21,PAL_LINE);
            if(occupied) text(v,i==0?"FRONT":i==g->counter_queue.count-1?"REAR":"",slot.x+6,slot.y+38,10,PAL_GOLD);
        }
        const char *labels[]={"1    )", "2    ]", "3    }"};
        const char closing[]={')',']','}'};
        for(int i=0;i<3;i++) if(button(v,(Rectangle){x+24+i*124,section+94,112,37},labels[i],d->phase==DUEL_BLOCK,false)) action=(UiAction){UI_BLOCK,closing[i]};
        bool waiting=d->phase==DUEL_WAITING;
        if(button(v,(Rectangle){x+24,section+147,w-48,40},waiting?"Space   Begin duel":"Enter   Launch entire sequence",waiting||d->phase==DUEL_BLOCK,true)) action.kind=waiting?UI_DUEL_START:UI_SUBMIT;
        if(button(v,(Rectangle){x+24,section+201,136,38},"C   Clear",d->phase==DUEL_BLOCK,false)) action.kind=UI_COUNTER_CLEAR;
        if(button(v,(Rectangle){x+172,section+201,196,38},"Backspace   Edit",d->phase==DUEL_BLOCK,false)) action.kind=UI_COUNTER_BACKSPACE;
        if(button(v,(Rectangle){x+24,section+255,w-48,40},"F   HOMEBOUND finishing move",d->phase==DUEL_READY,true)) action.kind=UI_FINISH;

    }
    return action;
}

static void backdrop(View *v)
{
    ClearBackground(PAL_BG);
    for(int i=0;i<60;i++) {
        int x=(i*197+71)%VIEW_W, y=(i*83+31)%VIEW_H;
        float alpha=.12f+.1f*sinf(v->time*.8f+i);
        DrawRectangle(x,y,i%7==0?2:1,i%7==0?2:1,Fade(PAL_INK,alpha));
    }
}

static UiAction story_screen(View *v)
{
    const RoomStory *s=&room_stories[v->story_room];
    Texture2D picture=v->story_images[v->story_room];
    if(picture.id) {
        float scale=fmaxf((float)VIEW_W/picture.width,(float)VIEW_H/picture.height);
        float width=VIEW_W/scale, height=VIEW_H/scale;
        DrawTexturePro(picture,(Rectangle){(picture.width-width)/2,(picture.height-height)/2,width,height},
            (Rectangle){0,0,VIEW_W,VIEW_H},(Vector2){0,0},0,WHITE);
    }
    DrawRectangle(0,0,VIEW_W,VIEW_H,Fade(PAL_BG,.16f));
    DrawRectangleGradientH(0,0,820,VIEW_H,Fade(PAL_BG,.97f),Fade(PAL_BG,.12f));
    for(int i=0;i<54;i++) {
        float alpha=.2f+.14f*sinf(v->time*.6f+i);
        DrawCircle(35+(i*137)%690,16+(i*83)%555,i%9==0?1.3f:.7f,Fade(PAL_INK,alpha));
    }
    text(v,s->chapter,48,32,15,PAL_TEAL);
    text(v,s->title,48,67,29,PAL_GOLD);
    text(v,"CAITLYN'S WAY HOME",48,113,14,PAL_MUTED);

    /* Project scanlines of a transparent text canvas onto a receding plane.
     * The reciprocal mapping compresses height as well as width, rather than
     * merely shrinking an otherwise upright font. Render textures are flipped. */
    const float top=172, bottom=568, focal=520;
    float scroll=60+v->story_elapsed*30;
    BeginScissorMode(40,(int)top,714,(int)(bottom-top));
    if(v->story_text.id) {
        for(int row=(int)top;row<(int)bottom;row++) {
            float scale=.25f+.75f*(row-top)/(bottom-top);
            float next=.25f+.75f*(row+1-top)/(bottom-top);
            float sy=scroll-focal*(1/scale-1);
            float span=focal*(1/scale-1/next);
            if(sy<0||sy+span>=v->story_height) continue;
            float alpha=fminf(1,(row-top)/60)*fminf(1,(bottom-row)/16);
            alpha*=fminf(1,(v->story_duration-v->story_elapsed)/4);
            DrawTexturePro(v->story_text.texture,
                (Rectangle){0,v->story_text.texture.height-sy-span,768,-span},
                (Rectangle){397-384*scale,(float)row,768*scale,1},
                (Vector2){0,0},0,Fade(WHITE,alpha));
        }
    } else wrapped(v,s->narrative,52,180,680,22,PAL_GOLD);
    EndScissorMode();
    float progress=fminf(1,v->story_elapsed/v->story_duration);
    DrawRectangle(48,574,704,2,Fade(PAL_GOLD,.2f));
    DrawRectangle(48,574,(int)(704*progress),2,PAL_GOLD);

    DrawRectangleRounded((Rectangle){32,592,1216,132},.045f,6,Fade(PAL_BG,.96f));
    text(v,"YOUR MISSION",52,606,13,PAL_TEAL);
    wrapped(v,PORTAL_PUZZLE?s->mission:s->reference_mission,52,632,1172,20,PAL_INK);
    text(v,s->data,52,698,15,PAL_GOLD);
    UiAction action={UI_NONE,0};
    if(button(v,(Rectangle){48,744,300,44},"Enter   Begin / return to room",true,true)) action.kind=UI_STORY_BEGIN;
    if(button(v,(Rectangle){368,744,180,44},v->story_paused?"Space   Resume":"Space   Pause",progress<1,false)) action.kind=UI_STORY_PAUSE;
    if(button(v,(Rectangle){568,744,180,44},"R   Replay",true,false)) action.kind=UI_STORY_RESTART;
    if(button(v,(Rectangle){768,744,180,44},"F1   Room guide",true,false)) action.kind=UI_GUIDE;
    if(button(v,(Rectangle){968,744,264,44},"Q   Close game",true,false)) action.kind=UI_QUIT;
    return action;
}

static UiAction title_screen(View *v, int checkpoint)
{
    UiAction action={UI_NONE,0};
    text(v,"A SMALL JOURNEY ABOUT GETTING THINGS WORKING",65,70,14,PAL_TEAL);
    text(v,"THE LAST",60,132,66,PAL_INK);
    text(v,"PORTAL",60,202,86,PAL_GOLD);
    wrapped(v,"Three challenges. One way home.\nCaitlyn needs a memory, a courier, and the courage to face the last guardian.",65,330,620,23,PAL_MUTED);
    if(button(v,(Rectangle){65,454,290,55},checkpoint>0?"Continue the journey":"Begin the journey",true,true)) action.kind=UI_CONTINUE;
    if(checkpoint>0&&button(v,(Rectangle){375,454,210,55},"Start from room one",true,false)) action.kind=UI_NEW;
    key(v,"WASD",65,551,"move"); key(v,"Z",276,551,"undo"); key(v,"E",418,551,"interact");
    text(v,"F1: what to do, what the data stores, and how to repair it.",65,614,18,PAL_INK);
    if(PORTAL_PUZZLE) text(v,"Edit the named C function, rebuild, then play the repaired room.",65,654,17,PAL_MUTED);
    text(v,PORTAL_TITLE_NOTE,65,741,14,PAL_MUTED);
    text(v,"Enter to begin   |   M sound   |   Q quit",65,773,14,PAL_MUTED);
    Vector2 centrepoint={970,394};
    for(int i=5;i>0;i--) DrawEllipse(970,394,85+i*15,145+i*14,Fade(PAL_TEAL,.022f));
    DrawEllipse(970,394,89,158,(Color){5,14,27,255});
    DrawEllipseLines(970,394,91,160,PAL_TEAL);
    DrawEllipseLines(970,394,83,151,Fade(PAL_TEAL,.3f));
    for(int i=0;i<10;i++) {
        float t=v->time*.3f+i*.628f;
        DrawCircle((int)(centrepoint.x+90*cosf(t)),(int)(centrepoint.y+158*sinf(t)),3,PAL_GOLD);
    }
    DrawRectangle(810,558,330,16,PAL_LINE); DrawRectangle(790,574,370,22,PAL_PANEL);
    hero((Vector2){872,534},v->time,3);
    bot((Vector2){1060,545},v->time,true);
    return action;
}

static UiAction overlay(View *v, const Game *g, bool pause)
{
    UiAction action={UI_NONE,0};
    DrawRectangle(0,0,VIEW_W,VIEW_H,Fade(PAL_BG,.86f));
    Rectangle panel={270,205,740,415};
    DrawRectangleRounded(panel,.035f,6,PAL_PANEL);
    text(v,pause?"TAKE A BREATH":g->won?"HOME, AT LAST.":"THE STATION REMEMBERS.",310,245,32,pause?PAL_INK:PAL_GOLD);
    const char *body=pause?"Your room is waiting. The checkpoint is saved when you reach a new room.":
        g->won?"A mistake rewound. A courier guided. Every attack answered.\nYou defeated the guardian and brought Caitlyn Home.":
        g->room==0?"You gave a mistake a way back. Next, memorize a route before its lights go out. The courier gets one chance per attempt.":
        "A few commands carried a power cell across the station. Now an alien guards the last alley Home.";
    wrapped(v,body,310,310,650,23,PAL_INK);
    if(button(v,(Rectangle){310,473,310,53},pause?"Resume":g->won?"Play again":"Continue to the next room",true,true)) action.kind=pause?UI_RESUME:g->won?UI_NEW:UI_NEXT;
    if(button(v,(Rectangle){640,473,325,53},pause?"Restart this room":"Close the game",true,false)) action.kind=pause?UI_RESET:UI_QUIT;
    text(v,pause?"Esc to resume   |   Q quit":"Enter to continue",310,565,17,PAL_MUTED);
    return action;
}

static UiAction mission_guide(View *v, const Game *g)
{
    DrawRectangle(0,0,VIEW_W,VIEW_H,Fade(PAL_BG,.92f));
    DrawRectangleRounded((Rectangle){70,60,1140,700},.025f,6,PAL_PANEL);
    text(v,TextFormat("ROOM %d / 3   |   WHAT TO DO",g->room+1),105,94,15,PAL_TEAL);
    text(v,room_titles[g->room],105,130,32,PAL_INK);
    const char *goals[]={"Goal: crate on the green plate, then Caitlyn through EXIT.",
        "Goal: memorize a random path, queue it completely, get the bot safely to D, then exit.",
        "Goal: counter three sequences (nine pairs), finish with F, then walk right into HOME."};
    wrapped(v,goals[g->room],105,184,1060,20,PAL_GOLD);
    text(v,"PLAY THIS ROOM",105,236,14,PAL_TEAL);
    const char *steps[3][6]={
        {"1. Press Z to undo the last push. The crate is trapped against the upper wall; it cannot be pulled.",
         "2. Use WASD / arrows to walk beside the crate and push it onto the green pressure plate.",
         "3. H shows a route if you get stuck. Z undoes one action; R restarts the room.",
         "4. Once the plate is down, walk into EXIT at the lower right. Enter continues to the next room.",NULL,NULL},
        {"1. Space reveals the random route for 30 seconds. Memorize S to D; click Start planning now (or Space/Enter) to hide it early.",
         "2. When it disappears, type ALL 10-14 arrows from memory. The buffer holds the entire plan. Your queued arrows stay visible.",
         "3. Backspace removes the last planned arrow; C clears the plan. Space (or Enter) launches only when the entire route is entered.",
         "4. The bot dequeues one arrow at a time. One wrong step fires the security laser and destroys it. No single-step probing or rewind.",
         "5. R creates a fresh random path for a new attempt. H gives strategy, never the answer. F1 / Esc pause the countdown and cover the path.",
         "6. A perfect run delivers the power cell and releases Caitlyn. Walk right into EXIT to reach the final duel."},
        {"1. Space starts the duel. Read the alien's WHOLE opening sequence left to right. Each opening is pushed onto its stack.",
         "2. Type the WHOLE closing sequence left to right: 1 = ), 2 = ], 3 = }. Your entries join a queue. C clears; Backspace edits.",
         "3. Example: enemy ([ needs ]). Enemy TOP [ meets your FRONT ]; then ( meets ). Enter launches the response.",
         "4. A match sends return shots automatically. A mismatch leaves both structures intact. There is no reaction timer.",
         "5. Clear all three sequences (2 + 3 + 4 pairs), then F fires HOMEBOUND. Repair the shield and charge logic when prompted.",
         "6. After the beam defeats the alien, hold Right / D to follow the only path into HOME."}

    };
    float y=268;
    for(int i=0;i<6&&steps[g->room][i];i++) y=wrapped(v,steps[g->room][i],105,y,620,18,PAL_INK)+13;
    DrawLine(764,236,764,663,PAL_LINE);
    text(v,"WHAT THE CODE STORES",795,236,14,PAL_TEAL);
    const char *data[]={
        "STACK: each entry is a complete BoardState from BEFORE an action: positions, both queues, courier state, attack IDs/brackets, delivery flag and duel state. Capacity is 128 snapshots. Push saves a value copy. Pop gives the newest saved state to the engine, which restores the board.",
        "QUEUE: one direction Command per entry. Enqueue stores the full remembered plan at REAR. During the run, dequeue retrieves FRONT and moves the bot. COMMAND_CAPACITY is 64; use that constant, never a four-slot limit. Empty is -1/-1 with count 0.",
        "ENEMY STACK: linked AttackNode values (id, opening, next), pushed at the head. YOUR QUEUE: closing Commands, read from the front. shield_accepts compares top to front without changing either. The finisher uses the second-highest DISTINCT charge."
    };
    y=wrapped(v,data[g->room],795,268,375,17,PAL_INK);
    if(PORTAL_PUZZLE) {
        text(v,"WHEN A REPAIR SCREEN APPEARS",795,y+25,13,PAL_GOLD);
        y=wrapped(v,"Read the data and behavior on that screen. repair_required(...) is only a supplied marker for an unfinished function. Replace the starter body, including that call; do not implement the marker itself.",795,y+52,375,16,PAL_MUTED);
        wrapped(v,"Source edits take effect only after you close, save, rebuild and relaunch. Reopening the old executable alone will not apply them.",795,y+16,375,16,PAL_MUTED);
    }
    UiAction action={UI_NONE,0};
    if(button(v,(Rectangle){105,693,320,44},"F1 / Esc   Back to game",true,true)) action.kind=UI_GUIDE;
    text(v,"The guide pauses play. F2 replays the story; H gives a route or coding clue.",453,705,15,PAL_MUTED);
    return action;
}

static UiAction repair_overlay(View *v)
{
    const RepairReport *r=repair_report();
    DrawRectangle(0,0,VIEW_W,VIEW_H,Fade(PAL_BG,.88f));
    DrawRectangleRounded((Rectangle){65,34,1150,762},.025f,6,PAL_PANEL);
    text(v,r->diagnosis?TextFormat("DIAGNOSIS %d / 2    |    %s",r->task-REPAIR_SHIELD_LOGIC+1,r->module):
        TextFormat("FOUNDATION REPAIR %d / 6    |    %s",r->task+1,r->module),100,65,15,PAL_TEAL);
    text(v,r->diagnosis?r->title:r->function,100,98,30,PAL_INK);
    mono(v,r->file,100,144,18,PAL_GOLD);
    DrawLine(772,185,772,628,PAL_LINE);
    text(v,"WHY THIS STOPS THE GAME",100,185,13,PAL_TEAL);
    float y=wrapped(v,r->purpose,100,210,635,18,PAL_INK);
    text(v,"THE DATA YOU RECEIVE",100,y+10,13,PAL_TEAL);
    y=wrapped(v,r->data,100,y+34,635,17,PAL_MUTED);
    text(v,r->diagnosis?"REQUIRED OPERATING RULE":"REQUIRED FUNCTION BEHAVIOR",100,y+10,13,PAL_TEAL);
    y=wrapped(v,r->contract,100,y+34,635,18,PAL_INK);
    wrapped(v,r->example,100,y+12,635,17,PAL_GOLD);
    if(r->diagnosis) {
        text(v,"DIAGNOSE EXISTING CODE",802,185,13,PAL_TEAL);
        y=wrapped(v,"These two routines already run. Locate the faulty logic in this file using the rule on the left. Choose the algorithm yourself.",802,212,375,17,PAL_INK);
        text(v,r->show_clue?"OPTIONAL CODING CLUE":"EXTRA CLUE AVAILABLE",802,y+22,13,PAL_GOLD);
        y=wrapped(v,r->show_clue?r->clue:"Press H for a trace to help your reasoning. All required behavior is already stated on the left.",802,y+47,375,17,PAL_MUTED);
        text(v,"APPLY THE REPAIR",802,y+18,13,PAL_TEAL);
        wrapped(v,"Close, edit and save the C file. Rebuild, then restart the duel.",802,y+42,375,17,PAL_INK);
    } else {
        text(v,"WHAT IS repair_required(...)?",802,185,13,PAL_TEAL);
        y=wrapped(v,"A supplied engine helper that opens this screen. It marks a missing function. Do not implement the helper or change src/repair.c. Replace the starter body, including the helper call.",802,212,375,17,PAL_INK);
        y=wrapped(v,"(void)x only silences an unused-parameter warning. Remove it when you use x.",802,y+16,375,17,PAL_MUTED);
        y=wrapped(v,"Return true on success. Return false for the rejected cases in the contract. These functions return bool, not a pointer or NULL.",802,y+16,375,17,PAL_MUTED);
        text(v,"APPLY THE REPAIR",802,y+20,13,PAL_TEAL);
        wrapped(v,"Close game. Edit the named function. Keep its signature. Save, rebuild and run again.",802,y+44,375,17,PAL_INK);
    }
    text(v,"REBUILD FROM THE EXTRACTED FOLDER",802,553,12,PAL_TEAL);
    mono(v,"Linux: ./run-linux.sh puzzle",802,580,14,PAL_INK);
    mono(v,"Windows: run-windows.bat puzzle",802,604,14,PAL_INK);
    DrawLine(100,641,1180,641,PAL_LINE);
    text(v,r->diagnosis?"OBSERVED:":r->missing?"STATUS:":"CHECK:",100,657,13,PAL_GOLD);
    wrapped(v,r->detail,195,654,985,17,PAL_GOLD);
    text(v,"Fix compiler errors first. Rebuilding checks your real code; removing the marker alone does not fix the operation.",100,709,15,PAL_MUTED);
    UiAction action={UI_NONE,0};
    if(button(v,(Rectangle){100,742,312,40},"Close game to edit source",true,true)) action.kind=UI_QUIT;
    if(button(v,(Rectangle){432,742,312,40},"F1   What do I do in this room?",true,false)) action.kind=UI_GUIDE;
    if(r->diagnosis) {
        if(button(v,(Rectangle){764,742,414,40},r->show_clue?"H   Hide extra coding clue":"H   Extra coding clue",true,false)) action.kind=UI_DIAGNOSIS_HINT;
    } else text(v,"Q / Esc also closes this build",802,754,16,PAL_MUTED);
    return action;
}

void view_init(View *v, bool audio)
{
    memset(v,0,sizeof(*v)); v->visual_room=-1;
    v->font=GetFontDefault(); v->mono=GetFontDefault();
    if(FileExists("assets/DejaVuSans.ttf")) {
        v->font=LoadFontEx("assets/DejaVuSans.ttf",64,NULL,0); v->custom_font=true;
        SetTextureFilter(v->font.texture,TEXTURE_FILTER_BILINEAR);
    }
    if(FileExists("assets/DejaVuSansMono.ttf")) {
        v->mono=LoadFontEx("assets/DejaVuSansMono.ttf",32,NULL,0); v->custom_mono=true;
        SetTextureFilter(v->mono.texture,TEXTURE_FILTER_BILINEAR);
    }
    if(audio) {
        InitAudioDevice(); v->audio=IsAudioDeviceReady();
        if(v->audio) {
            const char *names[]={"step","undo","bump","connect","success"};
            for(int i=0;i<5;i++) {
                v->sounds[i]=LoadSound(TextFormat("assets/%s.wav",names[i]));
                SetSoundVolume(v->sounds[i],i==0?.16f:.32f);
            }
        }
    }
    for(int room=0;room<3;room++) if(FileExists(room_stories[room].image)) {
        v->story_images[room]=LoadTexture(room_stories[room].image);
        if(v->story_images[room].id) SetTextureFilter(v->story_images[room],TEXTURE_FILTER_BILINEAR);
    }
    v->story_text=LoadRenderTexture(768,1536);
    if(v->story_text.id) SetTextureFilter(v->story_text.texture,TEXTURE_FILTER_BILINEAR);
}

void view_destroy(View *v)
{
    if(v->audio) { for(int i=0;i<5;i++) UnloadSound(v->sounds[i]); CloseAudioDevice(); }
    if(v->custom_font) UnloadFont(v->font);
    if(v->custom_mono) UnloadFont(v->mono);
    for(int room=0;room<3;room++) if(v->story_images[room].id) UnloadTexture(v->story_images[room]);
    if(v->story_text.id) UnloadRenderTexture(v->story_text);
}

void view_update(View *v, const Game *g, Screen screen, float dt)
{
    v->time+=dt;
    if(screen==SCREEN_STORY&&!v->story_paused&&!v->guide)
        v->story_elapsed=fminf(v->story_duration,v->story_elapsed+dt);
    Vector2 hp=centre(g->hero),bp=centre(g->bot);
    if(v->visual_room!=g->room) {
        v->hero_pos=hp; v->bot_pos=bp; v->visual_room=g->room;
    }
    float blend=fminf(1,dt*18);
    v->hero_pos.x+=(hp.x-v->hero_pos.x)*blend; v->hero_pos.y+=(hp.y-v->hero_pos.y)*blend;
    v->bot_pos.x+=(bp.x-v->bot_pos.x)*blend; v->bot_pos.y+=(bp.y-v->bot_pos.y)*blend;
    if(v->seen_event!=g->event_serial) {
        v->seen_event=g->event_serial;
        int sound=g->event==EVENT_UNDO?1:g->event==EVENT_BUMP||g->event==EVENT_CRASH?2:
            g->event==EVENT_POWER||g->event==EVENT_COMPLETE||g->event==EVENT_FINISH?4:g->event==EVENT_BLOCK?3:0;
        if(g->event!=EVENT_NONE&&v->audio&&!v->muted) PlaySound(v->sounds[sound]);
        if(g->event==EVENT_POWER||g->event==EVENT_COMPLETE) {
            Vector2 origin=centre(game_exit(g->room));
            for(int i=0;i<72;i++) {
                float angle=i*2.399963f;
                float speed=40+(i*31)%150;
                v->particles[i]=(Particle){origin,{cosf(angle)*speed,sinf(angle)*speed},1.2f+(i%5)*.1f,i&1?PAL_TEAL:PAL_GOLD};
            }
        }
    }
    for(int i=0;i<72;i++) if(v->particles[i].life>0) {
        Particle *p=&v->particles[i]; p->life-=dt;
        p->position.x+=p->velocity.x*dt; p->position.y+=p->velocity.y*dt;
        p->velocity.y+=30*dt;
    }
}

UiAction view_draw(View *v, const Game *g, Screen screen, int checkpoint)
{
    backdrop(v);
    if(screen==SCREEN_STORY) return v->guide?mission_guide(v,g):story_screen(v);
    if(screen==SCREEN_TITLE) return v->guide?mission_guide(v,g):title_screen(v,checkpoint);
    header(v,g); if(g->room<2) floor_map(v,g); else duel_arena(v,g);
    UiAction action={UI_NONE,0};
    if(button(v,(Rectangle){730,25,168,34},"F2   Room story",!g->room_complete,false)) action.kind=UI_STORY;
    if(g->room==0) {
        Vector2 plate=centre((Cell){5,4});
        DrawRectangle((int)plate.x-23,(int)plate.y-18,46,36,Fade(PAL_TEAL,game_ready(g)?.7f:.25f));
        DrawRectangleLinesEx((Rectangle){plate.x-23,plate.y-18,46,36},2,PAL_TEAL);
        DrawCircle((int)plate.x,(int)plate.y,6,PAL_TEAL);
        crate(g->crate);
        if(g->crate.y==1) text(v,"STUCK",centre(g->crate).x-23,BOARD_Y+119,14,PAL_GOLD);
    } else if(g->room==1) {
        memory_arena(v,g,screen==SCREEN_PLAY&&!v->guide&&!repair_is_blocked());
    }
    Vector2 hp=v->hero_pos;
    if(g->room>0&&g->hero.x==2&&g->hero.y==6) hp.x-=36;
    if(g->room<2) hero(hp,v->time,2);
    for(int i=0;i<72;i++) if(v->particles[i].life>0)
        DrawRectangle((int)v->particles[i].position.x,(int)v->particles[i].position.y,4,4,Fade(v->particles[i].color,fminf(1,v->particles[i].life)));
    UiAction side=side_panel(v,g); if(side.kind!=UI_NONE) action=side;
    if(g->room==2) { key(v,"1 2 3",34,699,"type"); key(v,"Enter / F",235,699,"launch / finish"); }
    else if(g->room==1) { key(v,"Arrows",34,699,"plan"); key(v,"Space",235,699,"reveal / launch"); }
    else { key(v,"WASD",34,699,"move"); key(v,"E",270,699,"interact"); }
    if(g->room==1) key(v,"Bksp",464,699,"edit");
    else key(v,"Z",464,699,"rewind");
    key(v,"R",633,699,"restart");
    if(button(v,(Rectangle){832,695,200,35},g->hint?"H   Hide hint":"H   Need a hint?",true,false)) action.kind=UI_HINT;
    if(button(v,(Rectangle){1047,695,201,35},"F1   What to do",true,false)) action.kind=UI_GUIDE;
    if(button(v,(Rectangle){930,100,152,33},!v->audio?"Sound off":v->muted?"M   Sound off":"M   Sound on",v->audio,false)) action.kind=UI_MUTE;
    if(button(v,(Rectangle){1100,100,148,33},"Esc   Pause",true,false)) action.kind=UI_PAUSE;
    DrawLine(32,746,1248,746,PAL_LINE);
    wrapped(v,g->message,36,764,1205,19,g->event==EVENT_CRASH?PAL_RED:g->event==EVENT_BUMP?PAL_GOLD:PAL_INK);
    if(g->hint&&!g->room_complete) {
        const char *hints[]={"Undo first. Walk above the crate, push it down twice, then stand to its right and push it left onto the plate.",
            "Memorize the turns in groups during the preview. S is the start; D is the dock. Enter every arrow before launching. H never reveals the hidden route. R gives a fresh path.",
            "Read both sequences left to right. Enemy ([ needs counter ]): keys 2 then 1, then Enter. Enemy TOP meets your FRONT. Three sequences unlock F."};
        DrawRectangleRounded((Rectangle){75,538,680,113},.07f,4,(Color){17,30,45,255});
        text(v,"A SMALL NUDGE",96,554,13,PAL_GOLD);
        wrapped(v,hints[g->room],96,579,638,17,PAL_INK);
    }
    if(v->guide) return mission_guide(v,g);
    if(repair_is_blocked()) return repair_overlay(v);
    if(screen==SCREEN_PAUSE||g->room_complete) return overlay(v,g,screen==SCREEN_PAUSE);
    return action;
}
