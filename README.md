# Smart Farming CEA (Controlled Environment Agriculture)

Controlled Environment Agriculture (CEA) adalah sistem budidaya yang lingkungan udara (aerial) dan zona perakaran (root zone)-nya dikelola secara teknis dan terukur agar pertumbuhan, hasil, kualitas, serta efisiensi sumber daya dapat dioptimalkan sepanjang tahun.

CAE Merupakan pendekatan atau budidaya pertanian di lahan tertutup. Pengendalian dilakukan dengan prinsip ukur – putuskan – eksekusi (sense – decide – act): sensor mengukur kondisi aktual, pengendali membandingkannya dengan target, lalu aktuator mengoreksi selisihnya.

## Bentuk Sistem CEA

| Sistem Budidaya | Bentuk | Ciri Utama | Tingkat Kendali |
|---|---|---|---|
| **Greenhouse (Rumah Tanam)** | Struktur bangunan semi-tertutup | Memanfaatkan cahaya matahari; kendali ventilasi, shading, fogging, dan evaporative cooling | Parsial (cahaya dan suhu luar tetap berpengaruh) |
| **Plant Factory / Vertical Farm (PFAL)** | Ruangan tertutup dengan rak bertingkat | Menggunakan lampu LED, HVAC, dan rak bertingkat | Hampir penuh |
| **Hidroponik / Aeroponik / NFT / DFT** | Sistem budidaya tanpa tanah | Nutrisi dalam larutan; root zone dikendalikan melalui EC, pH, suhu, dan DO | Tinggi pada root zone |
| **Akuaponik dan RAS (Aquaculture)** | Sistem budidaya ikan dan tanaman terintegrasi | Resirkulasi air; kualitas air (suhu, DO, pH, dan amonia) dikendalikan dan diintegrasikan dengan tanaman | Tinggi pada kualitas air |

# Parameter Lingkungan Budidaya
<ul>
   <li>Parameter lingkungan budidaya adalah faktor-faktor fisik, kimia, dan biologi yang diukur dan dikendalikan untuk mendukung pertumbuhan serta kelangsungan hidup organisme budidaya (seperti ikan, udang, atau tanaman).</li>
   <li>Dalam konteks agronomi, parameter lingkungan budidaya tanaman adalah seluruh kondisi biofisik di sekitar tanaman yang memengaruhi proses fotosintesis, respirasi, penyerapan hara, dan transpirasi tanaman.</li>
   <li>Contohnya Mikroklimat, Vapor Pressure Deficit (VPD), Pencahayaan dan Root Zone</li>
</ul>

# Langkah-langkah Konfigurasi Project Dengan IoT

## Prasyarat

Instal tools berikut di komputer (Windows):

| Tools | Fungsi |
|---|---|
| Mosquitto | MQTT broker |
| XAMPP | MySQL server |
| DBeaver | Universal Database Manager (melihat data) |
| Node.js (LTS) | Menjalankan backend |
| Arduino IDE | Upload firmware ke ESP8266 |

> **Penting:** komputer dan ESP8266 harus berada pada jaringan WiFi yang sama (gunakan WiFi **2,4 GHz**)

## Langkah Penggunaan

### A. Hubungkan ke Jaringan

1. Buka Command Prompt, jalankan `ipconfig`, lalu catat **IPv4 Address** komputer (contoh: `192.168.46.15`).
2. Ubah profil WiFi Windows menjadi **Private** (Settings → Network & Internet → WiFi → properti jaringan).
3. Buka Command Prompt **sebagai Administrator**, lalu buka port 1883:

   ```bat
   netsh advfirewall firewall add rule name="Mosquitto MQTT" dir=in action=allow protocol=TCP localport=1883
   ```

   Perintah ini membuka akses MQTT melalui Windows Firewall pada port TCP 1883 agar perangkat lain, seperti ESP8266, dapat terhubung ke Mosquitto Broker di laptop. Tanpa aturan ini, Windows Firewall dapat memblokir koneksi masuk dari ESP8266.

### B. Jalankan Mosquitto

1. Di Command Prompt Administrator, hentikan service bawaan agar tidak bentrok (service bawaan hanya menerima localhost):

   ```bat
   net stop mosquitto
   ```

2. Buat file konfigurasi, misalnya `mosquitto.conf`.
   ```bat
   listener 1883 0.0.0.0
   allow_anonymous true
   ```
4. Jalankan broker dengan konfigurasi proyek (sesuaikan path folder):

   ```bat
   cd /d D:\iot-dht11-esp8266\mosquitto
   "C:\Program Files\mosquitto\mosquitto.exe" -c mosquitto.conf -v
   ```

   Jendela ini **harus tetap terbuka** selama proyek dipakai.

5. Verifikasi di terminal lain:

   ```bat
   netstat -an | findstr 1883
   ```

   Harus muncul `0.0.0.0:1883 ... LISTENING`. Jika hanya `127.0.0.1:1883`, broker belum memakai `mosquitto.conf` proyek.

