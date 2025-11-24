#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


#define MAX_DICT_SIZE 4096     // kamus dictionary 12 bit
#define INITIAL_DICT_SIZE 256  // Karakter ASCII
#define MAX_PATH_LEN 256

void compressFile(const char *input, const char *output);
void decompressFile(const char *input, const char *output);


#define MAX_STRING_LEN 1023 

//Fungsi untuk mencari string dalam kamus, pakek strcmp...kalo ketemu direturn index dari kodenya...nanti dipanggil di dalam fungsi compress
static int findString(char **dict, int size, const char *str) {
    int len = strlen(str);  // percepat dengan cek panjang dulu
    for (int i = 0; i < size; i++) {
        if (dict[i] && strlen(dict[i]) == len && strcmp(dict[i], str) == 0)
            return i;
    }
    return -1;
}

//FUNGSI COMPRESS FILE: nanti akan dipanggil dibawah di bagian main
void compressFile(const char *input, const char *output) {
    
    //baca file input dan bukak file output 
    FILE *in = fopen(input, "rb");
    if (!in) { printf("File input tidak ditemukan\n"); return; }

    FILE *out = fopen(output, "wb"); //(pakek wb jadi nanti ke overwritten kalo udah ada nama file output yg sama)
    if (!out) { printf("Gagal membuka file output\n"); fclose(in); return; }

    char *dict[MAX_DICT_SIZE] = {0};       //deklarasi kamus disini
    int dictSize = 0;

    //ini masukkin 256 huruf ascii ke amus    
    for (int i = 0; i < INITIAL_DICT_SIZE; i++) {
        dict[i] = malloc(2);        //ukuran 2 karana tambpung huruf dan \0
                                    //jadi kita punya array dict berukuran 256 yang masing2 ukurannya 2 
                                    //dan isinya adalah huruf ascii
        dict[i][0] = (char)i;       //char dari angka langsung nyammbung ke huuruf korsepondennya
        dict[i][1] = '\0';
        dictSize++;
    }

    int c = fgetc(in);      //baca karakter pertama
    if (c == EOF) {         //pengecekan jaga2 jika file input kosong               
        for (int i = 0; i < dictSize; i++) free(dict[i]);
        fclose(in); 
        fclose(out); 
        return;
    }

    char w[MAX_STRING_LEN + 1] = {0};
    w[0] = c; w[1] = '\0';        //masukan char pertama yang di dalam c tadi ke array w

    while ((c = fgetc(in)) != EOF) { //disini loopin nilai dari c yang trus lanjut baca char slnjutny
        char wc[MAX_STRING_LEN + 1];

        if (strlen(w) >= MAX_STRING_LEN) {  //cek jika panjang string w udah terlalu boros maka kita resest...
                                            //w ini fungsinya hanya pengecekan per huruf..jdi gpp direset
            int code = findString(dict, dictSize, w);
            uint16_t outcode = (uint16_t)code;              //ubah ke 16 bit
            fwrite(&outcode, sizeof(uint16_t), 1, out);     
            w[0] = (char)c; w[1] = '\0';                    //print ulang lagi dari awal isi dari w nya
            continue;
        }

        //INI INTI DARI ALGORITMA LZW
        snprintf(wc, sizeof(wc), "%s%c", w, (char)c);
        if (findString(dict, dictSize, wc) != -1) {   //c berubah jadi char selanjutnya, w tetap char pertama,
                                                        //lalu cari apakah wc ada di kamus
            strcpy(w, wc);                              //kalau ada di kamus maka pola diperpanjang
        } else {
            int code = findString(dict, dictSize, w);  //cari dalam kamus bagian yang ga ada tu di index mana
            uint16_t outcode = (uint16_t)code;          //dengan index itu ubah ke 16 bit
            fwrite(&outcode, sizeof(uint16_t), 1, out); //print ke file outpur yang sudah dikompresi

            if (dictSize < MAX_DICT_SIZE) {              //cek ukuran kamus udah penuh belum
                dict[dictSize] = malloc(strlen(wc) + 1);   //malloc untuk string wc  
                strcpy(dict[dictSize], wc);              //masukin wc ke dalam kamus
                dictSize++;
            }
            w[0] = (char)c;
            w[1] = '\0';
        }
    }

    int code = findString(dict, dictSize, w);  //ada string terakhir di w yang ga masuk
    uint16_t outcode = (uint16_t)code;      //ubah ke 16 bit
    fwrite(&outcode, sizeof(uint16_t), 1, out);  //masukkan ke file output

    for (int i = 0; i < dictSize; i++)  //terakhir jangan lupa bebasin dict,
    free(dict[i]);                      //setiap malloc harus ada free
    fclose(in); fclose(out);                                

    printf("KOMPRESI SELESAI %s\n", output);
}

