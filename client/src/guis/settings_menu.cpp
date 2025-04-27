#include "settings_menu.h"
#include "raygui.h"
#include "raylib.h"
#include "utils_client.h"
#include <cmath>

void DrawSettingsMenu(GUI& state) {
  if(!state.settingsMenu.active) return;
  if(IsWindowResized()) state.settingsMenu.dirty = true;
  if(state.settingsMenu.dirty) {
    state.settingsMenu.dirty = false;
    float width = GetScreenWidth();
    float height = GetScreenHeight();

    UIScale scale = CalculateUIScale();

    // Scale the base font size (12) with the UI
    int scaledFontSize = (int)ScaleSize(12.0f, scale.uniformScale);
    GuiSetStyle(DEFAULT, TEXT_SIZE, scaledFontSize);

    // Define anchors
    state.settingsMenu.anchor01 = { width/2, height/2 };

    state.settingsMenu.layoutRecs[0] = { state.settingsMenu.anchor01.x + ScaleSize(-96, scale.uniformScale), state.settingsMenu.anchor01.y + ScaleSize(-88, scale.uniformScale), ScaleSize(192, scale.uniformScale), ScaleSize(184, scale.uniformScale) };    // GroupBox: settingsGroupBox
    state.settingsMenu.layoutRecs[1] = { state.settingsMenu.anchor01.x + ScaleSize(-24, scale.uniformScale), state.settingsMenu.anchor01.y + ScaleSize(40, scale.uniformScale), ScaleSize(80, scale.uniformScale), ScaleSize(24, scale.uniformScale) };    // Spinner: uiStyleSpinner
    state.settingsMenu.layoutRecs[2] = { state.settingsMenu.anchor01.x + ScaleSize(-24, scale.uniformScale), state.settingsMenu.anchor01.y + ScaleSize(-80, scale.uniformScale), ScaleSize(112, scale.uniformScale), ScaleSize(16, scale.uniformScale) };    // SliderBar: musicSliderBar
    state.settingsMenu.layoutRecs[3] = { state.settingsMenu.anchor01.x + ScaleSize(-24, scale.uniformScale), state.settingsMenu.anchor01.y + ScaleSize(-56, scale.uniformScale), ScaleSize(112, scale.uniformScale), ScaleSize(16, scale.uniformScale) };    // SliderBar: sfxSliderBar
    state.settingsMenu.layoutRecs[4] = { state.settingsMenu.anchor01.x + ScaleSize(-24, scale.uniformScale), state.settingsMenu.anchor01.y + ScaleSize(-24, scale.uniformScale), ScaleSize(72, scale.uniformScale), ScaleSize(24, scale.uniformScale) };    // ValueBOx: fpsValueBox
    state.settingsMenu.layoutRecs[5] = { state.settingsMenu.anchor01.x + ScaleSize(-96, scale.uniformScale), state.settingsMenu.anchor01.y + ScaleSize(-40, scale.uniformScale), ScaleSize(192, scale.uniformScale), ScaleSize(16, scale.uniformScale) };    // Line: Line
    state.settingsMenu.layoutRecs[6] = { state.settingsMenu.anchor01.x + ScaleSize(-24, scale.uniformScale), state.settingsMenu.anchor01.y + ScaleSize(72, scale.uniformScale), ScaleSize(112, scale.uniformScale), ScaleSize(16, scale.uniformScale) };    // Slider: uiScaleSlider
    state.settingsMenu.layoutRecs[7] = { state.settingsMenu.anchor01.x + ScaleSize(-24, scale.uniformScale), state.settingsMenu.anchor01.y + ScaleSize(8, scale.uniformScale), ScaleSize(96, scale.uniformScale), ScaleSize(24, scale.uniformScale) };    // DropdownBox: screenDropdownBox
    state.settingsMenu.layoutRecs[8] = { state.settingsMenu.anchor01.x + ScaleSize(-72, scale.uniformScale), state.settingsMenu.anchor01.y + ScaleSize(8, scale.uniformScale), ScaleSize(56, scale.uniformScale), ScaleSize(24, scale.uniformScale) };    // Label: screenLabel
  }

  DrawRectangle(state.settingsMenu.layoutRecs[0].x, state.settingsMenu.layoutRecs[0].y, state.settingsMenu.layoutRecs[0].width, state.settingsMenu.layoutRecs[0].height, GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

  if (state.settingsMenu.screenDropdownBoxEditMode) GuiLock();

  GuiGroupBox(state.settingsMenu.layoutRecs[0], state.settingsMenu.settingsGroupBoxText);
  if (GuiSpinner(state.settingsMenu.layoutRecs[1], state.settingsMenu.uiStyleSpinnerText, &state.settingsMenu.uiStyleSpinnerValue, 0, 100, state.settingsMenu.uiStyleSpinnerEditMode)) state.settingsMenu.uiStyleSpinnerEditMode = !state.settingsMenu.uiStyleSpinnerEditMode;
  GuiSliderBar(state.settingsMenu.layoutRecs[2], state.settingsMenu.musicSliderBarText, NULL, &state.settingsMenu.musicSliderBarValue, 0, 100);
  GuiSliderBar(state.settingsMenu.layoutRecs[3], state.settingsMenu.sfxSliderBarText, NULL, &state.settingsMenu.sfxSliderBarValue, 0, 100);
  if (GuiValueBox(state.settingsMenu.layoutRecs[4], state.settingsMenu.fpsValueBoxText, &state.settingsMenu.fpsValueBoxValue, 0, 100, state.settingsMenu.fpsValueBoxEditMode)) state.settingsMenu.fpsValueBoxEditMode = !state.settingsMenu.fpsValueBoxEditMode;
  GuiLine(state.settingsMenu.layoutRecs[5], state.settingsMenu.LineText);
  GuiSlider(state.settingsMenu.layoutRecs[6], state.settingsMenu.uiScaleSliderText, NULL, &state.settingsMenu.uiScaleSliderValue, 0, 100);
  GuiLabel(state.settingsMenu.layoutRecs[8], state.settingsMenu.screenLabelText);
  if (GuiDropdownBox(state.settingsMenu.layoutRecs[7], state.settingsMenu.screenDropdownBoxText, &state.settingsMenu.screenDropdownBoxActive, state.settingsMenu.screenDropdownBoxEditMode)) state.settingsMenu.screenDropdownBoxEditMode = !state.settingsMenu.screenDropdownBoxEditMode;

  GuiUnlock();
}
