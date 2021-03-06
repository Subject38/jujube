#include "Marker.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace fs = ghc::filesystem;

namespace Resources {

    void from_json(const nlohmann::json& j, Marker& m) {
        j.at("name").get_to(m.name);
        j.at("fps").get_to(m.fps);
        j.at("size").get_to(m.size);
        j.at("approach").get_to(m.approach);
        j.at("miss").get_to(m.miss);
        j.at("poor").get_to(m.poor);
        j.at("good").get_to(m.good);
        j.at("great").get_to(m.great);
        j.at("perfect").get_to(m.perfect);
    }
    
    Marker::Marker(const fs::path& marker_folder) :
        folder(marker_folder)
    {
        if (not fs::is_directory(folder)) {
            throw std::invalid_argument(folder.string()+" is not a folder");
        }
        if (not fs::exists(folder/"marker.json")) {
            throw std::invalid_argument("Marker folder ( "+folder.string()+" ) has no marker.json file");
        }
        std::ifstream marker_json{folder/"marker.json"};
        nlohmann::json j;
        marker_json >> j;
        j.get_to(*this);
        approach.load_and_check(folder, size, fps, {16, 30});
        miss.load_and_check(folder, size, fps, {16, 30});
        poor.load_and_check(folder, size, fps, {16, 30});
        good.load_and_check(folder, size, fps, {16, 30});
        great.load_and_check(folder, size, fps, {16, 30});
        perfect.load_and_check(folder, size, fps, {16, 30});
    }

    const SpriteSheet& Marker::get_sprite_sheet_from_enum(const MarkerAnimation& state) const {
        switch (state) {
        case MarkerAnimation::APPROACH:
            return approach;
            break;
        case MarkerAnimation::MISS:
            return miss;
            break;
        case MarkerAnimation::POOR:
            return poor;
            break;
        case MarkerAnimation::GOOD:
            return good;
            break;
        case MarkerAnimation::GREAT:
            return great;
            break;
        case MarkerAnimation::PERFECT:
            return perfect;
            break;
        default:
            throw std::runtime_error("wtf ?");
        }
    }


    std::optional<sf::Sprite> Marker::get_sprite(const MarkerAnimation& state, const sf::Time delta) const {
        return get_sprite(state, delta.asSeconds());
    }

    std::optional<sf::Sprite> Marker::get_sprite(const MarkerAnimation& state, const float seconds) const {
        auto raw_frame = static_cast<int>(std::floor(seconds*fps));
        if (raw_frame >= 0) {
            if (state == MarkerAnimation::APPROACH) {
                return get_sprite(MarkerAnimation::MISS, static_cast<std::size_t>(raw_frame));
            } else {
                return get_sprite(state, static_cast<std::size_t>(raw_frame));
            }
        } else {
            return get_sprite(
                MarkerAnimation::APPROACH,
                static_cast<std::size_t>(raw_frame+static_cast<int>(approach.count))
            );
        }
    }

    std::optional<sf::Sprite> Marker::get_sprite(const MarkerAnimation& state, const std::size_t frame) const {
        auto& sprite_sheet = get_sprite_sheet_from_enum(state);
        return sprite_sheet.get_sprite(frame, size);
    }

    Markers::Markers(const fs::path& jujube_path) {
        load_from_folder(jujube_path/"markers"/"tap");
        load_from_folder(jujube_path/"assets"/"markers"/"tap");
        if (empty()) {
            throw std::runtime_error("No tap note markers found in marker folder, jujube needs at least one to operate");
        }
    }

    void Markers::load_from_folder(const fs::path& markers_folder) {
        if (fs::exists(markers_folder)) {
            for (auto& p : fs::directory_iterator(markers_folder)) {
                if (p.is_directory()) {
                    try {
                        Marker m{p.path()};
                        emplace(m.name, m);
                    } catch (const std::exception& e) {
                        std::cerr << "Unable to load marker folder "
                        << p.path().filename().string() << " : "
                        << e.what() << '\n';
                    }
                }
            }
        }
    }
}
