import numpy as np
import pandas as pd
from shapely.geometry.point import Point
import shapely.affinity
from descartes import PolygonPatch
import shapely.affinity
import matplotlib.pyplot as plt


#Dummy player data (should be fetched from DB)
player1_units = ["ant","ant"]
player2_units = ["ant","ant","spider"]

#Dummy_game_settings
map_size = [1080,1080]
level_id = 7

#Load unit attributes from somewhere (to be determined)
unit_db = {"ant":{"max_hp":100}}

#Initialize game state
#d = ["unit_id": np.nan, "player": np.nan, "unit_health":np.nan, "past_action":np.nan, "lock_out_ticks":np.nan, "unit_hitbox": np.nan, "location_x":np.nan, "location_y":np.nan, "orientation":np.nan, "shape":np.nan]

unit = {"id":"","type":"","player":"","unit_health":"","past_action":"","lock_out_ticks":"","location_x":"","location_y":"","orientation":"","shape":""}

game_state = {"units":[]}

i=0
for entity in player1_units:
    unit["id"] = i
    unit["type"] = entity
    unit["player"] = 1
    unit["unit_health"] = unit_db[entity]["max_hp"]
    unit["past_action"] = ""
    unit["lock_out_ticks"] = 0
    unit["location_x"] = 100
    unit["location_y"] = (1080 / len(player1_units)+2) * i+1
    unit["orientation"] = 90


    i+=1


ellipse = ((0, 0),(2, 7))
# Let create a circle of radius 1 around center point:
circ = shapely.geometry.Point(ellipse[0]).buffer(1)
# Let create the ellipse along x and y:
ell  = shapely.affinity.scale(circ, int(ellipse[1][0]), int(ellipse[1][1]))


from shapely.geometry import Polygon
import matplotlib.pyplot as plt

plt.rcParams["figure.figsize"] = [7.00, 3.50]
plt.rcParams["figure.autolayout"] = True
x, y = ell.exterior.xy
plt.plot(x, y, c="red")
plt.show()