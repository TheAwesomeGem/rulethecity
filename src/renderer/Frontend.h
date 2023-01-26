#pragma once

#include <optional>
#include <vector>
#include <string>
#include <deque>
#include <functional>
#include <queue>
#include "game.h"
#include <stduuid/uuid.h>


using SubscriberId = uuids::uuid;
using Point = sf::Vector2<int>;

static constexpr const float CELL_RADIUS = 26.0F;

constexpr float get_cell_size() {
    return CELL_RADIUS * 2.0F;
}

enum class CellType {
    Entity,

    None
};

enum StageRenderType {
    Main,
    Reserve
};

struct SelectionRenderData {
    SelectionRenderData(StageRenderType stage_type_, CellType cell_type_, Point cell_pos_, float cell_radius_, Point screen_pos_) : stage_type{stage_type_}, cell_type{cell_type_},
                                                                                                                                    cell_pos{cell_pos_},
                                                                                                                                    screen_pos{screen_pos_} {

    }

    StageRenderType stage_type;
    CellType cell_type;
    Point cell_pos;
    Point screen_pos;
};

// NOTE: Render data is used for both drawing and querying collision
struct StageRenderData {
    StageRenderData(StageRenderType type_, int cell_column_count_, int cell_row_count_, float cell_spacing_) : type{type_},
                                                                                                               cell_column_count{cell_column_count_},
                                                                                                               cell_row_count{cell_row_count_}, cell_spacing{cell_spacing_} {
        cells.resize(cell_column_count * cell_row_count);
        std::fill(cells.begin(), cells.end(), CellType::None);
    }

    StageRenderData(StageRenderData&& move) = default;

    StageRenderData(const StageRenderData& copy) = delete;

    StageRenderData& operator=(const StageRenderData& copy) = delete;

    StageRenderType type;
    Point pos;
    int cell_column_count;
    int cell_row_count;
    float cell_spacing;
    std::vector<CellType> cells;

    [[nodiscard]] float get_stage_width() const {
        return (cell_spacing * ((float) cell_column_count + 1.0F)) + (get_cell_size() * (float) this->cell_column_count);
    }

    [[nodiscard]] float get_stage_height() const {
        return (cell_spacing * ((float) cell_row_count + 1.0F)) + (get_cell_size() * (float) this->cell_row_count);
    }

    [[nodiscard]] Point get_cell_screen_pos(int cell_x, int cell_y) const {
        int center_x = (int) ((cell_spacing * ((float) cell_x + 1.0F)) + ((float) cell_x * get_cell_size()) + (float) this->pos.x);
        int center_y = (int) ((cell_spacing * ((float) cell_y + 1.0F)) + ((float) cell_y * get_cell_size()) + (float) this->pos.y);

        return Point{
                center_x,
                center_y
        };
    }

    [[nodiscard]] Point get_cell_screen_center_pos(int cell_x, int cell_y) const {
        Point screen_pos = get_cell_screen_pos(cell_x, cell_y);
        screen_pos.x += CELL_RADIUS;
        screen_pos.y += CELL_RADIUS;

        return screen_pos;
    }

    [[nodiscard]] Point get_screen_cell_pos(int screen_x, int screen_y) const {
        int cell_x = (int) ((float) ((float) screen_x - (float) pos.x - (float) cell_spacing) / (float) ((float) cell_spacing + (float) get_cell_size()));
        int cell_y = (int) ((float) ((float) screen_y - (float) pos.y - (float) cell_spacing) / (float) ((float) cell_spacing + (float) get_cell_size()));

        return Point{
                cell_x,
                cell_y
        };
    }

    [[nodiscard]] Point get_cell_screen_pos(int index) const {
        int cell_y = index / cell_column_count;
        int cell_x = index % cell_column_count;

        return get_cell_screen_pos(cell_x, cell_y);
    }

    [[nodiscard]] sf::Rect<float> get_rec() const {
        return sf::Rect<float>{
                (float) this->pos.x,
                (float) this->pos.y,
                get_stage_width(),
                get_stage_height()
        };
    }

    [[nodiscard]] CellType& cell_at(int cell_x, int cell_y) {
        return cells[cell_y * this->cell_column_count + cell_x];
    }

    [[nodiscard]] const CellType& cell_at(int cell_x, int cell_y) const {
        return cells[cell_y * this->cell_column_count + cell_x];
    }
};

enum class EventType {
    CELL_SELECTED,
    CELL_DROP
};

struct EventData {
    explicit EventData(EventType type_) : type{type_} {

    }

    EventData(const EventData& copy) : type{copy.type}, from_cell_pos{copy.from_cell_pos}, to_cell_pos{copy.to_cell_pos} {
        printf("EventData Copy Constructor.\n");
    }

    EventData& operator=(const EventData& copy) {
        this->type = copy.type;
        this->from_cell_pos = copy.from_cell_pos;
        this->to_cell_pos = copy.to_cell_pos;
        printf("EventData Copy Assignment.\n");

        return *this;
    }

    EventType type;
    std::optional<Point> from_cell_pos;
    std::optional<Point> to_cell_pos;
};

struct SubscriberData {
    std::deque<EventData> queueable_events;
    std::function<bool(const EventData&)> priority_event;
};

class Frontend {
public:
    explicit Frontend(const GameState& game_state) : game{game_state}, renders{}, has_left_clicked{false}, selected_cell{}, debug_mouse_pos{} {
    }

    void draw_cells(sf::RenderWindow& window, const StageRenderData& render) const;

    void draw_stage(sf::RenderWindow& window, const StageRenderData& render) const;

    [[nodiscard]] std::optional<SelectionRenderData> get_cell_selection(sf::Vector2<float> mouse_vec, const StageRenderData& stage_render) const;

    [[nodiscard]] std::optional<Point> get_empty_cell(const StageRenderData& stage_render, Point screen_pos) const;

    void init();

    void event();

    void update();

    void draw(sf::RenderWindow& window);

    SubscriberId subscribe();

    SubscriberData& subscription(SubscriberId id);

private:
    void notify(const EventData& event);

    bool respond(const EventData& event);

    EventData cell_event(EventType type, std::optional<Point> from_cell_pos, std::optional<Point> to_cell_pos);

private:
    std::unordered_map<StageRenderType, StageRenderData> renders;
    bool has_left_clicked;
    std::optional<SelectionRenderData> selected_cell;
    std::string debug_mouse_pos;
    std::unordered_map<SubscriberId, SubscriberData> subscribers; // TODO: This should be a priority queue

    const GameState& game;
};
