# Stage 4 Notes

Tahap 4 dipakai untuk mengubah domain model menjadi perilaku simulasi yang terasa hidup, bukan sekadar data yang bergerak.

## Tujuan

- Memperkuat perilaku kendaraan pada level microsimulation.
- Menjadikan queue, wait time, dan phase transition lebih nyata.
- Menyediakan label ringan untuk HUD/debug overlay tanpa menambah coupling.

## Keputusan Desain

- Helper formatter diletakkan di `include/core/Formatters.hpp` dan dibuat header-only.
- Formatter hanya mengembalikan `std::string_view` supaya murah dan mudah dipakai di UI.
- Label dibuat sederhana dan stabil agar cocok untuk debug panel, log, dan status HUD.
- Tahap ini tetap menjaga pemisahan antara domain model, engine, dan presentasi.

## Pola Pakai

- `direction_label()` untuk label arah kendaraan dan approach.
- `light_label()` untuk status lampu per arah.
- `phase_label()` untuk nama fase sinyal di panel kontrol.
- `vehicle_type_label()` untuk identifikasi tipe kendaraan di overlay debug.

## Batasan

- Tahap ini belum mengubah struktur domain utama.
- Tahap ini belum menambah file konfigurasi baru.
- Tahap ini fokus pada helper yang aman dipakai lintas layer.

## Hasil Yang Diinginkan

- HUD bisa menampilkan status yang lebih jelas.
- Debug overlay bisa menampilkan label yang konsisten.
- Tahap berikutnya dapat memakai formatter yang sama tanpa duplikasi string literal.
