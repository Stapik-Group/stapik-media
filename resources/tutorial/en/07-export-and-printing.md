# Export and printing

## CSV

**File → Export all entries to CSV…** saves every entry that is loaded on this device, from all categories, to a CSV file. The file opens correctly in Excel: it is UTF-8 with a byte order mark, and the separator follows the language of the program. Dates are written as `2026-03-05`, `2026-03` or `2026`, depending on the precision of the release date, and the month of consumption as `2026-03`.

Entries of earlier years that are kept only in the cloud and have not been downloaded yet are not included. Choose the year in the **Year** filter first to download it.

## Import from CSV

**File → Import entries from CSV…** adds entries from a CSV file. The file should have the same columns as the export: category, title, release date, month of consumption, director / author / performer / studio, publisher, genre, platform and audiobook. The category is one of `movie`, `series`, `cartoon`, `book`, `album` or `game`, and the dates are written as in the export (`2026-03-05`, `2026-03` or `2026` for the release date, `2026-03` for the month of consumption). The first row is treated as a header unless it starts with a category. The separator, a comma or a semicolon, is recognized automatically.

Before anything is saved, a preview lists every row as new, duplicate or invalid. Duplicates are recognized the same way as when you add an entry: the same category, title and release year, both in your list and repeated inside the file. Duplicates, invalid rows and entries of earlier years whose cloud archive has not been downloaded are skipped; everything else is added after you confirm. The whole import is a single step that **Undo** reverts.

## Printing and PDF

**File → Print the list…** and **File → Export the list to PDF…** use the list as you see it: the selected category with the year, month and search filter, on every page, not only the page that is shown.
