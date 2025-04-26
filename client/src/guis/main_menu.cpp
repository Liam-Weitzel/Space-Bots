#include "main_menu.h"
#include "guis/settings.h"
#include "raygui.h"
#include "raylib.h"
#include "utils_client.h"
#include <cmath>

const char *realtimeButtonText = "Realtime (1885)";    // BUTTON: realtimeButton
const char *arenaButtonText = "Arena (1756)";    // BUTTON: arenaButton
const char *sandboxButtonText = "Sandbox";    // BUTTON: sandboxButton
const char *customGameButtonText = "Custom game";    // BUTTON: customGameButton
const char *settingsButtonText = "Settings";    // BUTTON: settingsButton
const char *exitButtonText = "Exit";    // BUTTON: exitButton
const char *songsListViewText = "ONE;TWO;THREE";    // LISTVIEW: songsListView
const char *partyGroupBoxText = "Party";    // GROUPBOX: partyGroupBox
const char *profileGroupBoxText = "Profile";    // GROUPBOX: profileGroupBox
const char *partyUsernameButton4Text = "USERNAME";    // BUTTON: partyUsernameButton4
const char *partyUsernameButton3Text = "USERNAME";    // BUTTON: partyUsernameButton3
const char *partyUsernameButton2Text = "USERNAME";    // BUTTON: partyUsernameButton2
const char *partyUsernameButton1Text = "USERNAME";    // BUTTON: partyUsernameButton1
const char *profileUsernameButtonText = "USERNAME";    // BUTTON: profileUsernameButton
const char *profileIconButtonText = "";    // BUTTON: profileIconButton
const char *partyIconButton1Text = "";    // BUTTON: partyIconButton1
const char *partyIconButton2Text = "";    // BUTTON: partyIconButton2
const char *partyIconButton3Text = "";    // BUTTON: partyIconButton3
const char *partyIconButton4Text = "";    // BUTTON: partyIconButton4
const char *partyKickButton1Text = "Kick";    // BUTTON: partyKickButton1
const char *partyKickButton2Text = "Kick";    // BUTTON: partyKickButton2
const char *partyKickButton3Text = "Kick";    // BUTTON: partyKickButton3
const char *partyKickButton4Text = "Kick";    // BUTTON: partyKickButton4
const char *partyLeaveButtonText = "Leave";    // BUTTON: partyLeaveButton
const char *partyInviteButtonText = "Invite";    // BUTTON: partyInviteButton
const char *musicSelectorGroupBoxText = "Music";    // GROUPBOX: musicSelectorGroupBox
const char *albumSpinnerText = "";  // SPINNER: albumSpinner
const char *chatScrollWindowText = ""; // SCROLLWINDOW: chatScrollWindow

// Define controls variables
Rectangle chatScrollWindowScrollView = { 0, 0, 0, 0 };
Vector2 chatScrollWindowScrollOffset = { 0, 0 };
Vector2 chatScrollWindowBoundsOffset = { 0, 0 };            // ScrollPanel: chatScrollWindow
int songsListViewScrollIndex = 0;
int songsListViewActive = 0;            // ListView: songsListView
bool albumSpinnerEditMode = false;
int albumSpinnerValue = 0;            // Spinner: albumSpinner
bool settingsActive = false;

