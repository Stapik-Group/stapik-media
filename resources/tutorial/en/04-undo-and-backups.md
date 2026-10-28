# Undo and backups

## Undo and redo

The **Edit** menu has **Undo** and **Redo** for adding, editing and deleting entries (Ctrl+Z and Ctrl+Shift+Z). The history is cleared when a newer version of your data arrives from the cloud or when you restore a backup.

## Backups

The program keeps copies of the list of the current year next to the data file: at most one copy every five minutes, the five newest are kept.

To go back to one of them choose **File → Restore from backup…** and pick a copy by its date. The entries of the current year are replaced by the copy; earlier years are not changed.

If a file cannot be saved, the program says so and tells you why, and your changes stay in the open window.

## Backup of the whole application

**File → Back up the whole application…** saves all entries of all years that are loaded on this device, together with the settings of the program, to one `.zip` file. Entries of earlier years that are kept only in the cloud and have not been downloaded yet are not included.

**File → Restore the whole application…** replaces the entries and the settings with the ones from such a file, after you confirm. Years that are not in the file end up empty, and the history of **Undo** is cleared. When the program is connected to the cloud, the restored entries are synchronized like any other change. A file that belongs to another program or was written by a newer version is refused and nothing is changed.
