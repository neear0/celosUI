#include "../ui.h"

float celosia::animations::functions::calc_linear(float current_value, const float goal_value, float speed) {
    speed += animations::base_speed;

    if (speed < 0)
        speed = 0;

    if (current_value == goal_value) return goal_value;

    if (current_value > goal_value) {
        current_value -= speed * delta_time;
        if (current_value <= goal_value)
            return goal_value;
    }
    else if (current_value < goal_value)
    {
        current_value += speed * delta_time;
        if (current_value >= goal_value)
            return goal_value;
    }

    return current_value;
}

float celosia::animations::functions::calc_smooth(float current_value, const float goal_value, float speed) {
    speed += base_speed;

    if (speed < 0)
        speed = 0;

    // close enough to stop: a thousandth is below a pixel and below a step of an 8 bit color.
    // snapping from further away shows as a jump at the end
    if (fabsf(goal_value - current_value) < 0.001f)
        return goal_value;

    // the same share of the way each second, whatever the frame rate
    return current_value + (goal_value - current_value) * (1.f - expf(-speed * delta_time));
}

void celosia::animations::new_frame() {
    // the frame time measured on the cpu jitters a few ms around the display's refresh interval (13 to 21 ms at 60 Hz),
    // while the frames are shown evenly. stepping by it moves animations unevenly from frame to frame, an average steps evenly.
    // a frame that is much longer or shorter (a hitch, or the frame rate changing) is taken as it is so animations don't lag behind
    const float dt = ImGui::GetIO().DeltaTime;
    if (delta_time <= 0.f || dt > delta_time * 1.5f || dt < delta_time / 1.5f)
        delta_time = dt;
    else
        delta_time += (dt - delta_time) * 0.1f;
}

ImGuiID celosia::animations::key(std::string_view name, ImGuiID seed) {
    return ImHashData(name.data(), name.size(), seed);
}

float celosia::animations::get(ImGuiID id) {
    auto it = values.find(id);
    return it != values.end() ? it->second.x : 0.f;
}

float celosia::animations::get(std::string_view name) {
    return get(key(name));
}

static float step(float current_value, const float goal_value, float speed, celosia::animations::e_method method) {
    if (method == celosia::animations::e_method::linear)
        return celosia::animations::functions::calc_linear(current_value, goal_value, speed);
    return celosia::animations::functions::calc_smooth(current_value, goal_value, speed);
}

float celosia::animations::set(ImGuiID id, const float goal_value, float speed, e_method method) {
    ImVec4& v = values[id];
    v.x = step(v.x, goal_value, speed, method);
    return v.x;
}

ImVec2 celosia::animations::set(ImGuiID id, const ImVec2 goal_value, float speed, e_method method) {
    ImVec4& v = values[id];
    v.x = step(v.x, goal_value.x, speed, method);
    v.y = step(v.y, goal_value.y, speed, method);
    return ImVec2(v.x, v.y);
}

ImVec4 celosia::animations::set(ImGuiID id, const ImVec4 goal_value, float speed, e_method method) {
    ImVec4& v = values[id];
    v.x = step(v.x, goal_value.x, speed, method);
    v.y = step(v.y, goal_value.y, speed, method);
    v.z = step(v.z, goal_value.z, speed, method);
    v.w = step(v.w, goal_value.w, speed, method);
    return v;
}

float celosia::animations::set(std::string_view name, const float goal_value, float speed, e_method method) {
    return set(key(name), goal_value, speed, method);
}

ImVec2 celosia::animations::set(std::string_view name, const ImVec2 goal_value, float speed, e_method method) {
    return set(key(name), goal_value, speed, method);
}

ImVec4 celosia::animations::set(std::string_view name, const ImVec4 goal_value, float speed, e_method method) {
    return set(key(name), goal_value, speed, method);
}
