#include "settings_menu.h"
#include "game_state.h"
#include "raygui.h"
#include "raylib.h"
#include "utils_client.h"
#include "rini.h"
#include <cmath>

void ClampSettingsMenuPosition(SettingsMenu& sm, float screenWidth, float screenHeight) {
    // Calculate maximum allowed positions
    float maxX = screenWidth - sm.layoutRecs[0].width/2;
    float minX = sm.layoutRecs[0].width/2;
    float maxY = screenHeight - sm.layoutRecs[0].height/2;
    float minY = sm.layoutRecs[0].height/2;
    
    // Clamp the anchor position
    sm.anchor01.x = Clamp(sm.anchor01.x, minX, maxX);
    sm.anchor01.y = Clamp(sm.anchor01.y, minY, maxY);
}

void UpdateSettingsMenu(SettingsMenu& sm, Settings& s) {
  if(!sm.active) return;

  Vector2 mousePos = GetMousePosition();
  Rectangle windowRect = {
    sm.layoutRecs[0].x,
    sm.layoutRecs[0].y,
    sm.layoutRecs[0].width,
    25  // Height of drag area
  };

  // Start dragging
  if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePos, windowRect)) {
    sm.isDragging = true;
    sm.dragOffset = {
      sm.anchor01.x - mousePos.x,
      sm.anchor01.y - mousePos.y
    };
  }

  // Continue dragging
  if (sm.isDragging && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
    sm.anchor01.x = mousePos.x + sm.dragOffset.x;
    sm.anchor01.y = mousePos.y + sm.dragOffset.y;
    sm.dirty = true;
  }

  // Stop dragging
  if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
    sm.isDragging = false;
  }

  if(IsWindowResized()) sm.dirty = true;
  if(sm.dirty) {
    sm.dirty = false;
    float width = GetScreenWidth();
    float height = GetScreenHeight();

    UIScale scale = CalculateUIScale(s.uiScale);

    // Scale the base font size (12) with the UI
    int scaledFontSize = (int)ScaleSize(12.0f, scale.uniformScale);
    GuiSetStyle(DEFAULT, TEXT_SIZE, scaledFontSize);

    // Ensure window stays in bounds after scaling
    ClampSettingsMenuPosition(sm, width, height);

    sm.layoutRecs[0] = { sm.anchor01.x + ScaleSize(-96, scale.uniformScale), sm.anchor01.y + ScaleSize(-124, scale.uniformScale), ScaleSize(192, scale.uniformScale), ScaleSize(248, scale.uniformScale) };    // GroupBox: settingsGroupBox
    sm.layoutRecs[1] = { sm.anchor01.x + ScaleSize(-24, scale.uniformScale), sm.anchor01.y + ScaleSize(28, scale.uniformScale), ScaleSize(80, scale.uniformScale), ScaleSize(24, scale.uniformScale) };    // Spinner: uiStyleSpinner
    sm.layoutRecs[2] = { sm.anchor01.x + ScaleSize(-24, scale.uniformScale), sm.anchor01.y + ScaleSize(-92, scale.uniformScale), ScaleSize(112, scale.uniformScale), ScaleSize(16, scale.uniformScale) };    // SliderBar: musicSliderBar
    sm.layoutRecs[3] = { sm.anchor01.x + ScaleSize(-24, scale.uniformScale), sm.anchor01.y + ScaleSize(-68, scale.uniformScale), ScaleSize(112, scale.uniformScale), ScaleSize(16, scale.uniformScale) };    // SliderBar: sfxSliderBar
    sm.layoutRecs[4] = { sm.anchor01.x + ScaleSize(-24, scale.uniformScale), sm.anchor01.y + ScaleSize(-4, scale.uniformScale), ScaleSize(72, scale.uniformScale), ScaleSize(24, scale.uniformScale) };    // ValueBOx: fpsValueBox
    sm.layoutRecs[5] = { sm.anchor01.x + ScaleSize(-96, scale.uniformScale), sm.anchor01.y + ScaleSize(-52, scale.uniformScale), ScaleSize(192, scale.uniformScale), ScaleSize(16, scale.uniformScale) };    // Line: Line
    sm.layoutRecs[6] = { sm.anchor01.x + ScaleSize(-24, scale.uniformScale), sm.anchor01.y + ScaleSize(60, scale.uniformScale), ScaleSize(112, scale.uniformScale), ScaleSize(16, scale.uniformScale) };    // Slider: uiScaleSlider
    sm.layoutRecs[7] = { sm.anchor01.x + ScaleSize(-24, scale.uniformScale), sm.anchor01.y + ScaleSize(-36, scale.uniformScale), ScaleSize(96, scale.uniformScale), ScaleSize(24, scale.uniformScale) };    // DropdownBox: screenDropdownBox
    sm.layoutRecs[8] = { sm.anchor01.x + ScaleSize(-72, scale.uniformScale), sm.anchor01.y + ScaleSize(-36, scale.uniformScale), ScaleSize(56, scale.uniformScale), ScaleSize(24, scale.uniformScale) };    // Label: screenLabel
    sm.layoutRecs[9] = { sm.anchor01.x + ScaleSize(-96, scale.uniformScale), sm.anchor01.y + ScaleSize(76, scale.uniformScale), ScaleSize(192, scale.uniformScale), ScaleSize(16, scale.uniformScale)}; // Line: Line2
    sm.layoutRecs[10] = { sm.anchor01.x + ScaleSize(-88, scale.uniformScale), sm.anchor01.y + ScaleSize(92, scale.uniformScale), ScaleSize(80, scale.uniformScale), ScaleSize(24, scale.uniformScale)}; // Button: cancelButton
    sm.layoutRecs[11] = { sm.anchor01.x + ScaleSize(8, scale.uniformScale), sm.anchor01.y + ScaleSize(92, scale.uniformScale), ScaleSize(80, scale.uniformScale), ScaleSize(24, scale.uniformScale)}; // Button: applyButton
  }
}

