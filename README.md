# ViBrowserBB10 (abandoned)

These are the remnants of a project I started to create a web browser for
BlackBerryOS 10 that behaves like Firefox with Vimperator, and could be
controlled with the physical qwerty keyboard that came with devices like the
Classic, Passport, etc.

I stopped working on this when there were rumors that BlackBerry was switching
to Android for future devices. Since it's obvious they were abandoning the
platform, I thought it pointless to continue development.

It's unfortunate, because I still think phones with physical keyboards rock and
I'm sad that a keyboard-centric web browser never surfaced for the platform.

### Status

Note that I haven't touched this code in years, so everything here is based off
of hazy memory:

From what I remember, basic scrolling with hjkl, a "hints" mode, a js
"console", page zooming, navigation history/bookmarks, and a few other basic
features were working OK for the most part.

Hints mode was very glitchy due to what I think was a bug in the qtWebView,
where the viewport would report incorrect screen geometry so not all links on
the screen would be highlighted when the 'f' key was pressed.

I think I started to implement some basic CLI commands (like key remapping),
but I don't think those were ever finished.
