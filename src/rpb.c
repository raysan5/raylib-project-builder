/*******************************************************************************************
*
*   rpb v1.0 - A simple and easy-to-use raylib project builder
*
*   FEATURES:
*       - Feature 01
*       - Feature 02
*       - Feature 03
*
*
*   LIMITATIONS:
*       - Limitation 01
*       - Limitation 02
*
*   CONFIGURATION:
*       #define COMMAND_LINE_ONLY
*           Compile tool only for command line usage
*
*       #define CUSTOM_MODAL_DIALOGS
*           Use custom raygui generated modal dialogs instead of native OS ones
*           NOTE: Avoids including tinyfiledialogs depencency library
*
*   VERSIONS HISTORY:
*       1.0  (01-Jun-2026)  First release
*
*   DEPENDENCIES:
*       raylib 6.1-dev          - Windowing/input management and drawing
*       raygui 5.0-dev          - Immediate-mode GUI controls with custom styling and icons
*       rpng 1.5                - PNG chunks management
*       rini 3.0                - Configuration file load/save
*       tinyfiledialogs 3.19.1  - Opensave file dialogs, it requires linkage with comdlg32 and ole32 libs
*
*   BUILDING:
*     - Windows (MinGW-w64):
*       gcc -o rpb.exe rpb.c external/tinyfiledialogs.c rpb.rc.data -s -O2 -std=c99 -Wall -Iexternal /
*           -lraylib -lopengl32 -lgdi32 -lcomdlg32 -lole32
*
*     - Linux (GCC):
*       gcc -o rpb rpb.c external/tinyfiledialogs.c -s -no-pie -Iexternal -D_DEFAULT_SOURCE /
*           -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
*
*   ADDITIONAL NOTES:
*       On PLATFORM_ANDROID and PLATFORM_WEB file dialogs are not available and CUSTOM_MODAL_DIALOGS are used
*
*   DEVELOPERS:
*       Ramon Santamaria (@raysan5):    Developer, supervisor, designer and maintainer
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2025-2026 raylib technologies (@raylibtech) / Ramon Santamaria (@raysan5)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#define TOOL_NAME                   "rpb"
#define TOOL_SHORT_NAME             "rpb"
#define TOOL_VERSION                "1.0"
#define TOOL_DESCRIPTION            "A simple and easy-to-use raylib project builder"
#define TOOL_DESCRIPTION_BREAK      "A simple and easy-to-use\nraylib project builder"
#define TOOL_RELEASE_DATE           "Jun.2026"
#define TOOL_LOGO_COLOR             0x7c7c80ff
#define TOOL_CONFIG_FILENAME        "rpb.ini"

#include "raylib.h"

#if defined(PLATFORM_WEB)
    #define CUSTOM_MODAL_DIALOGS            // Force custom modal dialogs usage
    #include <emscripten/emscripten.h>      // Emscripten library - LLVM to JavaScript compiler
    #include <emscripten/html5.h>           // Emscripten HTML5 browser functionality (emscripten_set_beforeunload_callback)
#endif

#define RAYGUI_IMPLEMENTATION
#include "external/raygui.h"

#undef RAYGUI_IMPLEMENTATION                // Avoid including raygui implementation again

#define GUI_MAIN_TOOLBAR_IMPLEMENTATION
#include "gui_main_toolbar.h"               // GUI: Main toolbar

#define GUI_WINDOW_HELP_IMPLEMENTATION
#include "gui_window_help.h"                // GUI: Help Window

#define GUI_WINDOW_ABOUT_IMPLEMENTATION
#include "gui_window_about_welcome.h"       // GUI: About/Welcome Window

#define GUI_FILE_DIALOGS_IMPLEMENTATION
#include "gui_file_dialogs.h"               // GUI: File Dialogs

// raygui embedded styles
// NOTE: Included in the same order as selector
#define MAX_GUI_STYLES_AVAILABLE      5
#include "styles/style_genesis.h"           // raygui style: genesis
#include "styles/style_cyber.h"             // raygui style: cyber
#include "styles/style_lavanda.h"           // raygui style: lavanda
#include "styles/style_terminal.h"          // raygui style: terminal
#include "styles/style_amber.h"             // raygui style: amber

// NOTE: Using same config than [rpc], for .rpc files consistency
#define RINI_MAX_VALUE_CAPACITY     256
#define RINI_MAX_TEXT_SIZE          256
#define RINI_KEY_SPACING             37
#define RINI_VALUE_SPACING           35
#define RINI_IMPLEMENTATION
#include "external/rini.h"                  // Config file values reader/writer

// WARNING: rini.h must be included before rpconfig to avoid re-definitions
#define RPCONFIG_IMPLEMENTATION
#include "rpconfig.h"                       // Data types and functionality (shared by [rpc] and [rpb] tools)

#define RPNG_IMPLEMENTATION
//#define RPNG_DEFLATE_IMPLEMENTATION       // Required when compiling raylib as DLL
#include "external/rpng.h"                  // PNG chunks management

// Standard C libraries
#include <stdlib.h>                         // Required for: NULL, malloc(), free()
#include <stdio.h>                          // Required for: fopen(), fclose(), fread()...
#include <string.h>                         // Required for: strcmp(), strlen()
#include <math.h>                           // Required for: powf()
#include <time.h>                           // Required for: time(), localtime()

//#define COMMAND_LINE_ONLY

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#if (!defined(_DEBUG) && (defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)))
// WARNING: Comment if LOG() output is required for this tool
bool __stdcall FreeConsole(void);           // Close console from code (kernel32.lib)
#endif

// Simple log system to avoid printf() calls if required
// NOTE: Avoiding those calls, also avoids const strings memory usage
#define SUPPORT_LOG_INFO
#if defined(SUPPORT_LOG_INFO)
    #define LOG(...) printf(__VA_ARGS__)
#else
    #define LOG(...)
#endif

#if defined(_WIN32)
    #define PUTENV _putenv
#else
    #define PUTENV putenv
#endif

#define RPC_MAX_SOURCE_FILES        128
#define RPC_MAX_ASSET_FILES         256
#define RPC_SOURCE_PATH_LENGTH      256     // Source file path length
#define RPC_ASSET_PATH_LENGTH       256     // Asset file path length

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// NOTE: [rpc] and [rpb] tools shared data types and functions are provided by rpcdata.h

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static const char *toolName = TOOL_NAME;
static const char *toolVersion = TOOL_VERSION;
static const char *toolDescription = TOOL_DESCRIPTION;

static const int screenWidth = 1060;        // Default screen width (at initialization)
static const int screenHeight = 860;        // Default screen height (at initialization)

// NOTE: Max length depends on OS, in Windows MAX_PATH = 256
static char inFileName[256] = { 0 };        // Input file name (required in case of drag & drop over executable)
static char outFileName[256] = { 0 };       // Output file name (required for file save/export)

static char inFilePath[256] = { 0 };        // Input file path
//static char inDirectoryPath[256] = { 0 };   // Input directory path

//static int framesCounter = 0;               // General pourpose frames counter (not used)
//static Vector2 mousePoint = { 0 };          // Mouse position
//static Texture2D texNoise = { 0 };          // Background noise texture
static bool lockBackground = false;         // Toggle lock background (controls locked)
static bool saveChangesRequired = false;    // Flag to notice save changes are required

//static RenderTexture2D target = { 0 };      // Render texture to render the tool (if required)

#define MAX_PLATFORMS   10
static Texture2D texPlatforms = { 0 };      // Platform logos texture for toggles
static bool platformEnabled[MAX_PLATFORMS] = { 0 };

static int tabActive = 1;

static Vector2 panelScroll = { 0 };
static Rectangle panelView = { 0 };

// HOST platforms:  0-Windows, 1-Linux, 2-macOS, 3-Web
// BUILD platforms: 0-Windows, 1-Linux, 2-macOS, 3-Wasm, 4-Android, 5-FreeBSD, 6-DRM, 7-ESP32, 8-Dreamcast, 9-Switch, ...
static bool buildPlatformsEnabled[10] = {
#if defined(_WIN32)
    true, true, false, true, true, false, false, true, true, false
#elif defined(__linux__)
    false, true, false, true, true, false, true, true, false, false
#elif defined(__APPLE__)
    false, false, true, true, false, false, false, false, false, false
#elif defined(__EMSCRIPTEN__)
    false, false, false, false, false, false, false, false, false, false
#endif
};

#if defined(_WIN32)
static int hostPlatformId = 0;
const char *hostPlatform = "Windows";
static int currentPlatform = RPC_PLATFORM_WINDOWS;
#elif defined(__linux__)
static int hostPlatformId = 1;
const char *hostPlatform = "Linux";
static int currentPlatform = RPC_PLATFORM_LINUX;
#elif defined(__APPLE__)
static int hostPlatformId = 2;
const char *hostPlatform = "macOS";
static int currentPlatform = RPC_PLATFORM_MACOS;
#elif defined(__EMSCRIPTEN__)
static int hostPlatformId = 3;
const char *hostPlatform = "Web";
static int currentPlatform = RPC_PLATFORM_WASM;
#endif

#if defined(__x86_64__) || defined(_M_X64)
const char *hostArch = "x64";
#elif defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86)
const char *hostArch = "x86";
#elif defined(__aarch64__) || defined(_M_ARM64)
const char *hostArch = "arm64";
#elif defined(mips) || defined(__mips__) || defined(__mips)
const char *hostArch = "MIPS";
#else
const char *hostArch = "-";
#endif

static const char *platformNames[] = { "Windows", "Linux", "macOS", "Wasm", "Android", "FreeBSD", "DRM", "ESP32", "Dreamcast", "Switch" };
static const char *platformOutExtension[] = { ".exe", "", ".app", ".html", ".apk", "", ".bin", ".bin", ".nsp" };

// Basic program variables
//----------------------------------------------------------------------------------
static rpcProjectConfig project = { 0 };        // Project config data

static char inProjectFilePath[256] = { 0 };     // Project file path
static char outProjectFilePath[256] = { 0 };    // Project output path (build path)

static bool showMessageReset = false;           // Show message: reset
static bool showMessageExit = false;            // Show message: exit (quit)

static bool saveProjectRequired = false;        // Flag to detect if project needs to be saved
static bool runProjectRequired = false;         // Flag to request project run after building
//-----------------------------------------------------------------------------------

// Support Message Box
//-----------------------------------------------------------------------------------
#if defined(SPLASH_SUPPORT_MESSAGE)
static bool showSupportMessage = true;      // Support message box splash message at startup
static int supportMessageRandBtn = 0;       // Support message buttons random position
#else
static bool showSupportMessage = false;
#endif
//-----------------------------------------------------------------------------------

// GUI: Main toolbar panel
//-----------------------------------------------------------------------------------
static GuiMainToolbarState mainToolbarState = { 0 };
//-----------------------------------------------------------------------------------

// GUI: Help Window
//-----------------------------------------------------------------------------------
static GuiWindowHelpState windowHelpState = { 0 };
//-----------------------------------------------------------------------------------

// GUI: About Window
//-----------------------------------------------------------------------------------
static GuiWindowAboutState windowAboutState = { 0 };
//-----------------------------------------------------------------------------------

// GUI: Issue Report Window
//-----------------------------------------------------------------------------------
static bool showIssueReportWindow = false;
//-----------------------------------------------------------------------------------

// GUI: User Window
//-----------------------------------------------------------------------------------
//static GuiWindowUserState windowUserState = { 0 };
//-----------------------------------------------------------------------------------

// GUI: Exit Window
//-----------------------------------------------------------------------------------
static bool closeWindow = false;
//static bool windowExitActive = false;
//-----------------------------------------------------------------------------------

// GUI: Custom file dialogs
//-----------------------------------------------------------------------------------
// Generic file/directory loaders for projeect config properties
static bool showLoadFileDialog = false;
static bool showLoadDirectoryDialog = false;

static bool showLoadProjectDialog = false;
static bool showSaveAsProjectDialog = false;
static bool showBuildProjectDialog = false;
static bool buildProjectDeferred = false;       // Defer project bulding to show message

static int projectEditProperty = -1;
//-----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
#if defined(PLATFORM_DESKTOP) || defined(COMMAND_LINE_ONLY)
// Command line functionality
static void ShowCommandLineInfo(void);                      // Show command line usage info
static void ProcessCommandLine(int argc, char *argv[]);     // Process command line input
#endif

static void UpdateDrawFrame(void);                          // Update and draw one frame

// Load/Save/Export data functions
static int BuildProject(rpcProjectConfig project, int platform, const char *buildPath); // Build project for target platform

// Auxiliar functions
static int DirectoryCopy(const char *srcPath, const char *dstPath); // Copy full directory with all content
static bool IsPathAbsolute(const char *path);               // Check if provided path is an absolute path
#if defined(_WIN32)
static const char *PathToWSL(const char *path);
#endif
//------------------------------------------------------------------------------------

// Load/Save application configuration
// NOTE: Functions operate over global variables
//------------------------------------------------------------------------------------
static void LoadApplicationConfig(void);
static void SaveApplicationConfig(void);
#if defined(PLATFORM_WEB)
// Load/Save data on web LocalStorage (persistent between sessions)
static void SaveWebLocalStorage(const char *key, const char *value);
static char *LoadWebLocalStorage(const char *key);
// Web function to be called before page unload/close
static const char *CallBeforeWebUnload(int eventType, const void *reserved, void *userData) { SaveApplicationConfig(); return NULL; }
#endif
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
//#if !defined(_DEBUG)
    SetTraceLogLevel(LOG_NONE);         // Disable raylib trace log messsages
//#endif
#if defined(COMMAND_LINE_ONLY)
    ProcessCommandLine(argc, argv);
#else
#if defined(PLATFORM_DESKTOP)
    // Command-line usage mode
    //--------------------------------------------------------------------------------------
    if (argc > 1)
    {
        if ((argc == 2) &&
            (strcmp(argv[1], "-h") != 0) &&
            (strcmp(argv[1], "--help") != 0))       // One argument (file dropped over executable?)
        {
            if (IsFileExtension(argv[1], ".rpc"))
            {
                strcpy(inProjectFilePath, argv[1]);        // Read input filename to open with gui interface
            }
        }
        else
        {
            ProcessCommandLine(argc, argv);
            return 0;
        }
    }
#endif

#if (!defined(_DEBUG) && (defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)))
    // WARNING (Windows): If program is compiled as Window application (instead of console),
    // no console is available to show output info... solution is compiling a console application
    // and closing console (FreeConsole()) when changing to GUI interface
    // WARNING: Comment in case LOG() output is required for this tool
    //FreeConsole();
#endif

    // GUI usage mode - Initialization
    //--------------------------------------------------------------------------------------
    //SetConfigFlags(FLAG_WINDOW_RESIZABLE);      // Window configuration flags
    InitWindow(screenWidth, screenHeight, TextFormat("%s v%s | %s", toolName, toolVersion, toolDescription));
    //SetWindowMinSize(1280, 720);
    SetExitKey(0);

    // Create a RenderTexture2D to be used for render to texture
    //target = LoadRenderTexture(512, 512);
    //SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);

    texPlatforms = LoadTexture("resources/platforms.png");
    SetTextureFilter(texPlatforms, TEXTURE_FILTER_BILINEAR);

    LOG("INIT: Ready to show project building info...\n");
    LOG("-----------------------------------------------------------------\n");

    // GUI: Main toolbar panel (file and visualization)
    //-----------------------------------------------------------------------------------
    mainToolbarState = InitGuiMainToolbar();

    // Set raygui style to start with
    // WARNING: It must be aligned with mainToolbarState.visualStyleActive
    GuiLoadStyleGenesis();

    GuiEnableTooltip();     // Enable tooltips by default
    //-----------------------------------------------------------------------------------

    // GUI: Help Window
    //-----------------------------------------------------------------------------------
    windowHelpState = InitGuiWindowHelp();
    //-----------------------------------------------------------------------------------

    // GUI: About Window
    //-----------------------------------------------------------------------------------
    windowAboutState = InitGuiWindowAbout();
    //-----------------------------------------------------------------------------------

    // GUI: User Window
    //-----------------------------------------------------------------------------------
    //windowUserState = InitGuiWindowUser();
    //-----------------------------------------------------------------------------------

    // Load application init configuration (if available)
    //-------------------------------------------------------------------------------------
    LoadApplicationConfig();
#if defined(PLATFORM_WEB)
    // Set callback to automatically save app config on page closing
    emscripten_set_beforeunload_callback(NULL, CallBeforeWebUnload);
#endif
    //-------------------------------------------------------------------------------------

#if !defined(PLATFORM_WEB)
    // File dropped over executable / command line input file
    //-------------------------------------------------------------------------------------
    if ((inProjectFilePath[0] != '\0') && (IsFileExtension(inProjectFilePath, ".rpc")))
    {
        rpcProjectConfig prjsrc = rpcLoadProjectConfig(inProjectFilePath);    // Load tool data from file
        rpcUnloadProjectConfig(project);
        project = rpcLoadProjectConfig("resources/project_template.rpc");

        // Copy loaded project data over project template data, so
        // all required build info is available if not provided on original file
        for (int j = 0; j < prjsrc.entryCount; j++)
        {
            int result = rpcSetText(project, prjsrc.entries[j].key, prjsrc.entries[j].text);
            if (result != -1) saveProjectRequired = true;
        }

        rpcUnloadProjectConfig(prjsrc);

        if (saveProjectRequired) SetWindowTitle(TextFormat("%s v%s - %s*", toolName, toolVersion, GetFileName(inProjectFilePath)));
        else SetWindowTitle(TextFormat("%s v%s - %s", toolName, toolVersion, GetFileName(inProjectFilePath)));
    }
    //-------------------------------------------------------------------------------------
#endif

    // Initialize inout file paths to working directory
    strcpy(inFileName, GetWorkingDirectory());
    strcpy(inFilePath, GetWorkingDirectory());

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);       // Set our game frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!closeWindow)    // Program must finish
    {
        UpdateDrawFrame();
    }
#endif

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(texPlatforms);

    rpcUnloadProjectConfig(project);

    //UnloadRenderTexture(target);    // Unload render texture

    // Save application init configuration for next run
    //--------------------------------------------------------------------------------------
    SaveApplicationConfig();
    //--------------------------------------------------------------------------------------

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

#endif // !COMMAND_LINE_ONLY
    return 0;
}

//--------------------------------------------------------------------------------------------
// Module Functions Definition
//--------------------------------------------------------------------------------------------
// Update and draw one frame
static void UpdateDrawFrame(void)
{
#if !defined(PLATFORM_WEB)
    // WARNING: ASINCIFY requires this line,
    // it contains the call to emscripten_sleep() for PLATFORM_WEB
    if (WindowShouldClose()) closeWindow = true;
#endif

    // Dropped files logic
    //----------------------------------------------------------------------------------
    if (IsFileDropped())
    {
        FilePathList droppedFiles = LoadDroppedFiles();

        if ((droppedFiles.count == 1) && IsFileExtension(droppedFiles.paths[0], ".rgs"))
        {
            // Reset to default internal style
            // NOTE: Required to unload any previously loaded font texture
            GuiLoadStyleDefault();
            GuiLoadStyle(droppedFiles.paths[0]);
        }
        else if (IsFileExtension(droppedFiles.paths[0], ".rpc"))
        {
            strcpy(inProjectFilePath, droppedFiles.paths[0]);
            rpcUnloadProjectConfig(project);
            project = rpcLoadProjectConfig(inProjectFilePath);
            /*
            rpcProjectConfig prjsrc = rpcLoadProjectConfig(inProjectFilePath); // Load tool data from file
            rpcUnloadProjectConfig(project);
            project = rpcLoadProjectConfig("resources/project_template.rpc");

            // Copy loaded project data over project template data, so
            // all required build info is available if not provided on original file
            for (int j = 0; j < prjsrc.entryCount; j++)
            {
                int result = rpcSetText(project, prjsrc.entries[j].key, prjsrc.entries[j].text);
                if (result != -1) saveProjectRequired = true;
            }

            rpcUnloadProjectConfig(prjsrc);
            */

            if (saveProjectRequired) SetWindowTitle(TextFormat("%s v%s - %s*", toolName, toolVersion, GetFileName(inProjectFilePath)));
            else SetWindowTitle(TextFormat("%s v%s - %s", toolName, toolVersion, GetFileName(inProjectFilePath)));
        }
        else if (!IsPathFile(droppedFiles.paths[0])) // Dropped directory
        {
            // TODO: Check if it is a valid project directory and contains a .rpc file
        }

        /*
        else if (IsFileExtension(droppedFiles.paths[0], ".rkey"))
        {
            if (windowUserState.windowActive && !windowUserState.registered &&
                CheckCollisionPointRec(GetMousePosition(), windowUserState.licenseBox))
            {
                // Load .rkey file and send it to windowUserState
                // NOTE: Text data is unloaded internally when processed (on current frame)
                windowUserState.externalKey = LoadFileText(droppedFiles.paths[0]);
            }
        }
        */

        UnloadDroppedFiles(droppedFiles);    // Unload filepaths from memory
    }
    //----------------------------------------------------------------------------------

    // Keyboard shortcuts
    //------------------------------------------------------------------------------------
    // Toggle window: help
    if (IsKeyPressed(KEY_F1)) windowHelpState.windowActive = !windowHelpState.windowActive;

    // Toggle window: about
    if (IsKeyPressed(KEY_F2)) windowAboutState.windowActive = !windowAboutState.windowActive;

    // Toggle window: issue report
    if (IsKeyPressed(KEY_F3)) showIssueReportWindow = !showIssueReportWindow;

    // Toggle window: user
    //if (IsKeyPressed(KEY_F4)) windowUserState.windowActive = !windowUserState.windowActive;

    // Show dialog: load file (.rpc)
    if ((IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_O)) || mainToolbarState.btnLoadFilePressed)
    {
        TextCopy(inProjectFilePath, GetWorkingDirectory());
        showLoadProjectDialog = true;
    }

    // Show dialog: save file as (.rpc)
    if ((IsKeyDown(KEY_LEFT_CONTROL) && IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_S)) || mainToolbarState.btnSaveFilePressed) showSaveAsProjectDialog = true;

    // Show dialog: build project
    if ((IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_B)) || mainToolbarState.btnBuildProjectPressed)
    {
#if defined(_WIN32)
        TextCopy(outProjectFilePath, TextFormat("%s\\%s", GetDirectoryPath(inFileName), rpcGetText(project, "BUILD_OUTPUT_PATH")));
#else
        TextCopy(outProjectFilePath, TextFormat("%s/%s", GetDirectoryPath(inFileName), rpcGetText(project, "BUILD_OUTPUT_PATH")));
#endif
        showBuildProjectDialog = true;
    }

    // Save current project config (.rpc)
    if (((IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S)) || mainToolbarState.btnLoadFilePressed) && saveProjectRequired)
    {
        // File to be updated with changes: inProjectFilePath
        // Update project configuration .rpc to defined values by [rpc] tool
        rini_data data = rini_load_full("resources/project_template.rpc");
        for (unsigned int i = 0; i < data.count; i++)
        {
            for (int j = 0; j < project.entryCount; j++)
            {
                if (TextIsEqual(project.entries[j].key, data.entries[i].key) &&
                    !TextIsEqual(project.entries[j].text, data.entries[i].text))
                {
                    memset(data.entries[i].text, 0, RINI_MAX_TEXT_SIZE);
                    strcpy(data.entries[i].text, project.entries[j].text);
                    // TODO: Check project.entries[j].type to set data.values[i].value and data.values[i].is_text
                }
            }
        }
        rini_save(data, inProjectFilePath);
        saveProjectRequired = false;

        SetWindowTitle(TextFormat("%s v%s - %s", toolName, toolVersion, GetFileName(inProjectFilePath)));
    }

    // Show closing window on ESC
    if (IsKeyPressed(KEY_ESCAPE))
    {
        if (windowHelpState.windowActive) windowHelpState.windowActive = false;
        else if (windowAboutState.windowActive) windowAboutState.windowActive = false;
        else if (showIssueReportWindow) showIssueReportWindow = false;
    #if defined(PLATFORM_DESKTOP)
        else if (saveChangesRequired) showMessageExit = !showMessageExit;
        else closeWindow = true;
    #else
        else if (showLoadFileDialog) showLoadFileDialog = false;
        else if (showLoadDirectoryDialog) showLoadDirectoryDialog = false;
        else if (showLoadProjectDialog) showLoadProjectDialog = false;
        else if (showSaveAsProjectDialog) showSaveAsProjectDialog = false;
        else if (showBuildProjectDialog) showBuildProjectDialog = false;
    #endif
    }
    //----------------------------------------------------------------------------------

    // Main toolbar logic
    //----------------------------------------------------------------------------------
    // Visual options logic
    if (mainToolbarState.visualStyleActive != mainToolbarState.prevVisualStyleActive)
    {
        // Reset to default internal style
        // NOTE: Required to unload any previously loaded font texture
        GuiLoadStyleDefault();

        switch (mainToolbarState.visualStyleActive)
        {
            case 0: GuiLoadStyleGenesis(); break;
            case 1: GuiLoadStyleCyber(); break;
            case 2: GuiLoadStyleAmber(); break;
            case 3: GuiLoadStyleTerminal(); break;
            case 4: GuiLoadStyleLavanda(); break;
            default: break;
        }

        mainToolbarState.prevVisualStyleActive = mainToolbarState.visualStyleActive;
    }

    // Help options logic
    if (mainToolbarState.btnHelpPressed) windowHelpState.windowActive = true;
    if (mainToolbarState.btnAboutPressed) windowAboutState.windowActive = true;
    if (mainToolbarState.btnIssuePressed) showIssueReportWindow = true;
    //if (mainToolbarState.btnUserPressed) windowUserState.windowActive = true;
    //----------------------------------------------------------------------------------

    // Basic program flow logic
    //----------------------------------------------------------------------------------