void DrawMainMenu() {
    float width = GetScreenWidth();
    float height = GetScreenHeight();
    
    UIScale scale = CalculateUIScale();

    // Scale the base font size (12) with the UI
    int scaledFontSize = (int)ScaleSize(12.0f, scale.uniformScale);
    GuiSetStyle(DEFAULT, TEXT_SIZE, scaledFontSize);
    
    // Define anchors
    Vector2 anchor01 = { 24 * scale.scaleX, height - 24 * scale.scaleY };
    Vector2 anchor02 = { 24 * scale.scaleX, 24 * scale.scaleY };
    Vector2 anchor03 = { width - 24 * scale.scaleX, height - 24 * scale.scaleY };
    Vector2 anchor04 = { width - 24 * scale.scaleX, 24 * scale.scaleY };

    Rectangle layoutRecs[28] = {
        {anchor02.x, anchor02.y, ScaleSize(144, scale.uniformScale), ScaleSize(48, scale.uniformScale)},    // Button: realtimeButton
        {anchor02.x, anchor02.y + ScaleSize(56, scale.uniformScale), ScaleSize(144, scale.uniformScale), ScaleSize(48, scale.uniformScale)},    // Button: arenaButton
        {anchor02.x, anchor02.y + ScaleSize(168, scale.uniformScale), ScaleSize(144, scale.uniformScale), ScaleSize(48, scale.uniformScale)},    // Button: sandboxButton
        {anchor02.x, anchor02.y + ScaleSize(112, scale.uniformScale), ScaleSize(144, scale.uniformScale), ScaleSize(48, scale.uniformScale)},    // Button: customGameButton
        {anchor01.x, anchor01.y - ScaleSize(344, scale.uniformScale), ScaleSize(144, scale.uniformScale), ScaleSize(48, scale.uniformScale)},    // Button: settingsButton
        {anchor01.x, anchor01.y - ScaleSize(288, scale.uniformScale), ScaleSize(576, scale.uniformScale), ScaleSize(288, scale.uniformScale)},    // ScrollPanel: chatScrollWindow
        {anchor03.x - ScaleSize(136, scale.uniformScale), anchor03.y - ScaleSize(288, scale.uniformScale), ScaleSize(136, scale.uniformScale), ScaleSize(288, scale.uniformScale)},    // GroupBox: musicSelectorGroupBox
        {anchor03.x - ScaleSize(128, scale.uniformScale), anchor03.y - ScaleSize(248, scale.uniformScale), ScaleSize(120, scale.uniformScale), ScaleSize(240, scale.uniformScale)},    // ListView: songsListView
        {anchor03.x - ScaleSize(128, scale.uniformScale), anchor03.y - ScaleSize(280, scale.uniformScale), ScaleSize(120, scale.uniformScale), ScaleSize(24, scale.uniformScale)},    // Spinner: albumSpinner
        {anchor04.x - ScaleSize(192, scale.uniformScale), anchor04.y + ScaleSize(80, scale.uniformScale), ScaleSize(192, scale.uniformScale), ScaleSize(208, scale.uniformScale)},    // GroupBox: partyGroupBox
        {anchor04.x - ScaleSize(192, scale.uniformScale), anchor04.y + ScaleSize(8, scale.uniformScale), ScaleSize(192, scale.uniformScale), ScaleSize(64, scale.uniformScale)},    // GroupBox: profileGroupBox
        {anchor04.x - ScaleSize(184, scale.uniformScale), anchor04.y + ScaleSize(240, scale.uniformScale), ScaleSize(112, scale.uniformScale), ScaleSize(32, scale.uniformScale)},    // Button: partyUsernameButton4
        {anchor04.x - ScaleSize(184, scale.uniformScale), anchor04.y + ScaleSize(192, scale.uniformScale), ScaleSize(112, scale.uniformScale), ScaleSize(32, scale.uniformScale)},    // Button: partyUsernameButton3
        {anchor04.x - ScaleSize(184, scale.uniformScale), anchor04.y + ScaleSize(144, scale.uniformScale), ScaleSize(112, scale.uniformScale), ScaleSize(32, scale.uniformScale)},    // Button: partyUsernameButton2
        {anchor04.x - ScaleSize(184, scale.uniformScale), anchor04.y + ScaleSize(96, scale.uniformScale), ScaleSize(112, scale.uniformScale), ScaleSize(32, scale.uniformScale)},    // Button: partyUsernameButton1
        {anchor04.x - ScaleSize(184, scale.uniformScale), anchor04.y + ScaleSize(24, scale.uniformScale), ScaleSize(112, scale.uniformScale), ScaleSize(32, scale.uniformScale)},    // Button: profileUsernameButton
        {anchor04.x - ScaleSize(64, scale.uniformScale), anchor04.y + ScaleSize(16, scale.uniformScale), ScaleSize(56, scale.uniformScale), ScaleSize(48, scale.uniformScale)},    // Button: profileIconButton
        {anchor04.x - ScaleSize(64, scale.uniformScale), anchor04.y + ScaleSize(88, scale.uniformScale), ScaleSize(56, scale.uniformScale), ScaleSize(48, scale.uniformScale)},    // Button: partyIconButton1
        {anchor04.x - ScaleSize(64, scale.uniformScale), anchor04.y + ScaleSize(136, scale.uniformScale), ScaleSize(56, scale.uniformScale), ScaleSize(48, scale.uniformScale)},    // Button: partyIconButton2
        {anchor04.x - ScaleSize(64, scale.uniformScale), anchor04.y + ScaleSize(184, scale.uniformScale), ScaleSize(56, scale.uniformScale), ScaleSize(48, scale.uniformScale)},    // Button: partyIconButton3
        {anchor04.x - ScaleSize(64, scale.uniformScale), anchor04.y + ScaleSize(232, scale.uniformScale), ScaleSize(56, scale.uniformScale), ScaleSize(48, scale.uniformScale)},    // Button: partyIconButton4
        {anchor04.x - ScaleSize(232, scale.uniformScale), anchor04.y + ScaleSize(104, scale.uniformScale), ScaleSize(32, scale.uniformScale), ScaleSize(16, scale.uniformScale)},    // Button: partyKickButton1
        {anchor04.x - ScaleSize(232, scale.uniformScale), anchor04.y + ScaleSize(152, scale.uniformScale), ScaleSize(32, scale.uniformScale), ScaleSize(16, scale.uniformScale)},    // Button: partyKickButton2
        {anchor04.x - ScaleSize(232, scale.uniformScale), anchor04.y + ScaleSize(200, scale.uniformScale), ScaleSize(32, scale.uniformScale), ScaleSize(16, scale.uniformScale)},    // Button: partyKickButton3
        {anchor04.x - ScaleSize(232, scale.uniformScale), anchor04.y + ScaleSize(248, scale.uniformScale), ScaleSize(32, scale.uniformScale), ScaleSize(16, scale.uniformScale)},    // Button: partyKickButton4
        {anchor04.x - ScaleSize(184, scale.uniformScale), anchor04.y + ScaleSize(296, scale.uniformScale), ScaleSize(80, scale.uniformScale), ScaleSize(24, scale.uniformScale)},    // Button: partyLeaveButton
        {anchor04.x - ScaleSize(96, scale.uniformScale), anchor04.y + ScaleSize(296, scale.uniformScale), ScaleSize(80, scale.uniformScale), ScaleSize(24, scale.uniformScale)},     // Button: partyInviteButton
        {anchor01.x + ScaleSize(152, scale.uniformScale), anchor01.y - ScaleSize(344, scale.uniformScale), ScaleSize(144, scale.uniformScale), ScaleSize(48, scale.uniformScale)}    // Button: exitButton
    };

    if (GuiButton(layoutRecs[0], realtimeButtonText)) RealtimeButton(); 
    if (GuiButton(layoutRecs[1], arenaButtonText)) ArenaButton(); 
    if (GuiButton(layoutRecs[2], sandboxButtonText)) SandboxButton(); 
    if (GuiButton(layoutRecs[3], customGameButtonText)) CustomGameButton(); 
    if (GuiButton(layoutRecs[4], settingsButtonText)) SettingsButton(); 
    GuiScrollPanel(
        {layoutRecs[5].x, layoutRecs[5].y, 
         layoutRecs[5].width - chatScrollWindowBoundsOffset.x, 
         layoutRecs[5].height - chatScrollWindowBoundsOffset.y},
        chatScrollWindowText,
        layoutRecs[5],
        &chatScrollWindowScrollOffset,
        &chatScrollWindowScrollView
    );
    GuiGroupBox(layoutRecs[6], musicSelectorGroupBoxText);
    GuiListView(layoutRecs[7], songsListViewText, &songsListViewScrollIndex, &songsListViewActive);
    if (GuiSpinner(layoutRecs[8], albumSpinnerText, &albumSpinnerValue, 0, 100, albumSpinnerEditMode)) albumSpinnerEditMode = !albumSpinnerEditMode;
    GuiGroupBox(layoutRecs[9], partyGroupBoxText);
    GuiGroupBox(layoutRecs[10], profileGroupBoxText);
    if (GuiButton(layoutRecs[11], partyUsernameButton4Text)) PartyUsernameButton4(); 
    if (GuiButton(layoutRecs[12], partyUsernameButton3Text)) PartyUsernameButton3(); 
    if (GuiButton(layoutRecs[13], partyUsernameButton2Text)) PartyUsernameButton2(); 
    if (GuiButton(layoutRecs[14], partyUsernameButton1Text)) PartyUsernameButton1(); 
    if (GuiButton(layoutRecs[15], profileUsernameButtonText)) ProfileUsernameButton(); 
    if (GuiButton(layoutRecs[16], profileIconButtonText)) ProfileIconButton(); 
    if (GuiButton(layoutRecs[17], partyIconButton1Text)) PartyIconButton1(); 
    if (GuiButton(layoutRecs[18], partyIconButton2Text)) PartyIconButton2(); 
    if (GuiButton(layoutRecs[19], partyIconButton3Text)) PartyIconButton3(); 
    if (GuiButton(layoutRecs[20], partyIconButton4Text)) PartyIconButton4(); 
    if (GuiButton(layoutRecs[21], partyKickButton1Text)) PartyKickButton1(); 
    if (GuiButton(layoutRecs[22], partyKickButton2Text)) PartyKickButton2(); 
    if (GuiButton(layoutRecs[23], partyKickButton3Text)) PartyKickButton3(); 
    if (GuiButton(layoutRecs[24], partyKickButton4Text)) PartyKickButton4(); 
    if (GuiButton(layoutRecs[25], partyLeaveButtonText)) PartyLeaveButton(); 
    if (GuiButton(layoutRecs[26], partyInviteButtonText)) PartyInviteButton(); 
    if (GuiButton(layoutRecs[27], exitButtonText)) ExitButton(); 

    if(settingsActive) DrawSettings();
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

void SettingsButton() {
    settingsActive = !settingsActive;
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
