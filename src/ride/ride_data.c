/*
 * Whether a particular ride has a running track or not. Will probably end up
 * being used in various places in the game.
 *
 * Data source is 0x0097E3AC
 *
 * Generating function is here
 * https://gist.github.com/kevinburke/eaeb1d8149a6eef0dcc1
 */

#include "../localisation/localisation.h"
#include "ride.h"
#include "ride_data.h"

const bool hasRunningTrack[0x60] = {
	true,	// 0 Spiral Roller coaster
	true,	// 1 Stand Up Coaster
	true,	// 2 Suspended Swinging
	true,	// 3 Inverted
	true,	// 4 Steel Mini Coaster
	true,	// 5 Mini Railroad
	true,	// 6 Monorail
	true,	// 7 Mini Suspended Coaster
	false,	// 8 Boat Ride
	true,	// 9 Wooden Wild Mine/Mouse
	true,	// a Steeplechase/Motorbike/Soap Box Derby
	true,	// b Car Ride
	true,	// c Launched Freefall
	true,	// d Bobsleigh Coaster
	true,	// e Observation Tower
	true,	// f Looping Roller Coaster
	true,	// 10 Dinghy Slide
	true,	// 11 Mine Train Coaster
	true,	// 12 Chairlift
	true,	// 13 Corkscrew Roller Coaster
	false,	// 14 Maze
	false,	// 15 Spiral Slide
	true,	// 16 Go Karts
	true,	// 17 Log Flume
	true,	// 18 River Rapids
	false,	// 19 Bumper Cars
	false,	// 1a Pirate Ship
	false,	// 1b Swinging Inverter Ship
	false,	// 1c Food Stall
	false,	// 1d (none)
	false,	// 1e Drink Stall
	false,	// 1f (none)
	false,	// 20 Shop (all types)
	false,	// 21 Merry Go Round
	false,	// 22 Balloon Stall (maybe)
	false,	// 23 Information Kiosk
	false,	// 24 Bathroom
	false,	// 25 Ferris Wheel
	false,	// 26 Motion Simulator
	false,	// 27 3D Cinema
	false,	// 28 Topspin
	false,	// 29 Space Rings
	true,	// 2a Reverse Freefall Coaster
	true,	// 2b Elevator
	true,	// 2c Vertical Drop Roller Coaster
	false,	// 2d ATM
	false,	// 2e Twist
	false,	// 2f Haunted House
	false,	// 30 First Aid
	false,	// 31 Circus Show
	true,	// 32 Ghost Train
	true,	// 33 Twister Roller Coaster
	true,	// 34 Wooden Roller Coaster
	true,	// 35 Side-Friction Roller Coaster
	true,	// 36 Wild Mouse
	true,	// 37 Multi Dimension Coaster
	true,	// 38 (none)
	true,	// 39 Flying Roller Coaster
	true,	// 3a (none)
	true,	// 3b Virginia Reel
	true,	// 3c Splash Boats
	true,	// 3d Mini Helicopters
	true,	// 3e Lay-down Roller Coaster
	true,	// 3f Suspended Monorail
	true,	// 40 (none)
	true,	// 41 Reverser Roller Coaster
	true,	// 42 Heartline Twister Roller Coaster
	true,	// 43 Mini Golf
	true,	// 44 Giga Coaster
	true,	// 45 Roto-Drop
	false,	// 46 Flying Saucers
	false,	// 47 Crooked House
	true,	// 48 Monorail Cycles
	true,	// 49 Compact Inverted Coaster
	true,	// 4a Water Coaster
	true,	// 4b Air Powered Vertical Coaster
	true,	// 4c Inverted Hairpin Coaster
	false,	// 4d Magic Carpet
	false,	// 4e Submarine Ride
	true,	// 4f River Rafts
	false,	// 50 (none)
	false,	// 51 Enterprise
	false,	// 52 (none)
	false,	// 53 (none)
	false,	// 54 (none)
	true,	// 55 (none)
	true,	// 56 Inverted Impulse Coaster
	true,	// 57 Mini Roller Coaster
	true,	// 58 Mine Ride
	true,	// 59 LIM Launched Roller Coaster
};

/**
 * Data about ride running costs. This is widely adjusted by the upkeep
 * function, so values that don't make much sense here (a roller coaster having
 * cheaper upkeep than a car ride) are fixed later on.
 *
 * data generation script: https://gist.github.com/kevinburke/6bcf4a8fcc95faad7bac
 */
const uint8 initialUpkeepCosts[0x60] = {
	41,	// 00 Spiral Roller coaster
	40,	// 01 Stand Up Coaster
	40,	// 02 Suspended Swinging
	40,	// 03 Inverted
	40,	// 04 Steel Mini Coaster
	60,	// 05 Mini Railroad
	65,	// 06 Monorail
	40,	// 07 Mini Suspended Coaster
	50,	// 08 Boat Ride
	40,	// 09 Wooden Wild Mine/Mouse
	40,	// 0a Steeplechase/Motorbike/Soap Box Derby
	70,	// 0b Car Ride
	50,	// 0c Launched Freefall
	40,	// 0d Bobsleigh Coaster
	50,	// 0e Observation Tower
	40,	// 0f Looping Roller Coaster
	40,	// 10 Dinghy Slide
	40,	// 11 Mine Train Coaster
	60,	// 12 Chairlift
	40,	// 13 Corkscrew Roller Coaster
	50,	// 14 Maze
	50,	// 15 Spiral Slide
	50,	// 16 Go Karts
	80,	// 17 Log Flume
	82,	// 18 River Rapids
	50,	// 19 Bumper Cars
	50,	// 1a Pirate Ship
	50,	// 1b Swinging Inverter Ship
	50,	// 1c Food Stall
	50,	// 1d (none)
	50,	// 1e Drink Stall
	50,	// 1f (none)
	50,	// 20 Shop (all types)
	50,	// 21 Merry Go Round
	50,	// 22 Balloon Stall (maybe)
	50,	// 23 Information Kiosk
	50,	// 24 Bathroom
	50,	// 25 Ferris Wheel
	50,	// 26 Motion Simulator
	50,	// 27 3D Cinema
	50,	// 28 Topspin
	50,	// 29 Space Rings
	80,	// 2a Reverse Freefall Coaster
	50,	// 2b Elevator
	44,	// 2c Vertical Drop Roller Coaster
	40,	// 2d ATM
	50,	// 2e Twist
	50,	// 2f Haunted House
	45,	// 30 First Aid
	50,	// 31 Circus Show
	80,	// 32 Ghost Train
	43,	// 33 Twister Roller Coaster
	40,	// 34 Wooden Roller Coaster
	39,	// 35 Side-Friction Roller Coaster
	40,	// 36 Wild Mouse
	75,	// 37 Multi Dimension Coaster
	75,	// 38 (none)
	49,	// 39 Flying Roller Coaster
	49,	// 3a (none)
	39,	// 3b Virginia Reel
	70,	// 3c Splash Boats
	70,	// 3d Mini Helicopters
	49,	// 3e Lay-down Roller Coaster
	70,	// 3f Suspended Monorail
	49,	// 40 (none)
	39,	// 41 Reverser Roller Coaster
	47,	// 42 Heartline Twister Roller Coaster
	30,	// 43 Mini Golf
	10,	// 44 Giga Coaster
	50,	// 45 Roto-Drop
	90,	// 46 Flying Saucers
	30,	// 47 Crooked House
	47,	// 48 Monorail Cycles
	40,	// 49 Compact Inverted Coaster
	60,	// 4a Water Coaster
	90,	// 4b Air Powered Vertical Coaster
	40,	// 4c Inverted Hairpin Coaster
	50,	// 4d Magic Carpet
	50,	// 4e Submarine Ride
	50,	// 4f River Rafts
	50,	// 50 (none)
	50,	// 51 Enterprise
	50,	// 52 (none)
	50,	// 53 (none)
	50,	// 54 (none)
	40,	// 55 (none)
	180,	// 56 Inverted Impulse Coaster
	35,	// 57 Mini Roller Coaster
	50,	// 58 Mine Ride
	42,	// 59 LIM Launched Roller Coaster
};

const uint8 costPerTrackPiece[0x60] = {
	80,	// 00 Spiral Roller coaster
	80,	// 01 Stand Up Coaster
	80,	// 02 Suspended Swinging
	80,	// 03 Inverted
	80,	// 04 Steel Mini Coaster
	0,	// 05 Mini Railroad
	0,	// 06 Monorail
	80,	// 07 Mini Suspended Coaster
	0,	// 08 Boat Ride
	80,	// 09 Wooden Wild Mine/Mouse
	80,	// 0a Steeplechase/Motorbike/Soap Box Derby
	0,	// 0b Car Ride
	0,	// 0c Launched Freefall
	80,	// 0d Bobsleigh Coaster
	0,	// 0e Observation Tower
	80,	// 0f Looping Roller Coaster
	80,	// 10 Dinghy Slide
	80,	// 11 Mine Train Coaster
	0,	// 12 Chairlift
	80,	// 13 Corkscrew Roller Coaster
	0,	// 14 Maze
	0,	// 15 Spiral Slide
	0,	// 16 Go Karts
	0,	// 17 Log Flume
	0,	// 18 River Rapids
	0,	// 19 Bumper Cars
	0,	// 1a Pirate Ship
	0,	// 1b Swinging Inverter Ship
	0,	// 1c Food Stall
	0,	// 1d (none)
	0,	// 1e Drink Stall
	0,	// 1f (none)
	0,	// 20 Shop (all types)
	0,	// 21 Merry Go Round
	0,	// 22 Balloon Stall (maybe)
	0,	// 23 Information Kiosk
	0,	// 24 Bathroom
	0,	// 25 Ferris Wheel
	0,	// 26 Motion Simulator
	0,	// 27 3D Cinema
	0,	// 28 Topspin
	0,	// 29 Space Rings
	0,	// 2a Reverse Freefall Coaster
	0,	// 2b Elevator
	80,	// 2c Vertical Drop Roller Coaster
	0,	// 2d ATM
	0,	// 2e Twist
	0,	// 2f Haunted House
	0,	// 30 First Aid
	0,	// 31 Circus Show
	0,	// 32 Ghost Train
	80,	// 33 Twister Roller Coaster
	80,	// 34 Wooden Roller Coaster
	80,	// 35 Side-Friction Roller Coaster
	80,	// 36 Wild Mouse
	90,	// 37 Multi Dimension Coaster
	90,	// 38 (none)
	90,	// 39 Flying Roller Coaster
	90,	// 3a (none)
	80,	// 3b Virginia Reel
	0,	// 3c Splash Boats
	0,	// 3d Mini Helicopters
	90,	// 3e Lay-down Roller Coaster
	0,	// 3f Suspended Monorail
	90,	// 40 (none)
	80,	// 41 Reverser Roller Coaster
	80,	// 42 Heartline Twister Roller Coaster
	80,	// 43 Mini Golf
	80,	// 44 Giga Coaster
	0,	// 45 Roto-Drop
	0,	// 46 Flying Saucers
	0,	// 47 Crooked House
	0,	// 48 Monorail Cycles
	80,	// 49 Compact Inverted Coaster
	80,	// 4a Water Coaster
	0,	// 4b Air Powered Vertical Coaster
	80,	// 4c Inverted Hairpin Coaster
	0,	// 4d Magic Carpet
	0,	// 4e Submarine Ride
	0,	// 4f River Rafts
	0,	// 50 (none)
	0,	// 51 Enterprise
	0,	// 52 (none)
	0,	// 53 (none)
	0,	// 54 (none)
	80,	// 55 (none)
	80,	// 56 Inverted Impulse Coaster
	80,	// 57 Mini Roller Coaster
	80,	// 58 Mine Ride
	80,	// 59 LIM Launched Roller Coaster
};

/**
 * Data initially at 0x0097E3B4
 */
const uint8 rideUnknownData1[0x60] = {
	10,	// 00 Spiral Roller coaster
	10,	// 01 Stand Up Coaster
	20,	// 02 Suspended Swinging
	13,	// 03 Inverted
	8,	// 04 Steel Mini Coaster
	10,	// 05 Mini Railroad
	10,	// 06 Monorail
	10,	// 07 Mini Suspended Coaster
	4,	// 08 Boat Ride
	9,	// 09 Wooden Wild Mine/Mouse
	10,	// 0a Steeplechase/Motorbike/Soap Box Derby
	8,	// 0b Car Ride
	10,	// 0c Launched Freefall
	10,	// 0d Bobsleigh Coaster
	10,	// 0e Observation Tower
	10,	// 0f Looping Roller Coaster
	4,	// 10 Dinghy Slide
	10,	// 11 Mine Train Coaster
	4,	// 12 Chairlift
	11,	// 13 Corkscrew Roller Coaster
	0,	// 14 Maze
	0,	// 15 Spiral Slide
	8,	// 16 Go Karts
	9,	// 17 Log Flume
	10,	// 18 River Rapids
	5,	// 19 Bumper Cars
	0,	// 1a Pirate Ship
	0,	// 1b Swinging Inverter Ship
	0,	// 1c Food Stall
	0,	// 1d (none)
	0,	// 1e Drink Stall
	0,	// 1f (none)
	0,	// 20 Shop (all types)
	0,	// 21 Merry Go Round
	0,	// 22 Balloon Stall (maybe)
	0,	// 23 Information Kiosk
	0,	// 24 Bathroom
	0,	// 25 Ferris Wheel
	0,	// 26 Motion Simulator
	0,	// 27 3D Cinema
	0,	// 28 Topspin
	0,	// 29 Space Rings
	0,	// 2a Reverse Freefall Coaster
	10,	// 2b Elevator
	11,	// 2c Vertical Drop Roller Coaster
	0,	// 2d ATM
	0,	// 2e Twist
	0,	// 2f Haunted House
	0,	// 30 First Aid
	0,	// 31 Circus Show
	8,	// 32 Ghost Train
	11,	// 33 Twister Roller Coaster
	10,	// 34 Wooden Roller Coaster
	10,	// 35 Side-Friction Roller Coaster
	9,	// 36 Wild Mouse
	11,	// 37 Multi Dimension Coaster
	11,	// 38 (none)
	11,	// 39 Flying Roller Coaster
	11,	// 3a (none)
	10,	// 3b Virginia Reel
	9,	// 3c Splash Boats
	8,	// 3d Mini Helicopters
	11,	// 3e Lay-down Roller Coaster
	10,	// 3f Suspended Monorail
	11,	// 40 (none)
	10,	// 41 Reverser Roller Coaster
	11,	// 42 Heartline Twister Roller Coaster
	11,	// 43 Mini Golf
	12,	// 44 Giga Coaster
	10,	// 45 Roto-Drop
	5,	// 46 Flying Saucers
	0,	// 47 Crooked House
	8,	// 48 Monorail Cycles
	13,	// 49 Compact Inverted Coaster
	8,	// 4a Water Coaster
	0,	// 4b Air Powered Vertical Coaster
	9,	// 4c Inverted Hairpin Coaster
	0,	// 4d Magic Carpet
	4,	// 4e Submarine Ride
	9,	// 4f River Rafts
	0,	// 50 (none)
	0,	// 51 Enterprise
	0,	// 52 (none)
	0,	// 53 (none)
	0,	// 54 (none)
	11,	// 55 (none)
	11,	// 56 Inverted Impulse Coaster
	8,	// 57 Mini Roller Coaster
	10,	// 58 Mine Ride
	9,	// 59 LIM Launched Roller Coaster
};

