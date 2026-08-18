#include <raylib.h>
#include <imgui.h>
#include <rlImGui.h>

#include <chrono>

#include "app.h"
#include "camera.h"
#include "grid_renderer.h"

#include "wfc/grid.h"
#include "wfc/wfc_solver.h"
#include "wfc/wfc_trainer.h"
#include "wfc/wfc_settings_io.h"



// -----------
// --- App ---
// -----------

App::App(int screenWidth, int screenHeight, const char* title)
    : m_ScreenWidth(screenWidth), m_ScreenHeight(screenHeight), m_Title(title)
{
    InitWindow(m_ScreenWidth, m_ScreenHeight, m_Title);
    SetTargetFPS(60);
    rlImGuiSetup(true);

    // Init rendering components
    m_Camera = std::make_unique<AppCamera>();
    m_Renderer = std::make_unique<GridRenderer>();
    m_Grid = std::make_unique<Grid>();

    if (m_Renderer->SetSpritesheet(m_SpritesheetPathBuf))
    {
        m_SpritesheetStatus = "Loaded: " + std::string(m_SpritesheetPathBuf);
        m_SpritesheetStatusOk = true;
    }
    else
    {
        m_SpritesheetStatus = "Error: could not load file";
        m_SpritesheetStatusOk = false;
    }

    m_Renderer->SetGrid(m_Grid.get());
    m_Renderer->SetTileDstSize(static_cast<float>(m_TilePixelSize));

    // Try to load default settings and run an initial solve
    if (LoadWFCSettings(m_Settings, m_SettingsPathBuf))
    {
        m_SettingsStatus = "Loaded: " + std::string(m_SettingsPathBuf);
        m_SettingsStatusOk = true;
        RebuildAndSolve();
    }
    else
    {
        m_SettingsStatus = "Could not load default settings file";
        m_SettingsStatusOk = false;
    }
}

void App::RebuildAndSolve()
{
    using clock = std::chrono::high_resolution_clock;
    using seconds = std::chrono::duration<float>;

    // Rebuild grid
    m_Grid = std::make_unique<Grid>();
    m_Grid->GetTiles().Resize(m_GridRows, m_GridCols, -1);

    m_Renderer->SetGrid(m_Grid.get());

    // Rebuild solver
    m_WFC = std::make_unique<WFC_Solver>(*m_Grid, m_Settings);

    // Solve and time it
    auto t0 = clock::now();

    while (!m_WFC->Solve()) { };

    m_LastSolveMs = seconds(clock::now() - t0).count() * 1000.0f;

    m_SolveStatus = "Solved in " + std::to_string(m_LastSolveMs).substr(0, 6) + " ms";
    m_SolveStatusOk = true;
}

void App::Run()
{
    auto lastTime = std::chrono::high_resolution_clock::now();

    while (!WindowShouldClose())
    {
        auto now = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(now - lastTime).count();
        lastTime = now;

        Update(dt);
        Render();
    }
}

void App::Update(float deltaTime)
{
    m_Camera->Update(deltaTime);
}

