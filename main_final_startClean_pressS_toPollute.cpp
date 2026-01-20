#include <windows.h>
#include <GL/glut.h>
#include <math.h>
#include <stdlib.h>

float wasteOffset;
float cloudOffset;
float fishOffset;
float smogOffset;
float trashOffset;
float birdOffset;
float particleOffset;
float boatOffset;

bool showBoat = false;


bool pollutionStarted = false;


bool showAirCleaner = false;
float airCleanLevel = 0.0f;
float airFanAngle   = 0.0f;
float airFlowPhase  = 0.0f;
float airTruckX = -300.0f;
bool  airTruckParked = false;
bool  airCleaningArmed  = false;
bool  airCleaningActive = false;
bool  airCleanLocked = false;
int   airPostSprayFrames = 0;

const float AIR_TRUCK_STOP_X = 460.0f;
const float AIR_TRUCK_GROUND_Y = 200.0f;
const float AIR_TRUCK_SCALE = 1.05f;


const float AIR_SPRAY_R = 0.45f;
const float AIR_SPRAY_G = 1.00f;
const float AIR_SPRAY_B = 0.70f;


const float AIR_CLEAN_RATE = 0.004f;
const int   AIR_POST_SPRAY_FRAMES = 180;
bool cleaning = false;
float cleanLevel = 0.0f;

float brushAngle = 0.0f;
float beltPhase  = 0.0f;

const float BOAT_SCALE = 1.6f;
const float POLLUTED_START_X = 500.0f;

static float clamp01(float v) { return (v < 0.0f) ? 0.0f : (v > 1.0f ? 1.0f : v); }
static float mixf(float a, float b, float t) { return a + (b - a) * t; }
static float smoothstep(float e0, float e1, float x) {
    float t = clamp01((x - e0) / (e1 - e0));
    return t * t * (3.0f - 2.0f * t);
}

static float boatMaxOffset() {

    const float START_X = 20.0f;
    const float FRONT   = 66.0f;
    const float MARGIN  = 2.0f;
    return (POLLUTED_START_X - START_X) - (FRONT * BOAT_SCALE) - MARGIN;
}

void drawCircle(float cx, float cy, float r, int segments) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= segments; i++) {
        float theta = 2.0f * 3.14159f * i / segments;
        float x = r * cosf(theta);
        float y = r * sinf(theta);
        glVertex2f(cx + x, cy + y);
    }
    glEnd();
}


void drawText(float x, float y, const char* s) {
    glRasterPos2f(x, y);
    for (const char* p = s; *p; ++p) {
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *p);
    }
}

void drawRotatingBrush(float cx, float cy, float r, float angleDeg) {

    glColor3f(0.18f, 0.18f, 0.18f);
    drawCircle(cx, cy, r, 20);


    glColor3f(0.75f, 0.75f, 0.75f);
    drawCircle(cx, cy, r * 0.22f, 14);


    glColor4f(0.92f, 0.92f, 0.92f, 0.85f);
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < 5; i++) {
        float a0 = (angleDeg + i * 72.0f) * 3.14159f / 180.0f;
        float a1 = a0 + 0.55f;
        float a2 = a0 - 0.55f;

        float ir = r * 0.28f;
        float orr = r * 0.92f;

        glVertex2f(cx + ir * cosf(a0),  cy + ir * sinf(a0));
        glVertex2f(cx + orr * cosf(a1), cy + orr * sinf(a1));
        glVertex2f(cx + orr * cosf(a2), cy + orr * sinf(a2));
    }
    glEnd();


    glColor4f(0.2f, 0.2f, 0.2f, 0.6f);
    glLineWidth(1);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 20; i++) {
        float a = (2.0f * 3.14159f * i) / 20.0f;
        glVertex2f(cx + (r * 0.98f) * cosf(a), cy + (r * 0.98f) * sinf(a));
    }
    glEnd();
}




void drawAirCleanerTruck(float cx, float groundY, float scale, bool running) {
    float s = scale;
    float x = cx;
    float y = groundY;
    float a = clamp01(airCleanLevel);


    glPushMatrix();
    glTranslatef(x, 0.0f, 0.0f);
    glScalef(-1.0f, 1.0f, 1.0f);
    glTranslatef(-x, 0.0f, 0.0f);



    float wheelY = y + 12.0f * s;
    float wR = 10.5f * s;

    glColor3f(0.10f, 0.10f, 0.10f);
    drawCircle(x - 55.0f * s, wheelY, wR, 18);
    drawCircle(x + 45.0f * s, wheelY, wR, 18);
    glColor3f(0.70f, 0.70f, 0.72f);
    drawCircle(x - 55.0f * s, wheelY, wR * 0.45f, 16);
    drawCircle(x + 45.0f * s, wheelY, wR * 0.45f, 16);


    glColor3f(0.22f, 0.22f, 0.24f);
    glBegin(GL_QUADS);
    glVertex2f(x - 85*s, y + 18*s);
    glVertex2f(x + 85*s, y + 18*s);
    glVertex2f(x + 85*s, y + 42*s);
    glVertex2f(x - 85*s, y + 42*s);
    glEnd();


    glColor3f(0.12f, 0.12f, 0.12f);
    glBegin(GL_QUADS);
    glVertex2f(x - 90*s, y + 22*s);
    glVertex2f(x - 80*s, y + 22*s);
    glVertex2f(x - 80*s, y + 36*s);
    glVertex2f(x - 90*s, y + 36*s);
    glEnd();


    glColor3f(0.88f, 0.90f, 0.92f);
    glBegin(GL_QUADS);
    glVertex2f(x - 80*s, y + 42*s);
    glVertex2f(x - 25*s, y + 42*s);
    glVertex2f(x - 25*s, y + 78*s);
    glVertex2f(x - 75*s, y + 78*s);
    glEnd();


    glColor3f(0.78f, 0.80f, 0.82f);
    glBegin(GL_QUADS);
    glVertex2f(x - 75*s, y + 78*s);
    glVertex2f(x - 25*s, y + 78*s);
    glVertex2f(x - 32*s, y + 88*s);
    glVertex2f(x - 70*s, y + 88*s);
    glEnd();


    glColor4f(0.55f, 0.75f, 0.90f, 0.75f);
    glBegin(GL_QUADS);
    glVertex2f(x - 70*s, y + 55*s);
    glVertex2f(x - 38*s, y + 55*s);
    glVertex2f(x - 40*s, y + 75*s);
    glVertex2f(x - 70*s, y + 75*s);
    glEnd();


    glColor3f(1.0f, 0.95f, 0.75f);
    drawCircle(x - 84*s, y + 30*s, 3.2f*s, 12);
    drawCircle(x - 84*s, y + 26*s, 2.6f*s, 12);


    glColor3f(0.40f, 0.58f, 0.78f);
    glBegin(GL_QUADS);
    glVertex2f(x - 20*s, y + 42*s);
    glVertex2f(x + 85*s, y + 42*s);
    glVertex2f(x + 85*s, y + 92*s);
    glVertex2f(x - 20*s, y + 92*s);
    glEnd();


    glColor3f(0.33f, 0.50f, 0.68f);
    glBegin(GL_QUADS);
    glVertex2f(x - 20*s, y + 42*s);
    glVertex2f(x - 12*s, y + 42*s);
    glVertex2f(x - 12*s, y + 92*s);
    glVertex2f(x - 20*s, y + 92*s);
    glEnd();


    drawRotatingBrush(x + 25*s, y + 66*s, 11*s, airFanAngle);


    glColor3f(0.25f, 0.28f, 0.30f);
    glBegin(GL_QUADS);
    glVertex2f(x + 55*s, y + 92*s);
    glVertex2f(x + 72*s, y + 92*s);
    glVertex2f(x + 72*s, y + 128*s);
    glVertex2f(x + 55*s, y + 128*s);
    glEnd();
    glColor3f(0.18f, 0.18f, 0.18f);
    drawCircle(x + 63.5f*s, y + 128*s, 9.0f*s, 18);


    glColor3f(mixf(0.8f, 0.1f, a), mixf(0.1f, 0.9f, a), 0.1f);
    drawCircle(x + 5*s, y + 88*s, 3.8f*s, 14);



    glPopMatrix();


    glColor3f(0.05f, 0.05f, 0.05f);
    drawText(x - 10*s, y + 48*s, "AIR VAC");

    if (running) {

        float nx = x - 63.5f*s;
        float ny = y + 132.0f*s;

        glColor3f(0.12f, 0.12f, 0.12f);
        glBegin(GL_QUADS);
        glVertex2f(nx - 6*s, ny - 2*s);
        glVertex2f(nx + 6*s, ny - 2*s);
        glVertex2f(nx + 6*s, ny + 8*s);
        glVertex2f(nx - 6*s, ny + 8*s);
        glEnd();


        glColor4f(AIR_SPRAY_R, AIR_SPRAY_G, AIR_SPRAY_B, 0.32f);
        glBegin(GL_TRIANGLES);
        glVertex2f(nx, ny + 8*s);
        glVertex2f(nx - 72*s, ny + 140*s);
        glVertex2f(nx + 72*s, ny + 140*s);
        glEnd();


        for (int i = 0; i < 18; i++) {
            float t = fmodf((airFlowPhase * 0.02f) + i * 0.07f, 1.0f);
            float spread = (0.2f + 0.8f * t);
            float px = nx + (sinf(airFlowPhase * 0.03f + i) * 18.0f*s) * spread;
            float py = (ny + 10*s) + t * (150.0f*s) + cosf(airFlowPhase * 0.02f + i) * 4.0f*s;
            glColor4f(AIR_SPRAY_R*0.95f, AIR_SPRAY_G*0.92f, AIR_SPRAY_B*0.98f, 0.22f + 0.38f * (1.0f - t));
            drawCircle(px, py, (3.5f + (i % 4)) * s, 14);
        }


        glColor4f(0.70f, 0.90f, 1.0f, 0.10f + 0.10f * clamp01(airCleanLevel));
        glBegin(GL_QUADS);
        glVertex2f(560, 330);
        glVertex2f(800, 330);
        glVertex2f(800, 520);
        glVertex2f(560, 520);
        glEnd();
    }

}
void drawCloud(float x, float y, float scale) {
    glColor3f(1.0f, 1.0f, 1.0f);
    drawCircle(x, y, 15 * scale, 20);
    drawCircle(x + 12 * scale, y, 18 * scale, 20);
    drawCircle(x + 25 * scale, y, 15 * scale, 20);
    drawCircle(x + 12 * scale, y + 8 * scale, 12 * scale, 20);
}

