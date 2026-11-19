// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#include "windowshdrcapture.h"

#include <QElapsedTimer>
#include <QFloat16>
#include <QImage>
#include <QScreen>

#ifdef FLAMESHOT_DEBUG_CAPTURE
#include <QDebug>
#endif

#include <qt_windows.h>

#include <d3d11.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <cmath>
#include <cwchar>
#include <iterator>
#include <map>
#include <mutex>
#include <vector>

using Microsoft::WRL::ComPtr;

namespace {

// scRGB reference white: 1.0 is 80 nits
constexpr double kScRgbWhiteNits = 80.0;
// Longest wait for Desktop Duplication to deliver a desktop image
constexpr qint64 kFrameTimeoutMs = 500;
constexpr UINT kFramePollMs = 100;

void logFallback(const char* reason, HRESULT hr = S_OK)
{
#ifdef FLAMESHOT_DEBUG_CAPTURE
    qDebug().nospace() << "Windows HDR capture failed, using GDI: " << reason
                       << " (hr=0x" << Qt::hex << static_cast<quint32>(hr)
                       << ")";
#else
    Q_UNUSED(reason)
    Q_UNUSED(hr)
#endif
}

HMONITOR monitorFor(QScreen* screen)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    auto* windowsScreen =
      screen->nativeInterface<QNativeInterface::QWindowsScreen>();
    return windowsScreen ? windowsScreen->handle() : nullptr;
#else
    // No native monitor handle before Qt 6.7: keep the GDI capture
    Q_UNUSED(screen)
    return nullptr;
#endif
}

template<typename T>
LONG getDeviceInfo(T& info,
                   DISPLAYCONFIG_DEVICE_INFO_TYPE type,
                   const LUID& adapterId,
                   UINT32 id)
{
    info.header.type = type;
    info.header.size = static_cast<UINT32>(sizeof(T));
    info.header.adapterId = adapterId;
    info.header.id = id;
    return DisplayConfigGetDeviceInfo(&info.header);
}

// Returns the monitor's SDR white level in nits when Windows reports
// advanced color as enabled for it, or 0 otherwise (or if the query fails).
// Only DisplayConfig is used here, so SDR monitors never touch DXGI.
double advancedColorSdrWhiteNits(const MONITORINFOEXW& monitorInfo)
{
    UINT32 pathCount = 0;
    UINT32 modeCount = 0;
    if (GetDisplayConfigBufferSizes(
          QDC_ONLY_ACTIVE_PATHS, &pathCount, &modeCount) != ERROR_SUCCESS) {
        return 0.0;
    }
    std::vector<DISPLAYCONFIG_PATH_INFO> paths(pathCount);
    std::vector<DISPLAYCONFIG_MODE_INFO> modes(modeCount);
    if (QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS,
                           &pathCount,
                           paths.data(),
                           &modeCount,
                           modes.data(),
                           nullptr) != ERROR_SUCCESS) {
        return 0.0;
    }
    paths.resize(pathCount);

    for (const DISPLAYCONFIG_PATH_INFO& path : paths) {
        DISPLAYCONFIG_SOURCE_DEVICE_NAME source = {};
        if (getDeviceInfo(source,
                          DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME,
                          path.sourceInfo.adapterId,
                          path.sourceInfo.id) != ERROR_SUCCESS ||
            std::wcscmp(source.viewGdiDeviceName, monitorInfo.szDevice) != 0) {
            continue;
        }

        DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO color = {};
        const LONG colorResult =
          getDeviceInfo(color,
                        DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO,
                        path.targetInfo.adapterId,
                        path.targetInfo.id);
        if (colorResult != ERROR_SUCCESS || !color.advancedColorEnabled) {
            return 0.0;
        }

        // SDRWhiteLevel is a multiplier of 80 nits, scaled by 1000. A zero
        // level would divide by zero later, so it keeps the GDI capture; any
        // other ULONG value gives a finite, positive number of nits.
        DISPLAYCONFIG_SDR_WHITE_LEVEL white = {};
        if (getDeviceInfo(white,
                          DISPLAYCONFIG_DEVICE_INFO_GET_SDR_WHITE_LEVEL,
                          path.targetInfo.adapterId,
                          path.targetInfo.id) != ERROR_SUCCESS ||
            white.SDRWhiteLevel == 0) {
            return 0.0;
        }
        return white.SDRWhiteLevel / 1000.0 * kScRgbWhiteNits;
    }
    return 0.0;
}