/**
 * Data at 0x0097E3B6, originally set to either be 3 or 0 and replaced here by
 * a boolean table. This may be exactly the same as hasRunningTrack above.
 */
const bool rideUnknownData2[0x60] = {
	true,	// 00 Spiral Roller coaster
	true,	// 01 Stand Up Coaster
	true,	// 02 Suspended Swinging
	true,	// 03 Inverted
	true,	// 04 Steel Mini Coaster
	true,	// 05 Mini Railroad
	true,	// 06 Monorail
	true,	// 07 Mini Suspended Coaster
	false,	// 08 Boat Ride
	true,	// 09 Wooden Wild Mine/Mouse
	true,	// 0a Steeplechase/Motorbike/Soap Box Derby
	true,	// 0b Car Ride
	false,	// 0c Launched Freefall
	true,	// 0d Bobsleigh Coaster
	false,	// 0e Observation Tower
	true,	// 0f Looping Roller Coaster
	true,	// 10 Dinghy Slide
	true,	// 11 Mine Train Coaster
	true,	// 12 Chairlift
	true,	// 13 Corkscrew Roller Coaster
	false,	// 14 Maze
	false,	// 15 Spiral Slide
	false,	// 16 Go Karts
	false,	// 17 Log Flume
	false,	// 18 River Rapids
	false,	// 19 Bumper Cars
	false,	// 1a Pirate Ship
	false,	// 1b Swinging Inverter Ship
	false,	// 1c Food Stall
	false,	// 1d (none)
	false,	// 1e Drink Stall
	false,	// 1f (none)
	false,	// 20 Shop (all types)
	false,	// 21 Merry Go Round
	false,	// 22 Balloon Stall (maybe)
	false,	// 23 Information Kiosk
	false,	// 24 Bathroom
	false,	// 25 Ferris Wheel
	false,	// 26 Motion Simulator
	false,	// 27 3D Cinema
	false,	// 28 Topspin
	false,	// 29 Space Rings
	false,	// 2a Reverse Freefall Coaster
	false,	// 2b Elevator
	true,	// 2c Vertical Drop Roller Coaster
	false,	// 2d ATM
	false,	// 2e Twist
	false,	// 2f Haunted House
	false,	// 30 First Aid
	false,	// 31 Circus Show
	true,	// 32 Ghost Train
	true,	// 33 Twister Roller Coaster
	true,	// 34 Wooden Roller Coaster
	true,	// 35 Side-Friction Roller Coaster
	true,	// 36 Wild Mouse
	true,	// 37 Multi Dimension Coaster
	true,	// 38 (none)
	true,	// 39 Flying Roller Coaster
	true,	// 3a (none)
	true,	// 3b Virginia Reel
	false,	// 3c Splash Boats
	true,	// 3d Mini Helicopters
	true,	// 3e Lay-down Roller Coaster
	true,	// 3f Suspended Monorail
	true,	// 40 (none)
	true,	// 41 Reverser Roller Coaster
	true,	// 42 Heartline Twister Roller Coaster
	true,	// 43 Mini Golf
	true,	// 44 Giga Coaster
	false,	// 45 Roto-Drop
	false,	// 46 Flying Saucers
	false,	// 47 Crooked House
	true,	// 48 Monorail Cycles
	true,	// 49 Compact Inverted Coaster
	true,	// 4a Water Coaster
	false,	// 4b Air Powered Vertical Coaster
	true,	// 4c Inverted Hairpin Coaster
	false,	// 4d Magic Carpet
	false,	// 4e Submarine Ride
	false,	// 4f River Rafts
	false,	// 50 (none)
	false,	// 51 Enterprise
	false,	// 52 (none)
	false,	// 53 (none)
	false,	// 54 (none)
	true,	// 55 (none)
	true,	// 56 Inverted Impulse Coaster
	true,	// 57 Mini Roller Coaster
	true,	// 58 Mine Ride
	true,	// 59 LIM Launched Roller Coaster
};

/* Data at 0x0097E3B8 */
const uint8 rideUnknownData3[0x60] = {
	10,	// 00 Spiral Roller coaster
	10,	// 01 Stand Up Coaster
	10,	// 02 Suspended Swinging
	10,	// 03 Inverted
	5,	// 04 Steel Mini Coaster
	5,	// 05 Mini Railroad
	10,	// 06 Monorail
	10,	// 07 Mini Suspended Coaster
	0,	// 08 Boat Ride
	10,	// 09 Wooden Wild Mine/Mouse
	10,	// 0a Steeplechase/Motorbike/Soap Box Derby
	5,	// 0b Car Ride
	0,	// 0c Launched Freefall
	10,	// 0d Bobsleigh Coaster
	0,	// 0e Observation Tower
	10,	// 0f Looping Roller Coaster
	10,	// 10 Dinghy Slide
	10,	// 11 Mine Train Coaster
	10,	// 12 Chairlift
	10,	// 13 Corkscrew Roller Coaster
	0,	// 14 Maze
	0,	// 15 Spiral Slide
	0,	// 16 Go Karts
	10,	// 17 Log Flume
	10,	// 18 River Rapids
	0,	// 19 Bumper Cars
	0,	// 1a Pirate Ship
	0,	// 1b Swinging Inverter Ship
	0,	// 1c Food Stall
	0,	// 1d (none)
	0,	// 1e Drink Stall
	0,	// 1f (none)
	0,	// 20 Shop (all types)
	0,	// 21 Merry Go Round
	0,	// 22 Balloon Stall (maybe)
	0,	// 23 Information Kiosk
	0,	// 24 Bathroom
	0,	// 25 Ferris Wheel
	0,	// 26 Motion Simulator
	0,	// 27 3D Cinema
	0,	// 28 Topspin
	0,	// 29 Space Rings
	10,	// 2a Reverse Freefall Coaster
	0,	// 2b Elevator
	10,	// 2c Vertical Drop Roller Coaster
	0,	// 2d ATM
	0,	// 2e Twist
	0,	// 2f Haunted House
	0,	// 30 First Aid
	0,	// 31 Circus Show
	5,	// 32 Ghost Train
	10,	// 33 Twister Roller Coaster
	10,	// 34 Wooden Roller Coaster
	10,	// 35 Side-Friction Roller Coaster
	10,	// 36 Wild Mouse
	15,	// 37 Multi Dimension Coaster
	15,	// 38 (none)
	15,	// 39 Flying Roller Coaster
	15,	// 3a (none)
	10,	// 3b Virginia Reel
	10,	// 3c Splash Boats
	5,	// 3d Mini Helicopters
	15,	// 3e Lay-down Roller Coaster
	10,	// 3f Suspended Monorail
	15,	// 40 (none)
	10,	// 41 Reverser Roller Coaster
	10,	// 42 Heartline Twister Roller Coaster
	10,	// 43 Mini Golf
	40,	// 44 Giga Coaster
	0,	// 45 Roto-Drop
	0,	// 46 Flying Saucers
	0,	// 47 Crooked House
	5,	// 48 Monorail Cycles
	10,	// 49 Compact Inverted Coaster
	10,	// 4a Water Coaster
	10,	// 4b Air Powered Vertical Coaster
	10,	// 4c Inverted Hairpin Coaster
	0,	// 4d Magic Carpet
	0,	// 4e Submarine Ride
	10,	// 4f River Rafts
	0,	// 50 (none)
	0,	// 51 Enterprise
	0,	// 52 (none)
	0,	// 53 (none)
	0,	// 54 (none)
	10,	// 55 (none)
	10,	// 56 Inverted Impulse Coaster
	10,	// 57 Mini Roller Coaster
	10,	// 58 Mine Ride
	10,	// 59 LIM Launched Roller Coaster
};

const rct_ride_name_convention RideNameConvention[96] = {
	{ 1229,    1243,    1257,       0 }, // 00 Spiral Roller coaster
	{ 1229,    1243,    1257,       0 }, // 01 Stand Up Coaster
	{ 1229,    1243,    1257,       0 }, // 02 Suspended Swinging
	{ 1229,    1243,    1257,       0 }, // 03 Inverted
	{ 1229,    1243,    1257,       0 }, // 04 Steel Mini Coaster
	{ 1229,    1243,    1257,       0 }, // 05 Mini Railroad
	{ 1229,    1243,    1257,       0 }, // 06 Monorail
	{ 1264,    1243,    1257,       0 }, // 07 Mini Suspended Coaster
	{ 1236,    1250,    1250,       0 }, // 08 Boat Ride
	{ 1264,    1243,    1257,       0 }, // 09 Wooden Wild Mine/Mouse
	{ 1264,    1243,    1257,       0 }, // 0a Steeplechase/Motorbike/Soap Box Derby
	{ 1264,    1243,    1257,       0 }, // 0b Car Ride
	{ 1264,    1243,    1257,       0 }, // 0c Launched Freefall
	{ 1229,    1243,    1257,       0 }, // 0d Bobsleigh Coaster
	{ 1292,    1243,    1257,       0 }, // 0e Observation Tower
	{ 1229,    1243,    1257,       0 }, // 0f Looping Roller Coaster
	{ 1236,    1243,    1257,       0 }, // 10 Dinghy Slide
	{ 1229,    1243,    1257,       0 }, // 11 Mine Train Coaster
	{ 1264,    1243,    1257,       0 }, // 12 Chairlift
	{ 1229,    1243,    1257,       0 }, // 13 Corkscrew Roller Coaster
	{ 1229,    1243,    1257,       0 }, // 14 Maze
	{ 1229,    1271,    1257,       0 }, // 15 Spiral Slide
	{ 1264,    1243,    1257,       0 }, // 16 Go Karts
	{ 1236,    1243,    1257,       0 }, // 17 Log Flume
	{ 1236,    1243,    1257,       0 }, // 18 River Rapids
	{ 1264,    1271,    1257,       0 }, // 19 Bumper Cars
	{ 1285,    1278,    1257,       0 }, // 1a Pirate Ship
	{ 1285,    1278,    1257,       0 }, // 1b Swinging Inverter Ship
	{ 1264,    1271,    1257,       0 }, // 1c Food Stall
	{ 1264,    1271,    1257,       0 }, // 1d (none)
	{ 1264,    1271,    1257,       0 }, // 1e Drink Stall
	{ 1264,    1271,    1257,       0 }, // 1f (none)
	{ 1264,    1271,    1257,       0 }, // 20 Shop (all types)
	{ 1264,    1278,    1257,       0 }, // 21 Merry Go Round
	{ 1264,    1271,    1257,       0 }, // 22 Balloon Stall (maybe)
	{ 1264,    1271,    1257,       0 }, // 23 Information Kiosk
	{ 1264,    1271,    1257,       0 }, // 24 Bathroom
	{ 1299,    1278,    1257,       0 }, // 25 Ferris Wheel
	{ 1264,    1278,    1257,       0 }, // 26 Motion Simulator
	{ 1271,    1278,    1257,       0 }, // 27 3D Cinema
	{ 1264,    1278,    1257,       0 }, // 28 Topspin
	{ 1306,    1278,    1257,       0 }, // 29 Space Rings
	{ 1264,    1243,    1257,       0 }, // 2a Reverse Freefall Coaster
	{ 1292,    1243,    1257,       0 }, // 2b Elevator
	{ 1229,    1243,    1257,       0 }, // 2c Vertical Drop Roller Coaster
	{ 1264,    1271,    1257,       0 }, // 2d ATM
	{ 1278,    1278,    1257,       0 }, // 2e Twist
	{ 1271,    1278,    1257,       0 }, // 2f Haunted House
	{ 1264,    1271,    1257,       0 }, // 30 First Aid
	{ 1271,    1278,    1257,       0 }, // 31 Circus Show
	{ 1264,    1243,    1257,       0 }, // 32 Ghost Train
	{ 1229,    1243,    1257,       0 }, // 33 Twister Roller Coaster
	{ 1229,    1243,    1257,       0 }, // 34 Wooden Roller Coaster
	{ 1229,    1243,    1257,       0 }, // 35 Side-Friction Roller Coaster
	{ 1264,    1243,    1257,       0 }, // 36 Wild Mouse
	{ 1229,    1243,    1257,       0 }, // 37 Multi Dimension Coaster
	{ 1229,    1243,    1257,       0 }, // 38 (none)
	{ 1229,    1243,    1257,       0 }, // 39 Flying Roller Coaster
	{ 1229,    1243,    1257,       0 }, // 3a (none)
	{ 1264,    1243,    1257,       0 }, // 3b Virginia Reel
	{ 1236,    1243,    1257,       0 }, // 3c Splash Boats
	{ 1264,    1243,    1257,       0 }, // 3d Mini Helicopters
	{ 1229,    1243,    1257,       0 }, // 3e Lay-down Roller Coaster
	{ 1229,    1243,    1257,       0 }, // 3f Suspended Monorail
	{ 1229,    1243,    1257,       0 }, // 40 (none)
	{ 1264,    1243,    1257,       0 }, // 41 Reverser Roller Coaster
	{ 1264,    1243,    1257,       0 }, // 42 Heartline Twister Roller Coaster
	{ 1313,    1320,    1257,       0 }, // 43 Mini Golf
	{ 1229,    1243,    1257,       0 }, // 44 Giga Coaster
	{ 1264,    1243,    1257,       0 }, // 45 Roto-Drop
	{ 1264,    1271,    1257,       0 }, // 46 Flying Saucers
	{ 1271,    1278,    1257,       0 }, // 47 Crooked House
	{ 1264,    1243,    1257,       0 }, // 48 Monorail Cycles
	{ 1229,    1243,    1257,       0 }, // 49 Compact Inverted Coaster
	{ 1236,    1243,    1257,       0 }, // 4a Water Coaster
	{ 1229,    1243,    1257,       0 }, // 4b Air Powered Vertical Coaster
	{ 1264,    1243,    1257,       0 }, // 4c Inverted Hairpin Coaster
	{ 1264,    1278,    1257,       0 }, // 4d Magic Carpet
	{ 1236,    1243,    1250,       0 }, // 4e Submarine Ride
	{ 1236,    1243,    1257,       0 }, // 4f River Rafts
	{ 1264,    1271,    1257,       0 }, // 50 (none)
	{ 1299,    1278,    1257,       0 }, // 51 Enterprise
	{ 1264,    1271,    1257,       0 }, // 52 (none)
	{ 1264,    1271,    1257,       0 }, // 53 (none)
	{ 1264,    1271,    1257,       0 }, // 54 (none)
	{ 1229,    1243,    1257,       0 }, // 55 (none)
	{ 1229,    1243,    1257,       0 }, // 56 Inverted Impulse Coaster
	{ 1264,    1243,    1257,       0 }, // 57 Mini Roller Coaster
	{ 1229,    1243,    1257,       0 }, // 58 Mine Ride
	{ 1264,    1243,    1257,       0 }, // 59 LIM Launched Roller Coaster
	{ 1229,    1243,    1257,       0 }
};

