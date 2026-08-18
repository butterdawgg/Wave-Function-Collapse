#ifndef WFC_SETTINGS_IO_H
#define WFC_SETTINGS_IO_H

#include "wfc_solver.h"

/*
    Serialises / deserialises WFC_Settings to and from a JSON file.

    JSON schema:

    {
        "tileIds": [0, 1, 2, ...],

        "tileWeights": {
            "0": 25.0,
            "13": 10.5
        },

        "adjacencyRules": [
            { "tile": 0, "direction": 0, "neighbour": 3 },
            ...
        ]
    }

    Direction encoding matches the solver constants:
        0 = Up, 1 = Right, 2 = Down, 3 = Left
*/

/* Saves WFC settings into a .json file */
bool SaveWFCSettings(const WFC_Settings& settings, const char* path);
/* Loads WFC settings from a .json file */
bool LoadWFCSettings(WFC_Settings& outSettings, const char* path);

#endif // !WFC_SETTINGS_IO_H