// Finds the DXGI output driving the monitor, and the adapter it belongs to:
// Desktop Duplication needs a device created on that adapter.
bool findOutput(HMONITOR monitor,
                ComPtr<IDXGIAdapter1>& adapter,
                ComPtr<IDXGIOutput>& output)
{
    ComPtr<IDXGIFactory1> factory;
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)))) {
        return false;
    }
    for (UINT a = 0;
         SUCCEEDED(factory->EnumAdapters1(a, adapter.ReleaseAndGetAddressOf()));
         ++a) {
        for (UINT o = 0; SUCCEEDED(
               adapter->EnumOutputs(o, output.ReleaseAndGetAddressOf()));
             ++o) {
            DXGI_OUTPUT_DESC desc{};
            if (SUCCEEDED(output->GetDesc(&desc)) && desc.Monitor == monitor) {
                return true;
            }
        }
    }
    return false;
}

// A small entry is retained per monitor. Current display metadata is
// checked before reusing its device and duplication.
struct CaptureCache
{
    HMONITOR monitor = nullptr;
    MONITORINFOEXW monitorInfo{};
    DXGI_OUTPUT_DESC1 outputDesc{};
    ComPtr<IDXGIAdapter1> adapter;
    ComPtr<IDXGIOutput6> output;
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    ComPtr<IDXGIOutputDuplication> duplication;
};

std::map<HMONITOR, CaptureCache> g_captureCaches;
std::mutex g_captureMutex;

// Called with g_captureMutex held.
void pruneRemovedOutputs()
{
    for (auto it = g_captureCaches.begin(); it != g_captureCaches.end();) {
        MONITORINFO info{};
        info.cbSize = static_cast<DWORD>(sizeof(info));
        if (!GetMonitorInfoW(it->first, &info)) {
            it = g_captureCaches.erase(it);
        } else {
            ++it;
        }
    }
}

bool sameDesktopCoordinates(const RECT& a, const RECT& b)
{
    return EqualRect(&a, &b) != FALSE;
}

// Windows also reports advanced color for SDR monitors with automatic color
// management; only HDR uses the G2084/P2020 color space. Rotated outputs are
// delivered unrotated by Desktop Duplication, so they keep the GDI capture.
bool isCapturableHdrOutput(const DXGI_OUTPUT_DESC1& outputDesc)
{
    if (outputDesc.ColorSpace != DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020) {
        return false;
    }
    if (outputDesc.Rotation != DXGI_MODE_ROTATION_IDENTITY &&
        outputDesc.Rotation != DXGI_MODE_ROTATION_UNSPECIFIED) {
        logFallback("rotated output");
        return false;
    }
    return true;
}

bool duplicateOutput(CaptureCache& cache)
{
    // B8G8R8A8_UNORM must always be offered; only FP16 frames are used
    const DXGI_FORMAT formats[] = { DXGI_FORMAT_R16G16B16A16_FLOAT,
                                    DXGI_FORMAT_B8G8R8A8_UNORM };
    const HRESULT hr = cache.output->DuplicateOutput1(
      cache.device.Get(),
      0,
      static_cast<UINT>(std::size(formats)),
      formats,
      cache.duplication.ReleaseAndGetAddressOf());
    if (FAILED(hr)) {
        logFallback("DuplicateOutput1", hr);
        return false;
    }
    return true;
}

bool createDeviceAndDuplication(CaptureCache& cache)
{
    const HRESULT hr = D3D11CreateDevice(cache.adapter.Get(),
                                         D3D_DRIVER_TYPE_UNKNOWN,
                                         nullptr,
                                         0,
                                         nullptr,
                                         0,
                                         D3D11_SDK_VERSION,
                                         cache.device.GetAddressOf(),
                                         nullptr,
                                         cache.context.GetAddressOf());
    if (FAILED(hr)) {
        logFallback("D3D11CreateDevice", hr);
        return false;
    }
    return duplicateOutput(cache);
}