#if !defined(PLATFORM_WEB)
    if (WindowShouldClose())
    {
        if (saveChangesRequired) showMessageExit = true;
        else closeWindow = true;
    }
#endif

    // WARNING: Some windows should lock the main screen controls when shown
    if (windowHelpState.windowActive ||
        windowAboutState.windowActive ||
        showIssueReportWindow ||
        //windowUserState.windowActive ||
        showMessageExit ||
        showMessageReset ||
        showLoadFileDialog ||
        showLoadDirectoryDialog ||
        showLoadProjectDialog ||
        showSaveAsProjectDialog ||
        showBuildProjectDialog ||
        showSupportMessage)
    {
        lockBackground = true;
    }
    else lockBackground = false;
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

        ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

        // Draw all program UI
        //----------------------------------------------------------------------------------
        // Draw selected build platform
        for (int i = 0; i < MAX_PLATFORMS; i++)
        {
            if (currentPlatform == i) platformEnabled[i] = true;
            if (platformEnabled[i] && (currentPlatform != i)) platformEnabled[i] = false;
        }

        GuiLabel((Rectangle){ 12, 44, GetScreenWidth(), 24 },
            TextFormat("HOST PLATFORM: %s (%s) - SELECT TARGET BUILD PLATFORM:", hostPlatform, hostArch));

        // NOTE: Enabled platforms depend on HOST platform
        for (int i = 0; i < MAX_PLATFORMS; i++)
        {
            if (!buildPlatformsEnabled[i]) GuiDisable();

            if (i == hostPlatformId) DrawRectangleRec((Rectangle){ 12 + (96 + 8)*i - 2, 76 - 2, 100, 100 },
                Fade(GetColor(GuiGetStyle(DEFAULT, BASE_COLOR_PRESSED)), 0.8f));

            GuiToggle((Rectangle){ 12 + (96 + 8)*i, 76, 96, 96 }, NULL, &platformEnabled[i]);
            GuiEnable();

            Color colTex = GetColor(GuiGetStyle(TOGGLE, TEXT_COLOR_NORMAL));
            if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){ 12 + (96 + 8)*i, 76, 96, 96 }))
                colTex = GetColor(GuiGetStyle(TOGGLE, TEXT_COLOR_FOCUSED));
            if (!buildPlatformsEnabled[i]) colTex = GetColor(GuiGetStyle(TOGGLE, TEXT_COLOR_DISABLED));
            else if (platformEnabled[i]) colTex = GetColor(GuiGetStyle(TOGGLE, TEXT_COLOR_PRESSED));

            DrawTexturePro(texPlatforms, (Rectangle){ 128*i, 0, 128, 128 }, (Rectangle){ 12 + (96 + 8)*i, 76, 96, 96 },
                (Vector2){ 0.0f, 0.0f }, 0.0f, colTex);
        }

        for (int i = 0; i < MAX_PLATFORMS; i++)
        {
            if (platformEnabled[i] && (i != currentPlatform)) { currentPlatform = i; break; }
        }

        int propsPanelOffsetY = 76 + 96 + 12;

        // Draw tab bar for project config categories: PROJECT, BUILD, PLATFORM...
        if (project.entryCount == 0)
        {
            GuiSetIconScale(3);
            GuiSetStyle(DEFAULT, TEXT_SIZE, GuiGetFont().baseSize*2);
            GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
            GuiLabel((Rectangle){ 0, propsPanelOffsetY, GetScreenWidth(), GetScreenHeight() - 188 - 56 - 24 },
                "#10#Drag & drop or load a project config file .rpc");
            GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
            GuiSetStyle(DEFAULT, TEXT_SIZE, GuiGetFont().baseSize);
            GuiSetIconScale(1);

            GuiDisable();
        }

        GuiSetStyle(TABBAR, TAB_ITEMS_WIDTH, 172);
        GuiTabBar((Rectangle){ 0, propsPanelOffsetY, GetScreenWidth(), 28 },
            "#176#PROJECT SETTINGS;#140#BUILD SETTINGS;#181#PLATFORM SETTINGS;#178#DEPLOY OPTIONS;#12#IMAGERY EDITION;#133#raylib CONFIG", NULL, &tabActive);
        GuiEnable();

        int categoryHeight = 12;
        for (int i = 0; i < project.entryCount; i++)
        {
            if (project.entries[i].category == (tabActive + 1)) categoryHeight += (24 + 8);
        }
        if ((categoryHeight > (GetScreenHeight() - 188 - 48 - 24)) && ((tabActive + 1) != RPC_CAT_PLATFORM))
        {
            GuiScrollPanel((Rectangle){ 0, 188, GetScreenWidth(), GetScreenHeight() - 188 - 56 - 24 }, NULL,
                (Rectangle){ 0, 188, GetScreenWidth() - 16, categoryHeight }, &panelScroll, &panelView);
        }
        else
        {
            panelScroll = (Vector2){ 0 };
            panelView = (Rectangle){ 0, 188, GetScreenWidth(), GetScreenHeight() - 188 - 56 - 24 };
        }

        //DrawRectangleRec(panelView, Fade(RED, 0.6f)); // Debug

        BeginScissorMode((int)panelView.x, (int)panelView.y, (int)panelView.width, (int)panelView.height);

        for (int i = 0, k = 0; i < project.entryCount; i++)
        {
            if ((tabActive + 1) == project.entries[i].category)
            {
                if (project.entries[i].category == RPC_CAT_PROJECT)
                {
                    if (i == 0)
                    {
                        GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
                        GuiSetStyle(DEFAULT, TEXT_SIZE, GuiGetFont().baseSize*2);
                        GuiLabel((Rectangle){ 24, propsPanelOffsetY + 36, GetScreenWidth() - 48, 48 }, "#220# WARNING: Project settings should not be modified here, use [rpc] tool");
                        GuiSetStyle(DEFAULT, TEXT_SIZE, GuiGetFont().baseSize);
                        GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
                    }

                    GuiDisable();
                    propsPanelOffsetY = 76 + 96 + 12 + 64;
                }
                else
                {
                    GuiEnable();
                    propsPanelOffsetY = 76 + 96 + 12;
                }

                if ((project.entries[i].platform != RPC_PLATFORM_ANY) && (project.entries[i].platform != currentPlatform)) continue;

                if (project.entries[i].type != RPC_TYPE_BOOL)
                    GuiLabel((Rectangle){ 24, propsPanelOffsetY + 36 + (24 + 8)*k + panelScroll.y, 180, 24 }, TextFormat("%s:", project.entries[i].name));

                int descWidth = 460;
                int textWidth = GetScreenWidth() - (24 + 180 + 12 + descWidth + 24);

                GuiSetStyle(TEXTBOX, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
                switch (project.entries[i].type)
                {
                    case RPC_TYPE_BOOL:
                    {
                        bool checked = (bool)project.entries[i].value;
                        GuiCheckBox((Rectangle){ 24 + 2, propsPanelOffsetY + 36 + (24 + 8)*k + 2 + panelScroll.y, 20, 20 }, project.entries[i].name + 5, &checked);
                        project.entries[i].value = (checked? 1 : 0);
                    } break;
                    case RPC_TYPE_VALUE:
                    {
                        if (GuiValueBox((Rectangle){ 24 + 180, propsPanelOffsetY + 36 + (24 + 8)*k + panelScroll.y, 180, 24 },
                            NULL, &project.entries[i].value, 0, 1024, project.entries[i].editMode)) project.entries[i].editMode = !project.entries[i].editMode;
                    } break;
                    case RPC_TYPE_TEXT:
                    {
                        if (GuiTextBox((Rectangle){ 24 + 180, propsPanelOffsetY + 36 + (24 + 8)*k + panelScroll.y, textWidth, 24 },
                            project.entries[i].text, 255, project.entries[i].editMode)) project.entries[i].editMode = !project.entries[i].editMode;
                    } break;
                    case RPC_TYPE_TEXT_FILE:
                    {
                        if (GuiTextBox((Rectangle){ 24 + 180, propsPanelOffsetY + 36 + (24 + 8)*k + panelScroll.y, textWidth - 90, 24 },
                            project.entries[i].text, 255, project.entries[i].editMode)) project.entries[i].editMode = !project.entries[i].editMode;
                        if (GuiButton((Rectangle){ 24 + 180 + textWidth - 86, propsPanelOffsetY + 36 + (24 + 8)*k + panelScroll.y, 86, 24 }, "#6#Browse"))
                        {
                            memset(inFileName, 0, 256);
                            showLoadFileDialog = true;
                            projectEditProperty = i;
                        }
                    } break;
                    case RPC_TYPE_TEXT_PATH:
                    {
                        if (GuiTextBox((Rectangle){ 24 + 180, propsPanelOffsetY + 36 + (24 + 8)*k + panelScroll.y, textWidth - 90, 24 },
                            project.entries[i].text, 255, project.entries[i].editMode)) project.entries[i].editMode = !project.entries[i].editMode;

                        if (GuiButton((Rectangle){ 24 + 180 + textWidth - 86, propsPanelOffsetY + 36 + (24 + 8)*k + panelScroll.y, 86, 24 }, "#173#Browse"))
                        {
                            showLoadDirectoryDialog = true;
                            projectEditProperty = i;
                        }
                    } break;
                    default: break;
                }
                GuiSetStyle(TEXTBOX, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);

                // Draw field description
                if (project.entries[i].type == RPC_TYPE_BOOL)
                    GuiStatusBar((Rectangle){ 24 + 180, propsPanelOffsetY + 36 + (24 + 8)*k + panelScroll.y, (textWidth + descWidth + 12), 24 }, project.entries[i].desc);
                else GuiStatusBar((Rectangle){ 24 + 180 + textWidth + 12, propsPanelOffsetY + 36 + (24 + 8)*k + panelScroll.y, descWidth, 24 }, project.entries[i].desc);

                k++;

                GuiEnable();
            }
        }

        EndScissorMode();

        if (project.entryCount == 0) GuiDisable();
        if (GuiButton((Rectangle){ 8, GetScreenHeight() - 24 - 8 - 40, GetScreenWidth() - 16, 40 }, "#131#BUILD and RUN PROJECT"))
        {
#if defined(_WIN32)
            TextCopy(outProjectFilePath, TextFormat("%s\\%s", GetDirectoryPath(inFileName), rpcGetText(project, "BUILD_OUTPUT_PATH")));
#else
            TextCopy(outProjectFilePath, TextFormat("%s/%s", GetDirectoryPath(inFileName), rpcGetText(project, "BUILD_OUTPUT_PATH")));
#endif
            showBuildProjectDialog = true;
            runProjectRequired = true;
        }
        GuiEnable();
        //----------------------------------------------------------------------------------

        // GUI: Main toolbar panel
        //----------------------------------------------------------------------------------
        GuiMainToolbar(&mainToolbarState);
        //----------------------------------------------------------------------------------

        // GUI: Status bar
        //----------------------------------------------------------------------------------
        GuiStatusBar((Rectangle){ 0, GetScreenHeight() - 24, 320, 24 },
            (inProjectFilePath[0] == '\0')? "NO PROJECT LOADED" : TextFormat("PROJECT: %s", GetFileName(inProjectFilePath)));
        GuiStatusBar((Rectangle){ 320 - 1, GetScreenHeight() - 24, 260, 24 },
            TextFormat("TARGET PLATFORM: %s", (inProjectFilePath[0] == '\0')? "-" : platformNames[currentPlatform]));
        GuiStatusBar((Rectangle){ 320 + 260 - 2, GetScreenHeight() - 24, GetScreenWidth() - 320 - 260 + 2 - 24, 24 },
            (inProjectFilePath[0] == '\0')? "OUTPUT: -" : TextFormat("OUTPUT: %s/%s%s", rpcGetText(project, "BUILD_OUTPUT_PATH"),
                rpcGetText(project, "PROJECT_INTERNAL_NAME"), platformOutExtension[currentPlatform]));
        if (GuiButton((Rectangle){ GetScreenWidth() - 24, GetScreenHeight() - 24, 24, 24 }, "#173#"))
        {
            system(TextFormat("start explorer %s", inProjectFilePath)); //rpcGetText(project, "BUILD_OUTPUT_PATH")));
        }
        //----------------------------------------------------------------------------------

        // NOTE: If some overlap window is open and main window is locked, draw a background rectangle
        //if (GuiIsLocked())    // WARNING: It takes one extra frame to process, so just checking required conditions
        if (windowHelpState.windowActive ||
            windowAboutState.windowActive ||
            showIssueReportWindow ||
            //windowUserState.windowActive ||
            showMessageExit ||
            showMessageReset ||
            showLoadFileDialog ||
            showLoadDirectoryDialog ||
            showLoadProjectDialog ||
            showSaveAsProjectDialog ||
            showBuildProjectDialog ||
            showSupportMessage)
        {
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)), 0.8f));
        }

        // WARNING: Before drawing the windows, unlock raygui input
        GuiUnlock();

        // GUI: Help Window
        //----------------------------------------------------------------------------------------
        GuiWindowHelp(&windowHelpState);
        //----------------------------------------------------------------------------------------

        // GUI: About Window
        //----------------------------------------------------------------------------------------
        GuiWindowAbout(&windowAboutState);
        //----------------------------------------------------------------------------------------

        // GUI: Issue Report Window
        //----------------------------------------------------------------------------------------
        if (showIssueReportWindow)
        {
            Rectangle messageBox = { (float)GetScreenWidth()/2 - 300/2, (float)GetScreenHeight()/2 - 190/2 - 20, 300, 190 };
            int result = GuiMessageBox(messageBox, "#220#Report Issue",
                "Do you want to report any issue or\nfeature request for this program?\n\ngithub.com/raylibtech/rtools", "#186#Report on GitHub");

            if (result == 1)    // Report issue pressed
            {
                OpenURL("https://github.com/raylibtech/rtools/issues");
                showIssueReportWindow = false;
            }
            else if (result == 0) showIssueReportWindow = false;
        }
        //----------------------------------------------------------------------------------------

        // GUI: User Window
        //----------------------------------------------------------------------------------------
        //GuiWindowUser(&windowUserState);
        //----------------------------------------------------------------------------------------

        // GUI: Show message: Reset
        //----------------------------------------------------------------------------------------
        if (showMessageReset)
        {
            int message = GuiMessageBox((Rectangle){ GetScreenWidth()/2 - 280/2, GetScreenHeight()/2 - 120/2, 280, 120 }, "#143#Reset project", "Do you want to create a new project?", "Yes;Cancel");
            if (message >= 0)
            {
                if (message == 1)
                {
                    // TODO: Reset message box: Reset required variables

                    SetWindowTitle(TextFormat("%s v%s - unnamed.rpc*\0", toolName, toolVersion));
                }

                showMessageReset = false;
            }
        }
        //----------------------------------------------------------------------------------------

        // GUI: Load File Dialog (and loading logic) -GENERIC-
        //----------------------------------------------------------------------------------------
        if (showLoadFileDialog && !showLoadDirectoryDialog)
        {
#if defined(CUSTOM_MODAL_DIALOGS)
            int result = GuiFileDialog(DIALOG_MESSAGE, "Load file...", inFileName, "Ok", "Drag and drop your file over\nthe text field rectangle!");
#else
            int result = GuiFileDialog(DIALOG_OPEN_FILE, "Load file...", inFileName, NULL, NULL);
#endif
            if (result == 1)
            {
                if (inFileName[0] != '\0')
                {
                    // Update required property with selected path
                    memset(project.entries[projectEditProperty].text, 0, 256);
                    strcpy(project.entries[projectEditProperty].text, inFileName);
                }
            }

            if (result >= 0) showLoadFileDialog = false;
        }
        //----------------------------------------------------------------------------------------

        // GUI: Load Directory Dialog (and loading logic) -GENERIC-
        //----------------------------------------------------------------------------------------
        if (showLoadDirectoryDialog && !showLoadFileDialog)
        {
#if defined(CUSTOM_MODAL_DIALOGS)
            int result = GuiFileDialog(DIALOG_MESSAGE, "Load path...", inFilePath, "Ok", "Drag and drop your files");
#else
            int result = GuiFileDialog(DIALOG_OPEN_DIRECTORY, "Load path...", inFilePath, "", "");
#endif
            if (result == 1)
            {
                // Update required property with selected path
                memset(project.entries[projectEditProperty].text, 0, 256);
                strcpy(project.entries[projectEditProperty].text, inFilePath);
            }

            if (result >= 0) showLoadDirectoryDialog = false;
        }
        //----------------------------------------------------------------------------------------

        // GUI: Load Project Dialog (and loading logic)
        //----------------------------------------------------------------------------------------
        if (showLoadProjectDialog)
        {
#if defined(CUSTOM_MODAL_DIALOGS)
            int result = GuiFileDialog(DIALOG_MESSAGE, "Load project file...", inProjectFilePath, "Ok", "Just drag and drop your .rpc file!");
#else
            int result = GuiFileDialog(DIALOG_OPEN_FILE, "Load project file...", inProjectFilePath, "*.rpc", "Project Config Files (*.rpc)");
#endif
            if (result == 1)
            {
                rpcUnloadProjectConfig(project);
                project = rpcLoadProjectConfig(inProjectFilePath);
                /*
                rpcProjectConfig prjsrc = rpcLoadProjectConfig(inProjectFilePath);    // Load tool data from file
                rpcUnloadProjectConfig(project);
                project = rpcLoadProjectConfig("resources/project_template.rpc");

                // Copy loaded project data over project template data, so
                // all required build info is available if not provided on original file
                for (int j = 0; j < prjsrc.entryCount; j++)
                {
                    int result = rpcSetText(project, prjsrc.entries[j].key, prjsrc.entries[j].text);
                    if (result != -1) saveProjectRequired = true;
                }

                rpcUnloadProjectConfig(prjsrc);
                */

                if (saveProjectRequired) SetWindowTitle(TextFormat("%s v%s - %s*", toolName, toolVersion, GetFileName(inProjectFilePath)));
                else SetWindowTitle(TextFormat("%s v%s - %s", toolName, toolVersion, GetFileName(inProjectFilePath)));
            }

            if (result >= 0) showLoadProjectDialog = false;
        }
        //----------------------------------------------------------------------------------------

        // GUI: Save Project Dialog (and saving logic)
        //----------------------------------------------------------------------------------------
        if (showSaveAsProjectDialog)
        {
#if defined(CUSTOM_MODAL_DIALOGS)
            int result = GuiFileDialog(DIALOG_MESSAGE, "Save project file as...", outFileName, "Ok;Cancel", NULL);
#else
            int result = GuiFileDialog(DIALOG_SAVE_FILE, "Save project file as...", outFileName, "*.rpc", "Project Config File (*.rpc)");
#endif
            if (result == 1)
            {
                // Save file: outFileName
                // Check for valid extension and make sure it is
                if ((GetFileExtension(outFileName) == NULL) || !IsFileExtension(outFileName, ".rpc")) strcat(outFileName, ".rpc\0");

                FileRename(inProjectFilePath, outFileName); // Rename project file before saving

                // NOTE: Instead of recreating the full file, load full template, update/add required entries, save project
                // File to be updated with changes: inProjectFilePath
                // Update project configuration .rpc to defined values by [rpc] tool
                rini_data data = rini_load_full("resources/project_template.rpc");
                for (unsigned int i = 0; i < data.count; i++)
                {
                    for (int j = 0; j < project.entryCount; j++)
                    {
                        if (TextIsEqual(project.entries[j].key, data.entries[i].key) &&
                            !TextIsEqual(project.entries[j].text, data.entries[i].text))
                        {
                            memset(data.entries[i].text, 0, RINI_MAX_TEXT_SIZE);
                            strcpy(data.entries[i].text, project.entries[j].text);
                            // TODO: Check project.entries[j].type to set data.values[i].value and data.values[i].is_text
                        }
                    }
                }
                rini_save(data, inProjectFilePath);

                saveProjectRequired = false;
                SetWindowTitle(TextFormat("%s v%s - %s", toolName, toolVersion, GetFileName(inProjectFilePath)));

            #if defined(PLATFORM_WEB)
                // Download file from MEMFS (emscripten memory filesystem)
                // NOTE: Second argument must be a simple filename (can't use directories)
                // NOTE: Included security check to (partially) avoid malicious code on PLATFORM_WEB
                if (strchr(outFileName, '\'') == NULL) emscripten_run_script(TextFormat("saveFileFromMEMFSToDisk('%s','%s')", outFileName, GetFileName(outFileName)));
            #endif
            }

            if (result >= 0) showSaveAsProjectDialog = false;
        }
        //----------------------------------------------------------------------------------------

        // GUI: Build Project Dialog
        //----------------------------------------------------------------------------------------
        if (buildProjectDeferred)
        {
            // NOTE: Project building is deferred one frame to show a building dialog message,
            // a temporal solution before moving building into separate processes or threads

            // Build project to output directory defined
            BuildProject(project, currentPlatform, outProjectFilePath);

#if defined(PLATFORM_WEB)
            // Download file from MEMFS (emscripten memory filesystem)
            // NOTE: Second argument must be a simple filename (can't use directories)
            // NOTE: Included security check to (partially) avoid malicious code on PLATFORM_WEB
            if (strchr(outFileName, '\'') == NULL) emscripten_run_script(TextFormat("saveFileFromMEMFSToDisk('%s','%s')", outFileName, GetFileName(outFileName)));
#endif
            buildProjectDeferred = false;
        }

        if (showBuildProjectDialog)
        {
#if defined(CUSTOM_MODAL_DIALOGS)
            int result = GuiFileDialog(DIALOG_TEXTINPUT, "Select build output path...", outProjectFilePath, "Ok;Cancel", NULL);
#else
            int result = GuiFileDialog(DIALOG_OPEN_DIRECTORY, "Select build output path...", outProjectFilePath, "", "");
#endif
            if (result == 1)
            {
                buildProjectDeferred = true;
            }

            if (result >= 0) showBuildProjectDialog = false;
        }

        // NOTE: Project building deferred implies showing message window
        // a frame before actually start building project (blocking processes)
        if (buildProjectDeferred)
        {
            // Show project building screen
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)), 0.8f));

            GuiPanel((Rectangle){ 0, GetScreenHeight()/2 - 300/2, GetScreenWidth(), 300 }, NULL);
            GuiSetIconScale(3);
            GuiSetStyle(DEFAULT, TEXT_SIZE, GuiGetFont().baseSize*3);
            GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
            GuiLabel((Rectangle){ 0, GetScreenHeight()/2 - 300/2 + 20, GetScreenWidth(), 80 }, "#219#");
            GuiLabel((Rectangle){ 0, GetScreenHeight()/2 - 300/2 + 90, GetScreenWidth(), 80 }, "BUILDING PROJECT for TARGET PLATFORM");
            //GuiSetIconScale(2);
            GuiLabel((Rectangle){ 0, GetScreenHeight()/2 - 300/2 + 180, GetScreenWidth(), 80 }, "#220#WARNING: It could take some seconds...");
            GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
            GuiSetStyle(DEFAULT, TEXT_SIZE, GuiGetFont().baseSize);
            GuiSetIconScale(1);
        }
        //----------------------------------------------------------------------------------------

        // GUI: Show message: Exit Window (only on saveChangesRequired?)
        //----------------------------------------------------------------------------------------
        if (showMessageExit)
        {
            int message = GuiMessageBox((Rectangle){ GetScreenWidth()/2 - 320/2, GetScreenHeight()/2 - 50, 320, 100 }, TextFormat("#159#Closing %s", toolName), "Do you really want to exit?", "Yes;No");

            if ((message == 0) || (message == 2)) showMessageExit = false;
            else if (message == 1) closeWindow = true;
        }
        //----------------------------------------------------------------------------------------

