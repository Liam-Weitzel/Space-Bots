#include <chrono>
#include <thread>

#include "game_state.h"
#include "entt.hpp"
#include "utils.h"
#include "steam_gameserver.h"
#include "isteamgameserver.h"
#include "isteamnetworkingsockets.h"
#include "steamnetworkingtypes.h"
#include <vector>
#include <string>

#define STEAM_SERVER_PORT 27015
#define STEAM_QUERY_PORT 27016
#define VIRTUAL_PORT 27017

class ServerCallbacks;

static ISteamNetworkingSockets* g_pNetworkingSockets = nullptr;
static std::vector<HSteamNetConnection> g_activeConnections;
static ISteamGameServer* steamGameServer = nullptr;
static ServerCallbacks* g_pCallbacks = nullptr;

class ServerCallbacks {
public:
    ServerCallbacks() {
        LOG_TRACE("Server callbacks initialized");
    }
    
    ~ServerCallbacks() {
        LOG_TRACE("Server callbacks destroyed");
    }
    
    STEAM_CALLBACK(ServerCallbacks, OnSteamServersConnected, SteamServersConnected_t);
    STEAM_CALLBACK(ServerCallbacks, OnSteamServersDisconnected, SteamServersDisconnected_t);
    STEAM_CALLBACK(ServerCallbacks, OnSteamServersFailure, SteamServerConnectFailure_t);
    STEAM_CALLBACK(ServerCallbacks, OnValidateAuthTicketResponse, ValidateAuthTicketResponse_t);
    STEAM_CALLBACK(ServerCallbacks, OnNetConnectionStatusChanged, SteamNetConnectionStatusChangedCallback_t);
};

void ServerCallbacks::OnNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pCallback) {
    HSteamNetConnection hConn = pCallback->m_hConn;
    const SteamNetConnectionInfo_t& info = pCallback->m_info;
    ESteamNetworkingConnectionState eOldState = pCallback->m_eOldState;

    LOG_TRACE("Connection state changed - Old: %d, New: %d", eOldState, info.m_eState);

    // Handle new connection attempts
    if (info.m_hListenSocket && 
        eOldState == k_ESteamNetworkingConnectionState_None && 
        info.m_eState == k_ESteamNetworkingConnectionState_Connecting) {
        
        LOG_TRACE("New connection attempt from %llu", info.m_identityRemote.GetSteamID64());

        // Accept the connection
        EResult res = g_pNetworkingSockets->AcceptConnection(hConn);
        if (res != k_EResultOK) {
            LOG_ERROR("Failed to accept connection: %d", res);
            g_pNetworkingSockets->CloseConnection(hConn, 
                k_ESteamNetConnectionEnd_AppException_Generic, 
                "Failed to accept connection", 
                false);
            return;
        }

        // Add to active connections
        g_activeConnections.push_back(hConn);
        
        // Update server status
        steamGameServer->SetKeyValue("current_players", 
            std::to_string(g_activeConnections.size()).c_str());
        steamGameServer->SetKeyValue("status", "Game in progress");

        LOG_TRACE("Connection accepted successfully");
    }

    // Handle disconnections
    else if ((eOldState == k_ESteamNetworkingConnectionState_Connecting || 
              eOldState == k_ESteamNetworkingConnectionState_Connected) && 
             info.m_eState == k_ESteamNetworkingConnectionState_ClosedByPeer) {
        
        LOG_TRACE("Client disconnected: %llu", info.m_identityRemote.GetSteamID64());

        // Remove from active connections
        auto it = std::find(g_activeConnections.begin(), g_activeConnections.end(), hConn);
        if (it != g_activeConnections.end()) {
            g_activeConnections.erase(it);
            steamGameServer->SetKeyValue("current_players", 
                std::to_string(g_activeConnections.size()).c_str());
        }

        g_pNetworkingSockets->CloseConnection(hConn, 0, nullptr, false);
    }
}

void ServerCallbacks::OnSteamServersConnected(SteamServersConnected_t* pCallback) {
    LOG_TRACE("Steam servers reconnected!");
    if (!steamGameServer) {
        LOG_ERROR("Steam Game Server interface is null in callback!");
        return;
    }
    CSteamID serverID = steamGameServer->GetSteamID();
    LOG_TRACE("Server reconnected (Server ID: %llu)", serverID.ConvertToUint64());
}

void ServerCallbacks::OnSteamServersFailure(SteamServerConnectFailure_t* pCallback) {
    LOG_ERROR("Failed to connect to Steam: %d", pCallback->m_eResult);
}

void ServerCallbacks::OnSteamServersDisconnected(SteamServersDisconnected_t* pCallback) {
    LOG_ERROR("Server disconnected from Steam");
}

