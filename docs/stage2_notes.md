# Stage 2 Notes

Tahap 2 membangun fondasi runtime yang bisa dipakai oleh seluruh modul lain tanpa tergantung pada detail rendering atau simulasi.

## Tujuan

- Menyediakan helper fixed timestep untuk update deterministik.
- Menyediakan logger minimal yang aman dipakai lintas modul.
- Menjaga struktur proyek tetap modular sebelum engine simulasi yang sesungguhnya masuk.

## Keputusan Teknis

- `FixedTimestep` disediakan sebagai helper kecil, header-only, dan tanpa dependency eksternal.
- `Logger` dibuat thread-safe secara dasar dengan mutex internal.
- Interface logger sengaja kecil supaya bisa dipakai oleh `app`, `simulation`, `rendering`, dan `ui` tanpa coupling berlebihan.

## Pola Pakai

- `FixedTimestep` dipakai di loop utama untuk menyerap delta waktu frame lalu mengeluarkan step simulasi konstan.
- `Logger` dipakai melalui `default_logger()` untuk kebutuhan cepat, atau lewat instance eksplisit jika modul membutuhkan kontrol output yang berbeda.

## Batasan

- Tahap ini belum menambahkan sistem konfigurasi, command line parsing, atau file loader.
- Tahap ini belum mengubah logika simulasi kendaraan.
- Tahap ini fokus pada fondasi runtime, bukan fitur domain.

## Kriteria Lanjut

- helper timestep bisa dipakai tanpa modifikasi tambahan pada modul lain.
- logger bisa dipanggil dari beberapa modul tanpa saling berbagi state yang tidak jelas.
- struktur tahap berikutnya bisa dibangun di atas fondasi ini tanpa refactor besar.
