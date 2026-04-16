# Master Roadmap Pengembangan Ulang Sistem Simulasi Lalu Lintas C++

Dokumen ini adalah rencana pengembangan ulang proyek dari nol dengan C++. Semua artefak Python lama diasumsikan sudah dibuang dan tidak menjadi referensi implementasi. Tujuan roadmap ini bukan sekadar daftar tahap besar, tetapi menjadi dokumen kerja yang dapat dipakai sebagai dasar eksekusi proyek secara bertahap, terukur, dan disiplin.

Fokus versi awal:
- simulator lalu lintas 2D desktop
- arsitektur modular
- microsimulation pada level kendaraan individual
- simpang empat bersinyal sebagai cakupan awal
- dapat dikembangkan ke multi-lajur, multi-simpang, dan signal control yang lebih cerdas

Prinsip dasar:
- logika simulasi dipisah tegas dari rendering
- domain model harus stabil sebelum UI diperkaya
- semua fase harus menghasilkan artefak yang bisa diverifikasi
- performa, testability, dan maintainability diprioritaskan sejak awal
- kompleksitas dibuka bertahap, bukan dimasukkan sekaligus di awal

Target hasil akhir:
- aplikasi C++ yang dapat menjalankan simulasi lalu lintas real-time
- parameter sinyal dan demand dapat diubah
- statistik kinerja lalu lintas dapat diamati
- fondasi cukup kuat untuk berkembang menjadi simulator yang lebih serius

---

## Tahap 1: Analisis Kebutuhan, Scope, dan Keputusan Arsitektur

### Tujuan tahap
Tahap ini bertujuan mengunci bentuk produk yang benar sebelum satu baris kode C++ ditulis. Masalah terbesar pada proyek simulasi biasanya bukan kurangnya kode, tetapi scope yang kabur, asumsi domain yang longgar, dan arsitektur yang berubah di tengah jalan. Tahap 1 mencegah itu.

### Pertanyaan inti yang harus dijawab
- Simulator ini dipakai untuk apa:
  - eksperimen pribadi
  - presentasi visual
  - bahan riset
  - fondasi produk teknis
- Tingkat fidelitas seperti apa yang dibutuhkan:
  - visual sederhana
  - operasi lalu lintas realistis tingkat dasar
  - mendekati traffic engineering tool
- Cakupan versi pertama apa:
  - satu simpang
  - empat approach
  - satu lajur per approach
  - kendaraan homogen dan heterogen
  - dua fase lampu
- Apa yang sengaja belum dikerjakan di versi awal:
  - pejalan kaki
  - kendaraan prioritas
  - adaptive controller
  - routing jaringan besar
  - sinkronisasi antar-simpang

### Keputusan teknis yang harus dikunci
- Bahasa utama: `C++20` atau minimal `C++17`
- Build system: `CMake`
- Library rendering/input:
  - opsi 1: `SFML`
  - opsi 2: `SDL2`
  - rekomendasi awal: `SFML` jika prioritas adalah kecepatan pengembangan 2D desktop
- GUI debug/control:
  - manual UI sederhana
  - atau `Dear ImGui` untuk panel tuning dan statistik
- Testing:
  - `Catch2` atau `GoogleTest`
- Formatting/linting:
  - `clang-format`
  - `clang-tidy`
- Dokumentasi:
  - Markdown di `docs/`

### Keputusan domain yang harus ditetapkan
- Jenis simulasi: `microsimulation`
- Unit waktu internal: detik
- Unit jarak internal:
  - meter
  - atau unit abstrak screen-space yang kemudian dipetakan
  - rekomendasi: pakai meter secara domain, pixel hanya untuk rendering
- Resolution update:
  - fixed timestep untuk simulasi
  - render framerate terpisah
- Pendekatan movement:
  - deterministic sederhana pada versi awal
  - baru kemudian stokastik/driver variance

### Artefak yang harus dihasilkan
- `docs/vision.md`
- `docs/scope_v1.md`
- `docs/architecture_decision_record.md`
- `docs/domain_assumptions.md`
- diagram modul tingkat tinggi
- daftar fitur in-scope dan out-of-scope

### Deliverable konkret tahap 1
- definisi versi 1.0 simulator
- keputusan toolchain C++
- blueprint modul
- risiko teknis utama
- asumsi domain yang terdokumentasi

