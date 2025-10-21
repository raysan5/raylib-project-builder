/*******************************************************************************************
*
*   raylib project builder v1.0 - A simple and easy-to-use raylib project builder
*
*   FEATURES:
*       - Feature 01
*       - Feature 02
*       - Feature 03
*
*   POSSIBLE IMPROVEMENTS:
*       - Improvement 01
*       - Improvement 02
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
*       1.0  (xx-Nov-2025)  First release
*
*   DEPENDENCIES:
*       raylib 5.6-dev          - Windowing/input management and drawing
*       raygui 4.5-dev          - Immediate-mode GUI controls with custom styling and icons
*       rpng 1.5                - PNG chunks management
*       rini 3.0                - Configuration file load/save
*       rkls 1.0                - Key licensing system
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
*   Copyright (c) 2025 Ramon Santamaria (@raysan5)
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

#define TOOL_NAME                   "raylib project builder"
#define TOOL_SHORT_NAME             "rpb"
#define TOOL_VERSION                "1.0"
#define TOOL_DESCRIPTION            "A simple and easy-to-use raylib project builder"
#define TOOL_DESCRIPTION_BREAK      "A simple and easy-to-use\nraylib project builder"
#define TOOL_RELEASE_DATE           "Nov.2025"
#define TOOL_LOGO_COLOR             0x7c7c80ff
#define TOOL_CONFIG_FILENAME        "rpb.ini"

#if defined(PLATFORM_WEB)
    //#define SPLASH_SUPPORT_MESSAGE
#endif

#include "raylib.h"

#if defined(PLATFORM_WEB)
    #define CUSTOM_MODAL_DIALOGS            // Force custom modal dialogs usage
    #include <emscripten/emscripten.h>      // Emscripten library - LLVM to JavaScript compiler
    #include <emscripten/html5.h>           // Emscripten HTML5 browser functionality (emscripten_set_beforeunload_callback)
#endif

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"                         // Required for: IMGUI controls

#undef RAYGUI_IMPLEMENTATION                // Avoid including raygui implementation again

#define GUI_MAIN_TOOLBAR_IMPLEMENTATION
#include "gui_main_toolbar.h"               // GUI: Main toolbar

#define GUI_WINDOW_HELP_IMPLEMENTATION
#include "gui_window_help.h"                // GUI: Help Window

#define GUI_WINDOW_ABOUT_IMPLEMENTATION
#include "gui_window_about_welcome.h"       // GUI: About/Welcome Window

//#define GUI_WINDOW_USER_IMPLEMENTATION
//#include "gui_window_user.h"                // GUI: User Window

#define GUI_FILE_DIALOGS_IMPLEMENTATION
#include "gui_file_dialogs.h"               // GUI: File Dialogs

// raygui embedded styles
// NOTE: Included in the same order as selector
//@RTN_INCLUDE_STYLES
#define MAX_GUI_STYLES_AVAILABLE   10       // NOTE: Not using light style
#include "styles/style_cyber.h"             // raygui style: cyber
#include "styles/style_lavanda.h"           // raygui style: lavanda
#include "styles/style_terminal.h"          // raygui style: terminal
#include "styles/style_amber.h"             // raygui style: amber
#include "styles/style_genesis.h"           // raygui style: genesis

#define RINI_MAX_TEXT_SIZE        256       // WARNING: Text can contain paths and files-list, it should be probably bigger
#define RINI_VALUE_SPACING         38
#define RINI_IMPLEMENTATION
#include "external/rini.h"                  // Config file values reader/writer

#define RPNG_IMPLEMENTATION
//#define RPNG_DEFLATE_IMPLEMENTATION       // Required when compiling raylib as DLL
#include "external/rpng.h"                  // PNG chunks management

// Standard C libraries
#include <stdlib.h>                         // Required for: NULL, malloc(), free()
#include <stdio.h>                          // Required for: fopen(), fclose(), fread()...
#include <string.h>                         // Required for: strcmp(), strlen()
#include <math.h>                           // Required for: powf()
#include <time.h>                           // Required for: time(), localtime()

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

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

typedef enum {
    RPB_CAT_PROJECT = 0,
    RPB_CAT_BUILD,
    RPB_CAT_PLATFORM,
    RPB_CAT_DEPLOY,
    RPB_CAT_IMAGERY,
    RPB_CAT_RAYLIB
} rpbEntryCategory;

typedef enum {
    RPB_TYPE_BOOL = 0,
    RPB_TYPE_VALUE,
    RPB_TYPE_TEXT,
    RPB_TYPE_TEXT_FILE,
    RPB_TYPE_TEXT_PATH,
} rpbEntryType;

typedef enum {
    RPB_PLATFORM_WINDOWS = 0,
    RPB_PLATFORM_LINUX,
    RPB_PLATFORM_MACOS,
    RPB_PLATFORM_HTML5,
    RPB_PLATFORM_ANDROID,
    RPB_PLATFORM_DRM,
    RPB_PLATFORM_SWITCH,
    RPB_PLATFORM_DREAMCAST,
    RPB_PLATFORM_FREEBSD,
    RPB_PLATFORM_ANY
} rpbPlatform;

// Config entry data type
// NOTE: Useful to automatice UI generation,
// every data entry is read from rpc config file
typedef struct rpbEntry {
    char key[64];       // Entry key (as read from .rpc)
    char name[64];      // Entry name label for display, computed from key
    int category;       // Entry category: PROJECT, BUILDING, PLATFORM, DEPLOY, IMAGERY, raylib
    int platform;       // Entry platform-specific
    int type;           // Entry type of data: VALUE (int), BOOL (int), TEXT (string), FILE (string-file), PATH (string-path)
    int value;          // Entry value (type: VALUE, BOOL)
    // TODO: WARNING: rini expects a maximum len for text of 256 chars, multiple files can be longer that that
    char text[256];     // Entry text data (type: TEXT, FILE, PATH) - WARNING: It can include multiple paths
    char desc[128];     // Entry data description, useful for tooltips
    bool multi;         // Entry data contains multiple values, separated by ';'

    // Transient data
    bool editMode;      // Edit mode required for UI text control
} rpbEntry;

// Config options data
typedef struct rpbConfigData {
    int entryCount;     // Number of entries
    rpbEntry *entries;  // Entries
} rpbConfigData;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static const char *toolName = TOOL_NAME;
static const char *toolVersion = TOOL_VERSION;
static const char *toolDescription = TOOL_DESCRIPTION;

static const int screenWidth = 960;        // Default screen width (at initialization)
static const int screenHeight = 660;        // Default screen height (at initialization)

// NOTE: Max length depends on OS, in Windows MAX_PATH = 256
static char inFileName[512] = { 0 };        // Input file name (required in case of drag & drop over executable)
static char outFileName[512] = { 0 };       // Output file name (required for file save/export)

static int framesCounter = 0;               // General pourpose frames counter (not used)
static Vector2 mousePoint = { 0 };          // Mouse position
static Texture2D texNoise = { 0 };          // Background noise texture
static bool lockBackground = false;         // Toggle lock background (controls locked)
static bool saveChangesRequired = false;    // Flag to notice save changes are required

static RenderTexture2D target = { 0 };      // Render texture to render the tool (if required)

#define MAX_PLATFORMS   9
static Texture2D texPlatforms = { 0 };      // Platform logos texture for toggles
static bool platformEnabled[MAX_PLATFORMS] = { 0 };
static int currentPlatform = 0;

static int currentTab = 0;
const char *tabText[6] = {"#176#PROJECT SETTINGS", "#140#BUILD SETTINGS", "#181#PLATFORM SETTINGS", "#178#DEPLOY OPTIONS", "#12#IMAGERY EDITION", "#133#raylib CONFIG" };

static Vector2 panelScroll = { 0 };
static Rectangle panelView = { 0 };

// Basic program variables
//----------------------------------------------------------------------------------
static rpbConfigData project = { 0 };       // rpb project config data

static bool showMessageReset = false;       // Show message: reset
static bool showMessageExit = false;        // Show message: exit (quit)

static double baseTime = 0;                 // Base time in seconds to start counting
static double currentTime = 0;              // Current time counter in seconds

// Tool state modes
static int toolState = 0;                   // Tool current state (if multiple screens)
//-----------------------------------------------------------------------------------

// Support Message Box
//-----------------------------------------------------------------------------------
#if defined(SPLASH_SUPPORT_MESSAGE)
static bool showSupportMessage = true;      // Support message box splash message at startup
#else
static bool showSupportMessage = false;
#endif
static int supportMessageRandBtn = 0;       // Support message buttons random position
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

// GUI: Export Window
//-----------------------------------------------------------------------------------
static bool windowExportActive = false;
static int exportFormatActive = 0;         // ComboBox file type selection
//-----------------------------------------------------------------------------------

// GUI: Exit Window
//-----------------------------------------------------------------------------------
static bool closeWindow = false;
static bool windowExitActive = false;
//-----------------------------------------------------------------------------------

// GUI: Custom file dialogs
//-----------------------------------------------------------------------------------
static bool showLoadFileDialog = false;
static bool showSaveFileDialog = false;
static bool showExportFileDialog = false;
//-----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
#if defined(PLATFORM_DESKTOP) || defined(COMMAND_LINE_ONLY)
static void ShowCommandLineInfo(void);                      // Show command line usage info
static void ProcessCommandLine(int argc, char *argv[]);     // Process command line input
#endif

static void UpdateDrawFrame(void);                          // Update and draw one frame

// Load/Save/Export data functions
static rpbConfigData LoadProjectData(const char *fileName); // Load project config data data from .rpc file
static void SaveProjectData(rpbConfigData data, const char *fileName); // Save project config data to .rpc file

// Auxiliar functions
//...

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
static const char *WebBeforeUnload(int eventType, const void *reserved, void *userData) { SaveApplicationConfig(); return NULL; }
#endif
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
#if !defined(_DEBUG)
    SetTraceLogLevel(LOG_NONE);         // Disable raylib trace log messsages
#endif
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
            if (IsFileExtension(argv[1], ".xx1") ||
                IsFileExtension(argv[1], ".xx2"))
            {
                strcpy(inFileName, argv[1]);        // Read input filename to open with gui interface
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
    FreeConsole();
#endif

    // GUI usage mode - Initialization
    //--------------------------------------------------------------------------------------
    //SetConfigFlags(FLAG_WINDOW_RESIZABLE);      // Window configuration flags
    InitWindow(screenWidth, screenHeight, TextFormat("%s v%s | %s", toolName, toolVersion, toolDescription));
    //SetWindowMinSize(1280, 720);
    SetExitKey(0);

    // Create a RenderTexture2D to be used for render to texture
    target = LoadRenderTexture(512, 512);
    SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);

    texPlatforms = LoadTexture("resources/platforms.png");
    SetTextureFilter(texPlatforms, TEXTURE_FILTER_BILINEAR);

    project = LoadProjectData("resources/project_template.rpc");

    // GUI: Main toolbar panel (file and visualization)
    //-----------------------------------------------------------------------------------
    mainToolbarState = InitGuiMainToolbar();

    // Set raygui style to start with
    // WARNING: It must be aligned with mainToolbarState.visualStyleActive
    mainToolbarState.visualStyleActive = 4;
    mainToolbarState.prevVisualStyleActive = 4;
    GuiLoadStyleGenesis();
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

    // Trial message(s) and Support Message Box
    //-----------------------------------------------------------------------------------
#if defined(SPLASH_SUPPORT_MESSAGE)
    supportMessageRandBtn = GetRandomValue(0, 1); // Used for the support message button order
#endif
    //-----------------------------------------------------------------------------------

    // Load application init configuration (if available)
    //-------------------------------------------------------------------------------------
    LoadApplicationConfig();
#if defined(PLATFORM_WEB)
    // Set callback to automatically save app config on page closing
    emscripten_set_beforeunload_callback(NULL, WebBeforeUnload);
#endif
    //-------------------------------------------------------------------------------------

#if !defined(PLATFORM_WEB)
    // File dropped over executable / command line input file
    //-------------------------------------------------------------------------------------
    if ((inFileName[0] != '\0') && (IsFileExtension(inFileName, ".rpc")))
    {
        rpbConfigData data = LoadProjectData(inFileName);    // Load tool data from file

        // TODO: Do something with loaded data
    }
    else
    {
        // TODO: Set some default values
    }
    //-------------------------------------------------------------------------------------
#endif

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

    UnloadRenderTexture(target);    // Unload render texture

    // Save application init configuration for next run
    //--------------------------------------------------------------------------------------
    SaveApplicationConfig();
    //--------------------------------------------------------------------------------------

    CloseWindow();                  // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

#endif  // !COMMAND_LINE_ONLY
    return 0;
}

//--------------------------------------------------------------------------------------------
// Module Functions Definition
//--------------------------------------------------------------------------------------------
// Update and draw one frame
static void UpdateDrawFrame(void)
{
    // Dropped files logic
    //----------------------------------------------------------------------------------
    if (IsFileDropped())
    {
        FilePathList droppedFiles = LoadDroppedFiles();

        if (IsFileExtension(droppedFiles.paths[0], ".ex1") ||
            IsFileExtension(droppedFiles.paths[0], ".ex2"))
        {
            // TODO: Do something with recognized fileformats

            SetWindowTitle(TextFormat("%s v%s - %s", toolName, toolVersion, inFileName));
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

    // Toggle window: report issue
    if (IsKeyPressed(KEY_F3)) showIssueReportWindow = !showIssueReportWindow;

    // Toggle window: user
    //if (IsKeyPressed(KEY_F4)) windowUserState.windowActive = !windowUserState.windowActive;

#if defined(PLATFORM_DESKTOP)
    // Toggle screen size (x2) mode
    //if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_F)) screenSizeActive = !screenSizeActive;
#endif
    // Toggle full screen mode
    //if (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_ENTER)) ToggleFullscreen();

    // Show dialog: load file (.xxx)
    if ((IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_O)) || mainToolbarState.btnLoadFilePressed) showLoadFileDialog = true;

    // Show dialog: save file (.xxx)
    if ((IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S)) || mainToolbarState.btnSaveFilePressed) showSaveFileDialog = true;

    // Show dialog: export file (.xxx)
    if ((IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_E)) || mainToolbarState.btnExportFilePressed) showExportFileDialog = true;

    // Show closing window on ESC
    if (IsKeyPressed(KEY_ESCAPE))
    {
        if (windowHelpState.windowActive) windowHelpState.windowActive = false;
        else if (windowAboutState.windowActive) windowAboutState.windowActive = false;
        else if (showIssueReportWindow) showIssueReportWindow = false;
        //else if (windowUserState.windowActive) windowUserState.windowActive = false;
    #if defined(PLATFORM_DESKTOP)
        else if (saveChangesRequired) showMessageExit = !showMessageExit;
        else closeWindow = true;
    #else
        else if (showLoadFileDialog) showLoadFileDialog = false;
        else if (showSaveFileDialog) showSaveFileDialog = false;
        else if (showExportFileDialog) showExportFileDialog = false;
    #endif
    }
    //----------------------------------------------------------------------------------

    // Main toolbar logic
    //----------------------------------------------------------------------------------
    // TODO: Process required buttons not processed previously
    //if (mainToolbarState.btnNewProjectPressed) showMessageReset = true;
    //else if (mainToolbarState.btnOpenProjectPressed) showLoadFileDialog = true;
    //else if (mainToolbarState.btnSaveProjectPressed) showSaveFileDialog = true;

    if (mainToolbarState.visualStyleActive != mainToolbarState.prevVisualStyleActive)
    {
        // Reset to default internal style
        // NOTE: Required to unload any previously loaded font texture
        GuiLoadStyleDefault();

        switch (mainToolbarState.visualStyleActive)
        {
            case 0: GuiLoadStyleCyber(); break;
            case 1: GuiLoadStyleLavanda(); break;
            case 2: GuiLoadStyleTerminal(); break;
            case 3: GuiLoadStyleAmber(); break;
            case 4: GuiLoadStyleGenesis(); break;
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
    framesCounter++;                    // General usage frames counter
    mousePoint = GetMousePosition();    // Get mouse position each frame

#if !defined(PLATFORM_WEB)
    if (WindowShouldClose())
    {
        if (saveChangesRequired) showMessageExit = true;
        else closeWindow = true;
    }
#endif

    // Screen scale logic (x2) -> Not used in this tool
    //----------------------------------------------------------------------------------
    /*
    if (screenSizeActive)
    {
        // Screen size x2
        if (GetScreenWidth() < screenWidth*2)
        {
            SetWindowSize(screenWidth*2, screenHeight*2);
            SetMouseScale(0.5f, 0.5f);
        }
    }
    else
    {
        // Screen size x1
        if (screenWidth*2 >= GetScreenWidth())
        {
            SetWindowSize(screenWidth, screenHeight);
            SetMouseScale(1.0f, 1.0f);
        }
    }
    */
    //----------------------------------------------------------------------------------

    // WARNING: Some windows should lock the main screen controls when shown
    if (windowHelpState.windowActive ||
        windowAboutState.windowActive ||
        showIssueReportWindow ||
        //windowUserState.windowActive ||
        showMessageExit ||
        showMessageReset ||
        showLoadFileDialog ||
        showExportFileDialog ||
        showSupportMessage)
    {
        lockBackground = true;
    }
    else lockBackground = false;
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    /*
    // TODO: Use a render texture if required
    BeginTextureMode(target);

        ClearBackground(BLANK);     // Clear render target

        //DrawTexture(texture, 0, 0, WHITE);

    EndTextureMode();
    */

    BeginDrawing();

        ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

        // TODO: Draw all program here (use states if required)
        //----------------------------------------------------------------------------------
        for (int i = 0; i < MAX_PLATFORMS; i++)
        {
            if (currentPlatform == i) platformEnabled[i] = true;
            if (platformEnabled[i] && (currentPlatform != i)) platformEnabled[i] = false;
        }

        for (int i = 0; i < MAX_PLATFORMS; i++)
        {
            GuiToggle((Rectangle){ 12 + (96 + 8)*i, 52, 96, 96 }, NULL, &platformEnabled[i]);
            Color colTex = GetColor(GuiGetStyle(TOGGLE, TEXT_COLOR_NORMAL));
            if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){ 12 + (96 + 8)*i, 52, 96, 96 }))
                colTex = GetColor(GuiGetStyle(TOGGLE, TEXT_COLOR_FOCUSED));
            if (platformEnabled[i]) colTex = GetColor(GuiGetStyle(TOGGLE, TEXT_COLOR_PRESSED));

            DrawTexturePro(texPlatforms, (Rectangle){ 128*i, 0, 128, 128 }, (Rectangle){ 12 + (96 + 8)*i, 52, 96, 96 },
                (Vector2){ 0.0f, 0.0f }, 0.0f, colTex);
        }

        for (int i = 0; i < MAX_PLATFORMS; i++)
        {
            if (platformEnabled[i] && (i != currentPlatform)) { currentPlatform = i; break; }
        }

        GuiTabBar((Rectangle){ 0, 52 + 96 + 12, GetScreenWidth(), 24 }, tabText, 6, &currentTab);
        
        int categoryHeight = 12;
        for (int i = 0; i < project.entryCount; i++)
        {
            if (project.entries[i].category == currentTab) categoryHeight += (24 + 8);
        }
        if ((categoryHeight > (GetScreenHeight() - 188 - 24)) && (currentTab != RPB_CAT_PLATFORM))
        {
            GuiScrollPanel((Rectangle){ 0, 188, GetScreenWidth(), GetScreenHeight() - 188 - 24 }, NULL,
                (Rectangle){ 0, 188, GetScreenWidth() - 16, categoryHeight }, &panelScroll, &panelView);
        }
        else
        {
            panelScroll = (Vector2){ 0 };
            panelView = (Rectangle){ 0, 188, GetScreenWidth(), GetScreenHeight() - 188 - 24 };
        }

        BeginScissorMode(panelView.x, panelView.y, panelView.width, panelView.height);

        for (int i = 0, k = 0; i < project.entryCount; i++)
        {
            if (currentTab == project.entries[i].category)
            {
                if ((project.entries[i].platform != RPB_PLATFORM_ANY) && (project.entries[i].platform != currentPlatform)) continue;

                if (project.entries[i].type != RPB_TYPE_BOOL) GuiLabel((Rectangle){ 24, 52 + 96 + 12 + 36 + (24 + 8)*k + panelScroll.y, 180, 24 }, TextFormat("%s:", project.entries[i].name));
               
                GuiSetStyle(TEXTBOX, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
                switch (project.entries[i].type)
                {
                    case RPB_TYPE_BOOL:
                    {
                        bool checked = (bool)project.entries[i].value;
                        GuiCheckBox((Rectangle){ 24 + 2, 52 + 96 + 12 + 36 + (24 + 8)*k + 2 + panelScroll.y, 20, 20 }, project.entries[i].name + 5, &checked);
                        project.entries[i].value = (checked? 1 : 0);
                    } break;
                    case RPB_TYPE_VALUE:
                    {
                        if (GuiValueBox((Rectangle){ 24 + 180, 52 + 96 + 12 + 36 + (24 + 8)*k + panelScroll.y, 180, 24 }, NULL, &project.entries[i].value, 0, 1024, project.entries[i].editMode)) project.entries[i].editMode = !project.entries[i].editMode;
                    } break;
                    case RPB_TYPE_TEXT:
                    {
                        if (GuiTextBox((Rectangle){ 24 + 180, 52 + 96 + 12 + 36 + (24 + 8)*k + panelScroll.y, 400, 24 }, project.entries[i].text, 255, project.entries[i].editMode)) project.entries[i].editMode = !project.entries[i].editMode;
                    } break;
                    case RPB_TYPE_TEXT_FILE:
                    {
                        if (GuiTextBox((Rectangle){ 24 + 180, 52 + 96 + 12 + 36 + (24 + 8)*k + panelScroll.y, 400 - 90, 24 }, project.entries[i].text, 255, project.entries[i].editMode)) project.entries[i].editMode = !project.entries[i].editMode;
                        GuiButton((Rectangle){ 24 + 180 + 404 - 90, 52 + 96 + 12 + 36 + (24 + 8)*k + panelScroll.y, 86, 24 }, "#6#Browse");
                    } break;
                    case RPB_TYPE_TEXT_PATH:
                    {
                        if (GuiTextBox((Rectangle){ 24 + 180, 52 + 96 + 12 + 36 + (24 + 8)*k + panelScroll.y, 400 - 90, 24 }, project.entries[i].text, 255, project.entries[i].editMode)) project.entries[i].editMode = !project.entries[i].editMode;
                        GuiButton((Rectangle){ 24 + 180 + 404 - 90, 52 + 96 + 12 + 36 + (24 + 8)*k + panelScroll.y, 86, 24 }, "#173#Browse");
                    } break;
                    default: break;
                }
                GuiSetStyle(TEXTBOX, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);

                if (project.entries[i].type == RPB_TYPE_BOOL) GuiStatusBar((Rectangle){ 24 + 180, 52 + 96 + 12 + 36 + (24 + 8)*k + panelScroll.y, 320 + 412, 24 }, project.entries[i].desc);
                else GuiStatusBar((Rectangle){ 24 + 180 + 412, 52 + 96 + 12 + 36 + (24 + 8)*k + panelScroll.y, 320, 24 }, project.entries[i].desc);

                k++;
            }
        }
        
        EndScissorMode();
        //----------------------------------------------------------------------------------

        // GUI: Main toolbar panel
        //----------------------------------------------------------------------------------
        GuiMainToolbar(&mainToolbarState);
        //----------------------------------------------------------------------------------

        // GUI: Status bar
        //----------------------------------------------------------------------------------
        GuiStatusBar((Rectangle){ 0, GetScreenHeight() - 24, 320, 24 }, "NO FILE LOADED");
        GuiStatusBar((Rectangle){ 320 - 1, GetScreenHeight() - 24, 180, 24 }, "FILE INFO");
        GuiStatusBar((Rectangle){ 320 + 180 - 2, GetScreenHeight() - 24, 340, 24 }, "MORE FILE INFO");
        GuiStatusBar((Rectangle){ 320 + 180 + 340 - 3, GetScreenHeight() - 24, GetScreenWidth() - 600, 24 }, NULL);
        //----------------------------------------------------------------------------------

        // NOTE: If some overlap window is open and main window is locked, we draw a background rectangle
        //if (GuiIsLocked())    // WARNING: It takes one extra frame to process, so we just check required conditions
        if (windowHelpState.windowActive ||
            windowAboutState.windowActive ||
            showIssueReportWindow ||
            //windowUserState.windowActive ||
            showMessageExit ||
            showMessageReset ||
            showLoadFileDialog ||
            showSaveFileDialog ||
            showExportFileDialog ||
            showSupportMessage)
        {
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)), 0.8f));
        }

        // WARNING: Before drawing the windows, we unlock raygui input
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

        // GUI: Export Window
        //----------------------------------------------------------------------------------------
        if (windowExportActive)
        {
            Rectangle messageBox = { (float)screenWidth/2 - 248/2, (float)screenHeight/2 - 200/2, 248, 164 };
            int result = GuiMessageBox(messageBox, "#7#Export File", " ", "#7#Export File");

            /*
            GuiLabel((Rectangle){ messageBox.x + 12, messageBox.y + 12 + 24, 106, 24 }, "Data Format:");
            exportFormatActive = GuiComboBox((Rectangle){ messageBox.x + 12 + 88, messageBox.y + 12 + 24, 136, 24 }, "Text (.txt);Binary (.bin)", exportFormatActive);

            if (exportFormatActive == 1) { exportImagesChecked = true; GuiDisable(); }
            exportImagesChecked = GuiCheckBox((Rectangle){ messageBox.x + 20, messageBox.y + 48 + 24, 16, 16 }, "Export individual PNG images", exportImagesChecked);
            GuiEnable();
            exportTextChunkChecked = GuiCheckBox((Rectangle){ messageBox.x + 20, messageBox.y + 72 + 24, 16, 16 }, "Embed image text as rIPt chunk", exportTextChunkChecked);
            */

            if (result == 1)    // Export button pressed
            {
                windowExportActive = false;
                strcpy(outFileName, "icon.ico");
                showExportFileDialog = true;
            }
            else if (result == 0) windowExportActive = false;
        }
        //----------------------------------------------------------------------------------

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

        // GUI: Load File Dialog (and loading logic)
        //----------------------------------------------------------------------------------------
        if (showLoadFileDialog)
        {
#if defined(CUSTOM_MODAL_DIALOGS)
            int result = GuiFileDialog(DIALOG_MESSAGE, "Load file...", inFileName, "Ok", "Just drag and drop your .rpc file!");
#else
            int result = GuiFileDialog(DIALOG_OPEN_FILE, "Load file...", inFileName, "", "File Type (*.rpc)");
#endif
            if (result == 1)
            {
                // TODO: Load file: inFileName

                SetWindowTitle(TextFormat("%s v%s - %s", toolName, toolVersion, GetFileName(inFileName)));
            }

            if (result >= 0) showLoadFileDialog = false;
        }
        //----------------------------------------------------------------------------------------

        // GUI: Save File Dialog (and saving logic)
        //----------------------------------------------------------------------------------------
        if (showSaveFileDialog)
        {
#if defined(CUSTOM_MODAL_DIALOGS)
            int result = GuiFileDialog(DIALOG_TEXTINPUT, "Save file...", outFileName, "Ok;Cancel", NULL);
#else
            int result = GuiFileDialog(DIALOG_SAVE_FILE, "Save file...", outFileName, "*.rpc", "Project Config (*.rpc)");
#endif
            if (result == 1)
            {
                // Save file: outFileName
                // Check for valid extension and make sure it is
                if ((GetFileExtension(outFileName) == NULL) || !IsFileExtension(outFileName, ".rpc")) strcat(outFileName, ".rpc\0");
                SaveProjectData(project, outFileName);

            #if defined(PLATFORM_WEB)
                // Download file from MEMFS (emscripten memory filesystem)
                // NOTE: Second argument must be a simple filename (we can't use directories)
                // NOTE: Included security check to (partially) avoid malicious code on PLATFORM_WEB
                if (strchr(outFileName, '\'') == NULL) emscripten_run_script(TextFormat("saveFileFromMEMFSToDisk('%s','%s')", outFileName, GetFileName(outFileName)));
            #endif
            }

            if (result >= 0) showSaveFileDialog = false;
        }
        //----------------------------------------------------------------------------------------

        // GUI: Export File Dialog (and saving logic)
        //----------------------------------------------------------------------------------------
        if (showExportFileDialog)
        {
            // Consider different supported file types
            int fileTypeActive = 0;
            char filters[64] = { 0 };

            if (fileTypeActive == 0) strcpy(filters, "*.xx1");
            else if (fileTypeActive == 1) strcpy(filters, "*.xx2");
            else if (fileTypeActive == 2) strcpy(filters, "*.xx3");

#if defined(CUSTOM_MODAL_DIALOGS)
            int result = GuiFileDialog(DIALOG_TEXTINPUT, "Export file...", outFileName, "Ok;Cancel", NULL);
#else
            int result = GuiFileDialog(DIALOG_SAVE_FILE, "Export file...", outFileName, filters, TextFormat("File Type (%s)", filters));
#endif
            if (result == 1)
            {
                // TODO: Export file: outFileName

            #if defined(PLATFORM_WEB)
                // Download file from MEMFS (emscripten memory filesystem)
                // NOTE: Second argument must be a simple filename (we can't use directories)
                // NOTE: Included security check to (partially) avoid malicious code on PLATFORM_WEB
                if (strchr(outFileName, '\'') == NULL) emscripten_run_script(TextFormat("saveFileFromMEMFSToDisk('%s','%s')", outFileName, GetFileName(outFileName)));
            #endif
            }

            if (result >= 0) showExportFileDialog = false;
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
    printf("\n/////////////////////////////////////////////////////////////////////////////////\n");
    printf("//                                                                               //\n");
    printf("// %s v%s - %s                 //\n", toolName, toolVersion, toolDescription);
    printf("// powered by raylib v%s and raygui v%s                                   //\n", RAYLIB_VERSION, RAYGUI_VERSION);
    printf("// more info and bugs-report: github.com/raylibtech/rtools                       //\n");
    printf("// feedback and support:      ray[at]raylibtech.com                              //\n");
    printf("//                                                                               //\n");
    printf("// Copyright (c) 2022-2025 raylib technologies (@raylibtech)                     //\n");
    printf("//                                                                               //\n");
    printf("///////////////////////////////////////////////////////////////////////////////////\n\n");

    printf("USAGE:\n\n");
    printf("    > rpb [--help] --input <filename.ext> [--output <filename.ext>]\n");
    printf("                    [--format <value>]\n");

    printf("\nOPTIONS:\n\n");
    printf("    -h, --help                      : Show tool version and command line usage help\n\n");
    printf("    -i, --input <filename.ext>      : Define input file.\n");
    printf("                                      Supported extensions: .xx1, .xx2\n\n");
    printf("    -o, --output <filename.ext>     : Define output file.\n");
    printf("                                      Supported extensions: .xx1, .xx2\n");
    printf("                                      NOTE: If not specified, defaults to: output.xx1\n\n");
    printf("    -f, --format <value>            : Format output file.\n");
    printf("                                      Supported values:\n");
    printf("                                          Sample rate:      22050, 44100\n");
    printf("                                          Sample size:      8, 16, 32\n");
    printf("                                          Channels:         1 (mono), 2 (stereo)\n");
    printf("                                      NOTE: If not specified, defaults to: 44100, 16, 1\n\n");

    printf("\nEXAMPLES:\n\n");
    printf("    > rpb --input sound.rfx --output jump.wav\n");
    printf("        Process <sound.rfx> to generate <sound.wav> at 44100 Hz, 32 bit, Mono\n\n");
    printf("    > rpb --input sound.rfx --output jump.wav --format 22050 16 2\n");
    printf("        Process <sound.rfx> to generate <jump.wav> at 22050 Hz, 16 bit, Stereo\n\n");
    printf("    > rpb --input sound.rfx --play\n");
    printf("        Plays <sound.rfx>, wave data is generated internally but not saved\n\n");
}