#if defined(SPLASH_SUPPORT_MESSAGE)
        // GUI: Show support message box (splash)
        //----------------------------------------------------------------------------------------
        if (showSupportMessage)
        {
            GuiPanel((Rectangle){ -10, GetScreenHeight()/2 - 180, GetScreenWidth() + 20, 290 }, NULL);

            GuiSetStyle(DEFAULT, TEXT_SIZE, GuiGetFont().baseSize*3);
            GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
            GuiSetStyle(LABEL, TEXT_COLOR_NORMAL, GuiGetStyle(DEFAULT, TEXT_COLOR_PRESSED));
            GuiLabel((Rectangle){ -10, GetScreenHeight()/2 - 140, GetScreenWidth() + 20, 30 }, TextFormat("Enjoying %s v%s?", TOOL_NAME, TOOL_VERSION));
            GuiSetStyle(LABEL, TEXT_COLOR_NORMAL, GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL));
            GuiSetStyle(DEFAULT, TEXT_SIZE, GuiGetFont().baseSize*2);
            GuiLabel((Rectangle){ -10, GetScreenHeight()/2 - 30, GetScreenWidth() + 20, 30 }, "Please, consider buying this tool\n\nto support developer work. Thanks! :)");

            if (supportMessageRandBtn)
            {
                if (GuiButton((Rectangle){ 10, GetScreenHeight()/2 + 40, GetScreenWidth()/2 - 15, 40 }, "Next time...")) showSupportMessage = false;
                if (GuiButton((Rectangle){ 10 + GetScreenWidth()/2 - 5, GetScreenHeight()/2 + 40, GetScreenWidth()/2 - 15, 40 }, "Sure!"))
                {
                    OpenURL(TextFormat("https://raylibtech.itch.io/%s", TextToLower(TOOL_NAME)));
                    showSupportMessage = false;
                }
            }
            else
            {
                if (GuiButton((Rectangle){ 10, GetScreenHeight()/2 + 40, GetScreenWidth()/2 - 15, 40 }, "Sure!"))
                {
                    OpenURL(TextFormat("https://raylibtech.itch.io/%s", TextToLower(TOOL_NAME)));
                    showSupportMessage = false;
                }
                if (GuiButton((Rectangle){ 10 + GetScreenWidth()/2 - 5, GetScreenHeight()/2 + 40, GetScreenWidth()/2 - 15, 40 }, "Next time...")) showSupportMessage = false;
            }

            GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
            GuiSetStyle(DEFAULT, TEXT_SIZE, GuiGetFont().baseSize);
        }
        //----------------------------------------------------------------------------------------