/* rct2: 0x0097C8AC */
const uint8 RideAvailableModes[] = {
	RIDE_MODE_CONTINUOUS_CIRCUIT, RIDE_MODE_CONTINUOUS_CIRCUIT_BLOCK_SECTIONED, 0xFF,																		// 00 Spiral Roller coaster
	RIDE_MODE_CONTINUOUS_CIRCUIT, RIDE_MODE_CONTINUOUS_CIRCUIT_BLOCK_SECTIONED, 0xFF,																		// 01 Stand Up Coaster
	RIDE_MODE_CONTINUOUS_CIRCUIT, RIDE_MODE_CONTINUOUS_CIRCUIT_BLOCK_SECTIONED, 0xFF,																		// 02 Suspended Swinging
	RIDE_MODE_CONTINUOUS_CIRCUIT, RIDE_MODE_CONTINUOUS_CIRCUIT_BLOCK_SECTIONED, 0xFF,																		// 03 Inverted
	RIDE_MODE_CONTINUOUS_CIRCUIT, RIDE_MODE_CONTINUOUS_CIRCUIT_BLOCK_SECTIONED, RIDE_MODE_REVERSE_INCLINE_LAUNCHED_SHUTTLE, 0xFF,							// 04 Steel Mini Coaster
	RIDE_MODE_CONTINUOUS_CIRCUIT, RIDE_MODE_SHUTTLE, 0xFF,																									// 05 Mini Railroad
	RIDE_MODE_CONTINUOUS_CIRCUIT, RIDE_MODE_SHUTTLE, 0xFF,																									// 06 Monorail
	RIDE_MODE_CONTINUOUS_CIRCUIT, 0xFF,																														// 07 Mini Suspended Coaster
	RIDE_MODE_BOAT_HIRE, 0xFF,																																// 08 Boat Ride
	RIDE_MODE_CONTINUOUS_CIRCUIT, 0xFF,																														// 09 Wooden Wild Mine/Mouse
	RIDE_MODE_CONTINUOUS_CIRCUIT, RIDE_MODE_CONTINUOUS_CIRCUIT_BLOCK_SECTIONED, 0xFF,																		// 0A Steeplechase/Motorbike/Soap Box Derby
	RIDE_MODE_CONTINUOUS_CIRCUIT, 0xFF,																														// 0B Car Ride
	RIDE_MODE_UPWARD_LAUNCH, RIDE_MODE_DOWNWARD_LAUNCH, 0xFF,																								// 0C Launched Freefall
	RIDE_MODE_CONTINUOUS_CIRCUIT, RIDE_MODE_CONTINUOUS_CIRCUIT_BLOCK_SECTIONED, 0xFF,																		// 0D Bobsleigh Coaster
	RIDE_MODE_ROTATING_LIFT, 0xFF,																															// 0E Observation Tower
	RIDE_MODE_CONTINUOUS_CIRCUIT, RIDE_MODE_CONTINUOUS_CIRCUIT_BLOCK_SECTIONED, RIDE_MODE_REVERSE_INCLINE_LAUNCHED_SHUTTLE, RIDE_MODE_POWERED_LAUNCH_PASSTROUGH, RIDE_MODE_POWERED_LAUNCH, 0xFF,	// 0F Looping Roller Coaster
	RIDE_MODE_CONTINUOUS_CIRCUIT, 0xFF,																														// 10 Dinghy Slide
	RIDE_MODE_CONTINUOUS_CIRCUIT, RIDE_MODE_CONTINUOUS_CIRCUIT_BLOCK_SECTIONED, 0xFF,																		// 11 Mine Train Coaster
	RIDE_MODE_STATION_TO_STATION, 0xFF,																														// 12 Chairlift
	RIDE_MODE_CONTINUOUS_CIRCUIT, RIDE_MODE_CONTINUOUS_CIRCUIT_BLOCK_SECTIONED, RIDE_MODE_POWERED_LAUNCH_PASSTROUGH, RIDE_MODE_POWERED_LAUNCH, RIDE_MODE_REVERSE_INCLINE_LAUNCHED_SHUTTLE, 0xFF,																// 13 Corkscrew Roller Coaster
	RIDE_MODE_MAZE, 0xFF,																																	// 14 Maze
	RIDE_MODE_SINGLE_RIDE_PER_ADMISSION, RIDE_MODE_UNLIMITED_RIDES_PER_ADMISSION, 0xFF,																		// 15 Spiral Slide
	RIDE_MODE_RACE, RIDE_MODE_CONTINUOUS_CIRCUIT, 0xFF,																										// 16 Go Karts
	RIDE_MODE_CONTINUOUS_CIRCUIT, 0xFF,																														// 17 Log Flume
	RIDE_MODE_CONTINUOUS_CIRCUIT, 0xFF,																														// 18 River Rapids
	RIDE_MODE_BUMPERCAR, 0xFF,																																// 19 Bumper Cars
	RIDE_MODE_SWING, 0xFF,																																	// 1A Pirate Ship
	RIDE_MODE_SWING, 0xFF,																																	// 1B Swinging Inverter Ship
	RIDE_MODE_SHOP_STALL, 0xFF,																																// 1C Food Stall
	RIDE_MODE_SHOP_STALL, 0xFF,																																// 1D (none)
	RIDE_MODE_SHOP_STALL, 0xFF,																																// 1E Drink Stall
	RIDE_MODE_SHOP_STALL, 0xFF,																																// 1F (none)
	RIDE_MODE_SHOP_STALL, 0xFF,																																// 20 Shop (all types)
	RIDE_MODE_ROTATION, 0xFF,																																// 21 Merry Go Round
	RIDE_MODE_SHOP_STALL, 0xFF,																																// 22 Balloon Stall (maybe)
	RIDE_MODE_SHOP_STALL, 0xFF,																																// 23 Information Kiosk
	RIDE_MODE_SHOP_STALL, 0xFF,																																// 24 Bathroom
	RIDE_MODE_FORWARD_ROTATION, RIDE_MODE_BACKWARD_ROTATION, 0xFF,																							// 25 Ferris Wheel
	RIDE_MODE_FILM_AVENGING_AVIATORS, RIDE_MODE_FILM_THRILL_RIDERS, 0xFF,																					// 26 Motion Simulator
	RIDE_MODE_3D_FILM_MOUSE_TAILS, RIDE_MODE_3D_FILM_STORM_CHASERS, RIDE_MODE_3D_FILM_SPACE_RAIDERS, 0xFF,													// 27 3D Cinema
	RIDE_MODE_BEGINNERS, RIDE_MODE_INTENSE, RIDE_MODE_BERSERK, 0xFF,																						// 28 Topspin
	RIDE_MODE_SPACE_RINGS, 0xFF,																															// 29 Space Rings
	RIDE_MODE_LIM_POWERED_LAUNCH, 0xFF,																														// 2A Reverse Freefall Coaster
	RIDE_MODE_SHUTTLE, 0xFF,																																// 2B Elevator
	RIDE_MODE_CONTINUOUS_CIRCUIT, RIDE_MODE_CONTINUOUS_CIRCUIT_BLOCK_SECTIONED, 0xFF,																		// 2C Vertical Drop Roller Coaster
	RIDE_MODE_SHOP_STALL, 0xFF,																																// 2D ATM
	RIDE_MODE_ROTATION, 0xFF,																																// 2E Twist
	RIDE_MODE_HAUNTED_HOUSE, 0xFF,																															// 2F Haunted House
	RIDE_MODE_SHOP_STALL, 0xFF,																																// 30 First Aid
	RIDE_MODE_CIRCUS_SHOW, 0xFF,																															// 31 Circus Show
	RIDE_MODE_CONTINUOUS_CIRCUIT, 0xFF,																														// 32 Ghost Train
	RIDE_MODE_CONTINUOUS_CIRCUIT, RIDE_MODE_CONTINUOUS_CIRCUIT_BLOCK_SECTIONED, 0xFF,																		// 33 Twister Roller Coaster
	RIDE_MODE_CONTINUOUS_CIRCUIT, RIDE_MODE_CONTINUOUS_CIRCUIT_BLOCK_SECTIONED, 0xFF,																		// 34 Wooden Roller Coaster
	RIDE_MODE_CONTINUOUS_CIRCUIT, 0xFF,																														// 35 Side-Friction Roller Coaster
	RIDE_MODE_CONTINUOUS_CIRCUIT, RIDE_MODE_CONTINUOUS_CIRCUIT_BLOCK_SECTIONED, 0xFF,																		// 36 Wild Mouse
	RIDE_MODE_CONTINUOUS_CIRCUIT, RIDE_MODE_CONTINUOUS_CIRCUIT_BLOCK_SECTIONED, 0xFF,																		// 37 Multi Dimension Coaster
	RIDE_MODE_CONTINUOUS_CIRCUIT, RIDE_MODE_CONTINUOUS_CIRCUIT_BLOCK_SECTIONED, 0xFF,																		// 38 (none)
	RIDE_MODE_CONTINUOUS_CIRCUIT, RIDE_MODE_CONTINUOUS_CIRCUIT_BLOCK_SECTIONED, 0xFF,																		// 39 Flying Roller Coaster
	RIDE_MODE_CONTINUOUS_CIRCUIT, RIDE_MODE_CONTINUOUS_CIRCUIT_BLOCK_SECTIONED, 0xFF,																		// 3A (none)
	RIDE_MODE_CONTINUOUS_CIRCUIT, 0xFF,																														// 3B Virginia Reel
	RIDE_MODE_CONTINUOUS_CIRCUIT, 0xFF,																														// 3C Splash Boats
	RIDE_MODE_CONTINUOUS_CIRCUIT, 0xFF,																														// 3D Mini Helicopters
	RIDE_MODE_CONTINUOUS_CIRCUIT, RIDE_MODE_CONTINUOUS_CIRCUIT_BLOCK_SECTIONED, 0xFF,																		// 3E Lay-down Roller Coaster
	RIDE_MODE_CONTINUOUS_CIRCUIT, RIDE_MODE_SHUTTLE, 0xFF,																									// 3F Suspended Monorail
	RIDE_MODE_CONTINUOUS_CIRCUIT, RIDE_MODE_CONTINUOUS_CIRCUIT_BLOCK_SECTIONED, 0xFF,																		// 40 (none)
	RIDE_MODE_CONTINUOUS_CIRCUIT, 0xFF,																														// 41 Reverser Roller Coaster
	RIDE_MODE_CONTINUOUS_CIRCUIT, 0xFF,																														// 42 Heartline Twister Roller Coaster
	RIDE_MODE_CONTINUOUS_CIRCUIT, 0xFF,																														// 43 Mini Golf
	RIDE_MODE_CONTINUOUS_CIRCUIT, RIDE_MODE_CONTINUOUS_CIRCUIT_BLOCK_SECTIONED, 0xFF,																		// 44 Giga Coaster
	RIDE_MODE_FREEFALL_DROP, 0xFF,																															// 45 Roto-Drop
	RIDE_MODE_BUMPERCAR, 0xFF,																																// 46 Flying Saucers
	RIDE_MODE_CROOKED_HOUSE, 0xFF,																															// 47 Crooked House
	RIDE_MODE_CONTINUOUS_CIRCUIT, 0xFF,																														// 48 Monorail Cycles
	RIDE_MODE_CONTINUOUS_CIRCUIT, RIDE_MODE_CONTINUOUS_CIRCUIT_BLOCK_SECTIONED, RIDE_MODE_REVERSE_INCLINE_LAUNCHED_SHUTTLE, 0xFF,																	// 49 Compact Inverted Coaster
	RIDE_MODE_CONTINUOUS_CIRCUIT, RIDE_MODE_CONTINUOUS_CIRCUIT_BLOCK_SECTIONED, 0xFF,																		// 4A Water Coaster
	RIDE_MODE_POWERED_LAUNCH_PASSTROUGH, RIDE_MODE_POWERED_LAUNCH, 0xFF,																					// 4B Air Powered Vertical Coaster
	RIDE_MODE_CONTINUOUS_CIRCUIT, RIDE_MODE_CONTINUOUS_CIRCUIT_BLOCK_SECTIONED, 0xFF,																		// 4C Inverted Hairpin Coaster
	RIDE_MODE_SWING, 0xFF,																																	// 4D Magic Carpet
	RIDE_MODE_CONTINUOUS_CIRCUIT, 0xFF,																														// 4E Submarine Ride
	RIDE_MODE_CONTINUOUS_CIRCUIT, 0xFF,																														// 4F River Rafts
	RIDE_MODE_SHOP_STALL, 0xFF,																																// 50 (none)
	RIDE_MODE_ROTATION, 0xFF,																																// 51 Enterprise
	RIDE_MODE_SHOP_STALL, 0xFF,																																// 52 (none)
	RIDE_MODE_SHOP_STALL, 0xFF,																																// 53 (none)
	RIDE_MODE_SHOP_STALL, 0xFF,																																// 54 (none)
	RIDE_MODE_CONTINUOUS_CIRCUIT, 0xFF,																														// 55 (none)
	RIDE_MODE_POWERED_LAUNCH_PASSTROUGH, RIDE_MODE_POWERED_LAUNCH, 0xFF,																					// 56 Inverted Impulse Coaster
	RIDE_MODE_CONTINUOUS_CIRCUIT, RIDE_MODE_CONTINUOUS_CIRCUIT_BLOCK_SECTIONED, 0xFF,																		// 57 Mini Roller Coaster
	RIDE_MODE_CONTINUOUS_CIRCUIT, 0xFF,																														// 58 Mine Ride
	RIDE_MODE_CONTINUOUS_CIRCUIT, RIDE_MODE_CONTINUOUS_CIRCUIT_BLOCK_SECTIONED, 0xFF,																		// 59 Unknown
	RIDE_MODE_POWERED_LAUNCH_PASSTROUGH, RIDE_MODE_POWERED_LAUNCH, RIDE_MODE_POWERED_LAUNCH_BLOCK_SECTIONED, 0xFF											// 60 LIM Launched Roller Coaster
};

