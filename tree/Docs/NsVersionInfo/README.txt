libVerInfo
----------
This is free code, released under the MIT license with the hope that
it could be useful to someone else. There is absolutely NO WARRANTY.
Please read the license.txt for details.

This code is an adaptation of a project by Denis Zabavchik in MFC to GCC.
See http://www.codeproject.com/KB/library/VerInfoLib.aspx

Remarks, comments, questions and bug reports are welcome : drop me a
message at sebastien.kirche@free.fr

To the original code, I have added the possibility to correctly modify the 
VersionInfo resources of Powerbuilder compiled exe's and dll's. 

Basically these files has an overlay at the end that contain the PB
bytecode in a kind of linked list data. When modifying the resources
1) the Windows API destroys the overlay part and need to be restored
after the modification, and 2) if the modified info resource is
smaller or bigger that the original one, the data inside of the
overlay may need some 'realignment'. That is done by the libVerInfo if
needed.


How to compile :
--------------
The only compiler that I support myself is GCC (that is from MinGW,
though it should be compilable with MSVC).

To get the debug version (including runtime debug messages via
OutputDebugString()), use a make BUILD=debug

To get the release version (optimized, without debug symbols) use a
make BUILD=release

