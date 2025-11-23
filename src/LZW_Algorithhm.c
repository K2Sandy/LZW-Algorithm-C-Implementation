#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


#define MAX_DICT_SIZE 4096     // kamus dictionary 12 bit
#define INITIAL_DICT_SIZE 256  // Karakter ASCII

void compressFile(const char *input, const char *output);
void decompressFile(const char *input, const char *output);


/*FUNGSI COMPRESS FILE: nanti akan dipanggil dibawah di bagian main*/
#define MAX_STRING_LEN 1023 

static int findString(char **dict, int size, const char *str) {
    for (int i = 0; i < size; i++) {
        if (dict[i] && strcmp(dict[i], str) == 0)
            return i;
    }
    return -1;
}

void compressFile(const char *input, const char *output) {
    FILE *in = fopen(input, "rb");
    if (!in) { printf("File input tidak ditemukan!\n"); return; }

    FILE *out = fopen(output, "wb");
    if (!out) { printf("Gagal membuka file output!\n"); fclose(in); return; }

    char *dict[MAX_DICT_SIZE] = {0};
    int dictSize = 0;

    for (int i = 0; i < INITIAL_DICT_SIZE; i++) {
        dict[i] = malloc(2);
        dict[i][0] = (char)i;
        dict[i][1] = '\0';
        dictSize++;
    }

    int c = fgetc(in);
    if (c == EOF) {
        for (int i = 0; i < dictSize; i++) free(dict[i]);
        fclose(in); fclose(out); return;
    }

    char w[MAX_STRING_LEN + 1] = {0};
    w[0] = c; w[1] = '\0';

    while ((c = fgetc(in)) != EOF) {
        char wc[MAX_STRING_LEN + 1];

        if (strlen(w) >= MAX_STRING_LEN) {
            int code = findString(dict, dictSize, w);
            uint16_t outcode = (uint16_t)code;
            fwrite(&outcode, sizeof(uint16_t), 1, out);
            w[0] = (char)c; w[1] = '\0';
            continue;
        }

        snprintf(wc, sizeof(wc), "%s%c", w, (char)c);
        if (findString(dict, dictSize, wc) != -1) {
            strcpy(w, wc);
        } else {
            int code = findString(dict, dictSize, w);
            uint16_t outcode = (uint16_t)code;
            fwrite(&outcode, sizeof(uint16_t), 1, out);

            if (dictSize < MAX_DICT_SIZE) {
                dict[dictSize] = malloc(strlen(wc) + 1);
                strcpy(dict[dictSize], wc);
                dictSize++;
            }
            w[0] = (char)c;
            w[1] = '\0';
        }
    }

    int code = findString(dict, dictSize, w);
    uint16_t outcode = (uint16_t)code;
    fwrite(&outcode, sizeof(uint16_t), 1, out);

    for (int i = 0; i < dictSize; i++) free(dict[i]);
    fclose(in); fclose(out);

    printf("KOMPRESI SELESAI → %s\n", output);
}

/*FUNGSI DECOMPRESS FILE: nanti akan dipanggil dibawah di bagian main*/
void decompressFile(const char *input, const char *output) {
    FILE *in = fopen(input, "rb");
    if (!in) { printf("File input tidak ditemukan!\n"); return; }

    FILE *out = fopen(output, "wb");
    if (!out) { printf("Gagal membuka file output!\n"); fclose(in); return; }

    char *dict[MAX_DICT_SIZE] = {0};
    int dictSize = 0;

    for (int i = 0; i < INITIAL_DICT_SIZE; i++) {
        dict[i] = malloc(2);
        dict[i][0] = (char)i;
        dict[i][1] = '\0';
        dictSize++;
    }

    uint16_t prevCode;
    if (fread(&prevCode, sizeof(uint16_t), 1, in) != 1) {
        for (int i = 0; i < dictSize; i++) free(dict[i]);
        fclose(in); fclose(out); return;
    }

    if (prevCode >= dictSize) {
        printf("Error: Kode pertama tidak valid!\n");
        for (int i = 0; i < dictSize; i++) free(dict[i]);
        fclose(in); fclose(out); return;
    }

    char *prevStr = strdup(dict[prevCode]);
    fwrite(prevStr, 1, strlen(prevStr), out);

    uint16_t newCode;
    char *entry = NULL;

    while (fread(&newCode, sizeof(uint16_t), 1, in) == 1) {
        char *currentEntry;
        int isKSK = 0;

        if (newCode < dictSize) {
            currentEntry = dict[newCode];
        } else if (newCode == dictSize) {
            isKSK = 1;
            entry = realloc(entry, strlen(prevStr) + 2);
            strcpy(entry, prevStr);
            entry[strlen(prevStr)] = prevStr[0];
            entry[strlen(prevStr) + 1] = '\0';
            currentEntry = entry;
        } else {
            printf("Error: Kode %u di luar batas!\n", newCode);
            break;
        }

        fwrite(currentEntry, 1, strlen(currentEntry), out);

        if (dictSize < MAX_DICT_SIZE) {
            char *newEntry = malloc(strlen(prevStr) + 2);
            strcpy(newEntry, prevStr);
            newEntry[strlen(prevStr)] = currentEntry[0];
            newEntry[strlen(prevStr) + 1] = '\0';
            dict[dictSize++] = newEntry;
        }

        free(prevStr);
        prevStr = strdup(currentEntry);
    }

    for (int i = 0; i < dictSize; i++) free(dict[i]);
    if (prevStr) free(prevStr);
    if (entry) free(entry);

    fclose(in);
    fclose(out);

    printf("DEKOMPRESI SELESAI → %s\n", output);
}


/*    MAIN CODE DISINI : Tempat Tampilan Utama Terminal dan Pemanggilan Fungsi Compress dan Decompress*/
#define MAX_PATH_LEN 256

void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void get_path_input(char *buffer, size_t size) {
    if (fgets(buffer, size, stdin) == NULL) buffer[0] = '\0';
    buffer[strcspn(buffer, "\r\n")] = 0;
}

void add_extension(char *filename, const char *ext, size_t max_len) {
    size_t file_len = strlen(filename), ext_len = strlen(ext);
    if (file_len >= ext_len && strcmp(filename + file_len - ext_len, ext) == 0) return;
    if (file_len + ext_len + 1 <= max_len) strcat(filename, ext);
    else printf("Peringatan: Nama file terlalu panjang, ekstensi tidak ditambahkan.\n");
}

int main() {
    int pilih;
    char in[MAX_PATH_LEN], out[MAX_PATH_LEN];

    do {
        printf("\n=============================================");
        printf("\n   Kompresi File Text Dengan Algoritma LZW   \n");
        printf("=============================================\n");
        printf("1. Kompres");
        printf("2. Dekompres");
        printf("3. Keluar");
        printf("=============================================\n");
        printf("Pilih: ");

        if (scanf("%d", &pilih) != 1) {
            printf("Input tidak valid!\n");
            clear_input_buffer();
            pilih = 0; 
            continue;
        }
        clear_input_buffer();

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
