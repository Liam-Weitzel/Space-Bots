import ctypes
import os
import requests

current_dir = os.path.dirname(__file__)
libsteam_api = ctypes.CDLL(os.path.join(current_dir, 'libsteam_api.so'))
lib = ctypes.CDLL(os.path.join(current_dir, 'SteamworksPy.so'))

from steamworks import STEAMWORKS
from steamworks.exceptions import SteamNotRunningException

steamworks = STEAMWORKS()

try:
    steamworks.initialize()
except SteamNotRunningException:
    exit()

auth_token = steamworks.Users.GetAuthSessionTicket()
player_id = steamworks.Users.GetSteamID()
player_name = steamworks.Friends.GetPlayerName().decode('utf-8')
friends = ",".join([str(steamworks.Friends.GetFriendByIndex(i)) for i in range(steamworks.Friends.GetFriendCount())])
print(auth_token)
print(player_id)
print(player_name)
print(friends)

params = {
    "key": "KEYHERE", # see it on https://steamcommunity.com/dev/apikey
    "appid": "APPIDHERE",
    "ticket": auth_token
}

response = requests.get("https://api.steampowered.com/ISteamUserAuth/AuthenticateUserTicket/v1/", params=params)
print(response.json())

print("achievement_2: ", steamworks.UserStats.GetAchievementIcon("ACHIEVEMENT_2"))
print("first_launch: ", steamworks.UserStats.GetAchievementIcon("FIRST_LAUNCH"))
print("num of achievements: ", steamworks.UserStats.GetNumAchievements())
print("achievement name 0: ", steamworks.UserStats.GetAchievementName(0))
print("achievement name 1: ", steamworks.UserStats.GetAchievementName(1))
