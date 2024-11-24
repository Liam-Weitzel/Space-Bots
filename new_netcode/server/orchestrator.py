import asyncio
from typing import Dict
from game_object import game_object, rhino_beetle
import networking

game_objects: Dict[int, game_object] = {
    0: rhino_beetle(id=0, x=100.0, y=200.0, health=100, action=0, stdout=[]),
    1: rhino_beetle(id=1, x=50.0, y=75.0, health=80, action=0, stdout=[]),
    2: rhino_beetle(id=2, x=50.0, y=75.0, health=80, action=0, stdout=[]),
    3: rhino_beetle(id=3, x=50.0, y=75.0, health=80, action=0, stdout=[]),
    4: rhino_beetle(id=4, x=50.0, y=75.0, health=80, action=0, stdout=[])
}

async def main() -> None:
    """Main function to start the server and handle updates."""
    server = await asyncio.start_server(lambda r, w: networking.handle_client(r, w, game_objects), '127.0.0.1', 8888)
    assert server.sockets is not None, "Server did not start properly"

    print("Server started on port 8888")

    asyncio.create_task(networking.send_updates(game_objects))

    async with server:
        await server.serve_forever()

if __name__ == "__main__":
    asyncio.run(main())