### Risiko yang harus diidentifikasi sejak tahap ini
- simulasi terlalu visual tapi domain lemah
- domain terlalu ambisius sehingga development macet
- coupling antara rendering dan engine
- tidak ada definisi metrik keberhasilan
- desain file dan module boundaries berubah-ubah

### Kriteria selesai
- semua stakeholder internal setuju apa yang akan dibangun dan apa yang sengaja tidak dibangun dulu
- stack teknis final dipilih
- arsitektur konseptual tidak lagi abu-abu

---

## Tahap 2: Fondasi Engineering, Struktur Proyek, dan Runtime Skeleton

### Tujuan tahap
Membangun kerangka proyek C++ yang bersih, portable, bisa di-build, bisa dites, dan siap diisi domain logic. Ini bukan tahap simulasi, tetapi tahap menyiapkan tanahnya.

### Struktur folder yang direkomendasikan
```text
project_root/
  CMakeLists.txt
  cmake/
  external/
  assets/
  docs/
  include/
    app/
    core/
    domain/
    simulation/
    rendering/
    ui/
    analytics/
  src/
    app/
    core/
    domain/
    simulation/
    rendering/
    ui/
    analytics/
  tests/
    unit/
    integration/
  tools/
```

### Pekerjaan inti
- menyiapkan `CMakeLists.txt` utama
- membuat target executable utama
- menyiapkan dependency management
- menyiapkan mode build:
  - Debug
  - Release
- menyiapkan warning policy:
  - treat warnings seriously
  - aktifkan warning level tinggi
- menyiapkan logging abstraction
- menyiapkan game/application loop dasar
- menyiapkan abstraction waktu:
  - timestep
  - elapsed time
  - frame delta

### Modul teknis yang harus ada meski masih kosong
- `Application`
- `WindowSystem`
- `Renderer`
- `SimulationEngine`
- `InputManager`
- `ConfigLoader`
- `Logger`
- `Clock` atau `TimeStep`

### Desain lifecycle aplikasi
1. Inisialisasi dependency
2. Buat window
3. Inisialisasi renderer
4. Inisialisasi simulation engine
5. Main loop
6. Poll input
7. Update simulation dengan fixed timestep
8. Render frame
9. Shutdown rapi

### Standar coding yang harus diputuskan
- aturan penamaan class, method, enum, constant
- pemakaian `unique_ptr`, `shared_ptr`, dan reference
- larangan memory ownership ambigu
- header discipline:
  - forward declaration bila cukup
  - minimalkan include berat di header
- pemisahan interface dan implementation
- kebijakan exception:
  - pakai exception atau error code
  - harus konsisten

### Tooling yang perlu dipasang
- `clang-format`
- `clang-tidy`
- compiler:
  - `MSVC` untuk Windows
  - opsional `clang`/`gcc`
- test runner
- CI dasar jika repository akan diseriuskan

### Artefak yang harus dihasilkan
- aplikasi kosong yang membuka window
- loop update/render dasar
- konfigurasi build yang rapi
- test framework yang sudah bisa menjalankan sample test

### Deliverable konkret tahap 2
- proyek bisa di-clone dan di-build tanpa improvisasi manual
- runtime skeleton bisa jalan
- lingkungan engineering siap untuk pengembangan domain

### Kriteria selesai
- build debug dan release berhasil
- executable berjalan
- test dummy lulus
- struktur proyek tidak perlu dirombak lagi saat domain mulai masuk

---

## Tahap 3: Model Domain Lalu Lintas dan State Representation

### Tujuan tahap
Menetapkan representasi formal dari elemen lalu lintas yang akan disimulasikan. Tahap ini sangat penting karena semua logika di tahap-tahap berikutnya bergantung pada kualitas model domain.

### Entitas domain yang wajib didefinisikan
- `Direction`
- `Movement`
- `TurnIntent`
- `VehicleType`
- `SignalGroup`
- `LightState`
- `Lane`
- `Approach`
- `RoadSegment`
- `StopLine`
- `ConflictZone`
- `Intersection`
- `TrafficSignalPlan`
- `TrafficLightController`
- `Vehicle`
- `VehicleProfile`
- `SimulationState`
- `SimulationConfig`

### Atribut domain yang perlu dirancang

Untuk `Vehicle`:
- unique id
- vehicle type
- current lane
- current position
- speed
- acceleration
- desired speed
- length
- width
- route intent
- wait time
- state flags:
  - active
  - queued
  - in conflict zone
  - exited

Untuk `Lane`:
- lane id
- allowed movements
- geometric start-end
- queue storage
- spawn point
- stop line position

