#include"../include/common.h"
#include <cstdint>

// Initialize the features
void InitializeFeatures(LPVOID buffer)
{
    // TODO: Add a comment explaining what hk_InitFeatures does
    NTWindowsHookEx::hk_InitFeatures(pDllData.base);
}

// Hook into the DirectX 11 renderer
void OverlayHook(LPVOID buffer)
{
    const int kMaxAttempts = 5; // Limit the number of attempts to initialize kiero
    int attempts = 0;
    bool initialized = false;

    do {
        // Attempt to initialize kiero
        const auto status = kiero::init(kiero::RenderType::D3D11);
        if (status != kiero::Status::Success) {
            // TODO: Handle initialization errors appropriately
            continue;
        }

        // Bind the Present function to the hkPresent function
        if (kiero::bind(8, (void**)&pInterface->oPresent, hkPresent) == kiero::Status::Success) {
            initialized = true;
            break;
        }

        attempts++;
    } while (!initialized && attempts < kMaxAttempts);
}

// Initialize the program
void Initialize()
{
    // Create the overlay thread
    if (!NTWindowsHookEx::NTWindowsCreateThreadEx((LPTHREAD_START_ROUTINE)hk_Overlay, NULL, NULL))
    {
        // Handle error if the thread creation fails
        MessageBox(NULL, "Failed to create overlay thread", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    // Create the features initialization thread
    if (!NTWindowsHookEx::NTWindowsCreateThreadEx((LPTHREAD_START_ROUTINE)InitFeatures, NULL, NULL))
    {
        // Handle error if the thread creation fails
        MessageBox(NULL, "Failed to create features initialization thread", "Error", MB_OK | MB_ICONERROR);
        return;
    }
}

BOOL APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            // Disable thread library calls for this DLL
            DisableThreadLibraryCalls(hInstance);

            // Perform any necessary initialization tasks here
            if (!Initialize())
            {
                // Initialization failed, unload the DLL
                return FALSE;
            }
            break;
        }

        case DLL_PROCESS_DETACH:
        {
            // Perform any necessary cleanup tasks here
            Cleanup();
            break;
        }

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        {
            // Do nothing for thread-related reasons
            break;
        }
    }

    // Return TRUE to indicate success
    return TRUE;
}

