
#include <czml.hpp>

using nlohmann::json;
using namespace nlohmann::literals;

json CZML::export_json(const std::string &name, const std::vector<double> &positions, const std::string &epoch, bool isDebris)
{
    json templat;
    templat["id"] = "Satellite/" + name;
    templat["name"] = name;

    templat["point"] = R"({"show":[{"boolean":true}],"color":{"rgba":[255,255,0,50]},"outlineColor":{"rgba":[255,128,40,255]},"pixelSize":4,"outlineWidth ":0})"_json;
    templat["label"] = R"({"fillColor": {"rgba": [0, 255, 0, 255 ]},
                            "font": "11pt Lucida Console",
                            "horizontalOrigin": "LEFT",
                            "outlineColor": {"rgba": [0,0,0,255] },
                            "outlineWidth": 2,
                            "pixelOffset": {"cartesian2": [12,0] },
                            "show": false,
                            "style": "FILL_AND_OUTLINE",
                            "text": "Geoeye 1",
                            "verticalOrigin": "CENTER"
                            })"_json;

    templat["position"] = R"({
                            "interpolationAlgorithm": "LAGRANGE",
                            "interpolationDegree": 5,
                            "referenceFrame": "INERTIAL",
                            "epoch": "2012-03-15T10:00:00Z",
                            "cartesian": [
                            0,
                            4650397.56551457,
                            -3390535.52275848,
                            -4087729.48877329
                            ]
                        })"_json;
    templat["position"]["cartesian"] = positions;
    templat["position"]["epoch"] = epoch;
    templat["label"]["text"] = name;
    if (isDebris)
        templat["point"]["color"]["rgba"] = {128, 128, 128, 255};
    else
        templat["point"]["color"]["rgba"] = {0, 255, 0, 255};

    return templat;
}
