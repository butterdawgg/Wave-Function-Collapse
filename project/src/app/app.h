#ifndef APP_H
#define APP_H

#include <memory>
#include <string>
#include <vector>

#include "wfc/wfc_solver.h"



class AppCamera;
class GridRenderer;
class Grid;

class App final
{
    public:

    App(int screenWidth = 1280, int screenHeight = 720, const char* title = "WaveFunctionCollapse");

    void Run();

    ~App();

    private:

    void Update(float deltaTime);
    void Render();

    // Rebuilds m_Grid and m_WFC from m_Settings and m_GridRows/Cols, then runs WFC
    void RebuildAndSolve();

    // Main
    int m_ScreenWidth;
    int m_ScreenHeight;
    const char* m_Title;

    // Rendering
    std::unique_ptr<AppCamera> m_Camera;
    std::unique_ptr<GridRenderer> m_Renderer;

    // Algorithm
    std::unique_ptr<Grid> m_Grid;
    std::unique_ptr<WFC_Solver> m_WFC;
    WFC_Settings m_Settings;

    // Tileset panel
    char m_SpritesheetPathBuf[512] { "data/default_spritesheet.png" };
    int m_TilePixelSize { 16 };
    int m_TileScreenSize { 16 };
    std::string m_SpritesheetStatus { "No spritesheet loaded" };
    bool m_SpritesheetStatusOk { false };

    // Settings panel
    char m_SettingsPathBuf[512] { "data/default_settings.json" };
    std::string m_SettingsStatus { "No settings loaded" };
    bool m_SettingsStatusOk { false };

    // Training panel
    char m_TrainImageBuf[512] { "" };
    std::vector<std::string> m_TrainImages;
    char m_SaveSettingsPathBuf[512] { "data/settings_trained.json" };
    std::string m_TrainStatus { "" };
    bool m_TrainStatusOk { false };

    // Generate panel
    int m_GridRows { 40 };
    int m_GridCols { 40 };
    std::string m_SolveStatus { "" };
    bool m_SolveStatusOk { false };
    float m_LastSolveMs { 0.0f };
};

#endif // !APP_H