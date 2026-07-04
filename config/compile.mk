# =========================================================
# MẪU BIÊN DỊCH CÁC TỆP MÃ LỆNH
#
# Copyright (c) 2026 VNExos Inc.
#
# Được cấp phép theo Giấy phép MIT.
# Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
# =========================================================

define COMPILE_TEMPLATE

$(2)/%.o: $(1)/%.cpp
	@mkdir -p $$(dir $$@)
	@echo "$$(MSG_CXX) Đang biên dịch: $$@"
	@$$(CXX) $(3) $(5) -c $$< -o $$@

$(2)/%.s.o: $(1)/%.s
	@mkdir -p $$(dir $$@)
	@echo "$$(MSG_ASM) Đang biên dịch: $$@"
	@$$(ASM) $(4) -c $$< -o $$@

endef
