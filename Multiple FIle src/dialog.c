#include "lzw.h"
#include <string.h>
#include <commdlg.h>

static void zero_mem(void *ptr, size_t n) {
    if (ptr && n) memset(ptr, 0, n);
}

int openFileDialog(char *outPath, DWORD outSize, const char *filter) {
    OPENFILENAMEA ofn;
    zero_mem(&ofn, sizeof(ofn));
    zero_mem(outPath, outSize);

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner   = NULL;
    ofn.lpstrFilter = filter;
    ofn.lpstrFile   = outPath;
    ofn.nMaxFile    = outSize;
    ofn.Flags       = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;

    return (int)GetOpenFileNameA(&ofn);
}

int saveFileDialog(char *outPath, DWORD outSize, const char *filter, const char *defExt) {
    OPENFILENAMEA ofn;
    zero_mem(&ofn, sizeof(ofn));
    zero_mem(outPath, outSize);

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner   = NULL;
    ofn.lpstrFilter = filter;
    ofn.lpstrFile   = outPath;
    ofn.nMaxFile    = outSize;
    ofn.Flags       = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_EXPLORER;
    ofn.lpstrDefExt = defExt;

    return (int)GetSaveFileNameA(&ofn);
}