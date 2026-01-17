"""
Alat Simulasi Skenario Lampu Lalu Lintas Indonesia
==================================================
Simulasi lalu lintas 2D top-down untuk menguji konfigurasi waktu lampu lalu lintas.
Dibangun dengan Python dan Pygame.
Sistem Lalu Lintas Indonesia: Lajur Kiri (Keep Left)

Penulis: Traffic Engineering Simulation
Tanggal: 2026

Simulasi ini memungkinkan pengguna untuk:
- Menyesuaikan durasi lampu (Merah/Hijau) untuk arah UtaraSelatan dan TimurBarat
- Mengontrol kepadatan lalu lintas melalui pengaturan kemungkinan spawn
- Memantau statistik real-time (panjang antrian, waktu tunggu)
- Menguji konfigurasi berbeda untuk mengatasi kemacetan
"""

import pygame
import random
import math
from enum import Enum
from typing import List, Dict, Optional, Tuple
from dataclasses import dataclass
import os

# Initialize Pygame
pygame.init()

# =============================================================================
# CONSTANTS AND CONFIGURATION
# =============================================================================

# Screen dimensions
SCREEN_WIDTH = 1200
SCREEN_HEIGHT = 800

# Road configuration
ROAD_WIDTH = 120  # Width of the road
LANE_WIDTH = 60   # Width of each lane
TILE_SIZE = 40    # Size of each tile in the spritesheet

# Colors (used as placeholders if sprites are missing)
COLOR_ROAD = (60, 60, 60)
COLOR_ROAD_MARKING = (255, 255, 255)
COLOR_GRASS = (34, 139, 34)
COLOR_STOP_LINE = (255, 255, 255)
COLOR_SIDEWALK = (150, 150, 150)

# Traffic Light Colors
COLOR_RED = (255, 0, 0)
COLOR_YELLOW = (255, 255, 0)
COLOR_GREEN = (0, 255, 0)
COLOR_LIGHT_OFF = (50, 50, 50)

# Vehicle Colors (placeholders)
COLOR_CAR = (0, 100, 255)
COLOR_BIKE = (255, 165, 0)
COLOR_TRUCK = (139, 69, 19)

# UI Colors
COLOR_UI_BG = (40, 40, 40)
COLOR_UI_TEXT = (255, 255, 255)
COLOR_UI_SLIDER_BG = (80, 80, 80)
COLOR_UI_SLIDER_FG = (100, 200, 100)
COLOR_UI_BUTTON = (70, 130, 180)
COLOR_UI_BUTTON_HOVER = (100, 160, 210)

# Timing constants
YELLOW_DURATION = 3.0  # Fixed 3-second yellow phase
FPS = 60

# Vehicle behavior constants
SAFE_DISTANCE = 30      # Minimum distance between vehicles
ZEBRA_CROSSING_DISTANCE = 130  # Distance from intersection center to zebra crossing (stop line)
REACTION_DELAY_MIN = 0.2  # Minimum reaction delay (seconds)
REACTION_DELAY_MAX = 0.8  # Maximum reaction delay (seconds)
INTERSECTION_CENTER_SIZE = 120  # Core intersection area for turning decision
INTERSECTION_TURN_ZONE = 180    # Larger zone for smooth turning
TURN_LEFT_PROBABILITY = 0.35    # Probability a vehicle will turn left
TURN_RIGHT_PROBABILITY = 0.0    # Probability a vehicle will turn right (0 = keep left rule)
# Remaining probability = go straight


# =============================================================================
# ENUMERATIONS
# =============================================================================

class LightState(Enum):
    """Traffic light states"""
    RED = "red"
    YELLOW = "yellow"
    GREEN = "green"


class Direction(Enum):
    """Cardinal directions for vehicle movement"""
    NORTH = "north"  # Moving up (decreasing Y)
    SOUTH = "south"  # Moving down (increasing Y)
    EAST = "east"    # Moving right (increasing X)
    WEST = "west"    # Moving left (decreasing X)


class TurnIntent(Enum):
    """Vehicle turning intentions at intersection"""
    STRAIGHT = "straight"
    TURN_LEFT = "left"


class VehicleType(Enum):
    """Types of vehicles with different characteristics"""
    CAR = "car"
    BIKE = "bike"
    TRUCK = "truck"


# =============================================================================
# SPRITESHEET CLASS
# =============================================================================

class SpriteSheet:
    """
    SpriteSheet Loader Class
    ========================
    Handles loading and cropping of 3x3 grid spritesheets.

    The spritesheet is expected to be organized as a 3x3 grid:
    [0,0] [1,0] [2,0]
    [0,1] [1,1] [2,1]
    [0,2] [1,2] [2,2]

    Each tile is TILE_SIZE x TILE_SIZE pixels.
    """

    def __init__(self, filename: str, tile_size: int = TILE_SIZE):
        """
        Initialize the spritesheet loader.

        Args:
            filename: Path to the spritesheet image
            tile_size: Size of each tile (assumes square tiles)
        """
        self.tile_size = tile_size
        self.sheet = None
        self.tiles = {}
        self.loaded = False

        # Try to load the spritesheet
        if os.path.exists(filename):
            try:
                self.sheet = pygame.image.load(filename).convert_alpha()
                self.loaded = True
                self._crop_tiles()
                print(f"Successfully loaded spritesheet: {filename}")
            except pygame.error as e:
                print(f"Could not load spritesheet {filename}: {e}")
                self.loaded = False
        else:
            print(f"Spritesheet not found: {filename} - Using placeholders")
            self.loaded = False

    def _crop_tiles(self):
        """Crop all 9 tiles from the 3x3 grid spritesheet."""
        if not self.loaded:
            return

        for row in range(3):
            for col in range(3):
                # Calculate the rectangle for this tile
                rect = pygame.Rect(
                    col * self.tile_size,
                    row * self.tile_size,
                    self.tile_size,
                    self.tile_size
                )
                # Create a new surface for this tile
                tile = pygame.Surface(
                    (self.tile_size, self.tile_size), pygame.SRCALPHA)
                tile.blit(self.sheet, (0, 0), rect)
                # Store with (col, row) key for easy access
                self.tiles[(col, row)] = tile

    def get_tile(self, col: int, row: int) -> Optional[pygame.Surface]:
        """
        Get a specific tile from the spritesheet.

        Args:
            col: Column index (0-2)
            row: Row index (0-2)

        Returns:
            The tile surface or None if not loaded
        """
        if self.loaded:
            return self.tiles.get((col, row))
        return None

    def get_scaled_tile(self, col: int, row: int, scale: float) -> Optional[pygame.Surface]:
        """
        Get a scaled version of a tile.

        Args:
            col: Column index (0-2)
            row: Row index (0-2)
            scale: Scale factor

        Returns:
            Scaled tile surface or None if not loaded
        """
        tile = self.get_tile(col, row)
        if tile:
            new_size = int(self.tile_size * scale)
            return pygame.transform.scale(tile, (new_size, new_size))
        return None


# =============================================================================
# VEHICLE CLASS
# =============================================================================

@dataclass
class VehicleConfig:
    """Configuration for different vehicle types"""
    length: int
    width: int
    max_speed: float
    acceleration: float
    deceleration: float
    color: Tuple[int, int, int]


