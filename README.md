# Arduino Temperaturstyring

Et Arduino-baseret system, der simulerer et klimaanlæg med LED-indikatorer og potentiometerkontrol.

## Funktioner

- Temperatur- og fugtighedsmåling via DHT11-sensor
- Visuel feedback gennem RGB-LED'er:
  - Rød: Varmer
  - Blå: Køler
  - Grøn: Optimal
  - Gul: System tænd/sluk indikator
- Måltemperatur justeres via potentiometer
- System tænd/sluk gemmes i EEPROM
- Knap med debounce til tænd/sluk

## Hardware

Liste over hardware dele og hardwarediagram findes i Fritzing-mappen.

## Softwarekrav

- Arduino IDE

### Biblioteker

Projektet bruger følgende biblioteker:

- Bounce2
- DHT11
- SimpleTimer

Installér dem via Arduino IDE Library Manager eller kopier den medfølgende `libraries`-mappe til din Arduino libraries-mappe.

## Doxygen

Koden er dokumenteret med Doxygen.  
Dette er inkluderet i repo (aka skip til pkt. 3)

For at generere dokumentation:

1. Opsæt Doxygen-konfiguration ved at følge guiden på [Woolsey Workshop](https://www.woolseyworkshop.com/2020/03/20/documenting-arduino-sketches-with-doxygen/)

2. Kør `doxygen` i terminalen

3. Åbn `html/index.html` for at se dokumentationen

## Brugervejledning

1. Tilslut hardware ifølge Fritzing-diagrammet
2. Upload koden via Arduino IDE
3. Brug potentiometeret til at indstille måltemperaturen - Brug Serial Monitor til at se log-output.
4. Tryk på knappen for at tænde/slukke systemet
5. Se de fine LED'er
