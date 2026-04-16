# Domain Assumptions

## Simulation Level
Sistem ini adalah microsimulation. Setiap kendaraan dimodelkan sebagai entitas individual, bukan sebagai aliran agregat.

## Road Geometry
Asumsi awal:
- satu simpang empat arah
- satu lajur efektif per approach
- geometri top-down 2D
- stop line konsisten per approach
- tidak ada bundaran pada versi awal

## Traffic Flow Direction
Aturan lalu lintas mengikuti lajur kiri. Itu berarti orientasi kendaraan, posisi spawn, dan perilaku antrian harus disusun sesuai kebiasaan jalan di Indonesia.

## Time Model
Asumsi waktu:
- simulasi berjalan dalam detik
- update memakai fixed timestep
- render tidak menentukan hasil perilaku kendaraan

## Vehicle Behavior
Setiap kendaraan diasumsikan memiliki:
- panjang dan lebar fisik
- kecepatan maksimum
- akselerasi
- deselerasi
- jarak aman minimum
- reaksi sederhana terhadap lampu dan kendaraan di depan

## Queue Definition
Queue didefinisikan secara operasional sebagai kendaraan yang:
- berada di zona antrian dekat intersection
- bergerak di bawah ambang kecepatan tertentu

## Wait Time Definition
Wait time adalah akumulasi waktu ketika kendaraan berada dalam kondisi berhenti atau bergerak sangat lambat di bawah ambang yang ditentukan.

## Signal Model
Asumsi sinyal:
- dua fase utama
- fase north-south hijau saat east-west merah
- fase east-west hijau saat north-south merah
- fase kuning bersifat transisi tetap

## Spawn Model
Asumsi spawn:
- kendaraan muncul dari edge peta
- spawn rate dikontrol sebagai parameter demand
- spawn harus dicek terhadap kepadatan awal agar tidak overlap

## Randomness
Randomness boleh dipakai untuk:
- pemilihan tipe kendaraan
- interval reaksi
- probabilitas spawn

Tetapi randomness harus dapat dikendalikan dengan seed agar skenario bisa diulang.

## Rendering Assumptions
Visualisasi hanya bertugas menampilkan state yang sudah dihitung engine.
Rendering tidak boleh:
- mengubah hasil simulasi
- memutuskan collision
- memutuskan lampu
- menyimpan state domain utama

## Data and Metrics
Asumsi metrik minimum:
- queue length per orientasi
- average wait time per orientasi
- total vehicles spawned
- total vehicles completed
- active vehicles

## Non-Goals for v1
Yang tidak diasumsikan ada pada versi awal:
- pedestrian phase
- lane changing
- route planning kompleks
- multi-intersection synchronization
- adaptive control berbasis optimasi

## Implementation Consequences
Semua asumsi di atas berarti struktur internal harus memisahkan:
- configuration
- domain model
- simulation engine
- rendering/UI
- analytics
