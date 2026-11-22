# sensornode-ui

Ứng dụng HMI Qt5 cho [SensorNode-BBB](https://github.com/Leminuos/SensorNode-BBB) - sensor node Embedded Linux trên BeagleBone Black.

## Overview

Ứng dụng Qt5 chạy fullscreen trên màn hình TFT 240×320 của BBB. Vai trò:
- Hiển thị số liệu nhiệt độ / độ ẩm / ánh sáng và biểu đồ lịch sử.
- Chỉnh độ sáng màn hình (PWM) và cập nhật firmware OTA ngay trên UI.

## Cấu trúc

```
sensornode-ui/
├── CMakeLists.txt
├── tools/deploy.sh             # cross-build + inject binary lên board qua SSH
├── docs/                       # hướng dẫn phát triển UI (screen, shared widget, tag .ui)
├── tests/                      # Qt Test chạy trên host, daemon giả qua QLocalServer
├── src/
│   ├── main.cpp                # entrypoint: QApplication + Theme + AppWindow
│   ├── app/
│   │   ├── AppWindow.*         # sở hữu model + UI, nơi duy nhất nối các thành phần
│   │   └── Theme.*             # gộp các file QSS thành stylesheet của app
│   ├── ipc/
│   │   ├── DaemonClient.*      # client JSON Lines qua QLocalSocket, tự kết nối lại
│   │   └── DaemonEndpoints.h   # socket / DB của sensornode-core (override qua env)
│   ├── model/                  # QObject, không có QWidget
│   │   ├── Sample.h            # một mẫu số liệu (timestamp, °C, %, lx; NaN = cảm biến lỗi)
│   │   ├── TelemetryModel.*    # client telemetryd, phát sampleReceived
│   │   ├── HistoryModel.*      # đọc lịch sử từ SQLite read-only, gom theo bucket
│   │   ├── OtaModel.*          # client otad + popup / manual check
│   │   ├── SettingsModel.*     # client settingsd: thông số người dùng, factory reset
│   │   ├── DisplayModel.*      # ghi độ sáng vào sysfs backlight, báo STATUS cho systemd
│   │   └── OtaState.h          # enum trạng thái luồng OTA phía UI
│   ├── screens/                # view thuần: nhận dữ liệu qua slot, phát ý định qua signal
│   │   ├── HomeScreen.*        # tab Home — nhiệt độ / độ ẩm / ánh sáng
│   │   ├── ChartScreen.*       # tab Dashboard — 3 chart Qt Charts (1h/6h/24h)
│   │   ├── SettingsScreen.*    # backlight + chế độ OTA + version
│   │   ├── UpdatePopup.*       # popup version mới
│   │   └── UpdateProgressScreen.*  # màn tiến độ update (download -> flash -> done)
│   ├── widgets/                # shared widget
│   │   ├── BottomNavBar.*      # nav bar Home/Dashboard/Settings
│   │   └── StatusHeader.*      # chỉ báo online/offline
└── resources/                  # resources.qrc — icons + QSS stylesheet
```

## Yêu cầu build

- Qt 5.15: `Widgets`, `Network`, `Charts`
- `sqlite3`
- CMake ≥ 3.16, `pkg-config`, compiler C++17
