#include "settings_menu.h"
#include "game_state.h"
#include "raygui.h"
#include "raylib.h"
#include "utils_client.h"
#include "rini.h"
#include <cmath>

void ClampSettingsMenuPosition(SettingsMenu& state, float screenWidth, float screenHeight) {
    // Calculate maximum allowed positions
    float maxX = screenWidth - state.layoutRecs[0].width/2;
    float minX = state.layoutRecs[0].width/2;
    float maxY = screenHeight - state.layoutRecs[0].height/2;
    float minY = state.layoutRecs[0].height/2;
    
    // Clamp the anchor position
    state.anchor01.x = Clamp(state.anchor01.x, minX, maxX);
    state.anchor01.y = Clamp(state.anchor01.y, minY, maxY);
}

void UpdateSettingsMenu(SettingsMenu& s) {
  if(!s.active) return;

  Vector2 mousePos = GetMousePosition();
  Rectangle windowRect = {
    s.layoutRecs[0].x,
    s.layoutRecs[0].y,
    s.layoutRecs[0].width,
    25  // Height of drag area
  };

  // Start dragging
  if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePos, windowRect)) {
    s.isDragging = true;
    s.dragOffset = {
      s.anchor01.x - mousePos.x,
      s.anchor01.y - mousePos.y
    };
  }

  // Continue dragging
  if (s.isDragging && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
    s.anchor01.x = mousePos.x + s.dragOffset.x;
    s.anchor01.y = mousePos.y + s.dragOffset.y;
    s.dirty = true;
  }

  // Stop dragging
  if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
    s.isDragging = false;
  }

  if(IsWindowResized()) s.dirty = true;
  if(s.dirty) {
    s.dirty = false;
    float width = GetScreenWidth();
    float height = GetScreenHeight();

    UIScale scale = CalculateUIScale();

    // Scale the base font size (12) with the UI
    int scaledFontSize = (int)ScaleSize(12.0f, scale.uniformScale);
    GuiSetStyle(DEFAULT, TEXT_SIZE, scaledFontSize);

    // Ensure window stays in bounds after scaling
    ClampSettingsMenuPosition(s, width, height);

    s.layoutRecs[0] = { s.anchor01.x + ScaleSize(-96, scale.uniformScale), s.anchor01.y + ScaleSize(-124, scale.uniformScale), ScaleSize(192, scale.uniformScale), ScaleSize(248, scale.uniformScale) };    // GroupBox: settingsGroupBox
    s.layoutRecs[1] = { s.anchor01.x + ScaleSize(-24, scale.uniformScale), s.anchor01.y + ScaleSize(28, scale.uniformScale), ScaleSize(80, scale.uniformScale), ScaleSize(24, scale.uniformScale) };    // Spinner: uiStyleSpinner
    s.layoutRecs[2] = { s.anchor01.x + ScaleSize(-24, scale.uniformScale), s.anchor01.y + ScaleSize(-92, scale.uniformScale), ScaleSize(112, scale.uniformScale), ScaleSize(16, scale.uniformScale) };    // SliderBar: musicSliderBar
    s.layoutRecs[3] = { s.anchor01.x + ScaleSize(-24, scale.uniformScale), s.anchor01.y + ScaleSize(-68, scale.uniformScale), ScaleSize(112, scale.uniformScale), ScaleSize(16, scale.uniformScale) };    // SliderBar: sfxSliderBar
    s.layoutRecs[4] = { s.anchor01.x + ScaleSize(-24, scale.uniformScale), s.anchor01.y + ScaleSize(-4, scale.uniformScale), ScaleSize(72, scale.uniformScale), ScaleSize(24, scale.uniformScale) };    // ValueBOx: fpsValueBox
    s.layoutRecs[5] = { s.anchor01.x + ScaleSize(-96, scale.uniformScale), s.anchor01.y + ScaleSize(-52, scale.uniformScale), ScaleSize(192, scale.uniformScale), ScaleSize(16, scale.uniformScale) };    // Line: Line
    s.layoutRecs[6] = { s.anchor01.x + ScaleSize(-24, scale.uniformScale), s.anchor01.y + ScaleSize(60, scale.uniformScale), ScaleSize(112, scale.uniformScale), ScaleSize(16, scale.uniformScale) };    // Slider: uiScaleSlider
    s.layoutRecs[7] = { s.anchor01.x + ScaleSize(-24, scale.uniformScale), s.anchor01.y + ScaleSize(-36, scale.uniformScale), ScaleSize(96, scale.uniformScale), ScaleSize(24, scale.uniformScale) };    // DropdownBox: screenDropdownBox
    s.layoutRecs[8] = { s.anchor01.x + ScaleSize(-72, scale.uniformScale), s.anchor01.y + ScaleSize(-36, scale.uniformScale), ScaleSize(56, scale.uniformScale), ScaleSize(24, scale.uniformScale) };    // Label: screenLabel
    s.layoutRecs[9] = { s.anchor01.x + ScaleSize(-96, scale.uniformScale), s.anchor01.y + ScaleSize(76, scale.uniformScale), ScaleSize(192, scale.uniformScale), ScaleSize(16, scale.uniformScale)}; // Line: Line2
    s.layoutRecs[10] = { s.anchor01.x + ScaleSize(-88, scale.uniformScale), s.anchor01.y + ScaleSize(92, scale.uniformScale), ScaleSize(80, scale.uniformScale), ScaleSize(24, scale.uniformScale)}; // Button: cancelButton
    s.layoutRecs[11] = { s.anchor01.x + ScaleSize(8, scale.uniformScale), s.anchor01.y + ScaleSize(92, scale.uniformScale), ScaleSize(80, scale.uniformScale), ScaleSize(24, scale.uniformScale)}; // Button: applyButton
  }
}

