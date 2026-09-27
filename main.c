/* ============================================================
   TURBO DASH - PSP Edition
   ------------------------------------------------------------
   Personagem e jogo 100% originais, apenas inspirados no GENERO
   de plataforma rapida (misturando ideias de "pisar em inimigo
   pra derrotar" e "pads de velocidade / molas"). Nenhum nome,
   personagem, sprite ou trilha sonora de terceiros e usado.

   Mesmo motor em modo texto (pspdebug) do projeto anterior,
   pelos mesmos motivos: e muito mais dificil de dar tela preta
   e o chao/objetos nunca desalinham, porque sao redesenhados a
   cada frame a partir das mesmas constantes.
   ============================================================ */

#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <pspctrl.h>
#include <stdio.h>
#include <string.h>

PSP_MODULE_INFO("TurboDash", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTRIBUTE_USER);
PSP_HEAP_SIZE_KB(-1);

#define SCR_COLS      68
#define SCR_ROWS      34
#define GROUND_ROW    28
#define FLAG_COL      64
#define GRAVITY       0.35f
#define JUMP_VELOCITY (-3.2f)
#define SPRING_VELOCITY (-6.0f)
#define BASE_SPEED    0.5f
#define BOOST_SPEED   1.1f
#define MAX_ENEMIES   3
#define MAX_GEMS      6
#define START_LIVES   3

/* ---------- callbacks padrao PSP ---------- */

int exit_callback(int arg1, int arg2, void *common) {
    sceKernelExitGame();
    return 0;
}

int callback_thread(SceSize args, void *argp) {
    int cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
    sceKernelRegisterExitCallback(cbid);
    sceKernelSleepThreadCB();
    return 0;
}

int setup_callbacks(void) {
    int thid = sceKernelCreateThread("update_thread", callback_thread,
                                      0x11, 0xFA0, 0, 0);
    if (thid >= 0) sceKernelStartThread(thid, 0, 0);
    return thid;
}

/* ---------- pits (buracos no chao) ---------- */

typedef struct { int start, end; } Pit;
#define NUM_PITS 2
Pit pits[NUM_PITS] = {
    { 20, 24 },
    { 44, 47 },
};

int col_is_pit(int c) {
    int i;
    for (i = 0; i < NUM_PITS; i++)
        if (c >= pits[i].start && c <= pits[i].end) return 1;
    return 0;
}

/* ---------- pads especiais no chao ---------- */

#define BOOST_START 10
#define BOOST_END   16
#define SPRING_COL  52

/* ---------- inimigos (patrulham e podem ser pisados) ---------- */

typedef struct {
    float x;
    int min_x, max_x;
    int dir;
    int alive;
} Enemy;

Enemy enemies[MAX_ENEMIES] = {
    { 27.0f, 26, 32, 1, 1 },
    { 36.0f, 35, 42, 1, 1 },
    { 56.0f, 55, 61, 1, 1 },
};

/* ---------- gemas coletaveis ---------- */

typedef struct {
    int x;
    int collected;
} Gem;

Gem gems[MAX_GEMS] = {
    { 6, 0 }, { 18, 0 }, { 30, 0 }, { 40, 0 }, { 50, 0 }, { 58, 0 },
};

/* ---------- estado do jogador ---------- */

typedef struct {
    float x, y, vy;
    int on_ground;
    int alive;
    int won;
    int lives;
    int score;
    int boosting;
} Player;

void reset_position(Player *p) {
    p->x = 2.0f;
    p->y = (float)GROUND_ROW - 1.0f;
    p->vy = 0.0f;
    p->on_ground = 1;
    p->alive = 1;
    p->boosting = 0;
}

void reset_game(Player *p) {
    int i;
    reset_position(p);
    p->won = 0;
    p->lives = START_LIVES;
    p->score = 0;
    for (i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].alive = 1;
    }
    for (i = 0; i < MAX_GEMS; i++) gems[i].collected = 0;
}

/* ---------- desenho ---------- */