void drawDarkCloud(float x, float y, float scale) {

    float air = clamp01(airCleanLevel);
    float a = 0.85f * (1.0f - air);
    if (a <= 0.01f) return;


    float dx = sinf(smogOffset * 0.006f + x * 0.010f) * 10.0f * (1.0f - air);
    float dy = cosf(smogOffset * 0.005f + y * 0.012f) *  5.0f * (1.0f - air);
    float sc = 1.0f + 0.03f * sinf(smogOffset * 0.010f + x * 0.020f);

    x += dx;
    y += dy;
    scale *= sc;

    glColor4f(0.15f, 0.15f, 0.15f, a);
    drawCircle(x, y, 15 * scale, 20);
    drawCircle(x + 12 * scale, y, 18 * scale, 20);
    drawCircle(x + 25 * scale, y, 15 * scale, 20);
    drawCircle(x + 12 * scale, y + 8 * scale, 12 * scale, 20);

}


void drawTree(float x, float y, bool isDead) {
    if (isDead) {

        float t = smoothstep(0.15f, 1.0f, airCleanLevel);


        float tr = 0.40f;
        float tg = mixf(0.35f, 0.30f, t);
        float tb = mixf(0.30f, 0.20f, t);
        glColor3f(tr, tg, tb);


        glBegin(GL_QUADS);
        glVertex2f(x - 3, y);
        glVertex2f(x + 3, y);
        glVertex2f(x + 2, y + 25);
        glVertex2f(x - 2, y + 25);
        glEnd();


        glColor4f(0.35f, 0.30f, 0.25f, 1.0f - 0.90f * t);
        glBegin(GL_LINES);
        glVertex2f(x, y + 20); glVertex2f(x - 8, y + 28);
        glVertex2f(x, y + 18); glVertex2f(x + 7, y + 26);
        glVertex2f(x, y + 15); glVertex2f(x - 6, y + 20);
        glEnd();


        if (t > 0.02f) {
            float alpha = 0.10f + 0.90f * t;
            glColor4f(0.20f, 0.60f, 0.20f, alpha);

            float rMain = 6.0f + 4.0f * t;
            float rSide = 5.0f + 3.5f * t;
            drawCircle(x,     y + 25, rMain, 20);
            drawCircle(x - 7, y + 20, rSide, 20);
            drawCircle(x + 7, y + 20, rSide, 20);
        }

    } else {

        glColor3f(0.4f, 0.3f, 0.2f);

        glBegin(GL_QUADS);
        glVertex2f(x - 3, y);
        glVertex2f(x + 3, y);
        glVertex2f(x + 2, y + 20);
        glVertex2f(x - 2, y + 20);
        glEnd();

        glColor3f(0.2f, 0.6f, 0.2f);
        drawCircle(x, y + 25, 10, 20);
        drawCircle(x - 7, y + 20, 8, 20);
        drawCircle(x + 7, y + 20, 8, 20);
    }
}


void drawSign(float x, float y, int type) {

    glColor3f(0.5f, 0.5f, 0.5f);
    glBegin(GL_QUADS);
    glVertex2f(x - 2, y);
    glVertex2f(x + 2, y);
    glVertex2f(x + 2, y + 35);
    glVertex2f(x - 2, y + 35);
    glEnd();

    if (type == 0) {

        glColor3f(1.0f, 0.9f, 0.0f);
        glBegin(GL_TRIANGLES);
        glVertex2f(x, y + 55);
        glVertex2f(x - 18, y + 30);
        glVertex2f(x + 18, y + 30);
        glEnd();


        glColor3f(0.0f, 0.0f, 0.0f);
        glLineWidth(2);
        glBegin(GL_LINE_LOOP);
        glVertex2f(x, y + 55);
        glVertex2f(x - 18, y + 30);
        glVertex2f(x + 18, y + 30);
        glEnd();


        glColor3f(0.0f, 0.0f, 0.0f);

        drawCircle(x, y + 43, 6, 15);

        glColor3f(1.0f, 0.9f, 0.0f);
        drawCircle(x - 3, y + 44, 1.5f, 10);
        drawCircle(x + 3, y + 44, 1.5f, 10);

        glColor3f(0.0f, 0.0f, 0.0f);
        glLineWidth(2);
        glBegin(GL_LINES);
        glVertex2f(x - 4, y + 38);
        glVertex2f(x + 4, y + 38);
        glEnd();

    } else if (type == 1) {

        glColor3f(0.9f, 0.1f, 0.1f);
        drawCircle(x, y + 42, 15, 30);


        glColor3f(1.0f, 1.0f, 1.0f);
        drawCircle(x, y + 42, 12, 30);


        glColor3f(0.9f, 0.1f, 0.1f);
        glBegin(GL_QUADS);
        glVertex2f(x - 12, y + 40);
        glVertex2f(x + 12, y + 40);
        glVertex2f(x + 12, y + 44);
        glVertex2f(x - 12, y + 44);
        glEnd();

    } else {

        glColor3f(1.0f, 0.85f, 0.0f);
        glBegin(GL_QUADS);
        glVertex2f(x, y + 55);
        glVertex2f(x - 15, y + 42);
        glVertex2f(x, y + 29);
        glVertex2f(x + 15, y + 42);
        glEnd();


        glColor3f(0.0f, 0.0f, 0.0f);
        glLineWidth(3);
        glBegin(GL_LINE_LOOP);
        glVertex2f(x, y + 55);
        glVertex2f(x - 15, y + 42);
        glVertex2f(x, y + 29);
        glVertex2f(x + 15, y + 42);
        glEnd();


        glBegin(GL_QUADS);
        glVertex2f(x - 2, y + 36);
        glVertex2f(x + 2, y + 36);
        glVertex2f(x + 2, y + 48);
        glVertex2f(x - 2, y + 48);
        glEnd();
        drawCircle(x, y + 33, 2, 12);
    }
}