const uint8 AllRideModesAvailable[] = {
	RIDE_MODE_CONTINUOUS_CIRCUIT, RIDE_MODE_CONTINUOUS_CIRCUIT_BLOCK_SECTIONED, RIDE_MODE_REVERSE_INCLINE_LAUNCHED_SHUTTLE, RIDE_MODE_POWERED_LAUNCH_PASSTROUGH, RIDE_MODE_SHUTTLE, RIDE_MODE_NORMAL, RIDE_MODE_BOAT_HIRE, RIDE_MODE_UPWARD_LAUNCH, RIDE_MODE_ROTATING_LIFT, RIDE_MODE_STATION_TO_STATION, RIDE_MODE_SINGLE_RIDE_PER_ADMISSION, RIDE_MODE_UNLIMITED_RIDES_PER_ADMISSION, RIDE_MODE_MAZE, RIDE_MODE_RACE, RIDE_MODE_BUMPERCAR, RIDE_MODE_SWING, RIDE_MODE_SHOP_STALL, RIDE_MODE_ROTATION, RIDE_MODE_FORWARD_ROTATION, RIDE_MODE_BACKWARD_ROTATION, RIDE_MODE_FILM_AVENGING_AVIATORS, RIDE_MODE_3D_FILM_MOUSE_TAILS, RIDE_MODE_SPACE_RINGS, RIDE_MODE_BEGINNERS, RIDE_MODE_LIM_POWERED_LAUNCH, RIDE_MODE_FILM_THRILL_RIDERS, RIDE_MODE_3D_FILM_STORM_CHASERS, RIDE_MODE_3D_FILM_SPACE_RAIDERS, RIDE_MODE_INTENSE, RIDE_MODE_BERSERK, RIDE_MODE_HAUNTED_HOUSE, RIDE_MODE_CIRCUS_SHOW, RIDE_MODE_DOWNWARD_LAUNCH, RIDE_MODE_CROOKED_HOUSE, RIDE_MODE_FREEFALL_DROP, RIDE_MODE_POWERED_LAUNCH, RIDE_MODE_POWERED_LAUNCH_BLOCK_SECTIONED, 0xFF
};

const uint8 RideAvailableBreakdowns[] = {
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION) | (1 << BREAKDOWN_BRAKES_FAILURE),																						// 00 Spiral Roller coaster
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_RESTRAINTS_STUCK_CLOSED) | (1 << BREAKDOWN_RESTRAINTS_STUCK_OPEN) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION) | (1 << BREAKDOWN_BRAKES_FAILURE),	// 01 Stand Up Coaster
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_RESTRAINTS_STUCK_CLOSED) | (1 << BREAKDOWN_RESTRAINTS_STUCK_OPEN) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION) | (1 << BREAKDOWN_BRAKES_FAILURE),	// 02 Suspended Swinging
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_RESTRAINTS_STUCK_CLOSED) | (1 << BREAKDOWN_RESTRAINTS_STUCK_OPEN) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION) | (1 << BREAKDOWN_BRAKES_FAILURE),	// 03 Inverted
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_RESTRAINTS_STUCK_CLOSED) | (1 << BREAKDOWN_RESTRAINTS_STUCK_OPEN) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION) | (1 << BREAKDOWN_BRAKES_FAILURE),	// 04 Steel Mini Coaster
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION),																															// 05 Mini Railroad
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_DOORS_STUCK_CLOSED) | (1 << BREAKDOWN_DOORS_STUCK_OPEN) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION),												// 06 Monorail
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_RESTRAINTS_STUCK_CLOSED) | (1 << BREAKDOWN_RESTRAINTS_STUCK_OPEN) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION) | (1 << BREAKDOWN_BRAKES_FAILURE),	// 07 Mini Suspended Coaster
	(1 << BREAKDOWN_VEHICLE_MALFUNCTION),																																							// 08 Boat Ride
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION) | (1 << BREAKDOWN_BRAKES_FAILURE),																						// 09 Wooden Wild Mine/Mouse
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION) | (1 << BREAKDOWN_BRAKES_FAILURE),																						// 0A Steeplechase/Motorbike/Soap Box Derby
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION),																															// 0B Car Ride
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_RESTRAINTS_STUCK_CLOSED) | (1 << BREAKDOWN_RESTRAINTS_STUCK_OPEN) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION),										// 0C Launched Freefall
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION) | (1 << BREAKDOWN_BRAKES_FAILURE),																						// 0D Bobsleigh Coaster
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION),																															// 0E Observation Tower
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_RESTRAINTS_STUCK_CLOSED) | (1 << BREAKDOWN_RESTRAINTS_STUCK_OPEN) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION) | (1 << BREAKDOWN_BRAKES_FAILURE),	// 0F Looping Roller Coaster
	(1 << BREAKDOWN_SAFETY_CUT_OUT),																																								// 10 Dinghy Slide
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_RESTRAINTS_STUCK_CLOSED) | (1 << BREAKDOWN_RESTRAINTS_STUCK_OPEN) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION) | (1 << BREAKDOWN_BRAKES_FAILURE),	// 11 Mine Train Coaster
	(1 << BREAKDOWN_SAFETY_CUT_OUT),																																								// 12 Chairlift
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_RESTRAINTS_STUCK_CLOSED) | (1 << BREAKDOWN_RESTRAINTS_STUCK_OPEN) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION) | (1 << BREAKDOWN_BRAKES_FAILURE),	// 13 Corkscrew Roller Coaster
	0,																																																// 14 Maze
	(1 << BREAKDOWN_SAFETY_CUT_OUT),																																								// 15 Spiral Slide
	(1 << BREAKDOWN_VEHICLE_MALFUNCTION),																																							// 16 Go Karts
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_CONTROL_FAILURE),																																// 17 Log Flume
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_CONTROL_FAILURE),																																// 18 River Rapids
	(1 << BREAKDOWN_SAFETY_CUT_OUT),																																								// 19 Bumper Cars
	(1 << BREAKDOWN_SAFETY_CUT_OUT),																																								// 1A Pirate Ship
	(1 << BREAKDOWN_SAFETY_CUT_OUT),																																								// 1B Swinging Inverter Ship
	0,																																																// 1C Food Stall
	0,																																																// 1D (none)
	0,																																																// 1E Drink Stall
	0,																																																// 1F (none)
	0,																																																// 20 Shop (all types)
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_CONTROL_FAILURE),																																// 21 Merry Go Round
	0,																																																// 22 Balloon Stall (maybe)
	0,																																																// 23 Information Kiosk
	0,																																																// 24 Bathroom
	(1 << BREAKDOWN_SAFETY_CUT_OUT),																																								// 25 Ferris Wheel
	(1 << BREAKDOWN_SAFETY_CUT_OUT),																																								// 26 Motion Simulator
	(1 << BREAKDOWN_SAFETY_CUT_OUT),																																								// 27 3D Cinema
	(1 << BREAKDOWN_SAFETY_CUT_OUT),																																								// 28 Topspin
	(1 << BREAKDOWN_SAFETY_CUT_OUT),																																								// 29 Space Rings
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_RESTRAINTS_STUCK_CLOSED) | (1 << BREAKDOWN_RESTRAINTS_STUCK_OPEN) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION),										// 2A Reverse Freefall Coaster
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_DOORS_STUCK_CLOSED) | (1 << BREAKDOWN_DOORS_STUCK_OPEN),																						// 2B Elevator
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_RESTRAINTS_STUCK_CLOSED) | (1 << BREAKDOWN_RESTRAINTS_STUCK_OPEN) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION) | (1 << BREAKDOWN_BRAKES_FAILURE),	// 2C Vertical Drop Roller Coaster
	0,																																																// 2D ATM
	(1 << BREAKDOWN_SAFETY_CUT_OUT),																																								// 2E Twist
	(1 << BREAKDOWN_SAFETY_CUT_OUT),																																								// 2F Haunted House
	0,																																																// 30 First Aid
	(1 << BREAKDOWN_SAFETY_CUT_OUT),																																								// 31 Circus Show
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION),																															// 32 Ghost Train
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_RESTRAINTS_STUCK_CLOSED) | (1 << BREAKDOWN_RESTRAINTS_STUCK_OPEN) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION) | (1 << BREAKDOWN_BRAKES_FAILURE),	// 33 Twister Roller Coaster
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_RESTRAINTS_STUCK_CLOSED) | (1 << BREAKDOWN_RESTRAINTS_STUCK_OPEN) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION) | (1 << BREAKDOWN_BRAKES_FAILURE),	// 34 Wooden Roller Coaster
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION) | (1 << BREAKDOWN_BRAKES_FAILURE),																						// 35 Side-Friction Roller Coaster
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_RESTRAINTS_STUCK_CLOSED) | (1 << BREAKDOWN_RESTRAINTS_STUCK_OPEN) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION) | (1 << BREAKDOWN_BRAKES_FAILURE),	// 36 Wild Mouse
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_RESTRAINTS_STUCK_CLOSED) | (1 << BREAKDOWN_RESTRAINTS_STUCK_OPEN) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION) | (1 << BREAKDOWN_BRAKES_FAILURE),	// 37 Multi Dimension Coaster
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_RESTRAINTS_STUCK_CLOSED) | (1 << BREAKDOWN_RESTRAINTS_STUCK_OPEN) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION) | (1 << BREAKDOWN_BRAKES_FAILURE),	// 38 (none)
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_RESTRAINTS_STUCK_CLOSED) | (1 << BREAKDOWN_RESTRAINTS_STUCK_OPEN) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION) | (1 << BREAKDOWN_BRAKES_FAILURE),	// 39 Flying Roller Coaster
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_RESTRAINTS_STUCK_CLOSED) | (1 << BREAKDOWN_RESTRAINTS_STUCK_OPEN) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION) | (1 << BREAKDOWN_BRAKES_FAILURE),	// 3A (none)
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION) | (1 << BREAKDOWN_BRAKES_FAILURE),																						// 3B Virginia Reel
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_CONTROL_FAILURE),																																// 3C Splash Boats
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION),																															// 3D Mini Helicopters
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_RESTRAINTS_STUCK_CLOSED) | (1 << BREAKDOWN_RESTRAINTS_STUCK_OPEN) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION) | (1 << BREAKDOWN_BRAKES_FAILURE),	// 3E Lay-down Roller Coaster
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_DOORS_STUCK_CLOSED) | (1 << BREAKDOWN_DOORS_STUCK_OPEN) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION),												// 3F Suspended Monorail
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_RESTRAINTS_STUCK_CLOSED) | (1 << BREAKDOWN_RESTRAINTS_STUCK_OPEN) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION) | (1 << BREAKDOWN_BRAKES_FAILURE),	// 40 (none)
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION) | (1 << BREAKDOWN_BRAKES_FAILURE),																						// 41 Reverser Roller Coaster
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_RESTRAINTS_STUCK_CLOSED) | (1 << BREAKDOWN_RESTRAINTS_STUCK_OPEN) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION),										// 42 Heartline Twister Roller Coaster
	0,																																																// 43 Mini Golf
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_RESTRAINTS_STUCK_CLOSED) | (1 << BREAKDOWN_RESTRAINTS_STUCK_OPEN) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION) | (1 << BREAKDOWN_BRAKES_FAILURE),	// 44 Giga Coaster
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_RESTRAINTS_STUCK_CLOSED) | (1 << BREAKDOWN_RESTRAINTS_STUCK_OPEN) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION),										// 45 Roto-Drop
	(1 << BREAKDOWN_SAFETY_CUT_OUT),																																								// 46 Flying Saucers
	0,																																																// 47 Crooked House
	(1 << BREAKDOWN_VEHICLE_MALFUNCTION),																																							// 48 Monorail Cycles
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_RESTRAINTS_STUCK_CLOSED) | (1 << BREAKDOWN_RESTRAINTS_STUCK_OPEN) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION) | (1 << BREAKDOWN_BRAKES_FAILURE),	// 49 Compact Inverted Coaster
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_RESTRAINTS_STUCK_CLOSED) | (1 << BREAKDOWN_RESTRAINTS_STUCK_OPEN) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION) | (1 << BREAKDOWN_BRAKES_FAILURE),	// 4A Water Coaster
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_RESTRAINTS_STUCK_CLOSED) | (1 << BREAKDOWN_RESTRAINTS_STUCK_OPEN) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION),										// 4B Air Powered Vertical Coaster
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_RESTRAINTS_STUCK_CLOSED) | (1 << BREAKDOWN_RESTRAINTS_STUCK_OPEN) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION) | (1 << BREAKDOWN_BRAKES_FAILURE),	// 4C Inverted Hairpin Coaster
	(1 << BREAKDOWN_SAFETY_CUT_OUT),																																								// 4D Magic Carpet
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION),																															// 4E Submarine Ride
	(1 << BREAKDOWN_SAFETY_CUT_OUT),																																								// 4F River Rafts
	0,																																																// 50 (none)
	(1 << BREAKDOWN_SAFETY_CUT_OUT),																																								// 51 Enterprise
	0,																																																// 52 (none)
	0,																																																// 53 (none)
	0,																																																// 54 (none)
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_RESTRAINTS_STUCK_CLOSED) | (1 << BREAKDOWN_RESTRAINTS_STUCK_OPEN) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION) | (1 << BREAKDOWN_BRAKES_FAILURE),	// 55 (none)
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_RESTRAINTS_STUCK_CLOSED) | (1 << BREAKDOWN_RESTRAINTS_STUCK_OPEN) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION),										// 56 Inverted Impulse Coaster
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_RESTRAINTS_STUCK_CLOSED) | (1 << BREAKDOWN_RESTRAINTS_STUCK_OPEN) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION) | (1 << BREAKDOWN_BRAKES_FAILURE),	// 57 Mini Roller Coaster
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_RESTRAINTS_STUCK_CLOSED) | (1 << BREAKDOWN_RESTRAINTS_STUCK_OPEN) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION),										// 58 Mine Ride
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_RESTRAINTS_STUCK_CLOSED) | (1 << BREAKDOWN_RESTRAINTS_STUCK_OPEN) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION) | (1 << BREAKDOWN_BRAKES_FAILURE),	// 59 Unknown
	(1 << BREAKDOWN_SAFETY_CUT_OUT) | (1 << BREAKDOWN_RESTRAINTS_STUCK_CLOSED) | (1 << BREAKDOWN_RESTRAINTS_STUCK_OPEN) | (1 << BREAKDOWN_VEHICLE_MALFUNCTION) | (1 << BREAKDOWN_BRAKES_FAILURE)	// 60 LIM Launched Roller Coaster
};

