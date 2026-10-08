Dumpstate is a debug utility for the NSIS installer scripting system.
Dumpstate has only been tested with NSIS version 2. I can't think
of any reason that it wouldn't work under NSISv1, but you'll have
to try it for yourself.

I'd appreciate any positive or negative feedback you might have
regarding dumpstate. My email address can be found at my homepage:

  http://www.sullust.net/


v0.3.1 (anders_k)
=================

F2 keyboard shortcut triggers item editing in lists
Added exec_flags tooltips

v0.3.0 (anders_k)
=================

Added exec_flags dialog
Added fillregisters function

v0.2.1 (anders_k)
=================

Unicode & 64-bit support

v0.2.0
======

Fixes a bug in 0.1, which would cause strings longer than about 1000 characters to be truncated

======
v0.1.0
======

Initial version


Using Dumpstate
===============

Place dumpstate.dll into your NSIS plugins directory (under the
directory you installed NSIS into), and call it as follows inside
your script:

	Dumpstate::debug

Dumpstate brings up a dialog which allows you to view and alter
variables and the stack. The Up, Down, Add and Del buttons add,
delete and move elements around in the stack. You can also double
click on any variable or stack element to edit it's value.


Building Dumpstate from source
==============================

There's a Makefile for Visual Studio's nmake in the src directory. 
Edit the TARGETDIR macro to point to your NSIS plugins directory
(on my system it's C:\Program Files\NSISv2\Plugins) and run
"nmake" in the src directory to compile.

I haven't tried to compile it using anything other than Visual
C++. Please drop me a line if you do.


Contact, website
================

My name is Andrew Francis. My website is at <http://www.sullust.net/>.
I don't splash my email address around in plaintext, to avoid 
spambot harvesting. My email address is visible on my website.

Dumpstate lives at <http://www.sullust.net/sw/dumpstate/>, and any
subsequent releases will appear there.


License information
===================

Copyright (c) 2003, Andrew Francis.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

- Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer. 

- Redistributions in binary form must reproduce the above copyright 
  notice, this list of conditions and the following disclaimer in 
  the documentation and/or other materials provided with the 
  distribution. 

- The name of Andrew Francis may not be used to endorse or promote
  products derived from this software without specific prior
  written permission. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,	
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

Portions of this software are from the NSIS <http://nsis.sourceforge.net/>
distribution, which carries the following license:


Copyright (C) 1999-2002 Nullsoft, Inc.

 This software is provided 'as-is', without any express or implied
 warranty.  In no event will the authors be held liable for any damages
 arising from the use of this software.

 Permission is granted to anyone to use this software for any purpose,
 including commercial applications, and to alter it and redistribute it
 freely, subject to the following restrictions:

 1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.
 2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.
 3. This notice may not be removed or altered from any source distribution.


