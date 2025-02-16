#include "game_state.h"
#include "entt.hpp"
#include "utils.h"
#include "isteamuser.h"
#include "isteamuserstats.h"
#include "steam_api.h"
#include "steamtypes.h"
#include "isteammatchmaking.h"
#include "isteamnetworkingutils.h"
#include "isteamnetworkingsockets.h"
#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

#define RAYGUI_IMPLEMENTATION
#define RAYGUI_SUPPORT_ICONS
#include "raygui.h"

#define GUI_GUI_IMPLEMENTATION
#include "gui.h"

#define RRES_IMPLEMENTATION
#include "rres.h"

#define RRES_RAYLIB_IMPLEMENTATION
#define RRES_SUPPORT_COMPRESSION_LZ4
#define RRES_SUPPORT_ENCRYPTION_AES
#define RRES_SUPPORT_ENCRYPTION_XCHACHA20
#include "rres-raylib.h"

#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

#define GLSL_VERSION 330

#define VIRTUAL_PORT 27017
#define APP_ID 2415880

static ISteamNetworkingSockets* g_pNetworkingSockets = nullptr;
static HSteamNetConnection g_hConnection = k_HSteamNetConnection_Invalid;

class ServerListResponse : public ISteamMatchmakingServerListResponse {
public:
    virtual void ServerResponded(HServerListRequest hRequest, int iServer) {
        gameserveritem_t* pServer = SteamMatchmakingServers()->GetServerDetails(hRequest, iServer);
        if (pServer) {
            LOG_TRACE("Found server: %s (ID: %llu)", 
                pServer->GetName(), 
                pServer->m_steamID.ConvertToUint64());
            LOG_TRACE("Players: %d/%d, Ping: %d", 
                pServer->m_nPlayers, 
                pServer->m_nMaxPlayers, 
                pServer->m_nPing);
            LOG_TRACE("Tags: %s", pServer->m_szGameTags);
            LOG_TRACE("Map: %s", pServer->m_szMap);
            
            SteamNetworkingIdentity serverIdentity;
            serverIdentity.Clear();
            serverIdentity.SetSteamID(pServer->m_steamID);

            g_pNetworkingSockets = SteamNetworkingSockets();
            if (!g_pNetworkingSockets) {
                LOG_ERROR("Failed to get networking interface");
                return;
            }

            // Configure networking options for SDR-only to match server
            SteamNetworkingConfigValue_t opts[2];
            int opt = 0;
            opts[opt++].SetInt32(k_ESteamNetworkingConfig_P2P_Transport_ICE_Enable, 0);  // Disable ICE completely
            opts[opt++].SetInt32(k_ESteamNetworkingConfig_P2P_Transport_ICE_Penalty, 10000);  // Set very high penalty for ICE

            LOG_TRACE("Attempting P2P SDR connection to server %llu", serverIdentity.GetSteamID64());

            g_hConnection = g_pNetworkingSockets->ConnectP2P(
                serverIdentity,
                VIRTUAL_PORT,
                opt,
                opts
            );

            if (g_hConnection == k_HSteamNetConnection_Invalid) {
                LOG_ERROR("Failed to create connection");
                return;
            }

            LOG_TRACE("Connection attempt initiated, connection ID: %u", g_hConnection);
        }
    }

    virtual void ServerFailedToRespond(HServerListRequest hRequest, int iServer) {
        gameserveritem_t* pServer = SteamMatchmakingServers()->GetServerDetails(hRequest, iServer);
        if (pServer) {
            LOG_ERROR("Server failed to respond - ID: %llu, Name: %s", 
                pServer->m_steamID.ConvertToUint64(),
                pServer->GetName());
        } else {
            LOG_ERROR("Unknown server %d failed to respond", iServer);
        }
    }
    
    virtual void RefreshComplete(HServerListRequest hRequest, EMatchMakingServerResponse response) {
        LOG_TRACE("Server refresh complete, response: %d", response);
        switch (response) {
            case eServerResponded:
                LOG_TRACE("Servers found successfully");
                break;
            case eServerFailedToRespond:
                LOG_ERROR("All servers failed to respond");
                break;
            case eNoServersListedOnMasterServer:
                LOG_ERROR("No servers found on master server - verify server registration");
                break;
            default:
                LOG_ERROR("Unknown server list refresh response: %d", response);
                break;
        }
    }
};

