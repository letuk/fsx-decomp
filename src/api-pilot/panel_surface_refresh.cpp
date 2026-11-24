static int g_refreshingPanelSurfacePilot; // 2020e434

// 2012b7bd. Each registry member contributes its requested dimensions.
static void* __stdcall AccumulatePanelSurfaceSizePilot(void* record, void* groupRecord)
{
    ClipWindowObservedPilot* window = *static_cast<ClipWindowObservedPilot**>(record);
    PanelPixelsObservedPilot& pixels = static_cast<PanelSurfaceGroupObservedPilot*>(groupRecord)->pixels;
    if (pixels.width <= window->surfaceWidth) pixels.width = window->surfaceWidth;
    if (pixels.height <= window->surfaceHeight) pixels.height = window->surfaceHeight;
    return NULL;
}

// 2012b7fc. Dimensions stay per window. Publish shared backing fields.
static void* __stdcall PublishPanelSurfacePilot(void* record, void* groupRecord)
{
    ClipWindowObservedPilot* window = *static_cast<ClipWindowObservedPilot**>(record);
    const PanelPixelsObservedPilot& shared = static_cast<PanelSurfaceGroupObservedPilot*>(groupRecord)->pixels;
    PanelPixelsObservedPilot& target = *reinterpret_cast<PanelPixelsObservedPilot*>(window->unknown78);
    target.format = shared.format;
    target.pixels = shared.pixels;
    target.pitch = shared.pitch;
    target.storageBytes = shared.storageBytes;
    return NULL;
}

struct PanelBitmapInfoPilot {
    BITMAPINFOHEADER header;
    DWORD colors[256];
};
C_ASSERT(sizeof(PanelBitmapInfoPilot) == 0x428);

static LONG DescribePanelBitmapPilot(PanelPixelsObservedPilot& pixels, PanelBitmapInfoPilot& info)
{
    info.header.biSize = sizeof(info.header);
    info.header.biPlanes = 1;
    info.header.biWidth = (pixels.width + 3) & ~3L;
    info.header.biHeight = -pixels.height;
    switch (pixels.format) {
    case 6: info.header.biBitCount = 32; return 4;
    case 5: info.header.biBitCount = 24; return 3;
    case 3:
        info.header.biCompression = BI_BITFIELDS;
        info.colors[0] = 0xf800;
        info.colors[1] = 0x07e0;
        info.colors[2] = 0x001f;
        // Fall through to 16 bits per pixel.
    case 2: info.header.biBitCount = 16; return 2;
    default:
        pixels.format = 1;
        info.header.biBitCount = 8;
        for (DWORD i = 0; i < 256; ++i) {
            DWORD red = ((i >> 5) & 7) * 255 / 7;
            DWORD green = ((i >> 2) & 7) * 255 / 7;
            DWORD blue = (i & 3) * 255 / 3;
            info.colors[i] = (red << 16) | (green << 8) | blue;
        }
        return 1;
    }
}

static void RebuildPrivatePanelBitmapPilot(PanelUpdateContextObservedPilot* context,
    PanelPixelsObservedPilot& pixels, PanelBitmapInfoPilot& info, LONG bytesPerPixel, void* entry)
{
    if (context->link->bitmap != NULL) {
        DeleteObject(context->link->bitmap);
        context->link->bitmap = NULL;
        pixels.pixels = NULL;
    }
    if (entry == NULL && (context->window->flags & 0x40000000) == 0) {
        g_panelGraphicsNotificationPendingPilot = 0;
        void* graphics = GetGraphicsControllerSourcePilot();
        GraphicsClipSlotPilot<GraphicsClipNotifyPilot>(graphics, 0x38)(graphics);
        entry = context->window->cachedGraphicsWindow;
    }
    if (entry != NULL && (context->window->flags & 0x10000000) != 0 &&
        GraphicsClipSlotPilot<GraphicsClipFlagPilot>(entry, 0x4c)(entry)) {
        DWORD index = GraphicsClipSlotPilot<GraphicsClipIndexPilot>(entry, 0x30)(entry);
        context->link->bitmapGraphicsIndex = index;
    } else {
        HDC dc = GetDC(NULL);
        HGDIOBJ* bitmap = &context->link->bitmap;
        if (*bitmap != NULL) {
            DeleteObject(*bitmap);
            *bitmap = NULL;
        }
        void* bits = NULL;
        *bitmap = CreateDIBSection(dc, reinterpret_cast<BITMAPINFO*>(&info), DIB_RGB_COLORS, &bits, NULL, 0);
        pixels.pixels = static_cast<BYTE*>(bits);
        pixels.pitch = info.header.biWidth * bytesPerPixel;
        pixels.storageBytes = pixels.height * pixels.pitch;
        memset(pixels.pixels, 0, pixels.storageBytes);
        ReleaseDC(NULL, dc);
    }
    ClipWindowObservedPilot* window = context->window;
    typedef void (__stdcall *SurfaceCallbackPilot)(DWORD, ClipWindowObservedPilot*, DWORD, DWORD);
    SurfaceCallbackPilot callback = *reinterpret_cast<SurfaceCallbackPilot*>(
        reinterpret_cast<BYTE*>(window) + 0x18);
    if (callback != NULL) callback(6, window, 0, 0);
    DispatchCallbackListSourcePilot(&g_mainWindowCommandCallbacksPilot, 0x70, 0);
}

