#include <stdio.h>
#include <string.h>
#ifdef _WIN32
    #include <conio.h>
    #define CLEAR "cls"
#else
    #include <termios.h>
    #include <unistd.h>
    #define CLEAR "clear"
    int getch() { /* kode di atas */ }
#endif
#include <stdlib.h>
#include <time.h>
#include <cstring>

#define MAX 100

typedef struct {
    int kode;
    char nama[30];
    double hargaNormal;
    double hargaDiskon3; // harga jika beli minimal 3
} Barang;

typedef struct {
    Barang barang;
    int jumlah;
    int bonus;
} Keranjang;

// Buffer struk
char bufferStruk[20000];
int pos = 0;

// Makro untuk print ke layar + simpan ke buffer
#define PRINT_BUFFER(...) do { \
    pos += sprintf(bufferStruk+pos, __VA_ARGS__); \
    printf(__VA_ARGS__); \
} while(0)

// Fungsi print harga ke layar
void printRupiah(double value){
    long n = (long)value;
    char temp[20];
    sprintf(temp,"%ld",n);
    int len = strlen(temp);
    for(int i=0;i<len;i++){
        printf("%c", temp[i]);
        if((len-i-1)%3==0 && i!=len-1) printf(".");
    }
    printf(",00");
}

// Fungsi print harga ke layar + buffer
void printRupiahBuffer(double value){
    long n = (long)value;
    char temp[30];
    sprintf(temp,"%ld",n);
    int len = strlen(temp);
    for(int i=0;i<len;i++){
        pos += sprintf(bufferStruk+pos,"%c", temp[i]);
        printf("%c", temp[i]);
        if((len-i-1)%3==0 && i!=len-1){
            pos += sprintf(bufferStruk+pos,".");
            printf(".");
        }
    }
    pos += sprintf(bufferStruk+pos,",00");
    printf(",00");
}


void bacaPassword(const char *prompt, char *buf, size_t ukuran) {
    struct termios lama, baru;
    tcgetattr(STDIN_FILENO, &lama);      // simpan pengaturan terminal asli
    baru = lama;
    baru.c_lflag &= ~(ECHO | ICANON);    // matikan echo & mode baris
    tcsetattr(STDIN_FILENO, TCSANOW, &baru);

    printf("%s", prompt);
    fflush(stdout);

    size_t i = 0;
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        if (c == 127 || c == 8) {        // backspace
            if (i > 0) {
                i--;
                printf("\b \b");
                fflush(stdout);
            }
        } else if (i < ukuran - 1) {
            buf[i++] = c;
            putchar('*');
            fflush(stdout);
        }
    }
    buf[i] = 0;
    printf("\n");

    tcsetattr(STDIN_FILENO, TCSANOW, &lama);  // kembalikan terminal
}
// Fungsi input uang tunai interaktif
long inputTunai(){
    char input[20]="";
    int len=0;
    char ch;
    long uang=0;

    printf("Masukkan Uang Bayar : Rp. ");

    while(1){
        ch=getch();
        if(ch==13) break; // Enter
        else if(ch==8 && len>0){ // Backspace
            len--;
            input[len]='\0';
        }
        else if(ch>='0' && ch<='9' && len<18){
            input[len++]=ch;
            input[len]='\0';
        }

        // Clear baris dan print ulang
        printf("\rMasukkan Uang Bayar : Rp.            ");
        printf("\rMasukkan Uang Bayar : Rp. ");
        if(len>0){
            long num = atol(input);
            printRupiah(num);
        }
        fflush(stdout);
    }
    printf("\n");
    uang = atol(input);
    return uang;
}

// Fungsi print rata tengah
void printCenter(const char *text, int width){
    int len = strlen(text);
    int pad = (width - len) / 2;
    for(int i=0;i<pad;i++){ printf(" "); pos += sprintf(bufferStruk+pos," "); }
    printf("%s\n", text);
    pos += sprintf(bufferStruk+pos,"%s\n", text);
}

