#include "main_menu.h"
#include "game_state.h"
#include "raygui.h"
#include "raylib.h"
#include "utils_client.h"
#include <cmath>

void UpdateMainMenu(MainMenu& m) {
  if(IsWindowResized()) m.dirty = true;
  if(m.dirty) {
    m.dirty = false;
    float width = GetScreenWidth();
    float height = GetScreenHeight();

    UIScale scale = CalculateUIScale();

    // Scale the base font size (12) with the UI
    int scaledFontSize = (int)ScaleSize(12.0f, scale.uniformScale);
    GuiSetStyle(DEFAULT, TEXT_SIZE, scaledFontSize);
    
    // Define anchors
    m.anchor01 = { 24 * scale.scaleX, height - 24 * scale.scaleY };
    m.anchor02 = { 24 * scale.scaleX, 24 * scale.scaleY };
    m.anchor03 = { width - 24 * scale.scaleX, height - 24 * scale.scaleY };
    m.anchor04 = { width - 24 * scale.scaleX, 24 * scale.scaleY };

    m.layoutRecs[0] = {m.anchor02.x, m.anchor02.y, ScaleSize(144, scale.uniformScale), ScaleSize(48, scale.uniformScale)};    // Button: realtimeButton
    m.layoutRecs[1] = {m.anchor02.x, m.anchor02.y + ScaleSize(56, scale.uniformScale), ScaleSize(144, scale.uniformScale), ScaleSize(48, scale.uniformScale)};    // Button: arenaButton
    m.layoutRecs[2] = {m.anchor02.x, m.anchor02.y + ScaleSize(168, scale.uniformScale), ScaleSize(144, scale.uniformScale), ScaleSize(48, scale.uniformScale)};    // Button: sandboxButton
    m.layoutRecs[3] = {m.anchor02.x, m.anchor02.y + ScaleSize(112, scale.uniformScale), ScaleSize(144, scale.uniformScale), ScaleSize(48, scale.uniformScale)};    // Button: customGameButton
    m.layoutRecs[4] = {m.anchor01.x, m.anchor01.y - ScaleSize(344, scale.uniformScale), ScaleSize(144, scale.uniformScale), ScaleSize(48, scale.uniformScale)};    // Button: settingsButton
    m.layoutRecs[5] = {m.anchor01.x, m.anchor01.y - ScaleSize(288, scale.uniformScale), ScaleSize(576, scale.uniformScale), ScaleSize(288, scale.uniformScale)};    // ScrollPanel: chatScrollWindow
    m.layoutRecs[6] = {m.anchor03.x - ScaleSize(136, scale.uniformScale), m.anchor03.y - ScaleSize(288, scale.uniformScale), ScaleSize(136, scale.uniformScale), ScaleSize(288, scale.uniformScale)};    // GroupBox: musicSelectorGroupBox
    m.layoutRecs[7] = {m.anchor03.x - ScaleSize(128, scale.uniformScale), m.anchor03.y - ScaleSize(248, scale.uniformScale), ScaleSize(120, scale.uniformScale), ScaleSize(240, scale.uniformScale)};    // ListView: songsListView
    m.layoutRecs[8] = {m.anchor03.x - ScaleSize(128, scale.uniformScale), m.anchor03.y - ScaleSize(280, scale.uniformScale), ScaleSize(120, scale.uniformScale), ScaleSize(24, scale.uniformScale)};    // Spinner: albumSpinner
    m.layoutRecs[9] = {m.anchor04.x - ScaleSize(192, scale.uniformScale), m.anchor04.y + ScaleSize(80, scale.uniformScale), ScaleSize(192, scale.uniformScale), ScaleSize(208, scale.uniformScale)};    // GroupBox: partyGroupBox
    m.layoutRecs[10] = {m.anchor04.x - ScaleSize(192, scale.uniformScale), m.anchor04.y + ScaleSize(8, scale.uniformScale), ScaleSize(192, scale.uniformScale), ScaleSize(64, scale.uniformScale)};    // GroupBox: profileGroupBox
    m.layoutRecs[11] = {m.anchor04.x - ScaleSize(184, scale.uniformScale), m.anchor04.y + ScaleSize(240, scale.uniformScale), ScaleSize(112, scale.uniformScale), ScaleSize(32, scale.uniformScale)};    // Button: partyUsernameButton4
    m.layoutRecs[12] = {m.anchor04.x - ScaleSize(184, scale.uniformScale), m.anchor04.y + ScaleSize(192, scale.uniformScale), ScaleSize(112, scale.uniformScale), ScaleSize(32, scale.uniformScale)};    // Button: partyUsernameButton3
    m.layoutRecs[13] = {m.anchor04.x - ScaleSize(184, scale.uniformScale), m.anchor04.y + ScaleSize(144, scale.uniformScale), ScaleSize(112, scale.uniformScale), ScaleSize(32, scale.uniformScale)};    // Button: partyUsernameButton2
    m.layoutRecs[14] = {m.anchor04.x - ScaleSize(184, scale.uniformScale), m.anchor04.y + ScaleSize(96, scale.uniformScale), ScaleSize(112, scale.uniformScale), ScaleSize(32, scale.uniformScale)};    // Button: partyUsernameButton1
    m.layoutRecs[15] = {m.anchor04.x - ScaleSize(184, scale.uniformScale), m.anchor04.y + ScaleSize(24, scale.uniformScale), ScaleSize(112, scale.uniformScale), ScaleSize(32, scale.uniformScale)};    // Button: profileUsernameButton
    m.layoutRecs[16] = {m.anchor04.x - ScaleSize(64, scale.uniformScale), m.anchor04.y + ScaleSize(16, scale.uniformScale), ScaleSize(56, scale.uniformScale), ScaleSize(48, scale.uniformScale)};    // Button: profileIconButton
    m.layoutRecs[17] = {m.anchor04.x - ScaleSize(64, scale.uniformScale), m.anchor04.y + ScaleSize(88, scale.uniformScale), ScaleSize(56, scale.uniformScale), ScaleSize(48, scale.uniformScale)};    // Button: partyIconButton1
    m.layoutRecs[18] = {m.anchor04.x - ScaleSize(64, scale.uniformScale), m.anchor04.y + ScaleSize(136, scale.uniformScale), ScaleSize(56, scale.uniformScale), ScaleSize(48, scale.uniformScale)};    // Button: partyIconButton2
    m.layoutRecs[19] = {m.anchor04.x - ScaleSize(64, scale.uniformScale), m.anchor04.y + ScaleSize(184, scale.uniformScale), ScaleSize(56, scale.uniformScale), ScaleSize(48, scale.uniformScale)};    // Button: partyIconButton3
    m.layoutRecs[20] = {m.anchor04.x - ScaleSize(64, scale.uniformScale), m.anchor04.y + ScaleSize(232, scale.uniformScale), ScaleSize(56, scale.uniformScale), ScaleSize(48, scale.uniformScale)};    // Button: partyIconButton4
    m.layoutRecs[21] = {m.anchor04.x - ScaleSize(232, scale.uniformScale), m.anchor04.y + ScaleSize(104, scale.uniformScale), ScaleSize(32, scale.uniformScale), ScaleSize(16, scale.uniformScale)};    // Button: partyKickButton1
    m.layoutRecs[22] = {m.anchor04.x - ScaleSize(232, scale.uniformScale), m.anchor04.y + ScaleSize(152, scale.uniformScale), ScaleSize(32, scale.uniformScale), ScaleSize(16, scale.uniformScale)};    // Button: partyKickButton2
    m.layoutRecs[23] = {m.anchor04.x - ScaleSize(232, scale.uniformScale), m.anchor04.y + ScaleSize(200, scale.uniformScale), ScaleSize(32, scale.uniformScale), ScaleSize(16, scale.uniformScale)};    // Button: partyKickButton3
    m.layoutRecs[24] = {m.anchor04.x - ScaleSize(232, scale.uniformScale), m.anchor04.y + ScaleSize(248, scale.uniformScale), ScaleSize(32, scale.uniformScale), ScaleSize(16, scale.uniformScale)};    // Button: partyKickButton4
    m.layoutRecs[25] = {m.anchor04.x - ScaleSize(184, scale.uniformScale), m.anchor04.y + ScaleSize(296, scale.uniformScale), ScaleSize(80, scale.uniformScale), ScaleSize(24, scale.uniformScale)};    // Button: partyLeaveButton
    m.layoutRecs[26] = {m.anchor04.x - ScaleSize(96, scale.uniformScale), m.anchor04.y + ScaleSize(296, scale.uniformScale), ScaleSize(80, scale.uniformScale), ScaleSize(24, scale.uniformScale)};     // Button: partyInviteButton
    m.layoutRecs[27] = {m.anchor01.x + ScaleSize(152, scale.uniformScale), m.anchor01.y - ScaleSize(344, scale.uniformScale), ScaleSize(144, scale.uniformScale), ScaleSize(48, scale.uniformScale)};    // Button: exitButton
  }
}