#endif
    EndDrawing();
    //----------------------------------------------------------------------------------
}

#if defined(PLATFORM_DESKTOP) || defined(COMMAND_LINE_ONLY)
// Show command line usage info
static void ShowCommandLineInfo(void)
{
    printf("\n//////////////////////////////////////////////////////////////////////////////////\n");
    printf("//                                                                              //\n");
    printf("// %s v%s - %s                   //\n", TOOL_NAME, TOOL_VERSION, TOOL_DESCRIPTION);
    printf("// powered by raylib v%s and raygui v%s                               //\n", RAYLIB_VERSION, RAYGUI_VERSION);
    printf("// more info and bugs-report: github.com/raysan5/raylib-project-builder         //\n");
    printf("// feedback and support:      ray[at]raylib.com                                 //\n");
    printf("//                                                                              //\n");
    printf("// Copyright (c) 2025-2026 Ramon Santamaria (@raysan5)                          //\n");
    printf("//                                                                              //\n");
    printf("//////////////////////////////////////////////////////////////////////////////////\n\n");

    printf("USAGE:\n\n");
    printf("    > rpb [--help] --input <project.rpc> [--output build]\n");
    printf("          [--build <platform>] [--info]\n");


    printf("\nOPTIONS:\n\n");
    printf("    -h, --help                      : Show tool version and command line usage help\n\n");
    printf("    -i, --input <project.rpc>       : Define input project config file (.rpc)\n");
    printf("    -o, --output <path>             : Define output path for build\n");
    printf("                                      NOTE: If not defined, using defined one in .rpc\n\n");

    printf("    -b, --build <platform>          : Build project for required platform\n");
    printf("                                      NOTE: Supported build platforms depends on HOST platform\n");
#if defined(_WIN32)
    printf("                                         - HOST: Windows - Platforms: Windows, Linux, Wasm, Android\n");
#elif defined(__linux__)
    printf("                                         - HOST: Linux - Platforms: Linux, Wasm\n");
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
    printf("                                         - HOST: FreeBSD - Platforms: FreeBSD\n");
#elif defined(__APPLE__)
    printf("                                         - HOST: macOS - Platforms: macOS, Wasm\n");
#elif defined(__EMSCRIPTEN__)
    printf("                                         - HOST: Web - Platforms: -\n");
#endif

    //printf("    -p, --package <platform>        : Package project for target platform\n");
    //printf("    -d, --deploy <store>            : Deploy package to target store\n");
    printf("    -n, --info                      : Show project information\n");

    printf("\nEXAMPLES:\n\n");
    printf("    > rpb -i cool_game.rpc -o cool_game --build Windows\n");
    printf("        Build cool_game for Windows (expecting Windows Host)\n\n");
    printf("    > rpb -i game.rpc\n");
    printf("        Build using project defaults\n\n");
    printf("    > rpb -i game.rpc -b Windows\n");
    printf("        Build for Windows\n\n");
    printf("    > rpb -i game.rpc -o build -b Linux\n");
    printf("        Build for Linux to output build directory\n\n");
    printf("    > rpb -i game.rpc -b Wasm -o ./build/web\n");
    printf("        Build to a specific output directory\n\n");
}

