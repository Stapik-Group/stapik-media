Stapik Media for Windows
========================

Starting
--------
Unpack the whole folder anywhere (for example to your Documents or a USB stick) and run stapikmedia.exe.
There is no installer and nothing is written outside your user profile. Keep the files of the folder
together: the program needs the DLLs and the resources next to it.

Windows SmartScreen may warn about an unknown publisher the first time. Choose "More info" and
"Run anyway".

Your data
---------
Everything is stored per user and survives updates and deleting the program folder:

  %APPDATA%\stapikmedia\data      media.json (current year), media-YYYY.json (older years), backups, cloud connection
  %APPDATA%\stapikmedia\config    settings.json (language, theme, window size)
  %LOCALAPPDATA%\stapikmedia\cache\stapik.log   log file, useful when reporting a problem

Updating
--------
Unpack the new version over the old folder or into a new one. Your data stays where it is.

Removing
--------
Delete the program folder. To remove the data as well, delete the folders listed above.

Cloud sync
----------
File -> Connect connects the program to your Stapik Cloud server. Plain HTTP is accepted but not secure.
The program uses the Windows certificate store for HTTPS.

More: https://github.com/Stapik-Group/stapik-media