int main() {
    srand(time(NULL));

    Barang daftarBarang[20] = {
        {1,"Panadol",3500,3300},
        {2,"Sanmol",4000,3800},
        {3,"Tolak Angin",12000,11500},
        {4,"Betadine",28000,27000},
        {5,"Alkohol 70%",15000,14500},
        {6,"Perban",20000,19500},
        {7,"Kasa",65000,63000},
        {8,"Kapas",5000,4800},
        {9,"Sarung tangan medis",2000,1900},
        {10,"Teh Celup",8000,7700},
        {11,"Kopi Sachet",1500,1450},
        {12,"Susu UHT 1L",18000,17500},
        {13,"Keju Cheddar",25000,24500},
        {14,"Mentega 200g",12000,11500},
        {15,"Coklat Batangan",10000,9500},
        {16,"Kecap Manis",9000,8700},
        {17,"Saus Sambal",11000,10500},
        {18,"Tepung Terigu 1kg",14000,13500},
        {19,"Mie Instan Kari",3500,3300},
        {20,"Air Mineral Galon",17500,17000}
    };
    int promo[] = {1,8,19};

    int pilihan;
    printf("Pilih aksi:\n");
    printf("1. Login\n");
    printf("2. Registrasi\n");
    printf("3. Keluar\n");
    scanf("%d", &pilihan);
    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    //variable registrasi
    char namaKasir[50];
    char password[50];
    char validasi[50];
    // variable login
    char namaKasirforLogin[50];
    char passwordforLogin[50];

    if (pilihan == 2) {
          printf("Masukkan username kasir: ");
          fgets(namaKasir, sizeof(namaKasir), stdin);
      namaKasir[strcspn(namaKasir, "\n")] = 0;

      while (1) {
          bacaPassword("Masukkan password Min.8: ", password, sizeof(password));
          password[strcspn(password, "\n")] = 0;

          if (strlen(password) < 8) {
              printf("Password terlalu pendek!\n");
              continue;
          }

          bacaPassword("Validasi password: ", validasi, sizeof(validasi));
          validasi[strcspn(validasi, "\n")] = 0;
          if (strcmp(password, validasi) != 0) {
              printf("Validasi tidak cocok, ulangi.\n");
              continue;
          }

          printf("Regis berhasil, silahkan lanjut ke proses login %s!\n\n\n", namaKasir);
          break;
      }
    }else if (pilihan == 1) {
       while (1){
        printf("Masukkan username kasir:");
        fgets(namaKasirforLogin, sizeof(namaKasirforLogin), stdin);
        namaKasirforLogin[strcspn(namaKasirforLogin, "\n")] = 0;

        bacaPassword("Masukkan password Min.8: ", passwordforLogin, sizeof(passwordforLogin));
        passwordforLogin[strcspn(passwordforLogin, "\n")] = 0;
      
        if (strcmp(namaKasirforLogin, namaKasir) == 0) {
          if (strcmp(passwordforLogin, password) == 0) {
            printf("Login berhasil, selamat datang %s!\n");
            break;
          }else {
            printf("Password salah!\n");
            continue;
          }
        }else {
          printf("Password salah\n");
          continue;
        }
      }
     
    }

    int lanjutTransaksi = 1;

    do {
        pos = 0; // Reset buffer struk
        Keranjang keranjang[MAX];
        int idx = 0;
        int kode;

        printf("\n=== PROGRAM KASIR DIEN MART ===\nDaftar Barang:\n");
        for(int i=0;i<20;i++){
            printf("%2d. %-20s Rp. ", daftarBarang[i].kode, daftarBarang[i].nama);
            printRupiah(daftarBarang[i].hargaNormal);
            printf("\n");
        }

        while(1){
            printf("Masukkan kode barang (0 untuk selesai): ");
            scanf("%d",&kode);
            if(kode==0) break;
            if(kode<1 || kode>20){ printf("Kode tidak valid!\n"); continue; }

            int jumlah;
            printf("Masukkan jumlah beli: ");
            scanf("%d",&jumlah);
            if(jumlah<=0){ printf("Jumlah tidak valid!\n"); continue; }

            keranjang[idx].barang = daftarBarang[kode-1];
            if(jumlah>=3) keranjang[idx].barang.hargaNormal = keranjang[idx].barang.hargaDiskon3;

            int isPromo=0;
            for(int i=0;i<3;i++) if(kode==promo[i]) isPromo=1;
            if(isPromo && jumlah>=5) keranjang[idx].bonus = jumlah/5;
            else keranjang[idx].bonus=0;

            keranjang[idx].jumlah = jumlah;
            idx++;
        }

        int totalItem=0;
        double totalBelanja=0;
        for(int i=0;i<idx;i++){
            totalItem += keranjang[i].jumlah + keranjang[i].bonus;
            totalBelanja += keranjang[i].jumlah * keranjang[i].barang.hargaNormal;
        }
        double diskon = (totalBelanja>=250000)? totalBelanja*0.05 : 0;
        double ppn = (totalBelanja-diskon)*0.11;
        double totalPlusPPN = (totalBelanja-diskon)+ppn;
        double totalDibulat = ((int)(totalPlusPPN/100))*100;

        // ==== COUNTER (dibaca dulu) ====
        int nomorStruk = 1;
        FILE *fcounter = fopen("counter.txt","r");
        if(fcounter){
            fscanf(fcounter,"%d",&nomorStruk);
            fclose(fcounter);
        }

        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        char bufferDate[20];
        sprintf(bufferDate,"%02d%02d%02d",tm.tm_mday,tm.tm_mon+1,tm.tm_year%100);

        // kode unik random 3 digit
        int kodeUnik = rand()%1000;

        // HEADER STRUK
        PRINT_BUFFER("\n============================================================\n");
        PRINT_BUFFER("                         DIEN MART\n");
        PRINT_BUFFER("                Jl. Merdeka No.123, Surabaya\n");
        PRINT_BUFFER("                    Telp: (031) 1234567\n");
        PRINT_BUFFER("============================================================\n");
        PRINT_BUFFER("STRUK#%03d\n",nomorStruk);
        PRINT_BUFFER("RefID Struk      : TRX%s%02d-%03d\n",bufferDate,tm.tm_hour,kodeUnik);
        PRINT_BUFFER("Tanggal          : %02d-%02d-%04d %02d:%02d:%02d\n",
            tm.tm_mday, tm.tm_mon+1, tm.tm_year+1900,
            tm.tm_hour, tm.tm_min, tm.tm_sec);
        PRINT_BUFFER("Kasir            : %s\n",namaKasir);
        PRINT_BUFFER("============================================================\n");

        // DAFTAR BELANJA
        PRINT_BUFFER("%-20s %-18s %-7s %-10s\n","Nama Barang","Harga","Qty","Ket");
        for(int i=0;i<idx;i++){
            if(keranjang[i].bonus > 0){
                PRINT_BUFFER("%-20s Rp. %-15ld %-7d Bonus %d\n",
                    keranjang[i].barang.nama,
                    (long)keranjang[i].barang.hargaNormal,
                    keranjang[i].jumlah,
                    keranjang[i].bonus
                );
            } else {
                PRINT_BUFFER("%-20s Rp. %-15ld %-7d -\n",
                    keranjang[i].barang.nama,
                    (long)keranjang[i].barang.hargaNormal,
                    keranjang[i].jumlah
                );
            }
            PRINT_BUFFER("   Rp. %ld,00\n", (long)(keranjang[i].jumlah * keranjang[i].barang.hargaNormal));
        }

        // TOTAL
        PRINT_BUFFER("============================================================\n");
        PRINT_BUFFER("%-20s : %d\n","Jumlah Item",totalItem);
        PRINT_BUFFER("%-20s : Rp. ", "Total Belanja"); printRupiahBuffer(totalBelanja); PRINT_BUFFER("\n");
        PRINT_BUFFER("%-20s : Rp. ", "Diskon"); printRupiahBuffer(diskon); PRINT_BUFFER("\n");
        PRINT_BUFFER("%-20s : Rp. ", "PPN (11%)"); printRupiahBuffer(ppn); PRINT_BUFFER("\n");
        PRINT_BUFFER("%-20s : Rp. ", "Total + PPN"); printRupiahBuffer(totalPlusPPN); PRINT_BUFFER("\n");
        PRINT_BUFFER("%-20s : Rp. ", "Total Dibulat"); printRupiahBuffer(totalDibulat); PRINT_BUFFER("\n");
        PRINT_BUFFER("============================================================\n");

        // PEMBAYARAN
        int metode;
        printf("Pilih metode bayar [1. Tunai | 2. E-Wallet]: ");
        scanf("%d",&metode);

        int statusLunas = 0;
        int c;
        while ((c = getchar()) != '\n' && c != EOF);

        if(metode==1){
            long uangBayar;
            while(1){
                uangBayar = inputTunai();
                if(uangBayar < (long)totalDibulat){
                    printf("Pembayaran Tidak Valid, uang kurang! Silakan ulangi.\n");
                } else break;
            }
            PRINT_BUFFER("%-20s : Rp. ", "Uang Bayar Masuk"); printRupiahBuffer(uangBayar); PRINT_BUFFER("\n");
            PRINT_BUFFER("%-20s : Rp. ", "Kembalian"); printRupiahBuffer(uangBayar-(long)totalDibulat); PRINT_BUFFER("\n");
            PRINT_BUFFER("%-20s : %s\n", "Status", "LUNAS");
            PRINT_BUFFER("============================================================\n");
            statusLunas = 1;
        } else {
            int ewallet;
            printf("Pilih e-wallet [1. OVO | 2. GoPay | 3. Dana | 4. ShopeePay | 5. Lain]: ");
            scanf("%d",&ewallet);
            char *namaEwallet;
            switch(ewallet){
                case 1: namaEwallet="OVO"; break;
                case 2: namaEwallet="GoPay"; break;
                case 3: namaEwallet="Dana"; break;
                case 4: namaEwallet="ShopeePay"; break;
                default: namaEwallet="E-Wallet Lain"; break;
            }
            char refIDEwallet[40];
            sprintf(refIDEwallet,"EWL%s%03d-%03d",bufferDate,nomorStruk,kodeUnik);

            PRINT_BUFFER("Pembayaran via E-Wallet\n");
            PRINT_BUFFER("%-20s : %s\n","Tipe", namaEwallet);
            PRINT_BUFFER("%-20s : %s\n","RefID E-Wallet", refIDEwallet);
            PRINT_BUFFER("%-20s : %s\n","Status", "LUNAS");
            PRINT_BUFFER("============================================================\n");
            statusLunas = 1;
        }

        // FOOTER
        PRINT_BUFFER("------------------------------------------------------------\n");
        printCenter("Terima kasih sudah belanja!",60);
        printCenter("Belanja hemat? Ya di DIEN MART!",60);
        printCenter("Diskon 5% untuk belanja >= Rp250.000",60);
        PRINT_BUFFER("============================================================\n");

        // SIMPAN FILE JIKA LUNAS
        if(statusLunas){
            char namaFile[30];
            sprintf(namaFile,"STRUK%03d.txt",nomorStruk);

            FILE *f = fopen(namaFile,"w");
            if(f){
                fprintf(f,"%s", bufferStruk);
                fclose(f);
                printf("Struk berhasil disimpan di '%s'\n", namaFile);

                // update counter (hanya jika lunas)
                fcounter = fopen("counter.txt","w");
                fprintf(fcounter,"%d", nomorStruk+1);
                fclose(fcounter);
            } else {
                printf("Gagal membuat file struk!\n");
            }
        }

        // Tanyakan mau transaksi lagi?
        char lagi;
        printf("\nApakah ingin melakukan transaksi lain? (y/n): ");
        scanf(" %c",&lagi);
        if(lagi=='y' || lagi=='Y') lanjutTransaksi=1;
        else lanjutTransaksi=0;

    } while(lanjutTransaksi);

    printf("Program kasir selesai. Terima kasih!\n");
    return 0;
}