// rct2: 0x00993E7C and 0x00993E1C
const rct_ride_entrance_definition RideEntranceDefinitions[RIDE_ENTRANCE_STYLE_COUNT] = {
	{ 22664,	32,		2,		STR_PLAIN_ENTRANCE,				0x00000000,			0,	},		// RIDE_ENTRANCE_STYLE_PLAIN
	{ 22760,	31,		21,		STR_WOODEN_ENTRANCE,			0x000057A1,			0,	},		// RIDE_ENTRANCE_STYLE_WOODEN
	{ 22680,	43,		2,		STR_CANVAS_TENT_ENTRANCE,		0x800057AD,			3,	},		// RIDE_ENTRANCE_STYLE_CANVAS_TENT
	{ 22728,	43,		19,		STR_CASTLE_ENTRANCE_GREY,		0x000057B9,			0,	},		// RIDE_ENTRANCE_STYLE_CASTLE_GREY
	{ 22712,	43,		19,		STR_CASTLE_ENTRANCE_BROWN,		0x000057C5,			0,	},		// RIDE_ENTRANCE_STYLE_CASTLE_BROWN
	{ 22776,	33,		19,		STR_JUNGLE_ENTRANCE,			0x000057D1,			0,	},		// RIDE_ENTRANCE_STYLE_JUNGLE
	{ 22744,	32,		20,		STR_LOG_CABIN_ENTRANCE,			0x000057DD,			0,	},		// RIDE_ENTRANCE_STYLE_LOG_CABIN
	{ 22696,	34,		19,		STR_CLASSICAL_ROMAN_ENTRANCE,	0x000057E9,			0,	},		// RIDE_ENTRANCE_STYLE_CLASSICAL_ROMAN
	{ 22792,	40,		22,		STR_ABSTRACT_ENTRANCE,			0x400057F5,			1,	},		// RIDE_ENTRANCE_STYLE_ABSTRACT
	{ 22824,	35,		23,		STR_SNOW_ICE_ENTRANCE,			0x0000580D,			0,	},		// RIDE_ENTRANCE_STYLE_SNOW_ICE
	{ 22840,	33,		19,		STR_PAGODA_ENTRANCE,			0x00005819,			0,	},		// RIDE_ENTRANCE_STYLE_PAGODA
	{ 22856,	33,		2,		STR_SPACE_ENTRANCE,				0x00005825,			0,	},		// RIDE_ENTRANCE_STYLE_SPACE
	{ 0,		0,		2,		STR_ENTRANCE_NONE,				0x00000000,			0,	}		// RIDE_ENTRANCE_STYLE_NONE
};

// Data read from 0x0097D7C9 4 bytes at a time
const uint8 RideLiftHillAdjustments[0x60] = {
		7,		 // Spiral Roller coaster
		4,		 // Stand Up Coaster
		4,		 // Suspended Swinging
		5,		 // Inverted
		4,		 // Steel Mini Coaster
		5,		 // Mini Railroad
		5,		 // Monorail
		4,		 // Mini Suspended Coaster
		5,		 // Boat Ride
		4,		 // Wooden Wild Mine/Mouse
		4,		 // Steeplechase/Motorbike/Soap Box Derby
		5,		 // Car Ride
		5,		 // Launched Freefall
		4,		 // Bobsleigh Coaster
		5,		 // Observation Tower
		4,		 // Looping Roller Coaster
		4,		 // Dinghy Slide
		4,		 // Mine Train Coaster
		5,		 // Chairlift
		4,		 // Corkscrew Roller Coaster
		5,		 // Maze
		5,		 // Spiral Slide
		5,		 // Go Karts
		5,		 // Log Flume
		5,		 // River Rapids
		5,		 // Bumper Cars
		5,		 // Pirate Ship
		5,		 // Swinging Inverter Ship
		5,		 // Food Stall
		5,		 // (none)
		5,		 // Drink Stall
		5,		 // (none)
		5,		 // Shop (all types)
		5,		 // Merry Go Round
		5,		 // Balloon Stall (maybe)
		5,		 // Information Kiosk
		5,		 // Bathroom
		5,		 // Ferris Wheel
		5,		 // Motion Simulator
		5,		 // 3D Cinema
		5,		 // Topspin
		5,		 // Space Rings
		5,		 // Reverse Freefall Coaster
		5,		 // Elevator
		4,		 // Vertical Drop Roller Coaster
		5,		 // ATM
		5,		 // Twist
		5,		 // Haunted House
		5,		 // First Aid
		5,		 // Circus Show
		5,		 // Ghost Train
		5,		 // Twister Roller Coaster
		5,		 // Wooden Roller Coaster
		3,		 // Side-Friction Roller Coaster
		4,		 // Wild Mouse
		4,		 // Multi Dimension Coaster
		4,		 // (none)
		4,		 // Flying Roller Coaster
		4,		 // (none)
		3,		 // Virginia Reel
		5,		 // Splash Boats
		5,		 // Mini Helicopters
		4,		 // Lay-down Roller Coaster
		5,		 // Suspended Monorail
		4,		 // (none)
		3,		 // Reverser Roller Coaster
		4,		 // Heartline Twister Roller Coaster
		5,		 // Mini Golf
		5,		 // Giga Coaster
		5,		 // Roto-Drop
		5,		 // Flying Saucers
		5,		 // Crooked House
		5,		 // Monorail Cycles
		4,		 // Compact Inverted Coaster
		4,		 // Water Coaster
		5,		 // Air Powered Vertical Coaster
		4,		 // Inverted Hairpin Coaster
		5,		 // Magic Carpet
		5,		 // Submarine Ride
		5,		 // River Rafts
		5,		 // (none)
		5,		 // Enterprise
		5,		 // (none)
		5,		 // (none)
		5,		 // (none)
		4,		 // (none)
		4,		 // Inverted Impulse Coaster
		4,		 // Mini Roller Coaster
		5,		 // Mine Ride
		4		 // LIM Launched Roller Coaster
};