void drawBird(float x, float y) {
    glColor3f(0.2f, 0.2f, 0.2f);
    glLineWidth(2);
    glBegin(GL_LINE_STRIP);
    glVertex2f(x - 5, y);
    glVertex2f(x, y + 3);
    glVertex2f(x + 5, y);
    glEnd();
}


void drawBoat(float x, float y, float scale) {
    float s = scale;


    glColor4f(0.0f, 0.0f, 0.0f, 0.12f);
    glBegin(GL_QUADS);
    glVertex2f(x - 54 * s, y - 24 * s);
    glVertex2f(x + 54 * s, y - 24 * s);
    glVertex2f(x + 40 * s, y - 34 * s);
    glVertex2f(x - 40 * s, y - 34 * s);
    glEnd();


    glColor3f(0.16f, 0.20f, 0.26f);
    glBegin(GL_POLYGON);
    glVertex2f(x - 54 * s, y - 10 * s);
    glVertex2f(x + 46 * s, y - 10 * s);
    glVertex2f(x + 56 * s, y - 24 * s);
    glVertex2f(x + 40 * s, y - 34 * s);
    glVertex2f(x - 40 * s, y - 34 * s);
    glVertex2f(x - 56 * s, y - 22 * s);
    glEnd();

    glColor3f(0.06f, 0.08f, 0.10f);
    glLineWidth(2);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x - 54 * s, y - 10 * s);
    glVertex2f(x + 46 * s, y - 10 * s);
    glVertex2f(x + 56 * s, y - 24 * s);
    glVertex2f(x + 40 * s, y - 34 * s);
    glVertex2f(x - 40 * s, y - 34 * s);
    glVertex2f(x - 56 * s, y - 22 * s);
    glEnd();

    glColor3f(0.58f, 0.60f, 0.63f);
    glBegin(GL_QUADS);
    glVertex2f(x - 44 * s, y - 10 * s);
    glVertex2f(x + 38 * s, y - 10 * s);
    glVertex2f(x + 30 * s, y - 16 * s);
    glVertex2f(x - 36 * s, y - 16 * s);
    glEnd();


    float tx0 = x - 46 * s, tx1 = x - 10 * s;
    float ty0 = y - 10 * s, ty1 = y + 16 * s;
    float rr  = 7.0f * s;

    glColor3f(0.32f, 0.36f, 0.40f);

    glBegin(GL_QUADS);
    glVertex2f(tx0 + rr, ty0);
    glVertex2f(tx1 - rr, ty0);
    glVertex2f(tx1 - rr, ty1);
    glVertex2f(tx0 + rr, ty1);
    glEnd();

    glColor3f(0.30f, 0.34f, 0.38f);
    drawCircle(tx0 + rr, (ty0 + ty1) * 0.5f, rr, 18);
    drawCircle(tx1 - rr, (ty0 + ty1) * 0.5f, rr, 18);


    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(x - 41 * s, y + 2 * s, "TANK");


    float fill = clamp01(cleanLevel);
    if (fill > 0.02f) {
        glColor4f(0.25f, 0.20f, 0.12f, 0.75f);
        float fh = (ty1 - ty0 - 4 * s) * fill;
        glBegin(GL_QUADS);
        glVertex2f(tx0 + 6 * s, ty0 + 2 * s);
        glVertex2f(tx1 - 6 * s, ty0 + 2 * s);
        glVertex2f(tx1 - 6 * s, ty0 + 2 * s + fh);
        glVertex2f(tx0 + 6 * s, ty0 + 2 * s + fh);
        glEnd();
    }


    glColor3f(0.86f, 0.86f, 0.88f);
    glBegin(GL_QUADS);
    glVertex2f(x - 2 * s,  y - 10 * s);
    glVertex2f(x + 26 * s, y - 10 * s);
    glVertex2f(x + 26 * s, y + 18 * s);
    glVertex2f(x - 2 * s,  y + 18 * s);
    glEnd();


    glColor3f(0.2f, 0.2f, 0.22f);
    glLineWidth(2);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x - 2 * s,  y - 10 * s);
    glVertex2f(x + 26 * s, y - 10 * s);
    glVertex2f(x + 26 * s, y + 18 * s);
    glVertex2f(x - 2 * s,  y + 18 * s);
    glEnd();


    glColor3f(0.10f, 0.10f, 0.10f);
    drawCircle(x + 12 * s, y + 4 * s, 7.0f * s, 18);
    drawRotatingBrush(x + 12 * s, y + 4 * s, 6.2f * s, brushAngle);


    glColor3f(0.1f, 0.1f, 0.1f);
    drawText(x + 1 * s, y + 20 * s, "VACUUM");


    glColor3f(0.12f, 0.12f, 0.12f);
    glLineWidth(6);
    glBegin(GL_LINE_STRIP);
    glVertex2f(x + 30 * s, y + 6 * s);
    glVertex2f(x + 40 * s, y + 0 * s);
    glVertex2f(x + 50 * s, y - 10 * s);
    glVertex2f(x + 54 * s, y - 20 * s);
    glEnd();

    glColor4f(1.0f, 1.0f, 1.0f, 0.12f);
    glLineWidth(2);
    glBegin(GL_LINE_STRIP);
    glVertex2f(x + 30 * s, y + 7 * s);
    glVertex2f(x + 40 * s, y + 1 * s);
    glVertex2f(x + 50 * s, y - 9 * s);
    glVertex2f(x + 54 * s, y - 19 * s);
    glEnd();

    glColor3f(0.06f, 0.06f, 0.07f);
    glBegin(GL_QUADS);
    glVertex2f(x + 46 * s, y - 16 * s);
    glVertex2f(x + 66 * s, y - 16 * s);
    glVertex2f(x + 66 * s, y - 34 * s);
    glVertex2f(x + 46 * s, y - 34 * s);
    glEnd();


    glColor4f(0.95f, 0.95f, 0.95f, cleaning ? 0.55f : 0.25f);
    glLineWidth(1);
    glBegin(GL_LINES);
    for (int i = 0; i < 5; i++) {
        float xx = (x + 48 * s) + i * (4.0f * s);
        glVertex2f(xx, y - 17 * s);
        glVertex2f(xx, y - 33 * s);
    }
    for (int i = 0; i < 3; i++) {
        float yy = (y - 20 * s) - i * (5.0f * s);
        glVertex2f(x + 47 * s, yy);
        glVertex2f(x + 65 * s, yy);
    }
    glEnd();


    for (int i = 0; i < 6; i++) {
        if (i % 2 == 0) glColor3f(0.95f, 0.85f, 0.10f);
        else            glColor3f(0.10f, 0.10f, 0.10f);
        float sx0 = (x + 46 * s) + i * (3.4f * s);
        glBegin(GL_QUADS);
        glVertex2f(sx0, y - 16 * s);
        glVertex2f(sx0 + 3.0f * s, y - 16 * s);
        glVertex2f(sx0 + 3.0f * s, y - 13 * s);
        glVertex2f(sx0, y - 13 * s);
        glEnd();
    }

    if (cleaning) {
        glColor4f(0.80f, 0.95f, 1.0f, 0.65f);

        float p0x = x + 62 * s, p0y = y - 32 * s;
        float p1x = x + 56 * s, p1y = y - 20 * s;
        float p2x = x + 44 * s, p2y = y - 4  * s;
        float p3x = x + 30 * s, p3y = y + 6  * s;

        for (int i = 0; i < 14; i++) {
            float t = fmod(beltPhase * 0.02f + i * 0.09f, 1.0f);

            float px, py;
            if (t < 0.33f) {
                float u = t / 0.33f;
                px = p0x + (p1x - p0x) * u;
                py = p0y + (p1y - p0y) * u;
            } else if (t < 0.66f) {
                float u = (t - 0.33f) / 0.33f;
                px = p1x + (p2x - p1x) * u;
                py = p1y + (p2y - p1y) * u;
            } else {
                float u = (t - 0.66f) / 0.34f;
                px = p2x + (p3x - p2x) * u;
                py = p2y + (p3y - p2y) * u;
            }
            float rr2 = (1.4f + (i % 3) * 0.5f) * s;
            drawCircle(px + sinf((beltPhase + i) * 0.2f) * 1.6f * s,
                       py + cosf((beltPhase + i) * 0.22f) * 1.2f * s,
                       rr2, 10);
        }
    }


    glColor3f(0.95f, 0.55f, 0.10f);
    drawCircle(x + 26 * s, y + 20 * s, 2.2f * s, 12);


    glColor4f(1.0f, 1.0f, 1.0f, 0.28f);
    glLineWidth(2);
    glBegin(GL_LINES);
    glVertex2f(x - 60 * s, y - 22 * s);
    glVertex2f(x - 84 * s, y - 20 * s);
    glVertex2f(x - 56 * s, y - 30 * s);
    glVertex2f(x - 82 * s, y - 30 * s);
    glEnd();
}


