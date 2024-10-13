import pygame
import socket
import json
import pymunk

pygame.init()
screen = pygame.display.set_mode((800, 600))
pygame.display.set_caption("Game Client")
clock = pygame.time.Clock()

def connect_to_server(host, port):
    """Connect to the socket server."""
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((host, port))
    print(f"Connected to server at {host}:{port}")
    return client_socket

def receive_game_state(client_socket):
    """Receive the game state from the server."""
    try:
        data = client_socket.recv(4096).decode('utf-8').strip()
        if data:
            game_state = json.loads(data)
            return game_state
    except (ConnectionResetError, json.JSONDecodeError) as e:
        print(f"Error receiving game state: {e}")
        return None

client_socket = connect_to_server("localhost", 65432)

running = True
while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

    # Receive game state from server
    game_state = receive_game_state(client_socket)
    if game_state is not None:
        print("Received game state:", game_state)
    
    screen.fill((0, 0, 0))
    
    pygame.display.flip()
    
    clock.tick(60)

# Cleanup
client_socket.close()
pygame.quit()
