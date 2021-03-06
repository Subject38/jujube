#include "PanelLayout.hpp"

#include "Panels/Panel.hpp"

namespace MusicSelect {
    PanelLayout::PanelLayout(
        const std::map<std::string,std::vector<std::shared_ptr<Panel>>>& categories,
        ScreenResources& t_resources
    ) {
        for (auto &&[category, panels] : categories) {
            if (not panels.empty()) {
                std::vector<std::shared_ptr<Panel>> current_column;
                current_column.emplace_back(std::make_shared<CategoryPanel>(t_resources, category));
                for (auto& panel : panels) {
                    if (current_column.size() == 3) {
                        push_back({current_column[0],current_column[1],current_column[2]});
                        current_column.clear();
                    }
                    current_column.push_back(panel);
                }
                if (not current_column.empty()) {
                    while (current_column.size() < 3) {
                        current_column.emplace_back(std::make_shared<EmptyPanel>(t_resources));
                    }
                    push_back({current_column[0],current_column[1],current_column[2]});
                }
            }
        }
        fill_layout(t_resources);
    }

    PanelLayout::PanelLayout(
        const std::vector<std::shared_ptr<Panel>>& panels,
        ScreenResources& t_resources
    ) {
        std::vector<std::shared_ptr<Panel>> current_column;
        for (auto& panel : panels) {
            if (current_column.size() == 3) {
                push_back({current_column[0],current_column[1],current_column[2]});
                current_column.clear();
            }
            current_column.push_back(panel);
        }
        if (not current_column.empty()) {
            while (current_column.size() < 3) {
                current_column.emplace_back(std::make_shared<EmptyPanel>(t_resources));
            }
            push_back({current_column[0],current_column[1],current_column[2]});
        }
        fill_layout(t_resources);
    }

    PanelLayout PanelLayout::red_empty_layout(ScreenResources& t_resources) {
        std::vector<std::shared_ptr<Panel>> panels;
        for (size_t i = 0; i < 3*4; i++) {
            panels.emplace_back(std::make_shared<ColoredMessagePanel>(t_resources, sf::Color::Red, "- EMPTY -"));
        }
        return PanelLayout{panels, t_resources};
    }

    PanelLayout PanelLayout::title_sort(const Data::SongList& song_list, ScreenResources& t_resources) {
        std::vector<std::shared_ptr<const Data::Song>> songs;
        for (auto &&song : song_list.songs) {
            songs.push_back(song);
        }
        std::sort(
            songs.begin(),
            songs.end(),
            [](std::shared_ptr<const Data::Song> a, std::shared_ptr<const Data::Song> b){return Data::Song::sort_by_title(*a, *b);}
        );
        std::map<std::string, std::vector<std::shared_ptr<Panel>>> categories;
        for (const auto &song : songs) {
            if (song->title.size() > 0) {
                char letter = song->title[0];
                if ('A' <= letter and letter <= 'Z') {
                    categories
                    [std::string(1, letter)]
                    .emplace_back(std::make_shared<SongPanel>(t_resources, song));
                } else if ('a' <= letter and letter <= 'z') {
                    categories
                    [std::string(1, 'A' + (letter - 'a'))]
                    .emplace_back(std::make_shared<SongPanel>(t_resources, song));
                } else {
                    categories["?"].emplace_back(std::make_shared<SongPanel>(t_resources, song));
                }
            } else {
                categories["?"].emplace_back(std::make_shared<SongPanel>(t_resources, song));
            }
        }
        return PanelLayout{categories, t_resources};
    }
    
    void PanelLayout::fill_layout(ScreenResources& t_resources) {
        while (size() < 4) {
            push_back({
                std::make_shared<EmptyPanel>(t_resources),
                std::make_shared<EmptyPanel>(t_resources),
                std::make_shared<EmptyPanel>(t_resources)
            });
        }
    }
}