// Process command line input
static void ProcessCommandLine(int argc, char *argv[])
{
    // CLI required variables
    bool showUsageInfo = false;         // Toggle command line usage info
    int outputFormat = 0;               // Supported output formats

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
                if (IsFileExtension(argv[i + 1], ".xx1") ||
                    IsFileExtension(argv[i + 1], ".xx2"))
                {
                    strcpy(inFileName, argv[i + 1]);    // Read input filename
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
                if (IsFileExtension(argv[i + 1], ".xx1") ||
                    IsFileExtension(argv[i + 1], ".xx2"))
                {
                    strcpy(outFileName, argv[i + 1]);   // Read output filename
                }
                else printf("WARNING: Output file extension not recognized\n");

                i++;
            }
            else printf("WARNING: No output file provided\n");
        }
        else if ((strcmp(argv[i], "-f") == 0) || (strcmp(argv[i], "--format") == 0))
        {
            // Check for valid argument and valid parameters
            if (((i + 1) < argc) && (argv[i + 1][0] != '-'))
            {
                // TODO: CLI: Read provided values
            }
            else printf("WARNING: Format parameters provided not valid\n");
        }
    }

    // Process input file if provided
    if (inFileName[0] != '\0')
    {
        // Set a default name for output in case not provided
        if (outFileName[0] == '\0') strcpy(outFileName, TextFormat("%s.out", GetFileNameWithoutExt(inFileName)));

        printf("\nInput file:       %s", inFileName);
        printf("\nOutput file:      %s", outFileName);
        printf("\nOutput format:    %i\n\n", 0);

        // TODO: CLI: Process input --> output
    }

    if (showUsageInfo) ShowCommandLineInfo();
}
#endif      // PLATFORM_DESKTOP || COMMAND_LINE_ONLY

