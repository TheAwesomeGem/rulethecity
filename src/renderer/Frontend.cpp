#include <unordered_map>
#include <format>
#include "frontend.h"
#include "render_util.h"
#include "screen.h"
#include "random.h"


void Frontend::draw_cells(sf::RenderWindow& window, const StageRenderData& render) const {
    for (int y = 0; y < render.cell_row_count; ++y) {
        for (int x = 0; x < render.cell_column_count; ++x) {
            CellType cell_type = render.cell_at(x, y);
            Point cell_screen_pos = render.get_cell_screen_pos(x, y);

            switch (cell_type) {
                case CellType::Entity: {
                    DrawTopLeftCircle(window, cell_screen_pos.x, cell_screen_pos.y, CELL_RADIUS, sf::Color::Yellow);
                    break;
                }

                case CellType::None: {
                    DrawTopLeftCircleWireframe(window, cell_screen_pos.x, cell_screen_pos.y, CELL_RADIUS, sf::Color::Yellow);
                    break;
                }
            }
        }
    }
}

void Frontend::draw_stage(sf::RenderWindow& window, const StageRenderData& render) const {
    sf::RectangleShape border { sf::Vector2f { render.get_stage_width(), render.get_stage_height() } };
    border.setPosition(sf::Vector2f { (float) render.pos.x, (float) render.pos.y });
    border.setOutlineThickness(1.0F);
    border.setOutlineColor(sf::Color::Red);
    border.setFillColor(sf::Color { 255, 255, 255, 0 });

    window.draw(border);
    draw_cells(window, render);
}

std::optional<SelectionRenderData> Frontend::get_cell_selection(sf::Vector2<float> mouse_vec, const StageRenderData& stage_render) const {
    for (int y = 0; y < stage_render.cell_row_count; ++y) {
        for (int x = 0; x < stage_render.cell_column_count; ++x) {
            CellType cell_type = stage_render.cell_at(x, y);

            if (cell_type == CellType::None) {
                continue;
            }

            Point cell_screen_pos = stage_render.get_cell_screen_center_pos(x, y);

            if (CheckCollisionPointCircle(mouse_vec, cell_screen_pos, CELL_RADIUS)) {
                return std::make_optional<SelectionRenderData>(stage_render.type, cell_type, Point{x, y}, CELL_RADIUS, cell_screen_pos);
            }
        }
    }

    return std::nullopt;
}

std::optional<Point> Frontend::get_empty_cell(const StageRenderData& stage_render, Point screen_pos) const {
    Point pos = screen_pos;
    pos.x += CELL_RADIUS;
    pos.y += CELL_RADIUS;

    for (int y = 0; y < stage_render.cell_row_count; ++y) {
        for (int x = 0; x < stage_render.cell_column_count; ++x) {
            CellType cell_type = stage_render.cell_at(x, y);

            if (cell_type == CellType::Entity) {
                continue;
            }

            Point cell_screen_pos = stage_render.get_cell_screen_center_pos(x, y);

            if (CheckCollisionCircles(cell_screen_pos, CELL_RADIUS, pos, CELL_RADIUS)) {
                return Point{x, y};
            }
        }
    }

    return std::nullopt;
}

void Frontend::init() {
    // TODO: All of these should be based off game_state and needs to be queried in the update stage(perhaps we can think about optimizations in the future)

    this->renders.emplace(StageRenderType::Main, StageRenderData{StageRenderType::Main,
                                                                 11,
                                                                 5,
                                                                 16.0F});
    this->renders.emplace(StageRenderType::Reserve, StageRenderData{StageRenderType::Reserve,
                                                                    6,
                                                                    1,
                                                                    16.0F
    });

    StageRenderData& main_stage_render = this->renders.at(StageRenderType::Main);
    int main_stage_x = (SCREEN_WIDTH * 0.5F) - (main_stage_render.get_stage_width() * 0.5F);
    int main_stage_y = (SCREEN_HEIGHT * 0.5F) - (main_stage_render.get_stage_height() * 0.5F);
    main_stage_render.pos = Point{main_stage_x, main_stage_y};

    StageRenderData& reserve_stage_render = this->renders.at(StageRenderType::Reserve);
    int reserve_stage_x = (SCREEN_WIDTH * 0.5F) - (reserve_stage_render.get_stage_width() * 0.5F);
    int reserve_stage_y = main_stage_y + main_stage_render.get_stage_height() + 20.0F;
    reserve_stage_render.pos = Point{reserve_stage_x, reserve_stage_y};
}