// rct2: 0x0097D4F2
const rct_ride_data_4 RideData4[91] = {
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_PEEP_CHECK_GFORCES | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_HAS_AIR_TIME | RIDE_TYPE_FLAG4_ALLOW_MULTIPLE_CIRCUITS | RIDE_TYPE_FLAG4_11 | RIDE_TYPE_FLAG4_13,									MUSIC_STYLE_ROCK_STYLE_3,		0,		0,		0,		20,		20		},		// RIDE_TYPE_SPIRAL_ROLLER_COASTER
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_PEEP_CHECK_GFORCES | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_HAS_AIR_TIME | RIDE_TYPE_FLAG4_ALLOW_MULTIPLE_CIRCUITS | RIDE_TYPE_FLAG4_11 | RIDE_TYPE_FLAG4_13,									MUSIC_STYLE_TECHNO,				0,		0,		0,		20,		20		},		// RIDE_TYPE_STAND_UP_ROLLER_COASTER
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_PEEP_CHECK_GFORCES | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_HAS_AIR_TIME | RIDE_TYPE_FLAG4_11 | RIDE_TYPE_FLAG4_13,																			MUSIC_STYLE_ROCK,				0,		0,		0,		20,		20		},		// RIDE_TYPE_SUSPENDED_SWINGING_COASTER
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_PEEP_CHECK_GFORCES | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_HAS_AIR_TIME | RIDE_TYPE_FLAG4_ALLOW_MULTIPLE_CIRCUITS | RIDE_TYPE_FLAG4_11 | RIDE_TYPE_FLAG4_13,									MUSIC_STYLE_TECHNO,				0,		0,		0,		20,		20		},		// RIDE_TYPE_INVERTED_ROLLER_COASTER
	{ RIDE_TYPE_FLAG4_0 | RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_PEEP_CHECK_GFORCES | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_HAS_AIR_TIME | RIDE_TYPE_FLAG4_ALLOW_MULTIPLE_CIRCUITS | RIDE_TYPE_FLAG4_11 | RIDE_TYPE_FLAG4_13,				MUSIC_STYLE_SUMMER,				0,		0,		0,		10,		0		},		// RIDE_TYPE_JUNIOR_ROLLER_COASTER
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_6 | RIDE_TYPE_FLAG4_ALLOW_MULTIPLE_CIRCUITS | RIDE_TYPE_FLAG4_TRANSPORT_RIDE,																							MUSIC_STYLE_SUMMER,				0,		0,		0,		10,		0		},		// RIDE_TYPE_MINIATURE_RAILWAY
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_6 | RIDE_TYPE_FLAG4_ALLOW_MULTIPLE_CIRCUITS | RIDE_TYPE_FLAG4_TRANSPORT_RIDE,																							MUSIC_STYLE_SUMMER,				0,		0,		0,		20,		20		},		// RIDE_TYPE_MONORAIL
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_PEEP_CHECK_GFORCES | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_HAS_AIR_TIME | RIDE_TYPE_FLAG4_11 | RIDE_TYPE_FLAG4_13,																			MUSIC_STYLE_WILD_WEST,			0,		0,		0,		10,		0		},		// RIDE_TYPE_MINI_SUSPENDED_COASTER
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT,																																															MUSIC_STYLE_WATER,				0,		0,		0,		20,		20		},		// RIDE_TYPE_BOAT_RIDE
	{ RIDE_TYPE_FLAG4_0 | RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_PEEP_CHECK_GFORCES | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_HAS_AIR_TIME | RIDE_TYPE_FLAG4_11 | RIDE_TYPE_FLAG4_13,														MUSIC_STYLE_WILD_WEST,			0,		0,		0,		20,		20		},		// RIDE_TYPE_WOODEN_WILD_MOUSE
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_PEEP_CHECK_GFORCES | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_HAS_AIR_TIME | RIDE_TYPE_FLAG4_11 | RIDE_TYPE_FLAG4_13,																			MUSIC_STYLE_ROCK,				0,		0,		0,		15,		0		},		// RIDE_TYPE_STEEPLECHASE
	{ RIDE_TYPE_FLAG4_0 | RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_6 | RIDE_TYPE_FLAG4_14,																																MUSIC_STYLE_SUMMER,				0,		0,		0,		20,		0		},		// RIDE_TYPE_CAR_RIDE
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_13,																																										MUSIC_STYLE_TECHNO,				0,		0,		0,		20,		20		},		// RIDE_TYPE_LAUNCHED_FREEFALL
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_PEEP_CHECK_GFORCES | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_HAS_AIR_TIME | RIDE_TYPE_FLAG4_ALLOW_MULTIPLE_CIRCUITS | RIDE_TYPE_FLAG4_11 | RIDE_TYPE_FLAG4_13,									MUSIC_STYLE_ROCK,				0,		0,		0,		10,		0		},		// RIDE_TYPE_BOBSLEIGH_COASTER
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_14,																																										MUSIC_STYLE_SUMMER,				0,		0,		0,		20,		20		},		// RIDE_TYPE_OBSERVATION_TOWER
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_PEEP_CHECK_GFORCES | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_HAS_AIR_TIME | RIDE_TYPE_FLAG4_ALLOW_MULTIPLE_CIRCUITS | RIDE_TYPE_FLAG4_11 | RIDE_TYPE_FLAG4_13,									MUSIC_STYLE_ROCK,				0,		0,		0,		20,		20		},		// RIDE_TYPE_LOOPING_ROLLER_COASTER
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_HAS_AIR_TIME | RIDE_TYPE_FLAG4_11 | RIDE_TYPE_FLAG4_13,																													MUSIC_STYLE_WATER,				0,		0,		0,		20,		20		},		// RIDE_TYPE_DINGHY_SLIDE
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_PEEP_CHECK_GFORCES | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_HAS_AIR_TIME | RIDE_TYPE_FLAG4_ALLOW_MULTIPLE_CIRCUITS | RIDE_TYPE_FLAG4_11 | RIDE_TYPE_FLAG4_13,									MUSIC_STYLE_WILD_WEST,			0,		0,		0,		10,		0		},		// RIDE_TYPE_MINE_TRAIN_COASTER
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_6 | RIDE_TYPE_FLAG4_TRANSPORT_RIDE | RIDE_TYPE_FLAG4_14,																												MUSIC_STYLE_SUMMER,				0,		0,		0,		20,		20		},		// RIDE_TYPE_CHAIRLIFT
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_PEEP_CHECK_GFORCES | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_HAS_AIR_TIME | RIDE_TYPE_FLAG4_ALLOW_MULTIPLE_CIRCUITS | RIDE_TYPE_FLAG4_11 | RIDE_TYPE_FLAG4_13,									MUSIC_STYLE_ROCK,				0,		0,		0,		10,		0		},		// RIDE_TYPE_CORKSCREW_ROLLER_COASTER
	{ RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT,																																																						MUSIC_STYLE_SUMMER,				0,		0,		0,		15,		0		},		// RIDE_TYPE_MAZE
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_13,																																										MUSIC_STYLE_SUMMER,				0,		0,		0,		20,		0		},		// RIDE_TYPE_SPIRAL_SLIDE
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_13,																																										MUSIC_STYLE_TECHNO,				0,		0,		0,		20,		20		},		// RIDE_TYPE_GO_KARTS
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_6 | RIDE_TYPE_FLAG4_HAS_AIR_TIME | RIDE_TYPE_FLAG4_13,																													MUSIC_STYLE_WATER,				0,		0,		0,		20,		20		},		// RIDE_TYPE_LOG_FLUME
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_6 | RIDE_TYPE_FLAG4_HAS_AIR_TIME | RIDE_TYPE_FLAG4_13,																													MUSIC_STYLE_WATER,				0,		0,		0,		15,		0		},		// RIDE_TYPE_RIVER_RAPIDS
	{ RIDE_TYPE_FLAG4_MUSIC_ON_DEFAULT | RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_14,																																	MUSIC_STYLE_DODGEMS_BEAT,		0,		0,		0,		15,		0		},		// RIDE_TYPE_DODGEMS
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_SINGLE_SESSION | RIDE_TYPE_FLAG4_13,																																	MUSIC_STYLE_PIRATES,			0,		0,		0,		15,		0		},		// RIDE_TYPE_PIRATE_SHIP
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_SINGLE_SESSION | RIDE_TYPE_FLAG4_13,																																	MUSIC_STYLE_TECHNO,				0,		0,		0,		9,		0		},		// RIDE_TYPE_SWINGING_INVERTER_SHIP
	{ 0,																																																														MUSIC_STYLE_GENTLE,				0,		0,		0,		15,		0		},		// RIDE_TYPE_FOOD_STALL
	{ 0,																																																														MUSIC_STYLE_GENTLE,				0,		0,		0,		12,		0		},		// RIDE_TYPE_1D
	{ 0,																																																														MUSIC_STYLE_GENTLE,				0,		0,		0,		8,		0		},		// RIDE_TYPE_DRINK_STALL
	{ 0,																																																														MUSIC_STYLE_GENTLE,				0,		0,		0,		15,		0		},		// RIDE_TYPE_1F
	{ 0,																																																														MUSIC_STYLE_GENTLE,				0,		0,		0,		10,		0		},		// RIDE_TYPE_SHOP
	{ RIDE_TYPE_FLAG4_MUSIC_ON_DEFAULT | RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_SINGLE_SESSION | RIDE_TYPE_FLAG4_13,																									MUSIC_STYLE_FAIRGROUND_ORGAN,	0,		0,		0,		9,		0		},		// RIDE_TYPE_MERRY_GO_ROUND
	{ 0,																																																														MUSIC_STYLE_GENTLE,				0,		0,		0,		6,		25		},		// RIDE_TYPE_22
	{ 0,																																																														MUSIC_STYLE_GENTLE,				0,		0,		0,		0,		0		},		// RIDE_TYPE_INFORMATION_KIOSK
	{ 0,																																																														MUSIC_STYLE_GENTLE,				0,		0,		0,		10,		0		},		// RIDE_TYPE_TOILETS
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_SINGLE_SESSION | RIDE_TYPE_FLAG4_14,																																	MUSIC_STYLE_SUMMER,				0,		0,		0,		20,		0		},		// RIDE_TYPE_FERRIS_WHEEL
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_SINGLE_SESSION | RIDE_TYPE_FLAG4_14,																																	MUSIC_STYLE_ROCK,				0,		0,		0,		20,		0		},		// RIDE_TYPE_MOTION_SIMULATOR
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_SINGLE_SESSION,																																							MUSIC_STYLE_ROCK,				0,		0,		0,		20,		0		},		// RIDE_TYPE_3D_CINEMA
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_SINGLE_SESSION | RIDE_TYPE_FLAG4_13,																																	MUSIC_STYLE_ROCK,				0,		0,		0,		5,		0		},		// RIDE_TYPE_TOP_SPIN
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_SINGLE_SESSION | RIDE_TYPE_FLAG4_14,																																	MUSIC_STYLE_GENTLE,				0,		0,		0,		20,		0		},		// RIDE_TYPE_SPACE_RINGS
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_HAS_AIR_TIME | RIDE_TYPE_FLAG4_11 | RIDE_TYPE_FLAG4_13,																													MUSIC_STYLE_ROCK,				0,		0,		0,		10,		0		},		// RIDE_TYPE_REVERSE_FREEFALL_COASTER
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_TRANSPORT_RIDE,																																							MUSIC_STYLE_SUMMER,				0,		0,		0,		20,		20		},		// RIDE_TYPE_LIFT
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_PEEP_CHECK_GFORCES | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_HAS_AIR_TIME | RIDE_TYPE_FLAG4_11 | RIDE_TYPE_FLAG4_13,																			MUSIC_STYLE_ROCK,				0,		0,		0,		0,		0		},		// RIDE_TYPE_VERTICAL_DROP_ROLLER_COASTER
	{ 0,																																																														MUSIC_STYLE_GENTLE,				0,		0,		0,		10,		0		},		// RIDE_TYPE_CASH_MACHINE
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_SINGLE_SESSION | RIDE_TYPE_FLAG4_13,																																	MUSIC_STYLE_GENTLE,				0,		0,		0,		10,		0		},		// RIDE_TYPE_TWIST
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_SINGLE_SESSION,																																							MUSIC_STYLE_HORROR,				0,		0,		0,		0,		0		},		// RIDE_TYPE_HAUNTED_HOUSE
	{ 0,																																																														MUSIC_STYLE_GENTLE,				0,		0,		0,		15,		0		},		// RIDE_TYPE_FIRST_AID
	{ RIDE_TYPE_FLAG4_MUSIC_ON_DEFAULT | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_SINGLE_SESSION,																																					MUSIC_STYLE_CIRCUS_SHOW,		0,		0,		0,		15,		0		},		// RIDE_TYPE_CIRCUS_SHOW
	{ RIDE_TYPE_FLAG4_0 | RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_6 | RIDE_TYPE_FLAG4_HAS_AIR_TIME | RIDE_TYPE_FLAG4_13,																								MUSIC_STYLE_HORROR,				0,		0,		0,		20,		20		},		// RIDE_TYPE_GHOST_TRAIN
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_PEEP_CHECK_GFORCES | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_HAS_AIR_TIME | RIDE_TYPE_FLAG4_ALLOW_MULTIPLE_CIRCUITS | RIDE_TYPE_FLAG4_11 | RIDE_TYPE_FLAG4_13,									MUSIC_STYLE_ROCK,				0,		0,		0,		20,		20		},		// RIDE_TYPE_TWISTER_ROLLER_COASTER
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_PEEP_CHECK_GFORCES | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_HAS_AIR_TIME | RIDE_TYPE_FLAG4_ALLOW_MULTIPLE_CIRCUITS | RIDE_TYPE_FLAG4_11 | RIDE_TYPE_FLAG4_13,									MUSIC_STYLE_WILD_WEST,			0,		0,		0,		20,		20		},		// RIDE_TYPE_WOODEN_ROLLER_COASTER
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_PEEP_CHECK_GFORCES | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_HAS_AIR_TIME | RIDE_TYPE_FLAG4_ALLOW_MULTIPLE_CIRCUITS | RIDE_TYPE_FLAG4_11 | RIDE_TYPE_FLAG4_13,									MUSIC_STYLE_GENTLE,				0,		0,		0,		20,		20		},		// RIDE_TYPE_SIDE_FRICTION_ROLLER_COASTER
	{ RIDE_TYPE_FLAG4_0 | RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_PEEP_CHECK_GFORCES | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_HAS_AIR_TIME | RIDE_TYPE_FLAG4_11 | RIDE_TYPE_FLAG4_13,														MUSIC_STYLE_ROCK,				0,		0,		0,		20,		20		},		// RIDE_TYPE_WILD_MOUSE
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | (1 << 3) | RIDE_TYPE_FLAG4_PEEP_CHECK_GFORCES | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_HAS_AIR_TIME | RIDE_TYPE_FLAG4_ALLOW_MULTIPLE_CIRCUITS | RIDE_TYPE_FLAG4_11 | RIDE_TYPE_FLAG4_13,						MUSIC_STYLE_ROCK_STYLE_3,		56,		0,		0,		20,		20		},		// RIDE_TYPE_MULTI_DIMENSION_ROLLER_COASTER
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_13,																																																			MUSIC_STYLE_GENTLE,				0,		0,		0,		20,		20		},		// RIDE_TYPE_38
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | (1 << 3) | RIDE_TYPE_FLAG4_PEEP_CHECK_GFORCES | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_HAS_AIR_TIME | RIDE_TYPE_FLAG4_ALLOW_MULTIPLE_CIRCUITS | RIDE_TYPE_FLAG4_11 | RIDE_TYPE_FLAG4_13 | RIDE_TYPE_FLAG4_15,	MUSIC_STYLE_ROCK,				58,		0,		0,		20,		20		},		// RIDE_TYPE_FLYING_ROLLER_COASTER
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_13 | (1 << 15),																																																MUSIC_STYLE_GENTLE,				0,		0,		0,		20,		20		},		// RIDE_TYPE_3A
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_PEEP_CHECK_GFORCES | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_HAS_AIR_TIME | RIDE_TYPE_FLAG4_11 | RIDE_TYPE_FLAG4_13,																			MUSIC_STYLE_ROCK,				0,		0,		0,		20,		20		},		// RIDE_TYPE_VIRGINIA_REEL
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_6 | RIDE_TYPE_FLAG4_HAS_AIR_TIME | RIDE_TYPE_FLAG4_14,																													MUSIC_STYLE_WATER,				0,		0,		0,		15,		0		},		// RIDE_TYPE_SPLASH_BOATS
	{ RIDE_TYPE_FLAG4_0 | RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_6 | RIDE_TYPE_FLAG4_14,																																MUSIC_STYLE_SUMMER,				0,		0,		0,		20,		20		},		// RIDE_TYPE_MINI_HELICOPTERS
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | (1 << 3) | RIDE_TYPE_FLAG4_PEEP_CHECK_GFORCES | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_HAS_AIR_TIME | RIDE_TYPE_FLAG4_ALLOW_MULTIPLE_CIRCUITS | RIDE_TYPE_FLAG4_11 | RIDE_TYPE_FLAG4_13,						MUSIC_STYLE_ROCK,				64,		0,		0,		10,		0		},		// RIDE_TYPE_LAY_DOWN_ROLLER_COASTER
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_6 | RIDE_TYPE_FLAG4_ALLOW_MULTIPLE_CIRCUITS | RIDE_TYPE_FLAG4_TRANSPORT_RIDE,																							MUSIC_STYLE_SUMMER,				0,		0,		0,		20,		20		},		// RIDE_TYPE_SUSPENDED_MONORAIL
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_13,																																																			MUSIC_STYLE_GENTLE,				0,		0,		0,		20,		20		},		// RIDE_TYPE_40
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_PEEP_CHECK_GFORCES | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_HAS_AIR_TIME | RIDE_TYPE_FLAG4_11 | RIDE_TYPE_FLAG4_13,																			MUSIC_STYLE_TECHNO,				0,		0,		0,		15,		20		},		// RIDE_TYPE_REVERSER_ROLLER_COASTER
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_PEEP_CHECK_GFORCES | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_HAS_AIR_TIME | RIDE_TYPE_FLAG4_11 | RIDE_TYPE_FLAG4_13,																			MUSIC_STYLE_TECHNO,				0,		0,		0,		10,		0		},		// RIDE_TYPE_HEARTLINE_TWISTER_COASTER
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_14,																																										MUSIC_STYLE_SUMMER,				0,		0,		0,		20,		20		},		// RIDE_TYPE_MINI_GOLF
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_PEEP_CHECK_GFORCES | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_HAS_AIR_TIME | RIDE_TYPE_FLAG4_ALLOW_MULTIPLE_CIRCUITS | RIDE_TYPE_FLAG4_10 | RIDE_TYPE_FLAG4_11 | RIDE_TYPE_FLAG4_13,				MUSIC_STYLE_ROCK_STYLE_3,		0,		0,		0,		20,		0		},		// RIDE_TYPE_GIGA_COASTER
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_SINGLE_SESSION | RIDE_TYPE_FLAG4_13,																																	MUSIC_STYLE_ROCK,				0,		0,		0,		15,		0		},		// RIDE_TYPE_ROTO_DROP
	{ RIDE_TYPE_FLAG4_MUSIC_ON_DEFAULT | RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_SINGLE_SESSION | RIDE_TYPE_FLAG4_13,																									MUSIC_STYLE_ROCK,				0,		0,		0,		10,		0		},		// RIDE_TYPE_FLYING_SAUCERS
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_SINGLE_SESSION,																																							MUSIC_STYLE_GENTLE,				0,		0,		0,		15,		0		},		// RIDE_TYPE_CROOKED_HOUSE
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_14,																																										MUSIC_STYLE_SUMMER,				0,		0,		0,		20,		20		},		// RIDE_TYPE_MONORAIL_CYCLES
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_PEEP_CHECK_GFORCES | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_HAS_AIR_TIME | RIDE_TYPE_FLAG4_ALLOW_MULTIPLE_CIRCUITS | RIDE_TYPE_FLAG4_11 | RIDE_TYPE_FLAG4_13,									MUSIC_STYLE_TECHNO,				0,		0,		0,		20,		20		},		// RIDE_TYPE_COMPACT_INVERTED_COASTER
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_PEEP_CHECK_GFORCES | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_HAS_AIR_TIME | RIDE_TYPE_FLAG4_11 | RIDE_TYPE_FLAG4_13,																			MUSIC_STYLE_WATER,				0,		0,		0,		20,		0		},		// RIDE_TYPE_WATER_COASTER
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_PEEP_CHECK_GFORCES | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_HAS_AIR_TIME | RIDE_TYPE_FLAG4_11 | RIDE_TYPE_FLAG4_13,																			MUSIC_STYLE_ROCK_STYLE_2,		0,		0,		0,		20,		20		},		// RIDE_TYPE_AIR_POWERED_VERTICAL_COASTER
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_PEEP_CHECK_GFORCES | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_HAS_AIR_TIME | RIDE_TYPE_FLAG4_11 | RIDE_TYPE_FLAG4_13,																			MUSIC_STYLE_ROCK_STYLE_2,		0,		0,		0,		15,		0		},		// RIDE_TYPE_INVERTED_HAIRPIN_COASTER
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_SINGLE_SESSION | RIDE_TYPE_FLAG4_13,																																	MUSIC_STYLE_EGYPTIAN,			0,		0,		0,		10,		0		},		// RIDE_TYPE_MAGIC_CARPET
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_PEEP_CHECK_GFORCES | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_6,																																	MUSIC_STYLE_WATER,				0,		0,		0,		10,		20		},		// RIDE_TYPE_SUBMARINE_RIDE
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_6 | RIDE_TYPE_FLAG4_14,																																					MUSIC_STYLE_GENTLE,				0,		0,		0,		7,		0		},		// RIDE_TYPE_RIVER_RAFTS
	{ 0,																																																														MUSIC_STYLE_GENTLE,				0,		0,		0,		20,		0		},		// RIDE_TYPE_50
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_SINGLE_SESSION | RIDE_TYPE_FLAG4_13,																																	MUSIC_STYLE_ROCK_STYLE_2,		0,		0,		0,		12,		0		},		// RIDE_TYPE_ENTERPRISE
	{ 0,																																																														MUSIC_STYLE_GENTLE,				0,		0,		0,		15,		0		},		// RIDE_TYPE_52
	{ 0,																																																														MUSIC_STYLE_GENTLE,				0,		0,		0,		12,		0		},		// RIDE_TYPE_53
	{ 0,																																																														MUSIC_STYLE_GENTLE,				0,		0,		0,		20,		20		},		// RIDE_TYPE_54
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_PEEP_CHECK_GFORCES | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_HAS_AIR_TIME | RIDE_TYPE_FLAG4_ALLOW_MULTIPLE_CIRCUITS | RIDE_TYPE_FLAG4_11 | RIDE_TYPE_FLAG4_13,									MUSIC_STYLE_ROCK,				0,		0,		0,		20,		20		},		// RIDE_TYPE_55
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_PEEP_CHECK_GFORCES | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_HAS_AIR_TIME | RIDE_TYPE_FLAG4_ALLOW_MULTIPLE_CIRCUITS | RIDE_TYPE_FLAG4_11 | RIDE_TYPE_FLAG4_13,									MUSIC_STYLE_ROCK,				0,		0,		0,		20,		20		},		// RIDE_TYPE_INVERTED_IMPULSE_COASTER
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_PEEP_CHECK_GFORCES | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_HAS_AIR_TIME | RIDE_TYPE_FLAG4_ALLOW_MULTIPLE_CIRCUITS | RIDE_TYPE_FLAG4_11 | RIDE_TYPE_FLAG4_13,									MUSIC_STYLE_SUMMER,				0,		0,		0,		20,		20		},		// RIDE_TYPE_MINI_ROLLER_COASTER
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_PEEP_CHECK_GFORCES | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_HAS_AIR_TIME | RIDE_TYPE_FLAG4_ALLOW_MULTIPLE_CIRCUITS | RIDE_TYPE_FLAG4_11 | RIDE_TYPE_FLAG4_13,									MUSIC_STYLE_WILD_WEST,			0,		0,		0,		20,		20		},		// RIDE_TYPE_MINE_RIDE
	{ RIDE_TYPE_FLAG4_0 | RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_PEEP_CHECK_GFORCES | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_HAS_AIR_TIME | RIDE_TYPE_FLAG4_13,																				MUSIC_STYLE_ROCK_STYLE_2,		0,		0,		0,		20,		20		},		// RIDE_TYPE_59
	{ RIDE_TYPE_FLAG4_ALLOW_MUSIC | RIDE_TYPE_FLAG4_PEEP_CHECK_GFORCES | RIDE_TYPE_FLAG4_HAS_ENTRANCE_EXIT | RIDE_TYPE_FLAG4_HAS_AIR_TIME | RIDE_TYPE_FLAG4_ALLOW_MULTIPLE_CIRCUITS | RIDE_TYPE_FLAG4_11 | RIDE_TYPE_FLAG4_13,									MUSIC_STYLE_ROCK,				0,		0,		0,		12,		7		},		// RIDE_TYPE_LIM_LAUNCHED_ROLLER_COASTER
};