void App::Render()
{
    BeginDrawing();
    ClearBackground(BLACK);

    m_Renderer->DrawGrid(m_Camera.get());

    rlImGuiBegin();

    ImGui::SetNextWindowSize(ImVec2(400, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::Begin("Wave Function Collapse");



    // ----------------------
    // --- Controls panel ---
    // ----------------------
    if (ImGui::CollapsingHeader("Controls", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Indent();
        ImGui::TextDisabled("Camera");
        ImGui::Separator();
        ImGui::Text("Arrow keys   Move");
        ImGui::Text("+ / -        Zoom in / out");
        ImGui::Unindent();
    }

    ImGui::Spacing();



    // ---------------------
    // --- Tileset panel ---
    // ---------------------
    if (ImGui::CollapsingHeader("Tileset", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Indent();

        ImGui::SetNextItemWidth(-150.0f);
        ImGui::InputText("Spritesheet##spr", m_SpritesheetPathBuf, sizeof(m_SpritesheetPathBuf));
        ImGui::SetNextItemWidth(150.0f);
        ImGui::InputInt("Tile size in texture", &m_TilePixelSize);
        if (m_TilePixelSize < 1) m_TilePixelSize = 1;

        ImGui::SetNextItemWidth(150.0f);
        ImGui::InputInt("Tile size on screen", &m_TileScreenSize);
        if (m_TileScreenSize < 1) m_TileScreenSize = 1;

        ImGui::Spacing();

        if (ImGui::Button("Apply##tileset", ImVec2(-1, 0)))
        {
            if (m_Renderer->SetSpritesheet(m_SpritesheetPathBuf))
            {
                m_SpritesheetStatus = "Loaded: " + std::string(m_SpritesheetPathBuf);
                m_SpritesheetStatusOk = true;
            }
            else
            {
                m_SpritesheetStatus = "Error: could not load file";
                m_SpritesheetStatusOk = false;
            }

            m_Renderer->SetTileSrcSize(m_TilePixelSize);
            m_Renderer->SetTileDstSize(static_cast<float>(m_TileScreenSize));
        }

        if (!m_SpritesheetStatus.empty())
        {
            if (m_SpritesheetStatusOk)
                ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "%s", m_SpritesheetStatus.c_str());
            else
                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%s", m_SpritesheetStatus.c_str());
        }

        ImGui::Unindent();
    }

    ImGui::Spacing();



    // ----------------------
    // --- Settings panel ---
    // ----------------------
    if (ImGui::CollapsingHeader("Settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Indent();

        ImGui::SetNextItemWidth(-80.0f);
        ImGui::InputText("##settingsPath", m_SettingsPathBuf, sizeof(m_SettingsPathBuf));
        ImGui::SameLine();

        if (ImGui::Button("Load##settings"))
        {
            WFC_Settings loaded;
            if (LoadWFCSettings(loaded, m_SettingsPathBuf))
            {
                m_Settings = std::move(loaded);
                m_SettingsStatus = "Loaded: " + std::string(m_SettingsPathBuf);
                m_SettingsStatusOk = true;
            }
            else
            {
                m_SettingsStatus = "Error: could not load file";
                m_SettingsStatusOk = false;
            }
        }

        if (!m_SettingsStatus.empty())
        {
            if (m_SettingsStatusOk)
                ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "%s", m_SettingsStatus.c_str());
            else
                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%s", m_SettingsStatus.c_str());
        }

        ImGui::Unindent();
    }

    ImGui::Spacing();



    // ----------------------
    // --- Training panel ---
    // ----------------------
    if (ImGui::CollapsingHeader("Training"))
    {
        ImGui::Indent();

        ImGui::TextDisabled("Training images");
        ImGui::Separator();

        ImGui::SetNextItemWidth(-80.0f);
        ImGui::InputText("##trainImg", m_TrainImageBuf, sizeof(m_TrainImageBuf),
            ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::SameLine();

        if (ImGui::Button("Add") && m_TrainImageBuf[0] != '\0')
        {
            m_TrainImages.emplace_back(m_TrainImageBuf);
            m_TrainImageBuf[0] = '\0';
        }

        if (!m_TrainImages.empty())
        {
            int removeIdx = -1;
            for (int i = 0; i < static_cast<int>(m_TrainImages.size()); ++i)
            {
                ImGui::PushID(i);
                ImGui::BulletText("%s", m_TrainImages[i].c_str());
                ImGui::SameLine();
                if (ImGui::SmallButton("x"))
                    removeIdx = i;
                ImGui::PopID();
            }
            if (removeIdx >= 0)
                m_TrainImages.erase(m_TrainImages.begin() + removeIdx);
        }
        else
        {
            ImGui::TextDisabled("  (no images added)");
        }

        ImGui::Spacing();
        ImGui::TextDisabled("Save trained settings");
        ImGui::Separator();

        ImGui::SetNextItemWidth(-150.0f);
        ImGui::InputText("Save path##sv", m_SaveSettingsPathBuf, sizeof(m_SaveSettingsPathBuf));

        ImGui::Spacing();

        const bool canTrain = !m_TrainImages.empty();
        if (!canTrain) ImGui::BeginDisabled();

        if (ImGui::Button("Train & Apply", ImVec2(-1, 0)))
        {
            WFC_Trainer trainer;

            bool trainOk { true };

            if (!trainer.BeginTraining(m_TilePixelSize, m_SpritesheetPathBuf, { 0, 0, 0, 0 }))
            {
                m_TrainStatus = "Error: could not open spritesheet";
                m_TrainStatusOk = false;
                trainOk = false;
            }

            if (trainOk)
            {
                for (const auto& img : m_TrainImages)
                {
                    if (!trainer.Train(img.c_str()))
                    {
                        m_TrainStatus = "Error: could not open " + img;
                        m_TrainStatusOk = false;
                        trainOk = false;
                        break;
                    }
                }
            }

            // Apply settings
            if (trainOk)
            {
                m_Settings = trainer.GetTrainingData();

                m_SettingsStatus = "Settings from training (" +
                    std::to_string(m_TrainImages.size()) + " image(s))";

                m_SettingsStatusOk = true;

                if (m_SaveSettingsPathBuf[0] != '\0')
                {
                    if (SaveWFCSettings(m_Settings, m_SaveSettingsPathBuf))
                    {
                        m_TrainStatus = "Trained & saved to: " + std::string(m_SaveSettingsPathBuf);
                        m_TrainStatusOk = true;
                    }
                    else
                    {
                        m_TrainStatus = "Trained, but save failed";
                        m_TrainStatusOk = false;
                    }
                }
                else
                {
                    m_TrainStatus = "Trained (not saved — no path given)";
                    m_TrainStatusOk = true;
                }
            }
        }

        if (!canTrain)
        {
            ImGui::EndDisabled();
            ImGui::TextDisabled("Add at least one training image.");
        }

        if (!m_TrainStatus.empty())
        {
            if (m_TrainStatusOk)
                ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "%s", m_TrainStatus.c_str());
            else
                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%s", m_TrainStatus.c_str());
        }

        ImGui::Unindent();
    }

    ImGui::Spacing();



    // ----------------------
    // --- Generate panel ---
    // ----------------------
    if (ImGui::CollapsingHeader("Generate", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Indent();

        ImGui::TextDisabled("Grid size");
        ImGui::Separator();

        ImGui::SetNextItemWidth(150.0f);
        ImGui::InputInt("Rows", &m_GridRows);
        ImGui::SetNextItemWidth(150.0f);
        ImGui::InputInt("Cols", &m_GridCols);

        if (m_GridRows < 1) m_GridRows = 1;
        if (m_GridCols < 1) m_GridCols = 1;

        ImGui::Spacing();

        const bool hasSettings = !m_Settings.tileIds.empty();
        if (!hasSettings) ImGui::BeginDisabled();

        if (ImGui::Button("Generate##xx", ImVec2(-1, 0)))
            RebuildAndSolve();

        if (!hasSettings)
        {
            ImGui::EndDisabled();
            ImGui::TextDisabled("Load or train settings first.");
        }

        if (!m_SolveStatus.empty())
        {
            ImGui::Spacing();
            if (m_SolveStatusOk)
                ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "%s", m_SolveStatus.c_str());
            else
                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%s", m_SolveStatus.c_str());
        }

        ImGui::Unindent();
    }

    ImGui::End();

    rlImGuiEnd();
    EndDrawing();
}

App::~App()
{
    rlImGuiShutdown();
    CloseWindow();
}