Untuk `TrafficLightController`:
- active phase
- phase timer
- phase definitions
- amber timing
- all-red opsional
- phase transition rules

Untuk `Intersection`:
- approach list
- lane topology
- geometry
- conflict area
- stop lines
- controller association

### Keputusan domain yang harus dibuat
- apakah lane menyimpan kendaraan sebagai container sendiri atau engine menyimpan global list plus indexing
- apakah path kendaraan dipandang sebagai lane-following path atau free movement vector
- bagaimana definisi delay:
  - total stopped delay
  - control delay
  - queue delay
- bagaimana definisi queue:
  - speed threshold
  - jarak ke stop line
- apakah ada random seed yang dikontrol

### Data model yang harus dipisah
- configuration data
- static map geometry
- runtime simulation state
- analytics snapshot

### Fokus kualitas desain tahap ini
- domain object tidak bergantung pada renderer
- enum dan state machine jelas
- relasi antar-object tidak menyebabkan dependency spiral
- serialization atau debug dump dimungkinkan

### Artefak yang harus dihasilkan
- class diagram atau tabel struktur domain
- definisi field penting setiap entitas
- dokumen alur state kendaraan
- definisi fase sinyal dan aturan transisi

### Deliverable konkret tahap 3
- desain domain final versi awal
- object model siap diimplementasikan
- tidak ada kebingungan lagi antara logika lalu lintas dan logika tampilan

### Kriteria selesai
- semua entitas utama sudah punya tanggung jawab yang tegas
- hubungan antar modul domain dapat dijelaskan tanpa ambigu
- tidak ada state penting yang "nanti dipikirkan"

---

## Tahap 4: Engine Simulasi, Logika Operasional, dan Perilaku Kendaraan

### Tujuan tahap
Membuat simulator benar-benar hidup. Pada tahap ini kendaraan mulai spawn, bergerak, berhenti, antre, bereaksi terhadap sinyal, dan meninggalkan area simulasi.

### Komponen utama yang harus dibangun
- `SimulationEngine`
- `Spawner`
- `VehicleUpdateSystem`
- `SignalUpdateSystem`
- `QueueAnalysisSystem`
- `LifecycleSystem`
- `MetricsCollector`

### Urutan update yang direkomendasikan
1. Baca input perubahan parameter
2. Update sinyal lalu lintas
3. Spawn kendaraan baru
4. Hitung target behavior tiap kendaraan
5. Update speed/acceleration
6. Update position
7. Deteksi exit/out-of-bounds
8. Update queue metrics
9. Simpan statistik frame atau interval

### Perilaku kendaraan minimum versi awal
- spawn dari empat approach
- punya jenis kendaraan berbeda
- menjaga jarak aman
- berhenti sebelum stop line saat merah
- bisa lanjut saat hijau
- delay reaksi kecil saat lampu berubah hijau
- belok dapat ditunda sampai versi lanjut, atau disediakan hanya untuk skenario tertentu jika memang diprioritaskan

### Model perilaku yang perlu dipilih
- car-following model sederhana:
  - berbasis safe distance
  - berbasis target speed
- braking logic
- acceleration logic
- queue joining logic
- release logic saat green

### State machine kendaraan yang perlu jelas
- `Spawning`
- `Cruising`
- `ApproachingSignal`
- `Decelerating`
- `Queued`
- `Discharging`
- `CrossingIntersection`
- `Exiting`
- `Completed`

### Signal control versi awal
- dua fase:
  - utara-selatan hijau
  - timur-barat hijau
- amber tetap
- opsional all-red
- durasi fase configurable

### Statistik yang harus dihitung
- total kendaraan masuk
- total kendaraan keluar
- kendaraan aktif
- average wait time
- queue length per approach
- throughput per phase
- average speed
- phase utilization

### Aspek penting yang tidak boleh disepelekan
- fixed timestep agar simulasi stabil
- pemisahan update logic dan render frame
- penghapusan entity harus aman
- tidak boleh ada iterator invalidation liar
- random generation harus reproducible jika seed sama

### Pengujian yang perlu dilakukan di tahap ini
- kendaraan spawn pada lane benar
- kendaraan berhenti di lampu merah
- kendaraan tidak menabrak kendaraan di depan
- queue terbentuk saat demand tinggi
- vehicle throughput meningkat saat green
- kendaraan keluar area terhapus dengan benar

