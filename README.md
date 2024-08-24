# Summary
This is a photo gallery written using raylib as the API layer for reading in images; it also utilising the transitions functions from a scene handler.  It has been written for Raspberry Pi boards running in a framebuffer primarily (ie. not in a windowed environment like X11) but should run on pretty much anything.

## Dependencies
- raylib (Tested with 4.2 and 5.x)
- scene_handler

### scene_handler
This is a set of source files written as a scene handler for apps using raylib; this library is only using the transition handling code.
https://github.com/coochewgames/scene_handler

## Building
There has been some compilation shell scripts created to allow this to be built on different platforms.
- `cc_linux.sh`
- `cc_pi.sh` - To be utilised on Buster
- `cc_pi_drm.sh` - To be utilised on Bullseye or later

## Directory expectations
Everything is expected to be within the same relative source directory:
```
./scene_handler/transition_handler.*
./raylib/src/raylib.a
./photo_gallery/*
```

# INI File
The options to change the functionality of the photo gallery are set within a `photo_gallery.ini` file.

## Display Type
The Display Type affects how the images are displayed and can be one of:
- NO_SCALE
- NO_SCALE_CENTRE
- NO_SCALE_CENTRE_X
- NO_SCALE_CENTRE_Y
- STRETCH
- FIXED_RATIO

### Example
```
display_type=FIXED_RATIO
```

### Notes
The images are rendered as a texture and therefore impacted by the OpenGL texture limits; this is 2048 on a Pi (tested on a Pi Zero, a Pi Zero 2 and a Pi 3B+).

So if you are using NO_SCALE, then the images have to be lower than this max texture limit.  Running on a Pi Zero, images can be dislayed at 1920x1080 but 4000x3000 will present a blank screen.

## Width and Height
The display width and height that the images are to be displayed at are set here.

### Example
```
display_width=720
display_height=450
```

## Display Time
The seconds of duration that the photo is to be displayed for.

### Example
```
display_time=10
```

### Notes
This time is inclusive of the transition time.  Importantly, the processing time required to scale and image to the display height and width is CPU intensive and can affect the transitioning if this is time is too short.

On a Pi Zero, an image of 4000x3000 works best when there is at least 40 seconds of display time if the transitions are to be seen properly.  Using a Pi Zero 2, it can complete in around 15 seconds.

## Transition duration
This is the transition duration, which is inclusive of the display time.

### Example
```
transition_duration=5
```

## Get Type
This allows the photo selection to be random or sequential.  The options available are:
- RANDOM
- SEQUENTIAL

### Example
```
get_type=RANDOM
```

## Initial Directory
This is initial directory to search for photos; sub-directories will be recursively searched for image files.
- JPG
- PNG
- BMP

### Example
```
initial_dir=./photos
```

### Notes
The raylib build used will have to ensure that JPG file line have been uncommented in the `config.h` file.  If using an earlier version of raylib (for example, 4.2), the case of the filename extension can entail that the image data will not be properly processed.

# TODO
Saving the image files found in the scanned directories to improve the starting up time.