void DrawMainMenu(GUI& state) {
  MainMenu& m = state.mainMenu;
  if (GuiButton(m.layoutRecs[0], m.realtimeButtonText)) RealtimeButton(); 
  if (GuiButton(m.layoutRecs[1], m.arenaButtonText)) ArenaButton(); 
  if (GuiButton(m.layoutRecs[2], m.sandboxButtonText)) SandboxButton(); 
  if (GuiButton(m.layoutRecs[3], m.customGameButtonText)) CustomGameButton(); 
  if (GuiButton(m.layoutRecs[4], m.settingsButtonText)) SettingsButton(state.settingsMenu); 
  GuiScrollPanel(
    {m.layoutRecs[5].x, m.layoutRecs[5].y, 
     m.layoutRecs[5].width - m.chatScrollWindowBoundsOffset.x, 
     m.layoutRecs[5].height - m.chatScrollWindowBoundsOffset.y},
    m.chatScrollWindowText,
    m.layoutRecs[5],
    &m.chatScrollWindowScrollOffset,
    &m.chatScrollWindowScrollView
  );
  GuiGroupBox(m.layoutRecs[6], m.musicSelectorGroupBoxText);
  GuiListView(m.layoutRecs[7], m.songsListViewText, &m.songsListViewScrollIndex, &m.songsListViewActive);
  if (GuiSpinner(m.layoutRecs[8], m.albumSpinnerText, &m.albumSpinnerValue, 0, 100, m.albumSpinnerEditMode)) m.albumSpinnerEditMode = !m.albumSpinnerEditMode;
  GuiGroupBox(m.layoutRecs[9], m.partyGroupBoxText);
  GuiGroupBox(m.layoutRecs[10], m.profileGroupBoxText);
  if (GuiButton(m.layoutRecs[11], m.partyUsernameButton4Text)) PartyUsernameButton4(); 
  if (GuiButton(m.layoutRecs[12], m.partyUsernameButton3Text)) PartyUsernameButton3(); 
  if (GuiButton(m.layoutRecs[13], m.partyUsernameButton2Text)) PartyUsernameButton2(); 
  if (GuiButton(m.layoutRecs[14], m.partyUsernameButton1Text)) PartyUsernameButton1(); 
  if (GuiButton(m.layoutRecs[15], m.profileUsernameButtonText)) ProfileUsernameButton(); 
  if (GuiButton(m.layoutRecs[16], m.profileIconButtonText)) ProfileIconButton(); 
  if (GuiButton(m.layoutRecs[17], m.partyIconButton1Text)) PartyIconButton1(); 
  if (GuiButton(m.layoutRecs[18], m.partyIconButton2Text)) PartyIconButton2(); 
  if (GuiButton(m.layoutRecs[19], m.partyIconButton3Text)) PartyIconButton3(); 
  if (GuiButton(m.layoutRecs[20], m.partyIconButton4Text)) PartyIconButton4(); 
  if (GuiButton(m.layoutRecs[21], m.partyKickButton1Text)) PartyKickButton1(); 
  if (GuiButton(m.layoutRecs[22], m.partyKickButton2Text)) PartyKickButton2(); 
  if (GuiButton(m.layoutRecs[23], m.partyKickButton3Text)) PartyKickButton3(); 
  if (GuiButton(m.layoutRecs[24], m.partyKickButton4Text)) PartyKickButton4(); 
  if (GuiButton(m.layoutRecs[25], m.partyLeaveButtonText)) PartyLeaveButton(); 
  if (GuiButton(m.layoutRecs[26], m.partyInviteButtonText)) PartyInviteButton(); 
  if (GuiButton(m.layoutRecs[27], m.exitButtonText)) ExitButton(); 
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

void SettingsButton(SettingsMenu& state) {
  state.active = !state.active;
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
