#pragma once

#include "json.hpp"
#include <fstream>
#include <string>

namespace CZML
{
    using nlohmann::json;
    using namespace nlohmann::literals;

    json export_json(const std::string &name, const std::vector<double> &positions, const std::string &epoch, bool isDebris);
}