static ServerListResponse* g_pServerListResponse = nullptr;

void SearchForServers() {
    LOG_TRACE("Initiating server search...");

    ISteamNetworkingUtils* utils;
    utils = SteamNetworkingUtils();
    utils->InitRelayNetworkAccess();

    ISteamMatchmakingServers* matchmaking = SteamMatchmakingServers();
    if (!matchmaking) {
        LOG_ERROR("Failed to get matchmaking interface");
        return;
    }

    // Create filters for our specific game
    MatchMakingKeyValuePair_t filter1;
    MatchMakingKeyValuePair_t filter2;
    MatchMakingKeyValuePair_t filter3;
    MatchMakingKeyValuePair_t filter4;
    MatchMakingKeyValuePair_t filter5;

    // Filter #1: AppID
    strcpy(filter1.m_szKey, "appid");
    strcpy(filter1.m_szValue, std::to_string(APP_ID).c_str());
    
    // Filter #2: Dedicated servers only
    strcpy(filter2.m_szKey, "dedicated");
    strcpy(filter2.m_szValue, "1");

    // Filter #3: VAC secured servers only
    strcpy(filter3.m_szKey, "secure");
    strcpy(filter3.m_szValue, "0");

    // Filter #4: show SDR servers
    strcpy(filter4.m_szKey, "proxy");
    strcpy(filter4.m_szValue, "1");

    // Filter #5: Localhost only NOTE: 
    strcpy(filter5.m_szKey, "localhost");
    strcpy(filter5.m_szValue, "1");

    g_pServerListResponse = new ServerListResponse();
    MatchMakingKeyValuePair_t* pFilters[] = { &filter1, &filter2, &filter3, &filter4, &filter5 };

    LOG_TRACE("Requesting server list for App ID: %d", APP_ID);

    HServerListRequest hRequest = matchmaking->RequestInternetServerList(
        APP_ID,
        pFilters,
        5,
        g_pServerListResponse
    );

    if (hRequest == NULL) {
        LOG_ERROR("Failed to initiate server list request");
    } else {
        LOG_TRACE("Server list request initiated successfully");
    }
}

void render(GameState* state) {
    BeginDrawing();
        ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
        BeginMode3D(state->camera);

            auto shaderView = state->registry.view<Shader>();
            LOG_ASSERT(shaderView.size() == 1, "Expected exactly one shader");
            auto shaderEntity = shaderView.front();
            auto& shader = state->registry.get<Shader>(shaderEntity);

            BeginShaderMode(shader);
                DrawPlane(Vector3Zero(), (Vector2) { 10.0, 10.0 }, WHITE);
                DrawCube(Vector3Zero(), 2.0, 4.0, 2.0, WHITE);
            EndShaderMode();

            // Iterate through all entities with Light component
            auto lightView = state->registry.view<Light>();
            for (auto [entity, light] : lightView.each()) {
                if (light.enabled) DrawSphereEx(light.position, 0.2f, 8, 8, light.color);
                else DrawSphereWires(light.position, 0.2f, 8, 8, ColorAlpha(light.color, 0.3f));
            }

            DrawGrid(10, 1.0f);

        EndMode3D();

        DrawFPS(10, 10);

        DrawText("Use key [Y] to toggle lights", 10, 40, 20, GetColor(GuiGetStyle(DEFAULT, LINE_COLOR)));

        GuiGui(&state->gui);
    EndDrawing();
}

