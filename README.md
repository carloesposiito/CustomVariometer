# CustomVariometer

[By Carlo Esposito, 2024, Github: @carloesposiito]


Il progetto CustomVariometer permette di utilizzare un microcontrollore (Arduino, ESP32, ...) come variometro, ossia come un dispositivo che misura la variazione di altitudine e fornisce informazioni in tempo reale sulla velocita' di salita e discesa.


Il variometro, opportunamente configurato, suonera' quando oltrepassata una determinata velocita' di salita o discesa, emettendo suoni differenti.<br/>
Inoltre verranno riportate sullo schermo l'altitudine assoluta e quella relativa al punto di accensione, la velocita' di variazione dell'altezza in m/s e la temperatura registrata.<br/>
E' possibile utilizzare il programma anche senza un display: i componenti fondamentali sono microcontrollore, sensore BMP280 e buzzer passivo.


Per tutte le configurazioni possibili, si rimanda alla sezione _Codice_ in basso.<br/>


![](https://github.com/carloesposiito/CustomVariometer/blob/main/CustomVariometer/photos/mainScreen_CustomVariometer.png)


# Hardware necessario

- 1x ESP32 Super Mini;<br/>
- 1x Buzzer passivo;<br/>
- 1x Sensore BMP280;<br/>
- 1x Display OLED 0.96" 128x64;


In questo progetto si fa riferimento ad ESP32 Super Mini, ma e' possibile utilizzare anche altri microcontrollori.<br/>
Inoltre vi sono due tipi di sensore BMP280: in questo caso e' stato scelto il modello a 6 pin, ma anche quello a 4 pin e' compatibile con il progetto (con le opportune modifiche).


# Software necessario

Sono necessarie le librerie per il controllo del sensore, del display e del buzzer:<br/>
- Adafruit_BMP280;<br/>
- Adafruit_SSD1306;<br/>
- Adafruit_BusIO;<br/>
- Adafruit_GFX_Library;<br/>
- Adafruit_Unified_Sensor;<br/>
- CuteBuzzerSounds;


Possono essere installate manualmente tramite Arduino IDE, oppure estratte automaticamente nella cartella `C:\Users\USER\Documents\Arduino\libraries` eseguendo il file `CustomVariometer/libraries/libraries_CustomVariometer.exe`.<br/>
Nel secondo caso prestare attenzione che non esistano gia' cartelle con lo stesso nome di quelle che verranno estratte per evitare conflitti (tuttavia e' una situazione improbabile).


# Schema di collegamento

Lo schema di collegamento e' il seguente:


![](https://github.com/carloesposiito/CustomVariometer/blob/main/CustomVariometer/scheme/scheme_CustomVariometer.jpg)


Il modulo del sensore e' collegato tramite SPI, mentre il display tramite I2C.


# Codice

E' possibile configurare alcune funzionalita' del programma tramite la sezione **EDITABLE PARAMETERS**:


- **DEBUG**: se _true_ permette la visualizzazione dei dati nel monitor seriale.


- **MS_DETECTION_INTERVAL**: indica l'intervallo di rilevamento del sensore espresso in millisecondi.


- **USE_DISPLAY**: se _true_ attiva la modalita' di visualizzazione dei dati sul display OLED collegato.


- **BATTERY CHECK**: se impostato su _true_ permette di monitorare la carica della batteria collegata al pin indicato nella variabile **BATTERY_PIN**. Manca nello schema di collegamento.


- **USE_SEA_PRESSURE**: se _true_, il sensore BMP280 viene calibrato utilizzando come riferimento la pressione indicata nella variabile **SEA_PRESSURE_HPA**, altrimenti il riferimento e' la pressione rilevata dal sensore al momento dell'accensione.<br/>
Nota bene: se _true_, piu' e' accurato il valore (unita' di misura hPa) migliore sara' la rilevazione. Controlla dunque il valore nella tua localita' in quanto potrebbe cambiare quotidianamente!


- **SENSIBILTY**: indica la posizione del valore scelto all'interno della lista di valori dell'array **sensibilities** (la numerazione parte da zero).<br/>
Rappresenta la velocita' di salita (m/s) che deve essere oltrepassata per far suonare l'allarme.


- **SINK_ALARM**: indica la posizione del valore scelto all'interno della lista di valori dell'array **sinkAlarms** (la numerazione parte da zero).<br/>
Rappresenta la velocita' di discesa (m/s) in negativo che deve essere oltrepassata per far suonare l'allarme.
