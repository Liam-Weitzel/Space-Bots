#pragma once
#include "utils.h"
#include "utils_client.h"

struct MainMenu {
  char realtimeButtonText[32];
  char arenaButtonText[32];
  char sandboxButtonText[32];
  char customGameButtonText[32];
  char settingsButtonText[32];
  char exitButtonText[32];
  char songsListViewText[32];
  char partyGroupBoxText[32];
  char profileGroupBoxText[32];
  char partyUsernameButton4Text[32];
  char partyUsernameButton3Text[32];
  char partyUsernameButton2Text[32];
  char partyUsernameButton1Text[32];
  char profileUsernameButtonText[32];
  char profileIconButtonText[32];
  char partyIconButton1Text[32];
  char partyIconButton2Text[32];
  char partyIconButton3Text[32];
  char partyIconButton4Text[32];
  char partyKickButton1Text[32];
  char partyKickButton2Text[32];
  char partyKickButton3Text[32];
  char partyKickButton4Text[32];
  char partyLeaveButtonText[32];
  char partyInviteButtonText[32];
  char musicSelectorGroupBoxText[32];
  char albumSpinnerText[32];
  char chatScrollWindowText[32];

  Rectangle layoutRecs[28];
  Vector2 anchor01;
  Vector2 anchor02;
  Vector2 anchor03;
  Vector2 anchor04;
  Rectangle chatScrollWindowScrollView = { 0, 0, 0, 0 };
  Vector2 chatScrollWindowScrollOffset = { 0, 0 };
  Vector2 chatScrollWindowBoundsOffset = { 0, 0 };
  int songsListViewScrollIndex = 0;
  int songsListViewActive = 0;
  bool albumSpinnerEditMode = false;
  int albumSpinnerValue = 0;
  bool dirty = true;

  MainMenu() {
    strncpy(realtimeButtonText, "Realtime (1885)", sizeof(realtimeButtonText)-1);
    strncpy(arenaButtonText, "Arena (1756)", sizeof(arenaButtonText)-1);
    strncpy(sandboxButtonText, "Sandbox", sizeof(sandboxButtonText)-1);
    strncpy(customGameButtonText, "Custom game", sizeof(customGameButtonText)-1);
    strncpy(settingsButtonText, "Settings", sizeof(settingsButtonText)-1);
    strncpy(exitButtonText, "Exit", sizeof(exitButtonText)-1);
    strncpy(songsListViewText, "ONE;TWO;THREE", sizeof(songsListViewText)-1);
    strncpy(partyGroupBoxText, "Party", sizeof(partyGroupBoxText)-1);
    strncpy(profileGroupBoxText, "Profile", sizeof(profileGroupBoxText)-1);
    strncpy(partyUsernameButton4Text, "USERNAME", sizeof(partyUsernameButton4Text)-1);
    strncpy(partyUsernameButton3Text, "USERNAME", sizeof(partyUsernameButton3Text)-1);
    strncpy(partyUsernameButton2Text, "USERNAME", sizeof(partyUsernameButton2Text)-1);
    strncpy(partyUsernameButton1Text, "USERNAME", sizeof(partyUsernameButton1Text)-1);
    strncpy(profileUsernameButtonText, "USERNAME", sizeof(profileUsernameButtonText)-1);
    strncpy(profileIconButtonText, "", sizeof(profileIconButtonText)-1);
    strncpy(partyIconButton1Text, "", sizeof(partyIconButton1Text)-1);
    strncpy(partyIconButton2Text, "", sizeof(partyIconButton2Text)-1);
    strncpy(partyIconButton3Text, "", sizeof(partyIconButton3Text)-1);
    strncpy(partyIconButton4Text, "", sizeof(partyIconButton4Text)-1);
    strncpy(partyKickButton1Text, "Kick", sizeof(partyKickButton1Text)-1);
    strncpy(partyKickButton2Text, "Kick", sizeof(partyKickButton2Text)-1);
    strncpy(partyKickButton3Text, "Kick", sizeof(partyKickButton3Text)-1);
    strncpy(partyKickButton4Text, "Kick", sizeof(partyKickButton4Text)-1);
    strncpy(partyLeaveButtonText, "Leave", sizeof(partyLeaveButtonText)-1);
    strncpy(partyInviteButtonText, "Invite", sizeof(partyInviteButtonText)-1);
    strncpy(musicSelectorGroupBoxText, "Music", sizeof(musicSelectorGroupBoxText)-1);
    strncpy(albumSpinnerText, "", sizeof(albumSpinnerText)-1);
    strncpy(chatScrollWindowText, "", sizeof(chatScrollWindowText)-1);
  }
};

struct SettingsMenu {
  char settingsWindowBoxText[32];
  char musicSliderBarText[32];
  char sfxSliderBarText[32];
  char uiScaleSliderText[32];
  char screenDropdownBoxText[64];
  char screenLabelText[32];
  char LineText[32];
  char fpsValueBoxText[32];
  char uiStyleSpinnerText[32];
  char Line2Text[32];
  char cancelButtonText[32];
  char applyButtonText[32];

