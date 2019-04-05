# NppExec (plugin for Notepad++)

Here is some brief info about NppExec for new users:
* NppExec adds a Console window at the bottom of Notepad++'s window;
This Console window allows to run different console programs directly in it;
Also there is Plugins -> NppExec -> Execute... window that allows to execute NppExec's scripts (set of commands).
* NppExec's commands and variables are listed in the "NppExec.txt" file.
* To get more details, type "help" (without the quotes) directly in NppExec's Console. To get a detailed help about any NppExec's command, type "help [command]" - e.g. "help npp_open" or "help set".
And don't forget about Plugins -> NppExec -> Help/Manual.

Installation:
* Just unpack the NppExec zip-archive (the last \*dll\*.zip under either https://github.com/d0vgan/nppexec/releases or https://sourceforge.net/projects/npp-plugins/files/NppExec/) to the folder "Notepad++\plugins".
It will extract the file "NppExec.dll" as well as the "doc\NppExec" and "NppExec" subfolders (the latter one contains header files used by NppExec at runtime).
* The only difference between the \*dll\*.zip and \*dll\*_PA.zip is the plugin folder structure. The \*dll\*.zip corresponds to the plugin folder structure prior to Notepad++ 7.6.x, whereas the \*dll\*_PA.zip follows the new plugin folder structure introduced in Notepad++ 7.6.x together with the Plugins Admin.

Note:
If you see just blank pages inside the NppExec Manual or see messages similar to "This web content was blocked" or "This web page could not be shown", please follow the instructions below:
* Unblocking CHM files for Windows XP : http://support.microsoft.com/kb/902225
* Unblocking CHM files for Windows 7 : http://support.microsoft.com/kb/2021383