### Deliverable konkret tahap 4
- simulasi core berjalan tanpa harus terlihat cantik
- perilaku utama lalu lintas sudah muncul
- metrik dasar sudah dapat dikumpulkan

### Kriteria selesai
- engine dapat menjalankan skenario simpang tunggal stabil dalam waktu lama
- tidak ada behavior absurd yang jelas
- statistik dasar tidak kosong dan masuk akal

---

## Tahap 5: Rendering 2D, Antarmuka Interaktif, dan Workflow Eksperimen

### Tujuan tahap
Mengubah engine yang sudah bekerja menjadi alat eksperimen yang bisa dipakai manusia. Tahap ini fokus pada keterbacaan visual, kontrol simulasi, dan kejelasan hasil.

### Komponen visual yang harus ada
- background area simulasi
- geometri jalan
- marka lajur
- stop line
- zebra crossing jika dibutuhkan
- traffic signal heads
- kendaraan per tipe
- panel statistik
- panel kontrol parameter

### Tujuan visualisasi
- status lalu lintas dapat dipahami dalam beberapa detik
- queue terlihat jelas
- perubahan fase sinyal terlihat jelas
- perbedaan vehicle type terlihat jelas
- tidak ada clutter visual yang mengganggu pembacaan

### Desain UI minimum
- tombol:
  - start
  - pause
  - reset
- slider atau input:
  - durasi hijau utara-selatan
  - durasi hijau timur-barat
  - spawn rate
  - speed multiplier opsional
- tampilan statistik:
  - total spawn
  - completed vehicles
  - active vehicles
  - queue NS
  - queue EW
  - avg wait NS
  - avg wait EW

### Fitur observabilitas yang disarankan
- current phase label
- countdown phase timer
- highlight lane yang sedang hijau
- debug overlay:
  - bounding boxes
  - lane id
  - stop line marker
  - queue detection zone
- panel status runtime:
  - FPS
  - update time
  - entity count

### Keputusan UI/UX penting
- apakah UI dibuat langsung di canvas/rendering engine
- apakah memakai `Dear ImGui` sebagai control panel
- bagaimana membedakan mode:
  - simulation mode
  - debug mode
- apakah parameter bisa berubah real-time atau hanya saat reset

### Kebutuhan teknis rendering
- kamera statis versi awal
- coordinate transform world-to-screen
- layering:
  - road
  - markings
  - signals
  - vehicles
  - UI
- text rendering yang stabil

### Workflow eksperimen yang harus didukung
1. user menjalankan aplikasi
2. user mengubah green split
3. user mengatur demand
4. simulasi berjalan
5. statistik berubah real-time
6. user pause untuk inspeksi
7. user reset dan bandingkan skenario lain

### Deliverable konkret tahap 5
- simulator usable secara visual
- parameter dapat diubah dengan jelas
- hasil eksperimen dapat dibaca tanpa harus membuka debugger

### Kriteria selesai
- orang lain bisa menjalankan simulator dan memahami apa yang terjadi tanpa penjelasan panjang
- UI cukup stabil untuk workflow eksperimen berulang

---

## Tahap 6: Validasi, Ekstensi Fitur, Optimasi, dan Persiapan Skala Lanjut

### Tujuan tahap
Menaikkan proyek dari prototipe yang bekerja menjadi sistem yang layak dikembangkan secara serius. Tahap ini menutup celah kualitas, performa, reproducibility, dan arah ekspansi.

### Jalur kerja tahap ini

#### 6.1 Validasi domain
- cek apakah queue terbentuk masuk akal
- cek apakah delay meningkat saat demand dinaikkan
- cek apakah throughput berubah saat green split diubah
- cek apakah metrik konsisten antar-run jika seed sama
- cek apakah perilaku release queue realistis secara dasar

#### 6.2 Pengujian formal
- unit test untuk:
  - state light transition
  - queue detector
  - vehicle safe distance logic
  - spawn rules
  - metric aggregation
- integration test untuk:
  - skenario red-to-green discharge
  - skenario oversaturation
  - skenario low demand
- regression test untuk bug yang pernah muncul

#### 6.3 Optimasi performa
- profiling update loop
- profiling rendering
- pengurangan alokasi dinamis per frame
- pemilihan container yang tepat
- cache-friendly data access bila nanti skala naik
- pertimbangan ECS hanya jika benar-benar dibutuhkan, bukan karena tren

