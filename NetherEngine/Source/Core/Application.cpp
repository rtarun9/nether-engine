#include "Pch.hpp"

#include "Application.hpp"

#include "Engine.hpp"
#include "Common/Utils.hpp"
#include "Common/DataTypes.hpp"

// For ImGui.
#include <imgui_impl_win32.h>

// Forward declare message handler from imgui_impl_win32.cpp.
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace nether::core
{
    int Application::Run(Engine* const engine, const HINSTANCE instance, const std::wstring_view windowTitle)
    {
        try
        {
            // Register the window class.
            // Specify that it should redraw the entire window if the client region dimension's change due to movement or size adjustment.
            const WNDCLASSEXW windowClass
            {
                .cbSize = sizeof(WNDCLASSEX),
                .style = CS_HREDRAW | CS_VREDRAW,
                .lpfnWndProc = Application::WindowProc,
                .hInstance = instance,
                .hCursor = ::LoadCursor(nullptr, IDC_ARROW),
                .lpszClassName = L"Base Window Class",
            };

            if (!(::RegisterClassExW(&windowClass)))
            {
                utils::ErrorMessage(L"Failed to register window class.");
            }

            // Enable DPI awareness.
            ::SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

            // Find window dimension's so that the client region dimensions are 80% of the monitor width and height.
            const int32_t monitorWidth = ::GetSystemMetrics(SM_CXSCREEN);
            const int32_t monitorHeight = ::GetSystemMetrics(SM_CYSCREEN);

            const Uint2 clientDimensions 
            {
                .x = static_cast<uint32_t>(monitorWidth * 0.8f),
                .y = static_cast<uint32_t>(monitorHeight * 0.8f)
            };

            RECT windowRect
            {
                .left = 0l,
                .top = 0l,
                .right = static_cast<long>(clientDimensions.x),
                .bottom = static_cast<long>(clientDimensions.y)
            };

            ::AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);
            const int32_t windowWidth = static_cast<int32_t>(windowRect.right - windowRect.left);
            const int32_t windowHeight = static_cast<int32_t>(windowRect.bottom - windowRect.top);

            const int32_t windowXPos = std::max<int32_t>(0, (monitorWidth - windowWidth) / 2);
            const int32_t windowYPos = std::max<int32_t>(0, (monitorHeight - windowHeight) / 2);

            // Create the window.
            mWindowHandle = ::CreateWindowExW(0, L"Base Window Class", L"Nether Engine",
                WS_OVERLAPPEDWINDOW, windowXPos, windowYPos, windowWidth, windowHeight, nullptr, nullptr, instance,
                engine);

            if (!mWindowHandle)
            {
                utils::ErrorMessage(L"Failed to create window handle.");
            }

            engine->Init(mWindowHandle, clientDimensions);

            assert(mWindowHandle && "Window Handle is NULL.");
            ::ShowWindow(mWindowHandle, SW_SHOWNORMAL);

            // Message loop.
            MSG message{};
            while (message.message != WM_QUIT)
            {
                mCurrentFrameTime = mClock.now();
                const float deltaTime = static_cast<float>((mCurrentFrameTime - mPreviousFrameTime).count() * 1e-9);
                mPreviousFrameTime = mCurrentFrameTime;

                if (::PeekMessageW(&message, nullptr, 0u, 0u, PM_REMOVE))
                {
                    ::TranslateMessage(&message);
                    ::DispatchMessageW(&message);
                }

                engine->Update(deltaTime);
                engine->Render();
            }

            return static_cast<int32_t>(message.wParam);
        }
        catch (const std::exception exception)
        {
            // ErrorMessage displays the error to the user, so we can just return as user has been notified of the exception already.
            return -1;
        }
    }

	LRESULT Application::WindowProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
	{
        Engine* engine = reinterpret_cast<Engine*>(GetWindowLongPtr(windowHandle, GWLP_USERDATA));

        // Handle ImGUI messages.
        if (ImGui_ImplWin32_WndProcHandler(windowHandle, message, wParam, lParam))
        {
            return true;
        }

        switch (message)
        {
        case WM_CREATE:
        {
            // Save the Engine* passed in to CreateWindow.
            LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
            SetWindowLongPtr(windowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));

            return 0;
        }break;

        case WM_KEYDOWN:
        {
            if (wParam == VK_ESCAPE)
            {
                ::DestroyWindow(windowHandle);
            }

            if (engine)
            {
                engine->OnKeyAction(wParam, true);
            }

            return 0;
        }break;

        case WM_KEYUP:
        {
            if (engine)
            {
                engine->OnKeyAction(wParam, false);
            }

            return 0;
        }break;

        case WM_SIZE:
        {
            if (engine)
            {
                RECT clientRect{};
                ::GetClientRect(windowHandle, &clientRect);

                const Uint2 clientDimensions
                {
                    .x = static_cast<uint32_t>(clientRect.right),
                    .y = static_cast<uint32_t>(clientRect.bottom)
                };

                engine->Resize(clientDimensions);
            }
            
            return 0;
        }break;

        case WM_DESTROY:
        {
            ::PostQuitMessage(0);
            return 0;
        }break;
        }

        return ::DefWindowProc(windowHandle, message, wParam, lParam);
	}
}