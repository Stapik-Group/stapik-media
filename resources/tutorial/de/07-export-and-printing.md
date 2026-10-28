# Export und Drucken

## CSV

**Datei → Alle Einträge als CSV exportieren…** speichert alle auf diesem Gerät geladenen Einträge aller Kategorien in einer CSV-Datei. Die Datei lässt sich in Excel korrekt öffnen: Sie ist UTF-8 mit Byte-Order-Mark, und das Trennzeichen richtet sich nach der Sprache des Programms. Daten werden je nach Genauigkeit des Erscheinungsdatums als `2026-03-05`, `2026-03` oder `2026` geschrieben, der Monat des Konsums als `2026-03`.

Einträge früherer Jahre, die nur in der Cloud liegen und noch nicht heruntergeladen wurden, sind nicht enthalten. Wählen Sie zuerst das Jahr im Filter **Jahr**, um es herunterzuladen.

## Import aus CSV

**Datei → Einträge aus CSV importieren…** fügt Einträge aus einer CSV-Datei hinzu. Die Datei sollte dieselben Spalten wie der Export haben: Kategorie, Titel, Erscheinungsdatum, Konsummonat, Regisseur / Autor / Interpret / Studio, Herausgeber, Genre, Plattform, Hörbuch, Bewertung (1–10) und Notiz. Alles nach dem Konsummonat darf leer sein oder fehlen. Die Kategorie ist eine von `movie`, `series`, `cartoon`, `book`, `album` oder `game`, und die Datumsangaben entsprechen dem Export (`2026-03-05`, `2026-03` oder `2026` für das Erscheinungsdatum, `2026-03` für den Konsummonat). Die erste Zeile gilt als Kopfzeile, außer sie beginnt mit einer Kategorie. Das Trennzeichen, Komma oder Semikolon, wird automatisch erkannt.

Bevor etwas gespeichert wird, zeigt eine Vorschau jede Zeile als neu, Duplikat oder ungültig. Duplikate werden wie beim Hinzufügen eines Eintrags erkannt: gleiche Kategorie, gleicher Titel und gleiches Erscheinungsjahr, sowohl in Ihrer Liste als auch innerhalb der Datei. Duplikate, ungültige Zeilen und Einträge früherer Jahre, deren Cloud-Archiv nicht geladen ist, werden übersprungen; alles andere wird nach der Bestätigung hinzugefügt. Der gesamte Import ist ein einzelner Schritt, den **Rückgängig** zurücknimmt.

## Drucken und PDF

**Datei → Liste drucken…** und **Datei → Liste als PDF exportieren…** verwenden die Liste so, wie Sie sie sehen: die gewählte Kategorie mit Jahres-, Monats- und Suchfilter, auf allen Seiten und nicht nur auf der angezeigten.