// Process command line input
static void ProcessCommandLine(int argc, char *argv[])
{
    // CLI required variables
    bool showUsageInfo = false;         // Toggle command line usage info
    int buildPlatform = -1;             // Target build platform
    char buildPath[256] = { 0 };  // Build output path

#if defined(COMMAND_LINE_ONLY)
    if (argc == 1) showUsageInfo = true;
#endif

    // Process command line arguments
    for (int i = 1; i < argc; i++)
    {
        if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0))
        {
            showUsageInfo = true;
        }
        else if ((strcmp(argv[i], "-i") == 0) || (strcmp(argv[i], "--input") == 0))
        {
            // Check for valid argument and valid file extension
            if (((i + 1) < argc) && (argv[i + 1][0] != '-'))
            {
                if (IsFileExtension(argv[i + 1], ".rpc"))
                {
                    strcpy(inFileName, argv[i + 1]); // Read input filename
                }
                else printf("WARNING: Input file extension not recognized\n");

                i++;
            }
            else printf("WARNING: No input file provided\n");
        }
        else if ((strcmp(argv[i], "-o") == 0) || (strcmp(argv[i], "--output") == 0))
        {
            // Check for valid upcoming argumment and valid file extension: output
            if (((i + 1) < argc) && (argv[i + 1][0] != '-'))
            {
                strcpy(buildPath, argv[i + 1]);   // Read output path

                i++;
            }
            else printf("WARNING: No output file provided\n");
        }
        else if ((strcmp(argv[i], "-b") == 0) || (strcmp(argv[i], "--build") == 0))
        {
            // Check for valid argument and valid parameters
            if (((i + 1) < argc) && (argv[i + 1][0] != '-'))
            {
                if (TextIsEqual(argv[i + 1], "Windows")) buildPlatform = 0;
                else if (TextIsEqual(argv[i + 1], "Linux")) buildPlatform = 1;
                else if (TextIsEqual(argv[i + 1], "macOS")) buildPlatform = 2;
                else if (TextIsEqual(argv[i + 1], "Wasm")) buildPlatform = 3;
                else if (TextIsEqual(argv[i + 1], "Android")) buildPlatform = 4;
                else printf("WARNING: Requested build platform not supported (%s)\n", argv[i + 1]);

                // WARNING: Requested build platform should be validated against host platform
            }
            else printf("WARNING: Format parameters provided not valid\n");
        }
    }

    // Process input file if provided
    if (inFileName[0] != '\0')
    {
        printf("INFO: Working directory: %s\n", GetWorkingDirectory());
        char inputFilePath[256] = { 0 };

        // Validate input project file
        if (IsPathAbsolute(inFileName) && FileExists(inFileName)) strcpy(inputFilePath, inFileName);
        else if (FileExists(TextFormat("%s/%s", GetWorkingDirectory(), inFileName))) strcpy(inputFilePath, TextFormat("%s/%s", GetWorkingDirectory(), inFileName));
        else printf("WARNING: [%s] Input project file can not be found\n", inFileName);

        if (inputFilePath[0] != '\0')
        {
            // Load input project file
            rpcProjectConfig config = rpcLoadProjectConfig(inputFilePath);

            printf("INFO: [%s] Loaded input project: %s\n", GetFileName(inputFilePath), rpcGetText(config, "PROJECT_INTERNAL_NAME"));

            printf("  > Project repo name:       %s\n", rpcGetText(config, "PROJECT_REPO_NAME"));
            printf("  > Project internal name:   %s\n", rpcGetText(config, "PROJECT_INTERNAL_NAME"));
            printf("  > Project commercial name: %s\n", rpcGetText(config, "PROJECT_COMMERCIAL_NAME"));
            printf("  > Project short name:      %s\n", rpcGetText(config, "PROJECT_SHORT_NAME"));
            printf("  > Project version:         %s\n", rpcGetText(config, "PROJECT_VERSION"));
            printf("  > Project description:     %s\n", rpcGetText(config, "PROJECT_DESCRIPTION"));
            printf("  > Project publisher name:  %s\n", rpcGetText(config, "PROJECT_PUBLISHER_NAME"));
            printf("  > Project developer name:  %s\n", rpcGetText(config, "PROJECT_DEVELOPER_NAME"));
            printf("  > Project developer url:   %s\n\n", rpcGetText(config, "PROJECT_DEVELOPER_URL"));

            // NOTE: Build output path validation done inside BuildProject()

            // Validate platform selected for current host
#if defined(_WIN32)
            if ((buildPlatform != 0) && (buildPlatform != 1) && (buildPlatform != 3))
            {
                buildPlatform = 0; // Revert to Windows
                printf("WARNING: Requested platform not supported on current host platforms (HOST: Windows)\n");
                printf("WARNING: Revert to default platform: %s\n", platformNames[buildPlatform]);
            }
#elif defined(__linux__)
            if ((buildPlatform != 1) && (buildPlatform != 3))
            {
                buildPlatform = 1; // Revert to Linux
                printf("WARNING: Requested platform not supported on current host platforms (HOST: Linux)\n");
                printf("WARNING: Revert to default platform: %s\n", platformNames[buildPlatform]);
            }
#elif defined(__APPLE__)
            if ((buildPlatform != 2) && (buildPlatform != 3))
            {
                buildPlatform = 2; // Revert to macOS
                printf("WARNING: Requested platform not supported on current host platforms (HOST: macOS)\n");
                printf("WARNING: Revert to default platform: %s\n", platformNames[buildPlatform]);
            }
#endif

            printf("INFO: Build output platform:   %s\n", platformNames[buildPlatform]);

            // Build provided project to output build directory for selected platform
            int result = BuildProject(config, buildPlatform, buildPath);

            if (result == 0) printf("Build process finished successfully!\n");
        }
    }

    if (showUsageInfo) ShowCommandLineInfo();
}
#endif // PLATFORM_DESKTOP || COMMAND_LINE_ONLY

