#include <fstream>

#include <nlohmann/json.hpp>

#include "wfc_settings_io.h"
#include "debug_utils.h"

using json = nlohmann::json;



// ----------------
// --- Helpers  ---
// ----------------

/*
    Walks the AdjacencyRules map and writes every stored rule as a flat
    array of { tile, direction, neighbour } objects
*/
static json SerializeRules(const AdjacencyRules& rules)
{
    json arr = json::array();

    for (const auto& [tileId, dirs] : rules.rules)
    {
        for (int dir { 0 }; dir < static_cast<int>(dirs.size()); ++dir)
        {
            for (int neighbourId : dirs[dir])
            {
                arr.push_back(
                    {
                        { "tile", tileId },
                        { "direction", dir },
                        { "neighbour", neighbourId }
                    }
                );
            }
        }
    }

    return arr;
}

/* Rebuilds an AdjacencyRules instance from the flat array produced by SerializeRules() */
static AdjacencyRules DeserializeRules(const json& arr)
{
    AdjacencyRules rules;

    for (const auto& entry : arr)
    {
        const int tile { entry.at("tile").get<int>() };
        const int direction { entry.at("direction").get<int>() };
        const int neighbour { entry.at("neighbour").get<int>() };

        rules.AddRule(tile, direction, neighbour);
    }

    return rules;
}



// ------------------
// --- Public API ---
// ------------------

bool SaveWFCSettings(const WFC_Settings& settings, const char* path)
{
    json root;

    // tileIds
    root["tileIds"] = settings.tileIds;

    // tileWeights (keys must be strings in JSON objects)
    json weights = json::object();

    for (const auto& [id, weight] : settings.tileWeights)
        weights[std::to_string(id)] = weight;

    root["tileWeights"] = std::move(weights);

    // adjacencyRules
    root["adjacencyRules"] = SerializeRules(settings.rules);

    // Write to disk
    std::ofstream file(path);

    if (!file.is_open())
    {
        logError("[WFC] SaveSettings: could not open '{}' for writing", path);

        return false;
    }

    file << root.dump(4); // 4-space indentation for readability

    if (file.fail())
    {
        logError("[WFC] SaveSettings: write error on '{}'", path);

        return false;
    }

    return true;
}

bool LoadWFCSettings(WFC_Settings& outSettings, const char* path)
{
    std::ifstream file(path);

    if (!file.is_open())
    {
        logError("[WFC] LoadSettings: could not open '{}'", path);
        return false;
    }

    json root;

    try
    {
        root = json::parse(file);
    }
    catch (const json::parse_error& e)
    {
        logError("[WFC] LoadSettings: JSON parse error in '{}': {}", path, e.what());

        return false;
    }

    try
    {
        // tileIds
        outSettings.tileIds = root.at("tileIds").get<std::vector<int>>();

        // tileWeights (stored with string keys, convert back to int)
        outSettings.tileWeights.clear();

        for (const auto& [key, value] : root.at("tileWeights").items())
            outSettings.tileWeights[std::stoi(key)] = value.get<float>();

        // adjacencyRules
        outSettings.rules = DeserializeRules(root.at("adjacencyRules"));
    }
    catch (const json::exception& e)
    {
        logError("[WFC] LoadSettings: unexpected JSON structure in '{}': {}", path, e.what());

        return false;
    }
    catch (const std::invalid_argument& e)
    {
        logError("[WFC] LoadSettings: invalid tile id key in '{}': {}", path, e.what());

        return false;
    }

    return true;
}