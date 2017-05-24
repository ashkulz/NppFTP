# NppFTP
Audible confirmation on file upload. No more will the FTP plugin take up real estate on the screen!

The only thing changed was in the windows/FTPWindow.cpp, on line 1041.

Added:

MessageBeep(MB_OK);
