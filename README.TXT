
================================================================================
     tsf-vim version 0.1.1 (2014-03-23)

     https://github.com/deton/tsf-vim
     deton@m1.interq.or.jp
================================================================================

  Windows IME to edit text like the Vi editor.


--------------------------------------------------------------------------------
     Install
--------------------------------------------------------------------------------


* Required Library

  Install x64 and x86 packages for 64bit Windows.

  Microsoft Visual C++ 2010 SP1 Redistributable Package (x86)
  http://www.microsoft.com/en-us/download/details.aspx?id=8328

  Microsoft Visual C++ 2010 SP1 Redistributable Package (x64)
  http://www.microsoft.com/en-us/download/details.aspx?id=13523


--------------------------------------------------------------------------------
     Vi Commands
--------------------------------------------------------------------------------

  tsf-vim sends some keys on each Vi command.
    Example: When user presses 'h' key, tsf-vim sends VK_LEFT to application.

  tsf-vim acquires text using TSF(Text Services Framework) for
  f,t,F,T,),(,w,e,b,W,E,B commands.
  If application does not support TSF, tsf-vim uses IMR_DOCUMENTFEED to
  acquire text.
  If application does not support IMR_DOCUMENTFEED,
  these commands does not work.

  Supported applications:
  + Word 2010
  + Outlook 2010
  + (f etc. does not works on PowerPoint 2010, Excel 2010, Visual Studio 2010/2012)
  + WordPad
  + notepad

Command  Key sequences to send
0       HOME
g$      END (count not supported)
$       to newline(*) RIGHT (count not supported)
c       CTRL-X,tsf-vim OFF
d       CTRL-X
y       CTRL-C
cc      [count-1]DOWN,END,RIGHT,start selection,[count-1]UP,CTRL-X,tsf-vim OFF
cj      [count]DOWN,END,RIGHT,start selection,[count]UP,CTRL-X,tsf-vim OFF
ck      END,RIGHT,start selection,[count]UP,CTRL-X,tsf-vim OFF
dd      [count-1]DOWN,END,RIGHT,start selection,[count-1]UP,CTRL-X
dj      [count]DOWN,END,RIGHT,start selection,[count]UP,CTRL-X
dk      END,RIGHT,start selection,[count]UP,CTRL-X
yy      [count-1]DOWN,END,RIGHT,start selection,[count-1]UP,CTRL-C
yj      [count]DOWN,END,RIGHT,start selection,[count]UP,CTRL-C
yk      END,RIGHT,start selection,[count]UP,CTRL-C
C       start selection,END,CTRL-X,tsf-vim OFF
D       start selection,END,CTRL-X
f{char} to {char}(*)RIGHT
t{char} before {char}(*)RIGHT
F{char} to {char}(*)LEFT
T{char} after {char}(*)LEFT
;       repeat latest f, t, F or T
,       repeat latest f, t, F or T in opposite direction
r{char} DELETE,{char}
s       start selection,RIGHT,CTRL-X,tsf-vim OFF
gg      CTRL-HOME
G       CTRL-END,HOME (same as gg when count is specified)
CTRL-F  PageDown
CTRL-B  PageUp
h       LEFT
l       RIGHT
<Space> RIGHT
j       DOWN
k       UP
+       next line and to first non-blank character(*)RIGHT
CTRL-M  next line and to first non-blank character(*)RIGHT
i       tsf-vim OFF
I       to first non-blank character(*)RIGHT or LEFT,tsf-vim OFF
a       RIGHT,tsf-vim OFF
A       END,tsf-vim OFF
o       END,RETURN,tsf-vim OFF
O       HOME,RETURN,UP,tsf-vim OFF
p       RIGHT,CTRL-V
P       CTRL-V
u       CTRL-Z
x       start selection,RIGHT,CTRL-X
X       start selection,LEFT,CTRL-X
w       to next word(*)RIGHT
W       to next WORD(*)RIGHT
e       to the end of word(*)RIGHT
E       to the end of WORD(*)RIGHT
b       to previous word(*)LEFT
B       to previous WORD(*)LEFT
)       to next sentence(*)RIGHT
(       to previous sentence(*)LEFT
J       to the end of line(*)RIGHT,
        to first non-blank character on next line(*)DELETE
        (count not supported)

  You can combine c,d,y and motion commands(h,f{char},w,$,etc.).
  Some commands support count.
  Commands marked (*) do not work for application which does not support TSF.


--------------------------------------------------------------------------------
     Misc
--------------------------------------------------------------------------------


* Source

 * IME functions from CorvusSKK.
    http://code.google.com/p/corvus-skk/
 * Surrounding text functions to acquire text around cursor position by TSF
   from Mozc.
    http://code.google.com/p/mozc/

* BUGS

 * 'dd' on last line deletes last line and upper line.
 * 'yy' on last line yanks last line and upper line.
 * 'cc' on last line changes last line and upper line.
 * Many unsupported commands(includes text-objects, Visual mode).

* Development tools

  Visual C++ 2010 Express + SP1
  Windows SDK 7.1
  Windows Driver Kit 7.1.0 (for ATL used by Mozc)
  WiX 3.8
