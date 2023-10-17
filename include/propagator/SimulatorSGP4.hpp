#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <unordered_map>
#include "date.h"

#include <perturb/perturb.hpp>
#include <czml.hpp>

#include "SHA256.h"

// #include "SimulatorBase.hpp"

/*
std::ostream &
operator<<(std::ostream &os, const std::array<double, 3> &a)
{
    os << a[0] << " " << a[1] << " " << a[2];
    return os;
}

std::array<double, 3> operator-(const std::array<double, 3> a, const std::array<double, 3> b)
{
    return {a[0] - b[0], a[1] - b[1], a[2] - b[2]};
}

*/
struct CDM_Cesium
{
    std::string time_string;
    std::vector<double> position;
};

struct CollisionDetector
{
    // Given a current position vector, quantize and fill bins to detect collisions.

    double dot(const std::array<double, 3> &a, const std::array<double, 3> &b)
    {
        return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
    }

    std::array<double, 3> normalize(const std::array<double, 3> &v)
    {
        double divisor = dot(v, v);
        return {v[0] / divisor, v[1] / divisor, v[2] / divisor};
    }

    double rel_vel(const perturb::StateVector &a, const perturb::StateVector &b)
    {
        perturb::Vec3 diff({a.velocity[0] - b.velocity[0],
                            a.velocity[1] - b.velocity[1],
                            a.velocity[2] - b.velocity[2]});
        return std::sqrt(dot(diff, diff));
    }

    // double distance(const perturb::StateVector &a, const perturb::StateVector &b)
    // {
    //     auto diff = a.position - b.position;
    //     return std::sqrt(dot(diff, diff));
    // }

    double distance(const perturb::StateVector &a, const perturb::StateVector &b)
    {
        return std::sqrt(
            (a.position[0] - b.position[0]) * (a.position[0] - b.position[0]) +
            (a.position[1] - b.position[1]) * (a.position[1] - b.position[1]) +
            (a.position[2] - b.position[2]) * (a.position[2] - b.position[2]));
    }

    double angle(const perturb::StateVector &a, const perturb::StateVector &b)
    {
        auto a_n = normalize(a.velocity);
        auto b_n = normalize(b.velocity);

        return std::acos(dot(a_n, b_n));
    }

    bool run(const std::vector<perturb::StateVector> &positions,
             const std::vector<perturb::Satellite> &satellites,
             const std::string &start_time,
             const std::string &end_time,
             std::vector<CDM> &collisions,
             std::vector<CDM_Cesium> &cdm_cesium)
    {
        for (int i = 0; i < positions.size(); i++)
        {
            auto s = positions[i];
            auto d = std::sqrt(s.position[0] * s.position[0] + s.position[0] * s.position[0] + s.position[0] * s.position[0]);
            uint64_t x = int(std::abs(s.position[0])) / 10;
            uint64_t y = int(std::abs(s.position[1])) / 10;
            uint64_t z = int(std::abs(s.position[2])) / 10;

            uint64_t idx = (x << 32) + (y << 16) + z;

            position_bins[idx].push_back(i);
        }

        for (const auto &bin : position_bins)
        {

            if (bin.second.size() < 2)
                continue;
            // std::cerr << bin.first << " : ";

            for (size_t i = 0; i < bin.second.size(); i++)
            {
                auto p = bin.second[i];
                for (size_t j = i + 1; j < bin.second.size(); j++)
                {
                    auto q = bin.second[j];
                    double dist = distance(positions[p], positions[q]);
                    double angle_rads = M_PI / 180 * 15;
                    if (dist > 5 or rel_vel(positions[p], positions[q]) < 10)
                        continue;
                    // std::cerr << "(" << p << "," << q << "," << dist << ", " << angle(positions[p], positions[q]) << ")" << std::endl;

                    CDM msg;
                    msg.sat1_satnumber = satellites[p].sat_rec.satnum;
                    msg.sat2_satnumber = satellites[q].sat_rec.satnum;
                    msg.relative_velocity = rel_vel(positions[p], positions[q]);
                    msg.min_distance = dist;
                    msg.TimeClosestApproach = start_time;

                    if (collisions.empty())
                    {
                        std::cerr << "\tFOUND CDM "
                                  << msg.TimeClosestApproach << "/" << end_time << " "
                                  << msg.sat1_satnumber << " "
                                  << msg.sat2_satnumber << " "
                                  << msg.min_distance << " "
                                  << msg.relative_velocity << std::endl;

                        collisions.push_back(msg);

                        CDM_Cesium pt;
                        pt.time_string = msg.TimeClosestApproach + "/" + end_time;
                        for (int idx = 0; idx < 3; idx++)
                            pt.position.push_back(positions[p].position[idx]);

                        cdm_cesium.push_back(pt);
                    }
                    else
                    {
                        auto &m_last = collisions.back();
                        if (msg.sat1_satnumber == m_last.sat1_satnumber and msg.sat2_satnumber == m_last.sat2_satnumber)
                        {
                            if (msg.min_distance < m_last.min_distance)
                            {

                                m_last.min_distance = msg.min_distance;
                                m_last.relative_velocity = msg.relative_velocity;
                                m_last.TimeClosestApproach = msg.TimeClosestApproach;

                                msg = m_last;
                                std::cerr << "\tUPDATED CDM (closer) "
                                          << msg.TimeClosestApproach << " "
                                          << msg.sat1_satnumber << " "
                                          << msg.sat2_satnumber << " "
                                          << msg.min_distance << " "
                                          << msg.relative_velocity << std::endl;
                            }
                        }
                        else
                        {
                            std::cerr << "\tFOUND CDM "
                                      << msg.TimeClosestApproach << "/" << end_time << " "
                                      << msg.sat1_satnumber << " "
                                      << msg.sat2_satnumber << " "
                                      << msg.min_distance << " "
                                      << msg.relative_velocity << std::endl;
                            CDM_Cesium pt;
                            pt.time_string = msg.TimeClosestApproach + "/" + end_time;
                            for (int idx = 0; idx < 3; idx++)
                                pt.position.push_back(positions[p].position[idx]);

                            cdm_cesium.push_back(pt);
                            collisions.push_back(msg);
                        }
                    }
                }

                // std::cerr << std::endl;
            }
        }

        return !collisions.empty();
    }

