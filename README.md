# BattleDot
## Roadmap
- [ ] Logging
- [ ] Testing
- [ ] Cross-Platform
- [ ] CI/CD Pipeline
- [ ] Serialization
- [ ] Docs
- [ ] TUI

## Original Requirements
BattleDot is based off an assignment I was given in the past, the original requirements are as follows:

### Technical Requirements
Please implement a Linux-running program in 
- [x] C 
- [ ] C++ 
- [ ] Python 
- [ ] Java.
The whole implementation needs to start with one command (program/script in any language).
The game plays by itself after it starts, it is not interactive/does not require input.

### Problem Statement
- Rather than having two players oppose each other directly, any player will be attacked by one opponent and in turn will attack a different opponent. 
- Players are connected in a ring: A is bombing B who is bombing C, ... who is bombing Z who is bombing A.
- Each player has a 10x10 grid of "dots" where one "single-dot ship" is positioned randomly.
- A player loses if this ship is bombed
- Players cannot see each other's grids directly.
- Each player randomly selects a dot location on the enemy grid to bomb and sends the bomb to the enemy.
- If the bomb lands in the enemy's dot-ship, the enemy dies; otherwise, it lives.
- When a player dies, relevant neighbors are matched up so that their unfinished games can continue. 
- Must create a log file

### Bonus Points
- [x] A multi-threaded program, or a multi-process program, where the processes communicate using IPC of your choice.
- [x] If a process leaves the game (for ex. by an external signal), their neighbors become similarly matched up.
- [x] The solution must use connections that could work between multiple computers.
- [ ] A new player can be added to BattleDotNet by specifying two adjacent nodes in the BattleDotNet.
- [ ] Implement without a "master" node - only peer-to-peer