void drawPerson(float x, float y, bool hasMask) {

    glColor3f(0.9f, 0.75f, 0.6f);
    drawCircle(x, y + 18, 4, 15);

    glColor3f(0.3f, 0.3f, 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(x - 3, y + 14);
    glVertex2f(x + 3, y + 14);
    glVertex2f(x + 3, y + 6);
    glVertex2f(x - 3, y + 6);
    glEnd();


    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
    glVertex2f(x - 2, y + 6);
    glVertex2f(x - 1, y + 6);
    glVertex2f(x - 1, y);
    glVertex2f(x - 2, y);
    glEnd();
    glBegin(GL_QUADS);
    glVertex2f(x + 1, y + 6);
    glVertex2f(x + 2, y + 6);
    glVertex2f(x + 2, y);
    glVertex2f(x + 1, y);
    glEnd();

    glBegin(GL_QUADS);
    glVertex2f(x - 5, y + 12);
    glVertex2f(x - 3, y + 12);
    glVertex2f(x - 3, y + 8);
    glVertex2f(x - 5, y + 8);
    glEnd();
    glBegin(GL_QUADS);
    glVertex2f(x + 3, y + 12);
    glVertex2f(x + 5, y + 12);
    glVertex2f(x + 5, y + 8);
    glVertex2f(x + 3, y + 8);
    glEnd();

    if (hasMask) {

        glColor3f(0.95f, 0.95f, 0.95f);
        glBegin(GL_QUADS);
        glVertex2f(x - 3, y + 16);
        glVertex2f(x + 3, y + 16);
        glVertex2f(x + 3, y + 19);
        glVertex2f(x - 3, y + 19);
        glEnd();
    }
}

void drawTrash(float x, float y, int type) {
    if (type == 0) {

        glColor3f(0.7f, 0.85f, 0.95f);
        glBegin(GL_QUADS);
        glVertex2f(x - 2, y - 4);
        glVertex2f(x + 2, y - 4);
        glVertex2f(x + 2, y + 4);
        glVertex2f(x - 2, y + 4);
        glEnd();
        glBegin(GL_QUADS);
        glVertex2f(x - 1.5f, y + 4);
        glVertex2f(x + 1.5f, y + 4);
        glVertex2f(x + 1.5f, y + 6);
        glVertex2f(x - 1.5f, y + 6);
        glEnd();
    } else if (type == 1) {

        glColor3f(0.6f, 0.6f, 0.6f);
        glBegin(GL_QUADS);
        glVertex2f(x - 3, y - 3);
        glVertex2f(x + 3, y - 3);
        glVertex2f(x + 3, y + 3);
        glVertex2f(x - 3, y + 3);
        glEnd();
    } else if (type == 2) {

        glColor4f(0.85f, 0.85f, 0.85f, 0.6f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(x, y);
        for (int i = 0; i <= 6; i++) {
            float angle = i * 60.0f * 3.14159f / 180.0f;
            float radius = 4 + sin(angle * 2) * 1.5f;
            glVertex2f(x + radius * cos(angle), y + radius * sin(angle));
        }
        glEnd();
    } else if (type == 3) {

        glColor3f(0.15f, 0.15f, 0.15f);
        drawCircle(x, y, 5, 20);
        glColor3f(0.1f, 0.1f, 0.1f);
        drawCircle(x, y, 3, 15);
    } else if (type == 4) {

        glColor3f(0.7f, 0.3f, 0.2f);
        glBegin(GL_QUADS);
        glVertex2f(x - 4, y - 5);
        glVertex2f(x + 4, y - 5);
        glVertex2f(x + 4, y + 5);
        glVertex2f(x - 4, y + 5);
        glEnd();
        glColor3f(0.5f, 0.5f, 0.5f);
        glBegin(GL_QUADS);
        glVertex2f(x - 4, y);
        glVertex2f(x + 4, y);
        glVertex2f(x + 4, y + 1);
        glVertex2f(x - 4, y + 1);
        glEnd();
    } else if (type == 5) {

        glColor3f(0.55f, 0.4f, 0.3f);
        glBegin(GL_QUADS);
        glVertex2f(x - 6, y - 2);
        glVertex2f(x + 6, y - 2);
        glVertex2f(x + 6, y + 2);
        glVertex2f(x - 6, y + 2);
        glEnd();
    } else if (type == 6) {

        glColor3f(0.55f, 0.55f, 0.6f);
        glBegin(GL_TRIANGLES);
        glVertex2f(x, y + 4);
        glVertex2f(x - 4, y - 3);
        glVertex2f(x + 4, y - 3);
        glEnd();
    } else {

        glColor3f(0.7f, 0.6f, 0.4f);
        glBegin(GL_QUADS);
        glVertex2f(x - 4, y - 4);
        glVertex2f(x + 4, y - 4);
        glVertex2f(x + 4, y + 4);
        glVertex2f(x - 4, y + 4);
        glEnd();
        glColor3f(0.5f, 0.4f, 0.2f);
        glBegin(GL_LINES);
        glVertex2f(x - 4, y);
        glVertex2f(x + 4, y);
        glEnd();
    }
}

void drawFoam(float x, float y, float alphaMul) {
    float a = 0.7f * alphaMul;
    if (a <= 0.01f) return;
    glColor4f(0.9f, 0.9f, 0.85f, a);
    for (int i = 0; i < 4; i++) {
        drawCircle(x + i * 5 - 7, y + sin(i * 1.5f) * 2, 3, 10);
    }
}

void drawFence(float x, float y, float width) {
    glColor3f(0.3f, 0.3f, 0.3f);

    glBegin(GL_QUADS);
    glVertex2f(x, y + 15);
    glVertex2f(x + width, y + 15);
    glVertex2f(x + width, y + 17);
    glVertex2f(x, y + 17);
    glEnd();
    glBegin(GL_QUADS);
    glVertex2f(x, y + 25);
    glVertex2f(x + width, y + 25);
    glVertex2f(x + width, y + 27);
    glVertex2f(x, y + 27);
    glEnd();


    for (int i = 0; i <= 6; i++) {
        glBegin(GL_QUADS);
        glVertex2f(x + i * (width / 6), y);
        glVertex2f(x + i * (width / 6) + 3, y);
        glVertex2f(x + i * (width / 6) + 3, y + 30);
        glVertex2f(x + i * (width / 6), y + 30);
        glEnd();
    }
}

void drawSickAnimal(float x, float y) {

    glColor3f(0.5f, 0.5f, 0.5f);

    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + 12, y);
    glVertex2f(x + 12, y + 6);
    glVertex2f(x, y + 6);
    glEnd();

    drawCircle(x + 12, y + 5, 4, 15);

    glBegin(GL_LINES);
    glVertex2f(x + 2, y);
    glVertex2f(x + 2, y - 4);
    glVertex2f(x + 10, y);
    glVertex2f(x + 10, y - 4);
    glEnd();
}

void drawSun(float x, float y, bool isDimmed) {
    if (isDimmed) {
        glColor4f(0.9f, 0.8f, 0.5f, 0.4f);
    } else {
        glColor3f(1.0f, 0.9f, 0.3f);
    }
    drawCircle(x, y, 25, 30);

    if (isDimmed) {
        glColor4f(0.95f, 0.85f, 0.6f, 0.3f);
    } else {
        glColor3f(1.0f, 0.95f, 0.5f);
    }
    drawCircle(x, y, 20, 30);
}

void drawSteam(float x, float y, float offset) {
    glColor4f(0.95f, 0.95f, 0.95f, 0.5f);
    for (int i = 0; i < 3; i++) {
        float rise = fmod(offset + i * 15, 40);
        drawCircle(x + i * 5 - 5, y + rise, 4 - rise * 0.05f, 15);
    }
}

void drawFish(float x, float y, float scale, bool isDead) {

    float t = (isDead ? clamp01(cleanLevel) : 1.0f);


    float br = mixf(0.60f, 1.00f, t);
    float bg = mixf(0.60f, 0.55f, t);
    float bb = mixf(0.60f, 0.05f, t);

    glColor3f(br, bg, bb);


    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);
    for (int i = 0; i <= 20; i++) {
        float theta = 2.0f * 3.14159f * i / 20;
        float fx = (12 * scale) * cosf(theta);
        float fy = (6 * scale) * sinf(theta);
        glVertex2f(x + fx, y + fy);
    }
    glEnd();


    glColor4f(0.10f, 0.75f, 1.00f, 0.75f * t);
    glBegin(GL_QUADS);
    glVertex2f(x - 6.0f * scale, y - 1.2f * scale);
    glVertex2f(x + 8.0f * scale, y - 1.2f * scale);
    glVertex2f(x + 8.0f * scale, y + 1.2f * scale);
    glVertex2f(x - 6.0f * scale, y + 1.2f * scale);
    glEnd();


    glColor3f(mixf(0.55f, 0.95f, t), mixf(0.55f, 0.45f, t), mixf(0.55f, 0.08f, t));
    glBegin(GL_TRIANGLES);
    glVertex2f(x - 12 * scale, y);
    glVertex2f(x - 20 * scale, y - 6 * scale);
    glVertex2f(x - 20 * scale, y + 6 * scale);
    glEnd();


    glColor4f(0.95f, 0.20f, 0.80f, 0.65f * t);
    glBegin(GL_TRIANGLES);
    glVertex2f(x - 2.0f * scale, y + 5.0f * scale);
    glVertex2f(x + 2.0f * scale, y + 5.0f * scale);
    glVertex2f(x + 0.5f * scale, y + 9.0f * scale);
    glEnd();


    if (isDead && t < 0.35f) {
        glColor3f(0.30f, 0.30f, 0.30f);
        drawCircle(x + 6 * scale, y + 2 * scale, 1.5f * scale, 10);

        glColor4f(0.20f, 0.20f, 0.20f, 0.8f * (1.0f - t));
        glLineWidth(1.5f);
        glBegin(GL_LINES);
        glVertex2f(x + 5.0f * scale, y + 1.0f * scale);
        glVertex2f(x + 7.0f * scale, y + 3.0f * scale);
        glVertex2f(x + 7.0f * scale, y + 1.0f * scale);
        glVertex2f(x + 5.0f * scale, y + 3.0f * scale);
        glEnd();
        glLineWidth(1.0f);
    } else {

        glColor4f(1.0f, 1.0f, 1.0f, 0.85f);
        drawCircle(x + 6 * scale, y + 2 * scale, 2.0f * scale, 12);
        glColor3f(0.0f, 0.0f, 0.0f);
        drawCircle(x + 6.6f * scale, y + 2.0f * scale, 0.8f * scale, 10);
    }
}
void drawFactory(float x, float y) {

    glColor3f(0.35f, 0.35f, 0.35f);
    glBegin(GL_QUADS);
    glVertex2f(x - 5, y - 5);
    glVertex2f(x + 135, y - 5);
    glVertex2f(x + 135, y);
    glVertex2f(x - 5, y);
    glEnd();


    glColor3f(0.55f, 0.45f, 0.42f);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + 130, y);
    glVertex2f(x + 130, y + 90);
    glVertex2f(x, y + 90);
    glEnd();


    glColor3f(0.45f, 0.35f, 0.32f);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + 5, y);
    glVertex2f(x + 5, y + 90);
    glVertex2f(x, y + 90);
    glEnd();
    glBegin(GL_QUADS);
    glVertex2f(x + 125, y);
    glVertex2f(x + 130, y);
    glVertex2f(x + 130, y + 90);
    glVertex2f(x + 125, y + 90);
    glEnd();

    glColor3f(0.6f, 0.6f, 0.6f);
    for (int i = 1; i <= 3; i++) {
        glBegin(GL_QUADS);
        glVertex2f(x, y + i * 22);
        glVertex2f(x + 130, y + i * 22);
        glVertex2f(x + 130, y + i * 22 + 3);
        glVertex2f(x, y + i * 22 + 3);
        glEnd();
    }

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 3; j++) {

            glColor3f(0.3f, 0.3f, 0.3f);
            glBegin(GL_QUADS);
            glVertex2f(x + 15 + i * 28, y + 12 + j * 26);
            glVertex2f(x + 25 + i * 28, y + 12 + j * 26);
            glVertex2f(x + 25 + i * 28, y + 22 + j * 26);
            glVertex2f(x + 15 + i * 28, y + 22 + j * 26);
            glEnd();


            glColor3f(0.6f, 0.7f, 0.8f);
            glBegin(GL_QUADS);
            glVertex2f(x + 16 + i * 28, y + 13 + j * 26);
            glVertex2f(x + 24 + i * 28, y + 13 + j * 26);
            glVertex2f(x + 24 + i * 28, y + 21 + j * 26);
            glVertex2f(x + 16 + i * 28, y + 21 + j * 26);
            glEnd();


            glColor3f(0.25f, 0.25f, 0.25f);
            glBegin(GL_LINES);
            glVertex2f(x + 20 + i * 28, y + 13 + j * 26);
            glVertex2f(x + 20 + i * 28, y + 21 + j * 26);
            glVertex2f(x + 16 + i * 28, y + 17 + j * 26);
            glVertex2f(x + 24 + i * 28, y + 17 + j * 26);
            glEnd();
        }
    }


    glColor3f(0.48f, 0.48f, 0.48f);
    glBegin(GL_QUADS);
    glVertex2f(x + 20, y + 90);
    glVertex2f(x + 70, y + 90);
    glVertex2f(x + 70, y + 105);
    glVertex2f(x + 20, y + 105);
    glEnd();


    glColor3f(0.38f, 0.38f, 0.38f);
    glBegin(GL_QUADS);
    glVertex2f(x + 18, y + 105);
    glVertex2f(x + 72, y + 105);
    glVertex2f(x + 72, y + 108);
    glVertex2f(x + 18, y + 108);
    glEnd();


    glColor3f(0.38f, 0.35f, 0.32f);
    glBegin(GL_QUADS);
    glVertex2f(x + 85, y + 90);
    glVertex2f(x + 108, y + 90);
    glVertex2f(x + 105, y + 150);
    glVertex2f(x + 88, y + 150);
    glEnd();


    glColor3f(0.5f, 0.5f, 0.5f);
    for (int i = 1; i <= 3; i++) {
        glBegin(GL_QUADS);
        glVertex2f(x + 86, y + 90 + i * 18);
        glVertex2f(x + 107, y + 90 + i * 18);
        glVertex2f(x + 107, y + 92 + i * 18);
        glVertex2f(x + 86, y + 92 + i * 18);
        glEnd();
    }


    glColor3f(0.45f, 0.45f, 0.45f);
    glBegin(GL_QUADS);
    glVertex2f(x + 85, y + 150);
    glVertex2f(x + 108, y + 150);
    glVertex2f(x + 110, y + 156);
    glVertex2f(x + 83, y + 156);
    glEnd();

    float smokeFade = 1.0f - clamp01(airCleanLevel);
    if (smokeFade > 0.01f) {

        const float H = 240.0f;
        for (int k = 0; k < 20; k++) {
            float tt = fmodf(smogOffset * 1.20f + k * 21.0f + (k % 4) * 7.0f, H);
            float u  = tt / H;

            float baseX = x + 96.5f;
            float baseY = y + 156.0f;


            float driftX = u * 28.0f;
            float xwig = sinf(smogOffset * 0.020f + k * 1.9f) * (11.0f - 7.0f * u)
                       + cosf(smogOffset * 0.014f + k * 0.8f) * (5.0f - 2.5f * u);

            float yy = baseY + tt;


            float rr = (15.0f - 8.0f * u) + (k % 5) * 0.55f;


            float a = (0.90f * (1.0f - u)) * smokeFade;


            glColor4f(0.08f, 0.08f, 0.08f, 0.85f * a);
            drawCircle(baseX + driftX + xwig, yy, rr, 20);


            glColor4f(0.18f, 0.18f, 0.18f, 0.55f * a);
            drawCircle(baseX + driftX + xwig + 5.0f, yy + 2.0f, rr * 0.96f, 20);
        }
    }


    glColor3f(0.35f, 0.35f, 0.35f);
    glBegin(GL_QUADS);
    glVertex2f(x + 5, y);
    glVertex2f(x + 30, y);
    glVertex2f(x + 30, y + 35);
    glVertex2f(x + 5, y + 35);
    glEnd();


    glColor3f(0.3f, 0.3f, 0.3f);
    for (int i = 0; i < 3; i++) {
        glBegin(GL_LINE_LOOP);
        glVertex2f(x + 8, y + 5 + i * 9);
        glVertex2f(x + 27, y + 5 + i * 9);
        glVertex2f(x + 27, y + 12 + i * 9);
        glVertex2f(x + 8, y + 12 + i * 9);
        glEnd();
    }


    glColor3f(0.35f, 0.35f, 0.35f);
    glBegin(GL_QUADS);
    glVertex2f(x + 130, y + 25);
    glVertex2f(x + 165, y + 25);
    glVertex2f(x + 165, y + 38);
    glVertex2f(x + 130, y + 38);
    glEnd();


    glColor3f(0.4f, 0.4f, 0.4f);
    drawCircle(x + 130, y + 31.5f, 8, 15);
    drawCircle(x + 165, y + 31.5f, 8, 15);
    drawCircle(x + 147, y + 31.5f, 6, 15);


    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_QUADS);
    glVertex2f(x + 145, y + 25);
    glVertex2f(x + 149, y + 25);
    glVertex2f(x + 149, y + 10);
    glVertex2f(x + 145, y + 10);
    glEnd();

    glColor3f(0.9f, 0.8f, 0.0f);
    for (int i = 0; i < 3; i++) {
        glBegin(GL_QUADS);
        glVertex2f(x + 135 + i * 10, y + 27);
        glVertex2f(x + 138 + i * 10, y + 27);
        glVertex2f(x + 138 + i * 10, y + 36);
        glVertex2f(x + 135 + i * 10, y + 36);
        glEnd();
    }
}