  Rectangle layoutRecs[12];
  Vector2 anchor01 = {0, 0};
  bool uiStyleSpinnerEditMode = false;
  int uiStyleSpinnerValue = 0;
  float musicSliderBarValue = 50.0f;
  float sfxSliderBarValue = 50.0f;
  bool fpsValueBoxEditMode = false;
  int fpsValueBoxValue = 120;
  float uiScaleSliderValue = 50.0f;
  bool screenDropdownBoxEditMode = false;
  int screenDropdownBoxActive = 0;
  bool isDragging = false;
  Vector2 dragOffset = {0, 0};
  bool dirty = true;
  bool active = false;

  SettingsMenu() {
    strncpy(settingsWindowBoxText, "Settings", sizeof(settingsWindowBoxText)-1);
    strncpy(musicSliderBarText, "Music", sizeof(musicSliderBarText)-1);
    strncpy(sfxSliderBarText, "SFX", sizeof(sfxSliderBarText)-1);
    strncpy(uiScaleSliderText, "UI Scale", sizeof(uiScaleSliderText)-1);
    strncpy(screenDropdownBoxText, "Window; Borderless; Fullscreen", sizeof(screenDropdownBoxText)-1);
    strncpy(screenLabelText, "Screen", sizeof(screenLabelText)-1);
    strncpy(LineText, "", sizeof(LineText)-1);
    strncpy(fpsValueBoxText, "FPS ", sizeof(fpsValueBoxText)-1);
    strncpy(uiStyleSpinnerText, "UI Style ", sizeof(uiStyleSpinnerText)-1);
    strncpy(Line2Text, "", sizeof(Line2Text)-1);
    strncpy(cancelButtonText, "Cancel", sizeof(cancelButtonText)-1);
    strncpy(applyButtonText, "Apply", sizeof(applyButtonText)-1);
  }
};

struct GUI { // Permanent
  MainMenu mainMenu;
  SettingsMenu settingsMenu;
};

struct RoverAssets { // Reload
  Model* body = nullptr;
  Vector3 bodyOffset = {0.0f, 0.5f, 0.0f};
  Vector3 bodyRotationAxis = {0.0f, 1.0f, 0.0f};
  Model* wheel = nullptr;
  Vector3 wheelOffsets[4] = {
    {0.0f, 0.5f, 0.0f},    // backLeft
    {-2.1f, 0.5f, 0.0f},   // backRight
    {0.0f, 0.5f, -1.25f},  // frontLeft
    {-2.1f, 0.5f, -1.25f}  // frontRight
  };
  Vector3 wheelRotationAxis = {0.0f, 1.0f, 0.0f};
  Model* scan = nullptr;
  Vector3 scanOffset = {0.0f, 0.5f, 0.0f};
  Vector3 scanRotationAxis = {0.0f, 1.0f, 0.0f};
  Color tint = WHITE;
};

struct ResourceManager { // Reload
  rresCentralDir dir;
  RoverAssets roverAssets;

  void reload() {
    // Don't unload models as they are freed when arena is cleared
    rresUnloadCentralDirectory(dir);
  }
};

struct Shaders { // Reload
  Shader shadowShader;
  RenderTexture2D shadowMap;
  Vector3 lightDir;
  int lightDirLoc;
  int lightVPLoc;
  int shadowMapLoc;

  void reload() {
    UnloadShader(shadowShader);
    UnloadRenderTexture(shadowMap);

    if (shadowMap.id > 0) rlUnloadFramebuffer(shadowMap.id);
  }
};

struct Cameras { // Permanent
  Camera3D camera;
  Camera3D lightCamera;
};

struct RenderResources {
  Cameras* cameras;                  // Permanent
  Shaders* shaders;                  // Reload
  ResourceManager* resourceManager;  // Reload
  rresCentralDir* dir;               // Reload
  GUI* gui;                          // Permanent

  void reload() {
    shaders->reload();
    resourceManager->reload();
    shaders = nullptr;
    resourceManager = nullptr;
    dir = nullptr;
  }
};

enum class GameMode {
  MENU,
  REALTIME,
  AUTOBATTLE
};

struct GameState {
  uint32_t frameCount = 0;
  float deltaTime = 0;
  GameMode gameMode = GameMode::MENU;

  // Direct pointers to static arena-managed resources
  RenderResources renderResources;

  // Arenas
  Arena frameArena;        // Clears every frame
  Arena matchArena;        // Clears every match
  Arena reloadArena;       // Clears on hot-reload
  Arena permanentArena;    // Doesn't clear on hot-reload

  GameState()
    : frameArena(KB(5))
    , matchArena(MB(5))
    , reloadArena(MB(50))
    , permanentArena(MB(100))
  {
    frameArena.create_map_ct<const char*, void*, ArenaIndexSize>();
    matchArena.create_map_ct<const char*, void*, ArenaIndexSize>();
    reloadArena.create_map_ct<const char*, void*, ArenaIndexSize>();
    permanentArena.create_map_ct<const char*, void*, ArenaIndexSize>();
  }
};

struct Rover { // Stored in generational sparse set in match arena
  Transform transform;
  ArrayCT<float, 4> wheelRotations; // [backLeft, backRight, frontLeft, frontRight]
  bool isScanning = false;
};
