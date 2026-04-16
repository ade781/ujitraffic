# Vision

## Product Definition
Sistem ini adalah simulator lalu lintas 2D desktop berbasis C++ untuk memodelkan perilaku kendaraan pada simpang empat bersinyal. Targetnya adalah microsimulation, yaitu setiap kendaraan diperlakukan sebagai entitas individual yang bergerak, berhenti, antre, dan merespons sinyal lalu lintas secara real time.

## Why This Exists
Proyek ini dibangun untuk tiga tujuan utama:
- mengevaluasi dampak perubahan durasi sinyal terhadap antrean dan waktu tunggu
- menyediakan lingkungan eksperimen yang deterministik dan bisa diulang
- menjadi fondasi arsitektur C++ yang cukup rapi untuk dikembangkan ke simpang jamak, lajur jamak, dan kontrol adaptif

## What Success Looks Like
Versi awal dinyatakan berhasil jika:
- aplikasi desktop C++ dapat dijalankan stabil
- kendaraan spawn dari empat approach dan bergerak sesuai aturan lajur kiri
- lampu lalu lintas dua fase bekerja benar
- queue length dan wait time dapat diamati secara real time
- parameter utama dapat diubah tanpa mengubah kode

## Design Principles
- Logika simulasi tidak boleh bergantung pada rendering.
- Domain model harus bisa diuji tanpa window.
- Setiap entitas penting harus punya tanggung jawab tunggal.
- Perilaku awal harus sederhana namun benar secara domain.
- Parameterisasi lebih penting daripada hardcode.

## Audience
Pengguna utama sistem ini adalah:
- developer yang sedang menguji perilaku simulator
- orang yang ingin mengamati efek timing lampu terhadap arus lalu lintas
- pengembang yang akan memperluas simulasi ke skala yang lebih besar

## Non-Goals
Versi awal tidak menargetkan:
- jaringan jalan besar
- routing dinamis
- pejalan kaki
- kendaraan darurat atau prioritas
- visual 3D
- optimasi berbasis AI

## Long-Term Direction
Setelah versi awal stabil, sistem dapat berkembang ke:
- beberapa simpang yang saling terhubung
- sinyal adaptif berbasis kondisi antrean
- penyimpanan konfigurasi dan replay skenario
- visual debugging yang lebih kaya