// sRGB transfer function (IEC 61966-2-1) to 8 bits, indexed by the linear
// 0..1 value in 1/65535 steps
const std::vector<quint8>& srgbEncodingTable()
{
    static const std::vector<quint8> table = [] {
        std::vector<quint8> values(65536);
        for (size_t i = 0; i < values.size(); ++i) {
            const double l = static_cast<double>(i) / 65535.0;
            const double v = l <= 0.0031308
                               ? 12.92 * l
                               : 1.055 * std::pow(l, 1.0 / 2.4) - 0.055;
            values[i] = static_cast<quint8>(qBound(0, qRound(v * 255.0), 255));
        }
        return values;
    }();
    return table;
}

// Converts the captured FP16 scRGB desktop image to 8-bit sRGB. Values are
// normalized so that the monitor's SDR white level becomes 1.0, which keeps
// SDR content looking as it does on screen. Anything brighter than SDR white
// is deliberately clipped, since the result is an ordinary SDR image.
QImage convertScRgbToSdr(const uchar* bits,
                         qsizetype bytesPerLine,
                         int width,
                         int height,
                         double sdrWhiteNits)
{
    QImage image(width, height, QImage::Format_RGB32);
    if (image.isNull()) {
        return {};
    }

    const double sdrWhiteScale = sdrWhiteNits / kScRgbWhiteNits;
    const auto inverseSdrWhiteScale = static_cast<float>(1.0 / sdrWhiteScale);
    const quint8* srgb = srgbEncodingTable().data();
    // Clamps to [0, 1]: negative (out of gamut) values and NaN become 0,
    // values above SDR white (including infinity) become 1
    const auto encode = [inverseSdrWhiteScale, srgb](float scRgb) {
        const float normalized = scRgb * inverseSdrWhiteScale;
        const float linear = normalized > 0.0f ? qMin(normalized, 1.0f) : 0.0f;
        return srgb[static_cast<size_t>(linear * 65535.0f + 0.5f)];
    };

    std::vector<float> linearRgb(static_cast<size_t>(width) * 4);
    for (int y = 0; y < height; ++y) {
        // Rows are bytesPerLine apart, which may exceed width * 8 bytes
        qFloatFromFloat16(
          linearRgb.data(),
          reinterpret_cast<const qfloat16*>(bits + y * bytesPerLine),
          static_cast<qsizetype>(linearRgb.size()));
        auto* out = reinterpret_cast<QRgb*>(image.scanLine(y));
        for (int x = 0; x < width; ++x) {
            const float* pixel = &linearRgb[static_cast<size_t>(x) * 4];
            // qRgb() is opaque; the desktop alpha channel is not used
            out[x] = qRgb(encode(pixel[0]), encode(pixel[1]), encode(pixel[2]));
        }
    }
    return image;
}

// Grabs one desktop image through the cached duplication. The caller owns
// retry and fallback decisions so a successful retry does not log a fallback.
struct CaptureAttempt
{
    QImage image;
    const char* failureReason = nullptr;
    HRESULT hr = S_OK;
};

