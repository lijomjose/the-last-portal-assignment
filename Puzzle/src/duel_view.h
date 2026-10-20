/* Native pixel-art arena, included after the shared drawing helpers. */
static Color attack_color(char symbol)
{
    if(symbol=='('||symbol==')') return PAL_GOLD;
    if(symbol=='['||symbol==']') return PAL_BLUE;
    return (Color){224,121,237,255};
}

static void bracket_sprite(char symbol, Vector2 p, int pixel, Color ink)
{
    static const unsigned round[9]={6,12,8,16,16,16,8,12,6};
    static const unsigned square[9]={30,16,16,16,16,16,16,16,30};
    static const unsigned curly[9]={6,8,8,8,16,8,8,8,6};
    const unsigned *rows=symbol=='('||symbol==')'?round:symbol=='['||symbol==']'?square:curly;
    bool mirrored=symbol==')'||symbol==']'||symbol=='}';
    for(int y=0;y<9;y++) for(int x=0;x<5;x++) if(rows[y]&(1u<<(4-x)))
        DrawRectangle((int)p.x+(mirrored?2-x:x-2)*pixel,(int)p.y+(y-4)*pixel,pixel,pixel,ink);
}

static void alien_sprite(Vector2 p, float time, float attack, float opacity, bool flash)
{
    const int k=4;
    int x=(int)p.x, y=(int)p.y+(int)(sinf(time*2.2f)*2);
    Color dark=Fade((Color){27,23,43,255},opacity);
    Color skin=Fade(flash?PAL_INK:(Color){94,57,128,255},opacity);
    Color light=Fade(flash?PAL_INK:(Color){156,85,168,255},opacity);
    Color armour=Fade((Color){76,156,138,255},opacity);
    Color fang=Fade((Color){243,222,167,255},opacity);
    Color eye=Fade((Color){255,141,73,255},opacity);
    DrawEllipse(x,y+13*k,42,7,Fade(PAL_BG,.8f*opacity));
    /* Heavy legs, four clawed arms, a plated torso and a horned jaw. */
    DrawRectangle(x-8*k,y+3*k,6*k,10*k,dark);
    DrawRectangle(x+2*k,y+3*k,6*k,10*k,dark);
    DrawRectangle(x-10*k,y+12*k,9*k,3*k,armour);
    DrawRectangle(x+2*k,y+12*k,9*k,3*k,armour);
    DrawRectangle(x-9*k,y-11*k,18*k,17*k,dark);
    DrawRectangle(x-7*k,y-10*k,14*k,14*k,skin);
    for(int i=0;i<3;i++) DrawRectangle(x-6*k,y+(-7+4*i)*k,12*k,2*k,armour);
    for(int side=-1;side<=1;side+=2) {
        int sway=(int)(sinf(time*3+side)*2+attack*5);
        DrawRectangle(x+side*10*k-2*k,y-9*k,5*k,4*k,light);
        DrawRectangle(x+side*(13+sway/2)*k-2*k,y-8*k,4*k,9*k,skin);
        DrawRectangle(x+side*(14+sway/2)*k-2*k,y-2*k,5*k,3*k,dark);
        DrawRectangle(x+side*(14+sway/2)*k,y+k,2*k,3*k,fang);
        DrawRectangle(x+side*10*k-2*k,y+k,5*k,4*k,skin);
        DrawRectangle(x+side*12*k-2*k,y+4*k,4*k,5*k,dark);
        DrawRectangle(x+side*13*k-k,y+8*k,2*k,3*k,fang);
        for(int i=0;i<4;i++) DrawRectangle(x+side*(7+i)*k-k,y+(-18-i)*k,2*k,4*k,armour);
    }
    DrawRectangle(x-9*k,y-20*k,18*k,11*k,dark);
    DrawRectangle(x-7*k,y-20*k,14*k,8*k,skin);
    DrawRectangle(x-5*k,y-21*k,10*k,2*k,light);
    DrawRectangle(x-6*k,y-17*k,4*k,2*k,eye);
    DrawRectangle(x+2*k,y-17*k,4*k,2*k,eye);
    DrawRectangle(x-5*k,y-17*k,k,k,fang);
    DrawRectangle(x+3*k,y-17*k,k,k,fang);
    DrawRectangle(x-5*k,y-13*k,10*k,(3+(int)(attack*2))*k,dark);
    for(int i=-4;i<=4;i+=2) DrawRectangle(x+i*k,y-13*k,k,2*k,fang);
    DrawRectangle(x-7*k,y-12*k,2*k,4*k,light);
    DrawRectangle(x+5*k,y-12*k,2*k,4*k,light);
}

static void energy_shot(View *v, Vector2 p, char symbol, int direction)
{
    Color color=attack_color(symbol);
    for(int i=8;i>0;i--) {
        int size=3+(8-i)/2;
        DrawRectangle((int)p.x-direction*(15+i*8),(int)p.y-size/2,size,size,Fade(color,.08f+(8-i)*.045f));
    }
    DrawCircle((int)p.x,(int)p.y,24,Fade(color,.08f));
    DrawRectangle((int)p.x-14,(int)p.y-23,29,47,Fade(PAL_BG,.85f));
    bracket_sprite(symbol,p,4,color);
    DrawRectangle((int)p.x-3,(int)p.y-28,6,3,Fade(PAL_INK,.6f+.3f*sinf(v->time*18)));
}