static LONG ClampPanelSurfaceAxisPilot(LONG value, LONG minimum, LONG maximum)
{
    if (minimum != 0 && value < minimum) value = minimum;
    if (maximum != 0 && value > maximum) value = maximum;
    return value;
}

// 2012b9af.
extern "C" void __stdcall RefreshPanelSurfaceSourcePilot(PanelUpdateContextObservedPilot* context)
{
    if (g_refreshingPanelSurfacePilot != 0) return;
    g_refreshingPanelSurfacePilot = 1;
    void* entry = context->window->cachedGraphicsWindow;
    RECT client;
    GetClientRect(context->windowHandle, &client);
    if (client.right < 0 || client.bottom < 0) {
        g_refreshingPanelSurfacePilot = 0;
        return;
    }
    PanelPixelsObservedPilot* pixels = reinterpret_cast<PanelPixelsObservedPilot*>(context->window->unknown78);
    LONG previousWidth = pixels->width, previousHeight = pixels->height, previousPitch = pixels->pitch;
    short previousFormat = pixels->format;
    pixels->width = client.right - client.left;
    pixels->height = client.bottom - client.top;
    ReadPanelSizeLimitsPilot(context);
    pixels = reinterpret_cast<PanelPixelsObservedPilot*>(context->window->unknown78);
    pixels->width = ClampPanelSurfaceAxisPilot(pixels->width,
        PanelGeometryFieldPilot(context, 0x28), PanelGeometryFieldPilot(context, 0x30));
    pixels->height = ClampPanelSurfaceAxisPilot(pixels->height,
        PanelGeometryFieldPilot(context, 0x2c), PanelGeometryFieldPilot(context, 0x34));

    PanelSurfaceGroupObservedPilot* group = NULL;
    if (context->window->regionUnionFlag != 0) {
        LONG clipWidth = PanelGeometryFieldPilot(context, 0x18);
        LONG clipHeight = PanelGeometryFieldPilot(context, 0x1c);
        group = FindPanelSurfaceGroupPilot(context->window->regionUnionFlag);
        if (group == NULL) {
            g_refreshingPanelSurfacePilot = 0;
            return;
        }
        pixels = &group->pixels;
        previousHeight = pixels->height;
        previousPitch = pixels->pitch;
        previousFormat = pixels->format;
        pixels->width = pixels->height = 0;
        VisitCallbackArrayPilot(&group->windows, group, AccumulatePanelSurfaceSizePilot, 0);
        if (pixels->width <= clipWidth) pixels->width = clipWidth;
        if (pixels->height <= clipHeight) pixels->height = clipHeight;
        DWORD maximumWidth, maximumHeight, depth;
        ReadPanelGraphicsLimitsPilot(context->window, &maximumWidth, &maximumHeight, &depth);
        pixels->format = depth == 16 ? 3 : 2;
    }
    PanelBitmapInfoPilot info = { 0 };
    LONG bytesPerPixel = DescribePanelBitmapPilot(*pixels, info);
    if (group == NULL) {
        if (pixels->width != previousWidth || pixels->height != previousHeight ||
            pixels->format != previousFormat ||
            (entry != NULL && GraphicsClipSlotPilot<GraphicsClipIndexPilot>(entry, 0x30)(entry) !=
                context->link->bitmapGraphicsIndex))
            RebuildPrivatePanelBitmapPilot(context, *pixels, info, bytesPerPixel, entry);
    } else {
        pixels->pitch = info.header.biWidth * bytesPerPixel;
        if (PanelGeometryFieldPilot(context->window, 0x18) != 0 &&
            (pixels->pitch != previousPitch || pixels->height != previousHeight || pixels->format != previousFormat)) {
            context->bitmapDrawPending = 0;
            pixels->pixels = reinterpret_cast<BYTE*>(0xdeadbeef);
        }
        VisitCallbackArrayPilot(&group->windows, group, PublishPanelSurfacePilot, 0);
    }
    g_refreshingPanelSurfacePilot = 0;
}
