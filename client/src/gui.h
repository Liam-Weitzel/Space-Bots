/*******************************************************************************************
*
*   Gui v1.0.0 - game
*
*   MODULE USAGE:
*       #define GUI_GUI_IMPLEMENTATION
*       #include "gui_gui.h"
*
*       INIT: GuiGuiState state = InitGuiGui();
*       DRAW: GuiGui(&state);
*
*   LICENSE: Propietary License
*
*   Copyright (c) 2022 Liam Weitzel. All Rights Reserved.
*
*   Unauthorized copying of this file, via any medium is strictly prohibited
*   This project is proprietary and confidential unless the owner allows
*   usage in any other form by expresely written permission.
*
**********************************************************************************************/
#include "raylib.h"

// WARNING: raygui implementation is expected to be defined before including this header
#undef RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "rres.h"
#include "rres-raylib.h"

#include <string.h>     // Required for: strcpy()

#ifndef GUI_GUI_H
#define GUI_GUI_H

struct GUI; //forward declared

typedef struct {
    // Define anchors
    Vector2 anchor01;            // ANCHOR ID:1
    Vector2 anchor02;            // ANCHOR ID:2
    
    // Define controls variables
    bool WindowBox001Active;            // WindowBox: WindowBox001
    int ToggleGroup001Active;            // ToggleGroup: ToggleGroup001
    bool CheckBoxEx002Checked;            // CheckBoxEx: CheckBoxEx002
    bool Toggle003Active;            // Toggle: Toggle003

    // Define rectangles
    Rectangle layoutRecs[5];

    // Custom state variables (depend on development software)
    // NOTE: This variables should be added manually if required

} GuiGuiState;

#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// ...

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
GuiGuiState InitGuiGui();
void GuiGui(GUI* gui, rresCentralDir* dir);
static const char* styles[] = {
    "default",
    "ash.rgs",
    "bluish.rgs",
    "candy.rgs",
    "cherry.rgs",
    "cyber.rgs",
    "dark.rgs",
    "enefete.rgs",
    "jungle.rgs",
    "lavanda.rgs",
    "sunny.rgs",
    "terminal.rgs"
};
static int loaded_style = 0;

#ifdef __cplusplus
}
#endif

#endif // GUI_GUI_H

/***********************************************************************************
*
*   GUI_GUI IMPLEMENTATION
*
************************************************************************************/
#if defined(GUI_GUI_IMPLEMENTATION)

#include "raygui.h"

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Internal Module Functions Definition
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
GuiGuiState InitGuiGui()
{
    GuiGuiState state = { 0 };

    // Init anchors
    state.anchor01 = Vector2{ 24, 24 };            // ANCHOR ID:1
    state.anchor02 = Vector2{ 24, 24 };            // ANCHOR ID:2
    
    // Initilize controls variables
    state.WindowBox001Active = true;            // WindowBox: WindowBox001
    state.ToggleGroup001Active = 0;            // ToggleGroup: ToggleGroup001
    state.CheckBoxEx002Checked = false;            // CheckBoxEx: CheckBoxEx002
    state.Toggle003Active = true;            // Toggle: Toggle003

    // Init controls rectangles
    state.layoutRecs[0] = Rectangle{ state.anchor02.x + 72, state.anchor02.y + 40, 160, 160 };// WindowBox: WindowBox001
    state.layoutRecs[1] = Rectangle{ state.anchor02.x + 80, state.anchor02.y + 72, 40, 24 };// ToggleGroup: ToggleGroup001
    state.layoutRecs[2] = Rectangle{ state.anchor02.x + 80, state.anchor02.y + 104, 24, 24 };// CheckBoxEx: CheckBoxEx002
    state.layoutRecs[3] = Rectangle{ state.anchor02.x + 80, state.anchor02.y + 136, 88, 24 };// Toggle: Toggle003
    state.layoutRecs[4] = Rectangle{ state.anchor02.x + 80, state.anchor02.y + 168, 120, 24 };// Button: Button004

    // Custom variables initialization

    return state;
}

static void Button004(uint8_t& style) {
    style = (style + 1) % 12;
}

void GuiGui(GUI* gui, rresCentralDir* dir)
{
    if(loaded_style != gui->style) {
        if (strcmp(styles[gui->style], "default") == 0) GuiLoadStyleDefault();
        else {
            int idStyle = rresGetResourceId(*dir, styles[gui->style]);
            rresResourceChunk chunkStyle = rresLoadResourceChunk("resources.rres", idStyle);
            if(UnpackResourceChunk(&chunkStyle) == RRES_SUCCESS) {
                GuiLoadStyleFromMemory((const unsigned char*) chunkStyle.data.raw, chunkStyle.info.baseSize);
            }
            rresUnloadResourceChunk(chunkStyle);
        }
        loaded_style = gui->style;
    }

    GuiGuiState* state = &gui->gui_state;
    // Const text
    const char *WindowBox001Text = "SAMPLE TEXT";    // WINDOWBOX: WindowBox001
    const char *ToggleGroup001Text = "ONE;TWO;THREE";    // TOGGLEGROUP: ToggleGroup001
    const char *CheckBoxEx002Text = "SAMPLE TEXT";    // CHECKBOXEX: CheckBoxEx002
    const char *Toggle003Text = "SAMPLE TEXT";    // TOGGLE: Toggle003
    const char *Button004Text = "Switch Style";    // BUTTON: Button004
    
    // Draw controls
    if (state->WindowBox001Active)
    {
        state->WindowBox001Active = !GuiWindowBox(state->layoutRecs[0], WindowBox001Text);
        GuiToggleGroup(state->layoutRecs[1], ToggleGroup001Text, &state->ToggleGroup001Active);
        GuiCheckBox(state->layoutRecs[2], CheckBoxEx002Text, &state->CheckBoxEx002Checked);
        GuiToggle(state->layoutRecs[3], Toggle003Text, &state->Toggle003Active);
        if (GuiButton(state->layoutRecs[4], Button004Text)) Button004(gui->style); 
    }
}

#endif // GUI_GUI_IMPLEMENTATION
