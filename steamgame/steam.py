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
