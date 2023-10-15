#pragma once

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"

#include <fstream>
#include <cstdlib>
#include <fstream>

namespace SpaceTrackLiveData
{
    bool get_latest_tles(const std::string &filename)
    {
        const char *USER = std::getenv("USER");
        const char *PASS = std::getenv("PASS");

        if (!USER or !PASS)
        {
            std::cerr << "Couldn't read PASS and USER from environment while trying to get data from space-track.org" << std::endl;
            std::cerr << " Pleass user export USER=username and PASS=yourpassword for space-track credentials" << std::endl;
            return false;
        }

        httplib::Client client("https://www.space-track.org");
        std::string query("https://www.space-track.org/basicspacedata/query/class/gp/EPOCH/%3Enow-30/orderby/NORAD_CAT_ID,EPOCH/format/3le");
        std::string full_command = "identity=" + std::string(USER) + "&password=" + std::string(PASS) + "&query=" + query;

        auto res = client.Post("/ajaxauth/login", full_command, "application/x-www-form-urlencoded");

        if (res->status == 200)
        {
            std::string data = res->body;
            std::cerr << "Successfully read data from space-track.org" << std::endl;
            std::ofstream out(filename);
            out << data;
            out.close();
            std::cerr << "Written to " << filename << std::endl;
            return true;
        }

        std::cerr << "Query failed with error " << res->status << std::endl;
        return false;
    }
};