void init(GameState* state) {
    rresCentralDir dir = rresLoadCentralDirectory("resources.rres");

    int idMesh = rresGetResourceId(dir, "rover.glb");
    rresResourceMulti multiMesh = rresLoadResourceMulti("resources.rres", idMesh);
    int result = -1;
    for(int i = 0; i < multiMesh.count; ++i) {
        result = UnpackResourceChunk(&multiMesh.chunks[i]);
        if(result != RRES_SUCCESS) break;
    }
    Mesh mesh = { 0 };
    if(result == RRES_SUCCESS) {
        mesh = LoadMeshFromResource(multiMesh);
        rresUnloadResourceMulti(multiMesh);
    }
    UnloadMesh(mesh);

    int idIcons = rresGetResourceId(dir, "icons.rgi");
    rresResourceChunk chunkIcons = rresLoadResourceChunk("resources.rres", idIcons);
    if(UnpackResourceChunk(&chunkIcons) == RRES_SUCCESS) {
        GuiLoadIconsFromMemory((const unsigned char*) chunkIcons.data.raw, chunkIcons.info.baseSize, "icons");
    }
    rresUnloadResourceChunk(chunkIcons);

    rresUnloadCentralDirectory(dir);

    LOG_TRACE("Initializing Steam...");
    if (SteamAPI_Init()) {
        LOG_TRACE("Steam API initialized successfully!");

        if (SteamUser()->BLoggedOn()) {
            const char* playerName = SteamFriends()->GetPersonaName();
            const uint64 steamID = SteamUser()->GetSteamID().ConvertToUint64();
            LOG_TRACE("Steam user logged in - Name: %s, ID: %llu", playerName, steamID);
            
            const int MAX_TICKET_SIZE = 1024;
            unsigned char ticketBuffer[MAX_TICKET_SIZE];
            uint32 ticketSize;
            
            HAuthTicket sessionTicket = SteamUser()->GetAuthSessionTicket(
                ticketBuffer, 
                MAX_TICKET_SIZE, 
                &ticketSize,
                nullptr
            );

            if (sessionTicket != k_HAuthTicketInvalid) {
                LOG_TRACE("Auth ticket obtained successfully (size: %u)", ticketSize);
                SearchForServers();
            } else {
                LOG_ERROR("Failed to get auth ticket");
            }
        } else {
            LOG_ERROR("Steam user not logged in!");
            return;
        }
    } else {
        LOG_ERROR("Steam API initialization failed!");
        return;
    }

    const uint32 num_achievements = SteamUserStats()->GetNumAchievements();
    LOG_TRACE("num achievements: %u", num_achievements);

    //Hack used to check if this current load is a hot code reload
    bool isEcsDeinitialized = state->registry.storage<entt::entity>().empty();

    if(isEcsDeinitialized) {
        // Define the camera to look into our 3d world
        state->camera = { 0 };
        state->camera.position = (Vector3){ 2.0f, 4.0f, 6.0f };    // Camera position
        state->camera.target = (Vector3){ 0.0f, 0.5f, 0.0f };      // Camera looking at point
        state->camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
        state->camera.fovy = 45.0f;                                // Camera field-of-view Y
        state->camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

        rresCentralDir dir = rresLoadCentralDirectory("resources.rres");

        // Get resource IDs for shaders
        int vsId = rresGetResourceId(dir, "lighting.vs");
        int fsId = rresGetResourceId(dir, "lighting.fs");

        // Load shader chunks
        rresResourceChunk vsChunk = rresLoadResourceChunk("resources.rres", vsId);
        rresResourceChunk fsChunk = rresLoadResourceChunk("resources.rres", fsId);

        Shader shader = { 0 };
        if (UnpackResourceChunk(&vsChunk) == RRES_SUCCESS && 
            UnpackResourceChunk(&fsChunk) == RRES_SUCCESS) {
            // Load shader from memory
            shader = LoadShaderFromMemory(
                (const char*)vsChunk.data.raw,  // vertex shader code
                (const char*)fsChunk.data.raw   // fragment shader code
            );
        }

        // Clean up
        rresUnloadResourceChunk(vsChunk);
        rresUnloadResourceChunk(fsChunk);
        rresUnloadCentralDirectory(dir);

        // Get some required shader locations
        shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
        // NOTE: "matModel" location name is automatically assigned on shader loading, 
        // no need to get the location again if using that uniform name
        //shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(shader, "matModel");

        // Ambient light level (some basic lighting)
        int ambientLoc = GetShaderLocation(shader, "ambient");
        SetShaderValue(shader, ambientLoc, (float[4]){ 0.1f, 0.1f, 0.1f, 1.0f }, SHADER_UNIFORM_VEC4);

        auto shaderEntity = state->registry.create();
        state->registry.emplace<Shader>(shaderEntity, shader);

        Light yellowLight = CreateLight(LIGHT_POINT, (Vector3){ -2, 1, -2 }, Vector3Zero(), YELLOW, shader);
        auto yellowLightEntity = state->registry.create();
        state->registry.emplace<Light>(yellowLightEntity, yellowLight);

        Light redLight = CreateLight(LIGHT_POINT, (Vector3){ 2, 1, 2 }, Vector3Zero(), RED, shader);
        auto redLightEntity = state->registry.create();
        state->registry.emplace<Light>(redLightEntity, redLight);

        Light greenLight = CreateLight(LIGHT_POINT, (Vector3){ -2, 1, 2 }, Vector3Zero(), GREEN, shader);
        auto greenLightEntity = state->registry.create();
        state->registry.emplace<Light>(greenLightEntity, greenLight);

        Light blueLight = CreateLight(LIGHT_POINT, (Vector3){ 2, 1, -2 }, Vector3Zero(), BLUE, shader);
        auto blueLightEntity = state->registry.create();
        state->registry.emplace<Light>(blueLightEntity, blueLight);

        GuiGuiState gui_state = InitGuiGui();
        state->gui.gui_state = gui_state;
    }
}