### C. Jalankan MySQL

1. Di **XAMPP Control Panel**, klik **Start** pada MySQL.
2. Di **DBeaver**, buat koneksi MySQL:
   - Host: `127.0.0.1`
   - Port: `3306`
   - User: `root`
   - Password: *(kosong)*
3. Klik **Test Connection** (unduh driver jika diminta), lalu **Finish**.

### D. Jalankan Backend dan Dashboard

1. Buka `backend/.env`, lalu pastikan baris berikut memakai `127.0.0.1`, **bukan** `localhost`:

   ```env
   MQTT_URL=mqtt://127.0.0.1:1883
   ```

   Alasannya, pada Node.js versi baru `localhost` bisa menunjuk ke IPv6 (`::1`), sedangkan broker hanya mendengarkan IPv4.

2. Di terminal, jalankan:

   ```bat
   cd D:\iot-dht11-esp8266\backend
   npm install
   npm start
   ```

3. Pastikan terminal menampilkan:

   ```
   [MQTT] terhubung ke mqtt://127.0.0.1:1883
   [WEB] dashboard: http://localhost:3000
   ```

4. Buka <http://localhost:3000>. Status perangkat akan tetap **"tidak terhubung"** sampai ESP8266 menyala.

### E. Siapkan Firmware (ESP8266)

1. Atur konfigurasi WiFi dan MQTT pada sketch:

   ```cpp
   const char* WIFI_SSID = "NAMA_WIFI";
   const char* WIFI_PASS = "PASSWORD_WIFI";
   const char* MQTT_HOST = "192.168.46.15";   // IPv4 komputer dari langkah 1
   ```

   Gunakan WiFi **2,4 GHz** dan **jangan** mengisi `MQTT_HOST` dengan `localhost`.

2. Pilih **Tools → Board** (misalnya *NodeMCU 1.0 (ESP-12E Module)*) dan **Port** yang sesuai, lalu klik **Upload**.
3. Buka **Tools → Serial Monitor** (baud **115200**), lalu tekan tombol **RST** pada ESP8266. Harus muncul:

   ```
   Tersambung, IP: 192.168.46.xxx
   Menyambung MQTT 192.168.46.15:1883 ... OK
   ```

> Jika IP komputer berubah (misalnya setelah restart atau pindah WiFi), ubah `MQTT_HOST` lalu upload ulang firmware.

### F. Uji Coba

- Terminal backend menampilkan `[MQTT] perangkat online`, dan dashboard berubah menjadi **"Perangkat terhubung"**.
- Data suhu, kelembaban, dan VPD tampil pada dashboard beserta tabel **Riwayat** yang terisi waktu setiap pembacaan.

### G. Melihat Data di Database

Di DBeaver, klik kanan koneksi lalu **Refresh**, kemudian buka database proyek → **Tables** dan lihat tab **Data**. Atau jalankan query berikut di SQL Editor (`Ctrl+Enter`):

```sql
SELECT
  DATE_FORMAT(created_at, '%d %b %Y, %H.%i.%s') AS waktu,
  temperature AS suhu_c,
  humidity    AS kelembaban,
  vpd         AS vpd_kpa,
  status_bibit,
  status_vegetatif,
  status_generatif
FROM iot_dht11.v_sensor_vpd
ORDER BY id DESC;
```

## Status VPD per Fase Tanaman

| Fase | Rentang ideal (kPa) |
|---|---|
| Bibit | 0,6 – 1,0 |
| Vegetatif | 0,8 – 1,2 |
| Generatif | 1,2 – 1,5 |

- VPD **di bawah** rentang ideal → udara **lembap**
- VPD **di dalam** rentang ideal → **ideal**
- VPD **di atas** rentang ideal → udara **kering**

## Urutan Menyalakan di Hari Berikutnya

1. **Mosquitto:** `net stop mosquitto`, lalu jalankan `mosquitto.exe -c mosquitto.conf -v`.
2. **XAMPP:** Start MySQL.
3. **Backend:** `npm start`.
4. Colokkan ESP8266, lalu buka <http://localhost:3000>.

## Troubleshooting Singkat

| Masalah | Penyebab / Solusi |
|---|---|
| ESP8266 tidak bisa terhubung ke MQTT | Cek aturan firewall port 1883, profil WiFi Private, dan `MQTT_HOST` sudah berisi IPv4 komputer |
| `netstat` hanya menampilkan `127.0.0.1:1883` | Broker belum memakai `mosquitto.conf` proyek; hentikan service bawaan dan jalankan ulang |
| Backend gagal terhubung ke broker | Pastikan `MQTT_URL` memakai `127.0.0.1`, bukan `localhost` |
| Dashboard tetap "tidak terhubung" | ESP8266 belum online; cek Serial Monitor (115200) dan tekan RST |
| Koneksi putus setelah restart / pindah WiFi | IP komputer berubah; perbarui `MQTT_HOST` dan upload ulang |
