; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
ClassCount=3
Class1=CExLicensePageApp
LastClass=LicensePage
NewFileInclude2=#include "ExLicensePage.h"
ResourceCount=1
NewFileInclude1=#include "stdafx.h"
Class2=LicensePage
LastTemplate=generic CWnd
Class3=SubclassWnd
Resource1=IDD_LICENSEPAGE

[CLS:CExLicensePageApp]
Type=0
HeaderFile=ExLicensePage.h
ImplementationFile=ExLicensePage.cpp
Filter=N

[DLG:IDD_LICENSEPAGE]
Type=1
Class=LicensePage
ControlCount=3
Control1=IDC_OUTPUT,edit,1352730692
Control2=IDC_HEADER,static,1342308352
Control3=IDC_FOOTER,static,1342308352

[CLS:LicensePage]
Type=0
HeaderFile=LicensePage.h
ImplementationFile=LicensePage.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=IDC_OUTPUT

[CLS:SubclassWnd]
Type=0
HeaderFile=SubclassWnd.h
ImplementationFile=SubclassWnd.cpp
BaseClass=CWnd
Filter=W
VirtualFilter=WC

