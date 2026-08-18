#ifndef WFC_TRAINER_H
#define WFC_TRAINER_H

#include "wfc_solver.h"

#include <raylib.h>

class WFC_Trainer final
{
    public:

    WFC_Trainer() = default;
    ~WFC_Trainer() = default;

    /*
        Clears current training progress
        Call before Train()
        You'd need to provide a tileset image
        Tiles that are fully ignoreColor will be ignored
        (can be used to have gaps in the tileset)
        Returns false if image can't be loaded
    */
    bool BeginTraining(int tileSize, const char* tilesetImagePath, const Color& ignoreColor);

    /*
        Supplements current training data with the provided tilemap image
        Call AFTER BeginTraining()
        Call this multiple times to provide more statistical data to the model\
        Returns false if image can't be loaded
    */
    bool Train(const char* imagePath);

    /* Returns the current training data */
    const WFC_Settings& GetTrainingData() const;

    private:

    bool m_IsValid { false };

    WFC_Settings m_Settings { };

    int m_TileSize { };

    std::unordered_map<int, std::vector<Color>> m_TilePixels { };
};

#endif // !WFC_TRAINER_H