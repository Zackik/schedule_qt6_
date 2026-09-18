# Hướng dẫn tải và cài đặt SmartSchedule Qt6 cho người mới bắt đầu

Chào mừng bạn đến với SmartSchedule Qt6! Ứng dụng này được viết bằng C++ và sử dụng framework Qt6. Dưới đây là hướng dẫn chi tiết từng bước để bạn có thể tải về, mở và chạy ứng dụng trên máy tính cá nhân của mình.

---

## 1. Cách tải Source Code (Export)

Nếu bạn đang xem dự án này trên môi trường phát triển (như AI Studio Build), bạn có thể tải mã nguồn về máy:

1. Nhìn lên góc trên bên phải của giao diện.
2. Tìm menu **Settings** (Cài đặt) hoặc biểu tượng bánh răng/dấu ba chấm.
3. Chọn **Export to ZIP** (Tải xuống dạng file nén ZIP) hoặc **Export to GitHub**.
4. Giải nén file ZIP vừa tải về vào một thư mục trên máy tính của bạn (ví dụ: `C:\Projects\SmartSchedule`).

---

## 2. Cài đặt các công cụ cần thiết (Dependencies)

Để chạy được ứng dụng C++/Qt6, máy tính của bạn cần cài đặt 3 công cụ chính:
1. **C++ Compiler**: Trình biên dịch (GCC/G++ cho Linux, MinGW/MSVC cho Windows, Clang cho macOS).
2. **CMake**: Công cụ cấu hình dự án.
3. **Qt6**: Framework giao diện người dùng.

### 🐧 Dành cho Ubuntu / Linux (Dễ nhất)

Mở Terminal và chạy lần lượt các lệnh sau:

```bash
# 1. Cập nhật danh sách gói
sudo apt-get update

# 2. Cài đặt Git, Trình biên dịch C++, và CMake
sudo apt-get install -y git build-essential cmake ninja-build

# 3. Cài đặt Qt6 và các thư viện cần thiết
sudo apt-get install -y qt6-base-dev qt6-tools-dev qt6-tools-dev-tools libqt6networkauth6-dev
```

### 🪟 Dành cho Windows

1. **Cài đặt CMake & Git**:
   - Tải và cài đặt CMake: https://cmake.org/download/ (Nhớ tick chọn "Add CMake to the system PATH" khi cài).
   - Tải và cài đặt Git: https://git-scm.com/downloads.

2. **Cài đặt Qt6 & C++ Compiler (MinGW)**:
   - Truy cập trang chủ Qt: https://www.qt.io/download-open-source
   - Tải `Qt Online Installer` và chạy nó.
   - Đăng nhập bằng tài khoản Qt (miễn phí).
   - Khi đến phần chọn "Components", hãy mở rộng mục **Qt 6.x.x** và tích chọn:
     - `MinGW` (Đây là C++ Compiler).
     - `Qt Network Authorization` (Cần cho Google OAuth).
   - Nhấn Next và hoàn tất cài đặt.

### 🍎 Dành cho macOS

Mở Terminal và chạy:

```bash
# 1. Cài đặt Xcode Command Line Tools (C++ Compiler)
xcode-select --install

# 2. Cài đặt Homebrew (Nếu bạn chưa có, bỏ qua nếu đã cài)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# 3. Cài đặt CMake, Ninja và Qt6
brew install cmake ninja qt@6
```

---

## 3. Cách mở và chạy ứng dụng

### Cách 1: Sử dụng phần mềm Qt Creator (Khuyên dùng cho người mới)

Nếu bạn đã cài Qt trên Windows hoặc macOS, bạn sẽ có sẵn phần mềm **Qt Creator**.

1. Mở phần mềm **Qt Creator**.
2. Chọn **File -> Open File or Project...**
3. Tìm đến thư mục chứa mã nguồn bạn vừa giải nén, chọn file `CMakeLists.txt` và nhấn Open.
4. Một bảng "Configure Project" sẽ hiện ra. Đảm bảo kit "Desktop Qt 6.x.x MinGW" (hoặc kit tương ứng) được chọn. Nhấn **Configure**.
5. Nhấn nút **Play** (nút tam giác màu xanh lá cây) ở góc dưới bên trái để Build và Chạy ứng dụng.

### Cách 2: Chạy bằng dòng lệnh (Terminal / Command Prompt)

Mở Terminal (hoặc Command Prompt trên Windows) và di chuyển vào thư mục dự án:

```bash
cd /đường/dẫn/tới/thư/mục/SmartSchedule

# 1. Tạo thư mục build và cấu hình dự án
cmake -S . -B build

# 2. Biên dịch (Build) ứng dụng
cmake --build build --parallel

# 3. Chạy ứng dụng
# Trên Linux/macOS:
./build/SmartSchedule

# Trên Windows:
.\build\SmartSchedule.exe
```

---

## 4. Cấu hình Google Calendar (Bước tiếp theo)

Để sử dụng tính năng đồng bộ Google Calendar, bạn cần điền thông tin OAuth. 
Hãy đọc file `docs/GOOGLE_OAUTH_SETUP.md` (sẽ được tạo ở các bước tiếp theo) để xem hướng dẫn chi tiết cách lấy Google Client ID và Client Secret.

Chúc bạn thành công!
