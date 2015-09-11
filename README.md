# Demo for Dot-Dot-Dash

Inspired by the old Apple iPod silhouette (by Susan Alinsangan) campaign.

![silhouette girl](http://www.onewomanmarketing.com/wp-content/uploads/2009/08/090803-AppleExample.jpg)<br>
<sup>Apple iPod Silhouette Campaign</sup>

There are a lot of ways this could be achieved, and this definitely isn't
intended to look *exactly* like the campaign.

---

# Building / Installing
Clone into `/path/to/open-framework/apps` and then run the `projectGenerator` of
your choice inside the cloned repository. Build, and run.

This shouldn't be any different than other apps/examples.

# Controls
There are quite a few controls and two different debug views.

### General Controls
Along with the keyboard shortcuts below, you may also drag/drop an audio file
(music) directly onto the demo; it will start immediately, allowing you to
play with the sound settings.

- `f` - Toggle fullscreen
- `c`/`v` - Decrease/Increase the "frame delay" effect (default off)

#### Video Debug
The video portion of this video uses contour detection of video streamed from
the default webcam on the system.

The detected blobs are simplified, and then curved. Use the debugging controls
to help fine-tune the settings (`d`).

- `d` - Toggle the video debug view
- `[space]` - Learn background (should be used with nothing in the shot)
- `h` - Show [holes](http://openframeworks.cc/documentation/ofxOpenCv/ofxCvContourFinder.html#!show_findContours)
- `,`/`.` - Decrease/Increase the simplification factor on paths
- `[`/`]` - Decrease/Increase the background subtraction factor
- `n`/`m` - Decrease/Increase the minimum blob size (in pixels)
- `N`/`M` - Decrease/increase the maximum blob size (in pixels)
> The blob size settings don't make too much of a difference.

#### Sound Debug
The audio portion of this demo allows the user to drag/drop a music file onto
the application and perform FFT/peak average analysis on the playing music
to change the color of the background.

- `s` - Toggle the sound debug view
- `z`/`x` - Decrease/Increase the "bump" threshold. Hold Shift to fine-tune.
- `q`/`w` - Decrease/Increase the dither (interpolation) of the sampled peaks.
> This setting will *drastically* improve the ability to accurately refine
> the "bumps".
- `-`/`=` (`+`) - Decrease/Increase the "cooldown count". This is the number of
  sample updates before allowing another bump to occur after the previous bump.
  For instance, if this is at `5`, then every bump will cause a forced timeout
  of five more samples before another bump can occur.
- [`Shift`] `0`-`9` - Toggles a band to be included in the "bump" average.
  The first 18 bands can be toggled, the first nine with `0` - `9` and the
  second nine with `Shift + 0` - `Shift + 9`.
> I recommend against using the very first band, as I don't thing the underlying
  FFT algorithms perform a 20hz cut, causing that band to clip almost all of the
  time.
