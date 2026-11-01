#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <ctime>
#include <iostream>
#include <sqlite3.h>
#include <sodium.h>

#ifdef _WIN32
    #include <conio.h>
#else
    #include <termios.h>
    #include <unistd.h>

    // getch() untuk Linux: baca 1 karakter tanpa echo dan tanpa menunggu Enter
    int getch() {
        struct termios lama, baru;
        tcgetattr(STDIN_FILENO, &lama);
        baru = lama;
        baru.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &baru);
        int c = getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &lama);
        return c;
    }
#endif

#define MAX 50
#define JUMLAH_OBAT 20

typedef struct {
    int kode;
    char nama[30];
    long harga;
} Obat;

typedef struct {
    Obat obat;
    int jumlah;
} Keranjang;

// ===================== BUFFER STRUK =====================
char bufferStruk[20000];
int pos = 0;

// Print ke layar sekaligus simpan ke buffer struk
void cetak(const char* fmt, ...) {
    va_list args, salinan;
    va_start(args, fmt);
    va_copy(salinan, args);
    vprintf(fmt, args);
    va_end(args);

    if (pos < (int)sizeof(bufferStruk) - 1) {
        int n = vsnprintf(bufferStruk + pos, sizeof(bufferStruk) - pos, fmt, salinan);
        if (n > 0) pos += n;
        if (pos >= (int)sizeof(bufferStruk)) pos = sizeof(bufferStruk) - 1;
    }
    va_end(salinan);
}

void cetakTengah(const char* teks, int lebar) {
    int pad = (lebar - (int)strlen(teks)) / 2;
    for (int i = 0; i < pad; i++) cetak(" ");
    cetak("%s\n", teks);
}

// Format angka menjadi "12.500,00"
void formatRupiah(long nilai, char* out, size_t ukuran) {
    char temp[32];
    snprintf(temp, sizeof(temp), "%ld", nilai);
    int len = (int)strlen(temp);
    size_t j = 0;
    for (int i = 0; i < len && j + 5 < ukuran; i++) {
        out[j++] = temp[i];
        if ((len - i - 1) % 3 == 0 && i != len - 1) out[j++] = '.';
    }
    out[j] = '\0';
    strncat(out, ",00", ukuran - j - 1);
}

// ===================== INPUT =====================
// Buang sisa karakter di buffer stdin sampai newline
void buangSisaInput() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void bacaTeks(const char* prompt, char* buf, size_t ukuran) {
    printf("%s", prompt);
    fflush(stdout);
    if (!fgets(buf, (int)ukuran, stdin)) exit(0);  // EOF
    if (!strchr(buf, '\n')) buangSisaInput();      // input kepanjangan
    buf[strcspn(buf, "\n")] = '\0';
}

// Baca bilangan bulat dengan validasi (tidak looping tanpa henti kalau input huruf)
int bacaInt(const char* prompt) {
    char baris[32];
    while (true) {
        printf("%s", prompt);
        fflush(stdout);
        if (!fgets(baris, sizeof(baris), stdin)) exit(0);  // EOF
        if (!strchr(baris, '\n')) {
            buangSisaInput();
            printf("Input terlalu panjang!\n");
            continue;
        }
        char* akhir;
        long v = strtol(baris, &akhir, 10);
        if (akhir != baris && *akhir == '\n') return (int)v;
        printf("Input harus berupa angka!\n");
    }
}

// Baca password dengan tampilan bintang (*)
void bacaPassword(const char* prompt, char* buf, size_t ukuran) {
    printf("%s", prompt);
    fflush(stdout);

    size_t i = 0;
    int c;
    while ((c = getch()) != '\n' && c != '\r' && c != EOF) {
        if (c == 127 || c == 8) {              // backspace
            if (i > 0) {
                i--;
                printf("\b \b");
                fflush(stdout);
            }
        } else if (i < ukuran - 1 && c >= 32) {
            buf[i++] = (char)c;
            putchar('*');
            fflush(stdout);
        }
    }
    buf[i] = '\0';
    printf("\n");
}

