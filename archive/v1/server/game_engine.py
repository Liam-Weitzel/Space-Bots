import pymunk
from pymunk import Vec2d

class game_engine:
    def __init__(self, name, init_game_state, tick_rate):
        self.game_state = init_game_state
        self.name = name
        self.tick_rate = tick_rate
        self.dt = 1.0 / tick_rate
        self.space = pymunk.Space()
        self.object_mapping = {}
        
        # Init game_state into pymunk objects
        for unit in self.game_state['units']:
            self.spawn_object(self.space, unit)

        # for x in self.space.shapes:
            # x.body.apply_impulse_at_local_point(Vec2d(1, 2))

        walls = [
        pymunk.Segment(self.space.static_body, (0, 0), (0, 1080), 20), #TODO: fetch map_size using init_game_state
        pymunk.Segment(self.space.static_body, (0, 1080), (1080, 1080), 20),
        pymunk.Segment(self.space.static_body, (1080, 1080), (1080, 0), 20),
        pymunk.Segment(self.space.static_body, (1080, 0), (0, 0), 20)
        ]

        for wall in walls:
            wall.elasticity = 0.1
        self.space.add(*walls)

    def parse_space_to_game_state(self):
        for obj in self.object_mapping.keys():
            self.game_state['units'][obj]['position'] = self.object_mapping[obj].body.position
            if self.object_mapping[obj].body.velocity.int_tuple[0] > 0:
                self.game_state['units'][obj]['orientation'] = 'right'
            else:
                self.game_state['units'][obj]['orientation'] = 'left'

        return self.game_state

    def compute_player_actions(self, actions):
        print(actions)
        for unit in self.game_state["units"]:
            action_unit_id = str(unit['id'])
            # self.object_mapping[unit['id']].body.velocity = (actions[action_unit_id]['x']*500, actions[action_unit_id]['y']*500)
            # if actions[action_unit_id] == 'MOVE':
            self.object_mapping[unit['id']].body.apply_impulse_at_local_point((actions[action_unit_id]['x']*300, actions[action_unit_id]['y']*300)) #FIX:
        return
    
    def spawn_object(self, space, unit):
        ball_body = pymunk.Body(unit['mass'], float("inf"))
        ball_body.position = unit['position']

        ball_shape = pymunk.Circle(ball_body, 15) #TODO: fetch size & type from game_state
        ball_shape.elasticity = 1.0
        ball_shape.collision_type = 1

        def constant_velocity(body, gravity, damping, dt):
            max_velocity = unit['move_speed']*50 #FIX:
            pymunk.Body.update_velocity(body, (0,0), damping, dt)
            l = body.velocity.length
            if l > max_velocity:
                scale = max_velocity / l
                body.velocity = body.velocity * scale

        ball_body.velocity_func = constant_velocity

        space.add(ball_body, ball_shape)
        self.object_mapping[unit['id']] = ball_shape

    def run_tick(self, actions):
        self.compute_player_actions(actions)
        self.space.step(self.dt)
        return self.parse_space_to_game_state()