//--------------------------------------------------------------------------------------------
// Load/Save/Export functions
//--------------------------------------------------------------------------------------------
// Load project config data data from .rpc file
static rpbConfigData LoadProjectData(const char *fileName)
{
    rpbConfigData data = { 0 };

    if (FileExists(fileName))
    {
        rini_data config = { 0 };
        config = rini_load(fileName);
        
        // Process/organize config data for our application
        data.entries = (rpbEntry *)RL_CALLOC(config.count, sizeof(rpbEntry));
        data.entryCount = config.count;

        for (int i = 0; i < data.entryCount; i++)
        {
            strcpy(data.entries[i].key, config.values[i].key);
            strcpy(data.entries[i].desc, config.values[i].desc);
            data.entries[i].platform = RPB_PLATFORM_ANY;

            // Category is parsed from first word on key
            char category[32] = { 0 };
            int categoryLen = 0; //TextFindIndex(config.values[i].key, "_");
            for (int c = 0; c < 128; c++) { if (config.values[i].key[c] != '_') categoryLen++; else break; }
            strncpy(category, config.values[i].key, categoryLen);
            strcpy(data.entries[i].name, TextReplace(config.values[i].key + categoryLen + 1, "_", " "));

            if (TextIsEqual(category, "PROJECT")) data.entries[i].category = RPB_CAT_PROJECT;
            else if (TextIsEqual(category, "BUILD")) data.entries[i].category = RPB_CAT_BUILD;
            else if (TextIsEqual(category, "PLATFORM"))
            {
                data.entries[i].category = RPB_CAT_PLATFORM;

                // Get platform from key
                char platform[32] = { 0 };
                int platformLen = 0;//TextFindIndex(config.values[i].key + categoryLen + 1, "_");
                for (int c = 0; c < 128; c++) { if (config.values[i].key[c + categoryLen + 1] != '_') platformLen++; else break; }
                memcpy(platform, config.values[i].key + categoryLen + 1, platformLen);

                if (TextIsEqual(platform, "WINDOWS")) data.entries[i].platform = RPB_PLATFORM_WINDOWS;
                else if (TextIsEqual(platform, "LINUX")) data.entries[i].platform = RPB_PLATFORM_LINUX;
                else if (TextIsEqual(platform, "MACOS")) data.entries[i].platform = RPB_PLATFORM_MACOS;
                else if (TextIsEqual(platform, "HTML5")) data.entries[i].platform = RPB_PLATFORM_HTML5;
                else if (TextIsEqual(platform, "ANDROID")) data.entries[i].platform = RPB_PLATFORM_ANDROID;
                else if (TextIsEqual(platform, "DRM")) data.entries[i].platform = RPB_PLATFORM_DRM;
                else if (TextIsEqual(platform, "SWITCH")) data.entries[i].platform = RPB_PLATFORM_SWITCH;
                else if (TextIsEqual(platform, "DREAMCAST")) data.entries[i].platform = RPB_PLATFORM_DREAMCAST;
                else if (TextIsEqual(platform, "FREEBSD")) data.entries[i].platform = RPB_PLATFORM_FREEBSD;

                memset(data.entries[i].name, 0, 64);
                strcpy(data.entries[i].name, config.values[i].key + categoryLen + platformLen + 2);
            }
            else if (TextIsEqual(category, "DEPLOY")) data.entries[i].category = RPB_CAT_DEPLOY;
            else if (TextIsEqual(category, "IMAGERY")) data.entries[i].category = RPB_CAT_IMAGERY;
            else if (TextIsEqual(category, "RAYLIB")) data.entries[i].category = RPB_CAT_RAYLIB;
        }
        
        for (int i = 0; i < data.entryCount; i++)
        {
            // Type is parsed from key and value
            if (!config.values[i].isText)
            {
                if (TextFindIndex(data.entries[i].key, "_FLAG")) data.entries[i].type = RPB_TYPE_BOOL;
                else data.entries[i].type = RPB_TYPE_VALUE;
                
                // Get the value
                data.entries[i].value = TextToInteger(config.values[i].text);
            }
            else // Value is text
            {
                if (TextFindIndex(data.entries[i].key, "_FILES") > 0)
                {
                    // TODO: How we check if files list includes multiple files,
                    // checking for ';' separator???
                    data.entries[i].type = RPB_TYPE_TEXT_FILE;
                    data.entries[i].multi = true;
                }
                else if (TextFindIndex(data.entries[i].key, "_FILE")  > 0) data.entries[i].type = RPB_TYPE_TEXT_FILE;
                else if (TextFindIndex(data.entries[i].key, "_PATH")  > 0) data.entries[i].type = RPB_TYPE_TEXT_PATH;
                else
                {
                    data.entries[i].type = RPB_TYPE_TEXT;
                }

                strcpy(data.entries[i].text, config.values[i].text);
            }
        }

        rini_unload(&config);
    }

    return data;
}

