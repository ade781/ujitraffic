# Traffic Light Scenario Testing Tool

A 2D top-down traffic simulation built with Python and Pygame. This tool allows traffic engineers to test whether adjusting traffic light durations can resolve congestion at intersections.

## Features

### Visual Elements
- **2D Top-down view** of a 4-way intersection
- **Traffic lights** with realistic Red/Yellow/Green states
- **Multiple vehicle types**: Cars, Motorcycles, and Trucks
- **Road markings**: Lane dividers, stop lines, and crosswalks

### Traffic Light Logic
- **Two-phase system**: North-South (NS) and East-West (EW)
- **Configurable green durations** via sliders (10-60 seconds)
- **Fixed 3-second yellow transition** between phases

### Vehicle Behavior
- **Car-following model**: Vehicles maintain safe distances
- **Staggered start**: Realistic reaction delay when lights turn green
- **Variable speeds**: Different vehicle types have different characteristics
- **Collision avoidance**: Vehicles decelerate when approaching others

### Dashboard & Statistics
- Real-time **queue length** per direction (NS/EW)
- **Average wait time** calculation
- Traffic **density control** slider
- **Reset** functionality

## Installation

1. Ensure you have Python 3.7+ installed
2. Install Pygame:
   ```bash
   pip install pygame
   ```

## Running the Simulation

```bash
python traffic_simulation.py
```

## Controls

| Key | Action |
|-----|--------|
| `SPACE` | Pause/Resume simulation |
| `R` | Reset simulation |
| `ESC` | Quit |

## UI Sliders

- **NS Green Duration**: Adjust how long the North-South direction has a green light (10-60 seconds)
- **EW Green Duration**: Adjust how long the East-West direction has a green light (10-60 seconds)
- **Traffic Density**: Control vehicle spawn probability (affects congestion level)

## Understanding the Statistics

### Queue Length
Number of vehicles currently waiting (speed < 20 px/s) within 200 pixels of the intersection.

### Average Wait Time
The average time vehicles in the queue have been waiting. This is calculated by summing the wait time of all vehicles currently in the queue and dividing by the number of waiting vehicles.

**Wait Time Calculation Logic:**
```
For each simulation frame:
    if vehicle.speed < WAIT_SPEED_THRESHOLD (5 px/s):
        vehicle.wait_time += delta_time
```

This captures:
- Time stopped at red lights
- Time in queue behind other vehicles
- Slow crawl time during congestion

## Asset Files (Optional)

The simulation runs with placeholder graphics by default. For custom sprites, create:

### road_tiles.png (3x3 grid, 40x40 pixels per tile)
```
[0,0] Straight road vertical    [1,0] Straight road horizontal  [2,0] Intersection
[0,1] Corner NE                 [1,1] Corner NW                 [2,1] Corner SE
[0,2] Corner SW                 [1,2] T-junction                [2,2] Stop line
```

### vehicle_sprites.png (3x3 grid, 40x40 pixels per tile)
```
[0,0] Car (facing up)           [1,0] Motorcycle (facing up)    [2,0] Truck (facing up)
[0,1] Car alternate color       [1,1] Motorcycle alt            [2,1] Truck alt
[0,2] Reserved                  [1,2] Reserved                  [2,2] Reserved
```

## Code Architecture

### Classes

1. **SpriteSheet**: Handles loading and cropping of 3x3 grid spritesheets
2. **Vehicle**: Manages individual vehicle behavior, movement, and state
3. **TrafficLight**: Controls light timing and state transitions
4. **Slider/Button**: UI components for user interaction
5. **Simulation**: Main game loop, event handling, and rendering

### Vehicle Types

| Type | Length | Max Speed | Acceleration | Color |
|------|--------|-----------|--------------|-------|
| Car | 30px | 150 px/s | 80 px/s² | Blue |
| Bike | 20px | 180 px/s | 120 px/s² | Orange |
| Truck | 50px | 100 px/s | 40 px/s² | Brown |

### Timing Constants

- Yellow light duration: 3 seconds (fixed)
- Reaction delay: 0.2-0.8 seconds (randomized per vehicle)
- Safe following distance: 30 pixels
- Stop line distance from center: 80 pixels

## Experimenting with Configurations

### Scenario 1: Balanced Traffic
- Set NS and EW green durations to 30 seconds each
- Set density to 0.03
- Observe queue lengths stabilize

### Scenario 2: Heavy NS Traffic
- Increase NS green to 45 seconds
- Decrease EW green to 20 seconds
- Set density to 0.05
- Observe if NS queues decrease

### Scenario 3: Stress Test
- Set both durations to minimum (10 seconds)
- Set density to maximum (0.1)
- Observe congestion buildup

## Troubleshooting

**Simulation runs slowly?**
- Reduce traffic density slider
- Close other applications

**Vehicles overlap?**
- This shouldn't happen with the car-following model
- If it does, check for very high spawn densities

**No graphics showing?**
- Ensure Pygame is properly installed
- Check terminal for error messages

## License

MIT License - Feel free to modify and use for educational purposes.