    std::unordered_map<uint64_t, std::vector<uint32_t>> position_bins;
};

struct SimulatorSGP4
{

    perturb::DateTime generate_date_time(std::chrono::time_point<std::chrono::system_clock> start_time)
    {
        auto dp = date::floor<date::days>(start_time);
        auto ymd = date::year_month_day{dp};
        auto time = date::make_time(std::chrono::duration_cast<std::chrono::milliseconds>(start_time - dp));
        const auto t = perturb::JulianDate(perturb::DateTime{(int)ymd.year(),
                                                             (unsigned int)ymd.month(),
                                                             (unsigned int)ymd.day(),
                                                             (int)time.hours().count(),
                                                             (int)time.minutes().count(),
                                                             (float)time.seconds().count() + time.subseconds().count() / 1000.});

        return t.to_datetime();
    }

    SimulatorSGP4(const char *filename)
    {
        std::ifstream in(filename);

        char name[80];
        char line1[80];
        char line2[80];

        std::vector<int> names;

        while (in.is_open() and in.good())
        {
            in.getline(name, sizeof(name));
            in.getline(line1, sizeof(line1));
            in.getline(line2, sizeof(line2));

            perturb::StateVector v;

            if (in.good())
            {
                auto s = perturb::Satellite::from_tle(line1, line2);

                for (auto n : names)
                {
                    if (n == atoi(s.sat_rec.satnum))
                    {
                        continue;
                    }
                }

                // std::cerr << s.sat_rec.classification << std::endl;
                // if (std::string(name).find("DEB") == std::string::npos)

                //     colors.push_back(COLOR{0, 0, 1});
                // else
                //     colors.push_back(COLOR{0.5, 0.5, 0.5});

                // s.propagate_from_epoch(0, v);
                // const auto &pos = v.position;

                // auto d = std::sqrt(pos[0] * pos[0] + pos[1] * pos[1] + pos[2] * pos[2]);

                // if (d > radius_of_earth_km + 500)
                //     continue;

                if (s.last_error() == perturb::Sgp4Error::NONE)
                {
                    if (std::string(name).find("DEB") == std::string::npos)
                        s.sat_rec.classification = 'D';
                    satellites.push_back(s);
                    names.push_back(atoi(s.sat_rec.satnum));
                    sat_names.push_back((const char *)&name[2]);
                }

                if (satellites.size() == 5000)
                    break;
            }
        }

        in.close();

        states.resize(satellites.size());

        int numberOfOrbits = satellites.size();
        start_time = std::chrono::system_clock::now();
        epoch_time_start = start_time;

        auto dp = date::floor<date::days>(start_time);
        auto ymd = date::year_month_day{dp};
        auto time = date::make_time(std::chrono::duration_cast<std::chrono::milliseconds>(start_time - dp));

        // std::cout << "year        = " << ymd.year() << '\n';
        // std::cout << "month       = " << ymd.month() << '\n';
        // std::cout << "day         = " << ymd.day() << '\n';
        // std::cout << "hour        = " << time.hours().count() << "h\n";
        // std::cout << "minute      = " << time.minutes().count() << "min\n";
        // std::cout << "second      = " << time.seconds().count() << "s\n";
        // std::cout << "millisecond = " << time.subseconds().count() << "ms\n";

        const auto t = perturb::JulianDate(perturb::DateTime{(int)ymd.year(),
                                                             (unsigned int)ymd.month(),
                                                             (unsigned int)ymd.day(),
                                                             (int)time.hours().count(),
                                                             (int)time.minutes().count(),
                                                             (float)time.seconds().count() + time.subseconds().count() / 1000.});

        dt = t.to_datetime();
        epoch = this->displayTime();
    }

