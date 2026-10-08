#include "../ui.h"

namespace celosia::animations::functions {
    float calc_linear(float current_value, const float goal_value, float speed) {
        ImGuiIO* io = celosia::render::io;
        speed += animations::base_speed;

        if (speed < 0)
            speed = 0;

        if (current_value == goal_value) return goal_value;

        if (current_value > goal_value) {
            current_value -= speed * io->DeltaTime;
            if (current_value <= goal_value)
                return goal_value;
        }
        else if (current_value < goal_value)
        {
            current_value += speed * io->DeltaTime;
            if (current_value >= goal_value)
                return goal_value;
        }

        return current_value;
    }

    float calc_smooth(float current_value, const float goal_value, float speed) {
        ImGuiIO* io = celosia::render::io;
        speed += base_speed;

        if (speed < 0)
            speed = 0;

        if ((current_value == goal_value) ||
            (current_value + 0.11f >= goal_value && current_value < goal_value) ||
            (current_value - 0.11f <= goal_value && current_value > goal_value)) // this works with any values pretty sure
            return goal_value;

        current_value += (goal_value - current_value) * io->DeltaTime * speed;
        return current_value;
    }
}

namespace celosia::animations {
    ImGuiID key(std::string_view name, ImGuiID seed) {
        return ImHashData(name.data(), name.size(), seed);
    }

    float get(ImGuiID id) {
        auto it = values.find(id);
        return it != values.end() ? it->second.x : 0.f;
    }

    float get(std::string_view name) {
        return get(key(name));
    }

    static float step(float current_value, const float goal_value, float speed, e_method method) {
        if (method == e_method::linear)
            return functions::calc_linear(current_value, goal_value, speed);
        return functions::calc_smooth(current_value, goal_value, speed);
    }

    float set(ImGuiID id, const float goal_value, float speed, e_method method) {
        ImVec4& v = values[id];
        v.x = step(v.x, goal_value, speed, method);
        return v.x;
    }

    ImVec2 set(ImGuiID id, const ImVec2 goal_value, float speed, e_method method) {
        ImVec4& v = values[id];
        v.x = step(v.x, goal_value.x, speed, method);
        v.y = step(v.y, goal_value.y, speed, method);
        return ImVec2(v.x, v.y);
    }

    ImVec4 set(ImGuiID id, const ImVec4 goal_value, float speed, e_method method) {
        ImVec4& v = values[id];
        v.x = step(v.x, goal_value.x, speed, method);
        v.y = step(v.y, goal_value.y, speed, method);
        v.z = step(v.z, goal_value.z, speed, method);
        v.w = step(v.w, goal_value.w, speed, method);
        return v;
    }

    float set(std::string_view name, const float goal_value, float speed, e_method method) {
        return set(key(name), goal_value, speed, method);
    }

    ImVec2 set(std::string_view name, const ImVec2 goal_value, float speed, e_method method) {
        return set(key(name), goal_value, speed, method);
    }

    ImVec4 set(std::string_view name, const ImVec4 goal_value, float speed, e_method method) {
        return set(key(name), goal_value, speed, method);
    }
}
