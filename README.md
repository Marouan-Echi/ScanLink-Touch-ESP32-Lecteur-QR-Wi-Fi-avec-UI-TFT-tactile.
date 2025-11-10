# ScanLink Touch ESP32 Lecteur QR + Wi‑Fi avec UI TFT tactile


Ce projet implémente un module embarqué basé sur ESP32 qui lit des données depuis un lecteur de QR/Code-barres connecté en UART et s’y connecte automatiquement au Wi‑Fi si le QR contient des identifiants. Les identifiants validés sont sauvegardés en mémoire persistante pour des reconnexions automatiques.

Le cœur de l’application se trouve dans `main/main.ino`.

## Fonctionnalités
- **Lecture UART du scanner** sur `Serial1` (9600 bauds), déclenchée par réception d’une ligne.
- **Décodage de QR Wi‑Fi** au format standard `WIFI:S:<SSID>;P:<PASSWORD>;`.
- **Connexion Wi‑Fi automatique** avec timeout 10 s et affichage de l’IP locale.
- **Persistance des identifiants** via `Preferences` (namespace `wifi`).
- **Fallback pratique**: si aucun QR n’est reçu, possibilité d’entrer la trame via le Moniteur Série (USB) en collant la ligne `WIFI:...` et Entrée.

## Architecture rapide
- **Carte**: ESP32 (broches utilisées compatibles ESP32‑S3: GPIO 43/44). 
- **Périphérique**: Lecteur QR/Code-barres UART (module tiers qui envoie une ligne ASCII terminée par `\n`).
- **Fichier principal**: `main/main.ino`.
- **Librairies Arduino utilisées**: `WiFi.h`, `Preferences.h` (fournies par l’ESP32 Arduino Core).
- **Dossier `library/`**: contient des libs tierces (TFT, Touch, ArduinoJson) non utilisées par `main.ino` actuellement.

## Matériel requis
- 1× **Carte ESP32** (idéalement ESP32‑S3 disposant des GPIO 43/44)
- 1× **Lecteur QR/Code‑barres UART** (TTL 3.3 V ou 5 V selon module)
- Câbles Dupont et alimentation adaptée (3.3 V / 5 V selon module)
- Optionnel: convertisseur de niveau si le module est 5 V‑TTL non compatible 3.3 V

