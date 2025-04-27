#include "main_menu.h"
#include "game_state.h"
#include "raygui.h"
#include "raylib.h"
#include "utils_client.h"
#include <cmath>

void DrawMainMenu(GUI& state) {
  if(IsWindowResized()) state.mainMenu.dirty = true;
  if(state.mainMenu.dirty) {
    state.mainMenu.dirty = false;
    float width = GetScreenWidth();
    float height = GetScreenHeight();

    UIScale scale = CalculateUIScale();

    // Scale the base font size (12) with the UI
    int scaledFontSize = (int)ScaleSize(12.0f, scale.uniformScale);
    GuiSetStyle(DEFAULT, TEXT_SIZE, scaledFontSize);
    
    // Define anchors
    state.mainMenu.anchor01 = { 24 * scale.scaleX, height - 24 * scale.scaleY };
    state.mainMenu.anchor02 = { 24 * scale.scaleX, 24 * scale.scaleY };
    state.mainMenu.anchor03 = { width - 24 * scale.scaleX, height - 24 * scale.scaleY };
    state.mainMenu.anchor04 = { width - 24 * scale.scaleX, 24 * scale.scaleY };

    state.mainMenu.layoutRecs[0] = {state.mainMenu.anchor02.x, state.mainMenu.anchor02.y, ScaleSize(144, scale.uniformScale), ScaleSize(48, scale.uniformScale)};    // Button: realtimeButton
    state.mainMenu.layoutRecs[1] = {state.mainMenu.anchor02.x, state.mainMenu.anchor02.y + ScaleSize(56, scale.uniformScale), ScaleSize(144, scale.uniformScale), ScaleSize(48, scale.uniformScale)};    // Button: arenaButton
    state.mainMenu.layoutRecs[2] = {state.mainMenu.anchor02.x, state.mainMenu.anchor02.y + ScaleSize(168, scale.uniformScale), ScaleSize(144, scale.uniformScale), ScaleSize(48, scale.uniformScale)};    // Button: sandboxButton
    state.mainMenu.layoutRecs[3] = {state.mainMenu.anchor02.x, state.mainMenu.anchor02.y + ScaleSize(112, scale.uniformScale), ScaleSize(144, scale.uniformScale), ScaleSize(48, scale.uniformScale)};    // Button: customGameButton
    state.mainMenu.layoutRecs[4] = {state.mainMenu.anchor01.x, state.mainMenu.anchor01.y - ScaleSize(344, scale.uniformScale), ScaleSize(144, scale.uniformScale), ScaleSize(48, scale.uniformScale)};    // Button: settingsButton
    state.mainMenu.layoutRecs[5] = {state.mainMenu.anchor01.x, state.mainMenu.anchor01.y - ScaleSize(288, scale.uniformScale), ScaleSize(576, scale.uniformScale), ScaleSize(288, scale.uniformScale)};    // ScrollPanel: chatScrollWindow
    state.mainMenu.layoutRecs[6] = {state.mainMenu.anchor03.x - ScaleSize(136, scale.uniformScale), state.mainMenu.anchor03.y - ScaleSize(288, scale.uniformScale), ScaleSize(136, scale.uniformScale), ScaleSize(288, scale.uniformScale)};    // GroupBox: musicSelectorGroupBox
    state.mainMenu.layoutRecs[7] = {state.mainMenu.anchor03.x - ScaleSize(128, scale.uniformScale), state.mainMenu.anchor03.y - ScaleSize(248, scale.uniformScale), ScaleSize(120, scale.uniformScale), ScaleSize(240, scale.uniformScale)};    // ListView: songsListView
    state.mainMenu.layoutRecs[8] = {state.mainMenu.anchor03.x - ScaleSize(128, scale.uniformScale), state.mainMenu.anchor03.y - ScaleSize(280, scale.uniformScale), ScaleSize(120, scale.uniformScale), ScaleSize(24, scale.uniformScale)};    // Spinner: albumSpinner
    state.mainMenu.layoutRecs[9] = {state.mainMenu.anchor04.x - ScaleSize(192, scale.uniformScale), state.mainMenu.anchor04.y + ScaleSize(80, scale.uniformScale), ScaleSize(192, scale.uniformScale), ScaleSize(208, scale.uniformScale)};    // GroupBox: partyGroupBox
    state.mainMenu.layoutRecs[10] = {state.mainMenu.anchor04.x - ScaleSize(192, scale.uniformScale), state.mainMenu.anchor04.y + ScaleSize(8, scale.uniformScale), ScaleSize(192, scale.uniformScale), ScaleSize(64, scale.uniformScale)};    // GroupBox: profileGroupBox
    state.mainMenu.layoutRecs[11] = {state.mainMenu.anchor04.x - ScaleSize(184, scale.uniformScale), state.mainMenu.anchor04.y + ScaleSize(240, scale.uniformScale), ScaleSize(112, scale.uniformScale), ScaleSize(32, scale.uniformScale)};    // Button: partyUsernameButton4
    state.mainMenu.layoutRecs[12] = {state.mainMenu.anchor04.x - ScaleSize(184, scale.uniformScale), state.mainMenu.anchor04.y + ScaleSize(192, scale.uniformScale), ScaleSize(112, scale.uniformScale), ScaleSize(32, scale.uniformScale)};    // Button: partyUsernameButton3
    state.mainMenu.layoutRecs[13] = {state.mainMenu.anchor04.x - ScaleSize(184, scale.uniformScale), state.mainMenu.anchor04.y + ScaleSize(144, scale.uniformScale), ScaleSize(112, scale.uniformScale), ScaleSize(32, scale.uniformScale)};    // Button: partyUsernameButton2
    state.mainMenu.layoutRecs[14] = {state.mainMenu.anchor04.x - ScaleSize(184, scale.uniformScale), state.mainMenu.anchor04.y + ScaleSize(96, scale.uniformScale), ScaleSize(112, scale.uniformScale), ScaleSize(32, scale.uniformScale)};    // Button: partyUsernameButton1
    state.mainMenu.layoutRecs[15] = {state.mainMenu.anchor04.x - ScaleSize(184, scale.uniformScale), state.mainMenu.anchor04.y + ScaleSize(24, scale.uniformScale), ScaleSize(112, scale.uniformScale), ScaleSize(32, scale.uniformScale)};    // Button: profileUsernameButton
    state.mainMenu.layoutRecs[16] = {state.mainMenu.anchor04.x - ScaleSize(64, scale.uniformScale), state.mainMenu.anchor04.y + ScaleSize(16, scale.uniformScale), ScaleSize(56, scale.uniformScale), ScaleSize(48, scale.uniformScale)};    // Button: profileIconButton
    state.mainMenu.layoutRecs[17] = {state.mainMenu.anchor04.x - ScaleSize(64, scale.uniformScale), state.mainMenu.anchor04.y + ScaleSize(88, scale.uniformScale), ScaleSize(56, scale.uniformScale), ScaleSize(48, scale.uniformScale)};    // Button: partyIconButton1
    state.mainMenu.layoutRecs[18] = {state.mainMenu.anchor04.x - ScaleSize(64, scale.uniformScale), state.mainMenu.anchor04.y + ScaleSize(136, scale.uniformScale), ScaleSize(56, scale.uniformScale), ScaleSize(48, scale.uniformScale)};    // Button: partyIconButton2
    state.mainMenu.layoutRecs[19] = {state.mainMenu.anchor04.x - ScaleSize(64, scale.uniformScale), state.mainMenu.anchor04.y + ScaleSize(184, scale.uniformScale), ScaleSize(56, scale.uniformScale), ScaleSize(48, scale.uniformScale)};    // Button: partyIconButton3
    state.mainMenu.layoutRecs[20] = {state.mainMenu.anchor04.x - ScaleSize(64, scale.uniformScale), state.mainMenu.anchor04.y + ScaleSize(232, scale.uniformScale), ScaleSize(56, scale.uniformScale), ScaleSize(48, scale.uniformScale)};    // Button: partyIconButton4
    state.mainMenu.layoutRecs[21] = {state.mainMenu.anchor04.x - ScaleSize(232, scale.uniformScale), state.mainMenu.anchor04.y + ScaleSize(104, scale.uniformScale), ScaleSize(32, scale.uniformScale), ScaleSize(16, scale.uniformScale)};    // Button: partyKickButton1
    state.mainMenu.layoutRecs[22] = {state.mainMenu.anchor04.x - ScaleSize(232, scale.uniformScale), state.mainMenu.anchor04.y + ScaleSize(152, scale.uniformScale), ScaleSize(32, scale.uniformScale), ScaleSize(16, scale.uniformScale)};    // Button: partyKickButton2
    state.mainMenu.layoutRecs[23] = {state.mainMenu.anchor04.x - ScaleSize(232, scale.uniformScale), state.mainMenu.anchor04.y + ScaleSize(200, scale.uniformScale), ScaleSize(32, scale.uniformScale), ScaleSize(16, scale.uniformScale)};    // Button: partyKickButton3
    state.mainMenu.layoutRecs[24] = {state.mainMenu.anchor04.x - ScaleSize(232, scale.uniformScale), state.mainMenu.anchor04.y + ScaleSize(248, scale.uniformScale), ScaleSize(32, scale.uniformScale), ScaleSize(16, scale.uniformScale)};    // Button: partyKickButton4
    state.mainMenu.layoutRecs[25] = {state.mainMenu.anchor04.x - ScaleSize(184, scale.uniformScale), state.mainMenu.anchor04.y + ScaleSize(296, scale.uniformScale), ScaleSize(80, scale.uniformScale), ScaleSize(24, scale.uniformScale)};    // Button: partyLeaveButton
    state.mainMenu.layoutRecs[26] = {state.mainMenu.anchor04.x - ScaleSize(96, scale.uniformScale), state.mainMenu.anchor04.y + ScaleSize(296, scale.uniformScale), ScaleSize(80, scale.uniformScale), ScaleSize(24, scale.uniformScale)};     // Button: partyInviteButton
    state.mainMenu.layoutRecs[27] = {state.mainMenu.anchor01.x + ScaleSize(152, scale.uniformScale), state.mainMenu.anchor01.y - ScaleSize(344, scale.uniformScale), ScaleSize(144, scale.uniformScale), ScaleSize(48, scale.uniformScale)};    // Button: exitButton
  }

  if (GuiButton(state.mainMenu.layoutRecs[0], state.mainMenu.realtimeButtonText)) RealtimeButton(); 
  if (GuiButton(state.mainMenu.layoutRecs[1], state.mainMenu.arenaButtonText)) ArenaButton(); 
  if (GuiButton(state.mainMenu.layoutRecs[2], state.mainMenu.sandboxButtonText)) SandboxButton(); 
  if (GuiButton(state.mainMenu.layoutRecs[3], state.mainMenu.customGameButtonText)) CustomGameButton(); 
  if (GuiButton(state.mainMenu.layoutRecs[4], state.mainMenu.settingsButtonText)) SettingsButton(state); 
  GuiScrollPanel(
    {state.mainMenu.layoutRecs[5].x, state.mainMenu.layoutRecs[5].y, 
     state.mainMenu.layoutRecs[5].width - state.mainMenu.chatScrollWindowBoundsOffset.x, 
     state.mainMenu.layoutRecs[5].height - state.mainMenu.chatScrollWindowBoundsOffset.y},
    state.mainMenu.chatScrollWindowText,
    state.mainMenu.layoutRecs[5],
    &state.mainMenu.chatScrollWindowScrollOffset,
    &state.mainMenu.chatScrollWindowScrollView
  );
  GuiGroupBox(state.mainMenu.layoutRecs[6], state.mainMenu.musicSelectorGroupBoxText);
  GuiListView(state.mainMenu.layoutRecs[7], state.mainMenu.songsListViewText, &state.mainMenu.songsListViewScrollIndex, &state.mainMenu.songsListViewActive);
  if (GuiSpinner(state.mainMenu.layoutRecs[8], state.mainMenu.albumSpinnerText, &state.mainMenu.albumSpinnerValue, 0, 100, state.mainMenu.albumSpinnerEditMode)) state.mainMenu.albumSpinnerEditMode = !state.mainMenu.albumSpinnerEditMode;
  GuiGroupBox(state.mainMenu.layoutRecs[9], state.mainMenu.partyGroupBoxText);
  GuiGroupBox(state.mainMenu.layoutRecs[10], state.mainMenu.profileGroupBoxText);
  if (GuiButton(state.mainMenu.layoutRecs[11], state.mainMenu.partyUsernameButton4Text)) PartyUsernameButton4(); 
  if (GuiButton(state.mainMenu.layoutRecs[12], state.mainMenu.partyUsernameButton3Text)) PartyUsernameButton3(); 
  if (GuiButton(state.mainMenu.layoutRecs[13], state.mainMenu.partyUsernameButton2Text)) PartyUsernameButton2(); 
  if (GuiButton(state.mainMenu.layoutRecs[14], state.mainMenu.partyUsernameButton1Text)) PartyUsernameButton1(); 
  if (GuiButton(state.mainMenu.layoutRecs[15], state.mainMenu.profileUsernameButtonText)) ProfileUsernameButton(); 
  if (GuiButton(state.mainMenu.layoutRecs[16], state.mainMenu.profileIconButtonText)) ProfileIconButton(); 
  if (GuiButton(state.mainMenu.layoutRecs[17], state.mainMenu.partyIconButton1Text)) PartyIconButton1(); 
  if (GuiButton(state.mainMenu.layoutRecs[18], state.mainMenu.partyIconButton2Text)) PartyIconButton2(); 
  if (GuiButton(state.mainMenu.layoutRecs[19], state.mainMenu.partyIconButton3Text)) PartyIconButton3(); 
  if (GuiButton(state.mainMenu.layoutRecs[20], state.mainMenu.partyIconButton4Text)) PartyIconButton4(); 
  if (GuiButton(state.mainMenu.layoutRecs[21], state.mainMenu.partyKickButton1Text)) PartyKickButton1(); 
  if (GuiButton(state.mainMenu.layoutRecs[22], state.mainMenu.partyKickButton2Text)) PartyKickButton2(); 
  if (GuiButton(state.mainMenu.layoutRecs[23], state.mainMenu.partyKickButton3Text)) PartyKickButton3(); 
  if (GuiButton(state.mainMenu.layoutRecs[24], state.mainMenu.partyKickButton4Text)) PartyKickButton4(); 
  if (GuiButton(state.mainMenu.layoutRecs[25], state.mainMenu.partyLeaveButtonText)) PartyLeaveButton(); 
  if (GuiButton(state.mainMenu.layoutRecs[26], state.mainMenu.partyInviteButtonText)) PartyInviteButton(); 
  if (GuiButton(state.mainMenu.layoutRecs[27], state.mainMenu.exitButtonText)) ExitButton(); 
}