CaptureAttempt captureOutput(CaptureCache& cache,
                             const DXGI_OUTPUT_DESC1& outputDesc,
                             double sdrWhiteNits,
                             bool reusedDuplication)
{
    // A frame may carry only a mouse pointer update (LastPresentTime == 0);
    // wait for one that carries the desktop image. A reused duplication with
    // no new desktop image is restarted once to obtain a current first frame.
    ComPtr<IDXGIResource> resource;
    DXGI_OUTDUPL_FRAME_INFO frameInfo{};
    QElapsedTimer waited;
    waited.start();
    const qint64 timeoutMs = reusedDuplication ? 0 : kFrameTimeoutMs;
    const UINT pollMs = reusedDuplication ? 0 : kFramePollMs;
    HRESULT hr;
    for (;;) {
        hr = cache.duplication->AcquireNextFrame(
          pollMs, &frameInfo, resource.ReleaseAndGetAddressOf());
        if (SUCCEEDED(hr)) {
            if (frameInfo.LastPresentTime.QuadPart != 0) {
                break;
            }
            hr = cache.duplication->ReleaseFrame();
            if (FAILED(hr)) {
                return { {}, "ReleaseFrame", hr };
            }
        } else if (hr != DXGI_ERROR_WAIT_TIMEOUT) {
            return { {}, "AcquireNextFrame", hr };
        }
        if (waited.elapsed() >= timeoutMs) {
            return { {}, "no desktop frame", DXGI_ERROR_WAIT_TIMEOUT };
        }
    }

    // Copy the frame to a CPU readable texture and release it right away
    const auto width = static_cast<UINT>(outputDesc.DesktopCoordinates.right -
                                         outputDesc.DesktopCoordinates.left);
    const auto height = static_cast<UINT>(outputDesc.DesktopCoordinates.bottom -
                                          outputDesc.DesktopCoordinates.top);
    ComPtr<ID3D11Texture2D> frame;
    ComPtr<ID3D11Texture2D> staging;
    D3D11_TEXTURE2D_DESC frameDesc{};
    hr = resource.As(&frame);
    if (SUCCEEDED(hr)) {
        frame->GetDesc(&frameDesc);
    }
    const bool usable = SUCCEEDED(hr) &&
                        frameDesc.Format == DXGI_FORMAT_R16G16B16A16_FLOAT &&
                        frameDesc.Width == width && frameDesc.Height == height;
    if (usable) {
        D3D11_TEXTURE2D_DESC stagingDesc = frameDesc;
        stagingDesc.MipLevels = 1;
        stagingDesc.ArraySize = 1;
        stagingDesc.SampleDesc = { 1, 0 };
        stagingDesc.Usage = D3D11_USAGE_STAGING;
        stagingDesc.BindFlags = 0;
        stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        stagingDesc.MiscFlags = 0;
        hr = cache.device->CreateTexture2D(
          &stagingDesc, nullptr, staging.GetAddressOf());
        if (SUCCEEDED(hr)) {
            cache.context->CopyResource(staging.Get(), frame.Get());
        }
    }
    // The desktop image must not be referenced once the frame is released
    frame.Reset();
    resource.Reset();
    const HRESULT releaseHr = cache.duplication->ReleaseFrame();
    if (FAILED(releaseHr)) {
        return { {}, "ReleaseFrame", releaseHr };
    }
    if (!usable) {
        return { {}, "frame is not FP16 or has an unexpected size", hr };
    }
    if (FAILED(hr)) {
        return { {}, "staging texture", hr };
    }

    D3D11_MAPPED_SUBRESOURCE mapped{};
    hr = cache.context->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &mapped);
    if (FAILED(hr)) {
        return { {}, "Map", hr };
    }
    QImage image = convertScRgbToSdr(static_cast<const uchar*>(mapped.pData),
                                     static_cast<qsizetype>(mapped.RowPitch),
                                     static_cast<int>(width),
                                     static_cast<int>(height),
                                     sdrWhiteNits);
    cache.context->Unmap(staging.Get(), 0);
    if (image.isNull()) {
        return { {}, "QImage allocation", S_OK };
    }
    return { image, nullptr, S_OK };
}

