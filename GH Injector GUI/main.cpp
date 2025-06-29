/*
 * Author:       Broihon
 * Copyright:    Guided Hacking™ © 2012-2023 Guided Hacking LLC
 */

#include "pch.h"

#include "CmdArg.h"
#include "DebugConsole.h"
#include "GuiMain.h"

#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:wmainCRTStartup")

int wmain(int argc, wchar_t *argv[])
{
#ifdef DEBUG_CONSOLE_TO_CMD
    AllocConsole();
    FILE *pFile = nullptr;
    freopen_s(&pFile, "CONOUT$", "w", stdout);
#endif

    if (argc > 1)
    {
        bool silent = false;
        auto ret = CmdArg(argc, argv, silent);

        if (silent)
            return 0;

        if (ret != 0)
            Sleep((DWORD)-1);

        Sleep(1500);

        return 0;
    }

    std::vector<QByteArray> utf8Args;
    std::vector<char *> argvUtf8;

    for (int i = 0; i < argc; ++i)
    {
        utf8Args.emplace_back(QString::fromWCharArray(argv[i]).toUtf8());
        argvUtf8.push_back(utf8Args.back().data());
    }

    argvUtf8.push_back(nullptr); // Null-terminate as expected by convention

    SetProcessDPIAware();

    QApplication app(argc, argvUtf8.data());
    QApplication::setWindowIcon(QIcon(":/GuiMain/gh_resource/GH Icon.ico"));

    GuiMain MainWindow;
    MainWindow.show();

    g_print("GH Injector V%ls\n", GH_INJ_GUI_VERSIONW.c_str());
    g_print("Initializing GUI\n");

    MainWindow.initSetup();
    g_print("GUI initialized\n");

    return app.exec();
}