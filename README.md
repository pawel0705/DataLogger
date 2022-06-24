# DataLogger
Urządzenie odczytujące temperaturę z otoczenia i zapisujące ją na kartę SD co określony czas.

## Zasada działania urządzenia

- Urządzenie ma na celu pomiar temperatury jego otoczenia oraz zapis na kartę SD.
- Użytkownik może odczytać aktualną wartość temperatury z wyświetlacza LCD, a także wiele pomiarów temperatur zapisanych na karcie SD w postaci pliku tekstowego.
- Użytkownik może przyciskiem modyfikować odstęp czasowy zapisu danych na kartę SD (urządzenie oferuje kilka opcji np. zapis co 5min, 10min…).
- Użytkownik może ustawić wartość temperatury, po której przekroczeniu będzie informowany pikaniem urządzenia (urządzenie oferuje kilka opcji).
- Mruganie diody podczas zapisu na kartę SD lub jej braku.

## Cel urządzenia

Monitorowanie temperatury pomieszczeń, która nie może przekraczać pewnej wartości np.:
- przewóz/przechowywanie żywności
- kotłownia
- hodowla roślin

-> Użytkownik zostaje powiadomiony pikaniem urządzenia
-> Użytkownik kolejno może odczytać z pliku karty SD jak temperatura się zmieniała (przykładowo co 5min, jeżeli użytkownik wybierze taki przedział czasowy)

## Wykorzystane moduły

### Atmega 328P-PU

- Zasilanie: 1.8V – 5.5V (w projekcie 3.3V)
- Taktowanie do 20 MHz (w projekcie zewnętrzny kwarc 8MHz)
- Pamięć flash 32KB (w projekcie wykorzystano połowę dostępnej pamięci)
- 23 linie wejścia/wyjścia (w projekcie prawie wszystkie wykorzystano

### Przycisk 4x TACT SWITCH

- Mikroprzycisk 4.5mm x 4.5mm
- Powierzchniowy SMD
- 4 piny
- OFF-(ON)
- Wielkość popychacza 5mm
- Prąd styków (50mA)
- Napięcie przełączania 12V DC

### Buzzer elektromagnetyczny

- Poziom dźwięku 75dB
- Napięcie pracy 2-5V
- Prąd pracy 20mA
- Wysokość 5mm
- Montaż przewlekany (THT)
- Częstotliwość rezonansowa 2.5kHz

### Koszyk na baterie

- 4x AA
- Pojemnik z pokrywą
- Z przewodem
- Z wyłącznikiem

### Bezpiecznik polimerowy

- Maksymalny prąd pracy 250mA
- Maksymalne napięcie pracy 16V DC
- Rodzaj obudowy 1206
- Sposób montażu powierzchniowy (SMD)

### Przetwornica step-down 3.3V

- Polulu 2842 3.3V
- Step-Down Voltage Regulator
- Maximum output 500mA
- Zakres napięć 3V-36V
- Montaż przewlekany (THT)

### Dioda LED 3mm czerwona

- Soczewka w kolorze czerwonym
- Obudowa: DIP 3mm

### Rezonator kwarcowy 8Mhz

- Częstotliwość 8MHz
- Montaż powierzchniowy (SMD)
- Temperatura pracy -20…+70C

### Czujnik temperatury DS18B20+

- Montaż przewlekany (THT)
- Napięcie zasilania 3V-5.5V
- Zakres pomiarów -55C do 125C
- Dokładność pomiarów +/- 0.5C
- Rozdzielczość od 9 do 12 bitów

### Złącze do kart SD

- Rodzaj karty MMC;SD
- Liczba torów – 9
- Montaż powierzchniowy (SMD)
- Pokrycie styków złocone

### Wyświetlacz LCD-AC-1602E-YIY

- Wyświetlacz LCD 2x16 80x36mm
- Podświetlenie LED 3V (yellow-green)
- Rozszerzony zakres temperatury
- Wyświetlacz dedykowany do pracy 3.3V / rezystory w podświetleniu dobrane do napięcia 3.3V
- Wbudowany generator napięcia ujemnego, praca od 2V

### Potencjometr 10kohm 3224J-102E

- Rezystancja 10kohm
- Wymiary korpusu 4.8x4.6x3.7mm
- Montaż powierzchniowy (SDM)
- Tolerancja 10%
- Rodzaj potencjometru – wieloobrotowy
- Charakterystyka liniowa

### Tranzystor unipolarny BS170FTA

- Kierunek przewodnictwa N-MOSFET
- Prąd drenu 0.5A
- Napięcie dren-źródło 60V
- Montaż powierzchniowy (SMD)
- Moc 320mW

### Zestaw rezystorów i kondensatorów SMD o obudowie 1206


## Zdjęcia

![Image](/images/schemat.png)

![Image](/images/plytka.png)

![Image](/images/fiz.png)
