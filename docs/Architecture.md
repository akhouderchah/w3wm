# Overall Structure
On 32-bit Windows, w3wm consists of a main application and a DLL containing various hooks
(the most important one being the keyboard hook).
The main application utilizes SetWindowsHookEx to inject the DLL into the process space of all running applications.
This allows w3wm to receive keyboard input before it reaches the focused window, such that w3wm may react to hotkeys
in an appropriate manner.

> #### Wait, doesn't Windows provide a function to deal with hotkeys?
> Those with some familiarity with the Win32 API may be reminded of the RegisterHotKey function, which allows
> a program to do roughly what the DLL injection here achieves. However, RegisterHotKey has two fundamental problems
> for the case of a general-purpose window manager:
> 1. RegisterHotKey may NOT override an existing hotkey, and the corresponding UnregisterHotKey may only unregister
> hotkeys that were registered by the caller. So if a program registers Ctrl+P as a hotkey, a window manager would
> be unable to use Ctrl+P as a hotkey when that program is in focus.
> 2. The above point is also true for Windows hotkeys. In particular, Windows reserves a number of Win+\<KEY> hotkeys
> for Windows shortcuts (think Win+L). This is especially unfortunate, as w3wm--by default--uses Win+\<KEY> hotkeys rather
> extensively.
>
> The solution of using global keyboard hooks through DLL injection is not perfect (see
> [Issue #3](https://github.com/Khouderchah-Alex/w3wm/issues/3)). It is, however, far preferable to the use
> of RegisterHotKey, which is entirely unsuitable for the purposes of a customizable window manager.

The main w3wm application provides the DLL with an array HotkeyDefs, informing the DLL of which hotkeys to
listen for and what message to send w3wm when a hotkey occurs. The DLL communicates with the main w3wm
application with SendMessage when a relevant hotkey is pressed.

Thus a high-level layout of w3wm running on a 32-bit machine looks like this:

    +---------------------+
    |        Hook         |
    |        DLL          |
    +---------------------+
            .     .
           / \    |
    Hotkey  |     | Hotkey
    Defs    |     | Events
            |    \ /
            .     .
    +---------------------+
    |        w3wm         |  
    |     Application     |
    +---------------------+
    
The situation is slightly more complex for 64-bit Windows. A 64-bit Windows system is likely to have both 32-bit and 64-bit
code executing on it. However, 32-bit code cannot be injected into a 64-bit program, and 64-bit code cannot be injected
into a 32-bit program. Thus two sets of DLLs--a 32-bit DLL and a 64-bit DLL--are needed. Since each DLL needs a program to
be pumping the Windows message loop, we also need two w3wm applications. It would be quite a shame, however, to have two
separate programs doing the job of window management that is done by one program on 32-bit Windows.

The solution in this case is to have one of the programs act as a "stub", simply pumping the Windows message loop
and forwarding hotkey events to the other program, which acts as the main w3wm program. Since w3wm always requires
a 32-bit program, it made sense for this to be the main application on both 32-bit and 64-bit systems. Note that since
a well-made window manager should not be using excessive system resources, the memory limitations of 32-bit applications
should not be an issue here. When the 32-bit application starts up, it determines whether it is running on 
a 32-bit or 64-bit system. In the case of a 32-bit system, it will create the 64-bit stub, which handles 
setting up the 64-bit DLL on its own.

The high-level layout of w3wm running on a 64-bit machine, then, looks like this:

    +---------------------+                       +---------------------+
    |     32-bit Hook     |                       |     64-bit Hook     |
    |         DLL         |                       |         DLL         |
    +---------------------+                       +---------------------+
            .     .                                       .     .
           / \    |                                      / \    |
    Hotkey  |     | Hotkey                         Hotkey |     | Hotkey
    Defs    |     | Events                         Defs   |     | Events
            |    \ /                                      |    \ /
            .     .                                       .     .
    +---------------------+     Hotkey Defs       +---------------------+
    |     32-bit w3wm     |  ------------------>  |     64-bit w3wm     |
    |     Application     | <-----------------    |        Stub         |
    +---------------------+    Hotkey Events      +---------------------+
    
