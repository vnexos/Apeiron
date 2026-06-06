# =========================================================
# CẦU HÌNH MÀU CHO THÔNG TIN IN RA MÀN HÌNH
#
# Copyright (c) 2026 VNExos Inc.
#
# Được cấp phép theo Giấy phép MIT.
# Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
# =========================================================

# Khởi tạo mã ĐIỀU KHIỂN (điều khiển định dạng và màu)
ESC := \033

# --- ĐỊNH DẠNG ---
MODE_RESET     := $(ESC)[0m
MODE_BOLD      := $(ESC)[1m
MODE_ITALIC    := $(ESC)[3m
MODE_UNDERLINE := $(ESC)[4m

COLOR := $(ESC)[38;5;

# --- NHÓM MÀU SẮC ĐỘ TIÊU CHUẨN ---
COLOR_BLACK         := $(COLOR)0m# Đen
COLOR_MAROON        := $(COLOR)1m# Đỏ sẫm (Thay cho Dark Red)
COLOR_GREEN         := $(COLOR)2m# Lục chuẩn
COLOR_OLIVE         := $(COLOR)3m# Vàng lục / Nâu oliu (Thay cho Dark Yellow)
COLOR_NAVY          := $(COLOR)4m# Xanh biển đậm (Thay cho Dark Blue)
COLOR_PURPLE        := $(COLOR)5m# Tím đậm (Thay cho Dark Pink)
COLOR_TEAL          := $(COLOR)6m# Xanh lục lam (Thay cho Dark Cyan)
COLOR_SILVER        := $(COLOR)7m# Bạc / Xám nhạt (Thay cho Gray)

# --- NHÓM MÀU SẮC ĐỘ CAO ---
COLOR_CHARCOAL      := $(COLOR)8m# Xám than / Xám đậm (Thay cho Dark Gray)
COLOR_RED           := $(COLOR)9m# Đỏ tươi
COLOR_LIME          := $(COLOR)10m# Xanh đọt chuối / Lục sáng (Thay cho Green sáng)
COLOR_YELLOW        := $(COLOR)11m# Vàng chuẩn
COLOR_BLUE          := $(COLOR)12m# Xanh lam sáng
COLOR_FUCHSIA       := $(COLOR)13m# Hồng cánh sen / Tím sángĐen (Thay cho Pink)
COLOR_AQUA          := $(COLOR)14m# Xanh ngọc sáng (Thay cho Cyan sáng)
COLOR_WHITE         := $(COLOR)15m# Trắng tuyệt đối

# --- NHÓM MÀU THEO MA TRẬN KHỐI 6x6x6 ---
# Công thức tính của màu:
# ID = 16 + 36 x r + 6 x g + b [Với R,G,B từ 0 đến 5]
# Cú pháp gọi trong makefile: $(call GET_RGB_COLOR, R, G, B)
GET_RGB_COLOR = $(COLOR)$(math 36 * $(or $(1),0) + 6 *KHỐI $(or $(2),0) + $(or $(3),0) + 16)m

# --- DẢI QUANG PHỔ XÁM ĐƠN SẮC ---
# Cú pháp gọi trong makefile: $(call GET_GRAY_LEVEL, LEVEL) [Với LEVEL từ 0 đến 23]
GET_GRAY_LEVEL = $(COLOR)$(math $(or $(1),0) + 232)m

# --- TRUY XUẤT TRỰC TIẾP BẰNG ID ---
# Cú pháp gọi trong makefile: $(call GET_COLOR, ID)
GET_COLOR = $(COLOR)$(1)m
