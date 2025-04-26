#include "main_menu.h"
#include "raygui.h"

const char *realtimeButtonText = "Realtime (1885)";    // BUTTON: realtimeButton
const char *arenaButtonText = "Arena (1756)";    // BUTTON: arenaButton
const char *sandboxButtonText = "Sandbox";    // BUTTON: sandboxButton
const char *customGameButtonText = "Custom game";    // BUTTON: customGameButton
const char *settingsButtonText = "Settings";    // BUTTON: settingsButton
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
const char *musicSelectorPanelText = "";    // PANEL: musicSelectorPanel
const char *albumSpinnerText = "";  // SPINNER: albumSpinner
const char *chatScrollWindowText = ""; // SCROLLWINDOW: chatScrollWindow

// Define anchors
Vector2 anchor01 = { 24, 936 };            // ANCHOR ID:1
Vector2 anchor02 = { 24, 24 };            // ANCHOR ID:2
Vector2 anchor03 = { 1648, 936 };            // ANCHOR ID:3
Vector2 anchor04 = { 1648, 24 };            // ANCHOR ID:4

// Define controls variables
Rectangle chatScrollWindowScrollView = { 0, 0, 0, 0 };
Vector2 chatScrollWindowScrollOffset = { 0, 0 };
Vector2 chatScrollWindowBoundsOffset = { 0, 0 };            // ScrollPanel: chatScrollWindow
int songsListViewScrollIndex = 0;
int songsListViewActive = 0;            // ListView: songsListView
bool albumSpinnerEditMode = false;
int albumSpinnerValue = 0;            // Spinner: albumSpinner

void DrawMainMenu() {
    // TODO: re-calculate where anchor points should be & the size of each box should be & the font size
    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);

    Rectangle layoutRecs[27] = {
        {anchor02.x + 0, anchor02.y + 0, 144, 48},    // Button: realtimeButton
        {anchor02.x + 0, anchor02.y + 56, 144, 48},    // Button: arenaButton
        {anchor02.x + 0, anchor02.y + 168, 144, 48},    // Button: sandboxButton
        {anchor02.x + 0, anchor02.y + 112, 144, 48},    // Button: customGameButton
        {anchor01.x + 0, anchor01.y - 344, 144, 48},    // Button: settingsButton
        {anchor01.x + 0, anchor01.y - 288, 576, 288},    // ScrollPanel: chatScrollWindow
        {anchor03.x - 136, anchor03.y - 288, 136, 288},    // Panel: musicSelectorPanel
        {anchor03.x - 128, anchor03.y - 248, 120, 240},    // ListView: songsListView
        {anchor03.x - 128, anchor03.y - 280, 120, 24},    // Spinner: albumSpinner
        {anchor04.x - 192, anchor04.y + 80, 192, 208},    // GroupBox: partyGroupBox
        {anchor04.x - 192, anchor04.y + 8, 192, 64},    // GroupBox: profileGroupBox
        {anchor04.x - 184, anchor04.y + 240, 112, 32},    // Button: partyUsernameButton4
        {anchor04.x - 184, anchor04.y + 192, 112, 32},    // Button: partyUsernameButton3
        {anchor04.x - 184, anchor04.y + 144, 112, 32},    // Button: partyUsernameButton2
        {anchor04.x - 184, anchor04.y + 96, 112, 32},    // Button: partyUsernameButton1
        {anchor04.x - 184, anchor04.y + 24, 112, 32},    // Button: profileUsernameButton
        {anchor04.x - 64, anchor04.y + 16, 56, 48},    // Button: profileIconButton
        {anchor04.x - 64, anchor04.y + 88, 56, 48},    // Button: partyIconButton1
        {anchor04.x - 64, anchor04.y + 136, 56, 48},    // Button: partyIconButton2
        {anchor04.x - 64, anchor04.y + 184, 56, 48},    // Button: partyIconButton3
        {anchor04.x - 64, anchor04.y + 232, 56, 48},    // Button: partyIconButton4
        {anchor04.x - 232, anchor04.y + 104, 32, 16},    // Button: partyKickButton1
        {anchor04.x - 232, anchor04.y + 152, 32, 16},    // Button: partyKickButton2
        {anchor04.x - 232, anchor04.y + 200, 32, 16},    // Button: partyKickButton3
        {anchor04.x - 232, anchor04.y + 248, 32, 16},    // Button: partyKickButton4
        {anchor04.x - 184, anchor04.y + 296, 80, 24},    // Button: partyLeaveButton
        {anchor04.x - 96, anchor04.y + 296, 80, 24}     // Button: partyInviteButton
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
    GuiPanel(layoutRecs[6], musicSelectorPanelText);
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
    // TODO: Implement control logic
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
