import asyncio
import struct
from typing import Optional, Tuple, List, Dict
from game_object import game_object

clients: List[asyncio.StreamWriter] = []

async def handle_client(reader: asyncio.StreamReader, writer: asyncio.StreamWriter, game_objects: Dict[int, game_object]) -> None: #TODO: implement
    """Handle client connections and messages."""
    assert isinstance(reader, asyncio.StreamReader), "Reader must be an asyncio.StreamReader"
    assert isinstance(writer, asyncio.StreamWriter), "Writer must be an asyncio.StreamWriter"

    clients.append(writer)
    addr = writer.get_extra_info('peername')
    print(f"New connection from {addr}")

    try:
        while True:
            data = await reader.read(100) #FIX: What should the max packet size be??
            if not data:
                break
            print(f"Received data from {addr}")

            bitmask = data[0] #FIX: read BITMASK_LENGTH number of bits
            game_object_id = int.from_bytes(data[1:5], 'little')
            assert game_object_id in game_objects, "Invalid game object ID"

            game_obj = game_objects.get(game_object_id)
            if game_obj:
                game_obj.apply(data[5:], bitmask)
                print(
                    f"Applied updates for object ID {game_object_id}: {game_obj}"
                )
    except asyncio.CancelledError:
        pass
    finally:
        print(f"Connection closed from {addr}")
        clients.remove(writer)
        writer.close()
        await writer.wait_closed()

async def prepare_update(obj: game_object, prev_sent_obj: game_object) -> Optional[Tuple[bytes, int]]:
    """Prepare the update message based on the difference from the previous state."""
    assert isinstance(obj, game_object), "obj must be a game_object instance"
    assert isinstance(prev_sent_obj, game_object), "game_object must be a rhino_beetle instance or None"
    assert type(obj) == type(prev_sent_obj), "obj must the same type as prev_sent_obj"

    # Check if the object is marked for removal
    if obj.is_removed(): #TODO: Handle removed objects payload
        return struct.pack('b', 0b0), 0b0 #Special removal bitmask

    # Calculate the differences if the object is not marked for removal
    bitmask, changes = (obj.to_diff_format() if prev_sent_obj is None else obj.diff(prev_sent_obj))

    assert isinstance(bitmask, int), "bitmask should be an integer"
    assert isinstance(changes, list), "changes should be a list of values"

    if not bitmask:
        return None

    # Verify each change value
    for change in changes:
        assert isinstance(change, (int, float)), "Each change must be an int or float"

    # Pack the bitmask and changes #TODO: Rersturcture accroding to protocol design
    message = struct.pack('b', bitmask) + b''.join(struct.pack('f', value) if isinstance(value, float) else struct.pack('i', value) for value in changes)

    assert isinstance(message, bytes), "message should be bytes"

    return message, bitmask

async def send_updates(game_objects: Dict[int, game_object]) -> None:
    """Continuously check for changes in game objects and send updates to clients."""
    previous_sent_state = {obj_id: obj.copy() for obj_id, obj in game_objects.items()}

    while True:
        tasks = []
        game_objects_to_remove = []

        # Prepare update tasks for each game object
        for obj_id, obj in game_objects.items():
            assert isinstance(obj_id, int), "obj_id should be an integer"
            assert isinstance(obj, game_object), "obj should be a game_object instance"
            obj.simulate_player() #HACK: simulate player's interacting with the game
            print(f"Current state of object {obj_id}: {obj}")
            tasks.append(prepare_update(obj, previous_sent_state[obj_id]))

        # Gather results from parallel processing
        results = await asyncio.gather(*tasks)

        # Send updates if any
        for (obj_id, obj), result in zip(game_objects.items(), results):
            if result is None:
                continue

            message, bitmask = result
            assert isinstance(message, bytes), "message should be bytes"
            assert isinstance(bitmask, int), "bitmask should be an integer"

            binary_message = ' '.join(f'{byte:08b}' for byte in message)
            print(f"Sending update to all clients for object {obj_id}: {binary_message}")

            for client in clients:
                assert hasattr(client, 'write'), "client should have a write method"
                assert hasattr(client, 'drain'), "client should have a drain method"
                client.write(message)
                print(f"Sent to {client.get_extra_info('peername')}")
                await client.drain()

            print(f"Bitmask for object {obj_id}: {format(bitmask, f'0{obj.BITMASK_LENGTH}b')}")

            # Check if the returned bitmask indicates removal
            if bitmask == 0b0:
                game_objects_to_remove.append(obj_id)
            else: 
                # Update previous_sent_state based on the received bitmask
                previous_sent_state[obj_id].sync_with(game_objects[obj_id], bitmask)

                # Ensure previous_sent_state is correctly updated
                assert obj_id in previous_sent_state, f"obj_id {obj_id} should exist in previous_sent_state"
                assert previous_sent_state[obj_id].id == game_objects[obj_id].id, "obj_id in previous_sent_state does not match game_objects"

                print(f"Updated previous_sent_state for object {obj_id}: {previous_sent_state[obj_id]}")

        for obj_id in game_objects_to_remove:
            del game_objects[obj_id]
            del previous_sent_state[obj_id]
            print(f"Removed object {obj_id}")

        await asyncio.sleep(0.1)