# Vehicle configurations
VEHICLE_CONFIGS = {
    VehicleType.CAR: VehicleConfig(
        length=30, width=20, max_speed=150, acceleration=80, deceleration=200, color=COLOR_CAR
    ),
    VehicleType.BIKE: VehicleConfig(
        length=20, width=12, max_speed=180, acceleration=120, deceleration=250, color=COLOR_BIKE
    ),
    VehicleType.TRUCK: VehicleConfig(
        length=50, width=24, max_speed=100, acceleration=40, deceleration=150, color=COLOR_TRUCK
    ),
}


class Vehicle:
    """
    Vehicle Class
    =============
    Handles individual vehicle behavior including:
    - Movement and physics
    - Collision detection with other vehicles
    - Traffic light compliance
    - Reaction delay for realistic start behavior

    Wait Time Calculation Logic:
    ---------------------------
    Wait time is calculated as the cumulative time a vehicle spends stationary
    or moving below a threshold speed (5 pixels/second). This represents the
    time lost due to traffic conditions.

    The wait time starts accumulating when:
    1. Vehicle stops at a red light
    2. Vehicle stops behind another vehicle
    3. Vehicle is in queue waiting for traffic to clear

    Wait time stops accumulating when the vehicle is moving at normal speed.
    """

    def __init__(self, x: float, y: float, direction: Direction,
                 vehicle_type: VehicleType, spritesheet: Optional[SpriteSheet] = None):
        """
        Initialize a vehicle.

        Args:
            x: Starting X position
            y: Starting Y position
            direction: Direction of travel
            vehicle_type: Type of vehicle (car, bike, truck)
            spritesheet: Optional spritesheet for vehicle graphics
        """
        self.x = x
        self.y = y
        self.direction = direction
        self.vehicle_type = vehicle_type
        self.config = VEHICLE_CONFIGS[vehicle_type]
        self.spritesheet = spritesheet

        # Physics
        self.speed = 0.0  # Current speed (pixels per second)
        self.target_speed = self.config.max_speed

        # State
        self.stopped = False
        self.waiting_for_green = False
        self.reaction_timer = 0.0  # Timer for staggered start
        self.reaction_delay = random.uniform(
            REACTION_DELAY_MIN, REACTION_DELAY_MAX)
        self.has_reacted = True  # Initially true, set false when light turns green

        # Statistics tracking
        # Wait time accumulates when vehicle speed is below threshold
        self.wait_time = 0.0  # Total time spent waiting (seconds)
        self.WAIT_SPEED_THRESHOLD = 5.0  # Speed below which we consider "waiting"

        # Turning behavior (keep left rule - no right turns)
        rand = random.random()
        if rand < TURN_LEFT_PROBABILITY:
            self.turn_intent = TurnIntent.TURN_LEFT
        else:
            self.turn_intent = TurnIntent.STRAIGHT
        self.is_turning = False
        self.turn_angle = 0.0  # For smooth turning animation
        
        # For removal when off-screen
        self.active = True

        # Unique ID for tracking
        self.id = id(self)

    def get_length(self) -> int:
        """Get vehicle length based on orientation."""
        return self.config.length

    def get_width(self) -> int:
        """Get vehicle width based on orientation."""
        return self.config.width

    def get_front_position(self) -> Tuple[float, float]:
        """Get the position of the front of the vehicle."""
        if self.direction == Direction.NORTH:
            return (self.x, self.y - self.config.length / 2)
        elif self.direction == Direction.SOUTH:
            return (self.x, self.y + self.config.length / 2)
        elif self.direction == Direction.EAST:
            return (self.x + self.config.length / 2, self.y)
        else:  # WEST
            return (self.x - self.config.length / 2, self.y)

    def get_rect(self) -> pygame.Rect:
        """Get the bounding rectangle of the vehicle."""
        if self.direction in [Direction.NORTH, Direction.SOUTH]:
            return pygame.Rect(
                self.x - self.config.width / 2,
                self.y - self.config.length / 2,
                self.config.width,
                self.config.length
            )
        else:
            return pygame.Rect(
                self.x - self.config.length / 2,
                self.y - self.config.width / 2,
                self.config.length,
                self.config.width
            )

    def distance_to(self, other: 'Vehicle') -> float:
        """Calculate distance to another vehicle (front to back)."""
        my_front = self.get_front_position()
        other_rect = other.get_rect()

        # Calculate distance based on direction
        if self.direction == Direction.NORTH:
            # Distance to bottom of vehicle ahead
            return self.y - self.config.length / 2 - (other.y + other.config.length / 2)
        elif self.direction == Direction.SOUTH:
            return (other.y - other.config.length / 2) - (self.y + self.config.length / 2)
        elif self.direction == Direction.EAST:
            return (other.x - other.config.length / 2) - (self.x + self.config.length / 2)
        else:  # WEST
            return self.x - self.config.length / 2 - (other.x + other.config.length / 2)

    def is_ahead_of_me(self, other: 'Vehicle') -> bool:
        """Check if another vehicle is ahead of this one in the same lane."""
        if other.direction != self.direction:
            return False

        # Check if in same lane (approximate)
        lane_tolerance = LANE_WIDTH / 2

        if self.direction in [Direction.NORTH, Direction.SOUTH]:
            if abs(self.x - other.x) > lane_tolerance:
                return False
            if self.direction == Direction.NORTH:
                return other.y < self.y
            else:
                return other.y > self.y
        else:
            if abs(self.y - other.y) > lane_tolerance:
                return False
            if self.direction == Direction.EAST:
                return other.x > self.x
            else:
                return other.x < self.x

    def should_stop_for_light(self, light_state: LightState,
                              intersection_center: Tuple[float, float]) -> bool:
        """
        Determine if vehicle should stop for traffic light.
        
        Real-world traffic rules (Indonesia - Lajur Kiri):
        - Kendaraan yang belok kiri BOLEH lewat saat lampu merah (Kiri Jalan Terus)
        - LAMPU MERAH: BERHENTI sebelum masuk persimpangan
        - LAMPU KUNING: Either terus (jika aman) atau berhenti sebelum persimpangan
        - LAMPU HIJAU: Jalan terus

        Args:
            light_state: Current state of the relevant traffic light
            intersection_center: Center point of the intersection

        Returns:
            True if vehicle should stop
        """
        # Kiri jalan terus - kendaraan yang belok kiri bisa lewat lampu merah
        if light_state == LightState.RED and self.turn_intent == TurnIntent.TURN_LEFT:
            return False
        
        if light_state == LightState.GREEN:
            return False

        front_x, front_y = self.get_front_position()
        cx, cy = intersection_center
        
        # MERAH: HARUS berhenti sebelum persimpangan
        # KUNING: Jika sudah close, boleh terus; jika masih jauh, berhenti sebelum
        # Stop line adalah di ZEBRA_CROSSING_DISTANCE dari pusat
        stop_line = ZEBRA_CROSSING_DISTANCE
        
        if self.direction == Direction.SOUTH:
            # Vehicle dari UTARA ke SELATAN (bergerak ke bawah, Y meningkat)
            # Stop line di cy + stop_line
            distance_to_stop = (cy + stop_line) - front_y
            
            if light_state == LightState.RED:
                return distance_to_stop > 0  # Stop jika belum melewati garis
            else:  # YELLOW
                # Kuning: stop jika masih jauh dari garis (>50px)
                return distance_to_stop > 50
            
        elif self.direction == Direction.NORTH:
            # Vehicle dari SELATAN ke UTARA (bergerak ke atas, Y menurun)
            # Stop line di cy - stop_line
            distance_to_stop = front_y - (cy - stop_line)
            
            if light_state == LightState.RED:
                return distance_to_stop > 0  # Stop jika belum melewati garis
            else:  # YELLOW
                return distance_to_stop > 50
            
        elif self.direction == Direction.EAST:
            # Vehicle dari BARAT ke TIMUR (bergerak ke kanan, X meningkat)
            # Stop line di cx + stop_line
            distance_to_stop = (cx + stop_line) - front_x
            
            if light_state == LightState.RED:
                return distance_to_stop > 0
            else:  # YELLOW
                return distance_to_stop > 50
            
        else:  # WEST
            # Vehicle dari TIMUR ke BARAT (bergerak ke kiri, X menurun)
            # Stop line di cx - stop_line
            distance_to_stop = front_x - (cx - stop_line)
            
            if light_state == LightState.RED:
                return distance_to_stop > 0
            else:  # YELLOW
                return distance_to_stop > 50

        return False
    
    def get_new_direction_after_turn(self) -> Direction:
        """Calculate the new direction after a turn at intersection (left turn only - keep left rule)."""
        if self.turn_intent == TurnIntent.STRAIGHT:
            return self.direction
        elif self.turn_intent == TurnIntent.TURN_LEFT:
            if self.direction == Direction.NORTH:
                return Direction.WEST
            elif self.direction == Direction.SOUTH:
                return Direction.EAST
            elif self.direction == Direction.EAST:
                return Direction.NORTH
            else:  # WEST
                return Direction.SOUTH
        return self.direction
    
    def is_in_turn_zone(self, intersection_center: Tuple[float, float]) -> bool:
        """Check if vehicle is in the turning zone (wider intersection area)."""
        cx, cy = intersection_center
        return (abs(self.x - cx) < INTERSECTION_TURN_ZONE // 2 and 
                abs(self.y - cy) < INTERSECTION_TURN_ZONE // 2)
    
    def is_at_intersection_center(self, intersection_center: Tuple[float, float]) -> bool:
        """Check if vehicle is at the center of intersection (for turn decision)."""
        cx, cy = intersection_center
        return (abs(self.x - cx) < INTERSECTION_CENTER_SIZE // 2 and 
                abs(self.y - cy) < INTERSECTION_CENTER_SIZE // 2)
    
    def should_turn(self, intersection_center: Tuple[float, float]) -> bool:
        """Check if vehicle should execute its turn now."""
        if self.is_turning or self.turn_intent == TurnIntent.STRAIGHT:
            return False
        # Turn when vehicle reaches the center of intersection
        return self.is_at_intersection_center(intersection_center)

    def update(self, dt: float, vehicles: List['Vehicle'],
               light_state: LightState, intersection_center: Tuple[float, float],
               light_just_turned_green: bool = False):
        """
        Update vehicle state and position.

        Args:
            dt: Delta time in seconds
            vehicles: List of all vehicles for collision detection
            light_state: Current traffic light state for this vehicle's direction
            intersection_center: Center of the intersection
            light_just_turned_green: True if light just changed to green

        Wait Time Calculation:
        ---------------------
        If the vehicle's speed is below WAIT_SPEED_THRESHOLD, we accumulate
        wait time. This captures:
        - Time stopped at red lights
        - Time in queue behind other vehicles
        - Time during slow crawl in congestion
        """
        # Handle reaction delay when light turns green
        if light_just_turned_green and self.waiting_for_green:
            self.has_reacted = False
            self.reaction_timer = 0.0
            self.reaction_delay = random.uniform(
                REACTION_DELAY_MIN, REACTION_DELAY_MAX)

        # Update reaction timer
        if not self.has_reacted:
            self.reaction_timer += dt
            if self.reaction_timer >= self.reaction_delay:
                self.has_reacted = True
                self.waiting_for_green = False

        # Determine target speed based on conditions
        should_stop = False

        # Check traffic light
        if self.should_stop_for_light(light_state, intersection_center):
            should_stop = True
            self.waiting_for_green = True

        # Check for vehicles ahead (car-following model)
        min_distance = float('inf')
        for other in vehicles:
            if other.id == self.id:
                continue
            if self.is_ahead_of_me(other):
                dist = self.distance_to(other)
                if 0 < dist < min_distance:
                    min_distance = dist

        # Apply car-following model
        if min_distance < SAFE_DISTANCE:
            should_stop = True
        elif min_distance < SAFE_DISTANCE * 3:
            # Gradual slowdown as we approach vehicle ahead
            speed_factor = (min_distance - SAFE_DISTANCE) / (SAFE_DISTANCE * 2)
            self.target_speed = self.config.max_speed * max(0.2, speed_factor)
        else:
            if not should_stop:
                self.target_speed = self.config.max_speed

        # Don't move if waiting for reaction delay
        if not self.has_reacted and self.waiting_for_green:
            should_stop = True

        # Update speed with acceleration/deceleration
        if should_stop:
            self.target_speed = 0

        if self.speed < self.target_speed:
            # Accelerate
            self.speed = min(
                self.speed + self.config.acceleration * dt, self.target_speed)
        elif self.speed > self.target_speed:
            # Decelerate
            self.speed = max(
                self.speed - self.config.deceleration * dt, self.target_speed)

        # Update position based on direction and speed
        distance = self.speed * dt
        
        # Check if should turn at intersection
        if self.should_turn(intersection_center):
            self.is_turning = True
            self.direction = self.get_new_direction_after_turn()
        
        # Move vehicle
        if self.direction == Direction.NORTH:
            self.y -= distance
        elif self.direction == Direction.SOUTH:
            self.y += distance
        elif self.direction == Direction.EAST:
            self.x += distance
        else:  # WEST
            self.x -= distance
        
        # Reset turning flag when vehicle leaves the wider turn zone
        if self.is_turning and not self.is_in_turn_zone(intersection_center):
            self.is_turning = False

        # =====================================================================
        # WAIT TIME CALCULATION
        # =====================================================================
        # Wait time accumulates when the vehicle is moving slowly or stopped.
        # This represents the delay experienced by the vehicle due to:
        # 1. Red traffic lights
        # 2. Queue behind other vehicles
        # 3. Congestion causing slow movement
        #
        # We use a speed threshold rather than checking for zero speed because:
        # - Vehicles may creep forward slowly in heavy traffic
        # - This slow movement still represents "waiting" from user perspective
        # - It provides more accurate congestion metrics
        # =====================================================================
        if self.speed < self.WAIT_SPEED_THRESHOLD:
            self.wait_time += dt

        # Check if vehicle is off-screen
        margin = 100
        if (self.x < -margin or self.x > SCREEN_WIDTH + margin or
                self.y < -margin or self.y > SCREEN_HEIGHT + margin):
            self.active = False

    def draw(self, screen: pygame.Surface):
        """Draw the vehicle on screen."""
        rect = self.get_rect()

        # Try to use sprite if available
        if self.spritesheet and self.spritesheet.loaded:
            # Map vehicle type to sprite position
            sprite_map = {
                VehicleType.CAR: (0, 0),
                VehicleType.BIKE: (1, 0),
                VehicleType.TRUCK: (2, 0),
            }
            col, row = sprite_map.get(self.vehicle_type, (0, 0))
            sprite = self.spritesheet.get_tile(col, row)
            if sprite:
                # Rotate sprite based on direction
                angle = {
                    Direction.NORTH: 0,
                    Direction.SOUTH: 180,
                    Direction.EAST: -90,
                    Direction.WEST: 90,
                }.get(self.direction, 0)
                rotated = pygame.transform.rotate(sprite, angle)
                # Scale to vehicle size
                scaled = pygame.transform.scale(
                    rotated, (int(rect.width), int(rect.height)))
                screen.blit(scaled, rect.topleft)
                return

        # Fallback: Draw colored rectangle
        pygame.draw.rect(screen, self.config.color, rect)
        pygame.draw.rect(screen, (0, 0, 0), rect, 2)

        # Draw direction indicator (small triangle at front)
        front_x, front_y = self.get_front_position()
        indicator_size = 5
        if self.direction == Direction.NORTH:
            points = [(front_x, front_y), (front_x - indicator_size, front_y + indicator_size),
                      (front_x + indicator_size, front_y + indicator_size)]
        elif self.direction == Direction.SOUTH:
            points = [(front_x, front_y), (front_x - indicator_size, front_y - indicator_size),
                      (front_x + indicator_size, front_y - indicator_size)]
        elif self.direction == Direction.EAST:
            points = [(front_x, front_y), (front_x - indicator_size, front_y - indicator_size),
                      (front_x - indicator_size, front_y + indicator_size)]
        else:
            points = [(front_x, front_y), (front_x + indicator_size, front_y - indicator_size),
                      (front_x + indicator_size, front_y + indicator_size)]
        pygame.draw.polygon(screen, (255, 255, 255), points)


# =============================================================================
# TRAFFIC LIGHT CLASS
# =============================================================================

class TrafficLight:
    """
    Traffic Light Class
    ===================
    Manages traffic light timing and states for a two-phase system.

    Phase System:
    - Phase 1: North-South GREEN, East-West RED
    - Yellow transition (fixed 3 seconds)
    - Phase 2: North-South RED, East-West GREEN
    - Yellow transition (fixed 3 seconds)
    - Repeat

    The yellow phase is always 3 seconds as per traffic engineering standards.
    """

    def __init__(self, ns_green_duration: float = 30.0, ew_green_duration: float = 30.0):
        """
        Initialize traffic light controller.

        Args:
            ns_green_duration: Duration of North-South green phase (seconds)
            ew_green_duration: Duration of East-West green phase (seconds)
        """
        self.ns_green_duration = ns_green_duration
        self.ew_green_duration = ew_green_duration

        # Current states
        self.ns_state = LightState.GREEN
        self.ew_state = LightState.RED

        # Timing
        self.phase_timer = 0.0
        self.current_phase = "NS_GREEN"  # NS_GREEN, NS_YELLOW, EW_GREEN, EW_YELLOW

        # Track state changes for vehicle reaction
        self.ns_just_turned_green = False
        self.ew_just_turned_green = False

        # Statistics
        self.cycle_count = 0

    def get_state(self, direction: Direction) -> LightState:
        """Get the current light state for a direction."""
        if direction in [Direction.NORTH, Direction.SOUTH]:
            return self.ns_state
        else:
            return self.ew_state

    def update(self, dt: float):
        """
        Update traffic light timing.

        Args:
            dt: Delta time in seconds
        """
        self.phase_timer += dt
        self.ns_just_turned_green = False
        self.ew_just_turned_green = False

        if self.current_phase == "NS_GREEN":
            if self.phase_timer >= self.ns_green_duration:
                self.current_phase = "NS_YELLOW"
                self.ns_state = LightState.YELLOW
                self.phase_timer = 0.0

        elif self.current_phase == "NS_YELLOW":
            if self.phase_timer >= YELLOW_DURATION:
                self.current_phase = "EW_GREEN"
                self.ns_state = LightState.RED
                self.ew_state = LightState.GREEN
                self.ew_just_turned_green = True
                self.phase_timer = 0.0

        elif self.current_phase == "EW_GREEN":
            if self.phase_timer >= self.ew_green_duration:
                self.current_phase = "EW_YELLOW"
                self.ew_state = LightState.YELLOW
                self.phase_timer = 0.0

        elif self.current_phase == "EW_YELLOW":
            if self.phase_timer >= YELLOW_DURATION:
                self.current_phase = "NS_GREEN"
                self.ew_state = LightState.RED
                self.ns_state = LightState.GREEN
                self.ns_just_turned_green = True
                self.phase_timer = 0.0
                self.cycle_count += 1

    def get_time_remaining(self) -> float:
        """Get time remaining in current phase."""
        if self.current_phase == "NS_GREEN":
            return self.ns_green_duration - self.phase_timer
        elif self.current_phase == "EW_GREEN":
            return self.ew_green_duration - self.phase_timer
        else:
            return YELLOW_DURATION - self.phase_timer

    def draw(self, screen: pygame.Surface, intersection_center: Tuple[float, float],
             spritesheet: Optional[SpriteSheet] = None):
        """
        Draw traffic lights at the intersection.

        Args:
            screen: Pygame surface to draw on
            intersection_center: Center point of intersection
            spritesheet: Optional spritesheet for light graphics
        """
        cx, cy = intersection_center
        light_offset = ROAD_WIDTH / 2 + 30
        light_size = 20

        # Light positions: [NS lights, EW lights]
        positions = [
            # North-South lights (top and bottom of intersection)
            (cx - 35, cy - light_offset, self.ns_state),  # Top
            (cx + 35, cy + light_offset, self.ns_state),  # Bottom
            # East-West lights (left and right of intersection)
            (cx - light_offset, cy + 35, self.ew_state),  # Left
            (cx + light_offset, cy - 35, self.ew_state),  # Right
        ]

        for lx, ly, state in positions:
            # Draw light housing
            housing_rect = pygame.Rect(lx - 15, ly - 35, 30, 70)
            pygame.draw.rect(screen, (30, 30, 30), housing_rect)
            pygame.draw.rect(screen, (60, 60, 60), housing_rect, 2)

            # Draw three circles for R, Y, G
            colors = [
                (COLOR_RED if state == LightState.RED else COLOR_LIGHT_OFF, ly - 20),
                (COLOR_YELLOW if state == LightState.YELLOW else COLOR_LIGHT_OFF, ly),
                (COLOR_GREEN if state == LightState.GREEN else COLOR_LIGHT_OFF, ly + 20),
            ]

            for color, y_pos in colors:
                pygame.draw.circle(
                    screen, color, (int(lx), int(y_pos)), light_size // 2)
                # Add glow effect for active light
                if color not in [COLOR_LIGHT_OFF]:
                    glow_surface = pygame.Surface(
                        (light_size * 2, light_size * 2), pygame.SRCALPHA)
                    pygame.draw.circle(glow_surface, (*color[:3], 50),
                                       (light_size, light_size), light_size)
                    screen.blit(
                        glow_surface, (lx - light_size, y_pos - light_size))


# =============================================================================
# UI COMPONENTS
# =============================================================================

class Slider:
    """
    Slider UI Component
    ===================
    A draggable slider for adjusting numerical values.
    """

    def __init__(self, x: int, y: int, width: int, height: int,
                 min_val: float, max_val: float, initial_val: float,
                 label: str, format_str: str = "{:.1f}"):
        """
        Initialize slider.

        Args:
            x, y: Position
            width, height: Dimensions
            min_val, max_val: Value range
            initial_val: Starting value
            label: Text label
            format_str: Format string for value display
        """
        self.rect = pygame.Rect(x, y, width, height)
        self.min_val = min_val
        self.max_val = max_val
        self.value = initial_val
        self.label = label
        self.format_str = format_str
        self.dragging = False
        self.knob_radius = height // 2 + 2

    def get_knob_x(self) -> int:
        """Calculate knob X position based on value."""
        ratio = (self.value - self.min_val) / (self.max_val - self.min_val)
        return int(self.rect.x + ratio * self.rect.width)

    def handle_event(self, event: pygame.event.Event) -> bool:
        """
        Handle mouse events.

        Returns:
            True if value changed
        """
        if event.type == pygame.MOUSEBUTTONDOWN:
            knob_x = self.get_knob_x()
            knob_rect = pygame.Rect(knob_x - self.knob_radius,
                                    self.rect.centery - self.knob_radius,
                                    self.knob_radius * 2, self.knob_radius * 2)
            if knob_rect.collidepoint(event.pos) or self.rect.collidepoint(event.pos):
                self.dragging = True
                return self._update_value(event.pos[0])

        elif event.type == pygame.MOUSEBUTTONUP:
            self.dragging = False

        elif event.type == pygame.MOUSEMOTION and self.dragging:
            return self._update_value(event.pos[0])

        return False

    def _update_value(self, mouse_x: int) -> bool:
        """Update value based on mouse position."""
        old_value = self.value
        ratio = (mouse_x - self.rect.x) / self.rect.width
        ratio = max(0, min(1, ratio))
        self.value = self.min_val + ratio * (self.max_val - self.min_val)
        return self.value != old_value

    def draw(self, screen: pygame.Surface, font: pygame.font.Font):
        """Draw the slider."""
        # Draw label
        label_surface = font.render(self.label, True, COLOR_UI_TEXT)
        screen.blit(label_surface, (self.rect.x, self.rect.y - 25))

        # Draw track
        pygame.draw.rect(screen, COLOR_UI_SLIDER_BG,
                         self.rect, border_radius=5)

        # Draw filled portion
        knob_x = self.get_knob_x()
        filled_rect = pygame.Rect(self.rect.x, self.rect.y,
                                  knob_x - self.rect.x, self.rect.height)
        pygame.draw.rect(screen, COLOR_UI_SLIDER_FG,
                         filled_rect, border_radius=5)

        # Draw knob
        pygame.draw.circle(screen, (255, 255, 255),
                           (knob_x, self.rect.centery), self.knob_radius)
        pygame.draw.circle(screen, COLOR_UI_SLIDER_FG,
                           (knob_x, self.rect.centery), self.knob_radius - 3)

        # Draw value
        value_str = self.format_str.format(self.value)
        value_surface = font.render(value_str, True, COLOR_UI_TEXT)
        screen.blit(value_surface, (self.rect.right +
                    10, self.rect.centery - 8))


class Button:
    """Simple button UI component."""

    def __init__(self, x: int, y: int, width: int, height: int, text: str):
        self.rect = pygame.Rect(x, y, width, height)
        self.text = text
        self.hovered = False

    def handle_event(self, event: pygame.event.Event) -> bool:
        """Returns True if button was clicked."""
        if event.type == pygame.MOUSEMOTION:
            self.hovered = self.rect.collidepoint(event.pos)
        elif event.type == pygame.MOUSEBUTTONDOWN:
            if self.rect.collidepoint(event.pos):
                return True
        return False

    def draw(self, screen: pygame.Surface, font: pygame.font.Font):
        """Draw the button."""
        color = COLOR_UI_BUTTON_HOVER if self.hovered else COLOR_UI_BUTTON
        pygame.draw.rect(screen, color, self.rect, border_radius=5)
        pygame.draw.rect(screen, (255, 255, 255),
                         self.rect, 2, border_radius=5)

        text_surface = font.render(self.text, True, COLOR_UI_TEXT)
        text_rect = text_surface.get_rect(center=self.rect.center)
        screen.blit(text_surface, text_rect)


class TextInputField:
    """Text input field for numerical input."""
    
    def __init__(self, x: int, y: int, width: int, height: int, 
                 label: str, min_val: float = 0.0, max_val: float = 1.0):
        self.rect = pygame.Rect(x, y, width, height)
        self.label = label
        self.text = str(min_val)
        self.active = False
        self.min_val = min_val
        self.max_val = max_val
        self.value = min_val
        self.cursor_visible = True
        self.cursor_timer = 0.0
    
    def handle_event(self, event: pygame.event.Event) -> bool:
        """Handle mouse and keyboard events. Returns True if value changed."""
        if event.type == pygame.MOUSEBUTTONDOWN:
            self.active = self.rect.collidepoint(event.pos)
            self.cursor_timer = 0.0
            self.cursor_visible = True
            return False
        
        elif event.type == pygame.KEYDOWN and self.active:
            if event.key == pygame.K_RETURN:
                self.active = False
                return self._validate_input()
            elif event.key == pygame.K_BACKSPACE:
                self.text = self.text[:-1]
                return False
            elif event.unicode.isdigit() or event.unicode == '.':
                self.text += event.unicode
                return False
        
        return False
    
    def _validate_input(self) -> bool:
        """Validate and clamp the input value."""
        try:
            new_value = float(self.text)
            new_value = max(self.min_val, min(self.max_val, new_value))
            if new_value != self.value:
                self.value = new_value
                self.text = str(round(new_value, 3))
                return True
        except ValueError:
            self.text = str(round(self.value, 3))
        return False
    
    def update(self, dt: float):
        """Update cursor blink."""
        self.cursor_timer += dt
        if self.cursor_timer >= 0.5:
            self.cursor_visible = not self.cursor_visible
            self.cursor_timer = 0.0
    
    def draw(self, screen: pygame.Surface, font: pygame.font.Font):
        """Draw the text input field."""
        # Draw label
        label_surface = font.render(self.label, True, COLOR_UI_TEXT)
        screen.blit(label_surface, (self.rect.x, self.rect.y - 25))
        
        # Draw input box
        border_color = (100, 200, 100) if self.active else (80, 80, 80)
        pygame.draw.rect(screen, COLOR_UI_SLIDER_BG, self.rect)
        pygame.draw.rect(screen, border_color, self.rect, 2)
        
        # Draw text
        text_surface = font.render(self.text, True, COLOR_UI_TEXT)
        screen.blit(text_surface, (self.rect.x + 5, self.rect.y + 5))
        
        # Draw cursor
        if self.active and self.cursor_visible:
            cursor_x = self.rect.x + 5 + text_surface.get_width() + 2
            pygame.draw.line(screen, COLOR_UI_TEXT, 
                           (cursor_x, self.rect.y + 3),
                           (cursor_x, self.rect.y + self.rect.height - 3), 2)


# =============================================================================
# SIMULATION CLASS
# =============================================================================

class Simulation:
    """
    Main Simulation Class
    =====================
    Manages the entire traffic simulation including:
    - Main game loop
    - Event handling
    - Vehicle spawning
    - Rendering
    - Statistics tracking
    """

    def __init__(self):
        """Initialize the simulation."""
        # Display setup
        self.screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT), pygame.SHOWN)
        pygame.display.set_caption("Traffic Light Scenario Testing Tool")
        # Force window to foreground on some systems
        import ctypes
        try:
            ctypes.windll.kernel32.SetForegroundWindow(pygame.display.get_surface().get_flags())
        except:
            pass
        self.clock = pygame.time.Clock()
        self.font = pygame.font.Font(None, 24)
        self.font_large = pygame.font.Font(None, 32)
        self.font_title = pygame.font.Font(None, 40)

        # Load spritesheets
        self.road_spritesheet = SpriteSheet("road_tiles.png")
        self.vehicle_spritesheet = SpriteSheet("vehicle_sprites.png")

        # Intersection center
        self.intersection_center = (
            SCREEN_WIDTH // 2 - 100, SCREEN_HEIGHT // 2)

        # Game objects
        self.traffic_light = TrafficLight(
            ns_green_duration=30.0, ew_green_duration=30.0)
        self.vehicles: List[Vehicle] = []

        # Spawn configuration
        self.spawn_probability = 0.02  # Probability per frame per lane
        self.spawn_timer = 0.0
        self.spawn_interval = 0.1  # Check spawn every 0.1 seconds

        # UI Elements
        ui_x = SCREEN_WIDTH - 300
        self.sliders = {
            'ns_green': Slider(ui_x, 80, 220, 15, 10, 60, 30, "Durasi Hijau U-S (detik)"),
            'ew_green': Slider(ui_x, 150, 220, 15, 10, 60, 30, "Durasi Hijau T-B (detik)"),
            'density': Slider(ui_x, 220, 220, 15, 0.001, 0.2, 0.02, "Kepadatan Lalu Lintas", "{:.4f}"),
        }
        # Text input field for manual density input
        self.density_input = TextInputField(ui_x, 270, 220, 25, "Input Kepadatan:", 0.001, 0.2)
        self.reset_button = Button(ui_x, 310, 220, 45, "Reset Simulasi")

        # Statistics
        self.stats = {
            'ns_queue': 0,
            'ew_queue': 0,
            'ns_wait_time': 0.0,
            'ew_wait_time': 0.0,
            'total_vehicles': 0,
            'completed_vehicles': 0,
        }

        # For calculating average wait time of completed vehicles
        self.completed_wait_times: List[float] = []

        # Running state
        self.running = True
        self.paused = False

    def spawn_vehicles(self, dt: float):
        """
        Spawn vehicles based on density setting.
        
        SISTEM LAJUR KIRI INDONESIA (Keep Left Rule):
        - Utara→Selatan: Lajur KIRI = sisi BARAT (cx - LANE_WIDTH/2)
        - Selatan→Utara: Lajur KIRI = sisi TIMUR (cx + LANE_WIDTH/2)
        - Timur→Barat: Lajur KIRI = sisi UTARA (cy - LANE_WIDTH/2)
        - Barat→Timur: Lajur KIRI = sisi SELATAN (cy + LANE_WIDTH/2)

        Vehicles spawn from the edges of the screen in keep-left lanes.
        """
        self.spawn_timer += dt
        if self.spawn_timer < self.spawn_interval:
            return
        self.spawn_timer = 0.0

        cx, cy = self.intersection_center
        spawn_margin = 50

        # Spawn points for each direction - LAJUR KIRI SYSTEM (CORRECTED)
        # Koordinat dan Direction harus sesuai:
        # UTARA→SELATAN: dari atas ke bawah (spawn top, Direction.SOUTH, Y meningkat)
        # SELATAN→UTARA: dari bawah ke atas (spawn bottom, Direction.NORTH, Y menurun)
        spawn_configs = [
            # (x, y, direction, description)
            # Utara→Selatan: spawn dari ATAS, Direction.SOUTH, Lajur KIRI = sisi BARAT
            (cx - LANE_WIDTH // 2, -spawn_margin, Direction.SOUTH, "UTARA→SELATAN (Lajur Kiri/Barat)"),
            # Selatan→Utara: spawn dari BAWAH, Direction.NORTH, Lajur KIRI = sisi TIMUR
            (cx + LANE_WIDTH // 2, SCREEN_HEIGHT + spawn_margin, Direction.NORTH, "SELATAN→UTARA (Lajur Kiri/Timur)"),
            # Timur→Barat: spawn dari KANAN, Direction.WEST, Lajur KIRI = sisi UTARA
            (SCREEN_WIDTH + spawn_margin, cy - LANE_WIDTH // 2, Direction.WEST, "TIMUR→BARAT (Lajur Kiri/Utara)"),
            # Barat→Timur: spawn dari KIRI, Direction.EAST, Lajur KIRI = sisi SELATAN
            (-spawn_margin, cy + LANE_WIDTH // 2, Direction.EAST, "BARAT→TIMUR (Lajur Kiri/Selatan)"),
        ]

        for x, y, direction, desc in spawn_configs:
            if random.random() < self.spawn_probability:
                # Check if spawn point is clear
                spawn_clear = True
                for v in self.vehicles:
                    if v.direction == direction:
                        dist = math.sqrt((v.x - x) ** 2 + (v.y - y) ** 2)
                        if dist < 80:
                            spawn_clear = False
                            break

                if spawn_clear:
                    # Random vehicle type with weighted probability
                    vehicle_type = random.choices(
                        [VehicleType.CAR, VehicleType.BIKE, VehicleType.TRUCK],
                        weights=[0.6, 0.25, 0.15]
                    )[0]

                    vehicle = Vehicle(
                        x, y, direction, vehicle_type, self.vehicle_spritesheet)
                    self.vehicles.append(vehicle)
                    self.stats['total_vehicles'] += 1

    def update_statistics(self):
        """
        Update simulation statistics.

        Queue Length Calculation:
        ------------------------
        A vehicle is considered "in queue" if:
        1. It's within a certain distance of the intersection
        2. Its speed is below a threshold (indicating it's waiting)

        Average Wait Time Calculation:
        -----------------------------
        We track the wait time of each vehicle. When a vehicle leaves the
        simulation area, its wait time is added to a running average.
        For active vehicles, we show the current average wait time of
        vehicles that are currently waiting.
        """
        cx, cy = self.intersection_center
        queue_distance = 200  # Distance from intersection to count as "in queue"
        speed_threshold = 20  # Speed below which vehicle is "waiting"

        ns_queue = 0
        ew_queue = 0
        ns_total_wait = 0.0
        ew_total_wait = 0.0
        ns_waiting = 0
        ew_waiting = 0

        for v in self.vehicles:
            # Check if in queue zone
            if v.direction == Direction.NORTH:
                in_queue = v.y > cy and v.y < cy + queue_distance
            elif v.direction == Direction.SOUTH:
                in_queue = v.y < cy and v.y > cy - queue_distance
            elif v.direction == Direction.EAST:
                in_queue = v.x < cx and v.x > cx - queue_distance
            else:
                in_queue = v.x > cx and v.x < cx + queue_distance

            if in_queue and v.speed < speed_threshold:
                if v.direction in [Direction.NORTH, Direction.SOUTH]:
                    ns_queue += 1
                    ns_total_wait += v.wait_time
                    ns_waiting += 1
                else:
                    ew_queue += 1
                    ew_total_wait += v.wait_time
                    ew_waiting += 1

        self.stats['ns_queue'] = ns_queue
        self.stats['ew_queue'] = ew_queue
        self.stats['ns_wait_time'] = ns_total_wait / max(1, ns_waiting)
        self.stats['ew_wait_time'] = ew_total_wait / max(1, ew_waiting)

    def reset(self):
        """Reset the simulation to initial state."""
        self.vehicles.clear()
        self.traffic_light = TrafficLight(
            ns_green_duration=self.sliders['ns_green'].value,
            ew_green_duration=self.sliders['ew_green'].value
        )
        self.stats = {
            'ns_queue': 0,
            'ew_queue': 0,
            'ns_wait_time': 0.0,
            'ew_wait_time': 0.0,
            'total_vehicles': 0,
            'completed_vehicles': 0,
        }
        self.completed_wait_times.clear()

    def handle_events(self):
        """Handle pygame events."""
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                self.running = False
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_ESCAPE:
                    self.running = False
                elif event.key == pygame.K_SPACE:
                    self.paused = not self.paused
                elif event.key == pygame.K_r:
                    self.reset()

            # Handle UI elements
            for name, slider in self.sliders.items():
                if slider.handle_event(event):
                    if name == 'ns_green':
                        self.traffic_light.ns_green_duration = slider.value
                    elif name == 'ew_green':
                        self.traffic_light.ew_green_duration = slider.value
                    elif name == 'density':
                        self.spawn_probability = slider.value
                        print(f"Density slider changed to: {self.spawn_probability:.4f}")
            
            # Handle density text input
            if self.density_input.handle_event(event):
                self.spawn_probability = self.density_input.value
                # Also update slider to match
                self.sliders['density'].value = self.density_input.value
                print(f"Density input changed to: {self.spawn_probability:.4f}")

            if self.reset_button.handle_event(event):
                self.reset()

    def update(self, dt: float):
        """Update simulation state."""
        if self.paused:
            return
        
        # Update text input cursor
        self.density_input.update(dt)

        # Update traffic light
        self.traffic_light.update(dt)

        # Spawn new vehicles
        self.spawn_vehicles(dt)

        # Update vehicles
        for vehicle in self.vehicles:
            light_state = self.traffic_light.get_state(vehicle.direction)

            # Check if light just turned green for this direction
            light_just_green = False
            if vehicle.direction in [Direction.NORTH, Direction.SOUTH]:
                light_just_green = self.traffic_light.ns_just_turned_green
            else:
                light_just_green = self.traffic_light.ew_just_turned_green

            vehicle.update(dt, self.vehicles, light_state,
                           self.intersection_center, light_just_green)

        # Remove inactive vehicles and track their wait times
        for vehicle in self.vehicles[:]:
            if not vehicle.active:
                self.completed_wait_times.append(vehicle.wait_time)
                self.stats['completed_vehicles'] += 1
                self.vehicles.remove(vehicle)

        # Update statistics
        self.update_statistics()

    def draw_road(self):
        """Draw the road network."""
        cx, cy = self.intersection_center

        # Fill background with grass
        self.screen.fill(COLOR_GRASS)

        # Draw roads
        # Vertical road (North-South)
        road_ns = pygame.Rect(cx - ROAD_WIDTH // 2, 0,
                              ROAD_WIDTH, SCREEN_HEIGHT)
        pygame.draw.rect(self.screen, COLOR_ROAD, road_ns)

        # Horizontal road (East-West)
        road_ew = pygame.Rect(0, cy - ROAD_WIDTH // 2,
                              SCREEN_WIDTH, ROAD_WIDTH)
        pygame.draw.rect(self.screen, COLOR_ROAD, road_ew)

        # Draw lane markings
        dash_length = 20
        dash_gap = 15

        # Center line - vertical
        for y in range(0, SCREEN_HEIGHT, dash_length + dash_gap):
            if abs(y - cy) > ROAD_WIDTH // 2:  # Skip intersection
                pygame.draw.line(self.screen, COLOR_ROAD_MARKING,
                                 (cx, y), (cx, y + dash_length), 2)

        # Center line - horizontal
        for x in range(0, SCREEN_WIDTH, dash_length + dash_gap):
            if abs(x - cx) > ROAD_WIDTH // 2:  # Skip intersection
                pygame.draw.line(self.screen, COLOR_ROAD_MARKING,
                                 (x, cy), (x + dash_length, cy), 2)

        # Draw stop lines
        stop_offset = ROAD_WIDTH // 2 + 5
        line_width = 4

        # North approach stop line
        pygame.draw.line(self.screen, COLOR_STOP_LINE,
                         (cx - ROAD_WIDTH // 2, cy + stop_offset),
                         (cx, cy + stop_offset), line_width)

        # South approach stop line
        pygame.draw.line(self.screen, COLOR_STOP_LINE,
                         (cx, cy - stop_offset),
                         (cx + ROAD_WIDTH // 2, cy - stop_offset), line_width)

        # East approach stop line
        pygame.draw.line(self.screen, COLOR_STOP_LINE,
                         (cx - stop_offset, cy),
                         (cx - stop_offset, cy + ROAD_WIDTH // 2), line_width)

        # West approach stop line
        pygame.draw.line(self.screen, COLOR_STOP_LINE,
                         (cx + stop_offset, cy - ROAD_WIDTH // 2),
                         (cx + stop_offset, cy), line_width)

        # Draw crosswalks
        crosswalk_width = 8
        crosswalk_gap = 8
        num_stripes = 6

        # Draw crosswalk stripes for each approach
        for i in range(num_stripes):
            offset = i * (crosswalk_width + crosswalk_gap) - \
                (num_stripes * (crosswalk_width + crosswalk_gap)) // 2

            # North crosswalk
            pygame.draw.rect(self.screen, COLOR_ROAD_MARKING,
                             (cx - ROAD_WIDTH // 2 + offset + ROAD_WIDTH // 2,
                              cy + ROAD_WIDTH // 2 + 10,
                              crosswalk_width, 15))

            # South crosswalk
            pygame.draw.rect(self.screen, COLOR_ROAD_MARKING,
                             (cx - ROAD_WIDTH // 2 + offset + ROAD_WIDTH // 2,
                              cy - ROAD_WIDTH // 2 - 25,
                              crosswalk_width, 15))

    def draw_ui(self):
        """Draw the user interface panel."""
        # UI background panel
        ui_panel = pygame.Rect(SCREEN_WIDTH - 320, 0, 320, SCREEN_HEIGHT)
        pygame.draw.rect(self.screen, COLOR_UI_BG, ui_panel)
        pygame.draw.line(self.screen, (100, 100, 100),
                         (SCREEN_WIDTH - 320, 0), (SCREEN_WIDTH - 320, SCREEN_HEIGHT), 2)

        # Title
        title = self.font_title.render("Kontrol Lalu Lintas", True, COLOR_UI_TEXT)
        self.screen.blit(title, (SCREEN_WIDTH - 310, 15))

        # Draw sliders
        for slider in self.sliders.values():
            slider.draw(self.screen, self.font)
        
        # Draw density text input
        self.density_input.draw(self.screen, self.font)
        
        # Draw current density value
        density_display = self.font.render(f"Saat ini: {self.spawn_probability:.4f}", True, (100, 255, 100))
        self.screen.blit(density_display, (SCREEN_WIDTH - 310, 305))
        
        # Draw reset button
        self.reset_button.draw(self.screen, self.font)

        # Statistics section
        stats_y = 370
        stats_title = self.font_large.render("Statistik", True, COLOR_UI_TEXT)
        self.screen.blit(stats_title, (SCREEN_WIDTH - 310, stats_y))

        # Draw statistics
        stats_items = [
            f"Antrian U-S: {self.stats['ns_queue']}",
            f"Antrian T-B: {self.stats['ew_queue']}",
            f"Rata² Tunggu U-S: {self.stats['ns_wait_time']:.1f}d",
            f"Rata² Tunggu T-B: {self.stats['ew_wait_time']:.1f}d",
            f"Total Spawn: {self.stats['total_vehicles']}",
            f"Selesai: {self.stats['completed_vehicles']}",
            f"Kendaraan Aktif: {len(self.vehicles)}",
        ]

        for i, text in enumerate(stats_items):
            surface = self.font.render(text, True, COLOR_UI_TEXT)
            self.screen.blit(
                surface, (SCREEN_WIDTH - 310, stats_y + 35 + i * 25))

        # Current light status
        light_y = stats_y + 35 + len(stats_items) * 25 + 20
        light_title = self.font_large.render(
            "Status Lampu", True, COLOR_UI_TEXT)
        self.screen.blit(light_title, (SCREEN_WIDTH - 310, light_y))

        ns_color = {
            LightState.RED: COLOR_RED,
            LightState.YELLOW: COLOR_YELLOW,
            LightState.GREEN: COLOR_GREEN,
        }[self.traffic_light.ns_state]

        ew_color = {
            LightState.RED: COLOR_RED,
            LightState.YELLOW: COLOR_YELLOW,
            LightState.GREEN: COLOR_GREEN,
        }[self.traffic_light.ew_state]

        # NS light indicator
        pygame.draw.circle(self.screen, ns_color,
                           (SCREEN_WIDTH - 280, light_y + 50), 12)
        ns_state_text = {
            LightState.RED: "MERAH",
            LightState.YELLOW: "KUNING",
            LightState.GREEN: "HIJAU",
        }[self.traffic_light.ns_state]
        ns_text = self.font.render(
            f"U-S: {ns_state_text}", True, COLOR_UI_TEXT)
        self.screen.blit(ns_text, (SCREEN_WIDTH - 260, light_y + 42))

        # EW light indicator
        pygame.draw.circle(self.screen, ew_color,
                           (SCREEN_WIDTH - 280, light_y + 80), 12)
        ew_state_text = {
            LightState.RED: "MERAH",
            LightState.YELLOW: "KUNING",
            LightState.GREEN: "HIJAU",
        }[self.traffic_light.ew_state]
        ew_text = self.font.render(
            f"T-B: {ew_state_text}", True, COLOR_UI_TEXT)
        self.screen.blit(ew_text, (SCREEN_WIDTH - 260, light_y + 72))

        # Time remaining
        time_text = self.font.render(f"Waktu Fase: {self.traffic_light.get_time_remaining():.1f}d",
                                     True, COLOR_UI_TEXT)
        self.screen.blit(time_text, (SCREEN_WIDTH - 310, light_y + 110))

        # Instructions
        inst_y = SCREEN_HEIGHT - 140
        instructions = [
            "Kontrol:",
            "SPASI - Jeda/Lanjut",
            "R - Reset",
            "ESC - Keluar",
            "",
            "Aturan: KIRI JALAN TERUS",
            "Belok kiri saat MERAH BOLEH",
        ]
        for i, text in enumerate(instructions):
            if text == "":
                continue
            color = (150, 150, 150) if i == 0 else (100, 200, 100) if i >= 5 else COLOR_UI_TEXT
            surface = self.font.render(text, True, color)
            self.screen.blit(surface, (SCREEN_WIDTH - 310, inst_y + i * 20))

        # Pause indicator
        if self.paused:
            pause_text = self.font_title.render(
                "DIJEDA", True, (255, 100, 100))
            pause_rect = pause_text.get_rect(
                center=(SCREEN_WIDTH // 2 - 150, 50))
            pygame.draw.rect(self.screen, (0, 0, 0),
                             pause_rect.inflate(20, 10))
            self.screen.blit(pause_text, pause_rect)

    def draw(self):
        """Render the simulation."""
        # Draw road network
        self.draw_road()

        # Draw traffic lights
        self.traffic_light.draw(self.screen, self.intersection_center, None)

        # Draw vehicles (sorted by Y for proper layering)
        sorted_vehicles = sorted(self.vehicles, key=lambda v: v.y)
        for vehicle in sorted_vehicles:
            vehicle.draw(self.screen)

        # Draw UI
        self.draw_ui()

        # Update display
        pygame.display.flip()

    def run(self):
        """Main simulation loop."""
        print("=" * 60)
        print("Simulasi Lalu Lintas Indonesia - Sistem Lajur Kiri")
        print("=" * 60)
        print("\nKontrol:")
        print("  SPASI - Jeda/Lanjut simulasi")
        print("  R     - Reset simulasi")
        print("  ESC   - Keluar")
        print("\nGunakan slider untuk mengatur:")
        print("  - Durasi Hijau Utara-Selatan")
        print("  - Durasi Hijau Timur-Barat")
        print("  - Kepadatan lalu lintas")
        print("\nATURAN KHUSUS:")
        print("  ✓ Kendaraan BOLEH belok kiri saat MERAH (Kiri Jalan Terus)")
        print("  ✓ Kendaraan berhenti di ZEBRA CROSS")
        print("  ✓ Jika belum ke zebra cross, kendaraan bisa terus maju")
        print("=" * 60)

        while self.running:
            dt = self.clock.tick(FPS) / 1000.0  # Convert to seconds

            self.handle_events()
            self.update(dt)
            self.draw()

        pygame.quit()


# =============================================================================
# MAIN ENTRY POINT
# =============================================================================

def main():
    """Main entry point for the simulation."""
    simulation = Simulation()
    simulation.run()


if __name__ == "__main__":
    main()
