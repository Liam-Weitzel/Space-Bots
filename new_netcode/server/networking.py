import asyncio
import struct
import sys
from typing import Optional, Tuple, List, Dict
from game_object import game_object, rhino_beetle

# Constants for protocol
TICK_MESSAGE_HEADER_SIZE = 4  # Size of length header in bytes
TICK_MESSAGE_BODY_SIZE = 65536  # Maximum allowed packet size

clients: List[asyncio.StreamWriter] = []

async def handle_client(reader: asyncio.StreamReader, writer: asyncio.StreamWriter, game_objects: Dict[int, game_object]) -> None:
    """Handle client connections and messages."""
    assert isinstance(reader, asyncio.StreamReader), "Reader must be an asyncio.StreamReader"
    assert isinstance(writer, asyncio.StreamWriter), "Writer must be an asyncio.StreamWriter"

    clients.append(writer)
    addr = writer.get_extra_info('peername')
    if __debug__:
        print(f"New connection from {addr}")

    try:
        while True:
            header = await reader.read(100)
            if not header:
                break

            size = struct.unpack('!I', header)[0]

            data = await reader.read(size)
            if not data:
                break

            #TODO: design what kind of data the game server will receive from the client. what do we do with this data?

    except asyncio.CancelledError:
        pass
    finally:
        if __debug__:
            print(f"Connection closed from {addr}")
        clients.remove(writer)
        writer.close()
        await writer.wait_closed()

async def prepare_update(obj: game_object, prev_sent_obj: Optional[game_object]) -> Optional[Tuple[bytes, int]]:
    """Prepare the update message based on the difference from the previous state."""
    assert isinstance(obj, game_object), "obj must be a game_object instance"
    if prev_sent_obj is not None:
        assert isinstance(prev_sent_obj, game_object), "prev_sent_obj must be a game_object instance"
        assert type(obj) == type(prev_sent_obj), "obj must the same type as prev_sent_obj"

    # Add obj header: type (1 byte) + id (2 bytes)
    obj_header = struct.pack('!BH',
        obj.TYPE_ID,  # Object type (1 byte)
        obj.id        # Object ID (2 bytes)
    )

    assert obj.id <= 65535, "obj_id must fit in 2 bytes"
    assert obj.TYPE_ID <= 256, "object type id must fit in 1 byte"
    assert len(obj_header) == 3, "object header must be 3 bytes"

    if obj.removed:
        bitmask = 0b0 # Special removal bitmask
        obj_message = bytearray()
        obj_message.extend(obj_header)
        obj_message.extend(struct.pack(f'!B', bitmask))
        return obj_message, bitmask

    bitmask, changes = obj.to_diff_format() if prev_sent_obj is None else obj.diff(prev_sent_obj)

    if not bitmask:
        return None

    assert isinstance(bitmask, int), "bitmask should be an integer"
    assert isinstance(changes, list), "changes should be a list of values"
    assert 0 <= bitmask <= (1 << obj.BITMASK_LENGTH) - 1, f"bitmask must fit in {obj.BITMASK_LENGTH} bits"

    # Verify each change value
    for change in changes:
        assert isinstance(change, (int, float)), "Each change must be an int or float"

    # Pack using network byte order
    format_char = {1: 'B', 2: 'H', 4: 'I', 8: 'Q'}[max(1, int(obj.BITMASK_LENGTH/8))]
    data = struct.pack(f'!{format_char}', bitmask) + b''.join(
        struct.pack('!f', value) if isinstance(value, float) else struct.pack('!i', value)
        for value in changes
    )

    assert isinstance(data, bytes), "data should be bytes"
    assert len(data) <= TICK_MESSAGE_BODY_SIZE, f"Message data {len(data)} already exceeds maximum packet size {TICK_MESSAGE_BODY_SIZE}"

    obj_message = bytearray()
    obj_message.extend(obj_header)
    obj_message.extend(data)

    assert sys.getsizeof(obj_message) <= TICK_MESSAGE_BODY_SIZE, f"Message would exceed MAX_PACKET_SIZE ({obj_message} > {TICK_MESSAGE_BODY_SIZE})"

    return obj_message, bitmask