// Input uang tunai interaktif (tampil format rupiah saat mengetik)
long inputTunai() {
    char input[20] = "";
    int len = 0;
    char fmt[40];

    printf("Masukkan Uang Bayar : Rp. ");
    fflush(stdout);

    while (true) {
        int ch = getch();
        if (ch == 13 || ch == 10) break;                       // Enter
        else if ((ch == 8 || ch == 127) && len > 0) {          // Backspace
            len--;
            input[len] = '\0';
        } else if (ch >= '0' && ch <= '9' && len < 18) {
            input[len++] = (char)ch;
            input[len] = '\0';
        }

        printf("\rMasukkan Uang Bayar : Rp.                              ");
        printf("\rMasukkan Uang Bayar : Rp. ");
        if (len > 0) {
            formatRupiah(atol(input), fmt, sizeof(fmt));
            printf("%s", fmt);
        }
        fflush(stdout);
    }
    printf("\n");
    return atol(input);
}

// ===================== DATABASE USER =====================
bool buatTabelUser(sqlite3* db) {
    const char* sql =
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT NOT NULL UNIQUE,"
        "password_hash TEXT NOT NULL,"
        "created_at TEXT DEFAULT CURRENT_TIMESTAMP);";
    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Error: " << errMsg << "\n";
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool registrasiUser(sqlite3* db, const char* username, const char* password) {
    char hash[crypto_pwhash_STRBYTES];
    if (crypto_pwhash_str(hash, password, strlen(password),
                          crypto_pwhash_OPSLIMIT_INTERACTIVE,
                          crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) return false;

    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO users (username, password_hash) VALUES (?, ?);";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hash, -1, SQLITE_TRANSIENT);
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);  // false jika username sudah ada
    sqlite3_finalize(stmt);
    return ok;
}

bool loginUser(sqlite3* db, const char* username, const char* password) {
    sqlite3_stmt* stmt;
    const char* sql = "SELECT password_hash FROM users WHERE username = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);

    bool valid = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* hash = (const char*)sqlite3_column_text(stmt, 0);
        if (hash) valid = (crypto_pwhash_str_verify(hash, password, strlen(password)) == 0);
    }
    sqlite3_finalize(stmt);
    return valid;
}