    std::string displayTime(int interval = 0)
    {
        char buffer[64];

        int seconds = dt.sec;
        int mins = dt.min;

        seconds = seconds + interval;
        mins = mins + seconds / 60;
        seconds = seconds % 60;

        if (mins == 60)
        {
            mins = 59;
            seconds = 59;
        }

        sprintf(buffer, "%04d-%02d-%02dT%02d:%02d:%02dZ",
                dt.year,
                dt.month,
                dt.day,
                dt.hour,
                mins, int(seconds));
        return std::string(buffer);
    }

    virtual std::string getTime()
    {
        char buffer[64];
        sprintf(buffer, "%04d:%02d:%02dT%02d:%02d:%02d.%02d",
                dt.year,
                dt.month,
                dt.day,
                dt.hour,
                dt.min, int(dt.sec), int(100 * (dt.sec - int(dt.sec))));
        return std::string(buffer);
    }

    virtual void draw(int point_size, int num_satellites)
    {
    }

    virtual void propagate(double delta_t, std::vector<CDM> &cdms)
    {
        start_time += std::chrono::milliseconds(int(1000 * delta_t));
        auto dp = date::floor<date::days>(start_time);
        auto ymd = date::year_month_day{dp};
        auto time = date::make_time(std::chrono::duration_cast<std::chrono::milliseconds>(start_time - dp));

        // std::cout << "year        = " << ymd.year() << '\n';
        // std::cout << "month       = " << ymd.month() << '\n';
        // std::cout << "day         = " << ymd.day() << '\n';
        // std::cout << "hour        = " << time.hours().count() << "h\n";
        // std::cout << "minute      = " << time.minutes().count() << "min\n";
        // std::cout << "second      = " << time.seconds().count() << "s\n";
        // std::cout << "millisecond = " << time.subseconds().count() << "ms\n";

        const auto t = perturb::JulianDate(perturb::DateTime{(int)ymd.year(),
                                                             (unsigned int)ymd.month(),
                                                             (unsigned int)ymd.day(),
                                                             (int)time.hours().count(),
                                                             (int)time.minutes().count(),
                                                             (float)time.seconds().count() + time.subseconds().count() / 1000.});

        dt = t.to_datetime();
        // std::cerr << dt.year << ":" << dt.month << ":" << dt.day << "T" << dt.hour << ":" << dt.min << ":" << dt.sec << std::endl;

        for (size_t i = 0; i < satellites.size(); i++)
        {
            if (satellites[i].last_error() == perturb::Sgp4Error::NONE)
            {
                satellites[i].propagate(t, states[i]);
            }
        }

        CollisionDetector detector;
        std::vector<std::pair<int, int>> collisions;

        detector.run(states, satellites, this->displayTime(0), this->displayTime(120), cdms, cdm_cesium);

        auto n_seconds_since_start = std::chrono::duration_cast<std::chrono::seconds>(start_time - epoch_time_start).count();
        bool shouldLog = n_seconds_since_start % (5 * 60) == 0;
        if (shouldLog)
        {
            std::cerr << this->displayTime() << " " << timestamps.size() << std::endl;
            timestamps.push_back(n_seconds_since_start);
            for (size_t i = 0; i < 5000; i++)
            {
                position_history[atoi(satellites[i].sat_rec.satnum)].push_back(states[i].position);
            }

            std::cerr << "CDMS # " << cdms.size() << std::endl;
        }

        if (timestamps.size() >= number_step_viz)
        {
            push_data_to_server();
            epoch = this->displayTime();
            timestamps.clear();
            position_history.clear();
            for (auto c : cdm_cesium)
            {
                std::cerr << c.time_string << " " << c.position[0] << " " << c.position[1] << " " << c.position[2] << std::endl;
            }
            cdm_cesium.clear();

            std::cin.get();
        }
    }

