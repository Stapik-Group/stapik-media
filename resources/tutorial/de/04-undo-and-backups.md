# Rückgängig und Sicherungen

## Rückgängig und Wiederholen

Das Menü **Bearbeiten** bietet **Rückgängig** und **Wiederholen** für das Hinzufügen, Bearbeiten und Löschen von Einträgen (Strg+Z und Strg+Umschalt+Z). Der Verlauf wird geleert, wenn eine neuere Version Ihrer Daten aus der Cloud eintrifft oder wenn Sie eine Sicherung wiederherstellen.

## Sicherungen

Das Programm legt Kopien der Liste des laufenden Jahres neben der Datendatei ab: höchstens eine Kopie alle fünf Minuten, die fünf neuesten bleiben erhalten.

Um zu einer davon zurückzukehren, wählen Sie **Datei → Aus Sicherung wiederherstellen…** und die Kopie nach ihrem Datum. Die Einträge des laufenden Jahres werden durch die Kopie ersetzt; frühere Jahre bleiben unverändert.

Kann eine Datei nicht gespeichert werden, teilt das Programm dies mit und nennt den Grund, und Ihre Änderungen bleiben im geöffneten Fenster.

## Sicherung der gesamten Anwendung

**Datei → Gesamte Anwendung sichern…** speichert alle auf diesem Gerät geladenen Einträge aller Jahre zusammen mit den Einstellungen des Programms in einer `.zip`-Datei. Einträge früherer Jahre, die nur in der Cloud liegen und noch nicht heruntergeladen wurden, sind nicht enthalten.

**Datei → Gesamte Anwendung wiederherstellen…** ersetzt nach der Bestätigung die Einträge und die Einstellungen durch die aus einer solchen Datei. Jahre, die nicht in der Datei sind, werden leer, und der Verlauf von **Rückgängig** wird gelöscht. Ist das Programm mit der Cloud verbunden, werden die wiederhergestellten Einträge wie jede andere Änderung synchronisiert. Eine Datei eines anderen Programms oder einer neueren Version wird abgelehnt, und nichts wird verändert.