void RealtimeButton() {
  // TODO: Implement control logic
}

void ArenaButton() {
  // TODO: Implement control logic
}

void SandboxButton() {
  // TODO: Implement control logic
}

void CustomGameButton() {
  // TODO: Implement control logic
}

void SettingsButton(GUI& state) {
  state.settingsMenu.active = !state.settingsMenu.active;
}

void PartyUsernameButton4() {
  // TODO: Implement control logic
}

void PartyUsernameButton3() {
  // TODO: Implement control logic
}

void PartyUsernameButton2() {
  // TODO: Implement control logic
}

void PartyUsernameButton1() {
  // TODO: Implement control logic
}

void ProfileUsernameButton() {
  // TODO: Implement control logic
}

void ProfileIconButton() {
  // TODO: Implement control logic
}

void PartyIconButton1() {
  // TODO: Implement control logic
}

void PartyIconButton2() {
  // TODO: Implement control logic
}

void PartyIconButton3() {
  // TODO: Implement control logic
}

void PartyIconButton4() {
  // TODO: Implement control logic
}

void PartyKickButton1() {
  // TODO: Implement control logic
}

void PartyKickButton2() {
  // TODO: Implement control logic
}

void PartyKickButton3() {
  // TODO: Implement control logic
}

void PartyKickButton4() {
  // TODO: Implement control logic
}

void PartyLeaveButton() {
  // TODO: Implement control logic
}

void PartyInviteButton() {
  // TODO: Implement control logic
}

void ExitButton() {
  // TODO: Implement control logic
}
