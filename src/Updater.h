#pragma once

#include "json.hpp"

namespace Updater
{
    inline bool hasUpdate = false;
    inline nlohmann::json request;

    void init();
    void checkForUpdate();

    void draw();
};