void drawWaste(float x, float y, float strength) {
    strength = clamp01(strength);
    if (strength <= 0.01f) return;


    glColor4f(0.4f, 0.8f, 0.2f, strength);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + 15, y);
    glVertex2f(x + 15, y - 25);
    glVertex2f(x, y - 25);
    glEnd();


    glBegin(GL_TRIANGLES);
    glVertex2f(x + 7.5f, y - 25);
    glVertex2f(x - 20, y - 60);
    glVertex2f(x + 35, y - 60);
    glEnd();


    int bubbles = (int)(5 * strength + 0.5f);
    for (int i = 0; i < bubbles; i++) {
        float offset = fmod(wasteOffset + i * 30, 150);
        glColor4f(0.5f, 0.9f, 0.3f, 0.7f * strength);
        drawCircle(x + 7.5f + (i - 2) * 15, y - 30 - offset, 5, 10);
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);


    float tClean = clamp01(cleanLevel);
    float dirt = pollutionStarted ? (1.0f - tClean) : 0.0f;


    glBegin(GL_QUADS);
    glColor3f(0.53f, 0.81f, 0.92f);
    glVertex2f(0, 200);
    glVertex2f(800, 200);
    glColor3f(0.68f, 0.85f, 0.9f);
    glVertex2f(800, 600);
    glVertex2f(0, 600);
    glEnd();


    float air = clamp01(airCleanLevel);
    bool sunDim = pollutionStarted && (air < 0.98f);
    drawSun(120, 520, sunDim);


    float pA = 0.30f * (1.0f - air);
    if (pA > 0.01f) glColor4f(0.4f, 0.4f, 0.4f, pA);
    for (int i = 0; i < 20 && pA > 0.01f; i++) {
        float px = 500 + fmod(i * 25 + particleOffset, 300);
        float py = 350 + i * 15 + sin(particleOffset * 0.02f + i) * 10;
        drawCircle(px, py, 2 + (i % 3), 8);
    }


    drawCloud(100 + cloudOffset, 500, 1.0f);
    drawCloud(300 + cloudOffset * 0.7f, 480, 0.8f);
    drawCloud(500 + cloudOffset * 1.2f, 520, 1.1f);


    if (pollutionStarted) {
        drawDarkCloud(650, 460, 1.3f);
        drawDarkCloud(700, 490, 1.1f);
        drawDarkCloud(740, 470, 1.0f);
        drawDarkCloud(680, 510, 0.9f);
    }


    float hazeA = 0.35f * (1.0f - air);
    if (hazeA > 0.01f) {
        glColor4f(0.25f, 0.25f, 0.25f, hazeA);
        glBegin(GL_QUADS);
    glVertex2f(550, 300);
    glVertex2f(800, 300);
    glColor4f(0.25f, 0.25f, 0.25f, 0.0f);
    glVertex2f(800, 420);
        glVertex2f(550, 420);
        glEnd();
    }

    glBegin(GL_QUADS);

    glColor3f(mixf(0.15f, 0.2f, tClean), mixf(0.3f, 0.4f, tClean), mixf(0.4f, 0.7f, tClean));
    glVertex2f(500, 0);
    glVertex2f(800, 0);
    glColor3f(mixf(0.25f, 0.25f, tClean), mixf(0.4f, 0.4f, tClean), mixf(0.5f, 0.5f, tClean));
    glVertex2f(800, 200);
    glVertex2f(500, 200);
    glEnd();


    glBegin(GL_QUADS);
    glColor3f(0.2f, 0.4f, 0.7f);
    glVertex2f(0, 0);
    glVertex2f(500, 0);
    glColor3f(0.25f, 0.4f, 0.5f);
    glVertex2f(500, 200);
    glVertex2f(0, 200);
    glEnd();


    glColor4f(0.3f, 0.5f, 0.2f, 0.4f * dirt);
    for (int i = 0; i < 6; i++) {
        float px = 550 + i * 40 + sin(wasteOffset * 0.02f + i) * 20;
        float py = 80 + i * 20 + cos(wasteOffset * 0.03f + i) * 15;
        float pr = (15 + i * 2) * dirt;
        if (pr > 0.5f) drawCircle(px, py, pr, 15);
    }


    int foamCount = (int)(8 * dirt + 0.5f);
    for (int i = 0; i < foamCount; i++) {
        float fx = 560 + i * 30;
        float fy = 60 + i * 15 + sin(wasteOffset * 0.03f + i) * 5;
        drawFoam(fx, fy, dirt);
    }


    int trashCount = (int)(20 * dirt + 0.5f);
    for (int i = 0; i < trashCount; i++) {
        float tx = 520 + (i * 35 + trashOffset);
        if (tx > 800) tx -= 280;
        float ty = 40 + (i * 8) % 160 + sin(trashOffset * 0.02f + i) * 6;
        if (ty < 200) drawTrash(tx, ty, i % 8);
    }


    glColor3f(0.35f, 0.5f, 0.6f);
    glLineWidth(2);
    for (int i = 0; i < 4; i++) {
        glBegin(GL_LINE_STRIP);
        for (float x = 0; x < 800; x += 10) {
            float y = 50 + i * 40 + 5 * sin((x + wasteOffset) * 0.05f);
            glVertex2f(x, y);
        }
        glEnd();
    }


    if (showBoat) {

        float bx = 20.0f + boatOffset;
        float by = 155.0f
                 + 4.0f * sin((boatOffset + wasteOffset) * 0.03f)
                 + 3.0f * sin((bx + wasteOffset) * 0.05f);
        drawBoat(bx, by, BOAT_SCALE);

        if (cleaning) {
            float s = BOAT_SCALE;
            float nx = bx + 62.0f * s;
            float ny = by - 28.0f * s;


            glColor4f(0.75f, 0.92f, 1.0f, 0.18f);
            glBegin(GL_TRIANGLES);
            glVertex2f(nx + 6 * s, ny);
            glVertex2f(nx - 26 * s, ny + 16 * s);
            glVertex2f(nx - 26 * s, ny - 22 * s);
            glEnd();

            glColor4f(0.85f, 0.97f, 1.0f, 0.65f);
            for (int i = 0; i < 18; i++) {
                float tt = fmod((wasteOffset * 0.02f) + i * 0.08f, 1.0f);
                float r  = (1.0f - tt) * (18.0f * s);
                float a  = (wasteOffset * 2.6f + i * 40.0f) * 3.14159f / 180.0f;
                float px = nx - tt * (16.0f * s) + r * cosf(a);
                float py = ny + (r * 0.55f) * sinf(a);
                drawCircle(px, py, (1.6f + (i % 3) * 0.6f) * s, 10);
            }
        }
    }


    drawFish(650, 120 + sin(fishOffset * 0.05f) * 5, 0.7f, true);
    drawFish(700, 80 + cos(fishOffset * 0.04f) * 4, 0.65f, true);
    drawFish(620, 150 + sin(fishOffset * 0.06f + 1) * 6, 0.6f, true);
    drawFish(740, 130 + cos(fishOffset * 0.055f + 2) * 5, 0.72f, true);
    drawFish(680, 100 + sin(fishOffset * 0.045f + 3) * 4, 0.62f, true);
    drawFish(710, 160 + sin(fishOffset * 0.05f + 4) * 5, 0.68f, true);
    drawFish(590, 90 + cos(fishOffset * 0.048f + 5) * 4, 0.63f, true);


    drawFish(150 - fishOffset * 0.3f, 120 + sin(fishOffset * 0.1f) * 8, 0.65f, false);
    drawFish(300 - fishOffset * 0.25f, 90 + cos(fishOffset * 0.12f) * 10, 0.7f, false);
    drawFish(200 - fishOffset * 0.35f, 160 + sin(fishOffset * 0.11f + 1) * 9, 0.62f, false);

    float airT = smoothstep(0.05f, 1.0f, clamp01(airCleanLevel));
    float soilR = mixf(0.50f, 0.30f, airT);
    float soilG = mixf(0.40f, 0.65f, airT);
    float soilB = mixf(0.30f, 0.25f, airT);

    glBegin(GL_QUADS);
    glColor3f(soilR, soilG, soilB);
    glVertex2f(500, 200);
    glVertex2f(800, 200);

    glColor3f(0.40f, 0.60f, 0.30f);
    glVertex2f(800, 300);
    glVertex2f(500, 300);
    glEnd();

    glBegin(GL_QUADS);
    glColor3f(0.3f, 0.65f, 0.25f);
    glVertex2f(0, 200);
    glVertex2f(500, 200);
    glColor3f(0.4f, 0.6f, 0.3f);
    glVertex2f(500, 300);
    glVertex2f(0, 300);
    glEnd();

    drawTree(540, 200, true);
    drawTree(570, 200, true);
    drawTree(760, 200, true);
    drawTree(790, 200, true);


    drawTree(80, 200, false);
    drawTree(120, 200, false);
    drawTree(200, 200, false);
    drawTree(280, 200, false);
    drawTree(360, 200, false);
    drawTree(440, 200, false);

    bool showPollutionProps = pollutionStarted && (dirt > 0.05f || air < 0.95f);


    if (showPollutionProps) {
        drawSickAnimal(520, 203);

        drawSign(460, 200, 0);
        drawSign(360, 200, 1);
        drawSign(280, 200, 2);
    }


    drawFence(520, 200, 60);


    drawFactory(580, 240);

    if (showAirCleaner) {
        drawAirCleanerTruck(airTruckX, AIR_TRUCK_GROUND_Y, AIR_TRUCK_SCALE, airCleaningActive);
    }

    if (showPollutionProps) {
        drawPerson(560, 200, true);
        drawPerson(720, 200, true);
        drawPerson(690, 200, true);
    }

    drawWaste(745, 272, dirt);

    if (dirt > 0.2f) {
        drawSteam(750, 215, wasteOffset);
        drawSteam(762, 215, wasteOffset + 10);
    }

    if (pollutionStarted && air < 0.95f) {
        for (int i = 0; i < 8; i++) {
        float bx = 450 - (birdOffset * 0.5f + i * 40);
        if (bx < -20) bx += 500;
        float by = 420 + i * 15 + sin(birdOffset * 0.05f + i) * 10;
        drawBird(bx, by);
    }

    }

    glutSwapBuffers();
}

