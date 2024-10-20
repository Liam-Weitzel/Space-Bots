import pymunk
import init_game_state
from pymunk import Vec2d

class game_engine:
    def __init__(self, name, init_game_state, tick_rate):
        self.game_state = init_game_state
        self.name = name
        self.tick_rate = tick_rate
        self.dt = 1.0 / tick_rate
        self.space = pymunk.Space()
        self.object_mapping = {}
        
        # Init pymunk objects
        for unit in self.game_state['units']:
            self.spawn_object(self.space, (unit['location_x'], unit['location_y']), unit['id'], self.object_mapping)

        for x in self.space.shapes:
            x.body.apply_impulse_at_local_point(Vec2d(1, 2))
        
        self.tick()
        self.tick()

    def tick(self):
        for x in self.object_mapping.keys():
            print(f"{x}: {self.object_mapping[x].body.position}")

        self.space.step(self.dt)
    
    def spawn_object(self, space, position, id, object_mapping):
        ball_body = pymunk.Body(1, float("inf"))
        ball_body.position = position

        ball_shape = pymunk.Circle(ball_body, 5)
        ball_shape.elasticity = 1.0
        ball_shape.collision_type = 1

        def constant_velocity(body, gravity, damping, dt):
            body.velocity = body.velocity.normalized() * 400

        ball_body.velocity_func = constant_velocity

        space.add(ball_body, ball_shape)
        object_mapping[id] = ball_shape

game_engine = game_engine("test", init_game_state.main(), 60)
