/* Memory-run field: only the timed reveal draws the safe path. */
static void memory_arena(View *v, const Game *g, bool active)
{
    BeginScissorMode(BOARD_X,BOARD_Y,MAP_W*TILE,MAP_H*TILE);
    const CourierState *c=&g->courier;
    bool reveal=active&&c->phase==COURIER_REVEAL;
    for(int y=1;y<=5;y++) for(int x=1;x<=10;x++) {
        int px=BOARD_X+x*TILE, py=BOARD_Y+y*TILE;
        DrawRectangle(px+3,py+3,TILE-6,TILE-6,(Color){30,42,58,255});
        DrawRectangleLines(px+6,py+6,TILE-12,TILE-12,(Color){43,57,73,255});
        DrawRectangle(px+9,py+10,4,4,(Color){62,72,86,255});
    }
    if(reveal) for(int i=0;i<=c->length;i++) {
        Vector2 p=centre(c->path[i]);
        DrawRectangle((int)p.x-28,(int)p.y-28,56,56,(Color){40,95,94,255});
        DrawRectangleLinesEx((Rectangle){p.x-27,p.y-27,54,54},2,PAL_TEAL);
        if(i<c->length) {
            Cell next=c->path[i+1];
            int dx=next.x-c->path[i].x,dy=next.y-c->path[i].y;
            arrow((Vector2){p.x-dx*8,p.y-dy*8},(Vector2){p.x+dx*17,p.y+dy*17},PAL_TEAL,3);
            if(i==0) arrow((Vector2){p.x+dx*24,p.y+dy*24},(Vector2){p.x+dx*48,p.y+dy*48},PAL_GOLD,3);
            mono(v,TextFormat("%d",i+1),p.x-23,p.y-24,12,PAL_GOLD);
        }
    }
    Vector2 start=centre(c->path[0]), dock=centre(c->path[c->length]);
    DrawRectangleLinesEx((Rectangle){start.x-29,start.y-29,58,58},2,PAL_BLUE);
    DrawRectangleLinesEx((Rectangle){dock.x-29,dock.y-29,58,58},3,g->delivered?PAL_TEAL:PAL_GOLD);
    text(v,"D",dock.x+17,dock.y-25,13,PAL_GOLD);
    text(v,"S",start.x-3,start.y+27,10,PAL_BLUE);
    for(int x=1;x<=10;x++) {
        Vector2 p=centre((Cell){x,0});
        DrawRectangle((int)p.x-6,BOARD_Y+46,12,10,PAL_LINE);
        DrawRectangle((int)p.x-2,BOARD_Y+53,4,8,c->phase==COURIER_CRASH?PAL_RED:PAL_MUTED);
    }
    DrawRectangle(BOARD_X+16,BOARD_Y+9,736,30,PAL_BG);
    const char *status=c->phase==COURIER_WAITING?"SPACE: REVEAL THE ROUTE":
        c->phase==COURIER_REVEAL?TextFormat("MEMORIZE: %.1f seconds",fmaxf(0,MEMORY_REVEAL_SECONDS-c->clock)):
        c->phase==COURIER_PLAN?"ROUTE HIDDEN: PLAN EVERY MOVE":
        c->phase==COURIER_RUN?"BOT RUNNING: NO SECOND CHANCES":
        c->phase==COURIER_CRASH?"BOT DESTROYED  /  R: NEW ATTEMPT":"PERFECT RUN: CARGO DELIVERED";
    text(v,status,BOARD_X+27,BOARD_Y+15,16,c->phase==COURIER_CRASH?PAL_RED:PAL_GOLD);
    text(v,TextFormat("%d moves  /  attempt %d",c->length,c->attempt),BOARD_X+512,BOARD_Y+16,14,PAL_MUTED);
    Vector2 p=v->bot_pos;
    if(c->phase==COURIER_CRASH) {
        float t=c->clock;
        if(t<.55f) {
            DrawLineEx((Vector2){p.x,BOARD_Y+60},p,9,Fade(PAL_RED,.7f));
            DrawLineEx((Vector2){p.x,BOARD_Y+60},p,3,PAL_INK);
            for(int i=0;i<22;i++) {
                float angle=i*2.39996f,radius=t*(70+i*7);
                DrawRectangle((int)(p.x+cosf(angle)*radius),(int)(p.y+sinf(angle)*radius),4+i%4,4+i%4,i%3?PAL_GOLD:PAL_RED);
            }
            DrawCircle((int)p.x,(int)p.y,(int)(14+25*t),Fade(PAL_GOLD,1-t));
        } else {
            DrawRectangle((int)p.x-19,(int)p.y+2,32,13,(Color){49,44,52,255});
            DrawRectangle((int)p.x+9,(int)p.y-4,12,14,PAL_LINE);
            DrawRectangle((int)p.x-9,(int)p.y+3,4,4,PAL_RED);
            for(int i=0;i<4;i++) DrawRectangle((int)p.x-12+i*9,(int)p.y-12-(int)(fmodf(t*12+i*7,25)),5,5,Fade(PAL_MUTED,.35f));
        }
    } else {
        if(c->phase==COURIER_RUN) for(int i=0;i<5;i++)
            DrawRectangle((int)p.x-14+i*7,(int)p.y+24+(int)(sinf(v->time*23+i)*3),3,3,PAL_GOLD);
        bot(p,v->time,g->delivered);
    }
    EndScissorMode();
}
