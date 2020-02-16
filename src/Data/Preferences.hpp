#pragma once

#include <cstddef>
#include <ghc/filesystem.hpp>
#include <fstream>

#include <cereal/archives/json.hpp>
#include <SFML/System.hpp>

namespace Data {
    // By convention all axis-independant lengths are expressed as a ratio of the screen WIDTH
    // see panel_size and panel_spacing for example
    struct Screen {
        std::size_t width = 768;
        std::size_t height = 1360;
        bool fullscreen = false;

        template<class Archive>
        void serialize(Archive & archive) {
            archive(
                CEREAL_NVP(width),
                CEREAL_NVP(height),
                CEREAL_NVP(fullscreen) 
            ); 
        }
    };

    struct Layout {
        float panel_size = 160.f / 768.f;
        float panel_spacing = (112.f / 3.f) / 768.f;
        float panel_step() const {return panel_size+panel_spacing;};
        float ribbon_x = 8.f / 768.f;
        float ribbon_y = 602.f / 768.f;
        float big_cover_size = 320.f / 768.f;
        float big_cover_x = 0.5f;
        float big_cover_y = 0.017f;
        float big_level_x = 656.f / 768.f;
        float big_level_y = 30.f / 768.f;
        float upper_part_height = 464.f / 768.f;

        template<class Archive>
        void serialize(Archive & archive) {
            archive(
                CEREAL_NVP(panel_size),
                CEREAL_NVP(panel_spacing),
                CEREAL_NVP(ribbon_x),
                CEREAL_NVP(ribbon_y),
                CEREAL_NVP(big_cover_size),
                CEREAL_NVP(upper_part_height)
            ); 
        }
    };

    // RAII style class which loads preferences from the dedicated file when constructed and saves them when destructed
    struct Preferences {
        Screen screen;
        Layout layout;

        Preferences() : screen(), layout() {
            load();
        }
        
        ~Preferences() {
            save();
        }

        void load() {
            auto path = ghc::filesystem::path("data/preferences.json");
            if (ghc::filesystem::exists(path)) {
                std::ifstream prefs_file;
                prefs_file.open(path);
                try {
                    cereal::JSONInputArchive archive{prefs_file};
                    serialize(archive);
                } catch(const std::exception& e) {
                    std::cerr << "Error while loading data/preferences.json : " << e.what() << std::endl;
                    std::cerr << "Using fallback preferences instead" << std::endl;
                }
            }
        };

        void save() {
            auto data_folder = ghc::filesystem::path("data");
            if (not ghc::filesystem::exists(data_folder)) {
                ghc::filesystem::create_directory(data_folder);
            }
            if (not ghc::filesystem::is_directory(data_folder)) {
                std::cerr << "Can't create data folder to save preferences, a file named 'data' exists" << std::endl;
            }
            std::ofstream preferences_file;
            preferences_file.open(data_folder / "preferences.json", std::ofstream::trunc | std::ofstream::out);
            {
                cereal::JSONOutputArchive archive{preferences_file};
                serialize(archive);
            }            
        };

        template<class Archive>
        void serialize(Archive & archive) {
            archive(
                CEREAL_NVP(screen),
                CEREAL_NVP(layout) 
            ); 
        }
    };
}