//--------------------------------------------------------------------------------------------
// Load/Save/Export functions
//--------------------------------------------------------------------------------------------
// NOTE: rpconfig.h provides required functions, shared by [rpc] and [rpb] tools

// Build project for selected platform
// WARNING: Build target platform support depends on host platform
static int BuildProject(rpcProjectConfig project, int platform, const char *buildPath)
{
    int result = 0;

    // Project building requires multiple steps
    // NOTE: Some of those steps are optional or not implemented
    // 1. Setup environment
    //    - Download and install required SDKs
    //    - Configure required directories
    //    - Set environment variables
    // 2. Build raylib library
    //    - Set library config options
    //    - Set default output directories
    //    - Build raylib with selected configuration
    // 3. Build project
    //    - Set output directory with required structure
    //    - Build project source files (defined by .rpc)
    //    - Copy binary to build directory
    // 4. Process assets
    //    - Process and package assets
    //    - Copy assets to output build assets path
    // 5. Package project
    //    - Sign executable and/or package
    //    - Compress output build (.zip/.7z)
    //    - Create installer for target platform
    // 6. Run project (depends on host platform)

    LOG("INFO: Starting project building for requested platform...\n");

    // Check build output path
    // WARNING: Using global inProjectFilePath to get output path
    char buildOutputPath[256] = { 0 }; // Project build path
    if (buildPath[0] == '\0')
    {
        // Read default built path from .rpc file (relative to .rpc location)
        //strcpy(buildOutputPath, TextFormat("%s/%s", GetDirectoryPath(inProjectFilePath), rpcGetText(project, "BUILD_OUTPUT_PATH")));
#if defined(_WIN32)
        TextCopy(buildOutputPath, TextFormat("%s\\%s", GetDirectoryPath(inProjectFilePath), rpcGetText(project, "BUILD_OUTPUT_PATH")));
#else
        TextCopy(buildOutputPath, TextFormat("%s/%s", GetDirectoryPath(inProjectFilePath), rpcGetText(project, "BUILD_OUTPUT_PATH")));
#endif
    }
    else
    {
        // Validate provided build path
        if (IsPathAbsolute(buildPath)) strcpy(buildOutputPath, buildPath);
        else
        {
#if defined(_WIN32)
            TextCopy(buildOutputPath, TextFormat("%s\\%s", GetWorkingDirectory(), buildPath));
#else
            TextCopy(buildOutputPath, TextFormat("%s/%s", GetWorkingDirectory(), buildPath));
#endif
        }
    }

    if (!DirectoryExists(buildOutputPath))
    {
        // Create build directory if required
        MakeDirectory(buildOutputPath);
        LOG("INFO: [%s] Created build output path", buildOutputPath);
    }

#if defined(__EMSCRIPTEN__)
    // TODO: Connect to server to build project remotely
#endif
//#else

    switch (platform) // Target platform
    {
        case RPC_PLATFORM_WINDOWS:
        {
            LOG("INFO: Building project for platform: %s\n", platformNames[platform]);

#if defined(_WIN32)
            if (TextIsEqual(rpcGetText(project, "PLATFORM_WINDOWS_DEFAULT_TOOLCHAIN"), "MSBUILD"))
            {
                // 1. Setup environment (...)
                // 2. Build raylib library
                // 3. Build project (MSBuild)
                // Check if VS2022 project is available
                if (DirectoryExists(TextFormat("%s/projects/VS2022", GetDirectoryPath(inProjectFilePath))))
                {
                    ChangeDirectory(TextFormat("%s/projects/VS2022", GetDirectoryPath(inProjectFilePath)));
                    system(TextFormat("%s\\msbuild.exe %s.sln /target:%s /property:Configuration=Release /property:Platform=x64 /property:RaylibSrcPath = \"%s\"",
                        rpcGetText(project, "PLATFORM_WINDOWS_MSBUILD_PATH"), rpcGetText(project, "PROJECT_INTERNAL_NAME"), rpcGetText(project, "PROJECT_INTERNAL_NAME"), rpcGetText(project, "RAYLIB_SRC_PATH")));
                }
                else LOG("WARNING: VS2022 project not found\n");

                // TODO: Copy VS2022 build output to build directory
            }
            else
            {
                // 1. Setup environment
                PUTENV(TextFormat("PROJECT_BUILD_PATH=%s", buildOutputPath));
                PUTENV(TextFormat("RAYLIB_DIR=%s", rpcGetText(project, "RAYLIB_SRC_PATH")));
                PUTENV(TextFormat("PATH=%PATH%;%s", rpcGetText(project, "PLATFORM_WINDOWS_W64DEVKIT_PATH")));

                // 2. Build raylib library
                if (rpcGetValue(project, "RAYLIB_FLAG_BUILDING_REQUIRED") == 1)
                {
                    ChangeDirectory(TextFormat("%s", rpcGetText(project, "RAYLIB_SRC_PATH")));
                    system("make PLATFORM=PLATFORM_DESKTOP -B");
                }

                // 3. Build project (Makefile)
                ChangeDirectory(TextFormat("%s", buildOutputPath));
                system(TextFormat("make -C %s PLATFORM=PLATFORM_DESKTOP -B", TextFormat("%s/src", GetDirectoryPath(inProjectFilePath))));
            }

            // 4. Process assets
            // NOTE: Copy to destination assets output, directory created automatically
            DirectoryCopy(TextFormat("%s/%s", GetDirectoryPath(inProjectFilePath), rpcGetText(project, "PROJECT_ASSETS_PATH")),
                TextFormat("%s/%s", buildOutputPath, rpcGetText(project, "PROJECT_ASSETS_OUTPUT_PATH")));

            // 5. Package project (...)

            // 6. Run project
            if (runProjectRequired)
            {
                if (FileExists(TextFormat("%s/%s.exe", buildOutputPath, rpcGetText(project, "PROJECT_INTERNAL_NAME"))))
                    system(TextFormat("%s/%s.exe", buildOutputPath, rpcGetText(project, "PROJECT_INTERNAL_NAME")));
                else LOG("WARNING: Project executable file not found\n");
            }
#else
            LOG("WARNING: Target platform not supported on this host platform\n");
#endif
        } break;
        case RPC_PLATFORM_LINUX:
        {
#if defined(_WIN32)
            // Host platform: Windows --> Requires WSL2 installed with required libraries

            // 1. Setup environment
            PUTENV("PATH=%PATH%;C:\\Windows\\System32"); // Make sure WSL is available in the path

            // 2. Build raylib library
            if (rpcGetValue(project, "RAYLIB_FLAG_BUILDING_REQUIRED") == 1)
            {
                // Rebuild raylib library for current platform
                ChangeDirectory(TextFormat("%s", rpcGetText(project, "RAYLIB_SRC_PATH")));
                system("wsl make PLATFORM=PLATFORM_DESKTOP -B");
            }

            //system("wsl wslpath -a .");
            //system("wsl bash -lc \"cd \\\"$(wslpath -a .)\\\" && make \"");
            //system("wsl bash -lc \"cd /mnt/c/path/to/project && make\"");

            // 3. Build project (Makefile)
            ChangeDirectory(TextFormat("%s\\src", GetDirectoryPath(inProjectFilePath)));
            system(TextFormat("wsl make PLATFORM=PLATFORM_DESKTOP PROJECT_BUILD_PATH=%s RAYLIB_SRC_PATH=$(wslpath -a %s) -B",
                PathToWSL(buildOutputPath), rpcGetText(project, "RAYLIB_SRC_PATH")));

            // 4. Process assets
            // Copy to destination assets output, directory created automatically
            DirectoryCopy(TextFormat("%s/%s", PathToWSL(GetDirectoryPath(inProjectFilePath)), rpcGetText(project, "PROJECT_ASSETS_PATH")),
                TextFormat("%s/%s", PathToWSL(buildOutputPath), rpcGetText(project, "PROJECT_ASSETS_OUTPUT_PATH")));
            //system(TextFormat("wsl cp %s %s", rpcGetText(project, "PROJECT_INTERNAL_NAME"), PathToWSL(buildOutputPath)));

            // 5. Package project (...)

            // 6. Run project
            if (runProjectRequired)
            {
                system(TextFormat("wsl %s/%s", PathToWSL(buildOutputPath), rpcGetText(project, "PROJECT_INTERNAL_NAME")));
            }
#elif defined(__linux__)
            // Host platform: Linux --> Requires raylib library installed

            // 1. Setup environment
            PUTENV(TextFormat("PROJECT_BUILD_PATH=%s", buildOutputPath));

            // 2. Build raylib library
            if (rpcGetValue(project, "RAYLIB_FLAG_BUILDING_REQUIRED") == 1)
            {
                ChangeDirectory(TextFormat("%s", rpcGetText(project, "RAYLIB_SRC_PATH")));
                system("make PLATFORM=PLATFORM_DESKTOP -B");
            }

            // 3. Build project (Makefile)
            ChangeDirectory(TextFormat("%s", buildOutputPath));
            system(TextFormat("make -C %s PLATFORM=PLATFORM_DESKTOP -B", TextFormat("%s/src", GetDirectoryPath(inProjectFilePath))));

            // 4. Process assets
            // NOTE: Copy to destination assets output, directory created automatically
            DirectoryCopy(TextFormat("%s/%s", GetDirectoryPath(inProjectFilePath), rpcGetText(project, "PROJECT_ASSETS_PATH")),
                TextFormat("%s/%s", buildOutputPath, rpcGetText(project, "PROJECT_ASSETS_OUTPUT_PATH")));

            // 5. Package project (...)

            // 6. Run project
            if (runProjectRequired)
            {
                if (FileExists(TextFormat("%s/%s", buildOutputPath, rpcGetText(project, "PROJECT_INTERNAL_NAME"))))
                    system(TextFormat("%s/%s", buildOutputPath, rpcGetText(project, "PROJECT_INTERNAL_NAME")));
                else LOG("WARNING: Project executable file not found\n");
            }
#else
            LOG("WARNING: Target platform not supported on this host platform\n");
#endif
        } break;
        case RPC_PLATFORM_MACOS:
        {
#if defined(__APPLE__)
            // Host platform: macOS --> Requires raylib library installed

            LOG("INFO: Building project for platform: %s\n", platformNames[platform]); // TODO: Log architecture: macOS (arm64)

            // 1. Setup environment
            // NOTE: Create required dirctory structure for application
            ChangeDirectory(TextFormat("%s", buildOutputPath));
            MakeDirectory(TextFormat("%s.app/Contents/MacOS", rpcGetText(project, "PROJECT_INTERNAL_NAME")));
            MakeDirectory(TextFormat("%s.app/Contents/Resources", rpcGetText(project, "PROJECT_INTERNAL_NAME")));

            // 2. Build raylib library
            if (rpcGetValue(project, "RAYLIB_FLAG_BUILDING_REQUIRED") == 1)
            {
                // Rebuild raylib library for current platform
                ChangeDirectory(TextFormat("%s", rpcGetText(project, "RAYLIB_SRC_PATH")));
                system("wsl make PLATFORM=PLATFORM_DESKTOP -B");
            }

            // 3. Build project (Makefile)
            ChangeDirectory(TextFormat("%s/src", GetDirectoryPath(inProjectFilePath)));
            system(TextFormat("make PLATFORM=PLATFORM_DESKTOP PROJECT_BUILD_PATH=%s%s.app/Contents/MacOS RAYLIB_SRC_PATH=%s -B",
                buildOutputPath, rpcGetText(project, "PROJECT_INTERNAL_NAME"), "/Users/raysan5/GitHub/raylib/src"));

            // 4. Process assets
            // NOTE: Copy to destination assets output, directory created automatically
            DirectoryCopy(TextFormat("%s/%s", GetDirectoryPath(inProjectFilePath), rpcGetText(project, "PROJECT_ASSETS_PATH")),
                TextFormat("%s%s.app/Contents/Resources", buildOutputPath, rpcGetText(project, "PROJECT_INTERNAL_NAME")));

            // 5. Package project (...)
            FileCopy(TextFormat("%s/src/Info.plist", GetDirectoryPath(inProjectFilePath)),
                TextFormat("%s%s.app/Contents/Info.plist", buildOutputPath, rpcGetText(project, "PROJECT_INTERNAL_NAME")));

            // 6. Run project
            if (runProjectRequired)
            {
                if (FileExists(TextFormat("%s%s.app/Contents/MacOS/%s", buildOutputPath, rpcGetText(project, "PROJECT_INTERNAL_NAME"), rpcGetText(project, "PROJECT_INTERNAL_NAME"))))
                    system(TextFormat("%s%s.app/Contents/MacOS/%s", buildOutputPath, rpcGetText(project, "PROJECT_INTERNAL_NAME"), rpcGetText(project, "PROJECT_INTERNAL_NAME")));
                else LOG("WARNING: Project executable file not found\n");
            }
#else
            LOG("WARNING: Target platform not supported on this host platform\n");
#endif
        } break;
        case RPC_PLATFORM_WASM:
        {
            // Host platform: Windows/Linux/macOS --> Considering all hosts
            // WARNING: Emscripten must be installed!

            LOG("INFO: Building project for platform: %s\n", platformNames[platform]);

            // 1. Setup environment
            PUTENV(TextFormat("PROJECT_BUILD_PATH=%s", buildOutputPath));
#if defined(_WIN32)
            PUTENV(TextFormat("RAYLIB_DIR=%s", rpcGetText(project, "RAYLIB_SRC_PATH")));
            PUTENV(TextFormat("PATH=%PATH%;%s", rpcGetText(project, "PLATFORM_WINDOWS_W64DEVKIT_PATH")));
            PUTENV(TextFormat("EMSDK_PATH=%s", rpcGetText(project, "PLATFORM_WEB_EMSDK_PATH")));
#endif
            // 2. Build raylib library
            if (rpcGetValue(project, "RAYLIB_FLAG_BUILDING_REQUIRED") == 1)
            {
                // Rebuild raylib library for current platform
                ChangeDirectory(TextFormat("%s", rpcGetText(project, "RAYLIB_SRC_PATH")));

                // TODO: Check if we have "raylib.h rcore.c rshapes.c...", RAYLIB_SRC_PATH could
                // not be properly configured and be left in another folder with a Makefile (same rpb directory)
                //system("make PLATFORM=PLATFORM_WEB -B");
            }

            // 3. Build project (Makefile)
            // NOTE: Required resources should be already in Makefile
            // WARNING: raylib.h can not be found by emcc /usr/local/include must be added
            // WARNING: Path to libraylib.web.a must be provided to be found
            ChangeDirectory(TextFormat("%s", buildOutputPath));
            system(TextFormat("make -C %s/src PLATFORM=PLATFORM_WEB RAYLIB_LIB_PATH=/home/ray/GitHub/raylib/src/ BUILD_WEB_SHELL=%s/%s BUILD_WEB_HEAP_SIZE=%iMB -B", GetDirectoryPath(inProjectFilePath),
                GetDirectoryPath(inProjectFilePath), rpcGetText(project, "PLATFORM_WEB_SHELL_FILE"),
                rpcGetValue(project, "PLATFORM_WEB_HEAP_MEMORY_SIZE")));

            // 4. Process assets (...)
            // NOTE: Resources/assets already processed by emscripten building process

            // 5. Package project (...)

            // 6. Run project
            if (runProjectRequired)
            {
                if (FileExists(TextFormat("%s/%s.html", buildOutputPath, rpcGetText(project, "PROJECT_INTERNAL_NAME"))) &&
                    FileExists(TextFormat("%s/%s.wasm", buildOutputPath, rpcGetText(project, "PROJECT_INTERNAL_NAME"))) &&
                    FileExists(TextFormat("%s/%s.js", buildOutputPath, rpcGetText(project, "PROJECT_INTERNAL_NAME"))))
                {
                    // WARNING: Example download is asynchronous so reading fails on next step
                    // when looking for a file that could not have been downloaded yet
                    ChangeDirectory(TextFormat("%s", buildOutputPath));
                #if defined(_WIN32)
                    system("start python -m http.server 8080"); // Init localhost just once
                    system(TextFormat("start explorer \"http://localhost:8080/%s.html\"", rpcGetText(project, "PROJECT_INTERNAL_NAME")));
                #elif defined(__linux__)
                    system("pwd");
                    // WARNING: python3 process is left running in background
                    system("python3 -m http.server 8080 &"); // Init localhost just once
                    system(TextFormat("xdg-open http://localhost:8080/%s.html", rpcGetText(project, "PROJECT_INTERNAL_NAME")));
                #elif defined(__APPLE__)
                    system("python3 -m http.server 8080 &"); // Init localhost just once
                    system(TextFormat("open http://localhost:8080/%s.html", rpcGetText(project, "PROJECT_INTERNAL_NAME")));
                #endif
                }
            }

        } break;
        case RPC_PLATFORM_ANDROID:
        {
        } break;
        case RPC_PLATFORM_ESP32:
        {
            #if defined(_WIN32)

            #elif defined(__linux__)

            #endif
        } break;
        case RPC_PLATFORM_DREAMCAST:
        {
            #if defined(_WIN32)

            #elif defined(__linux__)

            #endif
        } break;
        case RPC_PLATFORM_SWITCH:
        {
            #if defined(_WIN32)

            #elif defined(__linux__)

            #endif
        } break;
        default:
        {
            LOG("WARNING: Target build platform not supported on this host platform\n");
        }
    }

    runProjectRequired = false;
    return result;
}

