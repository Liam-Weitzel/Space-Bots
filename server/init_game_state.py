def main():
    #Dummy player data (should be fetched from DB)
    player_units = [["ant","ant","spider"], ["ant","ant"]]

    #Dummy_game_settings
    map_size = [1080,1080]
    level_id = 7

    #Load unit attributes from somewhere (to be determined)
    unit_db = {
            "ant":{
                "max_hp":"100",
                "shape":{
                    "type":"ellipse",
                    "width":"2",
                    "height":"7"
                    },
                "mass":"5"
                },
            "spider":{
                "max_hp":"150",
                "shape":{
                    "type":"ellipse",
                    "width":"5",
                    "height":"7"
                    },
                "mass":"10"
                }
            }

    game_state = {"units":[]}

    for player, units in enumerate(player_units):
        for i, entity in enumerate(units):
            unit = {}
            unit["id"] = i
            unit["type"] = entity
            unit["player"] = player
            unit["unit_health"] = unit_db[entity]["max_hp"]
            unit["past_action"] = ""
            unit["lock_out_ticks"] = "0"
            if(player == 0):
                unit["location_x"] = "100"
            else:
                unit["location_x"] = str(map_size[0]-100)
            unit["location_y"] = str(round((map_size[1] / (len(units)+1)) * (i+1)))
            if(player == 0):
                unit["orientation"] = "90"
            else:
                unit["orientation"] = "270"
            unit["shape"] = unit_db[entity]["shape"]
            unit["mass"] = unit_db[entity]["mass"]
            game_state["units"].append(unit)

    return game_state

if __name__=="__main__":
    main()
