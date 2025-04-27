#include "settings_menu.h"
#include "raygui.h"
#include "raylib.h"
#include "utils_client.h"
#include <cmath>

static bool isDragging = false;
static Vector2 dragOffset = {0, 0};

float Clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

void ClampSettingsMenuPosition(GUI& state, float screenWidth, float screenHeight) {
    // Calculate maximum allowed positions
    float maxX = screenWidth - state.settingsMenu.layoutRecs[0].width/2;
    float minX = state.settingsMenu.layoutRecs[0].width/2;
    float maxY = screenHeight - state.settingsMenu.layoutRecs[0].height/2;
    float minY = state.settingsMenu.layoutRecs[0].height/2;
    
    // Clamp the anchor position
    state.settingsMenu.anchor01.x = Clamp(state.settingsMenu.anchor01.x, minX, maxX);
    state.settingsMenu.anchor01.y = Clamp(state.settingsMenu.anchor01.y, minY, maxY);
}

void DrawSettingsMenu(GUI& state) {
  if(!state.settingsMenu.active) return;

  Vector2 mousePos = GetMousePosition();
  Rectangle windowRect = {
    state.settingsMenu.layoutRecs[0].x,
    state.settingsMenu.layoutRecs[0].y,
    state.settingsMenu.layoutRecs[0].width,
    40  // Height of drag area
  };

  // Start dragging
  if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePos, windowRect)) {
    isDragging = true;
    dragOffset = {
      state.settingsMenu.anchor01.x - mousePos.x,
      state.settingsMenu.anchor01.y - mousePos.y
    };
  }

  // Continue dragging
  if (isDragging && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
    state.settingsMenu.anchor01.x = mousePos.x + dragOffset.x;
    state.settingsMenu.anchor01.y = mousePos.y + dragOffset.y;
    state.settingsMenu.dirty = true;
  }

  // Stop dragging
  if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
    isDragging = false;
  }

  if(IsWindowResized()) state.settingsMenu.dirty = true;
  if(state.settingsMenu.dirty) {
    state.settingsMenu.dirty = false;
    float width = GetScreenWidth();
    float height = GetScreenHeight();

    UIScale scale = CalculateUIScale();

    // Scale the base font size (12) with the UI
    int scaledFontSize = (int)ScaleSize(12.0f, scale.uniformScale);
    GuiSetStyle(DEFAULT, TEXT_SIZE, scaledFontSize);

    // Ensure window stays in bounds after scaling
    ClampSettingsMenuPosition(state, width, height);

    state.settingsMenu.layoutRecs[0] = { state.settingsMenu.anchor01.x + ScaleSize(-96, scale.uniformScale), state.settingsMenu.anchor01.y + ScaleSize(-124, scale.uniformScale), ScaleSize(192, scale.uniformScale), ScaleSize(248, scale.uniformScale) };    // GroupBox: settingsGroupBox
    state.settingsMenu.layoutRecs[1] = { state.settingsMenu.anchor01.x + ScaleSize(-24, scale.uniformScale), state.settingsMenu.anchor01.y + ScaleSize(28, scale.uniformScale), ScaleSize(80, scale.uniformScale), ScaleSize(24, scale.uniformScale) };    // Spinner: uiStyleSpinner
    state.settingsMenu.layoutRecs[2] = { state.settingsMenu.anchor01.x + ScaleSize(-24, scale.uniformScale), state.settingsMenu.anchor01.y + ScaleSize(-92, scale.uniformScale), ScaleSize(112, scale.uniformScale), ScaleSize(16, scale.uniformScale) };    // SliderBar: musicSliderBar
    state.settingsMenu.layoutRecs[3] = { state.settingsMenu.anchor01.x + ScaleSize(-24, scale.uniformScale), state.settingsMenu.anchor01.y + ScaleSize(-68, scale.uniformScale), ScaleSize(112, scale.uniformScale), ScaleSize(16, scale.uniformScale) };    // SliderBar: sfxSliderBar
    state.settingsMenu.layoutRecs[4] = { state.settingsMenu.anchor01.x + ScaleSize(-24, scale.uniformScale), state.settingsMenu.anchor01.y + ScaleSize(-4, scale.uniformScale), ScaleSize(72, scale.uniformScale), ScaleSize(24, scale.uniformScale) };    // ValueBOx: fpsValueBox
    state.settingsMenu.layoutRecs[5] = { state.settingsMenu.anchor01.x + ScaleSize(-96, scale.uniformScale), state.settingsMenu.anchor01.y + ScaleSize(-52, scale.uniformScale), ScaleSize(192, scale.uniformScale), ScaleSize(16, scale.uniformScale) };    // Line: Line
    state.settingsMenu.layoutRecs[6] = { state.settingsMenu.anchor01.x + ScaleSize(-24, scale.uniformScale), state.settingsMenu.anchor01.y + ScaleSize(60, scale.uniformScale), ScaleSize(112, scale.uniformScale), ScaleSize(16, scale.uniformScale) };    // Slider: uiScaleSlider
    state.settingsMenu.layoutRecs[7] = { state.settingsMenu.anchor01.x + ScaleSize(-24, scale.uniformScale), state.settingsMenu.anchor01.y + ScaleSize(-36, scale.uniformScale), ScaleSize(96, scale.uniformScale), ScaleSize(24, scale.uniformScale) };    // DropdownBox: screenDropdownBox
    state.settingsMenu.layoutRecs[8] = { state.settingsMenu.anchor01.x + ScaleSize(-72, scale.uniformScale), state.settingsMenu.anchor01.y + ScaleSize(-36, scale.uniformScale), ScaleSize(56, scale.uniformScale), ScaleSize(24, scale.uniformScale) };    // Label: screenLabel
    state.settingsMenu.layoutRecs[9] = { state.settingsMenu.anchor01.x + ScaleSize(-96, scale.uniformScale), state.settingsMenu.anchor01.y + ScaleSize(76, scale.uniformScale), ScaleSize(192, scale.uniformScale), ScaleSize(16, scale.uniformScale)}; // Line: Line2
    state.settingsMenu.layoutRecs[10] = { state.settingsMenu.anchor01.x + ScaleSize(-88, scale.uniformScale), state.settingsMenu.anchor01.y + ScaleSize(92, scale.uniformScale), ScaleSize(80, scale.uniformScale), ScaleSize(24, scale.uniformScale)}; // Button: cancelButton
    state.settingsMenu.layoutRecs[11] = { state.settingsMenu.anchor01.x + ScaleSize(8, scale.uniformScale), state.settingsMenu.anchor01.y + ScaleSize(92, scale.uniformScale), ScaleSize(80, scale.uniformScale), ScaleSize(24, scale.uniformScale)}; // Button: applyButton
  }

  if (state.settingsMenu.screenDropdownBoxEditMode) GuiLock();

  state.settingsMenu.active = !GuiWindowBox(state.settingsMenu.layoutRecs[0], state.settingsMenu.settingsWindowBoxText);
  if (GuiSpinner(state.settingsMenu.layoutRecs[1], state.settingsMenu.uiStyleSpinnerText, &state.settingsMenu.uiStyleSpinnerValue, 0, 100, state.settingsMenu.uiStyleSpinnerEditMode)) state.settingsMenu.uiStyleSpinnerEditMode = !state.settingsMenu.uiStyleSpinnerEditMode;
  GuiSliderBar(state.settingsMenu.layoutRecs[2], state.settingsMenu.musicSliderBarText, NULL, &state.settingsMenu.musicSliderBarValue, 0, 100);
  GuiSliderBar(state.settingsMenu.layoutRecs[3], state.settingsMenu.sfxSliderBarText, NULL, &state.settingsMenu.sfxSliderBarValue, 0, 100);
  if (GuiValueBox(state.settingsMenu.layoutRecs[4], state.settingsMenu.fpsValueBoxText, &state.settingsMenu.fpsValueBoxValue, 20, 300, state.settingsMenu.fpsValueBoxEditMode)) state.settingsMenu.fpsValueBoxEditMode = !state.settingsMenu.fpsValueBoxEditMode;
  GuiLine(state.settingsMenu.layoutRecs[5], state.settingsMenu.LineText);
  GuiSlider(state.settingsMenu.layoutRecs[6], state.settingsMenu.uiScaleSliderText, NULL, &state.settingsMenu.uiScaleSliderValue, 0.25, 4);
  GuiLabel(state.settingsMenu.layoutRecs[8], state.settingsMenu.screenLabelText);
  if (GuiDropdownBox(state.settingsMenu.layoutRecs[7], state.settingsMenu.screenDropdownBoxText, &state.settingsMenu.screenDropdownBoxActive, state.settingsMenu.screenDropdownBoxEditMode)) state.settingsMenu.screenDropdownBoxEditMode = !state.settingsMenu.screenDropdownBoxEditMode;
  GuiLine(state.settingsMenu.layoutRecs[9], state.settingsMenu.Line2Text);
  if (GuiButton(state.settingsMenu.layoutRecs[10], state.settingsMenu.cancelButtonText)) CancelButton(state); 
  if (GuiButton(state.settingsMenu.layoutRecs[11], state.settingsMenu.applyButtonText)) ApplyButton(state); 

  GuiUnlock();
}

void CancelButton(GUI& state)
{
  // TODO: Implement control logic
  state.settingsMenu.active = false;
}
void ApplyButton(GUI& state)
{
  // TODO: Implement control logic
  state.settingsMenu.active = false;
}
