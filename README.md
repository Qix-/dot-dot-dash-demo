# Demo for Dot-Dot-Dash

Inspired by the old Apple iPod silhouette (by Susan Alinsangan) campaign.

![silhouette girl](http://www.onewomanmarketing.com/wp-content/uploads/2009/08/090803-AppleExample.jpg)<br>
<sup>Apple iPod Silhouette Campaign</sup>

There are a lot of ways this could be achieved, and this definitely isn't
intended to look *exactly* like the campaign.

---

This demo shows a silhouette of one or more persons in front of a camera.
It is intended to be played with music, which is analyzed to detect "bumps"
in the music - at which points the background changes to another color.

The path algorithm uses contour detection via OpenCV, feeds the vertices into
a simplification algorithm (which reduces the number of points while keeping
the best shape possible), and then re-connecting them using curves before
drawing them on the screen.

The sound component uses Fast Fourier Transform (FFT) to deduce 27 bands of
audio, and allows the configurator to specify one or more of the first 18 bands
to be averaged and turned into a net peak. This peak is compared against a
threshold, which is then processed by a debounce algorithm, in order to detect
"bumps". These bumps trigger the background to change to another "friendly"
color.

The colors used are simply different hue shifts with a slightly desaturated
base color.

---

There are many ways this demo could have been achieved. My initial goal was to
train a Haar detector to detect ears (as I couldn't find any existing cascades)
and then use an adjacent upper body detector to add an overlay of the Apple
earbuds and an iPod attached to the "belt" area - similar to the original
campaign. Unfortunately, training a haar cascade is tedious and takes lots and
lots of data - something I couldn't do in a single day, as originally planned.

The other improvement to this demo would be to change how the analysis works for
bump detection. Ideally, tempo detection would produce the most consistent
results, coupled with peak detection would get song phrasing to look nice
(instead of "bumping" even in quiet parts of the song).

Ultimately I envisioned this to be used on a sidewalk-type area where
pedestrians would be able to passively interact with it. This means, using
four haar detectors (ear/upper body, frontal and profile), passerbys would
see themselves with the earbuds - without having to do much more than just 'be
there'.

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
The video portion of this demo uses contour detection of video streamed from
the default camera on the system.

> Mac cameras have built in light compensation which is handled at the hardware
> level, making any sort of OpenCV stuff incredibly annoying. If you're running
> into this problem with the background diffing, then you'll probably need more
> light, or need to step back further.

The detected blobs are simplified, and then curved. Use the debugging controls
to help fine-tune the settings (`d`).

- `d` - Toggle the video debug view
- `[space]` - Learn background (should be used with nothing in the shot)
- `h` - Show [holes](http://openframeworks.cc/documentation/ofxOpenCv/ofxCvContourFinder.html#!show_findContours) (off by default)
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