// Save project config data to .rpc file
// NOTE: Same function as [rpc] tool but but adding more data
static void SaveProjectData(rpbConfigData data, const char *fileName)
{
    rini_data config = rini_load(NULL);   // Create empty config with 32 entries (RINI_MAX_CONFIG_CAPACITY)

    // Define header comment lines
    rini_set_comment_line(&config, NULL);   // Empty comment line, but including comment prefix delimiter
    rini_set_comment_line(&config, "raylib project creator - project definition file");
    rini_set_comment_line(&config, NULL);
    rini_set_comment_line(&config, "This definition file contains all required info to descrive a project");
    rini_set_comment_line(&config, "and allow building it for multiple platforms");
    rini_set_comment_line(&config, NULL);
    rini_set_comment_line(&config, "This file follow certain conventions to be able to display the information in");
    rini_set_comment_line(&config, "an easy-configurable UI manner when loaded through [raylib project builder]");
    rini_set_comment_line(&config, "CONVENTIONS:");
    rini_set_comment_line(&config, "   - ID containing [_FLAG_]: Value is considered a boolean, it displays with a [GuiCheckBox]");
    rini_set_comment_line(&config, "   - ID do not contain "": Value is considered as an integer, it displays as [GuiValueBox]");
    rini_set_comment_line(&config, "   - ID ends with _FILE or _FILES: Value is considered as a text file path, it displays as [GuiTextBox] with a [BROWSE-File] button");
    rini_set_comment_line(&config, "   - ID ends with _PATH: Value is considered as a text directory path, it displays as [GuiTextBox] with a [BROWSE-Dir] button");
    rini_set_comment_line(&config, NULL);
    rini_set_comment_line(&config, "NOTE: The comments/description for each entry is used as tooltip when editing the entry on [rpb]");
    rini_set_comment_line(&config, "\n");

    /*
    char key[64];       // Entry key (as read from .rpc)
    char name[64];      // Entry name label for display, computed from key
    int category;       // Entry category: PROJECT, BUILDING, PLATFORM, DEPLOY, IMAGERY, raylib
    int platform;       // Entry platform-specific
    int type;           // Entry type of data: VALUE (int), BOOL (int), TEXT (string), FILE (string-file), PATH (string-path)
    int value;          // Entry value (type: VALUE, BOOL)
    // TODO: WARNING: rini expects a maximum len for text of 256 chars, multiple files can be longer that that
    char text[256];     // Entry text data (type: TEXT, FILE, PATH) - WARNING: It can include multiple paths
    char desc[128];     // Entry data description, useful for tooltips
    */

    // We are saving data into file organized by categories and platforms,
    // independently of the format it was originally loaded (in case of manual edition)

    // Saving PROJECT category data
    rini_set_comment_line(&config, "Project settings");
    rini_set_comment_line(&config, "------------------------------------------------------------------------------------");
    for (int i = 0; i < data.entryCount; i++)
    {
        rpbEntry *entry = &data.entries[i];

        if (entry->category == RPB_CAT_PROJECT)
        {
            switch (entry->type)
            {
                case RPB_TYPE_BOOL:
                case RPB_TYPE_VALUE: rini_set_value(&config, entry->key, entry->value, entry->desc); break;
                case RPB_TYPE_TEXT:
                case RPB_TYPE_TEXT_FILE:
                case RPB_TYPE_TEXT_PATH: rini_set_value_text(&config, entry->key, entry->text, entry->desc); break;
                default: break;
            }
        }
    }

    rini_save(config, fileName);
    rini_unload(&config);
}

