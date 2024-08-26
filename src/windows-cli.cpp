#include <iostream>
#include <windows.h>

std::string joinArgs(int argc, char* argv[])
{
    std::string result;
    for (int i = 1; i < argc; ++i) {
        if (i > 1) {
            result += " ";
        }
        result += argv[i];
    }
    return result;
}

bool setDirectory()
{
    char path[MAX_PATH];
    int pathLength = GetModuleFileName(NULL, path, MAX_PATH);
    // fullpath - length of 'flameshot-cli.exe'
    std::string moduleDir = std::string(path, pathLength - 18);
    return SetCurrentDirectory(moduleDir.c_str());
}

void CallFlameshot(const std::string args, bool wait)
{
    // _popen doesn't handle spaces in filepath,
    // so make sure we are in right directory before calling
    setDirectory();
    std::string cmd = "flameshot.exe " + args;
    FILE* stream = _popen(cmd.c_str(), "r");
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
int main(int argc, char* argv[])
{
    if (argc == 1) {
        std::cout << "Starting flameshot in daemon mode";
        CallFlameshot("", false);
    } else {
        std::string argString = joinArgs(argc, argv);
        CallFlameshot(argString, true);
    }
    std::cout.flush();
    return 0;
}