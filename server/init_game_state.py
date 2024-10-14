def main():
    #Dummy player data (should be fetched from DB)
    player_units = [["acid_ant","acid_ant","rhino_beetle"], ["lava_ant","lava_ant"]]

    #Dummy_game_settings
    map_size = [1080,1080]
    level_id = 7

    #Load unit attributes from somewhere (to be determined)
    unit_db = {
            "acid_ant":{
                "max_hp":"100",
                "shape":{
                    "type":"ellipse",
                    "width":"2",
                    "height":"7"
                    },
                "mass":"5"
                },
            "lava_ant":{
                "max_hp":"200",
                "shape":{
                    "type":"ellipse",
                    "width":"2",
                    "height":"7"
                    },
                "mass":"5"
                },
            "rhino_beetle":{
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
        i = 0;
        for entity in units:
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
                unit["orientation"] = "right"
            else:
                unit["orientation"] = "left"
            unit["shape"] = unit_db[entity]["shape"]
            unit["mass"] = unit_db[entity]["mass"]
            game_state["units"].append(unit)
            i+=1

    return game_state

if __name__=="__main__":
    main()
