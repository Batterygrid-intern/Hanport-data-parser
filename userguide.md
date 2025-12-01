Hanport Data Parser - Användarguide

Innehåll
- [Systemkrav](#Systemkrav)
- [Installation]
- [Konfiguration]
- [Körning]
- [Felsökning]

Systemkrav

Hårdvara
- Raspberry Pi (testad på Raspberry Pi 4)
- Hanport mätare ansluten via seriell port (standard: `/dev/ttyAMA0`)
- Nätverksanslutning för MQTT

Programvara
- Raspberry Pi OS (Bullseye eller senare)
- Root-rättigheter för installation
- Inga externa bibliotek krävs (allt inkluderat i releasen)

Installation

1. Förbered systemet

```bash
# Uppdatera systemet
sudo apt update
sudo apt upgrade -y 
# Inga ytterligare paket behövs - allt är statiskt länkat i binären
```

2. Ladda ner och installera

```bash
# Ladda ner senaste release (ersätt X.Y.Z med aktuell version)
curl -LO https://github.com/Batterygrid-intern/Hanport-data-parser/releases/download/v1.0.0/hanport-deploy-20251127_111403.zip

# Packa upp
unzip hanport-data-parser-vX.Y.Z.zip
cd hanport-data-parser-vX.Y.Z

# Kör installationsskript
sudo ./scripts/install.sh
```

Installationsskriptet gör följande:
- Kopierar binären till `/opt/hanport/`
- Skapar konfigurationskatalog `/opt/hanport/configs/`
- Skapar loggkatalog `/var/log/hanport/`
- Installerar systemd service

3. Konfigurera seriell port

```bash
# Aktivera seriell port i raspi-config
sudo raspi-config
# Navigera till: Interface Options -> Serial Port
# Välj: "No" för login shell, "Yes" för serial port hardware

# Lägg till användare i dialout-gruppen
sudo usermod -a -G dialout pi

# Logga ut och in igen för att aktivera gruppmedlemskap
```

## Konfiguration

### Redigera konfigurationsfil

```bash
sudo nano /opt/hanport/configs/app.ini
```

### Konfigurationsexempel (app.ini)

```ini
[SERIALPORT]
PATH=/dev/ttyAMA0

[MQTT]
BROKER=localhost:1883
CLIENT_ID=hanport_client
SITE_ID=my-site
DEVICE_ID=hanport_meter_01
USERNAME=mqtt_user
PASSWORD=mqtt_password
MEASUREMENT_TOPIC=hanport_data
WILL_TOPIC=hanport_client/status
WILL_MESSAGE=offline

[MODBUS]
PORT=1502
```

Viktiga konfigurationsparametrar

| Parameter | Beskrivning | Standard |
|-----------|-------------|----------|
| SERIALPORT/PATH | Sökväg till seriell enhet | /dev/ttyAMA0 |
| MQTT/BROKER | MQTT broker adress:port | localhost:1883 |
| MQTT/SITE_ID | Unik identifierare för platsen | bgs-office |
| MQTT/DEVICE_ID | Unik identifierare för enheten | hanport_meter_01 |
| MODBUS/PORT | Modbus TCP port (använd >1024 för icke-root) | 1502 |

OBS: Modbus port 502 kräver root-rättigheter. Detta konfigureras när man kör installationsskriptet

Körning

Manuell start

```bash
# Kör programmet
cd /opt/hanport
sudo ./hanport-data-parser --config configs/app.ini
```

Automatisk start med systemd

Systemd service installeras automatiskt av installationsskriptet.

```bash
# Aktivera autostart
sudo systemctl enable hanport.service

# Starta tjänsten
sudo systemctl start hanport.service

# Kontrollera status
sudo systemctl status hanport.service

# Stoppa tjänsten
sudo systemctl stop hanport.service

# Starta om tjänsten
sudo systemctl restart hanport.service
```

Visa loggar

```bash
# Realtidsloggar från systemd
sudo journalctl -u hanport.service -f

# Visa senaste 100 rader
sudo journalctl -u hanport.service -n 100

# Loggar sedan senaste uppstart
sudo journalctl -u hanport.service -b

# Loggar som skrivs till logg filen. /var/log/hanportx.y.z.log 
sudo tail -f /var/log/hanport/XYZ.log
```

Programmet loggar även till stdout/stderr som visas via journalctl.

Felsökning

Programmet startar inte

Kontrollera systemd status:
```bash
sudo systemctl status hanport.service
```

Kontrollera att seriell port är tillgänglig:
```bash
ls -l /dev/ttyAMA0
# Ska visa: crw-rw---- 1 root dialout ...
```

Kontrollera användarrättigheter:
```bash
groups
# Ska innehålla 'dialout' om du kör manuellt
```

Testa seriell anslutning:
```bash
sudo cat /dev/ttyAMA0
# Ska visa data från mätaren
```

MQTT-anslutning misslyckas

Kontrollera att MQTT broker körs:
```bash
# Om du använder Mosquitto
sudo systemctl status mosquitto

# Testa anslutning manuellt
mosquitto_pub -h localhost -p 1883 -t test -m "hello"
```

Vanliga problem:
- Fel broker-adress i config
- Fel användarnamn/lösenord
- Firewall blockerar port 1883
- MQTT broker inte installerad

Installera Mosquitto (om den saknas):
```bash
sudo apt install mosquitto mosquitto-clients
sudo systemctl enable mosquitto
sudo systemctl start mosquitto
```

Modbus fungerar inte

Kontrollera att rätt port används:
```bash
# Port >1024 rekommenderas (kräver inte root)
sudo netstat -tulpn | grep hanport
```

Om du måste använda port 502:
```bash
# Ge binären capability att binda privilegierade portar
sudo setcap 'cap_net_bind_service=+ep' /opt/hanport/hanport-data-parser (görs i installationsscriptet)

# Uppdatera config till PORT=502
sudo nano /opt/hanport/configs/app.ini

# Starta om
sudo systemctl restart hanport
```

Testa Modbus-anslutning:
```bash
# Installera modbus test-verktyg
sudo apt install mbpoll

# Läs första 10 registren
mbpoll -a 1 -r 0 -c 10 -t 4 localhost 1502
```

Inga data läses från mätaren

Kontrollera seriell kommunikation:
```bash
# Lyssna på seriell port (Ctrl+C för att avsluta)
sudo cat /dev/ttyAMA0
# Ska visa binär data från mätaren
```

Vanliga problem:
- Fel seriell port i config
- Mätaren inte inkopplad
- Fel kabelanslutning (TX/RX)
- Fel baudrate (ska vara 115200)
- Seriell port använd av annat program

Verifiera seriell konfiguration:
```bash
stty -F /dev/ttyAMA0
# Kontrollera baudrate och inställningar
```

 CRC-fel i loggarna

```
MessageValidator: CRC mismatch - Calculated: 1234 Transmitted: 5678
```

Möjliga orsaker:
- Störningar på seriell linje
- Dålig kabelanslutning
- EMI från annan utrustning
- Mätaren sänder korrupt data

Åtgärder:
- Kontrollera kabelanslutningar
- Använd skärmad kabel
- Flytta bort från störningskällor
- Starta om mätaren

Programmet kraschar

Visa senaste krasch:
```bash
sudo journalctl -u hanport.service -n 50
```

Kör manuellt för mer detaljerad felsökning:
```bash
cd /opt/hanport
sudo ./hanport-data-parser --config configs/app.ini
# Ctrl+C för att avsluta
```

Vanliga krascher:
- Kan inte öppna seriell port → Kontrollera permissions
- Config-fil saknas → Kontrollera sökväg
- Minnesfel → Kontrollera systemresurser

Kontrollera systemresurser

```bash
# Minnesstatus
free -h

# CPU-användning
top

# Diskutrymme
df -h

# Kontrollera att hanport körs
ps aux | grep hanport
```

Avinstallation

```bash
# Stoppa och avaktivera service 
sudo systemctl stop hanport.service
sudo systemctl disable hanport.service

# Ta bort service-fil
sudo rm /etc/systemd/system/hanport.service
sudo systemctl daemon-reload

# Ta bort programfiler
sudo rm -rf /opt/hanport

# Ta bort loggar (valfritt)
sudo rm -rf /var/log/hanport

# Kör uninstall script som ligger i zip filen.
sudo bash uninstall.sh
```

Uppgradering

```bash
# Ladda ner ny version
curl - LO https://github.com/Batterygrid-intern/Hanport-data-parser/releases/download/v1.0.0/hanport-deploy-20251127_111403.zip 

Stoppa tjänsten
sudo systemctl stop hanport.service

# Packa upp och installera
unzip hanport-data-parser-vX.Y.Z.zip
cd hanport-data-parser-vX.Y.Z
sudo ./scripts/install.sh

# Din config bevaras automatiskt om den redan finns
# Starta tjänsten igen
sudo systemctl start hanport.service

# Kontrollera att allt fungerar
sudo systemctl status hanport.service
```

Support

För teknisk support eller buggrapporter, öppna ett ärende på GitHub:
https://github.com/Batterygrid-intern/Hanport-data-parser 

Inkludera följande information:
- Version av programmet
- Raspberry Pi modell och OS-version
- Relevanta loggar från `journalctl`
- Konfigurationsfil (ta bort känsliga uppgifter som lösenord)