void Frontend::event() {
    this->debug_mouse_pos = "X: Unknown, Y: Unknown";

    // Event
    if (!this->has_left_clicked && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        this->has_left_clicked = true;
        // User just pressed left click

        // Selection System
        if (!this->selected_cell.has_value()) {
            Vector2 mouse_vec = Vector2{(float) GetMouseX(), (float) GetMouseY()};

            for (auto& [_, stage_render]: renders) {
                if (!CheckCollisionPointRec(mouse_vec, stage_render.get_rec())) {
                    continue;
                }

                this->selected_cell = get_cell_selection(mouse_vec, stage_render);

                if (!this->selected_cell.has_value()) {
                    continue;
                }

                EventData event = cell_event(EventType::CELL_SELECTED, this->selected_cell->cell_pos, std::nullopt);
                if (respond(event)) {
                    stage_render.cell_at(this->selected_cell->cell_pos.x, this->selected_cell->cell_pos.y) = CellType::None;
                    notify(event);
                } else {
                    this->selected_cell = std::nullopt;
                }

                break;
            }
        }

        // =========
    } else if (this->has_left_clicked && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        this->has_left_clicked = false;
        // User just released left click

        // Drop System
        if (this->selected_cell.has_value()) {
            bool dropped = false;

            for (auto& [_, stage_render]: this->renders) {
                if (!CheckCollisionPointRec(this->selected_cell->screen_pos.vec(), stage_render.get_rec())) {
                    continue;
                }

                std::optional<Point> cell_pos = get_empty_cell(stage_render, this->selected_cell->screen_pos);

                if (cell_pos.has_value()) {
                    if (respond(cell_event(EventType::CELL_DROP, this->selected_cell->cell_pos, cell_pos))) {
                        stage_render.cell_at(cell_pos->x, cell_pos->y) = CellType::Entity;
                        this->selected_cell.reset();
                        dropped = true;
                    }

                    break;
                }
            }

            if (!dropped) {
                this->renders.at(this->selected_cell->stage_type).cell_at(this->selected_cell->cell_pos.x, this->selected_cell->cell_pos.y) = CellType::Entity;
                this->selected_cell.reset();
            }
        }
    }
}

void Frontend::update() {
    for (size_t i = 0; i < game.people.size(); ++i) {
        // TODO: This should only be people that are in the reserves
        const Person& person = game.people[i];
        StageRenderData& reserve_stage = renders.at(StageRenderType::Reserve);
        reserve_stage.cells[i] = CellType::Entity;
    }

    // Drag System
    if (this->selected_cell.has_value()) {
        this->selected_cell->screen_pos.x = GetMouseX() - CELL_RADIUS;
        this->selected_cell->screen_pos.y = GetMouseY() - CELL_RADIUS;
    }

    for (auto& [_, stage_render]: this->renders) {
        Point screen_pos = Point{GetMouseX(), GetMouseY()};

        if (!CheckCollisionPointRec(screen_pos.vec(), stage_render.get_rec())) {
            continue;
        }

        Point cell_pos = stage_render.get_screen_cell_pos(screen_pos.x, screen_pos.y);
        this->debug_mouse_pos = std::format("X: {}, Y: {}", cell_pos.x, cell_pos.y);
    }
}

void Frontend::draw(sf::RenderWindow& window) {
    // TODO: Add center decorator and other layout decorators?

    for (const auto& [_, stage_render]: this->renders) {
        draw_stage(stage_render);
    }

    if (this->selected_cell.has_value()) {
        DrawTopLeftCircle(this->selected_cell->screen_pos.x, this->selected_cell->screen_pos.y, CELL_RADIUS, ORANGE);
    }

    DrawText(this->debug_mouse_pos.c_str(), 400, 20, 20, ORANGE);
}

SubscriberId Frontend::subscribe() {
    SubscriberId id = Random::uuid_rng();
    this->subscribers.emplace(id, SubscriberData{});

    return id;
}

SubscriberData& Frontend::subscription(SubscriberId id) {
    return this->subscribers.at(id);
}

void Frontend::notify(const EventData& event) {
    for (auto& [_, subscriber]: subscribers) {
        subscriber.queueable_events.push_back(event);
    }
}

bool Frontend::respond(const EventData& event) {
    for (auto& [_, subscriber]: subscribers) {
        if (!subscriber.priority_event(event)) {
            return false;
        }
    }

    return true;
}

EventData Frontend::cell_event(EventType type, std::optional<Point> from_cell_pos, std::optional<Point> to_cell_pos) {
    EventData event{type};
    event.from_cell_pos = from_cell_pos;
    event.to_cell_pos = to_cell_pos;

    return event;
}

