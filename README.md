# idle-slayer-autoclicker
This project is a simple but effective autoclicker for Idle Slayer on Linux.
It utilizes the XLib library to click only when the cursor is above the window.

*This is my first C project, so my code may suck!*

## Running
Download the executable file in the releases tab and execute it with 1 argument that specifies the CPS. \
For example: './main 16'


*This is only tested on Fedora Linux. When the path to your Xlib differs, you may have to rebuild the executable.*

## Building
1. Clone the project
2. Run 'gcc main.c -o main -L/usr/include/X11/ -lX11' and replace /usr/include/X11/ with the path where your X11 library lies

### Support & Feedback
- Join my [Discord](https://nyon.dev/discord) for help or suggestions.