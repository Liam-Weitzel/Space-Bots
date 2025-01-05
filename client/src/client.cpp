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
        BeginMode2D(state->camera);
            ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

            rlPushMatrix();
                rlTranslatef(0, 25*50, 0);
                rlRotatef(90, 1, 0, 0);
                DrawGrid(100, 50);
            rlPopMatrix();

            auto view = state->registry.view<const Position, const Renderable>();
            view.each([](const Position& pos, const Renderable& rend) {
                DrawCircleV(pos.pos, rend.radius, rend.color);
            });
        EndMode2D();

        Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), state->camera);
        DrawCircleV(GetMousePosition(), 4, GetColor(GuiGetStyle(DEFAULT, LINE_COLOR)));
        DrawTextEx(GuiGetFont(), 
            TextFormat("[%.0f, %.0f]", mousePos.x, mousePos.y),
            Vector2Add(GetMousePosition(), (Vector2){ -44, -24 }), 20, 2, GetColor(GuiGetStyle(DEFAULT, LINE_COLOR)));

        GuiGui(&state->gui);
    EndDrawing();
}

void init(GameState* state) {
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

    if(state->registry.storage<entt::entity>().empty()) {
        state->camera = Camera2D{0};
        state->camera.zoom = 1.0f;
        state->camera.offset = (Vector2){400, 225};

        for(int i = 0; i < 10; i++) {
            auto entity = state->registry.create();
            state->registry.emplace<Position>(entity, Vector2{
                static_cast<float>(GetRandomValue(0, 800)),
                static_cast<float>(GetRandomValue(0, 450))
            });
            state->registry.emplace<Renderable>(entity, 
                Color{
                    static_cast<unsigned char>(GetRandomValue(0, 255)),
                    static_cast<unsigned char>(GetRandomValue(0, 255)),
                    static_cast<unsigned char>(GetRandomValue(0, 255)),
                    255
                },
                static_cast<float>(GetRandomValue(10, 30))
            );
        }

        GuiLoadStyleDefault();
        GuiLoadIcons("assets/icons.rgi", "icons");
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

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        Vector2 delta = GetMouseDelta();
        delta = Vector2Scale(delta, -1.0f/camera.zoom);
        camera.target = Vector2Add(camera.target, delta);
    }

    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
        Vector2 mousePos = GetMousePosition();
        Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
        camera.offset = GetMousePosition();
        camera.target = mouseWorldPos;
        float scaleFactor = 1.0f + (0.25f*fabsf(wheel));

        if (wheel < 0) {
            scaleFactor = 1.0f/scaleFactor;
        }
        float newZoom = camera.zoom*scaleFactor;
        camera.zoom = Clamp(newZoom, 0.5f, 5.0f);
    }
}

extern "C" void client_main(GameState* state) {
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
    
    delete g_pServerListResponse;
    SteamAPI_Shutdown();
    CloseWindow();
}
