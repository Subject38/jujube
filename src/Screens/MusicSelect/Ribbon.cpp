#include "Ribbon.hpp"

#include <cstdlib>
#include <map>
#include <vector>

#include "Panel.hpp"
#include "../../Toolkit/QuickRNG.hpp"

MusicSelect::MoveAnimation::MoveAnimation(unsigned int previous_pos, unsigned int next_pos, size_t ribbon_size, Direction direction) :
    normalized_to_pos(create_transform(previous_pos, next_pos, ribbon_size, direction)),
    seconds_to_normalized(0.f, 0.3f, 0.f, 1.f),
    clock(),
    ease_expo(-7.f)
{

}

Toolkit::AffineTransform<float> MusicSelect::MoveAnimation::create_transform(unsigned int previous_pos, unsigned int next_pos, size_t ribbon_size, Direction direction) {
    // We first deal with cases were we cross the end of the ribbon
    if (direction == Direction::Left and next_pos > previous_pos) {
        return Toolkit::AffineTransform<float>(
            0.f,
            1.f,
            static_cast<float>(previous_pos),
            static_cast<float>(next_pos-ribbon_size)
        );
    } else if (direction == Direction::Right and next_pos < previous_pos) {
        return Toolkit::AffineTransform<float>(
            0.f,
            1.f,
            static_cast<float>(previous_pos),
            static_cast<float>(next_pos+ribbon_size)
        );
    } else  {
        return Toolkit::AffineTransform<float>(
            0.f,
            1.f,
            static_cast<float>(previous_pos),
            static_cast<float>(next_pos)
        );
    }
}

float MusicSelect::MoveAnimation::get_position() {
    return normalized_to_pos.transform(
        ease_expo.transform(
            seconds_to_normalized.clampedTransform(
                clock.getElapsedTime().asSeconds()
            )
        )
    );
}

bool MusicSelect::MoveAnimation::ended() {
    return clock.getElapsedTime() > sf::milliseconds(300);
}

void MusicSelect::Ribbon::title_sort(const Data::SongList& song_list) {
    std::vector<std::reference_wrapper<const Data::Song>> songs;
    for (auto &&song : song_list.songs) {
        songs.push_back(std::cref(song));
    }
    
    (song_list.songs.begin(), song_list.songs.end());
    std::sort(songs.begin(), songs.end(), Data::Song::sort_by_title);
    std::map<std::string,std::vector<std::shared_ptr<Panel>>> categories;
    for (const auto& song : songs) {
        if (song.get().title.size() > 0) {
            char letter = song.get().title[0];
            if ('A' <= letter and letter <= 'Z') {
                categories
                    [std::string(1, letter)]
                    .push_back(
                        std::make_shared<SongPanel>(song)
                    );
            } else if ('a' <= letter and letter <= 'z') {
                categories
                    [std::string(1, 'A' + (letter - 'a'))]
                    .push_back(
                        std::make_shared<SongPanel>(song)
                    );
            } else {
                categories["?"].push_back(std::make_shared<SongPanel>(song));
            }
        } else {
            categories["?"].push_back(std::make_shared<SongPanel>(song));
        }
    }
    layout_from_category_map(categories);
}

void MusicSelect::Ribbon::test_sort() {
    layout.clear();
    layout.push_back(
        {
            std::make_shared<EmptyPanel>(),
            std::make_shared<CategoryPanel>("A"),
            std::make_shared<CategoryPanel>("truc")
        }
    );
    for (size_t i = 0; i < 3; i++) {
        layout.push_back(
            {
                std::make_shared<EmptyPanel>(),
                std::make_shared<EmptyPanel>(),
                std::make_shared<EmptyPanel>()
            }
        );
    }
}

void MusicSelect::Ribbon::test2_sort() {
    std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::map<std::string, std::vector<std::shared_ptr<Panel>>> categories;
    Toolkit::UniformIntRNG category_size_generator{1,10};
    Toolkit::UniformIntRNG panel_hue_generator{0,255};
    for (auto &&letter : alphabet) {
        auto category_size = category_size_generator.generate();
        for (int i = 0; i < category_size; i++) {
            categories[std::string(1, letter)].push_back(
                std::make_shared<MusicSelect::ColorPanel>(
                    sf::Color(
                        panel_hue_generator.generate(),
                        panel_hue_generator.generate(),
                        panel_hue_generator.generate()
                    )
                )
            );
        }
    }
    layout_from_category_map(categories);
}

const std::shared_ptr<MusicSelect::Panel>& MusicSelect::Ribbon::at(unsigned int button_index) const {
    return (
        layout
        .at((position + (button_index % 4)) % layout.size())
        .at(button_index / 4)
    );
}

void MusicSelect::Ribbon::layout_from_category_map(const std::map<std::string, std::vector<std::shared_ptr<MusicSelect::Panel>>>& categories) {
    layout.clear();
    for (auto &&[category, panels] : categories) {
        if (not panels.empty()) {
            std::vector<std::shared_ptr<Panel>> current_column;
            current_column.push_back(std::make_shared<CategoryPanel>(category));
            for (auto &&panel : panels) {
                if (current_column.size() == 3) {
                    layout.push_back({current_column[0], current_column[1], current_column[2]});
                    current_column.clear();
                } else {
                    current_column.push_back(std::move(panel));
                }
            }
            if (not current_column.empty()) {
                while (current_column.size() < 3) {
                    current_column.push_back(std::make_shared<EmptyPanel>());
                }
                layout.push_back({current_column[0], current_column[1], current_column[2]});
            }
        }
    }
}

void MusicSelect::Ribbon::move_right() {
    move_animation.emplace(position, position + 1, layout.size(), Direction::Right);
    position = (position + 1) % layout.size();
}

void MusicSelect::Ribbon::move_left() {
    move_animation.emplace(position, static_cast<int>(position)-1, layout.size(), Direction::Left);
    if (position == 0) {
        position = layout.size() - 1;
    } else {
        position--;
    }
}

void MusicSelect::Ribbon::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (move_animation) {
        if (not move_animation->ended()) {
            return draw_with_animation(target, states);
        } else {
            move_animation.reset();
        }
    }
    draw_without_animation(target, states);
}

void MusicSelect::Ribbon::draw_with_animation(sf::RenderTarget& target, sf::RenderStates states) const {
    auto float_position = move_animation->get_position();
    unsigned int column_zero = (static_cast<int>(float_position) + layout.size()) % layout.size();
    for (int column_offset = -1; column_offset <= 4; column_offset++) {
        unsigned int actual_column = (column_zero + column_offset + layout.size()) % layout.size();
        for (int row = 0; row < 3; row++) {
            layout.at(actual_column).at(row)->draw(
                resources,
                target,
                sf::FloatRect(
                    (static_cast<float>(column_zero + column_offset)-float_position)*150.f,
                    row*150.f,
                    150.f,
                    150.f
                )
            );
        }
    }
}

void MusicSelect::Ribbon::draw_without_animation(sf::RenderTarget& target, sf::RenderStates states) const {
    for (int column = -1; column <= 4; column++) {
        int actual_column_index = (column + position + layout.size()) % layout.size();
        for (int row = 0; row < 3; row++) {
            layout.at(actual_column_index).at(row)->draw(
                resources,
                target,
                sf::FloatRect(
                    column*150.f,
                    row*150.f,
                    150.f,
                    150.f
                )
            );
        }
    }
}
