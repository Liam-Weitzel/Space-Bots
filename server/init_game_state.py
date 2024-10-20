import random
import math

def check_rock_proximity(rock, existing_rocks, min_distance=150):
    rock_center_x = rock["position"][0] + rock["shape"]["width"] / 2
    rock_center_y = rock["position"][1] + rock["shape"]["height"] / 2

    for existing_rock in existing_rocks:
        existing_center_x = existing_rock["position"][0] + existing_rock["shape"]["width"] / 2
        existing_center_y = existing_rock["position"][1] + existing_rock["shape"]["height"] / 2

        # Calculate the Euclidean distance between the centers
        distance = math.sqrt(
            (rock_center_x - existing_center_x) ** 2 + (rock_center_y - existing_center_y) ** 2
        )

        if distance < min_distance:
            return True
    return False

def check_unit_collision(rock, units):
    for unit in units:
        left1, right1 = rock["position"][0] - rock["shape"]["width"] / 2, rock["position"][0] + rock["shape"]["width"] / 2
        top1, bottom1 = rock["position"][1] - rock["shape"]["height"] / 2, rock["position"][1] + rock["shape"]["height"] / 2

        left2, right2 = rock["position"][0] - rock["shape"]["width"] / 2, rock["position"][0] + unit["shape"]["sprite"] / 2
        top2, bottom2 = rock["position"][1] - rock["shape"]["height"] / 2, rock["position"][1] + unit["shape"]["sprite"] / 2

        if left1 < right2 and right1 > left2 and top1 < bottom2 and bottom1 > top2:
            return True
    return False