// rct2: 0x00982358
const money8 DefaultShopItemPrice[SHOP_ITEM_COUNT] = {
	MONEY(0,90),						// SHOP_ITEM_BALLOON
	MONEY(2,50),						// SHOP_ITEM_TOY
	MONEY(0,60),						// SHOP_ITEM_MAP
	MONEY(0,00),						// SHOP_ITEM_PHOTO
	MONEY(2,50),						// SHOP_ITEM_UMBRELLA
	MONEY(1,20),						// SHOP_ITEM_DRINK
	MONEY(1,50),						// SHOP_ITEM_BURGER
	MONEY(1,50),						// SHOP_ITEM_FRIES
	MONEY(0,90),						// SHOP_ITEM_ICE_CREAM
	MONEY(0,80),						// SHOP_ITEM_COTTON_CANDY
	MONEY(0,00),						// SHOP_ITEM_EMPTY_CAN
	MONEY(0,00),						// SHOP_ITEM_RUBBISH
	MONEY(0,00),						// SHOP_ITEM_EMPTY_BURGER_BOX
	MONEY(1,60),						// SHOP_ITEM_PIZZA
	MONEY(0,00),						// SHOP_ITEM_VOUCHER
	MONEY(1,20),						// SHOP_ITEM_POPCORN
	MONEY(1,00),						// SHOP_ITEM_HOT_DOG
	MONEY(1,50),						// SHOP_ITEM_TENTACLE
	MONEY(1,50),						// SHOP_ITEM_HAT
	MONEY(0,70),						// SHOP_ITEM_CANDY_APPLE
	MONEY(3,00),						// SHOP_ITEM_TSHIRT
	MONEY(0,70),						// SHOP_ITEM_DONUT
	MONEY(1,20),						// SHOP_ITEM_COFFEE
	MONEY(0,00),						// SHOP_ITEM_EMPTY_CUP
	MONEY(1,50),						// SHOP_ITEM_CHICKEN
	MONEY(1,20),						// SHOP_ITEM_LEMONADE
	MONEY(0,00),						// SHOP_ITEM_EMPTY_BOX
	MONEY(0,00),						// SHOP_ITEM_EMPTY_BOTTLE
	MONEY(0,00),						// 28
	MONEY(0,00),						// 29
	MONEY(0,00),						// 30
	MONEY(0,00),						// 31
	MONEY(0,00),						// SHOP_ITEM_PHOTO2
	MONEY(0,00),						// SHOP_ITEM_PHOTO3
	MONEY(0,00),						// SHOP_ITEM_PHOTO4
	MONEY(1,10),						// SHOP_ITEM_PRETZEL
	MONEY(1,20),						// SHOP_ITEM_CHOCOLATE
	MONEY(1,10),						// SHOP_ITEM_ICED_TEA
	MONEY(1,20),						// SHOP_ITEM_FUNNEL_CAKE
	MONEY(1,50),						// SHOP_ITEM_SUNGLASSES
	MONEY(1,50),						// SHOP_ITEM_BEEF_NOODLES
	MONEY(1,50),						// SHOP_ITEM_FRIED_RICE_NOODLES
	MONEY(1,50),						// SHOP_ITEM_WONTON_SOUP
	MONEY(1,50),						// SHOP_ITEM_MEATBALL_SOUP
	MONEY(1,20),						// SHOP_ITEM_FRUIT_JUICE
	MONEY(1,20),						// SHOP_ITEM_SOYBEAN_MILK
	MONEY(1,20),						// SHOP_ITEM_SU_JONGKWA
	MONEY(1,50),						// SHOP_ITEM_SUB_SANDWICH
	MONEY(0,70),						// SHOP_ITEM_COOKIE
	MONEY(0,00),						// SHOP_ITEM_EMPTY_BOWL_RED
	MONEY(0,00),						// SHOP_ITEM_EMPTY_DRINK_CARTON
	MONEY(0,00),						// SHOP_ITEM_EMPTY_JUICE_CUP
	MONEY(1,50),						// SHOP_ITEM_ROAST_SAUSAGE
	MONEY(0,00),						// SHOP_ITEM_EMPTY_BOWL_BLUE
	MONEY(0,00),						// 54
	MONEY(0,00),						// 55
};

