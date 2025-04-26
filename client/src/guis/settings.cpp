#include "settings.h"
#include "raygui.h"
#include "raylib.h"
#include "utils_client.h"
#include <cmath>

// Const text
const char *settingsGroupBoxText = "Settings";    // GROUPBOX: settingsGroupBox
const char *musicSliderBarText = "Music";    // SLIDERBAR: musicSliderBar
const char *sfxSliderBarText = "SFX";    // SLIDERBAR: sfxSliderBar
const char *uiScaleSliderText = "UI Scale";    // SLIDER: uiScaleSlider
const char *screenDropdownBoxText = "Windowed; Fullscreen";    // DROPDOWNBOX: screenDropdownBox
const char *screenLabelText = "Screen";    // LABEL: screenLabel
const char *LineText = "";
const char *fpsValueBoxText = "FPS";
const char *uiStyleSpinnerText = "UI Style";

// Define controls variables
bool uiStyleSpinnerEditMode = false;
int uiStyleSpinnerValue = 0;            // Spinner: uiStyleSpinner
float musicSliderBarValue = 0.0f;            // SliderBar: musicSliderBar
float sfxSliderBarValue = 0.0f;            // SliderBar: sfxSliderBar
bool fpsValueBoxEditMode = false;
int fpsValueBoxValue = 0;            // ValueBOx: fpsValueBox
float uiScaleSliderValue = 0.0f;            // Slider: uiScaleSlider
bool screenDropdownBoxEditMode = false;
int screenDropdownBoxActive = 0;            // DropdownBox: screenDropdownBox

void DrawSettings() {
    float width = GetScreenWidth();
    float height = GetScreenHeight();
    
    UIScale scale = CalculateUIScale();

    // Scale the base font size (12) with the UI
    int scaledFontSize = (int)ScaleSize(12.0f, scale.uniformScale);
    GuiSetStyle(DEFAULT, TEXT_SIZE, scaledFontSize);

    // Define anchors
    Vector2 anchor01 = { width/2, height/2 };

    // Define controls rectangles
    Rectangle layoutRecs[9] = {
        { anchor01.x + ScaleSize(-96, scale.uniformScale), anchor01.y + ScaleSize(-88, scale.uniformScale), ScaleSize(192, scale.uniformScale), ScaleSize(184, scale.uniformScale) },    // GroupBox: settingsGroupBox
        { anchor01.x + ScaleSize(-24, scale.uniformScale), anchor01.y + ScaleSize(40, scale.uniformScale), ScaleSize(80, scale.uniformScale), ScaleSize(24, scale.uniformScale) },    // Spinner: uiStyleSpinner
        { anchor01.x + ScaleSize(-24, scale.uniformScale), anchor01.y + ScaleSize(-80, scale.uniformScale), ScaleSize(112, scale.uniformScale), ScaleSize(16, scale.uniformScale) },    // SliderBar: musicSliderBar
        { anchor01.x + ScaleSize(-24, scale.uniformScale), anchor01.y + ScaleSize(-56, scale.uniformScale), ScaleSize(112, scale.uniformScale), ScaleSize(16, scale.uniformScale) },    // SliderBar: sfxSliderBar
        { anchor01.x + ScaleSize(-24, scale.uniformScale), anchor01.y + ScaleSize(-24, scale.uniformScale), ScaleSize(72, scale.uniformScale), ScaleSize(24, scale.uniformScale) },    // ValueBOx: fpsValueBox
        { anchor01.x + ScaleSize(-96, scale.uniformScale), anchor01.y + ScaleSize(-40, scale.uniformScale), ScaleSize(192, scale.uniformScale), ScaleSize(16, scale.uniformScale) },    // Line: Line
        { anchor01.x + ScaleSize(-24, scale.uniformScale), anchor01.y + ScaleSize(72, scale.uniformScale), ScaleSize(112, scale.uniformScale), ScaleSize(16, scale.uniformScale) },    // Slider: uiScaleSlider
        { anchor01.x + ScaleSize(-24, scale.uniformScale), anchor01.y + ScaleSize(8, scale.uniformScale), ScaleSize(96, scale.uniformScale), ScaleSize(24, scale.uniformScale) },    // DropdownBox: screenDropdownBox
        { anchor01.x + ScaleSize(-72, scale.uniformScale), anchor01.y + ScaleSize(8, scale.uniformScale), ScaleSize(56, scale.uniformScale), ScaleSize(24, scale.uniformScale) },    // Label: screenLabel
    };

    DrawRectangle(layoutRecs[0].x, layoutRecs[0].y, layoutRecs[0].width, layoutRecs[0].height, GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

    if (screenDropdownBoxEditMode) GuiLock();

    GuiGroupBox(layoutRecs[0], settingsGroupBoxText);
    if (GuiSpinner(layoutRecs[1], uiStyleSpinnerText, &uiStyleSpinnerValue, 0, 100, uiStyleSpinnerEditMode)) uiStyleSpinnerEditMode = !uiStyleSpinnerEditMode;
    GuiSliderBar(layoutRecs[2], musicSliderBarText, NULL, &musicSliderBarValue, 0, 100);
    GuiSliderBar(layoutRecs[3], sfxSliderBarText, NULL, &sfxSliderBarValue, 0, 100);
    if (GuiValueBox(layoutRecs[4], fpsValueBoxText, &fpsValueBoxValue, 0, 100, fpsValueBoxEditMode)) fpsValueBoxEditMode = !fpsValueBoxEditMode;
    GuiLine(layoutRecs[5], LineText);
    GuiSlider(layoutRecs[6], uiScaleSliderText, NULL, &uiScaleSliderValue, 0, 100);
    GuiLabel(layoutRecs[8], screenLabelText);
    if (GuiDropdownBox(layoutRecs[7], screenDropdownBoxText, &screenDropdownBoxActive, screenDropdownBoxEditMode)) screenDropdownBoxEditMode = !screenDropdownBoxEditMode;
    
    GuiUnlock();
}
