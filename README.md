# w3wm

### What is it?
![alt text](https://github.com/Khouderchah-Alex/w3wm/blob/master/docs/w3wm_Intro.gif "Intro GIF")

w3wm is a tiling window manger for Windows, similar to i3 on Linux.
It keeps your windows organized and allows you to use keyboard shortcuts in order to:
* move between windows
* move the windows themselves
* fullscreen the currently-selected window
* close the currently-selected window
* open new windows

w3wm is a modern desktop manager.
It was built from the ground-up to support multiple monitors and monitors with varying DPIs.
Simply start the program and enjoy a Windows experience with less mouse usage than ever before.

### How do I use it?
w3wm can be run at any time, whether it be at startup or at some time after startup.
Simply run the `w3wm.exe` executable (it is highly recommended to run as admin; most functionality will work even without administrator privileges, but certain inconveniences may exist, such as Win+L locking the screen even when Win+L is assigned to a different function).
w3wm will detect all your monitors and tile existing windows between those monitors.
When w3wm is running, there will be an icon in the notification area (the right-side of the toolbar), such that there is never any ambiguity over whether or not w3wm is running.

##### Default hotkeys
Users can fully customize the keyboard combinations used to control w3wm, but the default hotkeys are provided below.

Move Focus | Keyboard Combination
--- | ---
up | Win+K
down | Win+J
right | Win+L
left | Win+H

Move Window | Keyboard Combination
--- | ---
up | Win+Shift+K
down | Win+Shift+J
right | Win+Shift+L
left | Win+Shift+H

w3wm Function | Keyboard Combination
--- | ---
Fullscreen current window | Win+F
Close current window | Win+D
Open new window | Win+N
Open new console | Win+Enter
Lock screen | Win+Home
Restart w3wm | Win+R
Quit w3wm | Win+Q

##### Hotkey Customization
In the same directory as `w3wm.exe`, there is a file called `config.ini`.
This file enables the user to, among other things, customize the hotkeys used above.

The default `config.ini`--which w3wm will automatically generate if the configuration file was deleted--describes the syntax used and provides usage examples and function/key names.
Note that w3wm must be restarted in order for configuration file changes to be applied.
The default hotkey to restart w3wm is Win+R.