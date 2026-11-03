# apotek-cli

Aplikasi berbasis console (CLI) untuk pengelolaan apotek, ditulis dengan C++17. Data disimpan menggunakan SQLite dan aplikasi memakai libsodium untuk fungsi kriptografi.

## Daftar Isi

- [Teknologi](#teknologi)
- [Struktur Proyek](#struktur-proyek)
- [Prasyarat](#prasyarat)
- [Mengunduh Kode](#mengunduh-kode)
- [Build dan Menjalankan](#build-dan-menjalankan)
  - [Linux dan macOS](#linux-dan-macos)
  - [Windows (MSYS2)](#windows-msys2)
  - [Cross-compile Windows dari Linux](#cross-compile-windows-dari-linux)
- [Panduan Pengembangan](#panduan-pengembangan)
- [Kontribusi](#kontribusi)
- [Troubleshooting](#troubleshooting)

## Teknologi

| Komponen | Keterangan |
|---|---|
| Bahasa | C++17 |
| Build system | CMake (minimal 3.16) |
| Database | SQLite3 |
| Kriptografi | libsodium |
| Compiler | GCC, Clang, atau MSVC (Windows: MinGW-w64 direkomendasikan) |

## Struktur Proyek

```
apotek-cli/
├── src/                  # Kode sumber (entry point: src/main.cpp)
├── build-win/            # Direktori output build target Windows
├── CMakeLists.txt        # Konfigurasi build
├── ming-toolchain.cmake  # Toolchain MinGW-w64 untuk cross-compile ke Windows
└── README.md
```

## Prasyarat

Pastikan perangkat sudah memiliki: **compiler C++17**, **CMake ≥ 3.16**, **pkg-config**, **SQLite3 (dev)**, dan **libsodium (dev)**.

### Debian / Ubuntu

```bash
sudo apt update
sudo apt install -y build-essential cmake pkg-config libsqlite3-dev libsodium-dev git
```

### Arch Linux / Omarchy

```bash
sudo pacman -S --needed base-devel cmake pkgconf sqlite libsodium git
```

### Fedora

```bash
sudo dnf install -y gcc-c++ cmake pkgconf-pkg-config sqlite-devel libsodium-devel git
```

### macOS (Homebrew)

```bash
brew install cmake pkg-config sqlite libsodium git
```

### Windows (MSYS2)

1. Pasang [MSYS2](https://www.msys2.org/).
2. Buka terminal **MSYS2 MINGW64**, lalu jalankan:

```bash
pacman -S --needed git mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja \
  mingw-w64-x86_64-pkgconf mingw-w64-x86_64-sqlite3 mingw-w64-x86_64-libsodium
```

## Mengunduh Kode

```bash
git clone https://github.com/Heliandra9/apotek-cli.git
cd apotek-cli
```

## Build dan Menjalankan

Nama executable hasil build adalah `myapp` (`myapp.exe` di Windows).

### Linux dan macOS

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/myapp
```

### Windows (MSYS2)

Jalankan di terminal **MSYS2 MINGW64**:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/myapp.exe
```

Pada Windows, SQLite dan libsodium di-link secara statis sehingga `myapp.exe` tidak memerlukan DLL tambahan.

### Cross-compile Windows dari Linux

Membutuhkan toolchain `x86_64-w64-mingw32` beserta pustaka SQLite3 dan libsodium versi MinGW yang terpasang di `/usr/x86_64-w64-mingw32` (lihat `ming-toolchain.cmake`).

```bash
cmake -S . -B build-win -DCMAKE_TOOLCHAIN_FILE=ming-toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build-win -j
```

Hasilnya berupa `build-win/myapp.exe`, yang dapat dijalankan di Windows (atau melalui Wine: `wine build-win/myapp.exe`).

## Panduan Pengembangan

### Menambah file sumber

Daftarkan setiap file `.cpp` baru pada `add_executable` di `CMakeLists.txt`:

```cmake
add_executable(myapp
    src/main.cpp
    src/fitur.cpp
)
```

Header ditempatkan pada folder `include/` (sudah terdaftar sebagai include directory).

### Build debug

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
```

### Editor / IDE

`CMAKE_EXPORT_COMPILE_COMMANDS` sudah aktif, sehingga `build/compile_commands.json` dapat langsung dipakai oleh clangd, VS Code, CLion, maupun Neovim. Contoh agar clangd menemukannya:

```bash
ln -sf build/compile_commands.json compile_commands.json
```

### Standar kode

- Gunakan C++17.
- Kode harus bebas warning pada flag `-Wall -Wextra -Wpedantic` (GCC/Clang) atau `/W4` (MSVC).

## Kontribusi

1. Fork repositori ini.
2. Buat branch fitur: `git checkout -b fitur/nama-fitur`
3. Commit perubahan: `git commit -m "Tambah fitur ..."`
4. Push branch: `git push origin fitur/nama-fitur`
5. Buka Pull Request ke branch `main`.

Laporkan bug atau usulan fitur melalui [Issues](https://github.com/Heliandra9/apotek-cli/issues).

## Troubleshooting

| Masalah | Solusi |
|---|---|
| `Could NOT find SQLite3` | Pasang paket dev SQLite3 sesuai OS pada bagian [Prasyarat](#prasyarat). |
| `No package 'libsodium' found` | Pasang libsodium (dev) dan pastikan `pkg-config` terpasang. |
| `Could NOT find PkgConfig` | Pasang `pkg-config` / `pkgconf`. |
| Error `libsqlite3.a` tidak ditemukan di Windows | Pastikan paket `mingw-w64-x86_64-sqlite3` terpasang dan build dijalankan dari terminal **MINGW64**. |
| Cache CMake bentrok setelah ganti compiler | Hapus folder build lalu konfigurasi ulang: `rm -rf build`. |