void ServerCallbacks::OnValidateAuthTicketResponse(ValidateAuthTicketResponse_t* pCallback) {
    LOG_TRACE("Auth ticket response - SteamID: %llu, Auth Result: %d", 
        pCallback->m_SteamID.ConvertToUint64(), 
        pCallback->m_eAuthSessionResponse);
}

extern "C" void server_main(GameState* state) {
    LOG_TRACE("Initializing Steam Game Server...");
    
    if (!SteamGameServer_Init(
            0,                     // IP Address (0 = localhost)
            STEAM_SERVER_PORT,    // Game port
            STEAM_QUERY_PORT,     // Query port
            eServerModeAuthenticationAndSecure,  // Server mode
            "1.0.0"
    )) {
        LOG_ERROR("Steam Game Server initialization failed!");
        return;
    }

    LOG_TRACE("Steam Game Server initialized successfully");

    steamGameServer = SteamGameServer();
    if (!steamGameServer) {
        LOG_ERROR("Failed to get Steam Game Server interface!");
        SteamGameServer_Shutdown();
        return;
    }

    // Create and store callbacks object
    g_pCallbacks = new ServerCallbacks();
    
    LOG_TRACE("Setting up server details...");
    
    // Set required server parameters
    steamGameServer->SetModDir("Realm Explorers");
    steamGameServer->SetProduct("Realm Explorers");
    steamGameServer->SetGameDescription("Multiplayer Adventure Game");
    
    // Set discovery properties
    steamGameServer->SetServerName("SDR-Only Server #1");
    steamGameServer->SetDedicatedServer(true);
    steamGameServer->SetMaxPlayerCount(32);
    steamGameServer->SetBotPlayerCount(0);
    steamGameServer->SetPasswordProtected(false);
    steamGameServer->SetGameTags("sdr_only,dedicated");
    
    // Set map and game data
    steamGameServer->SetMapName("default");
    steamGameServer->SetKeyValue("current_players", "0");
    steamGameServer->SetKeyValue("max_players", "32");
    steamGameServer->SetKeyValue("status", "Waiting for players");

    // Set region and spectator info
    steamGameServer->SetRegion("na");
    steamGameServer->SetSpectatorPort(STEAM_QUERY_PORT);
    steamGameServer->SetSpectatorServerName("Spectator");

    steamGameServer->SetKeyValue("localhost", "1");
    steamGameServer->SetAdvertiseServerActive(true);

    LOG_TRACE("Logging into Steam anonymously...");
    steamGameServer->LogOnAnonymous();

    LOG_TRACE("Initializing networking...");
    g_pNetworkingSockets = SteamGameServerNetworkingSockets();
    if (!g_pNetworkingSockets) {
        LOG_ERROR("Failed to initialize Steam networking!");
        delete g_pCallbacks;
        SteamGameServer_Shutdown();
        return;
    }

    // Wait for server to be logged in with timeout
    LOG_TRACE("Waiting for server login...");
    int timeout = 30; // 30 seconds timeout
    while (!steamGameServer->BLoggedOn() && timeout > 0) {
        SteamGameServer_RunCallbacks();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        timeout--;
        if (timeout % 5 == 0) { // Log every 5 seconds
            LOG_TRACE("Still waiting for login... %d seconds remaining", timeout);
        }
    }

    if (!steamGameServer->BLoggedOn()) {
        LOG_ERROR("Failed to log into Steam after 30 seconds!");
        delete g_pCallbacks;
        SteamGameServer_Shutdown();
        return;
    }

    LOG_TRACE("Server logged in successfully!");

    SteamNetworkingConfigValue_t opts[11];
    int opt = 0;
    // Configure networking options for SDR-only
    opts[opt++].SetInt32(k_ESteamNetworkingConfig_P2P_Transport_ICE_Enable, 0);       // Disable ICE completely
    opts[opt++].SetInt32(k_ESteamNetworkingConfig_P2P_Transport_ICE_Penalty, 10000);  // Set very high penalty for ICE
    // Max & min send rate
    opts[opt++].SetInt32(k_ESteamNetworkingConfig_SendRateMin, 512*1024);             // 512 KB/s minimum
    opts[opt++].SetInt32(k_ESteamNetworkingConfig_SendRateMax, 2048*1024);            // 2 MB/s maximum
    // Increase buffer sizes
    opts[opt++].SetInt32(k_ESteamNetworkingConfig_SendBufferSize, 1024*1024);         // 1MB send buffer
    opts[opt++].SetInt32(k_ESteamNetworkingConfig_RecvBufferSize, 1024*1024);         // 1MB receive buffer
    // Adjust timeout values
    opts[opt++].SetInt32(k_ESteamNetworkingConfig_TimeoutInitial, 30000);             // 30 seconds initial connect timeout
    opts[opt++].SetInt32(k_ESteamNetworkingConfig_TimeoutConnected, 20000);           // 20 seconds timeout for established connections
    // Nagle's algorithm - slightly increased for better packet coalescing
    opts[opt++].SetInt32(k_ESteamNetworkingConfig_NagleTime, 10000);                  // 10ms (default is 5ms)
    // Maximum message size
    opts[opt++].SetInt32(k_ESteamNetworkingConfig_RecvMaxMessageSize, 1024*1024);     // 1MB max message size
    // Enable Steam's network diagnostics UI
    opts[opt++].SetInt32(k_ESteamNetworkingConfig_EnableDiagnosticsUI, 1);            // Enable Steam's network diagnostics UI
    // Increase etworking timeout limit
    opts[opt++].SetInt32(k_ESteamNetworkingConfig_TimeoutInitial, 30000);
    opts[opt++].SetInt32(k_ESteamNetworkingConfig_TimeoutConnected, 60000);

    LOG_TRACE("Creating P2P listen socket on virtual port %d...", VIRTUAL_PORT);
    HSteamListenSocket listenSocket = g_pNetworkingSockets->CreateListenSocketP2P(
        VIRTUAL_PORT,
        opt,
        opts
    );

    if (listenSocket == k_HSteamListenSocket_Invalid) {
        char errorBuffer[1024];
        g_pNetworkingSockets->GetDetailedConnectionStatus(listenSocket, errorBuffer, sizeof(errorBuffer));
        LOG_ERROR("Failed to create SDR listen socket! Details: %s", errorBuffer);
        delete g_pCallbacks;
        SteamGameServer_Shutdown();
        return;
    }

    LOG_TRACE("Server is now publicly advertised (Server ID: %llu)", 
        steamGameServer->GetSteamID().ConvertToUint64());

    LOG_TRACE("Server started successfully");
    LOG_TRACE("Steam Server ID: %llu", steamGameServer->GetSteamID().ConvertToUint64());
    LOG_TRACE("Query Port: %d, Game Port: %d, Virtual Port: %d", 
              STEAM_QUERY_PORT, STEAM_SERVER_PORT, VIRTUAL_PORT);

    time_t last_write_time = get_timestamp("./libserver.so");
    while(last_write_time == get_timestamp("./libserver.so")) {
        SteamGameServer_RunCallbacks();

        static time_t next_check = 0;
        time_t now = time(nullptr);
        if (now >= next_check) {
            next_check = now + 5;  // Check every 5 seconds
            
            // Check server visibility
            LOG_WARN("Server Status Check:");
            LOG_TRACE("  >Logged on: %d", steamGameServer->BLoggedOn());
            LOG_TRACE("  >Secure: %d", steamGameServer->BSecure());
            LOG_TRACE("  >Server ID: %llu", steamGameServer->GetSteamID().ConvertToUint64());
            SteamIPAddress_t ipAddr = steamGameServer->GetPublicIP();
            char ipStr[64];
            snprintf(ipStr, sizeof(ipStr), "%d.%d.%d.%d", 
                (ipAddr.m_unIPv4 >> 24) & 0xFF,
                (ipAddr.m_unIPv4 >> 16) & 0xFF,
                (ipAddr.m_unIPv4 >> 8) & 0xFF,
                ipAddr.m_unIPv4 & 0xFF);
            LOG_TRACE("  >Server IP: %s", ipStr);
        }

        // Just handle messages from existing connections
        for (const auto& conn : g_activeConnections) {
            ISteamNetworkingMessage* pIncomingMsg[32];
            int numMsgs = g_pNetworkingSockets->ReceiveMessagesOnConnection(conn, pIncomingMsg, 32);
            
            if (numMsgs > 0) {
                LOG_TRACE("Received %d messages from connection %u", numMsgs, conn);
                
                for (int i = 0; i < numMsgs; i++) {
                    LOG_TRACE("Message size: %d bytes", pIncomingMsg[i]->m_cbSize);
                    pIncomingMsg[i]->Release();
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    LOG_TRACE("Server shutting down...");
    
    for (const auto& conn : g_activeConnections) {
        g_pNetworkingSockets->CloseConnection(conn, 0, "Server shutting down", false);
    }
    g_activeConnections.clear();

    g_pNetworkingSockets->CloseListenSocket(listenSocket);
    
    delete g_pCallbacks;
    SteamGameServer_Shutdown();
    g_pNetworkingSockets = nullptr;
    
    LOG_TRACE("Server shutdown complete");
}