//--------------------------------------------------------------------------------------------
// Auxiliar functions (utilities)
//--------------------------------------------------------------------------------------------
// ...

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
        //mainToolbarState.showInfoWindowActive = rini_get_value_fallback(config, "SHOW_WINDOW_INFO", 1);
        //mainToolbarState.showTooltips = rini_get_value_fallback(config, "SHOW_CONTROL_TOOLTIPS", 1); // Default to 1 if key not found
        windowMaximized = rini_get_value(config, "INIT_WINDOW_MAXIMIZED");
        mainToolbarState.visualStyleActive = rini_get_value(config, "GUI_VISUAL_STYLE");
        //mainToolbarState.cleanModeActive = rini_get_value(config, "CLEAN_WINDOW_MODE");

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
    rini_data config = rini_load(NULL);   // Create empty config with 32 entries (RINI_MAX_CONFIG_CAPACITY)

    // Define header comment lines
    rini_set_comment_line(&config, NULL);   // Empty comment line, but including comment prefix delimiter
    rini_set_comment_line(&config, TextFormat("%s initialization configuration options", TOOL_NAME));
    rini_set_comment_line(&config, NULL);
    rini_set_comment_line(&config, "NOTE: This file is loaded at application startup,");
    rini_set_comment_line(&config, "if file is not found, default values are applied");
    rini_set_comment_line(&config, NULL);

#if defined(PLATFORM_DESKTOP)
    int windowMaximized = (int)IsWindowMaximized();
#endif
    rini_set_value(&config, "SHOW_WINDOW_WELCOME", (int)windowAboutState.showSplash, "Show welcome window at initialization");
    //rini_set_value(&config, "SHOW_WINDOW_INFO", (int)mainToolbarState.showInfoWindowActive, "Show image info window");
    //rini_set_value(&config, "SHOW_CONTROL_TOOLTIPS", (int)mainToolbarState.showTooltips, "Show controls tooltips on mouse hover");
#if defined(PLATFORM_DESKTOP)
    rini_set_value(&config, "INIT_WINDOW_MAXIMIZED", (int)windowMaximized, "Initialize window maximized");
#endif
    //rini_set_value(&config, "SHOW_IMAGE_GRID", (int)mainToolbarState.helperGridActive, "Show image grid");
    rini_set_value(&config, "GUI_VISUAL_STYLE", (int)mainToolbarState.visualStyleActive, "UI visual style selected");
    //rini_set_value(&config, "CLEAN_WINDOW_MODE", (int)mainToolbarState.cleanModeActive, "Clean window mode enabled");

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
