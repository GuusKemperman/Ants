# Welcome to Ants!

A prototype for a game where the players have to script the behaviour of individual ants, with the goal being to bring as much food back to the nest as possible. Heavily inspired by [Sebastian Lague's Ant Simulation](https://www.youtube.com/watch?v=X-iSQQgOd1A).

The prototype has **completely seperate** rendering and simulating threads. The simulation thread produces 'deltas', commands that deterministically alter the world. Both threads keep track of their own worlds, allowing the simulation to run continously in the background while the renderer can use the produced 'deltas' to render a pre-simulated world at an arbitrary timepoint. 

While the prototype proved it was technically feasible, the game idea did not pan out. The core gameplay is flawed, a simple script already does really well, and any significant improvements are terribly difficult to come by. This project will be left behind, but the architecture and vision (of having the player create behaviour to be simulated) will carry over to a future project. 

# Getting started

1. Run setup.ps1
2. Open the Ants.sln, select the Ants project to be your startup project.
3. Build and run!

# Simulate millions of Ants

https://github.com/user-attachments/assets/85a4236d-c815-48bf-bf1d-b4e0937210aa

# Ants powered by my Visual Scripting Tool

![AntScript](https://github.com/user-attachments/assets/567b10c8-deb9-4a19-840e-e7cfc6e79bc3)

# Adjust timescale, including reversing!

https://github.com/user-attachments/assets/8156251d-68d5-40d6-ada3-73f7428b5569

https://github.com/user-attachments/assets/c23d1176-7255-4835-9640-d6717ed91251

