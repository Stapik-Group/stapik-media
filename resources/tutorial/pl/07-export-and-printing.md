# Eksport i drukowanie

## CSV

**Plik → Eksportuj wszystkie wpisy do CSV…** zapisuje do pliku CSV wszystkie wpisy wczytane na tym urządzeniu, ze wszystkich kategorii. Plik poprawnie otwiera się w Excelu: jest w UTF-8 ze znacznikiem BOM, a separator zależy od języka programu. Daty zapisywane są jako `2026-03-05`, `2026-03` albo `2026`, zależnie od dokładności daty wydania, a miesiąc skonsumowania jako `2026-03`.

Wpisy z wcześniejszych lat, które są tylko w chmurze i nie zostały jeszcze pobrane, nie są uwzględniane. Wybierz najpierw rok w filtrze **Rok**, aby go pobrać.

## Import z CSV

**Plik → Importuj wpisy z CSV…** dodaje wpisy z pliku CSV. Plik powinien mieć te same kolumny co eksport: kategoria, tytuł, data wydania, miesiąc skonsumowania, reżyser / autor / wykonawca / studio, wydawca, gatunek, platforma, audiobook, ocena (1–10) i notatka. Wszystko po miesiącu skonsumowania może być puste lub pominięte. Kategoria to jedno z: `movie`, `series`, `cartoon`, `book`, `album` albo `game`, a daty zapisuje się tak jak w eksporcie (`2026-03-05`, `2026-03` albo `2026` dla daty wydania, `2026-03` dla miesiąca skonsumowania). Pierwszy wiersz jest traktowany jako nagłówek, chyba że zaczyna się od kategorii. Separator, przecinek lub średnik, jest rozpoznawany automatycznie.

Zanim cokolwiek zostanie zapisane, podgląd pokazuje każdy wiersz jako nowy, duplikat albo błędny. Duplikaty są rozpoznawane tak samo jak przy dodawaniu wpisu: ta sama kategoria, tytuł i rok wydania, zarówno względem Twojej listy, jak i powtórzeń w samym pliku. Duplikaty, błędne wiersze oraz wpisy z wcześniejszych lat, których archiwum w chmurze nie zostało pobrane, są pomijane; resztę dodaje się po potwierdzeniu. Cały import to jeden krok, który cofa **Cofnij**.

## Drukowanie i PDF

**Plik → Drukuj listę…** i **Plik → Eksportuj listę do PDF…** używają listy tak, jak ją widzisz: wybranej kategorii z filtrem roku, miesiąca i wyszukiwaniem, ze wszystkich stron, a nie tylko z tej, która jest wyświetlona.
