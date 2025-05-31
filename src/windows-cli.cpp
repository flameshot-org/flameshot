#include <iostream>
#include <windows.h>

std::wstring joinArgs(int argc, wchar_t* argv[])
{
    std::wstring result;
    for (int i = 1; i < argc; ++i) {
        if (i > 1) {
            result += L" ";
        }
        result += argv[i];
    }
    return result;
}

void CallFlameshot(const std::wstring args, bool wait)
{
    // generate full path for flameshot executable
    wchar_t path[MAX_PATH];
    int pathLength = GetModuleFileNameW(NULL, path, MAX_PATH);
    std::wstring pathstring(path);

    // Find the last backslash to isolate the filename
    size_t lastBackslash = pathstring.find_last_of(L'\\');
    std::wstring directory = (lastBackslash != std::wstring::npos)
                               ? pathstring.substr(0, lastBackslash + 1)
                               : L"";

    // generate command string
    // note: binary path placed within quotes in case of spaces in path
    int cmdSize = 32 + sizeof(directory) + sizeof(args);
    wchar_t* cmd = (wchar_t*)malloc(sizeof(wchar_t) * cmdSize);
    swprintf(cmd,
             cmdSize,
             L"\"%s\\flameshot.exe\" %s",
             directory.c_str(),
             args.c_str());
    // call subprocess
    FILE* stream = _wpopen(cmd, L"r");
    free(cmd);
    if (wait) {
        if (stream) {
            const int MAX_BUFFER = 2048;
            char buffer[MAX_BUFFER];
            while (!feof(stream)) {
                if (fgets(buffer, MAX_BUFFER, stream) != NULL) {
                    std::cout << buffer;
                }
            }
        }
        _pclose(stream);
    }
    return;
}

// Console 'wrapper' for flameshot on windows
int wmain(int argc, wchar_t* argv[])
{
    // if no args, do not wait for stdout
    if (argc == 1) {
        std::cout << "Starting flameshot in daemon mode" << std::endl;
        CallFlameshot(L"", false);
    } else {
        std::wstring argString = joinArgs(argc, argv);
        CallFlameshot(argString, true);
    }
    std::cout.flush();
    return 0;
}
