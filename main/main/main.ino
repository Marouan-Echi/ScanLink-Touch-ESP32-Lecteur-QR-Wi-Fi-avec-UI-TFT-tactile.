#include <WiFi.h>
#include <Preferences.h>

// === Broches du scanner QR ===
const int scannerRxPin = 43;  // RX du module
const int scannerTxPin = 44;  // TX du module

// === Mémoire persistante ===
Preferences preferences;

// === Prototypes ===
void connectToWiFiFromQR(const String& qrData);
void saveWiFiCredentials(const String& ssid, const String& password);
bool loadWiFiCredentials(String& ssid, String& password);
void setupWiFi();
void processScannerInput();

void setup() {
  Serial.begin(9600);                            
  Serial1.begin(9600, SERIAL_8N1, scannerRxPin, scannerTxPin); // Scanner QR

  Serial.println("=== Configuration Wi-Fi ===");
  setupWiFi(); // Tente la connexion avec les identifiants enregistrés
}

void loop() {
  processScannerInput(); // Attend un QR contenant les infos Wi-Fi
}

// === Lecture du QR depuis le scanner ou le moniteur série ===
void processScannerInput() {
  String dataqr = "";

  if (Serial1.available() > 0) {
    dataqr = Serial1.readStringUntil('\n');
    dataqr.trim();
  } else if (Serial.available() > 0) {
    dataqr = Serial.readStringUntil('\n');
    dataqr.trim();
  }

  if (dataqr.length() > 0 && dataqr.startsWith("WIFI:")) {
    connectToWiFiFromQR(dataqr);
  }

  delay(100);
}

// === Connexion Wi-Fi depuis QR ===
void connectToWiFiFromQR(const String& qrData) {
  String ssid = "";
  String password = "";

  int ssidStart = qrData.indexOf("S:") + 2;
  int ssidEnd = qrData.indexOf(";", ssidStart);
  int passStart = qrData.indexOf("P:") + 2;
  int passEnd = qrData.indexOf(";", passStart);

  if (ssidStart == 1 || ssidEnd == -1 || passStart == 1 || passEnd == -1) {
    Serial.println("❌ QR Wi-Fi invalide !");
    return;
  }

  ssid = qrData.substring(ssidStart, ssidEnd);
  password = qrData.substring(passStart, passEnd);

  Serial.println("📶 Tentative de connexion Wi-Fi...");
  Serial.println("SSID: " + ssid);
  Serial.println("Mot de passe: " + password);

  WiFi.begin(ssid.c_str(), password.c_str());
  unsigned long start = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
    delay(1000);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ Connecté avec succès !");
    Serial.print("Adresse IP : ");
    Serial.println(WiFi.localIP());
    saveWiFiCredentials(ssid, password);
  } else {
    Serial.println("\n❌ Échec de connexion !");
  }
}

// === Sauvegarde des identifiants Wi-Fi ===
void saveWiFiCredentials(const String& ssid, const String& password) {
  preferences.begin("wifi", false);
  preferences.putString("ssid", ssid);
  preferences.putString("password", password);
  preferences.end();
  Serial.println("💾 Identifiants Wi-Fi sauvegardés !");
}

// === Chargement des identifiants Wi-Fi enregistrés ===
bool loadWiFiCredentials(String& ssid, String& password) {
  preferences.begin("wifi", true);
  ssid = preferences.getString("ssid", "");
  password = preferences.getString("password", "");
  preferences.end();
  return !(ssid.isEmpty() || password.isEmpty());
}

// === Tentative de connexion à un Wi-Fi sauvegardé ===
void setupWiFi() {
  String ssid, password;

  if (loadWiFiCredentials(ssid, password)) {
    Serial.println("🔁 Connexion au Wi-Fi enregistré...");
    WiFi.begin(ssid.c_str(), password.c_str());

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
      delay(1000);
      Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\n✅ Connecté !");
      Serial.print("Adresse IP : ");
      Serial.println(WiFi.localIP());
      Serial.println("\nScanner un QR Wi-Fi pour tester une nouvelle connexion.");
      return;
    }
  }

  Serial.println("\n⚠️ Aucun Wi-Fi enregistré.");
  Serial.println("Scanner un QR Wi-Fi pour se connecter.");
}
