# PebbleFractal
Enjoy recursive artwork created from the angle of the minute and hour hands at that moment!

Contains many settings, including customizable colors, variable lengths and recursion factor, different font choice, and more.

How does it work?
The hands are redrawn at the tip of the previous layer's hands, but scaled down slightly. Angles that deviate from "noon" are carried through all recursion steps. This is repeated until the hands would be too small to draw again.

How's the battery life?
Very good. I can easily get my Pebble Time 2 to last for over two weeks. The screen updates once every 10 seconds, and there's a short 2-second growth animation when you switch to the face. I spent a great deal of time optimizing it to make sure it doesn't do any unnecessary work, so you should be able to rest easy!

Did you use AI to create this?
All of the core C code was written solely by myself because I like coding, and I wanted to maximize the battery life and maintain my artistic vision. I did use AI to explain some parts of the documentation (it could really use some examples) and to help write the shell scripts that make the gifs.

How does the date move around?
Disclaimer: I'm proud of this so I just wanted to yap about it. The program maintains a bit matrix (32 x int32s on newer hardware) that corresponds to regions on the screen. When fractal lines are drawn visually, they also fill in the bit matrix using Bresenham's line algorithm (or just two direct points if short enough). Then it iterates through the bit matrix and calculates the largest unobstructed rectangular area, and finally attempts to fit the date inside of it, using the built-in text layer line wrapping functionality for narrow rectangles.

Inspired by CodeParade's Fractal Clock: https://youtu.be/4SH_-YhN15A
