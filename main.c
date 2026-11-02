/*******************************************************************************************
*
*   Cubing - A rubik's cube solving timer
*
*   Based on the raylib [shapes] example "digital clock"
*   Copyright (c) 2025 Hamza RAHAL (@hmz-rhl) and Ramon Santamaria (@raysan5)
*   Modifications Copyright (c) 2026 Bentor (@bentxr)
*
*   SPDX-License-Identifier: Zlib
*   Full license text in LICENSE
*
********************************************************************************************/

#include "raylib.h"
#include <stdlib.h>
#include <time.h>

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

#define STOPPED 0
#define RUNNING 1

#define HIDDEN 1
#define SHOWING 0


//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------


typedef struct {

    int centisecond;                // centiseconds
    int second;                     // seconds
    int minute;                     // minutes
    bool state;                     // RUNNING or STOPPED
    bool hidden;                    // HIDDEN or SHOWING 
    float centisecondAccumulator;    // decimal part acumulator
    Color backgroundColor;
    Color timerColor;
    Color textColor;

} Timer;

typedef struct {

    char scramble[40];     // cube scramble
    bool hidden;        // HIDDEN or SHOWING

} Scramble;

typedef struct {

    Timer timer;
    Scramble scramble;

} State;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------

// Updates frame (needs the arg to be a void pointer for emscripten_set_main_loop_arg )
static void UpdateFrame(void *void_state);

// Updates timer
static void UpdateTimer(Timer *timer);

// Resets the timer
static void ResetTimer(Timer *timer); 

// Draw digital clock at desired position
static void DrawClockDigital(Timer timer, Vector2 position); 

// Selects the case to be drawn
static void DrawDisplayValue(Vector2 position, int value, Color colorOn, Color colorOff); 

// Draw seven segments display
static void Draw7SDisplay(Vector2 position, char segments, Color colorOn, Color colorOff); 

// Draw one 7-segment display segment, horizontal or vertical
static void DrawDisplaySegment(Vector2 center, int length, int thick, bool vertical, Color color); 

// Creates a valid scramble to show
static void scrambleCube(Scramble *scramble, int num_moves); 

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------

int main(void)
{
    //--------------------------------------------------------------------------------------
    // Setup
    //--------------------------------------------------------------------------------------

    Timer timer = {

        .centisecond = 0,
        .second = 0,
        .minute = 0,
        .centisecondAccumulator = 0,
        .state = STOPPED,
        .hidden = SHOWING,
        .backgroundColor = {0x11, 0x14, 0x18, 0xff},
        .textColor = {0x91, 0x91, 0x91, 0xff},
        .timerColor = {0xcc, 0xcc, 0xcc, 0xff}

    };

    Scramble scramble = {

        .hidden = HIDDEN

    };

    // Whole state needs to be carried on a single arg so that emscripten_set_main_loop_arg can be used
    State state = { 

        .timer = timer,
        .scramble = scramble

    };
    
    // Seeds the random number generator with the current time
    srand(time(NULL));

    // Define and Init the window
    const int screenWidth = 800;
    const int screenHeight = 450;


    InitWindow(screenWidth, screenHeight, "cubing timer");

    SetTargetFPS(60);

    //--------------------------------------------------------------------------------------
    // Main loop
    //--------------------------------------------------------------------------------------

    #if defined(PLATFORM_WEB)
        emscripten_set_main_loop_arg(UpdateFrame, &state, 0, 1);
    #else
        SetTargetFPS(60); 
        while (!WindowShouldClose()) UpdateFrame(&state); // Detect window close button or ESC key
    #endif 

    //--------------------------------------------------------------------------------------
    // De-Initialization
    //--------------------------------------------------------------------------------------

    CloseWindow(); // Close window and OpenGL context

    return 0;
}

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

