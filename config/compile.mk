# =========================================================
# MẪU BIÊN DỊCH CÁC TỆP MÃ LỆNH
#
# Copyright (c) 2026 VNExos Inc.
#
# Được cấp phép theo Giấy phép MIT.
# Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
# =========================================================

BOOT_ARCH_X86_64_CXX_FLAGS  := -mno-red-zone
BOOT_ARCH_AARCH64_CXX_FLAGS := -mno-implicit-float
BOOT_ARCH_RISCV64_CXX_FLAGS := -march=rv64imac -mno-implicit-float -mabi=lp64 -mcmodel=medany -mno-relax -msmall-data-limit=0 -fno-jump-tables -Wno-ignored-attributes

# Biên dịch cho từng dòng Vi xử lý được hỗ trợ cho chương trình giai đoạn khởi động
# 1: Thư mục chứa mã nguồn
# 2: Thư mục chứa tệp đối tượng
# 3: Các cờ biên dịch C++
# 4: Các cờ biên dịch Assembly
# 5: Các cờ tùy chỉnh cho C++
define HYPER_BOOT_COMPILE_TEMPLATE
$(2)/%.x86_64.o: $(1)/%.cpp
	@mkdir -p $$(dir $$@)
	@echo "$$(MSG_CXX) Đang biên dịch cho Vi xử lý x86_64: $$@"
	@$$(CXX) $(3) $(5) $$(BOOT_ARCH_X86_64_CXX_FLAGS) --target=x86_64-unknown-windows -fshort-wchar -mno-stack-arg-probe -c $$< -o $$@

$(2)/%.aarch64.o: $(1)/%.cpp
	@mkdir -p $$(dir $$@)
	@echo "$$(MSG_CXX) Đang biên dịch cho Vi xử lý aarch64: $$@"
	@$$(CXX) $(3) $(5) $$(BOOT_ARCH_AARCH64_CXX_FLAGS) --target=aarch64-unknown-windows -fshort-wchar -mno-stack-arg-probe -c $$< -o $$@

$(2)/%.riscv64.o: $(1)/%.cpp
	@mkdir -p $$(dir $$@)
	@echo "$$(MSG_CXX) Đang biên dịch cho Vi xử lý riscv64: $$@"
	@$$(CXX) $(3) $(5) $$(BOOT_ARCH_RISCV64_CXX_FLAGS) --target=riscv64-unknown-none-elf -fshort-wchar -mno-stack-arg-probe -c $$< -o $$@

$(2)/%.x86_64.s.o: $(1)/%.s
	@mkdir -p $$(dir $$@)
	@echo "$$(MSG_ASM) Đang biên dịch cho Vi xử lý x86_64: $$@"
	@$$(ASM) $(4) --target=x86_64-unknown-windows -c $$< -o $$@

$(2)/%.aarch64.s.o: $(1)/%.s
	@mkdir -p $$(dir $$@)
	@echo "$$(MSG_ASM) Đang biên dịch cho Vi xử lý aarch64: $$@"
	@$$(ASM) $(4) --target=aarch64-unknown-windows -c $$< -o $$@

$(2)/%.riscv64.s.o: $(1)/%.s
	@mkdir -p $$(dir $$@)
	@echo "$$(MSG_ASM) Đang biên dịch cho Vi xử lý riscv64: $$@"
	@$$(ASM) $(4) --target=riscv64-unknown-none-elf -c $$< -o $$@

endef

# Biên dịch cho từng dòng Vi xử lý được hỗ trợ
# 1: Thư mục chứa mã nguồn
# 2: Thư mục chứa tệp đối tượng
# 3: Các cờ biên dịch C++
# 4: Các cờ biên dịch Assembly
# 5: Các cờ tùy chỉnh cho C++
define HYPER_COMPILE_TEMPLATE
$(2)/%.x86_64.o: $(1)/%.cpp
	@mkdir -p $$(dir $$@)
	@echo "$$(MSG_CXX) Đang biên dịch cho Vi xử lý x86_64: $$@"
	@$$(CXX) $(3) $(5) $$(BOOT_ARCH_X86_64_CXX_FLAGS) --target=x86_64 -ffunction-sections -c $$< -o $$@

$(2)/%.aarch64.o: $(1)/%.cpp
	@mkdir -p $$(dir $$@)
	@echo "$$(MSG_CXX) Đang biên dịch cho Vi xử lý aarch64: $$@"
	@$$(CXX) $(3) $(5) $$(BOOT_ARCH_AARCH64_CXX_FLAGS) --target=aarch64 -ffunction-sections -c $$< -o $$@

$(2)/%.riscv64.o: $(1)/%.cpp
	@mkdir -p $$(dir $$@)
	@echo "$$(MSG_CXX) Đang biên dịch cho Vi xử lý riscv64: $$@"
	@$$(CXX) $(3) $(5) $$(BOOT_ARCH_RISCV64_CXX_FLAGS) --target=riscv64 -ffunction-sections -c $$< -o $$@

$(2)/%.x86_64.s.o: $(1)/%.s
	@mkdir -p $$(dir $$@)
	@echo "$$(MSG_ASM) Đang biên dịch cho Vi xử lý x86_64: $$@"
	@$$(ASM) $(4) --target=x86_64 -c $$< -o $$@

$(2)/%.aarch64.s.o: $(1)/%.s
	@mkdir -p $$(dir $$@)
	@echo "$$(MSG_ASM) Đang biên dịch cho Vi xử lý aarch64: $$@"
	@$$(ASM) $(4) --target=aarch64 -c $$< -o $$@

$(2)/%.riscv64.s.o: $(1)/%.s
	@mkdir -p $$(dir $$@)
	@echo "$$(MSG_ASM) Đang biên dịch cho Vi xử lý riscv64: $$@"
	@$$(ASM) $(4) --target=riscv64 -c $$< -o $$@
endef
