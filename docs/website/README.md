# Trang web EasyKey

Landing page tĩnh cho EasyKey, gồm hero, dải số liệu, lưới tính năng,
FAQ accordion và CTA đổi màu.

## Chạy thử

Không cần build — mở trực tiếp bằng trình duyệt:

```
docs/website/index.html
```

Hoặc chạy server cục bộ:

```bash
cd docs/website
python3 -m http.server 8090
# mở http://localhost:8090
```

## Tệp

| Tệp | Vai trò |
|---|---|
| `index.html` | Toàn bộ nội dung trang (tiếng Việt) |
| `style.css` | Giao diện + biến màu **dark / light** qua `[data-theme]` |
| `script.js` | Nút chuyển theme, demo gõ Telex, reveal khi cuộn, FAQ |
| `assets/` | Logo và ảnh chụp màn hình (copy từ `docs/images`, `docs/screenshots`) |

## Dark / Light mode

- Nút mặt trời/mặt trăng trên header chuyển theme, lưu vào `localStorage`.
- Lần đầu truy cập: theo `prefers-color-scheme` của hệ thống; nếu hệ thống
  đổi theme mà người dùng chưa tự chọn, trang đổi theo.
- Theme được áp bằng script nội tuyến trong `<head>` để không bị nháy màu.
- Toàn bộ màu định nghĩa bằng CSS variables — thêm theme mới chỉ cần khối
  `[data-theme="..."]`.

## Bố cục

1. **Header** dính (sticky), blur nền, logo + menu + nút theme + nút tải.
2. **Hero**: tiêu đề, hai nút tải (GitHub/GitLab), mock cửa sổ Cài đặt + popup
   khay — tự đổi màu theo theme.
3. **Dải số liệu**: < 1 MB · 0 telemetry · 3 kiểu gõ · 3 chế độ.
4. **Tính năng**: 8 thẻ, bám sát bảng "Vì sao chọn EasyKey" trong README.
5. **Bắt đầu trong một phút**: 3 bước + 3 demo gõ chữ động
   (Anh–Việt, Esc hoàn tác, gõ tắt).
6. **Ảnh chụp màn hình** thật (sáng/tối/flyout).
7. **Riêng tư**: cam kết không keylogger + terminal minh hoạ.
8. **FAQ** (accordion, đóng mục khác khi mở) + **CTA band** + footer.

## Deploy gợi ý

Toàn bộ thư mục này có thể publish thẳng lên GitHub Pages:
đặt ở nhánh `gh-pages` hoặc bật Pages với thư mục `docs/website`.
Các link tài liệu (`PRIVACY.md`, `CHANGELOG.md`, …) trỏ về repo GitHub
để không phụ thuộc cấu trúc thư mục khi deploy.