void draw_frame(Player *p) {
    static char screen[SCR_ROWS][SCR_COLS + 1];
    int r, c, i;

    for (r = 0; r < SCR_ROWS; r++) {
        for (c = 0; c < SCR_COLS; c++) screen[r][c] = ' ';
        screen[r][SCR_COLS] = '\0';
    }

    /* chao, com buracos */
    for (c = 0; c < SCR_COLS; c++) {
        if (!col_is_pit(c)) {
            if (c >= BOOST_START && c <= BOOST_END) screen[GROUND_ROW][c] = '>';
            else if (c == SPRING_COL) screen[GROUND_ROW][c] = 'W';
            else screen[GROUND_ROW][c] = '=';
        }
    }

    /* bandeira */
    if (FLAG_COL < SCR_COLS) {
        screen[GROUND_ROW - 1][FLAG_COL] = '|';
        screen[GROUND_ROW - 2][FLAG_COL] = 'P';
    }

    /* gemas */
    for (i = 0; i < MAX_GEMS; i++) {
        if (!gems[i].collected && gems[i].x < SCR_COLS)
            screen[GROUND_ROW - 1][gems[i].x] = '*';
    }

    /* inimigos */
    for (i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].alive) {
            int ec = (int)(enemies[i].x + 0.5f);
            if (ec >= 0 && ec < SCR_COLS) screen[GROUND_ROW - 1][ec] = 'e';
        }
    }

    /* jogador */
    int pr = (int)(p->y + 0.5f);
    int pc = (int)(p->x + 0.5f);
    if (pr >= 0 && pr < SCR_ROWS && pc >= 0 && pc < SCR_COLS) {
        screen[pr][pc] = p->alive ? (p->boosting ? '>' : '@') : 'X';
    }

    pspdebugScreenSetXY(0, 0);
    pspdebugScreenPrintf("TURBO DASH - PSP Edition                                          ");
    pspdebugScreenPrintf("Vidas: %d   Gemas: %d/%d   Pontos: %d                              ",
                          p->lives, p->score, MAX_GEMS, p->score);
    pspdebugScreenPrintf("D-Pad: mover  X: pular  >: pad de boost  W: mola  START: reiniciar ");

    for (r = 0; r < SCR_ROWS; r++) {
        pspdebugScreenSetXY(0, r + 3);
        pspdebugScreenPrintf("%s", screen[r]);
    }

    if (p->won) {
        pspdebugScreenSetXY(18, 10);
        pspdebugScreenPrintf("*** VOCE VENCEU! Aperte START para jogar de novo ***");
    } else if (p->lives <= 0) {
        pspdebugScreenSetXY(20, 10);
        pspdebugScreenPrintf("*** GAME OVER - Aperte START para recomecar ***");
    }
}

/* ---------- loop principal ---------- */

int main(int argc, char *argv[]) {
    SceCtrlData pad;
    Player player;
    int i;
    int prev_y_below_enemy[MAX_ENEMIES];

    setup_callbacks();
    pspdebugScreenInit();
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

    reset_game(&player);

    while (1) {
        sceCtrlReadBufferPositive(&pad, 1);

        if (pad.Buttons & PSP_CTRL_START) {
            reset_game(&player);
        }

        if (player.lives > 0 && !player.won) {
            int col = (int)(player.x + 0.5f);
            int on_boost = (col >= BOOST_START && col <= BOOST_END);
            float speed = on_boost ? BOOST_SPEED : BASE_SPEED;
            player.boosting = on_boost;

            /* movimento horizontal */
            if ((pad.Buttons & PSP_CTRL_LEFT) || pad.Lx < 80) {
                player.x -= speed;
            }
            if ((pad.Buttons & PSP_CTRL_RIGHT) || pad.Lx > 176) {
                player.x += speed;
            }
            if (player.x < 0) player.x = 0;
            if (player.x > SCR_COLS - 1) player.x = (float)(SCR_COLS - 1);

            /* pulo normal */
            if ((pad.Buttons & PSP_CTRL_CROSS) && player.on_ground) {
                player.vy = JUMP_VELOCITY;
                player.on_ground = 0;
            }

            /* gravidade */
            player.vy += GRAVITY;
            player.y += player.vy;

            col = (int)(player.x + 0.5f);
            int is_pit = col_is_pit(col);
            int on_spring = (col == SPRING_COL);
            float ground_level = (float)GROUND_ROW - 1.0f;

            if (!is_pit && player.y >= ground_level) {
                player.y = ground_level;
                if (on_spring && player.vy >= 0) {
                    player.vy = SPRING_VELOCITY; /* mola: pulo bem mais alto */
                    player.on_ground = 0;
                } else {
                    player.vy = 0;
                    player.on_ground = 1;
                }
            } else {
                player.on_ground = 0;
            }

            /* caiu no buraco / fora da tela: perde 1 vida */
            if (player.y > SCR_ROWS - 1) {
                player.lives--;
                reset_position(&player);
            }

            /* coletar gemas */
            for (i = 0; i < MAX_GEMS; i++) {
                if (!gems[i].collected && col == gems[i].x &&
                    (int)(player.y + 0.5f) == GROUND_ROW - 1) {
                    gems[i].collected = 1;
                    player.score += 10;
                }
            }

            /* inimigos: patrulham e podem ser pisados ou machucar */
            for (i = 0; i < MAX_ENEMIES; i++) {
                if (!enemies[i].alive) continue;

                enemies[i].x += 0.2f * enemies[i].dir;
                if (enemies[i].x >= enemies[i].max_x) enemies[i].dir = -1;
                if (enemies[i].x <= enemies[i].min_x) enemies[i].dir = 1;

                int ec = (int)(enemies[i].x + 0.5f);
                int player_row = (int)(player.y + 0.5f);

                if (ec == col) {
                    if (player_row == GROUND_ROW - 2 && player.vy > 0) {
                        /* pisou em cima: derrota o inimigo */
                        enemies[i].alive = 0;
                        player.score += 25;
                        player.vy = JUMP_VELOCITY * 0.6f; /* pequeno quique */
                    } else if (player_row >= GROUND_ROW - 1) {
                        /* colidiu de lado: perde 1 vida */
                        player.lives--;
                        reset_position(&player);
                    }
                }
            }

            /* chegou na bandeira */
            if (col >= FLAG_COL && player.on_ground) {
                player.won = 1;
            }

            if (player.lives <= 0) player.alive = 0;
        }

        draw_frame(&player);
        sceDisplayWaitVblankStart();
    }

    sceKernelExitGame();
    return 0;
}
