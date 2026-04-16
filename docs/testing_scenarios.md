# Testing Scenarios

Dokumen ini bukan loader resmi. Ini fondasi format skenario kecil untuk regression test, observability, dan diskusi perilaku simulasi tanpa membuka UI.

## Tujuan

- Menyimpan skenario eksperimen yang bisa direproduksi.
- Membekukan seed, timing signal, dan kondisi awal kendaraan untuk debugging.
- Menjadi target format jika nanti repo menambah loader scenario eksternal.

## Bentuk Data Yang Disarankan

```json
{
  "name": "single_intersection_queue_probe",
  "seed": 1337,
  "sim_size": { "width": 800, "height": 600 },
  "timing": {
    "north_south_green_seconds": 12.0,
    "east_west_green_seconds": 12.0,
    "yellow_seconds": 3.0
  },
  "demand": {
    "spawn_probability": 0.0,
    "spawn_interval_seconds": 0.2,
    "reaction_time_min_seconds": 0.0,
    "reaction_time_max_seconds": 0.0
  },
  "metrics": {
    "queue_speed_threshold": 12.0,
    "wait_speed_threshold": 1.5
  },
  "geometry": {
    "center": { "x": 400.0, "y": 300.0 },
    "road_width": 180.0,
    "lane_width": 60.0,
    "stop_line_distance": 90.0,
    "queue_distance": 220.0,
    "center_size": 120.0
  },
  "vehicles": [
    {
      "id": 1,
      "type": "Car",
      "direction": "North",
      "turn_intent": "Straight",
      "position": { "x": 430.0, "y": 320.0 },
      "speed": 0.0,
      "wait_time": 0.0
    }
  ],
  "expectations": {
    "after_seconds": 0.5,
    "north_south_queue_min": 1,
    "north_south_wait_time_min": 0.5
  }
}
```

## Kegunaan Langsung Untuk Test Saat Ini

- `TrafficLight` regression:
  Kunci timing dan verifikasi urutan fase, `time_remaining_seconds`, dan `cycle_count`.

- `Vehicle` rule regression:
  Bekukan posisi kendaraan dekat stop line atau center box untuk memverifikasi aturan berhenti, belok, gap, dan movement.

- `SimulationState` metrics regression:
  Inisialisasi kendaraan manual lalu verifikasi queue count, average wait, completed wait, dan pruning inactive vehicles.

- `SimulationEngine` integration regression:
  Kunci `seed`, `spawn_probability`, `spawn_interval_seconds`, reaction time, dan geometri untuk memastikan snapshot spawn dan metric update tetap deterministik.

## Saran Evolusi Tahap Berikutnya

- Tambahkan schema validator sederhana untuk file scenario.
- Simpan golden scenario untuk kasus lampu, queue buildup, dan offscreen completion.
- Tambahkan export observability yang menulis snapshot runtime ke format serupa agar bug report bisa direplay.