void update(GameState* state) {
    // Run Steam callbacks
    SteamAPI_RunCallbacks();

    // Check connection state if we're connecting
    if (g_hConnection != k_HSteamNetConnection_Invalid) {
        SteamNetConnectionInfo_t info;
        g_pNetworkingSockets->GetConnectionInfo(g_hConnection, &info);

        static ESteamNetworkingConnectionState lastState = k_ESteamNetworkingConnectionState_None;
        if (info.m_eState != lastState) {
            lastState = info.m_eState;
            LOG_TRACE("Connection state changed to: %d", info.m_eState);
            
            switch (info.m_eState) {
                case k_ESteamNetworkingConnectionState_Connected:
                    LOG_TRACE("Connected to server!");
                    break;
                case k_ESteamNetworkingConnectionState_ClosedByPeer:
                    LOG_TRACE("Connection closed by server");
                    break;
                case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
                    LOG_TRACE("Connection problem detected: %s", info.m_szEndDebug);
                    break;
                case k_ESteamNetworkingConnectionState_Connecting:
                    LOG_TRACE("Establishing connection...");
                    break;
            }
        }

        if (info.m_eState == k_ESteamNetworkingConnectionState_ClosedByPeer ||
            info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally) {
            LOG_TRACE("Cleaning up failed connection");
            g_pNetworkingSockets->CloseConnection(g_hConnection, 0, nullptr, false);
            g_hConnection = k_HSteamNetConnection_Invalid;
        }
    }

    auto& camera = state->camera;

    UpdateCamera(&camera, CAMERA_ORBITAL);

    // Iterate through all entities with Shader component
    auto shaderView = state->registry.view<Shader>();
    LOG_ASSERT(shaderView.size() == 1, "Expected exactly one shader");
    auto shaderEntity = shaderView.front();
    auto& shader = state->registry.get<Shader>(shaderEntity);

    // Update the shader with the camera view vector (points towards { 0.0f, 0.0f, 0.0f })
    float cameraPos[3] = { camera.position.x, camera.position.y, camera.position.z };
    SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);

    // Iterate through all entities with Light component
    auto lightView = state->registry.view<Light>();
    for (auto [entity, light] : lightView.each()) {
        if (IsKeyPressed(KEY_Y)) { light.enabled = !light.enabled; }
        // Update light values (actually, only enable/disable them)
        UpdateLightValues(shader, light);
    }
}

extern "C" void client_main(GameState* state) {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(800, 450, "video game");
    SetTargetFPS(60);
    init(state);
    time_t last_write_time = get_timestamp("./libclient.so");
    while(!WindowShouldClose()) {
        if(last_write_time != get_timestamp("./libclient.so")) break;
        update(state);
        render(state);
    }
    
    // Cleanup networking
    if (g_hConnection != k_HSteamNetConnection_Invalid) {
        g_pNetworkingSockets->CloseConnection(g_hConnection, 0, "Shutting down", false);
        g_hConnection = k_HSteamNetConnection_Invalid;
    }

    auto shaderView = state->registry.view<Shader>();
    LOG_ASSERT(shaderView.size() == 1, "Expected exactly one shader");
    auto shaderEntity = shaderView.front();
    auto& shader = state->registry.get<Shader>(shaderEntity);
    UnloadShader(shader);
    
    delete g_pServerListResponse;
    SteamAPI_Shutdown();
    CloseWindow();
}
