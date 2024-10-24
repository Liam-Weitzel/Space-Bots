import struct
import random
from typing import Tuple, List, Dict
from abc import ABC as abstract_class, abstractmethod

class game_object(abstract_class):
    """Abstract game object class."""

    def __init__(self) -> None:
        super().__init__()
        self.removed = False
    
    def mark_for_removal(self) -> None:
        self.removed = True

    def is_removed(self) -> bool:
        return self.removed

    @abstractmethod
    def diff(self, other: 'game_object') -> Tuple[int, List[Tuple]]:
        """Calculate the differences between this object and another."""
        pass

    @abstractmethod
    def to_diff_format(self) -> Tuple[int, List[Tuple]]:
        """Prepare the complete state for serialization."""
        pass

    @abstractmethod
    def sync_with(self, other: 'game_object', bitmask: int) -> None:
        """Synchronize fields of this object based on the received bitmask."""
        pass

    @abstractmethod
    def apply(self, data: bytes, bitmask: int) -> None:
        """Applies data received to self given the bitmask."""
        pass

    @abstractmethod
    def copy(self) -> 'game_object':
        """Return a copy of this object."""
        pass

    @abstractmethod
    def __str__(self) -> str:
        """String representation for debugging."""
        pass

class rhino_beetle(game_object):
    """Concrete rhino_beetle class inheriting from game_object."""
    
    BITMASK_LENGTH = 3
    thresholds: Dict[str, float] = {'x': 3.0, 'y': 3.0, 'health': 3.0}

    def __init__(self, id: int, x: float, y: float, health: int) -> None:
        super().__init__()
        assert isinstance(id, int) and id >= 0, "Invalid id"
        assert isinstance(x, float), "x should be a float"
        assert isinstance(y, float), "y should be a float"
        assert isinstance(health, int) and health >= 0, "Invalid health value"

        self.id: int = id
        self.x: float = x
        self.y: float = y
        self.health: int = health

    def diff(self, other: 'rhino_beetle') -> Tuple[int, List]:
        """Calculate differences between this and another rhino_beetle."""
        assert isinstance(other, rhino_beetle), "Other must be a rhino_beetle"
        assert other.id == self.id, "Self and other must represent to the same game_object"
        bitmask = 0
        changes = []

        if abs(self.x - other.x) >= self.thresholds['x']:
            bitmask |= 0b001
            changes.append(self.x)

        if abs(self.y - other.y) >= self.thresholds['y']:
            bitmask |= 0b010
            changes.append(self.y)

        if abs(self.health - other.health) >= self.thresholds['health']:
            bitmask |= 0b100
            changes.append(self.health)

        return bitmask, changes

    def to_diff_format(self) -> Tuple[int, List]:
        """Prepare the complete state for serialization."""
        return 0b111, [self.x, self.y, self.health]

    def simulate_player(self) -> None:
        """Simulate random movement and health changes."""
        self.x += random.uniform(-1, 1)
        self.y += random.uniform(-1, 1)
        self.health = max(0, self.health + random.randint(-1, 1))
        if(random.uniform(-1,1) > 0.95):
            self.removed = True

    def sync_with(self, other: 'rhino_beetle', bitmask: int) -> None:
        """Synchronize with another rhino_beetle based on the bitmask."""
        assert isinstance(other, rhino_beetle), "Other must be a rhino_beetle"
        assert isinstance(bitmask, int) and bitmask.bit_length() <= self.BITMASK_LENGTH, f"Bitmask must be {self.BITMASK_LENGTH} long"

        if bitmask & 0b001:
            self.x = other.x
        if bitmask & 0b010:
            self.y = other.y
        if bitmask & 0b100:
            self.health = other.health

    def apply(self, data: bytes, bitmask: int) -> None:
        """Apply received data to the rhino_beetle based on the bitmask."""
        index = 0
        if bitmask & 0b001:
            self.x = struct.unpack('f', data[index:index + 4])[0]
            index += 4
        if bitmask & 0b010:
            self.y = struct.unpack('f', data[index:index + 4])[0]
            index += 4
        if bitmask & 0b100:
            self.health = struct.unpack('i', data[index:index + 4])[0]

    def copy(self) -> 'rhino_beetle':
        """Return a copy of this rhino_beetle."""
        return rhino_beetle(self.id, self.x, self.y, self.health)

    def __str__(self) -> str:
        """String representation for debugging."""
        return f"x: {self.x}, y: {self.y}, health: {self.health}"
