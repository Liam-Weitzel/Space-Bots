import pygame
import socket
import json
import os

pygame.init()
screen = pygame.display.set_mode((1080, 1080)) #TODO: Maybe get this from game_state?
pygame.display.set_caption("Game Client")
clock = pygame.time.Clock()

def load_spritesheet(sheet_path, frame_width, frame_height):
    sheet = pygame.image.load(sheet_path).convert_alpha()
    frames = []
    sheet_width, sheet_height = sheet.get_size()

    for y in range(0, sheet_height, frame_height):
        for x in range(0, sheet_width, frame_width):
            frame = sheet.subsurface(pygame.Rect(x, y, frame_width, frame_height))
            frames.append(frame)

    return frames

def load_sprites_folder(sprite_folder, frame_width, frame_height):
    sprites = {}
    for filename in os.listdir(sprite_folder):
        if filename.endswith('.png'):
            sprite_name = filename.split('.')[0]
            sprite_path = os.path.join(sprite_folder, filename)
            sprites[sprite_name] = load_spritesheet(sprite_path, frame_width, frame_height)
    return sprites

def connect_to_server(host, port):
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((host, port))
    print(f"Connected to server at {host}:{port}")
    return client_socket

def receive_game_state(client_socket):
    try:
        data = client_socket.recv(16384).decode('utf-8').strip()
        if data:
            game_state = json.loads(data)
            return game_state
    except (ConnectionResetError, json.JSONDecodeError) as e:
        print(f"Error receiving game state: {e}")
        return None

def send_inputs_to_server(client_socket, inputs):
    try:
        serialized_data = json.dumps(inputs).encode('utf-8')
        client_socket.sendall(serialized_data + b'\n')
    except (BrokenPipeError, ConnectionResetError, OSError) as e:
        print(f"Error sending inputs to server: {e}")

small_units = load_sprites_folder('./sprites/units_48x48', 48, 48)
large_units = load_sprites_folder('./sprites/units_96x96', 96, 96)

units_animations = {}
for key, frames in list(small_units.items()) + list(large_units.items()):
    if key not in units_animations:
        units_animations[key] = {
            'frames': [],
            'current_frame': 0,
            'frame_time': 0,
            'animation_speed': 300
        }
    units_animations[key]['frames'].extend(frames)

client_socket = connect_to_server("localhost", 65432)
bg = pygame.image.load("sprites/bg.jpg")
small_rocks = load_spritesheet('./sprites/rocks_48x48.png', 48, 48)
large_rocks = load_spritesheet('./sprites/rocks_48x96.png', 48, 96)
extralarge_rocks = load_spritesheet('./sprites/rocks_96x96.png', 96, 96)

def get_rock_sprites(rock_type):
    if "48x48" in rock_type:
        return small_rocks
    elif "48x96" in rock_type:
        return large_rocks
    elif "96x96" in rock_type:
        return extralarge_rocks
    else:
        return None

#TODO: Visualize fog of war when unit is selected, also display their full stats

init_state = receive_game_state(client_socket)
if init_state is not None:
    print("Received init state:", init_state)
player_number = init_state['player']

running = True
while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

    action = None
    keys = pygame.key.get_pressed()

    if keys[pygame.K_LEFT]:
        action = 'left'
    elif keys[pygame.K_RIGHT]:
        action = 'right'
    elif keys[pygame.K_UP]:
        action = 'up'
    elif keys[pygame.K_DOWN]:
        action = 'down'

    if action:
        send_inputs_to_server(client_socket, action)

    game_state = receive_game_state(client_socket)
    if game_state is not None:
        print("Received game state:", game_state)

    # Create a buffer surface to draw everything
    buffer_surface = pygame.Surface((1080, 1080))
    buffer_surface.blit(bg, (0, 0))

    # Draw units
    for unit in game_state['units']:
        unit_type = unit['type']
        location_x = int(unit['location_x'])
        location_y = int(unit['location_y'])
        orientation = unit['orientation']
        current_hp = unit['hp']
        max_hp = unit['max_hp']
        unit_player = unit['player']

        if unit_type in units_animations:
            animation = units_animations[unit_type]
            frames = animation['frames']
            frame_count = len(frames)

            animation['frame_time'] += clock.get_time()
            if animation['frame_time'] > animation['animation_speed']:
                animation['current_frame'] = (animation['current_frame'] + 1) % frame_count
                animation['frame_time'] = 0

            sprite = frames[animation['current_frame']]

            if orientation == 'left':
                sprite = pygame.transform.flip(sprite, True, False)

            rect = sprite.get_rect(center=(location_x, location_y))
            buffer_surface.blit(sprite, rect)

            # Draw health bar above the unit
            base_health_bar_width = 50
            health_bar_width = int(base_health_bar_width * (max_hp / 100))
            health_bar_height = 5
            health_ratio = current_hp / max_hp
            health_bar_color = (255, 255, 255)
            if unit_player == player_number:
                health_bar_color = (0, 255, 0)
            else:
                health_bar_color = (255, 0, 0)

            # Full health background bar
            background_bar_rect = pygame.Rect(
                location_x - health_bar_width // 2,
                location_y - rect.height // 2 - health_bar_height - 5,
                health_bar_width,
                health_bar_height
            )
            pygame.draw.rect(buffer_surface, (0, 0, 0), background_bar_rect)

            # Current health bar
            health_bar_rect = pygame.Rect(
                location_x - health_bar_width // 2,
                location_y - rect.height // 2 - health_bar_height - 5,
                int(health_bar_width * health_ratio),
                health_bar_height
            )
            pygame.draw.rect(buffer_surface, health_bar_color, health_bar_rect)

    # Draw terrain (rocks)
    for terrain in game_state['terrain']:
        rock_type = terrain['type']
        variation = terrain['variation']
        location_x = int(terrain['location_x'])
        location_y = int(terrain['location_y'])
        orientation = terrain['orientation']

        rock_sprites = get_rock_sprites(rock_type)
        if rock_sprites:
            sprite = rock_sprites[variation]

            if orientation == 'right':
                sprite = pygame.transform.flip(sprite, True, False)

            rect = sprite.get_rect(center=(location_x, location_y))
            buffer_surface.blit(sprite, rect)

    # Flip the buffer surface if player_number is 1
    if player_number == 1:
        buffer_surface = pygame.transform.flip(buffer_surface, True, False)

    # Blit the buffer surface to the main screen
    screen.blit(buffer_surface, (0, 0))

    pygame.display.flip()
    clock.tick(60)

# Cleanup
client_socket.close()
pygame.quit()
