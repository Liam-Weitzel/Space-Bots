import os

os.add_dll_directory(os.getcwd())

from steamworks import STEAMWORKS
from steamworks.exceptions import SteamNotRunningException
import requests

steamworks = STEAMWORKS()

try:
    steamworks.initialize()
    
except SteamNotRunningException:
    exit()
except OSError:
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
    "key": "API_KEY", # see it on https://steamcommunity.com/dev/apikey
    "appid": "480",
    "ticket": auth_token
}

response = requests.get("https://api.steampowered.com/ISteamUserAuth/AuthenticateUserTicket/v1/", params=params)
print(response.json())