Transform static labels and listview subitem into links.

Link function
~~~~~~~~~~~~~

1. 	Transforms a label into a link.

	Must always be called with /NOUNLOAD to prevent a crash.
	
	Takes HWND and URL. For example:
	
	Linker::link /NOUNLOAD $HWND "http://www.google.com/"
	
2. 	Transforms a subitem in a listview into a link.

	Must always be called with /NOUNLOAD to prevent a crash.
	
	Takes HWND , item , subitem and URL. For example:
	
	Linker::lvlink /NOUNLOAD ${hwndLV_} 2 3 "http://www.google.com/"

	(It supports 40 items and 10 subutems each: MAX_LV_ITEMS 40 , MAX_LV_SUBITEMS 10)

Unload function
~~~~~~~~~~~~~~~

Dummy function that allows unloading the DLL so it won't be left
over in the plug-ins directory. Normally called from .onGUIEnd.

Function .onGUIEnd
	Linker::unload
FunctionEnd
