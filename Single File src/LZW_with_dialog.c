// lzw_with_dialog.c
// Compile (MinGW): gcc lzw_with_dialog.c -o lzw_with_dialog.exe -lcomdlg32

#include <windows.h>
#include <commdlg.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAX_DICT_SIZE 4096     // kamus dictionary 12 bit
#define INITIAL_DICT_SIZE 256  // Karakter ASCII
#ifndef MAX_PATH_LEN
#define MAX_PATH_LEN MAX_PATH
#endif

void compressFile(const char *input, const char *output);
void decompressFile(const char *input, const char *output);

#define MAX_STRING_LEN 1023 

// --- utility: Open/Save file dialogs (ANSI) ------------------------------
static int openFileDialog(char *outPath, DWORD outSize, const char *filter) {
    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ZeroMemory(outPath, outSize);

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner   = NULL;
    ofn.lpstrFilter = filter;               // e.g. "Text Files\0*.txt\0All Files\0*.*\0"
    ofn.lpstrFile   = outPath;
    ofn.nMaxFile    = outSize;
    ofn.Flags       = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;

    return GetOpenFileNameA(&ofn);
}

static int saveFileDialog(char *outPath, DWORD outSize, const char *filter, const char *defExt) {
    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ZeroMemory(outPath, outSize);

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner   = NULL;
    ofn.lpstrFilter = filter;
    ofn.lpstrFile   = outPath;
    ofn.nMaxFile    = outSize;
    ofn.Flags       = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_EXPLORER;
    ofn.lpstrDefExt = defExt; // default extension without dot, e.g. "lzw"

    return GetSaveFileNameA(&ofn);
}

// ------------------------------------------------------------------------

//Fungsi untuk mencari string dalam kamus
static int findString(char **dict, int size, const char *str) {
    int len = strlen(str);
    for (int i = 0; i < size; i++) {
        if (dict[i] && strlen(dict[i]) == len && strcmp(dict[i], str) == 0)
            return i;
    }
    return -1;
}

//FUNGSI COMPRESS FILE
void compressFile(const char *input, const char *output) {
    FILE *in = fopen(input, "rb");
    if (!in) { printf("File input tidak ditemukan: %s\n", input); return; }

    FILE *out = fopen(output, "wb");
    if (!out) { printf("Gagal membuka file output: %s\n", output); fclose(in); return; }

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
        fclose(in);
        fclose(out);
        return;
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
    fclose(in);
    fclose(out);

    printf("KOMPRESI SELESAI -> %s\n", output);
}

//FUNGSI DECOMPRESS FILE
void decompressFile(const char *input, const char *output) {
    FILE *in = fopen(input, "rb");
    if (!in) { printf("File input tidak ditemukan: %s\n", input); return; }

    FILE *out = fopen(output, "wb");
    if (!out) { printf("Gagal membuka file output: %s\n", output); fclose(in); return; }

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
        fclose(in);
        fclose(out);
        return;
    }

    if (prevCode >= dictSize) {
        printf("Error: Kode pertama tidak valid\n");
        for (int i = 0; i < dictSize; i++) free(dict[i]);
        fclose(in);
        fclose(out);
        return;
    }

    char *prevStr = _strdup(dict[prevCode]); // strdup alias
    if (!prevStr) {
        printf("Error: strdup gagal\n");
        for (int i = 0; i < dictSize; i++) free(dict[i]);
        fclose(in);
        fclose(out);
        return;
    }
    fwrite(prevStr, 1, strlen(prevStr), out);

    uint16_t newCode;
    char *entry = NULL;

    while (fread(&newCode, sizeof(uint16_t), 1, in) == 1) {
        char *currentEntry;

        if (newCode < dictSize) {
            currentEntry = dict[newCode];
        } else if (newCode == dictSize) {
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

        if (dictSize < MAX_DICT_SIZE) {
            char *newEntry = malloc(strlen(prevStr) + 2);
            strcpy(newEntry, prevStr);
            newEntry[strlen(prevStr)] = currentEntry[0];
            newEntry[strlen(prevStr) + 1] = '\0';
            dict[dictSize++] = newEntry;
        }

        free(prevStr);
        prevStr = _strdup(currentEntry);
        if (!prevStr) { printf("Error: strdup gagal\n"); break; }
        fwrite(prevStr, 1, strlen(prevStr), out);
    }

    for (int i = 0; i < dictSize; i++) free(dict[i]);
    if (prevStr) free(prevStr);
    if (entry) free(entry);

    fclose(in);
    fclose(out);

    printf("DEKOMPRESI SELESAI -> %s\n", output);
}

//    MAIN CODE : Menu + integrasi dialog
void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int main() {
    int pilih;
    char in[MAX_PATH_LEN], out[MAX_PATH_LEN];

    do {
        printf("\n=============================================");
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

        if (pilih == 1) {
            // Open input .txt with explorer dialog
            printf("Pilih file input (.txt)...\n");
            if (!openFileDialog(in, sizeof(in), "Text Files\0*.txt\0All Files\0*.*\0")) {
                printf("Tidak ada file yang dipilih (dibatalkan).\n");
                continue;
            }

            // Save dialog for output .lzw
            printf("Pilih lokasi dan nama file output (.lzw)...\n");
            // seed default filename in buffer (optional)
            strncpy(out, "output.lzw", sizeof(out)-1);
            out[sizeof(out)-1] = '\0';
            if (!saveFileDialog(out, sizeof(out), "LZW Files\0*.lzw\0All Files\0*.*\0", "lzw")) {
                printf("Penyimpanan dibatalkan.\n");
                continue;
            }

            // ensure extension .lzw
            size_t file_len = strlen(out);
            if (file_len < 4 || strcmp(out + file_len - 4, ".lzw") != 0) {
                if (file_len + 4 < sizeof(out)) strcat(out, ".lzw");
            }

            compressFile(in, out);
        }
        else if (pilih == 2) {
            printf("Pilih file input (.lzw)...\n");
            if (!openFileDialog(in, sizeof(in), "LZW Files\0*.lzw\0All Files\0*.*\0")) {
                printf("Tidak ada file yang dipilih (dibatalkan).\n");
                continue;
            }

            printf("Pilih lokasi dan nama file output (.txt)...\n");
            strncpy(out, "output.txt", sizeof(out)-1);
            out[sizeof(out)-1] = '\0';
            if (!saveFileDialog(out, sizeof(out), "Text Files\0*.txt\0All Files\0*.*\0", "txt")) {
                printf("Penyimpanan dibatalkan.\n");
                continue;
            }

            // ensure extension .txt
            size_t file_len = strlen(out);
            if (file_len < 4 || strcmp(out + file_len - 4, ".txt") != 0) {
                if (file_len + 4 < sizeof(out)) strcat(out, ".txt");
            }

            decompressFile(in, out);
        }
    } while (pilih != 3);

    return 0;
}