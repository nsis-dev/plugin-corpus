This is some quick and dirty code to implement an extra license file page.

Use Like:

ExLicensePage::showPage "license.txt"

The program is not really customizable at this point, if you like it and want a few extra features, let me know and I'll flesh it out more.
ExLicensePage expects to find the file "license.txt" in the same directory the dll is located. (Temp Dir usually).
ExLicensePage also implements cancel strangely, due to the fact that I cannot figure out a proper way of staying on the current page if cancel is pressed along with a no, it sends a message to the parent window forcing it back to the previous page.

I'd love to see some functions in NSIS to control GoBack, GoNext, GoCancel.

-Pkp
pkp@pkpsoft.com