async def send_updates(game_objects: Dict[int, game_object]) -> None:
    """Continuously check for changes in game objects and send updates to clients."""
    assert isinstance(game_objects, dict), "game_objects must be a dictionary"
    for obj_id, obj in game_objects.items():
        assert isinstance(obj_id, int), "game_object keys must be integers"
        assert isinstance(obj, game_object), "game_object values must be game_object instances"
        assert hasattr(obj, 'TYPE_ID'), "game_object must have TYPE_ID defined"
        assert isinstance(obj.TYPE_ID, int), "TYPE_ID must be an integer"
        assert 0 <= obj.TYPE_ID <= 255, "TYPE_ID must fit in one byte"

    previous_sent_state = {obj_id: obj.copy() for obj_id, obj in game_objects.items()}

    while True:
        tasks = []
        game_objects_to_remove = []

        # Prepare all object updates
        for obj_id, obj in game_objects.items():
            assert isinstance(obj_id, int), "obj_id should be an integer"
            assert obj_id == obj.id, "game_objects key should be equal to object id"
            assert isinstance(obj, game_object), "obj should be a game_object instance"
            obj.simulate_player() #TODO: Remove this...
            if __debug__:
                print(f"Current state of object {obj_id}: {obj}")
            if obj_id in previous_sent_state:
                assert type(obj) == type(previous_sent_state[obj_id]), "object types must match previous state"
                tasks.append(prepare_update(obj, previous_sent_state[obj_id]))
            else:
                tasks.append(prepare_update(obj, None))

        results = await asyncio.gather(*tasks)
        assert len(results) == len(game_objects), "number of results must match number of objects"

        # Combine all object updates into one message
        tick_message = bytearray()
        updates_count = 0

        for (obj_id, obj), result in zip(game_objects.items(), results):
            if result is None:
                continue

            obj_message, bitmask = result

            # Add the obj message to the tick message
            tick_message.extend(obj_message)
            updates_count += 1

            if __debug__:
                print(f"Added object {obj_id} to tick message, type: {obj.TYPE_ID}, bitmask: {format(bitmask, f'0{obj.BITMASK_LENGTH}b')}")

            if bitmask == 0b0:
                game_objects_to_remove.append(obj_id)
            else:
                if obj_id not in previous_sent_state:
                    previous_sent_state[obj_id] = game_objects[obj_id].copy()
                else:
                    previous_sent_state[obj_id].sync_with(game_objects[obj_id], bitmask)
                    assert obj_id in previous_sent_state, f"obj_id {obj_id} should exist in previous_sent_state"
                    assert previous_sent_state[obj_id].id == game_objects[obj_id].id, "obj_id mismatch"
                    assert type(previous_sent_state[obj_id]) == type(game_objects[obj_id]), "object types must match"

        if tick_message:  # Only send if there are updates
            assert updates_count > 0, "tick_message exists but no updates counted"

            message_size = len(tick_message)
            assert message_size > 0, "tick_message length must be greater than 0"
            assert message_size <= TICK_MESSAGE_BODY_SIZE, f"uncompressed message too large: {message_size}"

            # Add size header
            tick_message = struct.pack('!I', message_size) + tick_message
            assert len(tick_message) <= TICK_MESSAGE_BODY_SIZE + TICK_MESSAGE_HEADER_SIZE, f"final message too large: {len(tick_message)}"

            # TODO: compress the message?
            # compressed_message = lz4.frame.compress(tick_message)
            # compressed_size = len(compressed_message)
            # assert compressed_size > 0, "compressed message length must be greater than 0"
            # if __debug__:
            #   print(f"Compression: {original_size} -> {compressed_size} bytes ({(compressed_size/original_size)*100:.1f}%)")

            if __debug__:
                print(f"Sending tick update: {updates_count} objects")
                print("Bytes:", ' '.join(f'\\x{byte:02x}' for byte in tick_message))
                print("Bits: ", ' '.join(f'{byte:08b}' for byte in tick_message))

            # Send to all clients
            for client in clients:
                assert hasattr(client, 'write'), "client must have write method"
                assert hasattr(client, 'drain'), "client must have drain method"
                client.write(tick_message)
                if __debug__:
                    print(f"Sent to {client.get_extra_info('peername')}")
                await client.drain()

        # Clean up removed objects
        for obj_id in game_objects_to_remove:
            assert obj_id in game_objects, f"cannot remove non-existent object {obj_id}"
            del game_objects[obj_id]
            if obj_id in previous_sent_state:
                del previous_sent_state[obj_id]
            if __debug__:
                print(f"Removed object {obj_id}")

        await asyncio.sleep(0.1)