#### 6.4 Peningkatan fitur
- belok kiri
- belok kanan
- beberapa lajur per approach
- conflict movement yang lebih realistis
- yellow decision logic yang lebih baik
- all-red timing
- lane-based spawning distribution
- class-specific behavior:
  - motor
  - mobil
  - truk
- pedestrian phase
- adaptive traffic control
- import layout dari file konfigurasi
- export statistik ke CSV/JSON

#### 6.5 Kualitas engineering
- config file external
- save/load scenario
- deterministic replay
- random seed control
- structured logging
- crash-safe shutdown
- documentation refresh

#### 6.6 Packaging dan distribusi
- build script final
- dependency packaging
- release artifact
- user guide singkat
- sample scenario bawaan

### Roadmap fitur setelah versi 1 stabil
- multi-intersection corridor
- jaringan kecil dengan route choice
- sinkronisasi sinyal
- demand pattern berbasis waktu
- machine-learning-based signal policy
- integrasi peta atau layout editor

### Deliverable konkret tahap 6
- simulator stabil
- metrik dapat dipercaya untuk skenario dasar
- fondasi siap berkembang ke model yang lebih kompleks

### Kriteria selesai
- ada baseline performa
- ada baseline validasi
- ada test suite minimal yang menjaga proyek dari regresi
- arah pengembangan versi berikutnya sudah nyata

---

## Rencana Artefak Per Tahap

### Artefak Tahap 1
- dokumen visi
- scope v1
- ADR toolchain
- asumsi domain

### Artefak Tahap 2
- struktur repo
- CMake build
- app skeleton
- test skeleton

### Artefak Tahap 3
- model domain
- class design
- state definitions

### Artefak Tahap 4
- engine simulasi
- update systems
- statistik dasar

### Artefak Tahap 5
- rendering 2D
- panel kontrol
- dashboard statistik

### Artefak Tahap 6
- validasi
- test suite
- optimasi
- fitur lanjutan

---

## Risiko Besar Proyek dan Strategi Mitigasi

### Risiko 1: Scope melebar terlalu cepat
Mitigasi:
- kunci versi awal hanya pada simpang tunggal
- semua fitur tambahan masuk backlog, bukan implementasi spontan

### Risiko 2: Rendering dan engine terlalu menyatu
Mitigasi:
- larang domain object bergantung pada API rendering
- renderer hanya membaca state, bukan memegang otoritas logika

### Risiko 3: Simulasi tidak stabil karena timestep buruk
Mitigasi:
- gunakan fixed timestep untuk update
- render dipisah dari simulation step

### Risiko 4: Data structure tidak tahan skala
Mitigasi:
- mulai dari desain sederhana tapi disiplin
- profiling sebelum optimasi
- refactor berbasis bukti, bukan tebakan

### Risiko 5: Statistik ada tapi tidak bermakna
Mitigasi:
- definisikan metrik secara eksplisit sejak awal
- bedakan queue length, delay, throughput, average speed

### Risiko 6: Proyek terlalu teknis tapi sulit dipakai
Mitigasi:
- Tahap 5 harus dianggap penting, bukan kosmetik
- buat workflow eksperimen yang nyata

---

## Definisi Done Tingkat Proyek

Proyek dianggap mencapai milestone versi awal jika:
- simulator desktop C++ dapat dijalankan stabil
- simpang tunggal empat arah tersimulasikan
- kendaraan individual spawn, bergerak, queue, dan exit
- sinyal dua fase bekerja
- parameter utama dapat diubah
- statistik dasar tampil real-time
- test dasar ada
- arsitektur cukup bersih untuk ekspansi

---

## Urutan Eksekusi yang Direkomendasikan

1. Finalisasi Tahap 1 tanpa kompromi.
2. Bangun skeleton engineering Tahap 2.
3. Kunci domain model Tahap 3.
4. Baru masuk engine Tahap 4.
5. Setelah behavior inti stabil, kerjakan visual dan kontrol di Tahap 5.
6. Tahap 6 dipakai untuk membuat sistem ini layak dilanjutkan, bukan sekadar "fitur tambahan".

---

## Catatan Penutup

Kalau proyek ini ingin serius, kesalahan terbesar adalah langsung menulis kode visual tanpa domain dan engine yang tegas. Untuk simulator lalu lintas, kualitas arsitektur dan definisi perilaku lebih penting daripada banyaknya fitur awal. Roadmap ini sengaja memaksa urutan yang disiplin: tentukan masalah, siapkan fondasi, bentuk domain, bangun engine, baru poles interaksi dan kualitas lanjut.