- Optionnel: **Écran TFT SPI** (ex. ILI9341 2.4/2.8") avec **tactile XPT2046**

## Câblage
Référez‑vous au document `Cablage.pdf` pour le schéma. Rappel des liaisons critiques d’après le code:

- Scanner `TX` → ESP32 `GPIO 43` (entrée RX du microcontrôleur pour `Serial1`)
- Scanner `RX` → ESP32 `GPIO 44` (sortie TX du microcontrôleur pour `Serial1`)
- Scanner `GND` → ESP32 `GND`
- Scanner `VCC` → Alimentation adaptée (3.3 V ou 5 V selon le module)

Notes:
- Les noms dans le code sont `scannerRxPin = 43` et `scannerTxPin = 44`. C’est la correspondance côté ESP32 (RX/43, TX/44) pour `Serial1.begin(9600, SERIAL_8N1, 43, 44)`.
- Vérifiez la compatibilité de tension. Si le module est 5 V, utilisez un convertisseur de niveau ou alimentez correctement en respectant les IO à 3.3 V.

## Écran TFT tactile (optionnel)

Cette partie décrit l’intégration d’un **écran TFT SPI** (ex. contrôleur ILI9341) et d’un **panneau tactile résistif XPT2046**. Les bibliothèques nécessaires sont présentes dans `library/`.

### Bibliothèques
- `TFT_eSPI` (affichage TFT)
- `XPT2046_Touchscreen` (tactile)

### Configuration TFT_eSPI
TFT_eSPI requiert un réglage de configuration pour votre écran et vos pins SPI.

1. Ouvrir le fichier de configuration de `TFT_eSPI` (selon la version):
   - `library/TFT_eSPI/User_Setup.h` ou
   - `library/TFT_eSPI/User_Setup_Select.h` (sélection d’un setup prédéfini)
2. Sélectionner le pilote (ex. `ILI9341`) et définir les pins de votre carte ESP32.
3. Associer le bus SPI choisi (SCLK, MOSI, MISO) et les pins de contrôle:
   - `TFT_CS`, `TFT_DC` (ou `TFT_RS` selon terminologie), `TFT_RST` (facultatif si câblé au reset).

Notes importantes:
- Les pins exacts dépendent du modèle d’ESP32 (ESP32 classique vs ESP32‑S3). Reportez‑vous au pinout de la carte.
- Sur de nombreux écrans TFT, le tactile XPT2046 partage le bus SPI du TFT (mêmes SCLK/MOSI/MISO) et ajoute un **CS** séparé pour le tactile.

### Câblage générique (TFT + XPT2046)
Bus SPI partagé (exemple générique à adapter):
- `TFT_SCK` → ESP32 `SCLK`
- `TFT_MOSI` → ESP32 `MOSI`
- `TFT_MISO` → ESP32 `MISO` (lecture via SPI, utile pour certaines fonctions)
- `TFT_CS` → ESP32 `<TFT_CS_PIN>`
- `TFT_DC` → ESP32 `<TFT_DC_PIN>`
- `TFT_RST` → ESP32 `<TFT_RST_PIN>` (ou Reset commun)
- `TFT_BL` (rétroéclairage) → ESP32 `<BL_PIN>` via transistor ou directement selon module

Tactile XPT2046 (sur le même bus SPI):
- `T_CS` (Chip Select tactile) → ESP32 `<T_CS_PIN>`
- `T_IRQ` (interruption tactile) → ESP32 `<T_IRQ_PIN>` (entrée digitale)

Alimentation et masse:
- `VCC` écran/tactile → 3.3 V (ou 5 V selon module, vérifier la doc du module)
- `GND` écran/tactile → GND commun ESP32

Astuce: commencez par afficher un exemple simple `TFT_eSPI` (texte/couleur) avant d’ajouter le tactile.

### Exemples utiles
- Exemples `TFT_eSPI`: `library/TFT_eSPI/examples/`
- Exemples `XPT2046_Touchscreen`: `library/XPT2046_Touchscreen/examples/`

Ces exemples montrent l’initialisation des écrans courants et la lecture des coordonnées tactiles.

## Configuration logicielle
- **IDE**: Arduino IDE (ou PlatformIO)
- **Carte**: Installer le support ESP32 via le gestionnaire de cartes Arduino (ESP32 by Espressif Systems)
- **Vitesse Moniteur Série**: 9600 bauds
- **Ports Série**:
  - `Serial` (USB) pour logs et saisie manuelle.
  - `Serial1` (UART scanner) à 9600 bauds avec `GPIO 43/44`.

### Dépendances
Aucune librairie externe supplémentaire n’est requise pour le sketch principal en dehors de l’ESP32 Arduino Core (fournissant `WiFi` et `Preferences`).

## Format des QR attendus
Le projet attend une trame au format Wi‑Fi standard:

```
WIFI:S:<SSID>;P:<PASSWORD>;
```

Exemple:

```
WIFI:S:MonReseau;P:MonMotDePasse;
```

- La trame doit commencer par `WIFI:`.
- Les champs `S:` (SSID) et `P:` (mot de passe) doivent être terminés par `;`.
- Le scanner doit envoyer un retour chariot fin de ligne (`\n`).

## Utilisation
1. Câbler le module comme dans `Câblage` et `Cablage.pdf`.
2. Flasher `main/main/main.ino` sur la carte ESP32.
3. Ouvrir le Moniteur Série à 9600 bauds.
4. Au démarrage:
   - Le programme tente d’abord la **connexion Wi‑Fi enregistrée** (si disponible, 10 s de timeout).
   - Sinon, il attend des données du **scanner UART** ou une **saisie manuelle** dans le Moniteur Série.
5. Scanner un QR contenant la trame `WIFI:...` (ou coller la trame dans le Moniteur Série puis Entrée).
6. En cas de succès, l’ESP32 affiche `Connecté` et l’**adresse IP** puis **sauvegarde** les identifiants.

## Dépannage
- Rien ne s’affiche dans le Moniteur Série:
  - Vérifier le débit 9600 bauds et le bon port COM.
- Pas de données reçues du scanner:
  - Inverser TX/RX entre le scanner et l’ESP32 (TX→RX, RX→TX).
  - Vérifier l’alimentation du module et le niveau de tension (3.3 V vs 5 V).
  - Tester en collant la trame `WIFI:...` directement dans le Moniteur Série.
- Connexion Wi‑Fi échouée:
  - Vérifier SSID/mot de passe, proximité du point d’accès.
  - Augmenter le timeout si besoin (boucle d’attente dans `connectToWiFiFromQR`).
- Mémoire d’identifiants corrompue/incorrecte:
  - Modifier le namespace ou ajouter une fonction d’effacement `Preferences` si nécessaire.

## Personnalisation rapide
- **Pins UART**: modifier `scannerRxPin` et `scannerTxPin` dans `main.ino`.
- **Timeout Wi‑Fi**: ajuster la boucle d’attente (10 s par défaut).
- **Validation de trame**: renforcer le parseur dans `connectToWiFiFromQR` si d’autres formats de QR sont supportés.

## Documents
- Schéma de câblage: `Cablage.pdf`
- Code principal: `main/main.ino`
- Librairies tierces fournies dans `library/` (non utilisées par défaut dans ce sketch).

## Licence
Ce dépôt inclut des bibliothèques tierces sous leurs licences respectives (voir leurs READMEs). Le code du sketch principal peut être utilisé et adapté dans le cadre de votre projet interne.
