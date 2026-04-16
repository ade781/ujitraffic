# Architecture Decision Record

## ADR 001: Use C++20 for the simulator codebase

### Status
Accepted

### Context
Proyek ini akan dibangun ulang dari nol dan membutuhkan baseline modern untuk modularitas, type safety, dan maintainability.

### Decision
Gunakan `C++20` sebagai target utama. Jika lingkungan build terbatas, `C++17` masih diterima sebagai fallback minimum.

### Consequences
- fitur bahasa modern dapat dipakai
- codebase lebih mudah dipisah ke modul kecil
- build toolchain harus jelas sejak awal

## ADR 002: Use CMake as the build system

### Status
Accepted

### Context
Proyek akan berkembang ke beberapa target dan kemungkinan memerlukan dependency eksternal.

### Decision
Gunakan `CMake` sebagai build system utama.

### Consequences
- build dapat dipisah per target
- integrasi library eksternal lebih mudah
- struktur repo harus mengikuti praktik CMake yang rapi

## ADR 003: Build a 2D desktop simulator with SFML as the primary runtime library

### Status
Accepted

### Context
Versi awal menargetkan pengembangan cepat untuk simulator 2D desktop, bukan engine grafis kompleks.

### Decision
Gunakan `SFML` sebagai runtime library utama untuk windowing, input, timing, dan rendering 2D.

### Consequences
- implementasi lebih cepat dibanding membangun stack grafis mentah
- API lebih ringan untuk prototipe simulasi
- visual 2D sudah cukup untuk scope v1

## ADR 004: Keep simulation logic separate from rendering

### Status
Accepted

### Context
Simulator lalu lintas akan sulit dipelihara jika rendering dan domain logic saling bercampur.

### Decision
Pisahkan engine simulasi dari layer rendering dan UI.

### Consequences
- domain model bisa diuji tanpa window
- rendering dapat diganti tanpa memengaruhi logika inti
- refactor lebih aman saat fitur bertambah

## ADR 005: Use fixed timestep for simulation updates

### Status
Accepted

### Context
Traffic simulation sensitif terhadap variasi frame rate.

### Decision
Update simulasi menggunakan fixed timestep. Rendering boleh berjalan pada frame rate terpisah.

### Consequences
- hasil lebih stabil dan reproducible
- behavior kendaraan tidak terlalu dipengaruhi performa mesin
- implementasi sedikit lebih kompleks, tetapi jauh lebih benar

## ADR 006: Use explicit domain models for traffic entities

### Status
Accepted

### Context
Kendaraan, lane, intersection, dan traffic signal harus memiliki state yang jelas.

### Decision
Definisikan model domain eksplisit untuk entitas utama:
- Vehicle
- Lane
- Intersection
- TrafficLightController
- SimulationConfig
- SimulationState

### Consequences
- state lebih mudah dibaca dan dites
- sistem lebih siap untuk ekspansi
- debug dan observability lebih baik

## ADR 007: Prioritize testability and deterministic behavior

### Status
Accepted

### Context
Simulator akan dipakai untuk membandingkan skenario, jadi hasil harus bisa diulang.

### Decision
Sistem harus mendukung deterministic behavior melalui konfigurasi seed dan aturan update yang konsisten.

### Consequences
- debugging lebih mudah
- eksperimen lebih valid
- bug lebih mudah direproduksi

## ADR 008: Use CTest with a modern C++ unit testing framework

### Status
Accepted

### Context
Setiap layer inti perlu pengujian minimal.

### Decision
Gunakan `Catch2` atau `GoogleTest` untuk unit testing dan integrasikan melalui CTest.

### Consequences
- logic dapat divalidasi otomatis
- regression lebih mudah dicegah
- CI lebih sederhana untuk disiapkan

## Open Questions
- apakah UI kontrol awal akan memakai `Dear ImGui` atau UI custom
- apakah asset grafis awal akan berbasis sprite atau bentuk primitives
- apakah versi v1 akan langsung menyimpan konfigurasi ke file
