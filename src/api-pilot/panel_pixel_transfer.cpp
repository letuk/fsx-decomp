#include <string.h>

extern HDC g_panelMemoryDcPilot; // 2020d134
static int g_panelPaletteFormatPilot; // 2020ea88
static DWORD g_panelPalettePilot[256]; // 2020e688
static const DWORD g_panelPixelBytesPilot[] = {1, 1, 2, 2, 2, 3, 4}; // 201f9910

// 2011d05c / 182 and 2011d0b4 / 183.
extern "C" int __stdcall AcquirePanelMemoryDcPilot(ClipWindowObservedPilot* window, HDC* output)
{
    if (window == NULL) return -1;
    if (window->regionUnionFlag != 0) {
        *output = NULL;
        return -1;
    }
    ClipWindowLinkObservedPilot* link = window->link;
    SaveDC(g_panelMemoryDcPilot);
    SelectObject(g_panelMemoryDcPilot, link->bitmap);
    *output = g_panelMemoryDcPilot;
    return 0;
}

extern "C" int __stdcall ReleasePanelMemoryDcPilot(ClipWindowObservedPilot* window, HDC dc)
{
    if (window == NULL || dc == NULL || window->regionUnionFlag != 0) return -1;
    if (dc == g_panelMemoryDcPilot) RestoreDC(g_panelMemoryDcPilot, -1);
    return 0;
}

// 2011fe4c. The original cache key is the destination format alone.
extern "C" const DWORD* __stdcall ReadPanelPalettePilot(
    ClipWindowObservedPilot* window, const PanelPixelsObservedPilot* destination)
{
    if (g_panelPaletteFormatPilot == destination->format) return g_panelPalettePilot;
    g_panelPaletteFormatPilot = destination->format;
    HDC dc;
    RGBQUAD colors[256];
    AcquirePanelMemoryDcPilot(window, &dc);
    GetDIBColorTable(dc, 0, 256, colors);
    ReleasePanelMemoryDcPilot(window, dc);
    for (int i = 0; i < 256; ++i) {
        const RGBQUAD& color = colors[i];
        if (g_panelPaletteFormatPilot == 2)
            g_panelPalettePilot[i] = ((color.rgbRed & 0xf8) << 7) |
                ((color.rgbGreen & 0xf8) << 2) | (color.rgbBlue >> 3);
        else if (g_panelPaletteFormatPilot == 3)
            g_panelPalettePilot[i] = ((color.rgbRed & 0xf8) << 8) |
                ((color.rgbGreen & 0xfc) << 3) | (color.rgbBlue >> 3);
        else if (g_panelPaletteFormatPilot == 6)
            g_panelPalettePilot[i] = (color.rgbRed << 16) | (color.rgbGreen << 8) | color.rgbBlue;
    }
    return g_panelPalettePilot;
}

struct PanelPixelRowsPilot {
    BYTE* destination;
    const BYTE* source;
    LONG destinationPitch, sourcePitch;
    LONG width, height;
};

template<class Pixel>
static void ExpandPanelIndexedRowsPilot(PanelPixelRowsPilot rows, const DWORD* palette, int skipIndex)
{
    for (LONG y = 0; y < rows.height; ++y) {
        for (LONG x = 0; x < rows.width; ++x) {
            BYTE index = rows.source[x];
            if (index != skipIndex) {
                Pixel pixel = static_cast<Pixel>(palette[index]);
                memcpy(rows.destination + x * sizeof(pixel), &pixel, sizeof(pixel));
            }
        }
        rows.source += rows.sourcePitch;
        rows.destination += rows.destinationPitch;
    }
}

static WORD ExpandPanelGreenPilot(WORD pixel)
{
    return static_cast<WORD>((pixel & 0x1f) | ((pixel & 0x7fe0) << 1));
}

// 2011f532. The paired loop computes its width from the first row's
// alignment and reuses that width even if later rows change alignment.
static void ExpandPanelGreenRowsPilot(PanelPixelRowsPilot rows)
{
    LONG pairedWidth = rows.width - ((reinterpret_cast<DWORD>(rows.destination) & 2) != 0);
    for (LONG y = 0; y < rows.height; ++y) {
        bool leadingWord = (reinterpret_cast<DWORD>(rows.destination) & 2) != 0;
        LONG count = pairedWidth + leadingWord;
        if (count == 0) count = 1; // The zero-pair path falls into one word conversion.
        for (LONG x = 0; x < count; ++x) {
            WORD pixel;
            memcpy(&pixel, rows.source + 2 * x, sizeof(pixel));
            pixel = ExpandPanelGreenPilot(pixel);
            memcpy(rows.destination + 2 * x, &pixel, sizeof(pixel));
        }
        rows.source += rows.sourcePitch + 2 * (count - rows.width);
        rows.destination += rows.destinationPitch + 2 * (count - rows.width);
    }
}

