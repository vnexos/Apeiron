<div align="center">
  <h1>VNExos Bản Nguyên</h1>
  <p><em>VNExos Apeiron</em></p>
</div>

### 📖 Mục lục
| STT   | Mục                                                          |
|------:|--------------------------------------------------------------|
| 1     | [Giới thiệu](#-giới-thiệu)                                   |
| 2     | [Công nghệ sử dụng](#️-công-nghệ-sử-dụng)                    |
| 3     | [Cây thư mục](#-cây-thư-mục)                                 |
| 4     | [Quy ước ghi chú cam kết](#-quy-ước-ghi-chú-cam-kết)         |
| 5     | [Tên trong các ngôn ngữ khác](#-tên-trong-các-ngôn-ngữ-khác) |

---
### 📃 Giới thiệu
- Hệ điều hành **VNExos Bản Nguyên** (tiếng Anh là: **VNExos Apeiron OS**) là hệ điều hành được xây dựng bởi VNExos. Dự án mang theo khát vọng đóng góp một nền tảng vững chắc cho sự phát triển công nghệ của Việt Nam và vươn tầm thế giới.
- Đây là dự án mã nguồn hỗn hợp (đóng một phần) để đảm bảo sự bảo mật cho những chương trình cốt lõi của **VNExos Bản Nguyên**.
- Các tài liệu bên trong dự án hoặc chú thích bên trong mã nguồn trừ các tên riêng đều sẽ được viết bằng tiếng Việt.

### 🖥️ Công nghệ sử dụng
| Thành phần   |Tên | Mục đích |
|:-----------:|:---|----------|
| ![Assembly](https://img.shields.io/badge/assembly-%23000000.svg?style=for-the-badge&logo=assemblyscript&logoColor=white) | **Assembly** <br>*(Hợp ngữ)* | Được dùng chủ yếu ở **Bộ nạp khởi động** và **Nhân hệ thống** cho các tác vụ yêu cầu phải tương tác trực tiếp với thanh ghi, cấu hình vi xử lý (CPU) hoặc các lệnh đặc quyền mà ngôn ngữ bậc cao không can thiệp được. |
| ![C++](https://img.shields.io/badge/C%2B%2B-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white) | **C++** | Ngôn ngữ cốt lõi được sử dụng xuyên suốt để xây dựng toàn bộ **Quy trình hoạt động** của Nhân hệ thống, tầng trừu tượng phần cứng (HAL) và các thành phần dịch vụ quan trọng trong hệ sinh thái. |
| ![Makefile](https://img.shields.io/badge/MAKEFILE-ef5350?style=for-the-badge&logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZpZXdCb3g9IjAgMCAzMiAzMiI+PHBhdGggZmlsbD0iI2ZmZmZmZiIgZD0ibTI5LjUgMjQuMDItMS42LS45MmE0LjQgNC40IDAgMCAwIC4wOS0uOUExLjMgMS4zIDAgMCAwIDI4IDIyYTUuNiA1LjYgMCAwIDAtLjEtMS4xbDEuNi0uOTJhLjQ5My40OTMgMCAwIDAgLjE4LS42OGwtMS41LTIuNmEuNDUuNDUgMCAwIDAtLjE4LS4xOFY2LjAxYTIuMDA2IDIuMDA2IDAgMCAwLTItMkg0YTIuMDA2IDIuMDA2IDAgMCAwLTIgMlYyMmEyLjAwNiAyLjAwNiAwIDAgMCAyIDJoMTAuNTNsLS4wMy4wMmEuNDkzLjQ5MyAwIDAgMC0uMTguNjhsMS41IDIuNmEuNDkzLjQ5MyAwIDAgMCAuNjguMThsMS42LS45MmE1LjkgNS45IDAgMCAwIDEuOSAxLjA5djEuODVhLjQ5NS40OTUgMCAwIDAgLjUuNWgzYS40OTUuNDk1IDAgMCAwIC41LS41di0xLjg1YTUuOSA1LjkgMCAwIDAgMS45LTEuMDlsMS42LjkyYS40OTMuNDkzIDAgMCAwIC42OC0uMThsMS41LTIuNmEuNDkzLjQ5MyAwIDAgMC0uMTgtLjY4TTI0IDIyLjAxYTEuOTkgMS45OSAwIDAgMS0uODggMS42NWwtLjE4LjExYTIuMDQgMi4wNCAwIDAgMS0xLjg4IDBsLS4xOC0uMTFhMS45OSAxLjk5IDAgMCAxLS44OC0xLjY1VjIyYTIgMiAwIDAgMSAuODgtMS42NmwuMTgtLjExYTIuMDQgMi4wNCAwIDAgMSAxLjg4IDBsLjE4LjExQTIgMiAwIDAgMSAyNCAyMlptMi00LjYzLS4xLjA2YTUuOSA1LjkgMCAwIDAtMS45LTEuMDlWMTQuNWEuNDk1LjQ5NSAwIDAgMC0uNS0uNWgtM2EuNDk1LjQ5NSAwIDAgMC0uNS41djEuODVhNS45IDUuOSAwIDAgMC0xLjkgMS4wOWwtMS42LS45MmEuNDkzLjQ5MyAwIDAgMC0uNjguMThsLTEuNSAyLjZhLjQ5My40OTMgMCAwIDAgLjE4LjY4bDEuNi45MkE1LjYgNS42IDAgMCAwIDE2IDIydi4wMUw0IDIyVjEwLjAxaDIyWiIvPjwvc3ZnPg==) | **Makefile** | Công cụ quản lý và tự động hóa quy trình biên dịch (Build Toolchain), giúp điều phối việc đóng gói các phân vùng độc lập giữa các thành phần ngoại vi một cách chính xác. |
| ![Bash](https://img.shields.io/badge/bash-121011.svg?style=for-the-badge&logo=gnu-bash&logoColor=white) | **Bash** | Các kịch bản lệnh (Script) hỗ trợ tự động hóa và thực hiện các thao tác điều phối dự án một cách nhanh chóng, tối ưu. |

### 📂 Chuẩn bị môi trường

1. Cài đặt các chương trình cần thiết
    ```bash
    sudo apt update && sudo apt install -y \
        clang \
        lld \
        llvm \
        qemu-system-x86 \
        qemu-system-misc \
        qemu-system-arm \
        libsdl2-2.0-0 \
        swtpm \
        swtpm-tools \
        python3 \
        telnet \
        gdisk \
        dosfstools \
        mtools \
        sbsigntool \
        curl \
        python3-pefile
    ```
2. Cài đặt môi trường cho QEMU
    ```bash
    sudo apt install -y ovmf qemu-efi-aarch64 qemu-efi-riscv64 && \
    sudo rm -rf /usr/share/edk2 && \
    sudo mkdir -p /usr/share/edk2/x64 /usr/share/edk2/aarch64 /usr/share/edk2/riscv64 && \
    sudo cp -r /usr/share/OVMF/* /usr/share/edk2/x64/ && \
    sudo cp -r /usr/share/AAVMF/* /usr/share/edk2/aarch64/ && \
    sudo cp -r /usr/share/qemu-efi-riscv64/* /usr/share/edk2/riscv64/
    ```
3. Khởi tạo khóa cho dự án
    ```bash
    openssl req -new -x509 -newkey rsa:2048 -sha256 -nodes \
      -keyout cert/vnexos.key \
      -out cert/vnexos.crt \
      -subj "/CN=VNExos OS/O=VNExos Inc./C=VN/" \
      -days 3650
    ```
4. Chuyển đổi khóa CRT sang khóa DER
    ```bash
    openssl x509 -in cert/vnexos.crt -outform DER -out cert/vnexos.cer
    ```
5. Đặt quyền chạy cho toàn bộ tệp kịch bản
    ```bash
    sudo chmod +x *.sh
    ```
6. Chạy chương trình chính thức:
    ```bash
    ./run.sh x86_64
    ```

### 🌳 Cây thư mục
```plaintext
apeiron/                 ← Thư mục gốc (Mã nguồn mở)
├── config/              ← Chứa cấu hình xây dựng toàn hệ thống
├── modules/             ← Chứa các thành phần ngoại vi
├── cert/                ← Chứa các giấy phép chữ ký số để xác minh các tệp khởi động
├── grub/                ← Bộ nạp mồi để có thể tùy biến cho shim
├── internal/            ← Chương trình cốt lõi (Mã nguồn đóng)
│   ├── bootloader/      ← Bộ nạp khởi động VNExos
│   │   ├── uefi/        ← Chứa toàn bộ lệnh gọi hàm UEFI
│   │   └── dtb/         ← Chứa toàn bộ lệnh gọi hàm Device Tree
│   │                      (Dành cho các máy ARM64/RISC-V không có UEFI)
│   └── kernel/          ← Nhân lõi Bản Nguyên
│       ├── arch/        ← Tầng trừu tượng phần cứng (HAL) phụ thuộc chip
│       │   ├── x86_64/  ← GDT, IDT, Paging 4-cấp, 5-cấp
│       │   ├── arm64/   ← Các cấp ngoại lệ, bộ quản lý bộ nhớ của ARM64
│       │   └── riscv64/ ← M/S/U, thanh ghi CSRs, phân trang Sv39, Sv48
│       ├── include/     ← Các tệp thư viện nội bộ của Nhân lõi
│       └── src/         ← Các tệp mã cốt lõi (Bộ lên lịch, IPC,...)
├── shared               ← Thư viện chung cho toàn bộ hệ thống
├── tools/               ← Chứa các kịch bản, công cụ hỗ trợ biên dịch
├── .clang-format        ← Chứa các cấu hình định dạng mã C++
├── compile_flags.txt    ← Chứa các cờ cho tiện ích clangd
├── makefile             ← Quản lý toàn bộ xây dựng, chạy thử dự án
├── build.sh             ← Kịch bản xây dựng
├── run.sh               ← Kịch bản chạy
├── readme.md            ← là tệp này!
└── LICENSE              ← Giấy phép bản quyền cho dự án
```
### 📜 Quy ước ghi chú cam kết
- `✨ tm`: **Thêm mới** một tính năng hoặc một dòng code (không mang tính sửa chữa)
- `🛠️ sl`: **Sửa lại** một tính năng nào đó
- `🚀 tc`: **Tái cấu** hoặc dọn dẹp một phần hoặc toàn bộ
- `📃 tl`: **Tài liệu** được cập nhật trong cam kết này
- `⚙️ ch`: **Cấu hình** cho việc xây dựng được thay đổi
- `🔀 gn`: **Gộp nhánh** nào với nhánh nào
- `🌐 kt`: **Khởi tạo** dự án (chỉ có duy nhất một lần)

### 🌐 Tên trong các ngôn ngữ khác
<p><em>Chán quá làm cho vui</em></p>

| Ngôn ngữ    | Tên                | Phiên âm                          |
|:------------|:-------------------|:----------------------------------|
| Tiếng Việt  | VNExos Bản Nguyên  | (Vi-en-éc-sợt) Bản Nguyên         |
| 㗂越（字喃）  | VNExos 本原         | Bản Nguyên                       |
| 中文         | VNExos 本源        | Bẩn-doén                          |
| 日本語       | VNExos 本源         | Hon-ghen                          |
| 한국어       | VNExos 본원         | Bon-uôn                           |
| English     | VNExos Apeiron     | A-pia-rần                         |
| Français    | VNExos L'Apeiron   | La-pia-gòn                        |
| Deutsch     | VNExos Das Apeiron | Đát Ây-pai-on                     |
| Español     | VNExos El Ápeiron  | Ê-lá-pây-ròn                      |
| Português   | VNExos O Ápeiron   | U Á-pê-ròn                        |
| हिन्दी         | VNExos मूल          | Mun                               |
| العربية     | VNExos الأصيل       | Ên A-xi-lù                        |
| עברית       | VNExos מקור        | Mà-cô                             |
| Русский     | VNExos Апейрон     | A-pia-ròn                         |
| ภาษาไทย      | VNExos อาไพรอน      | A-phai-rôn                        |
| ພາສາລາວ     | VNExos ອາໄພຣອນ     | A-phai-ròn                        |