//FUNGSI DECOMPRESS FILE: nanti akan dipanggil dibawah di bagian main                       
void decompressFile(const char *input, const char *output) {                               

    //baca file input dan buka file output...kalo file outpur sudah ada maka akan overwritten
    FILE *in = fopen(input, "rb");
    if (!in) { printf("File input tidak ditemukan\n"); return; }

    FILE *out = fopen(output, "wb");
    if (!out) { printf("Gagal membuka file output\n"); fclose(in); return; }

    char *dict[MAX_DICT_SIZE] = {0};
    int dictSize = 0;

    for (int i = 0; i < INITIAL_DICT_SIZE; i++) {           //inisialisasi kamus 256 ascii, sama kayak yang di bagian compress
        dict[i] = malloc(2);
        dict[i][0] = (char)i;
        dict[i][1] = '\0';
        dictSize++;
    }

    uint16_t prevCode;      
    if (fread(&prevCode, sizeof(uint16_t), 1, in) != 1) {  //fread membaca 2 byte pertama dari file 
                                                        //dan memasukkannya ke prevCode
        for (int i = 0; i < dictSize; i++) 
            free(dict[i]);                                //bersihin memori kamus sebelumnya
        fclose(in); 
        fclose(out); 
        return;
    }

    if (prevCode >= dictSize) {   //misal Jika kode pertama hasil kompresi berada di luar kamus awal (256 entri),
                                  //maka file itu pasti corrupt
        printf("Error: Kode pertama tidak valid\n");
        for (int i = 0; i < dictSize; i++) free(dict[i]);
            fclose(in);
            fclose(out);
            return;
    }

    char *prevStr = strdup(dict[prevCode]);    //strdup itu untuk duplicate string, 
                                               //pindahin ke prevstr biar ga ganggu kamus aslinya      
    fwrite(prevStr, 1, strlen(prevStr), out);  //tulis string pertama ke file hasil dekompresi

    uint16_t newCode;           //deklar var 16bit baru
    char *entry = NULL;

    while (fread(&newCode, sizeof(uint16_t), 1, in) == 1) {   //mulai looping baca setiap kode 16 bit dri file     
        char *currentEntry;    //ini variabel wakilin newcode
        int isKSK = 0;   //ini variabel untuk kasus ksk

        if (newCode < dictSize) {       //kalo newcode dibawah angka 255 maka kan sudah pasti ada di kamus
            currentEntry = dict[newCode];    //masukin bentuk hurufnya ke char currententry
        } else if (newCode == dictSize) {    //penanganan kasus ksk
            isKSK = 1;
            if (entry) free(entry);
            entry = malloc(strlen(prevStr) + 2);
            strcpy(entry, prevStr);
            entry[strlen(prevStr)] = prevStr[0];
            entry[strlen(prevStr) + 1] = '\0';
            currentEntry = entry;
        } else {
            printf("Error: Kode %u di luar batas\n", newCode);
            break;
        }

        if (dictSize < MAX_DICT_SIZE) {                 //selama kamus belum penuh, jalankan
            char *newEntry = malloc(strlen(prevStr) + 2);   //buat lokasi memori untuk huruf setelahnya
            strcpy(newEntry, prevStr);                  //copy kode sblumnya, masukin ke kamus
            newEntry[strlen(prevStr)] = currentEntry[0];    
            newEntry[strlen(prevStr) + 1] = '\0';
            dict[dictSize++] = newEntry;        //perluas kamus
        }

        free(prevStr);
        prevStr = strdup(currentEntry);        //isi yang sekarang, balik lagi ke prevstr...
                                                //karena nanti currenentry berubah jadi char slanjutnya
        fwrite(prevStr, 1, strlen(prevStr), out);
    }

    for (int i = 0; i < dictSize; i++) free(dict[i]);                                             
    if (prevStr) free(prevStr);
    if (entry) free(entry);

    fclose(in);
    fclose(out);

    printf("DEKOMPRESI SELESAI %s\n", output);
}



//    MAIN CODE DISINI : Tempat Tampilan Utama Terminal dan Pemanggilan Fungsi Compress dan Decompress


void clear_input_buffer() {             //fungsi untuk bebasin  sisa input distdin
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void get_path_input(char *buffer, size_t size) {        
    if (fgets(buffer, size, stdin) == NULL) buffer[0] = '\0'; //simpan input pengguna dalam buffer
    buffer[strcspn(buffer, "\r\n")] = 0;  //biar enter setelah input ga keca
}
//Fungsi Nambahin Ekstension kalo g ada
void add_extension(char *filename, const char *ext, size_t max_len) { 
    size_t file_len = strlen(filename), ext_len = strlen(ext);
    if (file_len >= ext_len && strcmp(filename + file_len - ext_len, ext) == 0) 
        return;                              //cek kalo udah ada txt ato lzw nya
    if (file_len + ext_len + 1 <= max_len)    //kalo blm cek muat ato ga
        strcat(filename, ext);                 //kalo muat tambahin extension
    else printf("Peringatan: Nama file terlalu panjang, ekstensi tidak ditambahkan.\n");    //ga muat gausah
}

int main() {
    int pilih;
    char in[MAX_PATH_LEN], out[MAX_PATH_LEN];                                   

    do {
        printf("\n=============================================");                     //
        printf("\n   Kompresi File Text Dengan Algoritma LZW   \n");
        printf("=============================================\n");
        printf("1. Kompres\n");
        printf("2. Dekompres\n");
        printf("3. Keluar\n");
        printf("=============================================\n");
        printf("Pilih: ");

        if (scanf("%d", &pilih) != 1) {
            printf("Input tidak valid\n");                                           
            clear_input_buffer();
            pilih = 0; 
            continue;
        }
        clear_input_buffer();

        //menu main ya memanggil fungsi2 yang sudah dijelaskan diatas
        if (pilih == 1) {
            printf("File input (.txt)   : ");
            get_path_input(in, sizeof(in));
            printf("File output (.lzw)  : ");
            get_path_input(out, sizeof(out));
            if (strlen(in) && strlen(out)) {
                add_extension(out, ".lzw", sizeof(out));
                compressFile(in, out);
            } else printf("Jalur file tidak boleh kosong.\n");
        }
        else if (pilih == 2) {
            printf("File input (.lzw)   : ");
            get_path_input(in, sizeof(in));
            printf("File output (.txt)  : ");
            get_path_input(out, sizeof(out));
            if (strlen(in) && strlen(out)) {
                add_extension(out, ".txt", sizeof(out));
                decompressFile(in, out);
            } else printf("Jalur file tidak boleh kosong.\n");      
        }
    } while (pilih != 3);                                           

    return 0;
}
