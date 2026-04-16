# Scope V1

## Version Boundary
Versi 1 adalah simulasi simpang tunggal empat arah yang bersifat microsimulation dan berjalan real time di desktop. Semua keputusan scope di bawah ini bersifat final untuk iterasi awal.

## In Scope
- C++ desktop application
- single 4-way intersection
- 2D top-down view
- one lane per approach
- vehicle-level simulation
- two-phase signal control
- configurable green durations for north-south and east-west
- fixed yellow duration
- spawn control via demand parameter
- queue metrics
- average wait time metrics
- pause, resume, reset
- basic configuration file support if required by the runtime design

## Vehicle Types
Versi 1 mendukung:
- car
- motorcycle/bike
- truck

Masing-masing tipe kendaraan berbeda pada:
- ukuran fisik
- kecepatan maksimum
- akselerasi
- deselerasi
- warna atau sprite representasi

## Traffic Rules
Aturan yang wajib diterapkan:
- kendaraan berhenti sebelum stop line saat lampu merah
- kendaraan hanya bergerak jika fase sinyal mengizinkan
- kendaraan menjaga jarak aman terhadap kendaraan di depan
- simulasi mengikuti aturan lajur kiri
- fase kuning bersifat transisi, bukan fase bebas

## UI and Controls
Kontrol minimum:
- tombol pause/resume
- tombol reset
- slider durasi green NS
- slider durasi green EW
- kontrol demand atau spawn rate

Tampilan minimum:
- status sinyal
- timer fase aktif
- queue length per orientasi
- average wait time per orientasi
- jumlah kendaraan aktif
- total kendaraan selesai

## Out of Scope
Yang sengaja tidak masuk versi 1:
- pejalan kaki
- sepeda silang jalur khusus
- turning movement kompleks
- multi-lane conflict logic
- multi-intersection coordination
- dynamic route choice
- export CSV/JSON
- AI-based signal optimization
- 3D rendering

## Acceptance Criteria
Versi 1 selesai jika:
- aplikasi bisa dijalankan tanpa crash pada skenario normal
- perubahan durasi lampu memengaruhi antrean secara terukur
- kendaraan keluar layar dihapus dari simulasi
- statistik yang ditampilkan konsisten dengan state runtime
- kode terstruktur cukup rapi untuk modul lanjutan
