# Thiết kế giao diện

## Phong cách

Phong cách đã chốt là **Cloud**: nền sáng, thẻ trắng đổ bóng mềm, nút xanh dương, số liệu là nhân vật chính.

- Bề mặt nổi có gradient nhẹ, bóng mềm và viền sáng 1 px ở mép trên.
- Nút nổi khi thường, lún và mất bóng khi nhấn, phẳng khi khoá.
- Mỗi đại lượng có màu riêng: nhiệt độ đỏ, độ ẩm xanh dương, ánh sáng vàng cam.
- Bo góc lớn, khoảng trống rộng, ít chữ.

## Thành phần dùng chung

<img src="../img/c1-components.png" width="240" height="436">

| Widget | Vai trò |
|---|---|
| `StatusHeader` | Status bar: giờ, chip kiểm tra hệ thống, icon mạng |
| `BottomNavBar` | Thanh tab Tổng quan / Biểu đồ / Cài đặt |
| `SubPage` | Khung trang con: thanh tiêu đề có nút quay lại, nội dung 240×256 |
| `MetricTile` | Thẻ màu đại lượng, có đồ thị vùng tràn sát mép |
| `ListCard`, `ListRow` | Thẻ danh sách dòng 44 px, ô icon màu bên trái, chevron hoặc giá trị bên phải |
| `KeyValueList` | Thẻ cặp nhãn / giá trị chỉ đọc, giá trị dài bị cắt bằng dấu ba chấm |
| `ScrollList` | Danh sách cuộn, dùng khi không vừa một trang như danh sách vị trí |
| `RaisedButton` | Nút `primary`, `secondary`, `danger` với ba trạng thái |
| `SegmentedControl` | Chọn một trong vài giá trị, cao 36 px |
| `MiniSegment` | Segment nhỏ nằm trong dòng, ví dụ °C/°F |
| `TouchSlider` | Slider có núm 24 px, dễ kéo bằng cảm ứng điện trở |
| `BottomSheet` | Hộp xác nhận trượt từ dưới lên, có nền tối phía trên |
| `ProgressRing` | Vòng tiến độ OTA |
| `StepBar` | Thanh bước Tải về / Cài đặt / Khởi động / Kiểm tra |

Ô icon trong `ListRow` có các màu `default`, `yellow`, `aqua`, `red`, `blue`, `green`. Màu chỉ để nhận ra dòng nhanh, không mang nghĩa trạng thái, trừ đỏ dùng cho thao tác nguy hiểm.

## Danh mục màn hình

```
S1 Tổng quan
S2 Biểu đồ
S3 Cài đặt
├── S3a Màn hình
├── S3b Chu kỳ lấy mẫu
├── Đơn vị nhiệt độ: chỉnh ngay trên dòng
├── S3c Ngôn ngữ & vị trí
│   └── S3e Vị trí
├── S3d Cập nhật firmware
└── S4 Bảo trì
    ├── S4b Thiết bị
    ├── S4j Hệ thống → S4f xác nhận khởi động lại
    ├── S4e Mạng → S4e1 Ethernet, S4e2 Wi-Fi
    ├── S4i Nâng cao (chỉ bản dev)
    │   ├── S4a Firmware & khởi động
    │   ├── S4c Cảm biến
    │   ├── S5a → S5b → S5c Hiệu chỉnh cảm ứng
    │   └── S4d Nhật ký lỗi
    └── S4h Khôi phục cài đặt gốc
S6 Luồng OTA, hiện đè lên mọi trang
```

### Khởi động

| S0b Đang khởi động | S0b Gần xong |
|:---:|:---:|
| <img src="../img/s0b-boot.png" width="200" height="267"> | <img src="../img/s0b2-boot-done.png" width="200" height="267"> |

### Ba tab chính

| Tổng quan | Khi lỗi cảm biến | Biểu đồ | Cài đặt |
|:---:|:---:|:---:|:---:|
| <img src="../img/s1-overview.png" width="200" height="267"> | <img src="../img/s1e-overview-sensor-error.png" width="200" height="267"> | <img src="../img/s2-chart.png" width="200" height="267"> | <img src="../img/s3-settings.png" width="200" height="267"> |

### Cài đặt

| Độ sáng | Chu kỳ lấy mẫu | Ngôn ngữ & vị trí |
|:---:|:---:|:---:|
| <img src="../img/s3a-display.png" width="200" height="267"> | <img src="../img/s3b-sampling.png" width="200" height="267"> | <img src="../img/s3c-language-location.png" width="200" height="267"> |

| Ngôn ngữ & Vị trí | Cập nhật | Bảo trì |
|:---:|:---:|:---:|
| <img src="../img/s3e-location.png" width="200" height="267"> | <img src="../img/s3d-update.png" width="200" height="267"> | <img src="../img/s4-service.png" width="200" height="267"> |

### Bảo trì

| Thiết bị | Hệ thống | Khởi động lại | Khôi phục cài đặt gốc |
|:---:|:---:|:---:|:---:|
| <img src="../img/s4b-device.png" width="200" height="267"> | <img src="../img/s4j-system.png" width="200" height="267"> | <img src="../img/s4f-reboot-confirm.png" width="200" height="267"> | <img src="../img/s4h-factory-reset.png" width="200" height="267"> |

| Mạng | Ethernet chưa nối | Ethernet | Wi-Fi |
|:---:|:---:|:---:|:---:|
| <img src="../img/s4e-network.png" width="200" height="267"> | <img src="../img/s4e3-network-eth-down.png" width="200" height="267"> | <img src="../img/s4e1-ethernet.png" width="200" height="267"> | <img src="../img/s4e2-wifi.png" width="200" height="267"> |

### Nâng cao (chỉ bản dev)

| Nâng cao | Firmware & khởi động | Cảm biến | Nhật ký lỗi |
|:---:|:---:|:---:|:---:|
| <img src="../img/s4i-advanced.png" width="200" height="267"> | <img src="../img/s4a-firmware.png" width="200" height="267"> | <img src="../img/s4c-sensors.png" width="200" height="267"> | <img src="../img/s4d-error-log.png" width="200" height="267"> |

### Hiệu chỉnh cảm ứng

| S5a Giới thiệu | S5b Chạm điểm | S5c Kiểm tra |
|:---:|:---:|:---:|
| <img src="../img/s5a-calib-intro.png" width="200" height="267"> | <img src="../img/s5b-calib-points.png" width="200" height="267"> | <img src="../img/s5c-calib-verify.png" width="200" height="267"> |

### Luồng OTA

Màn OTA dùng chế độ `Takeover`

| Có bản mới | Tải về | Cài đặt | Cài xong |
|:---:|:---:|:---:|:---:|
| <img src="../img/s6a-ota-available.png" width="200" height="267"> | <img src="../img/s6b-ota-downloading.png" width="200" height="267"> | <img src="../img/s6c-ota-installing.png" width="200" height="267"> | <img src="../img/s6d-ota-done.png" width="200" height="267"> |

| Đang khởi động lại | Kiểm tra sau update |
|:---:|:---:|
| <img src="../img/s6e-ota-rebooting.png" width="200" height="267"> | <img src="../img/s6f-ota-verifying.png" width="200" height="267"> |

Khi có lỗi, thanh bước gạch chéo đúng bước hỏng:

| Lỗi khi tải về | Lỗi khi cài đặt | Backup |
|:---:|:---:|:---:|
| <img src="../img/s6g-ota-download-failed.png" width="200" height="267"> | <img src="../img/s6h-ota-install-failed.png" width="200" height="267"> | <img src="../img/s6i-ota-rolled-back.png" width="200" height="267"> |