void DrawSettingsMenu(SettingsMenu& s) {
  if(!s.active) return;

  if (s.screenDropdownBoxEditMode) GuiLock();

  s.active = !GuiWindowBox(s.layoutRecs[0], s.settingsWindowBoxText);
  if (GuiSpinner(s.layoutRecs[1], s.uiStyleSpinnerText, &s.uiStyleSpinnerValue, 0, 10, s.uiStyleSpinnerEditMode)) s.uiStyleSpinnerEditMode = !s.uiStyleSpinnerEditMode;
  GuiSliderBar(s.layoutRecs[2], s.musicSliderBarText, NULL, &s.musicSliderBarValue, 0, 100);
  GuiSliderBar(s.layoutRecs[3], s.sfxSliderBarText, NULL, &s.sfxSliderBarValue, 0, 100);
  if (GuiValueBox(s.layoutRecs[4], s.fpsValueBoxText, &s.fpsValueBoxValue, 10, 1000, s.fpsValueBoxEditMode)) s.fpsValueBoxEditMode = !s.fpsValueBoxEditMode;
  GuiLine(s.layoutRecs[5], s.LineText);
  GuiSlider(s.layoutRecs[6], s.uiScaleSliderText, NULL, &s.uiScaleSliderValue, 0, 100);
  GuiLabel(s.layoutRecs[8], s.screenLabelText);
  if (GuiDropdownBox(s.layoutRecs[7], s.screenDropdownBoxText, &s.screenDropdownBoxActive, s.screenDropdownBoxEditMode)) s.screenDropdownBoxEditMode = !s.screenDropdownBoxEditMode;
  GuiLine(s.layoutRecs[9], s.Line2Text);
  if (GuiButton(s.layoutRecs[10], s.cancelButtonText)) CancelButton(s); 
  if (GuiButton(s.layoutRecs[11], s.applyButtonText)) ApplyButton(s); 

  GuiUnlock();
}

void CancelButton(SettingsMenu& state)
{
  rini_config config = rini_load_config("settings.ini");

  state.musicSliderBarValue = rini_get_config_value(config, "MUSIC_VOLUME");
  state.sfxSliderBarValue = rini_get_config_value(config, "SFX_VOLUME");
  state.screenDropdownBoxActive = rini_get_config_value(config, "SCREEN");
  state.fpsValueBoxValue = rini_get_config_value(config, "FPS_LIMIT");
  state.uiStyleSpinnerValue = rini_get_config_value(config, "UI_STYLE");
  state.uiScaleSliderValue = rini_get_config_value(config, "UI_SCALE");

  rini_unload_config(&config);
  state.active = false;
}

void ApplyButton(SettingsMenu& state)
{
  rini_config config = rini_load_config(NULL);

  rini_set_config_comment_line(&config, NULL);
  rini_set_config_comment_line(&config, "Settings");
  rini_set_config_comment_line(&config, NULL);
  rini_set_config_comment_line(&config, "NOTE: This file is loaded at application startup,");
  rini_set_config_comment_line(&config, "if file is not found, default values are applied");
  rini_set_config_comment_line(&config, NULL);

  rini_set_config_value(&config, "MUSIC_VOLUME", state.musicSliderBarValue, "Music volume");
  rini_set_config_value(&config, "SFX_VOLUME", state.sfxSliderBarValue, "Sound effects volumes");
  rini_set_config_value(&config, "SCREEN", state.screenDropdownBoxActive, "Borderless; Windowed; Fullscreen");
  rini_set_config_value(&config, "FPS_LIMIT", state.fpsValueBoxValue, "The fps that the game should not exceed");
  rini_set_config_value(&config, "UI_STYLE", state.uiStyleSpinnerValue, "UI visual style selected");
  rini_set_config_value(&config, "UI_SCALE", state.uiScaleSliderValue, "UI scale multiplier");

  rini_save_config(config, "settings.ini");

  rini_unload_config(&config);
  state.active = false;
}