// Update frame
static void UpdateFrame(void *void_state) {

    State *state = (State *)void_state;
    
    //----------------------------------------------------------------------------------
    // Listen for key presses and update variables
    //----------------------------------------------------------------------------------

    if (IsKeyPressed(KEY_SPACE)) state->timer.state = !state->timer.state;

    if (state->timer.state) UpdateTimer(&state->timer); // Update timer required data

    if (IsKeyPressed(KEY_R)) ResetTimer(&state->timer);

    if (IsKeyPressed(KEY_H)) state->timer.hidden = !state->timer.hidden;

    if (IsKeyPressed(KEY_S)) {
        
        state->scramble.hidden = !state->scramble.hidden;
        if (!state->scramble.hidden) scrambleCube(&state->scramble, 20);
    
    }
    
    //----------------------------------------------------------------------------------
    // Draw
    //----------------------------------------------------------------------------------
    
    BeginDrawing();

        ClearBackground(state->timer.backgroundColor);

        DrawText(TextFormat("Press [SPACE] to %s", (state->timer.state == RUNNING)? "stop" : "continue"), 10, 10, 20, state->timer.textColor);
        
        DrawText(TextFormat("Press [R] to reset"), 10, 35, 20, state->timer.textColor);

        DrawText(TextFormat("Press [H] to %s the timer", (state->timer.hidden) ? "show" : "hide"), 10, 60, 20, state->timer.textColor);

        DrawText(TextFormat("Press [S] to %s", (!state->scramble.hidden) ? "hide the scramble" : "show a new scramble"), 10, 85, 20, state->timer.textColor);

        const char *timerHiddenText = TextFormat("TIMER HIDDEN");
        if (state->timer.state == RUNNING && state->timer.hidden) DrawText(timerHiddenText, GetScreenWidth()/2 - MeasureText(timerHiddenText, 50)/2, GetScreenHeight()/2, 50, state->timer.textColor);
        else DrawClockDigital(state->timer, (Vector2){ 30, 180 });

        if (!state->scramble.hidden) {
            const char *scrambleText = TextFormat("%s", state->scramble.scramble);
            DrawText(scrambleText, GetScreenWidth()/2 - MeasureText(scrambleText, 30)/2, 120, 30, state->timer.textColor);
        }

    EndDrawing();

}

// Update timer
static void UpdateTimer(Timer *timer) {

    float frameTime = GetFrameTime(); // Duration of the last frame in seconds

    float deltaCentisecond = 100*frameTime;
    timer->centisecondAccumulator += deltaCentisecond;

    // Update centiseconds
    if (timer->centisecondAccumulator > 1){
        timer->centisecond++;
        timer->centisecondAccumulator = 0;
    }

    // Update seconds
    if (timer->centisecond + deltaCentisecond >= 100) {
        timer->second++;
        timer->centisecond += deltaCentisecond - 100;
    }

    // Update minutes
    if (timer->second >= 60) {
        timer->minute++;
        timer->second -= 60;
    }

    // Reset timer
    if (timer->minute >= 60) {
        ResetTimer(timer);
    }

}

// Reset timer
static void ResetTimer(Timer *timer) {

    timer->minute = 0;
    timer->second = 0;
    timer->centisecond = 0;
    timer->centisecondAccumulator = 0;
    timer->state = STOPPED;
}

// Draw digital clock at desired position
// PARAM: position, refers to top-left corner
static void DrawClockDigital(Timer timer, Vector2 position) {

    // Draw clock using custom 7-segments display (made of shapes)
    DrawDisplayValue((Vector2){ position.x, position.y }, timer.minute/10, timer.timerColor, timer.backgroundColor); // Alternative for colorOff: Fade(LIGHTGRAY, 0.3f)
    DrawDisplayValue((Vector2){ position.x + 120, position.y }, timer.minute%10, timer.timerColor, timer.backgroundColor);

    DrawCircle((int)position.x + 240, (int)position.y + 70, 12, timer.timerColor);
    DrawCircle((int)position.x + 240, (int)position.y + 150, 12, timer.timerColor);

    DrawDisplayValue((Vector2){ position.x + 260, position.y }, timer.second/10, timer.timerColor, timer.backgroundColor);
    DrawDisplayValue((Vector2){ position.x + 380, position.y }, timer.second%10, timer.timerColor, timer.backgroundColor);

    DrawCircle((int)position.x + 500, (int)position.y + 70, 12,  timer.timerColor);
    DrawCircle((int)position.x + 500, (int)position.y + 150, 12, timer.timerColor);

    DrawDisplayValue((Vector2){ position.x + 520, position.y }, timer.centisecond/10, timer.timerColor, timer.backgroundColor);
    DrawDisplayValue((Vector2){ position.x + 640, position.y }, timer.centisecond%10, timer.timerColor, timer.backgroundColor);

}

// Selects the case to be drawn
static void DrawDisplayValue(Vector2 position, int value, Color colorOn, Color colorOff) {

    switch (value) {
        case 0: Draw7SDisplay(position, 0b00111111, colorOn, colorOff); break;
        case 1: Draw7SDisplay(position, 0b00000110, colorOn, colorOff); break;
        case 2: Draw7SDisplay(position, 0b01011011, colorOn, colorOff); break;
        case 3: Draw7SDisplay(position, 0b01001111, colorOn, colorOff); break;
        case 4: Draw7SDisplay(position, 0b01100110, colorOn, colorOff); break;
        case 5: Draw7SDisplay(position, 0b01101101, colorOn, colorOff); break;
        case 6: Draw7SDisplay(position, 0b01111101, colorOn, colorOff); break;
        case 7: Draw7SDisplay(position, 0b00000111, colorOn, colorOff); break;
        case 8: Draw7SDisplay(position, 0b01111111, colorOn, colorOff); break;
        case 9: Draw7SDisplay(position, 0b01101111, colorOn, colorOff); break;
        default: break;
    }

}