void timer(int value) {
    wasteOffset += 2.0f;
    if (wasteOffset > 150) wasteOffset = 0;

    cloudOffset += 0.3f;
    if (cloudOffset > 800) cloudOffset = -200;

    fishOffset += 1.0f;
    if (fishOffset > 1000) fishOffset = 0;

    smogOffset += 1.5f;
    if (smogOffset > 1000) smogOffset = 0;

    trashOffset += 0.5f;
    if (trashOffset > 500) trashOffset = 0;

    birdOffset += 1.2f;
    if (birdOffset > 600) birdOffset = 0;

    particleOffset += 0.8f;
    if (particleOffset > 500) particleOffset = 0;


    if (showBoat) {

        float maxOffset = boatMaxOffset();
        if (boatOffset < maxOffset) {
            boatOffset += 1.0f;
            if (boatOffset > maxOffset) boatOffset = maxOffset;
        } else {
            boatOffset = maxOffset;
        }
    }




    if (showBoat) {
        beltPhase += cleaning ? 2.2f : 0.6f;
        if (beltPhase > 10000.0f) beltPhase = 0.0f;
        brushAngle += cleaning ? 12.0f : 2.0f;
        if (brushAngle > 360.0f) brushAngle -= 360.0f;
    }

    if (showAirCleaner) {

        if (!airTruckParked) {
            airTruckX += 2.6f;
            if (airTruckX >= AIR_TRUCK_STOP_X) {
                airTruckX = AIR_TRUCK_STOP_X;
                airTruckParked = true;
            }
        }


        bool airCleaningWorking = airTruckParked && airCleaningArmed && (!airCleanLocked);


        airCleaningActive = airTruckParked && ((airCleaningArmed && !airCleanLocked) || (airPostSprayFrames > 0));

        airFlowPhase += airCleaningActive ? 3.0f : (airTruckParked ? 1.8f : 1.2f);
        airFanAngle  += airCleaningActive ? 18.0f : (airTruckParked ? 12.0f : 7.0f);
        if (airFanAngle > 360.0f) airFanAngle -= 360.0f;


        if (airCleaningWorking) {
            airCleanLevel += AIR_CLEAN_RATE;
            if (airCleanLevel >= 1.0f) {
                airCleanLevel = 1.0f;
                airCleanLocked = true;
                airCleaningArmed = false;


                airPostSprayFrames = AIR_POST_SPRAY_FRAMES;
            }
        }

        if (airCleanLocked && airPostSprayFrames > 0) {
            airPostSprayFrames--;
            if (airPostSprayFrames == 0) {
                showAirCleaner = false;
                airTruckParked = false;
                airTruckX = -300.0f;
                airCleaningActive = false;
            }
        }

    } else {

        airCleaningActive = false;
        airTruckParked = false;
        airTruckX = -300.0f;
        airCleaningArmed = false;
        airPostSprayFrames = 0;

        if (!airCleanLocked) {
            airCleanLevel -= 0.01f;
            if (airCleanLevel < 0.0f) airCleanLevel = 0.0f;
        } else {
            airCleanLevel = 1.0f;
        }
    }

    if (cleaning && cleanLevel < 1.0f) {
        cleanLevel += 0.005f;
        if (cleanLevel >= 1.0f) {
            cleanLevel = 1.0f;
            cleaning = false;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}


void keyboard(unsigned char key, int, int) {
    switch (key) {
        case 'b':
        case 'B':
            showBoat = !showBoat;
            if (showBoat) {
                boatOffset = 0.0f;
                brushAngle = 0.0f;
                beltPhase  = 0.0f;
            } else {
                cleaning = false;
            }
            break;
        case 'c':
        case 'C':

            if (showBoat) {
                float maxOffset = boatMaxOffset();
                if (boatOffset >= maxOffset - 0.5f && cleanLevel < 1.0f) {
                    cleaning = !cleaning;
                }
            }
            break;
        case 'm':
        case 'M':
            showAirCleaner = !showAirCleaner;
            if (showAirCleaner) {
                airFanAngle = 0.0f;
                airFlowPhase = 0.0f;
                if (!airCleanLocked) airCleanLevel = 0.0f; else airCleanLevel = 1.0f;
                airTruckX = -300.0f;
                airTruckParked = false;
                airCleaningActive = false;
                airCleaningArmed  = false;
                airPostSprayFrames = 0;
            }
            break;
        case 's':
        case 'S':

            pollutionStarted = true;


            cleaning = false;
            cleanLevel = 0.0f;
            showBoat = false;
            boatOffset = 0.0f;


            showAirCleaner = false;
            airTruckX = -300.0f;
            airTruckParked = false;
            airCleaningArmed = false;
            airCleaningActive = false;
            airPostSprayFrames = 0;
            airCleanLocked = false;
            airCleanLevel = 0.0f;
            break;

        case 'q':
        case 'Q':

            if (showAirCleaner && !airCleanLocked) {
                airCleaningArmed = !airCleaningArmed;
                if (!airCleaningArmed) airPostSprayFrames = 0;
                airCleaningActive = airTruckParked && airCleaningArmed;
                if (airCleaningActive && airCleanLevel < 0.01f) airCleanLevel = 0.01f;
            }
            break;
        case 27:
            exit(0);
            break;
        default:
            break;
    }
    glutPostRedisplay();
}

void init() {
    wasteOffset = 0.0;
    cloudOffset = 0.0;
    fishOffset = 0.0;
    smogOffset = 0.0;
    trashOffset = 0.0;
    birdOffset = 0.0;
    particleOffset = 0.0;
    boatOffset = 0.0;
    showBoat = false;
    cleaning = false;

    pollutionStarted = false;
    cleanLevel = 1.0f;

    showAirCleaner = false;
    airCleanLocked = true;
    airCleanLevel = 1.0f;
    airFanAngle = 0.0f;
    airFlowPhase = 0.0f;
    airTruckX = -300.0f;
    airTruckParked = false;
    airCleaningArmed = false;
    airCleaningActive = false;
    airCleanLocked = true;
    airPostSprayFrames = 0;

    brushAngle = 0.0f;
    beltPhase  = 0.0f;
    glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 600);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Factory Pollution Scene");

    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(0, timer, 0);

    glutMainLoop();
    return 0;
}
