import struct
import random
from typing import Tuple, List, Dict, Optional
from abc import ABC as abstract_class, abstractmethod
from enum import IntEnum

class game_object(abstract_class):
    """Abstract game object class."""
    BITMASK_LENGTH: int = NotImplemented
    THRESHOLDS: Dict[str, float] = NotImplemented
    TYPE_ID: int = NotImplemented
    NETWORK_FORMATS: Dict[str, Tuple[str, int]] = NotImplemented
    BITMASK_MAP: Dict[int, str] = NotImplemented

    def __init__(self) -> None:
        super().__init__()
        self.removed = False
        self.id: int = 0

    @abstractmethod
    def diff(self, other) -> Tuple[int, List[Tuple]]:
        """Calculate the differences between this object and another."""
        pass

    @abstractmethod
    def to_diff_format(self) -> Tuple[int, List[Tuple]]:
        """Prepare the complete state for serialization."""
        pass

    @abstractmethod
    def sync_with(self, other, bitmask: int) -> None:
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

    BITMASK_LENGTH: int = 4
    THRESHOLDS: Dict[str, float] = {'x': 3.0, 'y': 3.0, 'health': 3.0, 'action': 1.0}
    TYPE_ID: int = 1

    NETWORK_FORMATS: Dict[str, Tuple[str, int]] = {
        #attribute: (format, size in bytes)
        'x': ('!f', 4),      # big endian signed float
        'y': ('!f', 4),      # big endian signed float
        'health': ('!I', 4), # big endian unsigned int
        'action': ('!I', 4)  # big endian unsigned int
    }

    BITMASK_MAP: Dict[int, str] = {
        0b0001: 'x',
        0b0010: 'y',
        0b0100: 'health',
        0b1000: 'action'
    }

    class action_enum(IntEnum):
        IDLE = 0
        MOVING = 1
        ATTACKING = 2

        ERROR_GENERIC = 100
        ERROR_OOM = 101
        ERROR_TLE = 102

        @classmethod
        def from_int(cls, value: int) -> Optional['rhino_beetle.action_enum']:
            try:
                return cls(value)
            except ValueError:
                if __debug__:
                    print(f"Invalid command value: {value}")
                return cls.ERROR_GENERIC

        @property
        def is_error(self) -> bool:
            return self.value >= self.ERROR_GENERIC

    def __init__(self, id: int, x: float, y: float, health: int, action: int, stdout: List[str]) -> None:
        super().__init__()
        assert isinstance(id, int) and id >= 0, "Invalid id"
        assert isinstance(x, float), "x should be a float"
        assert isinstance(y, float), "y should be a float"
        assert isinstance(health, int) and health >= 0, "Invalid health value"
        assert isinstance(action, int) and action >= 0, "Invalid action value"

        self.id: int = id
        self.x: float = x
        self.y: float = y
        self.health: int = health
        self.action: int = action
        self.stdout: List[str] = stdout #NOTE: this will be fetched lazily... non essential info

    def diff(self, other) -> Tuple[int, List]:
        """Calculate differences between this and another rhino_beetle."""
        assert isinstance(other, rhino_beetle), "Other must be a rhino_beetle"
        assert other.id == self.id, "Self and other must represent to the same game_object"
        bitmask = 0
        changes = []

        for mask, attr in self.BITMASK_MAP.items():
            if abs(getattr(self, attr) - getattr(other, attr)) >= self.THRESHOLDS[attr]:
                bitmask |= mask
                changes.append(getattr(self, attr))

        return bitmask, changes

    def to_diff_format(self) -> Tuple[int, List]:
        """Prepare the complete state for serialization."""
        full_bitmask = sum(self.BITMASK_MAP.keys())
        return full_bitmask, [getattr(self, attr) for attr in self.BITMASK_MAP.values()]

    def simulate_player(self) -> None:
        """Simulate random movement and health changes."""
        self.x += random.uniform(-1, 1)
        self.y += random.uniform(-1, 1)
        self.health = max(0, self.health + random.randint(-1, 1))

        valid_action = [cmd for cmd in rhino_beetle.action_enum 
                       if cmd.value < rhino_beetle.action_enum.ERROR_GENERIC]
        self.action = random.choice(valid_action)

        if random.uniform(-1, 1) > 0.95:
            self.removed = True

    def sync_with(self, other, bitmask: int) -> None:
        """Synchronize with another rhino_beetle based on the bitmask."""
        assert isinstance(other, rhino_beetle), "Other must be a rhino_beetle"
        assert isinstance(bitmask, int) and bitmask.bit_length() <= self.BITMASK_LENGTH, f"Bitmask must be {self.BITMASK_LENGTH} long"

        for mask, attr in self.BITMASK_MAP.items():
            if bitmask & mask:
                setattr(self, attr, getattr(other, attr))

    def apply(self, data: bytes, bitmask: int) -> None:
        """Apply received data to the rhino_beetle based on the bitmask."""
        index = 0
        for mask, attr in self.BITMASK_MAP.items():
            if bitmask & mask:
                format_spec, size = self.NETWORK_FORMATS[attr]
                value = struct.unpack(format_spec, data[index:index + size])[0]
                setattr(self, attr, value)
                index += size

    def copy(self) -> 'rhino_beetle':
        """Return a copy of this rhino_beetle."""
        return rhino_beetle(self.id, self.x, self.y, self.health, self.action, self.stdout)

    def __str__(self) -> str:
        """String representation for debugging."""
        return f"type: {self.TYPE_ID}, id: {self.id}, x: {self.x}, y: {self.y}, health: {self.health}, action: {rhino_beetle.action_enum.from_int(self.action)}"
