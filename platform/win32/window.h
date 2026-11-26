#pragma once

#include <functional>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace orbis::win32
{

void enableDpiAwareness();

class Window
{
public:
    Window() = default;
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool create(const wchar_t* title, int width, int height);
    void destroy();

    bool poll();

    HWND handle() const { return hwnd_; }
    int width() const { return width_; }
    int height() const { return height_; }

    std::function<void(int, int)> onResize;

private:
    static LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HWND hwnd_ = nullptr;
    int width_ = 0;
    int height_ = 0;
    bool quit_ = false;
};

} // namespace orbis::win32