//--------------------------------------------------------------------------------------------
// Auxiliar functions (utilities)
//--------------------------------------------------------------------------------------------
// Copy full directory with all content (including subdirs)
static int DirectoryCopy(const char *srcPath, const char *dstPath)
{
    int result = 0;

    if (DirectoryExists(srcPath))
    {
        if (!DirectoryExists(dstPath)) MakeDirectory(dstPath);

        FilePathList files = LoadDirectoryFiles(srcPath);
        int srcPathLen = strlen(srcPath);

        for (unsigned int i = 0; i < files.count; i++)
        {
            // Checking if some subdirectory is required
            if (!DirectoryExists(GetDirectoryPath(files.paths[i]))) MakeDirectory(GetDirectoryPath(files.paths[i]));

            // NOTE: Skipping the src directory path to copy to destination
            FileCopy(files.paths[i], TextFormat("%s/%s", dstPath, files.paths[i] + srcPathLen));
        }

        UnloadDirectoryFiles(files);
    }
    else LOG("WARNING: Source directory does not exist");

    return result;
}

// Check if provided path is an absolute path
static bool IsPathAbsolute(const char *path)
{
    if ((path == NULL) || (path[0] == '\0')) return false;

#ifdef _WIN32
    // UNC path (\\server\share)
    if (path[0] == '\\' && path[1] == '\\') return true;
    // Drive letter (e.g. C:\ or D:/)
    if (isalpha((unsigned char)path[0]) &&
        (path[1] == ':') &&
        ((path[2] == '\\') || (path[2] == '/'))) return true;
    return false;
#else
    // POSIX: must start with /
    if (path[0] == '/') return true;
    else return false;
#endif
}