const rct_shop_item_string_types ShopItemStringIds[SHOP_ITEM_COUNT] = {
	{ STR_SHOP_ITEM_SINGULAR_BALLOON,				STR_SHOP_ITEM_PLURAL_BALLOON,				STR_SHOP_ITEM_INDEFINITE_BALLOON,				STR_SHOP_ITEM_DISPLAY_BALLOON				},
	{ STR_SHOP_ITEM_SINGULAR_CUDDLY_TOY,			STR_SHOP_ITEM_PLURAL_CUDDLY_TOY,			STR_SHOP_ITEM_INDEFINITE_CUDDLY_TOY,			STR_SHOP_ITEM_DISPLAY_CUDDLY_TOY			},
	{ STR_SHOP_ITEM_SINGULAR_PARK_MAP,				STR_SHOP_ITEM_PLURAL_PARK_MAP,				STR_SHOP_ITEM_INDEFINITE_PARK_MAP,				STR_SHOP_ITEM_DISPLAY_PARK_MAP				},
	{ STR_SHOP_ITEM_SINGULAR_ON_RIDE_PHOTO,			STR_SHOP_ITEM_PLURAL_ON_RIDE_PHOTO,			STR_SHOP_ITEM_INDEFINITE_ON_RIDE_PHOTO,			STR_SHOP_ITEM_DISPLAY_ON_RIDE_PHOTO			},
	{ STR_SHOP_ITEM_SINGULAR_UMBRELLA,				STR_SHOP_ITEM_PLURAL_UMBRELLA,				STR_SHOP_ITEM_INDEFINITE_UMBRELLA,				STR_SHOP_ITEM_DISPLAY_UMBRELLA				},
	{ STR_SHOP_ITEM_SINGULAR_DRINK,					STR_SHOP_ITEM_PLURAL_DRINK,					STR_SHOP_ITEM_INDEFINITE_DRINK,					STR_SHOP_ITEM_DISPLAY_DRINK					},
	{ STR_SHOP_ITEM_SINGULAR_BURGER,				STR_SHOP_ITEM_PLURAL_BURGER,				STR_SHOP_ITEM_INDEFINITE_BURGER,				STR_SHOP_ITEM_DISPLAY_BURGER				},
	{ STR_SHOP_ITEM_SINGULAR_CHIPS,					STR_SHOP_ITEM_PLURAL_CHIPS,					STR_SHOP_ITEM_INDEFINITE_CHIPS,					STR_SHOP_ITEM_DISPLAY_CHIPS					},
	{ STR_SHOP_ITEM_SINGULAR_ICE_CREAM,				STR_SHOP_ITEM_PLURAL_ICE_CREAM,				STR_SHOP_ITEM_INDEFINITE_ICE_CREAM,				STR_SHOP_ITEM_DISPLAY_ICE_CREAM				},
	{ STR_SHOP_ITEM_SINGULAR_CANDYFLOSS,			STR_SHOP_ITEM_PLURAL_CANDYFLOSS,			STR_SHOP_ITEM_INDEFINITE_CANDYFLOSS,			STR_SHOP_ITEM_DISPLAY_CANDYFLOSS			},
	{ STR_SHOP_ITEM_SINGULAR_EMPTY_CAN,				STR_SHOP_ITEM_PLURAL_EMPTY_CAN,				STR_SHOP_ITEM_INDEFINITE_EMPTY_CAN,				STR_SHOP_ITEM_DISPLAY_EMPTY_CAN				},
	{ STR_SHOP_ITEM_SINGULAR_RUBBISH,				STR_SHOP_ITEM_PLURAL_RUBBISH,				STR_SHOP_ITEM_INDEFINITE_RUBBISH,				STR_SHOP_ITEM_DISPLAY_RUBBISH				},
	{ STR_SHOP_ITEM_SINGULAR_EMPTY_BURGER_BOX,		STR_SHOP_ITEM_PLURAL_EMPTY_BURGER_BOX,		STR_SHOP_ITEM_INDEFINITE_EMPTY_BURGER_BOX,		STR_SHOP_ITEM_DISPLAY_EMPTY_BURGER_BOX		},
	{ STR_SHOP_ITEM_SINGULAR_PIZZA,					STR_SHOP_ITEM_PLURAL_PIZZA,					STR_SHOP_ITEM_INDEFINITE_PIZZA,					STR_SHOP_ITEM_DISPLAY_PIZZA					},
	{ STR_SHOP_ITEM_SINGULAR_VOUCHER,				STR_SHOP_ITEM_PLURAL_VOUCHER,				STR_SHOP_ITEM_INDEFINITE_VOUCHER,				STR_SHOP_ITEM_DISPLAY_VOUCHER				},
	{ STR_SHOP_ITEM_SINGULAR_POPCORN,				STR_SHOP_ITEM_PLURAL_POPCORN,				STR_SHOP_ITEM_INDEFINITE_POPCORN,				STR_SHOP_ITEM_DISPLAY_POPCORN				},
	{ STR_SHOP_ITEM_SINGULAR_HOT_DOG,				STR_SHOP_ITEM_PLURAL_HOT_DOG,				STR_SHOP_ITEM_INDEFINITE_HOT_DOG,				STR_SHOP_ITEM_DISPLAY_HOT_DOG				},
	{ STR_SHOP_ITEM_SINGULAR_TENTACLE,				STR_SHOP_ITEM_PLURAL_TENTACLE,				STR_SHOP_ITEM_INDEFINITE_TENTACLE,				STR_SHOP_ITEM_DISPLAY_TENTACLE				},
	{ STR_SHOP_ITEM_SINGULAR_HAT,					STR_SHOP_ITEM_PLURAL_HAT,					STR_SHOP_ITEM_INDEFINITE_HAT,					STR_SHOP_ITEM_DISPLAY_HAT					},
	{ STR_SHOP_ITEM_SINGULAR_TOFFEE_APPLE,			STR_SHOP_ITEM_PLURAL_TOFFEE_APPLE,			STR_SHOP_ITEM_INDEFINITE_TOFFEE_APPLE,			STR_SHOP_ITEM_DISPLAY_TOFFEE_APPLE			},
	{ STR_SHOP_ITEM_SINGULAR_T_SHIRT,				STR_SHOP_ITEM_PLURAL_T_SHIRT,				STR_SHOP_ITEM_INDEFINITE_T_SHIRT,				STR_SHOP_ITEM_DISPLAY_T_SHIRT				},
	{ STR_SHOP_ITEM_SINGULAR_DOUGHNUT,				STR_SHOP_ITEM_PLURAL_DOUGHNUT,				STR_SHOP_ITEM_INDEFINITE_DOUGHNUT,				STR_SHOP_ITEM_DISPLAY_DOUGHNUT				},
	{ STR_SHOP_ITEM_SINGULAR_COFFEE,				STR_SHOP_ITEM_PLURAL_COFFEE,				STR_SHOP_ITEM_INDEFINITE_COFFEE,				STR_SHOP_ITEM_DISPLAY_COFFEE				},
	{ STR_SHOP_ITEM_SINGULAR_EMPTY_CUP,				STR_SHOP_ITEM_PLURAL_EMPTY_CUP,				STR_SHOP_ITEM_INDEFINITE_EMPTY_CUP,				STR_SHOP_ITEM_DISPLAY_EMPTY_CUP				},
	{ STR_SHOP_ITEM_SINGULAR_FRIED_CHICKEN,			STR_SHOP_ITEM_PLURAL_FRIED_CHICKEN,			STR_SHOP_ITEM_INDEFINITE_FRIED_CHICKEN,			STR_SHOP_ITEM_DISPLAY_FRIED_CHICKEN			},
	{ STR_SHOP_ITEM_SINGULAR_LEMONADE,				STR_SHOP_ITEM_PLURAL_LEMONADE,				STR_SHOP_ITEM_INDEFINITE_LEMONADE,				STR_SHOP_ITEM_DISPLAY_LEMONADE				},
	{ STR_SHOP_ITEM_SINGULAR_EMPTY_BOX,				STR_SHOP_ITEM_PLURAL_EMPTY_BOX,				STR_SHOP_ITEM_INDEFINITE_EMPTY_BOX,				STR_SHOP_ITEM_DISPLAY_EMPTY_BOX				},
	{ STR_SHOP_ITEM_SINGULAR_EMPTY_BOTTLE,			STR_SHOP_ITEM_PLURAL_EMPTY_BOTTLE,			STR_SHOP_ITEM_INDEFINITE_EMPTY_BOTTLE,			STR_SHOP_ITEM_DISPLAY_EMPTY_BOTTLE			},
	{ STR_NONE,										STR_NONE,									STR_NONE,										STR_NONE									},
	{ STR_NONE,										STR_NONE,									STR_NONE,										STR_NONE									},
	{ STR_NONE,										STR_NONE,									STR_NONE,										STR_NONE									},
	{ STR_NONE,										STR_NONE,									STR_NONE,										STR_NONE									},
	{ STR_SHOP_ITEM_SINGULAR_ON_RIDE_PHOTO,			STR_SHOP_ITEM_PLURAL_ON_RIDE_PHOTO,			STR_SHOP_ITEM_INDEFINITE_ON_RIDE_PHOTO,			STR_SHOP_ITEM_DISPLAY_ON_RIDE_PHOTO			},
	{ STR_SHOP_ITEM_SINGULAR_ON_RIDE_PHOTO,			STR_SHOP_ITEM_PLURAL_ON_RIDE_PHOTO,			STR_SHOP_ITEM_INDEFINITE_ON_RIDE_PHOTO,			STR_SHOP_ITEM_DISPLAY_ON_RIDE_PHOTO			},
	{ STR_SHOP_ITEM_SINGULAR_ON_RIDE_PHOTO,			STR_SHOP_ITEM_PLURAL_ON_RIDE_PHOTO,			STR_SHOP_ITEM_INDEFINITE_ON_RIDE_PHOTO,			STR_SHOP_ITEM_DISPLAY_ON_RIDE_PHOTO			},
	{ STR_SHOP_ITEM_SINGULAR_PRETZEL,				STR_SHOP_ITEM_PLURAL_PRETZEL,				STR_SHOP_ITEM_INDEFINITE_PRETZEL,				STR_SHOP_ITEM_DISPLAY_PRETZEL				},
	{ STR_SHOP_ITEM_SINGULAR_HOT_CHOCOLATE,			STR_SHOP_ITEM_PLURAL_HOT_CHOCOLATE,			STR_SHOP_ITEM_INDEFINITE_HOT_CHOCOLATE,			STR_SHOP_ITEM_DISPLAY_HOT_CHOCOLATE			},
	{ STR_SHOP_ITEM_SINGULAR_ICED_TEA,				STR_SHOP_ITEM_PLURAL_ICED_TEA,				STR_SHOP_ITEM_INDEFINITE_ICED_TEA,				STR_SHOP_ITEM_DISPLAY_ICED_TEA				},
	{ STR_SHOP_ITEM_SINGULAR_FUNNEL_CAKE,			STR_SHOP_ITEM_PLURAL_FUNNEL_CAKE,			STR_SHOP_ITEM_INDEFINITE_FUNNEL_CAKE,			STR_SHOP_ITEM_DISPLAY_FUNNEL_CAKE			},
	{ STR_SHOP_ITEM_SINGULAR_SUNGLASSES,			STR_SHOP_ITEM_PLURAL_SUNGLASSES,			STR_SHOP_ITEM_INDEFINITE_SUNGLASSES,			STR_SHOP_ITEM_DISPLAY_SUNGLASSES			},
	{ STR_SHOP_ITEM_SINGULAR_BEEF_NOODLES,			STR_SHOP_ITEM_PLURAL_BEEF_NOODLES,			STR_SHOP_ITEM_INDEFINITE_BEEF_NOODLES,			STR_SHOP_ITEM_DISPLAY_BEEF_NOODLES			},
	{ STR_SHOP_ITEM_SINGULAR_FRIED_RICE_NOODLES,	STR_SHOP_ITEM_PLURAL_FRIED_RICE_NOODLES,	STR_SHOP_ITEM_INDEFINITE_FRIED_RICE_NOODLES,	STR_SHOP_ITEM_DISPLAY_FRIED_RICE_NOODLES	},
	{ STR_SHOP_ITEM_SINGULAR_WONTON_SOUP,			STR_SHOP_ITEM_PLURAL_WONTON_SOUP,			STR_SHOP_ITEM_INDEFINITE_WONTON_SOUP,			STR_SHOP_ITEM_DISPLAY_WONTON_SOUP			},
	{ STR_SHOP_ITEM_SINGULAR_MEATBALL_SOUP,			STR_SHOP_ITEM_PLURAL_MEATBALL_SOUP,			STR_SHOP_ITEM_INDEFINITE_MEATBALL_SOUP,			STR_SHOP_ITEM_DISPLAY_MEATBALL_SOUP			},
	{ STR_SHOP_ITEM_SINGULAR_FRUIT_JUICE,			STR_SHOP_ITEM_PLURAL_FRUIT_JUICE,			STR_SHOP_ITEM_INDEFINITE_FRUIT_JUICE,			STR_SHOP_ITEM_DISPLAY_FRUIT_JUICE			},
	{ STR_SHOP_ITEM_SINGULAR_SOYBEAN_MILK,			STR_SHOP_ITEM_PLURAL_SOYBEAN_MILK,			STR_SHOP_ITEM_INDEFINITE_SOYBEAN_MILK,			STR_SHOP_ITEM_DISPLAY_SOYBEAN_MILK			},
	{ STR_SHOP_ITEM_SINGULAR_SUJONGKWA,				STR_SHOP_ITEM_PLURAL_SUJONGKWA,				STR_SHOP_ITEM_INDEFINITE_SUJONGKWA,				STR_SHOP_ITEM_DISPLAY_SUJONGKWA				},
	{ STR_SHOP_ITEM_SINGULAR_SUB_SANDWICH,			STR_SHOP_ITEM_PLURAL_SUB_SANDWICH,			STR_SHOP_ITEM_INDEFINITE_SUB_SANDWICH,			STR_SHOP_ITEM_DISPLAY_SUB_SANDWICH			},
	{ STR_SHOP_ITEM_SINGULAR_COOKIE,				STR_SHOP_ITEM_PLURAL_COOKIE,				STR_SHOP_ITEM_INDEFINITE_COOKIE,				STR_SHOP_ITEM_DISPLAY_COOKIE				},
	{ STR_SHOP_ITEM_SINGULAR_EMPTY_BOWL_RED,		STR_SHOP_ITEM_PLURAL_EMPTY_BOWL_RED,		STR_SHOP_ITEM_INDEFINITE_EMPTY_BOWL_RED,		STR_SHOP_ITEM_DISPLAY_EMPTY_BOWL_RED		},
	{ STR_SHOP_ITEM_SINGULAR_EMPTY_DRINK_CARTON,	STR_SHOP_ITEM_PLURAL_EMPTY_DRINK_CARTON,	STR_SHOP_ITEM_INDEFINITE_EMPTY_DRINK_CARTON,	STR_SHOP_ITEM_DISPLAY_EMPTY_DRINK_CARTON	},
	{ STR_SHOP_ITEM_SINGULAR_EMPTY_JUICE_CUP,		STR_SHOP_ITEM_PLURAL_EMPTY_JUICE_CUP,		STR_SHOP_ITEM_INDEFINITE_EMPTY_JUICE_CUP,		STR_SHOP_ITEM_DISPLAY_EMPTY_JUICE_CUP		},
	{ STR_SHOP_ITEM_SINGULAR_ROAST_SAUSAGE,			STR_SHOP_ITEM_PLURAL_ROAST_SAUSAGE,			STR_SHOP_ITEM_INDEFINITE_ROAST_SAUSAGE,			STR_SHOP_ITEM_DISPLAY_ROAST_SAUSAGE			},
	{ STR_SHOP_ITEM_SINGULAR_EMPTY_BOWL_BLUE,		STR_SHOP_ITEM_PLURAL_EMPTY_BOWL_BLUE,		STR_SHOP_ITEM_INDEFINITE_EMPTY_BOWL_BLUE,		STR_SHOP_ITEM_DISPLAY_EMPTY_BOWL_BLUE		},
};

const uint32 ShopItemImage[SHOP_ITEM_COUNT] = {
	5061,									// SHOP_ITEM_BALLOON
	5062,									// SHOP_ITEM_TOY
	5063,									// SHOP_ITEM_MAP
	5064,									// SHOP_ITEM_PHOTO
	5065,									// SHOP_ITEM_UMBRELLA
	5066,									// SHOP_ITEM_DRINK
	5067,									// SHOP_ITEM_BURGER
	5068,									// SHOP_ITEM_FRIES
	5069,									// SHOP_ITEM_ICE_CREAM
	5070,									// SHOP_ITEM_COTTON_CANDY
	5071,									// SHOP_ITEM_EMPTY_CAN
	5072,									// SHOP_ITEM_RUBBISH
	5073,									// SHOP_ITEM_EMPTY_BURGER_BOX
	5074,									// SHOP_ITEM_PIZZA
	5075,									// SHOP_ITEM_VOUCHER
	5076,									// SHOP_ITEM_POPCORN
	5077,									// SHOP_ITEM_HOT_DOG
	5078,									// SHOP_ITEM_TENTACLE
	5079,									// SHOP_ITEM_HAT
	5080,									// SHOP_ITEM_CANDY_APPLE
	5081,									// SHOP_ITEM_TSHIRT
	5082,									// SHOP_ITEM_DONUT
	5083,									// SHOP_ITEM_COFFEE
	5084,									// SHOP_ITEM_EMPTY_CUP
	5085,									// SHOP_ITEM_CHICKEN
	5086,									// SHOP_ITEM_LEMONADE
	5087,									// SHOP_ITEM_EMPTY_BOX
	5088,									// SHOP_ITEM_EMPTY_BOTTLE
	0,										// 28
	0,										// 29
	0,										// 30
	0,										// 31
	5089,									// SHOP_ITEM_PHOTO2
	5090,									// SHOP_ITEM_PHOTO3
	5091,									// SHOP_ITEM_PHOTO4
	5092,									// SHOP_ITEM_PRETZEL
	5093,									// SHOP_ITEM_CHOCOLATE
	5094,									// SHOP_ITEM_ICED_TEA
	5095,									// SHOP_ITEM_FUNNEL_CAKE
	5096,									// SHOP_ITEM_SUNGLASSES
	5097,									// SHOP_ITEM_BEEF_NOODLES
	5098,									// SHOP_ITEM_FRIED_RICE_NOODLES
	5099,									// SHOP_ITEM_WONTON_SOUP
	5100,									// SHOP_ITEM_MEATBALL_SOUP
	5101,									// SHOP_ITEM_FRUIT_JUICE
	5102,									// SHOP_ITEM_SOYBEAN_MILK
	5103,									// SHOP_ITEM_SU_JONGKWA
	5104,									// SHOP_ITEM_SUB_SANDWICH
	5105,									// SHOP_ITEM_COOKIE
	5106,									// SHOP_ITEM_EMPTY_BOWL_RED
	5107,									// SHOP_ITEM_EMPTY_DRINK_CARTON
	5108,									// SHOP_ITEM_EMPTY_JUICE_CUP
	5109,									// SHOP_ITEM_ROAST_SAUSAGE
	5110,									// SHOP_ITEM_EMPTY_BOWL_BLUE
};

const rct_ride_type_vehicle CableLiftVehicle = {
	.rotation_frame_mask = 31,
	.var_02 = 0,
	.var_03 = 0,
	.var_04 = 0,
	.car_friction = 0,
	.tab_height = 0,
	.num_seats = 0,
	.sprite_flags = 0x7,
	.sprite_width = 0,
	.sprite_height_negative = 0,
	.sprite_height_positive = 0,
	.var_11 = 0,
	.var_12 = 0,
	.var_14 = 0,
	.var_16 = 1,
	.base_image_id = 29110,
	.var_1C = 0,
	.var_20 = 29142,
	.var_24 = 29214,
	.var_28 = 0,
	.var_2C = 0,
	.var_30 = 0,
	.var_34 = 0,
	.var_38 = 0,
	.var_3C = 0,
	.var_40 = 0,
	.var_44 = 0,
	.var_48 = 0,
	.var_4C = 0,
	.no_vehicle_images = 0,
	.no_seating_rows = 0,
	.spinning_inertia = 0,
	.spinning_friction = 255,
	.pad_57 = { 0,0,0 },
	.var_5A = 0,
	.powered_acceleration = 0,
	.powered_max_speed = 0,
	.car_visual = 0,
	.pad_5E = 1,
	.draw_order = 14,
	.special_frames = 0,
	.peep_loading_positions = NULL
};
