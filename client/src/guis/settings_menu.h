#pragma once

#include "game_state.h"

void UpdateSettingsMenu(SettingsMenu& state);
void DrawSettingsMenu(SettingsMenu& state);
void CancelButton(SettingsMenu& state);                // Button: cancelButton logic
void ApplyButton(SettingsMenu& state);                 // Button: applyButton logic
