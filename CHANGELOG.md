# Changelog

## [Unreleased]

## Known issues
- Issues with timing loop and sprite animation (it looks different from the original game)
- You cannot die - the code for aliens too low, and alien shots not yet implemented
- Sprite scrolling is not smooth - its locked in step with the music, when it should run independently.
- Mothership sound effect is poor, and overlaps the destruction sound (i.e. keeps playing)

## Todo
- Enemy bullets - firing, collision detection, sfx
- Shield destruction behaviour
- Code clean up - Variable name consistency, unrolling loops, additional code comments
- Smooth scrolling (is this even possible/desirable?)

## v0.8 - 01/07/2020
### Added
- Mothership destruction sprite
- Improved sprite animation timing
- Initial logic for enemy bullets, collision detection with player

## v0.7 - 30/06/2020
### Added
- Enemy bullet sprites
- Shield sprites
- Levels (simple, just increments the y by 4 pixels)
- Enemy speedup

## v0.6 - 28/06/2020
### Added
- Bonus/special ship loop
- Additional sounds (mothership)
- Mothership sprite (Bonus/special ship)
- Additional sprites, for enemy death and a "miss" at the top of the screen
### Removed
- vely speedup for enemies based on enemiesLeft - this did not work very well, needs to be rewritten for smooth sprite positioning.
### Changed
- Improved title/start screen, with ship sprites
- Background music - the ticks for enemy movement and animation could not be lined up easily with a playing XGM, so reverted to sfx for the music "beats".
- Changed sfx to stereo from mono

## v0.5 - 27/06/2020
### Added
- Background music
- Additional code comments
- vely speedup for enemies based on enemiesLeft;
### Changed
- Enemy generation loop, so that all enemies are contained within single entity

## v0.4 - 26/06/2020
### Added
- Sound (player shot, enemy destroyed)
- Colour (player ship)
- Enemy/Player collision detection
- Title screen
- Game Over screen

## v0.3 - 25/06/2020
### Added
- Joypad handling
- Player movement
- Bullet sprite
- Bullet sprite movement
- Collision detection
### Changed
- Improved code comments

## v0.2 - 24/06/2020
### Added
- Enemy movement

## v0.1 - 23/06
### Added
- Main game loop
- Player sprite
- Enemy sprites
