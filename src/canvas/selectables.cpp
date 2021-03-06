#include "selectables.hpp"
#include "canvas.hpp"
#include <assert.h>

namespace horizon {

Selectable::Selectable(const Coordf &center, const Coordf &box_center, const Coordf &box_dim, float a, bool always)
    : x(center.x), y(center.y), c_x(box_center.x), c_y(box_center.y), width(std::abs(box_dim.x)),
      height(std::abs(box_dim.y)), angle(a), flags(always ? static_cast<int>(Flag::ALWAYS) : 0)
{
}

bool Selectable::inside(const Coordf &c, float expand) const
{
    Coordf d = c - Coordf(c_x, c_y);
    float a = -angle;
    float dx = d.x * cos(a) - d.y * sin(a);
    float dy = d.x * sin(a) + d.y * cos(a);
    float w = std::max(width, expand) / 2;
    float h = std::max(height, expand) / 2;

    return (dx >= -w) && (dx <= w) && (dy >= -h) && (dy <= h);
}

float Selectable::area() const
{
    if (width == 0)
        return height;
    else if (height == 0)
        return width;
    else
        return width * height;
}

bool Selectable::is_line() const
{
    return (width == 0) != (height == 0);
}

bool Selectable::is_point() const
{
    return width == 0 && height == 0;
}

bool Selectable::is_box() const
{
    return width > 0 && height > 0;
}

static void rotate(float &x, float &y, float a)
{
    float ix = x;
    float iy = y;
    x = ix * cos(a) - iy * sin(a);
    y = ix * sin(a) + iy * cos(a);
}

std::array<Coordf, 4> Selectable::get_corners() const
{
    std::array<Coordf, 4> r;
    auto w = width + 100;
    auto h = height + 100;
    r[0] = Coordf(-w, -h) / 2;
    r[1] = Coordf(-w, h) / 2;
    r[2] = Coordf(w, h) / 2;
    r[3] = Coordf(w, -h) / 2;
    for (auto &it : r) {
        rotate(it.x, it.y, angle);
        it += Coordf(c_x, c_y);
    }
    return r;
}

void Selectable::set_flag(Selectable::Flag f, bool v)
{
    auto mask = static_cast<int>(f);
    if (v) {
        flags |= mask;
    }
    else {
        flags &= ~mask;
    }
}

bool Selectable::get_flag(Selectable::Flag f) const
{
    auto mask = static_cast<int>(f);
    return flags & mask;
}

Selectables::Selectables(const Canvas &c) : ca(c)
{
}

void Selectables::append(const UUID &uu, ObjectType ot, const Coordf &center, const Coordf &a, const Coordf &b,
                         unsigned int vertex, LayerRange layer, bool always)
{
    Placement tr = ca.transform;
    if (tr.mirror)
        tr.invert_angle();
    tr.mirror = false;
    auto box_center = ca.transform.transform((b + a) / 2);
    auto box_dim = b - a;
    append_angled(uu, ot, center, box_center, box_dim, tr.get_angle_rad(), vertex, layer, always);
}

void Selectables::append(const UUID &uu, ObjectType ot, const Coordf &center, unsigned int vertex, LayerRange layer,
                         bool always)
{
    append(uu, ot, center, center, center, vertex, layer, always);
}

void Selectables::append_angled(const UUID &uu, ObjectType ot, const Coordf &center, const Coordf &box_center,
                                const Coordf &box_dim, float angle, unsigned int vertex, LayerRange layer, bool always)
{
    items_map.emplace(std::piecewise_construct, std::forward_as_tuple(uu, ot, vertex, layer),
                      std::forward_as_tuple(items.size()));
    items.emplace_back(ca.transform.transform(center), box_center, box_dim, angle, always);
    items_ref.emplace_back(uu, ot, vertex, layer);
    items_group.push_back(group_current);
}

void Selectables::append_line(const UUID &uu, ObjectType ot, const Coordf &p0, const Coordf &p1, float width,
                              unsigned int vertex, LayerRange layer, bool always)
{
    float box_height = width;
    Coordf delta = p1 - p0;
    float box_width = width + sqrt(delta.mag_sq());
    float angle = atan2(delta.y, delta.x);
    auto center = (p0 + p1) / 2;
    append_angled(uu, ot, center, center, Coordf(box_width, box_height), angle, vertex, layer, always);
}


void Selectables::update_preview(const std::set<SelectableRef> &sel)
{
    std::set<int> groups;
    for (const auto &it : sel) {
        if (items_map.count(it)) {
            auto idx = items_map.at(it);
            auto group = items_group.at(idx);
            if (group != -1)
                groups.insert(group);
        }
    }
    for (size_t i = 0; i < items.size(); i++) {
        auto group = items_group.at(i);
        items.at(i).set_flag(Selectable::Flag::PREVIEW, groups.count(group));
    }
}

void Selectables::group_begin()
{
    assert(group_current == -1);
    group_current = group_max;
}

void Selectables::group_end()
{
    assert(group_current != -1);
    group_current = -1;
    group_max++;
}

void Selectables::clear()
{
    items.clear();
    items_ref.clear();
    items_group.clear();
    items_map.clear();
    group_max = 0;
}
} // namespace horizon