void DrawSettingsMenu(GameState& state) {
  GUI& gui = *state.renderResources.gui;
  SettingsMenu& sm = gui.settingsMenu;
  Settings& s = state.settings;

  if(!sm.active) return;

  if (sm.displayDropdownBoxEditMode) GuiLock();

  sm.active = !GuiWindowBox(sm.layoutRecs[0], sm.settingsWindowBoxText);
  if (GuiSpinner(sm.layoutRecs[1], sm.uiStyleSpinnerText, &sm.uiStyleSpinnerValue, 0, 10, sm.uiStyleSpinnerEditMode)) sm.uiStyleSpinnerEditMode = !sm.uiStyleSpinnerEditMode;
  GuiSliderBar(sm.layoutRecs[2], sm.musicSliderBarText, NULL, &sm.musicSliderBarValue, 0, 100);
  GuiSliderBar(sm.layoutRecs[3], sm.sfxSliderBarText, NULL, &sm.sfxSliderBarValue, 0, 100);
  if (GuiValueBox(sm.layoutRecs[4], sm.fpsValueBoxText, &sm.fpsValueBoxValue, 10, 1000, sm.fpsValueBoxEditMode)) sm.fpsValueBoxEditMode = !sm.fpsValueBoxEditMode;
  GuiLine(sm.layoutRecs[5], sm.LineText);
  GuiSlider(sm.layoutRecs[6], sm.uiScaleSliderText, NULL, &sm.uiScaleSliderValue, 0, 100);
  GuiLabel(sm.layoutRecs[8], sm.displayLabelText);
  if (GuiDropdownBox(sm.layoutRecs[7], sm.displayDropdownBoxText, &sm.displayDropdownBoxActive, sm.displayDropdownBoxEditMode)) sm.displayDropdownBoxEditMode = !sm.displayDropdownBoxEditMode;
  GuiLine(sm.layoutRecs[9], sm.Line2Text);
  if (GuiButton(sm.layoutRecs[10], sm.cancelButtonText)) CancelButton(gui, s); 
  if (GuiButton(sm.layoutRecs[11], sm.applyButtonText)) ApplyButton(state); 

  GuiUnlock();
}

void CancelButton(GUI& gui, Settings& s) {
  rini_config config = rini_load_config("settings.ini");

  s.musicVolume = rini_get_config_value(config, "MUSIC_VOLUME");
  s.sfxVolume = rini_get_config_value(config, "SFX_VOLUME");
  s.displayMode = rini_get_config_value(config, "DISPLAY_MODE");
  s.fpsLimit = rini_get_config_value(config, "FPS_LIMIT");
  s.uiStyle = rini_get_config_value(config, "UI_STYLE");
  s.uiScale = rini_get_config_value(config, "UI_SCALE");

  rini_unload_config(&config);
  gui.settingsMenu.active = false;
  gui.settingsMenu.dirty = true;
  gui.mainMenu.dirty = true;
}

void ApplyButton(GameState& state) {
  GUI& gui = *state.renderResources.gui;
  Settings& s = state.settings;

  s.fpsLimit = gui.settingsMenu.fpsValueBoxValue;
  s.uiStyle = gui.settingsMenu.uiStyleSpinnerValue;
  s.uiScale = gui.settingsMenu.uiScaleSliderValue;
  s.displayMode = gui.settingsMenu.displayDropdownBoxActive;
  s.sfxVolume = gui.settingsMenu.sfxSliderBarValue;
  s.musicVolume = gui.settingsMenu.musicSliderBarValue;

  rini_config config = rini_load_config(NULL);

  rini_set_config_comment_line(&config, NULL);
  rini_set_config_comment_line(&config, "Settings");
  rini_set_config_comment_line(&config, NULL);
  rini_set_config_comment_line(&config, "NOTE: This file is loaded at application startup,");
  rini_set_config_comment_line(&config, "if file is not found, default values are applied");
  rini_set_config_comment_line(&config, NULL);

  rini_set_config_value(&config, "MUSIC_VOLUME", s.musicVolume, "Music volume");
  rini_set_config_value(&config, "SFX_VOLUME", s.sfxVolume, "Sound effects volumes");
  rini_set_config_value(&config, "DISPLAY_MODE", s.displayMode, "Borderless; Windowed; Fullscreen");
  rini_set_config_value(&config, "FPS_LIMIT", s.fpsLimit, "The fps that the game should not exceed");
  rini_set_config_value(&config, "UI_STYLE", s.uiStyle, "UI visual style selected");
  rini_set_config_value(&config, "UI_SCALE", s.uiScale, "UI scale multiplier");

  rini_save_config(config, "settings.ini");

  rini_unload_config(&config);

  // TODO: UpdateSettings(state);

  // gui.settingsMenu.active = false;
  gui.settingsMenu.dirty = true;
  gui.mainMenu.dirty = true;
}