    void push_data_to_server()
    {
        std::ofstream out("test.czml");

        out << R"([{
            "id":"document",    
            "name":"simple",
            "version":"1.0",
            "clock":{
                "interval":")"
            << epoch << "/" << displayTime() << R"(",
                "currentTime":")"
            << epoch << R"(",
                "multiplier":50,
                "range":"LOOP_STOP",
                "step":"SYSTEM_CLOCK_MULTIPLIER"}
            },
            {
                "id":"9927edc4-e87a-4e1f-9b8b-0bfb3b05b227",
                "name":"Accesses",
                "description":"List of Accesses"
            },)";

        // std::vector<CZML::json> json_sats;

        for (auto c : cdm_cesium)
        {
            out << R"({"availability":")" << c.time_string
                << R"(","position" : {"cartesian" : [)" << c.position[0] * 1000 << "," << c.position[1] * 1000 << "," << c.position[2] * 1000 << R"(]},
                "point":{
                    "show" : [{"boolean" : true}],
                    "pixelSize" : 20,
                    "color":{"rgba" : [255, 0, 0, 255]} 
                    },
                  "label":{
                        "fillColor":{
                            "rgba":[
                            255,0,0,255
                            ]
                        },
                        "font":"11pt Lucida Console",
                        "horizontalOrigin":"LEFT",
                        "outlineColor":{
                            "rgba":[
                            0,0,0,255
                            ]
                        },
                        "outlineWidth":2,
                        "pixelOffset":{
                            "cartesian2":[
                            12,0
                            ]
                        },
                        "show":true,
                        "style":"FILL_AND_OUTLINE",
                        "text":")"
                << sha256(c.time_string) << R"(",
                        "verticalOrigin":"CENTER"
                        }
            },)";
        }

        size_t sz = 5000; // satellites.size(); // 4000;
        for (int i = 0; i < std::min(sz, satellites.size()); i++)
        {
            std::vector<double> positions_all_json;
            for (int j = 0; j < number_step_viz; j++)
            {
                positions_all_json.push_back(timestamps[j] - timestamps[0] + 1);
                int id = atoi(satellites[i].sat_rec.satnum);
                positions_all_json.push_back(position_history[id][j][0] * 1000);
                positions_all_json.push_back(position_history[id][j][1] * 1000);
                positions_all_json.push_back(position_history[id][j][2] * 1000);
            }

            auto sat_json = CZML::export_json(satellites[i].sat_rec.satnum,
                                              sat_names[i],
                                              positions_all_json,
                                              epoch,
                                              satellites[i].sat_rec.classification != 'D');
            out << sat_json.dump();
            if (i < std::min(satellites.size(), sz) - 1)
                out << " , " << std::endl;
            else
                out << "]";
        }
        out.close();
    }

    std::vector<perturb::Satellite>
        satellites;
    std::vector<perturb::StateVector> states;

    float seconds = 0;
    std::chrono::time_point<std::chrono::system_clock> start_time, epoch_time_start;
    // std::vector<std::pair<int, int>> collisions;
    std::vector<int> oh_no_these_collided;

    perturb::DateTime dt;

    std::map<int, std::vector<perturb::Vec3>> position_history;
    std::string epoch;
    std::vector<int> timestamps;
    std::vector<std::string> sat_names;

    int number_step_viz = 100;
    std::vector<CDM_Cesium> cdm_cesium;
};