bool prepareOutput(CaptureCache& cache,
                   HMONITOR monitor,
                   const MONITORINFOEXW& monitorInfo,
                   DXGI_OUTPUT_DESC1& outputDesc)
{
    if (cache.duplication.Get() != nullptr && cache.monitor == monitor &&
        std::wcscmp(cache.monitorInfo.szDevice, monitorInfo.szDevice) == 0 &&
        sameDesktopCoordinates(cache.monitorInfo.rcMonitor,
                               monitorInfo.rcMonitor) &&
        SUCCEEDED(cache.device->GetDeviceRemovedReason()) &&
        SUCCEEDED(cache.output->GetDesc1(&outputDesc)) &&
        outputDesc.Monitor == monitor &&
        sameDesktopCoordinates(cache.outputDesc.DesktopCoordinates,
                               outputDesc.DesktopCoordinates) &&
        outputDesc.Rotation == cache.outputDesc.Rotation &&
        outputDesc.ColorSpace == cache.outputDesc.ColorSpace) {
        return true;
    }

    cache = CaptureCache{};
    ComPtr<IDXGIOutput> output;
    if (!findOutput(monitor, cache.adapter, output) ||
        FAILED(output.As(&cache.output)) ||
        FAILED(cache.output->GetDesc1(&outputDesc))) {
        cache = CaptureCache{};
        logFallback("no DXGI output for the monitor");
        return false;
    }
    cache.monitor = monitor;
    cache.monitorInfo = monitorInfo;
    cache.outputDesc = outputDesc;
    return true;
}

} // namespace

QPixmap WindowsHdrCapture::grabScreen(QScreen* screen)
{
    if (!screen) {
        return {};
    }

    const HMONITOR monitor = monitorFor(screen);
    if (!monitor) {
        return {};
    }

    // Stage 1: DisplayConfig. SDR monitors stop here and keep the GDI path.
    MONITORINFOEXW monitorInfo{};
    monitorInfo.cbSize = static_cast<DWORD>(sizeof(monitorInfo));
    const double sdrWhiteNits = GetMonitorInfoW(monitor, &monitorInfo)
                                  ? advancedColorSdrWhiteNits(monitorInfo)
                                  : 0.0;
    if (sdrWhiteNits <= 0.0) {
        std::lock_guard<std::mutex> lock(g_captureMutex);
        pruneRemovedOutputs();
        g_captureCaches.erase(monitor);
        return {};
    }

    // A duplication and its immediate context cannot be used concurrently.
    // Keep the lock through acquisition, GPU readback and conversion.
    std::lock_guard<std::mutex> lock(g_captureMutex);
    // Drop unplugged outputs so an old HMONITOR cannot retain GPU resources.
    pruneRemovedOutputs();
    CaptureCache& cache = g_captureCaches[monitor];
    DXGI_OUTPUT_DESC1 outputDesc{};
    if (!prepareOutput(cache, monitor, monitorInfo, outputDesc)) {
        return {};
    }
    const bool reusedDuplication = cache.duplication.Get() != nullptr;

    // Stage 2: DXGI confirms the output is in HDR mode
    if (!isCapturableHdrOutput(outputDesc) ||
        (!reusedDuplication && !createDeviceAndDuplication(cache))) {
        cache = CaptureCache{};
        return {};
    }

    CaptureAttempt attempt =
      captureOutput(cache, outputDesc, sdrWhiteNits, reusedDuplication);
    if (attempt.image.isNull() && reusedDuplication) {
        bool ready;
        if (attempt.hr == DXGI_ERROR_WAIT_TIMEOUT) {
            // A still desktop has no new frame on a persistent duplication.
            // Restart just the duplication in that case, retaining the device.
            cache.duplication.Reset();
            ready = duplicateOutput(cache);
        } else {
            // Access lost, device removed and similar: rebuild everything
            cache = CaptureCache{};
            ready = prepareOutput(cache, monitor, monitorInfo, outputDesc) &&
                    isCapturableHdrOutput(outputDesc) &&
                    createDeviceAndDuplication(cache);
        }
        if (!ready) {
            cache = CaptureCache{};
            return {};
        }
        attempt = captureOutput(cache, outputDesc, sdrWhiteNits, false);
    }
    if (attempt.image.isNull()) {
        logFallback(attempt.failureReason, attempt.hr);
        cache = CaptureCache{};
        return {};
    }

    QPixmap pixmap = QPixmap::fromImage(attempt.image);
    // Same device pixel ratio as QScreen::grabWindow() gives a full screen grab
    pixmap.setDevicePixelRatio(screen->devicePixelRatio());
    return pixmap;
}