static void duel_arena(View *v, const Game *g)
{
    const DuelState *d=&g->duel;
    BeginScissorMode(BOARD_X,BOARD_Y,MAP_W*TILE,MAP_H*TILE);
    float shake=d->phase==DUEL_FINISH&&d->clock>.35f?sinf(d->clock*92)*4:0;
    Camera2D camera={.offset={shake,0},.target={0,0},.rotation=0,.zoom=1};
    BeginMode2D(camera);
    DrawRectangle(32,160,768,512,(Color){20,22,39,255});
    for(int row=0;row<9;row++) for(int column=0;column<14;column++) {
        int px=25+column*62+(row%2)*31, py=160+row*29;
        DrawRectangle(px+2,py+2,58,25,(Color){27,29,48,255});
        DrawLine(px+5,py+25,px+57,py+25,(Color){37,39,59,255});
    }
    DrawRectangleGradientV(32,352,768,320,(Color){25,28,44,255},(Color){43,43,59,255});
    for(int i=0;i<10;i++) DrawLine(410+(i-5)*28,353,32+i*91,672,Fade(PAL_BLUE,.15f));
    for(int i=0;i<8;i++) DrawLine(32,366+i*i*6,800,366+i*i*6,Fade(PAL_BLUE,.12f));
    for(int side=0;side<2;side++) {
        int px=side?766:32;
        DrawRectangle(px,160,34,512,(Color){14,19,33,255});
        for(int y=180;y<650;y+=73) {
            DrawRectangle(px+5,y,7,40,(Color){58,55,78,255});
            DrawRectangle(px+20,y+9,4,22,Fade(PAL_TEAL,.4f));
        }
    }
    for(int i=0;i<25;i++) {
        int px=70+(i*97)%675, py=365+(i*61)%240;
        DrawRectangle(px,py,3+i%4,2,Fade(PAL_GOLD,.12f));
    }
    /* Home is behind the alien; no path around it exists in the game map. */
    Vector2 gate=centre(game_exit(2));
    for(int i=3;i>0;i--) DrawEllipse((int)gate.x,(int)gate.y,26+i*5,42+i*4,Fade(d->defeated?PAL_TEAL:PAL_LINE,.05f));
    DrawEllipse((int)gate.x,(int)gate.y,27,45,PAL_BG);
    DrawEllipseLines((int)gate.x,(int)gate.y,28,46,d->defeated?PAL_TEAL:PAL_LINE);
    if(d->defeated) DrawEllipseLines((int)gate.x,(int)gate.y,22,38,PAL_GOLD);
    text(v,"HOME",gate.x-23,gate.y+60,15,d->defeated?PAL_TEAL:PAL_MUTED);

    Vector2 caitlyn=v->hero_pos;
    if(d->phase==DUEL_COUNTER) caitlyn.x-=5*sinf(fminf(1,d->clock/.55f)*3.14159f);
    Vector2 alien={594,423};
    float lunge=d->phase==DUEL_INCOMING?sinf(fminf(1,d->clock/1.0f)*3.14159f):0;
    alien.x-=lunge*10;
    if(d->phase==DUEL_COUNTER&&d->clock>.4f) alien.x+=5;
    if(!d->defeated) alien_sprite(alien,v->time,lunge,1,d->phase==DUEL_FINISH&&d->clock>.7f&&((int)(v->time*18)%2));
    hero(caitlyn,v->time,3);
    text(v,"CAITLYN",caitlyn.x-34,caitlyn.y+55,14,PAL_GOLD);
    if(!d->defeated) {
        text(v,"THE HOLLOW WARDEN",498,277,16,(Color){222,139,224,255});
        for(int i=0;i<DUEL_ATTACK_TOTAL;i++)
            DrawRectangle(499+i*21,307,16,6,i<DUEL_ATTACK_TOTAL-d->total_blocks?(Color){166,87,170,255}:PAL_LINE);
    }
    bool shielding=d->phase==DUEL_BLOCK||d->phase==DUEL_COUNTER;
    if(shielding) {
        Color color=!d->last_block_ok&&d->last_counter&&d->clock<.3f?PAL_RED:PAL_TEAL;
        for(int i=0;i<10;i++) {
            float angle=(-1.2f+i*.267f);
            DrawRectangle((int)(caitlyn.x+27+cosf(angle)*19),(int)(caitlyn.y-12+sinf(angle)*38),4,7,Fade(color,.7f));
        }
        DrawCircle((int)caitlyn.x+17,(int)caitlyn.y+4,10,Fade(color,.15f));
    }
    if(d->phase==DUEL_INCOMING) {
        float t=fminf(1,d->clock/1.0f);
        Vector2 p={alien.x-35+(caitlyn.x+29-alien.x+35)*t,411-22*sinf(t*3.14159f)};
        const char *pattern=game_wave_pattern(d->wave);
        int length=(int)strlen(pattern);
        for(int i=0;i<length;i++)
            energy_shot(v,(Vector2){p.x+(i-(length-1)*.5f)*38,p.y},pattern[i],-1);
    } else if(d->phase==DUEL_COUNTER) {
        float t=fminf(1,d->clock/.55f);
        Vector2 p={caitlyn.x+30+(alien.x-35-caitlyn.x-30)*t,427-32*sinf(t*3.14159f)};
        energy_shot(v,p,d->last_counter,1);
        if(t>.7f) for(int i=0;i<12;i++) {
            float angle=i*.5236f;
            DrawRectangle((int)(alien.x-30+cosf(angle)*(t-.7f)*130),(int)(414+sinf(angle)*(t-.7f)*130),4,4,PAL_GOLD);
        }
    } else if(d->phase==DUEL_FINISH) {
        float t=d->clock/1.3f;
        DrawCircle((int)caitlyn.x+17,(int)caitlyn.y+3,(int)(12+20*fminf(1,t*3)),Fade(PAL_TEAL,.2f));
        if(t>.25f) {
            int width=(int)((alien.x-caitlyn.x)*fminf(1,(t-.25f)*5));
            DrawRectangle((int)caitlyn.x+18,(int)caitlyn.y-22,width,49,Fade(PAL_TEAL,.22f));
            DrawRectangle((int)caitlyn.x+18,(int)caitlyn.y-9,width,23,PAL_TEAL);
            DrawRectangle((int)caitlyn.x+18,(int)caitlyn.y-2,width,8,PAL_INK);
        }
        if(t>.5f) for(int i=0;i<30;i++) {
            float angle=i*2.39996f, radius=(t-.5f)*(85+(i*13)%95);
            DrawRectangle((int)(alien.x+cosf(angle)*radius),(int)(alien.y-27+sinf(angle)*radius),5+i%5,5+i%5,i%3?PAL_GOLD:PAL_TEAL);
        }
    } else if(d->defeated&&d->clock<1.5f) {
        for(int i=0;i<18;i++) DrawRectangle(530+(i*37)%120,430+(i*17)%75+(int)(d->clock*18),5,5,Fade(PAL_BLUE,1-d->clock/1.5f));
    }

    text(v,TextFormat("SEQUENCE %d / 3     MATCHED %d / 9",d->wave+1,d->total_blocks),82,183,14,PAL_GOLD);
    DrawRectangleRounded((Rectangle){76,215,666,54},.09f,4,Fade(PAL_BG,.9f));
    text(v,"STACK",89,233,12,PAL_MUTED);
    int ids[ATTACK_CAPACITY], count=attack_copy_ids(g->attacks,ids,ATTACK_CAPACITY);
    const AttackNode *node=g->attacks;
    for(int i=0;i<count;i++,node=node->next) {
        Vector2 p={206+i*76,240};
        if(i==0) DrawRectangle((int)p.x-23,220,48,40,Fade(PAL_GOLD,.15f));
        bracket_sprite(node->opening,p,3,attack_color(node->opening));
        if(i<count-1) arrow((Vector2){p.x+21,p.y},(Vector2){p.x+53,p.y},PAL_LINE,2);
    }
    if(count==0) text(v,d->defeated?"all attacks answered":"no openings waiting",186,232,16,PAL_MUTED);
    else text(v,"TOP first",567,233,14,PAL_GOLD);

    mono(v,TextFormat("ENEMY SENT (L > R): %s",game_wave_pattern(d->wave)),82,525,17,PAL_MUTED);
    const char *caption=d->phase==DUEL_WAITING?"SPACE  /  FACE THE WARDEN":
        d->phase==DUEL_INCOMING?"WATCH THE BURST":d->phase==DUEL_BLOCK?"TYPE YOUR WHOLE COUNTER SEQUENCE":
        d->phase==DUEL_COUNTER?"BLOCKED!  RETURN FIRE":d->phase==DUEL_GAP?"WAVE CLEARED":
        d->phase==DUEL_READY?"HOMEBOUND IS CHARGED  /  PRESS F":d->phase==DUEL_FINISH?"HOMEBOUND!":"THE WAY HOME IS OPEN";
    text(v,caption,82,557,21,d->phase==DUEL_BLOCK?PAL_TEAL:PAL_GOLD);
    const char *note=d->defeated?"Hold Right / D. The portal waits beyond the defeated alien.":
        d->phase==DUEL_BLOCK?"1 = )    2 = ]    3 = }    ENTER launches / no timer":
        d->phase==DUEL_READY?"Three sequences, nine pairs blocked. The finishing move is unlocked.":
        "Both sequences read L to R: enemy STACK top vs your QUEUE front.";
    wrapped(v,note,82,599,647,16,PAL_MUTED);
    for(int y=162;y<672;y+=4) DrawLine(33,y,798,y,Fade(BLACK,.04f));
    EndMode2D();
    EndScissorMode();
}
