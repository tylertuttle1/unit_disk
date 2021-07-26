# Building

on windows, run `build.bat`. on linux, run `build.sh`. both scripts are set up to
run the program once it's built (for testing), so if you don't want that you'll have
to edit the scripts. you have to run them from the base directory.

on linux:

    git clone https://www.github.com/tylertuttle1/unit_disk.git
    cd unit_disk
    ./build.sh

on windows:

    git clone https://www.github.com/tylertuttle1/unit_disk.git
    cd unit_disk
    build

the windows build script will also open the images after it runs the program.

# Controlling the output

the output of the program right now is controlled via some global variables in the
file `start.cpp`. the variables `hgap`, `vgap`, and `pad` control the drawing of the
centroid tree. the `POINT_COUNT` and `scale` variables control the random generation
of the points. the points are generated uniformly at random from a square with side
lengths given by the `scale` variable.