def generate_terrain(map_size, units):
    rock_types = {
        "rocks_48x48": {"type": "rectangle", "width": 48, "height": 48, "variations": 8},
        "rocks_48x96": {"type": "rectangle", "width": 48, "height": 96, "variations": 4},
        "rocks_96x96": {"type": "rectangle", "width": 96, "height": 96, "variations": 3}
    }

    formations = [
        { #Vertical gate
            "rocks": [
                {"type": "rocks_48x96", "offset_x": -60, "offset_y": 0},
                {"type": "rocks_48x96", "offset_x": 170, "offset_y": 0},
                {"type": "rocks_48x48", "offset_x": -30, "offset_y": 0},
                {"type": "rocks_48x48", "offset_x": 0, "offset_y": 0},
                {"type": "rocks_48x48", "offset_x": 110, "offset_y": 0},
                {"type": "rocks_48x48", "offset_x": 140, "offset_y": 0}
            ]
        },
        { #The wall
            "rocks": [
                {"type": "rocks_96x96", "offset_x": -80, "offset_y": 0},
                {"type": "rocks_96x96", "offset_x": 0, "offset_y": 0},
                {"type": "rocks_96x96", "offset_x": 80, "offset_y": 0},
            ]
        },
        { #Horizontal gate
            "rocks": [
                {"type": "rocks_48x96", "offset_x": 0, "offset_y": 160},
                {"type": "rocks_48x96", "offset_x": 0, "offset_y": 80},
                {"type": "rocks_48x96", "offset_x": 0, "offset_y": -160},
                {"type": "rocks_48x96", "offset_x": 0, "offset_y": -80},
                {"type": "rocks_48x48", "offset_x": 130, "offset_y": 0}
            ]
        },
        { #Cluster
            "rocks": [
                {"type": "rocks_48x48", "offset_x": 0, "offset_y": 0},
                {"type": "rocks_96x96", "offset_x": 96, "offset_y": 48},
                {"type": "rocks_48x96", "offset_x": -48, "offset_y": 48},
                {"type": "rocks_48x48", "offset_x": 48, "offset_y": 144}
            ]
        }
    ]

    terrain = []
    num_formations = random.randint(2, 5)

    for _ in range(num_formations):
        placed_successfully = False
        attempt_count = 0;
        while not placed_successfully:
            if(attempt_count > 10):
                break
            attempt_count += 1
            formation = random.choice(formations)

            # Generate random position on the left half of the map, that is somewhat in the middle
            location_x = random.randint(150, (map_size[0] // 2) - 150)
            location_y = random.randint(150, map_size[1] - 150)

            new_rocks = []

            # Try adding each rock in the formation to the temporary list
            collision_detected = False
            for rock in formation["rocks"]:
                rock_type = rock["type"]
                rock_variation = random.randint(0, rock_types[rock_type]["variations"] - 1)

                left_rock = {
                    "type": rock_type,
                    "variation": rock_variation,
                    "position": [location_x + rock["offset_x"], location_y + rock["offset_y"]],
                    "shape": {
                        "type": rock_types[rock_type]["type"],
                        "width": rock_types[rock_type]["width"],
                        "height": rock_types[rock_type]["height"]
                    },
                    "orientation": "left"
                }

                if check_rock_proximity(left_rock, terrain) or check_unit_collision(left_rock, units):
                    collision_detected = True
                    break

                new_rocks.append(left_rock)

                # Mirror the formation on the right side
                right_rock = {
                    "type": rock_type,
                    "variation": rock_variation,
                    "position": [map_size[0] - (location_x + rock["offset_x"]), location_y + rock["offset_y"]],
                    "shape": left_rock["shape"],
                    "orientation": "right"
                }

                if check_rock_proximity(right_rock, terrain) or check_unit_collision(right_rock, units):
                    collision_detected = True #NOTE: Not really needed as its already done on left side
                    break

                new_rocks.append(right_rock)

            if not collision_detected:
                terrain.extend(new_rocks)
                placed_successfully = True

    return terrain

def main():
    player_units = [["acid_ant", "spider", "rhino_beetle", "bloated_bedbug", "dung_beetle"],
                    ["lava_ant", "lava_ant", "mantis", "foraging_maggot"]]
    map_size = [1080,1080]

    #Load unit attributes from somewhere (to be determined)
    unit_db = {
            "acid_ant":{ #Your standard soldier
                "max_hp":70,
                "damage":15,
                "attack_speed":2,
                "move_speed":2,
                "fov":100,
                "shape":{
                    "type":"ellipse",
                    "width":5,
                    "height":2,
                    "sprite": 48
                    },
                "mass":5
                },
            "bloated_bedbug":{ #Medium price, high alpha
                "max_hp":20,
                "damage":50,
                "attack_speed":4,
                "move_speed":2,
                "fov":100,
                "shape":{
                    "type":"ellipse",
                    "width":5,
                    "height":2,
                    "sprite": 48
                    },
                "mass":3
                },
            "dung_beetle":{ #Pusher, slow, tanky
                "max_hp":200,
                "damage":5,
                "attack_speed":2,
                "move_speed":0.5,
                "fov":100,
                "shape":{
                    "type":"ellipse",
                    "width":5,
                    "height":2,
                    "sprite": 48
                    },
                "mass":20
                },
            "engorged_tick":{ #Expensive, high dps, fast
                "max_hp":50,
                "damage":5,
                "attack_speed":0.4,
                "move_speed":3,
                "fov":100,
                "shape":{
                    "type":"ellipse",
                    "width":5,
                    "height":2,
                    "sprite": 48
                    },
                "mass":8
                },
            "famished_tick":{ #Cheap, super fast, very low mass, decent dps
                "max_hp":35,
                "damage":10,
                "attack_speed":1.5,
                "move_speed":5,
                "fov":100,
                "shape":{
                    "type":"ellipse",
                    "width":5,
                    "height":2,
                    "sprite": 48
                    },
                "mass":2
                },
            "foraging_maggot":{ #The worker unit
                "max_hp":35,
                "damage":5,
                "attack_speed":2,
                "move_speed":2,
                "fov":100,
                "shape":{
                    "type":"ellipse",
                    "width":5,
                    "height":2,
                    "sprite": 48
                    },
                "mass":5
                },
            "infected_mouse":{ #More expensive better slodier
                "max_hp":100,
                "damage":25,
                "attack_speed":2,
                "move_speed":2,
                "fov":100,
                "shape":{
                    "type":"ellipse",
                    "width":5,
                    "height":2,
                    "sprite": 48
                    },
                "mass":5
                },
            "lava_ant":{ #Different soldier ant
                "max_hp":50,
                "damage":20,
                "attack_speed":2,
                "move_speed":2,
                "fov":100,
                "shape":{
                    "type":"ellipse",
                    "width":5,
                    "height":2,
                    "sprite": 48
                    },
                "mass":5
                },
            "mantis":{ #Ultimate unit, way too expensive
                "max_hp":450,
                "damage":50,
                "attack_speed":3,
                "move_speed":1.5,
                "fov":100,
                "shape":{
                    "type":"ellipse",
                    "width":10,
                    "height":10,
                    "sprite": 96
                    },
                "mass":15
                },
            "mawing_beaver":{ #Tank unit!!
                "max_hp":250,
                "damage":5,
                "attack_speed":1,
                "move_speed":2.5,
                "fov":100,
                "shape":{
                    "type":"ellipse",
                    "width":5,
                    "height":2,
                    "sprite": 48
                    },
                "mass":7
                },
            "plagued_bat":{ #expensive, super fast, very low mass, good dps
                "max_hp":50,
                "damage":12,
                "attack_speed":1,
                "move_speed":5,
                "fov":100,
                "shape":{
                    "type":"ellipse",
                    "width":5,
                    "height":2,
                    "sprite": 48
                    },
                "mass":2
                },
            "rhino_beetle":{ #Tank, high alpha, super slow attack speed
                "max_hp":450,
                "damage":150,
                "attack_speed":15,
                "move_speed":1,
                "fov":150,
                "shape":{
                    "type":"ellipse",
                    "width":5,
                    "height":4,
                    "sprite": 48
                    },
                "mass":10
                },
            "spider":{ #Ultimate unit, way too expensive
                "max_hp":450,
                "damage":50,
                "attack_speed":3,
                "move_speed":1.5,
                "fov":200,
                "shape":{
                    "type":"ellipse",
                    "width":10,
                    "height":10,
                    "sprite": 96
                    },
                "mass":15
                },
            "swooping_bat":{ #expensive, super fast, very low mass, good dps
                "max_hp":50,
                "damage":36,
                "attack_speed":3,
                "move_speed":5,
                "fov":100,
                "shape":{
                    "type":"ellipse",
                    "width":5,
                    "height":2,
                    "sprite": 48
                    },
                "mass":2
                },
            "tainted_cockroach":{ #expensive soldier
                "max_hp":150,
                "damage":30,
                "attack_speed":1.75,
                "move_speed":2,
                "fov":100,
                "shape":{
                    "type":"ellipse",
                    "width":5,
                    "height":2,
                    "sprite": 48
                    },
                "mass":7
                },
            "tunneling_mole":{ #The inter, high mass soldier
                "max_hp":50,
                "damage":15,
                "attack_speed":2.5,
                "move_speed":1,
                "fov":100,
                "shape":{
                    "type":"ellipse",
                    "width":5,
                    "height":2,
                    "sprite": 48
                    },
                "mass":15
            }
    }

    game_state = {"units":[], "terrain":[]}

    unit_id = 0;
    for player, units in enumerate(player_units):
        for i, entity in enumerate(units):
            unit = {}
            unit["id"] = unit_id
            unit["type"] = entity
            unit["player"] = player
            unit["max_hp"] = unit_db[entity]["max_hp"]
            unit["hp"] = unit_db[entity]["max_hp"]
            unit["damage"] = unit_db[entity]["damage"]
            unit["attack_speed"] = unit_db[entity]["attack_speed"]
            unit["move_speed"] = unit_db[entity]["move_speed"]
            unit["fov"] = unit_db[entity]["fov"]
            unit["mass"] = unit_db[entity]["mass"]
            unit["past_action"] = ""
            unit["lock_out_ticks"] = 0
            if(player == 0):
                x = 100
            else:
                x = map_size[0]-100
            y = round((map_size[1] / (len(units)+1)) * (i+1))
            unit["position"] = [x, y]
            if(player == 0):
                unit["orientation"] = "right"
            else:
                unit["orientation"] = "left"
            unit["shape"] = unit_db[entity]["shape"]
            unit["mass"] = unit_db[entity]["mass"]
            game_state["units"].append(unit)
            unit_id+=1

    game_state["terrain"] = generate_terrain(map_size, game_state["units"])
    print(game_state)

    return game_state

if __name__=="__main__":
    main()
