import pygame
import socket
import json
import os

pygame.init()
screen = pygame.display.set_mode((1080, 1080)) #NOTE: Maybe get this from game_state?
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
        data = client_socket.recv(4096).decode('utf-8').strip()
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

units = load_sprites_folder('./sprites/units_48x48', 48, 48)
units_animations = {key: {'frames': frames, 'current_frame': 0, 'frame_time': 0, 'animation_speed': 300} for key, frames in units.items()}
client_socket = connect_to_server("localhost", 65432)
bg = pygame.image.load("sprites/bg.jpg")
small_rocks = load_spritesheet('./sprites/rocks_48x48.png', 48, 48)
large_rocks = load_spritesheet('./sprites/rocks_48x96.png', 48, 96)

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

    screen.blit(bg, (0, 0))
    screen.blit(small_rocks[1], (10, 10))
    screen.blit(small_rocks[2], (200, 200))
    screen.blit(small_rocks[3], (1000, 500))
    screen.blit(large_rocks[3], (600, 100))

    for unit in game_state.get('units', []):
        unit_type = unit['type']
        location_x = int(unit['location_x'])
        location_y = int(unit['location_y'])
        orientation = unit['orientation']

        if unit_type in units_animations:
            animation = units_animations[unit_type]
            frames = animation['frames']
            frame_count = len(frames)

            animation['frame_time'] += clock.get_time()  # Add elapsed time to frame time
            if animation['frame_time'] > animation['animation_speed']:  # Change frame based on animation speed
                animation['current_frame'] = (animation['current_frame'] + 1) % frame_count
                animation['frame_time'] = 0

            sprite = frames[animation['current_frame']]

            if orientation == 'left':
                sprite = pygame.transform.flip(sprite, True, False)

            rect = sprite.get_rect(center=(location_x, location_y))
            screen.blit(sprite, rect)

    pygame.display.flip()
    clock.tick(60)

# Cleanup
client_socket.close()
pygame.quit()