// ===================== MAIN =====================
int main() {
    if (sodium_init() < 0) {
        std::cerr << "Gagal inisialisasi libsodium\n";
        return 1;
    }

    sqlite3* db;
    if (sqlite3_open("data.db", &db) != SQLITE_OK) {
        std::cerr << "Gagal membuka database: " << sqlite3_errmsg(db) << "\n";
        return 1;
    }
    if (!buatTabelUser(db)) {
        sqlite3_close(db);
        return 1;
    }

    srand((unsigned)time(NULL));

    Obat daftarObat[JUMLAH_OBAT] = {
        {1,  "Panadol",              3500},
        {2,  "Sanmol",               4000},
        {3,  "Paramex",              3000},
        {4,  "Bodrex",               3500},
        {5,  "Promag",               9000},
        {6,  "Mylanta Tablet",       8500},
        {7,  "Tolak Angin",         12000},
        {8,  "Antangin JRG",         3500},
        {9,  "Betadine 15ml",       28000},
        {10, "Alkohol 70%",         15000},
        {11, "Perban",              20000},
        {12, "Kasa Steril",         65000},
        {13, "Kapas",                5000},
        {14, "Sarung Tangan Medis",  2000},
        {15, "Masker Medis",         3000},
        {16, "Vitamin C IPI",       15000},
        {17, "Hansaplast",           6000},
        {18, "Minyak Kayu Putih",   18000},
        {19, "Oralit",               2500},
        {20, "Termometer Digital",  35000}
    };

    // ---------- Menu login / registrasi ----------
    char kasirAktif[50] = "";
    bool sudahLogin = false;

    while (!sudahLogin) {
        printf("\nPilih aksi:\n1. Login\n2. Registrasi\n3. Keluar\n");
        int pilihan = bacaInt("> ");

        char user[50], pass[50], validasi[50];

        if (pilihan == 1) {
            bacaTeks("Masukkan username kasir: ", user, sizeof(user));
            bacaPassword("Masukkan password: ", pass, sizeof(pass));

            if (loginUser(db, user, pass)) {
                strcpy(kasirAktif, user);
                printf("Login berhasil, selamat datang %s!\n", kasirAktif);
                sudahLogin = true;
            } else {
                printf("Username atau password salah!\n");
            }
        } else if (pilihan == 2) {
            bacaTeks("Masukkan username kasir: ", user, sizeof(user));
            if (strlen(user) == 0) {
                printf("Username tidak boleh kosong!\n");
                continue;
            }
            bacaPassword("Masukkan password (min. 8): ", pass, sizeof(pass));
            bacaPassword("Validasi password: ", validasi, sizeof(validasi));

            if (strlen(pass) < 8)
                printf("Password terlalu pendek!\n");
            else if (strcmp(pass, validasi) != 0)
                printf("Validasi tidak cocok!\n");
            else if (registrasiUser(db, user, pass))
                printf("Registrasi berhasil, silakan login.\n");
            else
                printf("Registrasi gagal (username mungkin sudah dipakai).\n");
        } else if (pilihan == 3) {
            sqlite3_close(db);
            return 0;
        } else {
            printf("Pilihan tidak valid!\n");
        }
    }

    // ---------- Transaksi ----------
    bool lanjutTransaksi = true;

    do {
        pos = 0;  // reset buffer struk
        bufferStruk[0] = '\0';
        Keranjang keranjang[MAX];
        int idx = 0;
        char fmt1[40], fmt2[40];

        printf("\n=== PROGRAM KASIR DIEN MART ===\nDaftar Obat:\n");
        for (int i = 0; i < JUMLAH_OBAT; i++) {
            formatRupiah(daftarObat[i].harga, fmt1, sizeof(fmt1));
            printf("%2d. %-22s Rp. %s\n", daftarObat[i].kode, daftarObat[i].nama, fmt1);
        }

        while (true) {
            int kode = bacaInt("Masukkan kode obat (0 untuk selesai): ");
            if (kode == 0) break;
            if (kode < 1 || kode > JUMLAH_OBAT) {
                printf("Kode tidak valid!\n");
                continue;
            }
            if (idx >= MAX) {
                printf("Keranjang penuh (maks. %d jenis obat)!\n", MAX);
                break;
            }

            int jumlah = bacaInt("Masukkan jumlah beli: ");
            if (jumlah <= 0) {
                printf("Jumlah tidak valid!\n");
                continue;
            }

            keranjang[idx].obat = daftarObat[kode - 1];
            keranjang[idx].jumlah = jumlah;
            idx++;
        }

        if (idx == 0) {
            printf("Keranjang kosong, transaksi dibatalkan.\n");
        } else {
            long totalItem = 0;
            long totalBelanja = 0;
            for (int i = 0; i < idx; i++) {
                totalItem += keranjang[i].jumlah;
                totalBelanja += (long)keranjang[i].jumlah * keranjang[i].obat.harga;
            }
            double ppn = totalBelanja * 0.11;
            double totalPlusPPN = totalBelanja + ppn;
            long totalDibulat = ((long)(totalPlusPPN / 100)) * 100;

            // Nomor struk dari counter.txt
            int nomorStruk = 1;
            FILE* fcounter = fopen("counter.txt", "r");
            if (fcounter) {
                if (fscanf(fcounter, "%d", &nomorStruk) != 1) nomorStruk = 1;
                fclose(fcounter);
            }

            time_t t = time(NULL);
            struct tm tm = *localtime(&t);
            char bufferDate[20];
            snprintf(bufferDate, sizeof(bufferDate), "%02d%02d%02d",
                     tm.tm_mday, tm.tm_mon + 1, tm.tm_year % 100);

            int kodeUnik = rand() % 1000;

            // HEADER STRUK
            cetak("\n============================================================\n");
            cetak("                         DIEN MART\n");
            cetak("                Jl. Merdeka No.123, Surabaya\n");
            cetak("                    Telp: (031) 1234567\n");
            cetak("============================================================\n");
            cetak("STRUK#%03d\n", nomorStruk);
            cetak("RefID Struk      : TRX%s%02d-%03d\n", bufferDate, tm.tm_hour, kodeUnik);
            cetak("Tanggal          : %02d-%02d-%04d %02d:%02d:%02d\n",
                  tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900,
                  tm.tm_hour, tm.tm_min, tm.tm_sec);
            cetak("Kasir            : %s\n", kasirAktif);
            cetak("============================================================\n");

            // DAFTAR BELANJA
            cetak("%-20s %-15s %-5s %s\n", "Nama Obat", "Harga", "Qty", "Subtotal");
            for (int i = 0; i < idx; i++) {
                formatRupiah(keranjang[i].obat.harga, fmt1, sizeof(fmt1));
                formatRupiah((long)keranjang[i].jumlah * keranjang[i].obat.harga,
                             fmt2, sizeof(fmt2));
                cetak("%-20s %-15s %-5d %s\n",
                      keranjang[i].obat.nama, fmt1, keranjang[i].jumlah, fmt2);
            }

            // TOTAL
            cetak("============================================================\n");
            cetak("%-20s : %ld\n", "Jumlah Item", totalItem);
            formatRupiah(totalBelanja, fmt1, sizeof(fmt1));
            cetak("%-20s : Rp. %s\n", "Total Belanja", fmt1);
            formatRupiah((long)ppn, fmt1, sizeof(fmt1));
            cetak("%-20s : Rp. %s\n", "PPN (11%)", fmt1);
            formatRupiah((long)totalPlusPPN, fmt1, sizeof(fmt1));
            cetak("%-20s : Rp. %s\n", "Total + PPN", fmt1);
            formatRupiah(totalDibulat, fmt1, sizeof(fmt1));
            cetak("%-20s : Rp. %s\n", "Total Dibulat", fmt1);
            cetak("============================================================\n");

            // PEMBAYARAN
            int metode;
            while (true) {
                metode = bacaInt("Pilih metode bayar [1. Tunai | 2. E-Wallet]: ");
                if (metode == 1 || metode == 2) break;
                printf("Pilihan tidak valid!\n");
            }

            if (metode == 1) {
                long uangBayar;
                while (true) {
                    uangBayar = inputTunai();
                    if (uangBayar < totalDibulat) {
                        printf("Pembayaran tidak valid, uang kurang! Silakan ulangi.\n");
                    } else break;
                }
                formatRupiah(uangBayar, fmt1, sizeof(fmt1));
                cetak("%-20s : Rp. %s\n", "Uang Bayar Masuk", fmt1);
                formatRupiah(uangBayar - totalDibulat, fmt1, sizeof(fmt1));
                cetak("%-20s : Rp. %s\n", "Kembalian", fmt1);
                cetak("%-20s : %s\n", "Status", "LUNAS");
                cetak("============================================================\n");
            } else {
                int ewallet;
                while (true) {
                    ewallet = bacaInt("Pilih e-wallet [1. OVO | 2. GoPay | 3. Dana | 4. ShopeePay | 5. Lain]: ");
                    if (ewallet >= 1 && ewallet <= 5) break;
                    printf("Pilihan tidak valid!\n");
                }
                const char* namaEwallet;
                switch (ewallet) {
                    case 1: namaEwallet = "OVO"; break;
                    case 2: namaEwallet = "GoPay"; break;
                    case 3: namaEwallet = "Dana"; break;
                    case 4: namaEwallet = "ShopeePay"; break;
                    default: namaEwallet = "E-Wallet Lain"; break;
                }
                char refIDEwallet[40];
                snprintf(refIDEwallet, sizeof(refIDEwallet), "EWL%s%03d-%03d",
                         bufferDate, nomorStruk, kodeUnik);

                cetak("Pembayaran via E-Wallet\n");
                cetak("%-20s : %s\n", "Tipe", namaEwallet);
                cetak("%-20s : %s\n", "RefID E-Wallet", refIDEwallet);
                cetak("%-20s : %s\n", "Status", "LUNAS");
                cetak("============================================================\n");
            }

            // FOOTER
            cetak("------------------------------------------------------------\n");
            cetakTengah("Terima kasih sudah berbelanja!", 60);
            cetakTengah("Semoga lekas sehat!", 60);
            cetak("============================================================\n");

            // SIMPAN STRUK (transaksi sudah lunas)
            char namaFile[30];
            snprintf(namaFile, sizeof(namaFile), "STRUK%03d.txt", nomorStruk);

            FILE* f = fopen(namaFile, "w");
            if (f) {
                fprintf(f, "%s", bufferStruk);
                fclose(f);
                printf("Struk berhasil disimpan di '%s'\n", namaFile);

                fcounter = fopen("counter.txt", "w");
                if (fcounter) {
                    fprintf(fcounter, "%d", nomorStruk + 1);
                    fclose(fcounter);
                }
            } else {
                printf("Gagal membuat file struk!\n");
            }
        }

        // Transaksi lagi?
        char lagi[8];
        bacaTeks("\nApakah ingin melakukan transaksi lain? (y/n): ", lagi, sizeof(lagi));
        lanjutTransaksi = (lagi[0] == 'y' || lagi[0] == 'Y');

    } while (lanjutTransaksi);

    sqlite3_close(db);
    printf("Program kasir selesai. Terima kasih!\n");
    return 0;
}