#if defined(_WIN32)
// Convert windows path to WSL
static const char *PathToWSL(const char *path)
{
    static char result[256] = { 0 };
    memset(result, 0, 256);
    int pathLen = strlen(path);

    char *resultPtr = result;
    strcpy(resultPtr, "/mnt/");
    resultPtr += 5;

    // Process drive letter
    if (isalpha(path[0]) && (path[1] == ':'))
    {
        resultPtr[0] = tolower(path[0]);
        resultPtr++;
        path += 2; // Skip "C:"
    }

    // Update directory slash separators
    for (int i = 0; i < pathLen; i++)
    {
        char c = path[i];
        if (c == '\\') c = '/';
        resultPtr[i] = c;
    }

    return result;
}
#endif

// Load/Save application configuration functions
//------------------------------------------------------------------------------------
// Load aplication init configuration
static void LoadApplicationConfig(void)
{
    int windowMaximized = 0;
#if defined(PLATFORM_WEB)
    bool loadConfigData = true;
#else
    bool loadConfigData = FileExists(TextFormat("%s/%s", GetApplicationDirectory(), TOOL_CONFIG_FILENAME));
#endif

    if (loadConfigData)
    {
        rini_data config = { 0 };
#if defined(PLATFORM_WEB)
        int outputSize = 0;
        char *configDataBase64 = LoadWebLocalStorage(TOOL_CONFIG_FILENAME);
        char *configText = DecodeDataBase64(configDataBase64, &outputSize);
        config = rini_load_from_memory(configText);
        MemFree(configText);
#else
        config = rini_load(TextFormat("%s/%s", GetApplicationDirectory(), TOOL_CONFIG_FILENAME));
#endif
        // Load required config variables
        // NOTE: Keys not found default to 0 value, unless fallback is requested
        windowAboutState.showSplash = rini_get_value(config, "SHOW_WINDOW_WELCOME");
        windowMaximized = rini_get_value(config, "INIT_WINDOW_MAXIMIZED");
        mainToolbarState.visualStyleActive = rini_get_value(config, "GUI_VISUAL_STYLE");

        rini_unload(&config);

        // NOTE: Config is automatically saved when application is closed
    }

    // Setup application using config values (or default)
    if (windowAboutState.showSplash) { windowAboutState.welcomeMode = true; windowAboutState.windowActive = true; }
    else { windowAboutState.welcomeMode = false; windowAboutState.windowActive = false; }
    //if (mainToolbarState.showTooltips) GuiEnableTooltip();
    //else GuiDisableTooltip();

    if (windowMaximized == 1) MaximizeWindow();
}

// Save application configuration
static void SaveApplicationConfig(void)
{
    rini_data config = rini_load(NULL);   // Create empty config with RINI_MAX_VALUE_CAPACITY entries

    // Define header comment lines
    rini_set_comment_line(&config, " ");   // Empty comment line, but including comment prefix delimiter
    rini_set_comment_line(&config, TextFormat(" %s initialization configuration options", TOOL_NAME));
    rini_set_comment_line(&config, " ");
    rini_set_comment_line(&config, " NOTE: This file is loaded at application startup,");
    rini_set_comment_line(&config, " if file is not found, default values are applied");
    rini_set_comment_line(&config, " ");

#if defined(PLATFORM_DESKTOP)
    int windowMaximized = (int)IsWindowMaximized();
#endif
    rini_set_value(&config, "SHOW_WINDOW_WELCOME", (int)windowAboutState.showSplash, "Show welcome window at initialization");
#if defined(PLATFORM_DESKTOP)
    rini_set_value(&config, "INIT_WINDOW_MAXIMIZED", (int)windowMaximized, "Initialize window maximized");
#endif
    rini_set_value(&config, "GUI_VISUAL_STYLE", (int)mainToolbarState.visualStyleActive, "UI visual style selected");

#if defined(PLATFORM_WEB)
    int outputSize = 0;
    char *configText = rini_save_to_memory(config);
    char *configBase64 = EncodeDataBase64(configText, strlen(configText), &outputSize);
    SaveWebLocalStorage(TOOL_CONFIG_FILENAME, configBase64);
    MemFree(configBase64);
#else
    rini_save(config, TextFormat("%s/%s", GetApplicationDirectory(), TOOL_CONFIG_FILENAME));
#endif
    rini_unload(&config);
}

#if defined(PLATFORM_WEB)
// Save data to web LocalStorage (persistent between sessions)
// WARNING: Despite line-breaks are supposedly supported in value,
// emscripten interprets them as separate execution lines and fails -> Use Base64 string
static void SaveWebLocalStorage(const char *key, const char *value)
{
    char script[2048] = { 0 };
    snprintf(script, 2048, "localStorage.setItem(\"%s\", \"%s\")", key, value);

    // Run script to save config to local storage
    // WARNING: TextFormat() can not be used because defaults to MAX 1024 chars
    emscripten_run_script(script);
}

// Load data from web LocalStorage (persistent between sessions)
static char *LoadWebLocalStorage(const char *key)
{
    // NOTE: Make sure result has enough space for the retrieved data!
    static char result[2048] = { 0 };
    memset(result, 0, 2048);

    // Run the script and get the result as a string
    const char *loadedData = emscripten_run_script_string(TextFormat("localStorage.getItem('%s')", key));
    strncpy(result, loadedData, sizeof(result) - 1);
    result[sizeof(result) - 1] = '\0';

    return result;
}
#endif
//------------------------------------------------------------------------------------
