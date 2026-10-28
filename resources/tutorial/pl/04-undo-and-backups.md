# Cofanie i kopie zapasowe

## Cofnij i ponów

Menu **Edycja** zawiera **Cofnij** i **Ponów** dla dodawania, edycji i usuwania wpisów (Ctrl+Z i Ctrl+Shift+Z). Historia jest czyszczona, gdy z chmury nadejdzie nowsza wersja danych albo gdy przywrócisz kopię zapasową.

## Kopie zapasowe

Program trzyma kopie listy bieżącego roku obok pliku z danymi: najwyżej jedną co pięć minut, zachowuje pięć najnowszych.

Aby wrócić do jednej z nich, wybierz **Plik → Przywróć z kopii zapasowej…** i wskaż kopię według daty. Wpisy bieżącego roku zostaną zastąpione kopią; wcześniejsze lata pozostają bez zmian.

Jeśli plik nie może zostać zapisany, program informuje o tym i podaje przyczynę, a Twoje zmiany pozostają w otwartym oknie.

## Kopia całej aplikacji

**Plik → Zrób kopię całej aplikacji…** zapisuje wszystkie wpisy ze wszystkich lat wczytanych na tym urządzeniu, razem z ustawieniami programu, do jednego pliku `.zip`. Wpisy z wcześniejszych lat, które są tylko w chmurze i nie zostały jeszcze pobrane, nie są uwzględniane.

**Plik → Przywróć całą aplikację z kopii…** po potwierdzeniu zastępuje wpisy i ustawienia tymi z takiego pliku. Lata, których nie ma w pliku, zostają puste, a historia **Cofnij** jest czyszczona. Gdy program jest połączony z chmurą, przywrócone wpisy synchronizują się jak każda inna zmiana. Plik z innego programu albo zapisany przez nowszą wersję zostanie odrzucony i nic się nie zmieni.
