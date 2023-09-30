# a7-sprite-editor-s23-darcusmao
a7-sprite-editor-s23-darcusmao created by GitHub Classroom
# A7 Sprite Editor

by Tigers

A Sprite Editor application written in Qt6 for CS3505 A7-Sprite Editor.

## Requirements

The application must be able to:

* Set the size in pixels of the sprite. This can be bounded by a minimum, maximum, and by jumps in size if desired (historically, sprites rendered as textures on simple geometry were most efficient for graphics cards when sized at powers of 2). Or it can be more continuous. Keep in mind that you are not making a paint program, so if you are drawing on 500x500 images, you are probably not enough sprite editor and are aimed too much at being a paint program.
* Adjust the number of frames for the sprite animation - this can be incremental, like "add a frame" and "delete a frame".
* Allow the user to modify the pixels of a sprite. The most basic form is clicking on pixels. You should also allow the user to draw while moving the mouse.
* Provide a preview of the sprite animation cycle.
* The user should be able to adjust the frames per second of playback.
* There should be an option of seeing the playback at the actual size of the sprite (even when super tiny).
* Save and load a project. All values in the project file should be represented by a json data structure (Links to an external site.). The json data structure should have the following fields:
  * height with an integer value
  * width with an integer value
  * numberOfFrames with an integer value
  * frames with an array of frames, with each frame being a field with a name framen, where n is the frame number starting at 0. Each frame is an array of rows from top to bottom and each row being an array of pixels from left to right and each pixel being an array of [r, g, b, a] 0-255 integer values. A json file does not need to preserve ordering (it is more of a map structure) or newlines, so your file may differ from the example that will be provided at the start of the implementation phase.
* Use a sprite sheet project .ssp extension when saving. You should be able to add additional fields to this format if you have fancy features that need additional information to be saved but you need to be able to read and save this basic format. You can reject files that do not meet your constraints on sprite images (for example, reject non-square if that is a constraint of yours). 

## Additional Features

The application will also allow for some additional tools:

* Fill - Will fill all pixels that are of the same color as the pixel until it is blocked off by pixels of different colors
* Fill All - Will fill all pixels that are of the same color in the entire frame
* Shapes - Allows the user to draw circle's, rectangle's, and square's with minimal effort
* Mirror - Allows for symmetrical drawing on the frame, Mirror will automatically toggle off if the user decides to fill, fill all, or load/create a new sprite

## Other things of notice
* Canvas is set to fixed sizes of 16x16, 32x32, 64x64, 128x128, and 256x256, the user can freely choose which to use when creating a new sprite, but starts out in a default 32x32 size when first opening the application
* Eyedropper tool only functions inside the canvas and you can't replicate colors outside the canvas
* Cursor image changes according to the tool selected