// 2011f684, 2011f5e9, 2011f476, 2011f6c0 and 2011f749.
static void CopyPanelWordRowsPilot(PanelPixelRowsPilot rows, short format, bool masked)
{
    for (LONG y = 0; y < rows.height; ++y) {
        if (format == 2 && !masked) {
            memcpy(rows.destination, rows.source, 2 * rows.width);
        } else {
            for (LONG x = 0; x < rows.width; ++x) {
                WORD pixel;
                memcpy(&pixel, rows.source + 2 * x, sizeof(pixel));
                if (masked && (pixel & 0x8000) == 0) continue;
                if (format == 6) {
                    DWORD expanded = ((pixel & 0x1f) << 3) | ((pixel & 0x3e0) << 6) |
                        ((pixel & 0x7c00) << 9);
                    memcpy(rows.destination + 4 * x, &expanded, sizeof(expanded));
                } else {
                    if (format == 3) pixel = ExpandPanelGreenPilot(pixel);
                    memcpy(rows.destination + 2 * x, &pixel, sizeof(pixel));
                }
            }
        }
        rows.source += rows.sourcePitch;
        rows.destination += rows.destinationPitch;
    }
}

static void ClipPanelPixelAxisPilot(LONG& first, LONG& second, LONG& extent, LONG limit)
{
    if (first < 0) {
        extent += first;
        second -= first;
        first = 0;
    }
    if (extent + first > limit) extent = limit - first;
}

// 2011f7d3. The original receives destination RECT in EAX and source RECT in EDX.
extern "C" void __stdcall TransferPanelUpdateSourcePilot(ClipWindowObservedPilot* window,
    PanelPixelsObservedPilot* destination, const PanelPixelsObservedPilot* source,
    const RECT* destinationRect, const RECT* sourceRect)
{
    if (source == NULL || destination == NULL) return;
    LONG width = destinationRect->right - destinationRect->left;
    LONG height = destinationRect->bottom - destinationRect->top;
    if (width != sourceRect->right - sourceRect->left || height != sourceRect->bottom - sourceRect->top) return;
    LONG dx = destinationRect->left, dy = destinationRect->top;
    LONG sx = sourceRect->left, sy = sourceRect->top;
    ClipPanelPixelAxisPilot(dx, sx, width, destination->width);
    ClipPanelPixelAxisPilot(dy, sy, height, destination->height);
    ClipPanelPixelAxisPilot(sx, dx, width, source->width);
    ClipPanelPixelAxisPilot(sy, dy, height, source->height);
    if (width <= 0 || height <= 0 || source->pixels == NULL || destination->pixels == NULL) return;

    short destinationFormat = destination->format;
    short sourceFormat = source->format;
    PanelPixelRowsPilot rows = {
        destination->pixels + destination->pitch * dy + g_panelPixelBytesPilot[destinationFormat] * dx,
        source->pixels + source->pitch * sy + g_panelPixelBytesPilot[sourceFormat] * sx,
        destination->pitch, source->pitch, width, height
    };
    bool masked = (window->flags & 0x40000800) == 0x800;
    if (sourceFormat == 1) {
        BYTE transparentIndex = *source->pixels;
        const DWORD* palette = ReadPanelPalettePilot(window, destination);
        rows.sourcePitch = source->pitch;
        rows.destinationPitch = destination->pitch;
        int skipIndex = masked ? transparentIndex : -1;
        if (destination->format == 2 || destination->format == 3)
            ExpandPanelIndexedRowsPilot<WORD>(rows, palette, skipIndex);
        else if (destination->format == 6)
            ExpandPanelIndexedRowsPilot<DWORD>(rows, palette, skipIndex);
    } else if (sourceFormat == 2 && (destinationFormat == 2 || destinationFormat == 3 || destinationFormat == 6)) {
        if (destinationFormat == 3 && !masked) ExpandPanelGreenRowsPilot(rows);
        else CopyPanelWordRowsPilot(rows, destinationFormat, masked);
    }
}