// Draw seven segments display
// Parameter: position, refers to top-left corner of display
// Parameter: segments, defines in binary the segments to be activated
static void Draw7SDisplay(Vector2 position, char segments, Color colorOn, Color colorOff) {

    int segmentLen = 60;
    int segmentThick = 20;
    float offsetYAdjust = segmentThick*0.3f; // HACK: Adjust gap space between segment limits

    // Segment A
    DrawDisplaySegment((Vector2){ position.x + segmentThick + segmentLen/2.0f, position.y + segmentThick },
        segmentLen, segmentThick, false, (segments & 0b00000001)? colorOn : colorOff);
    // Segment B
    DrawDisplaySegment((Vector2){ position.x + segmentThick + segmentLen + segmentThick/2.0f, position.y + 2*segmentThick + segmentLen/2.0f - offsetYAdjust },
        segmentLen, segmentThick, true, (segments & 0b00000010)? colorOn : colorOff);
    // Segment C
    DrawDisplaySegment((Vector2){ position.x + segmentThick + segmentLen + segmentThick/2.0f, position.y + 4*segmentThick + segmentLen + segmentLen/2.0f - 3*offsetYAdjust },
        segmentLen, segmentThick, true, (segments & 0b00000100)? colorOn : colorOff);
    // Segment D
    DrawDisplaySegment((Vector2){ position.x + segmentThick + segmentLen/2.0f, position.y + 5*segmentThick + 2*segmentLen - 4*offsetYAdjust },
        segmentLen, segmentThick, false, (segments & 0b00001000)? colorOn : colorOff);
    // Segment E
    DrawDisplaySegment((Vector2){ position.x + segmentThick/2.0f, position.y + 4*segmentThick + segmentLen + segmentLen/2.0f - 3*offsetYAdjust },
        segmentLen, segmentThick, true, (segments & 0b00010000)? colorOn : colorOff);
    // Segment F
    DrawDisplaySegment((Vector2){ position.x + segmentThick/2.0f, position.y + 2*segmentThick + segmentLen/2.0f - offsetYAdjust },
        segmentLen, segmentThick, true, (segments & 0b00100000)? colorOn : colorOff);
    // Segment G
    DrawDisplaySegment((Vector2){ position.x + segmentThick + segmentLen/2.0f, position.y + 3*segmentThick + segmentLen - 2*offsetYAdjust },
        segmentLen, segmentThick, false, (segments & 0b01000000)? colorOn : colorOff);

    }

// Draw one 7-segment display segment, horizontal or vertical
static void DrawDisplaySegment(Vector2 center, int length, int thick, bool vertical, Color color) {

    if (!vertical) {
        // Horizontal segment points
        /*
             3___________________________5
            /                             \
           /1             x               6\
           \                               /
            \2___________________________4/
        */
        Vector2 segmentPointsH[6] = {
            (Vector2){ center.x - length/2.0f - thick/2.0f,  center.y },  // Point 1
            (Vector2){ center.x - length/2.0f,  center.y + thick/2.0f },  // Point 2
            (Vector2){ center.x - length/2.0f, center.y - thick/2.0f },   // Point 3 
            (Vector2){ center.x + length/2.0f,  center.y + thick/2.0f },  // Point 4
            (Vector2){ center.x + length/2.0f,  center.y - thick/2.0f },  // Point 5
            (Vector2){ center.x + length/2.0f + thick/2.0f,  center.y },  // Point 6
        };

        DrawTriangleStrip(segmentPointsH, 6, color);
    }
    
    else {
        // Vertical segment points
        Vector2 segmentPointsV[6] = {
            (Vector2){ center.x,  center.y - length/2.0f - thick/2.0f },  // Point 1
            (Vector2){ center.x - thick/2.0f,  center.y - length/2.0f },  // Point 2
            (Vector2){ center.x + thick/2.0f, center.y - length/2.0f },   // Point 3
            (Vector2){ center.x - thick/2.0f,  center.y + length/2.0f },  // Point 4
            (Vector2){ center.x + thick/2.0f,  center.y + length/2.0f },  // Point 5
            (Vector2){ center.x,  center.y + (float)length/2 + thick/2.0f },  // Point 6
        };

        DrawTriangleStrip(segmentPointsV, 6, color);
    }

}

// Creates a valid scramble to show
static void scrambleCube(Scramble *scramble, int num_moves) {

    char *valid_moves = "FURLBD";
    int index = 0;
    int current_move = 0;
    while (current_move < num_moves) {
        scramble->scramble[index++] = valid_moves[rand()%6];
        if (rand()%2) scramble->scramble[index++] = '\'';
        current_move++;
    }
    scramble->scramble[index] = '\0';

}