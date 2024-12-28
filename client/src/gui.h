/*******************************************************************************************
*
*   Gui v1.0.0 - Tool Description
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
#include <iostream>

// WARNING: raygui implementation is expected to be defined before including this header
#undef RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include <string.h>     // Required for: strcpy()

#ifndef GUI_GUI_H
#define GUI_GUI_H

typedef struct {
    // Define anchors
    
    // Define controls variables

    // Define rectangles
    Rectangle layoutRecs[8];

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
GuiGuiState InitGuiGui(void);
void GuiGui(GuiGuiState *state);
static void Button000();                // Button: Button000 logic
static void Button001();                // Button: Button001 logic
static void Button002();                // Button: Button002 logic
static void Button003();                // Button: Button003 logic
static void Button004();                // Button: Button004 logic
static void Button005();                // Button: Button005 logic
static void Button006();                // Button: Button006 logic
static void Button007();                // Button: Button007 logic

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
GuiGuiState InitGuiGui(void)
{
    GuiGuiState state = { 0 };

    // Init anchors
    
    // Initilize controls variables

    // Init controls rectangles
    state.layoutRecs[0] = (Rectangle){ 24, 24, 120, 24 };// Button: Button000
    state.layoutRecs[1] = (Rectangle){ 24, 72, 120, 24 };// Button: Button001
    state.layoutRecs[2] = (Rectangle){ 24, 120, 120, 24 };// Button: Button002
    state.layoutRecs[3] = (Rectangle){ 24, 168, 120, 24 };// Button: Button003
    state.layoutRecs[4] = (Rectangle){ 24, 216, 120, 24 };// Button: Button004
    state.layoutRecs[5] = (Rectangle){ 24, 264, 120, 24 };// Button: Button005
    state.layoutRecs[6] = (Rectangle){ 24, 312, 120, 24 };// Button: Button006
    state.layoutRecs[7] = (Rectangle){ 24, 360, 120, 24 };// Button: Button007

    // Custom variables initialization

    return state;
}
// Button: Button000 logic
static void Button000()
{
    std::cout << "Button 000 was pressed!" << std::endl;
    // TODO: Implement control logic
}
// Button: Button001 logic
static void Button001()
{
    // TODO: Implement control logic
}
// Button: Button002 logic
static void Button002()
{
    // TODO: Implement control logic
}
// Button: Button003 logic
static void Button003()
{
    // TODO: Implement control logic
}
// Button: Button004 logic
static void Button004()
{
    // TODO: Implement control logic
}
// Button: Button005 logic
static void Button005()
{
    // TODO: Implement control logic
}
// Button: Button006 logic
static void Button006()
{
    // TODO: Implement control logic
}
// Button: Button007 logic
static void Button007()
{
    // TODO: Implement control logic
}


void GuiGui(GuiGuiState *state)
{
    // Const text
    const char *Button000Text = "SAMPLE TEXT";    // BUTTON: Button000
    const char *Button001Text = "SAMPLE TEXT";    // BUTTON: Button001
    const char *Button002Text = "SAMPLE TEXT";    // BUTTON: Button002
    const char *Button003Text = "SAMPLE TEXT";    // BUTTON: Button003
    const char *Button004Text = "SAMPLE TEXT";    // BUTTON: Button004
    const char *Button005Text = "SAMPLE TEXT";    // BUTTON: Button005
    const char *Button006Text = "SAMPLE TEXT";    // BUTTON: Button006
    const char *Button007Text = "SAMPLE TEXT";    // BUTTON: Button007
    
    // Draw controls
    if (GuiButton(state->layoutRecs[0], Button000Text)) Button000(); 
    if (GuiButton(state->layoutRecs[1], Button001Text)) Button001(); 
    if (GuiButton(state->layoutRecs[2], Button002Text)) Button002(); 
    if (GuiButton(state->layoutRecs[3], Button003Text)) Button003(); 
    if (GuiButton(state->layoutRecs[4], Button004Text)) Button004(); 
    if (GuiButton(state->layoutRecs[5], Button005Text)) Button005(); 
    if (GuiButton(state->layoutRecs[6], Button006Text)) Button006(); 
    if (GuiButton(state->layoutRecs[7], Button007Text)) Button007(); 
}

#endif // GUI_GUI_IMPLEMENTATION
