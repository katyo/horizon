#include "action_button.hpp"
#include "imp/action_catalog.hpp"
#include "core/tool_id.hpp"
#include <iostream>

namespace horizon {

static const std::map<ActionToolID, const char *> action_icons = {
        {make_action(ToolID::DRAW_POLYGON), "action-draw-polygon-symbolic"},
        {make_action(ToolID::PLACE_TEXT), "action-place-text-symbolic"},
        {make_action(ToolID::PLACE_REFDES_AND_VALUE), "action-place-refdes-and-value-symbolic"},
        {make_action(ToolID::PLACE_SHAPE), "action-place-shape-circle-symbolic"},
        {make_action(ToolID::PLACE_SHAPE_OBROUND), "action-place-shape-obround-symbolic"},
        {make_action(ToolID::PLACE_SHAPE_RECTANGLE), "action-place-shape-rectangle-symbolic"},
        {make_action(ToolID::DRAW_LINE), "action-draw-line-symbolic"},
        {make_action(ToolID::DRAW_LINE_RECTANGLE), "action-draw-line-rectangle-symbolic"},
        {make_action(ToolID::PLACE_HOLE), "action-place-hole-symbolic"},
        {make_action(ToolID::PLACE_HOLE_SLOT), "action-place-hole-slot-symbolic"},
};

const char *get_icon(ActionToolID act)
{
    if (action_icons.count(act))
        return action_icons.at(act);
    else
        return "face-worried-symbolic";
}

ActionButton::ActionButton(ActionToolID act, const std::map<ActionToolID, std::string> &ks) : action(act), keys(ks)
{
    get_style_context()->add_class("osd");
    button = Gtk::manage(new Gtk::Button);
    set_primary_action(action);
    button->signal_clicked().connect([this] { s_signal_clicked.emit(action); });
    button->signal_button_press_event().connect(
            [this](GdkEventButton *ev) {
                if (!menu_button->get_visible())
                    return false;
                if (gdk_event_triggers_context_menu((GdkEvent *)ev)) {
                    menu.popup_at_widget(button, Gdk::GRAVITY_NORTH_EAST, Gdk::GRAVITY_NORTH_WEST, (GdkEvent *)ev);
                    return true;
                }
                else if (ev->button == 1) {
                    button_current = 1;
                }
                return false;
            },
            false);
    button->signal_button_release_event().connect([this](GdkEventButton *ev) {
        button_current = -1;
        return false;
    });
    button->signal_leave_notify_event().connect([this](GdkEventCrossing *ev) {
        if (ev->x > button->get_allocated_width() / 2 && button_current == 1) {
            menu.popup_at_widget(button, Gdk::GRAVITY_NORTH_EAST, Gdk::GRAVITY_NORTH_WEST, (GdkEvent *)ev);
        }
        button_current = -1;
        return false;
    });
    add(*button);
    button->show();
    menu_button = Gtk::manage(new Gtk::MenuButton);
    menu_button->set_image_from_icon_name("action-button-expander-symbolic", Gtk::ICON_SIZE_BUTTON);
    menu_button->get_style_context()->add_class("imp-action-bar-expander");
    menu_button->set_relief(Gtk::RELIEF_NONE);
    menu_button->set_halign(Gtk::ALIGN_END);
    menu_button->set_valign(Gtk::ALIGN_END);
    menu_button->set_direction(Gtk::ARROW_RIGHT);
    menu_button->set_menu(menu);
    menu_button->signal_button_press_event().connect(
            [this](GdkEventButton *ev) {
                if (!menu_button->get_visible())
                    return false;
                if (ev->button == 1) {
                    button_current = 1;
                }
                return false;
            },
            false);
    menu_button->signal_button_release_event().connect([this](GdkEventButton *ev) {
        button_current = -1;
        return false;
    });
    menu_button->signal_leave_notify_event().connect([this](GdkEventCrossing *ev) {
        if (ev->x > menu_button->get_allocated_width() / 2 && button_current == 1) {
            menu.popup_at_widget(menu_button, Gdk::GRAVITY_NORTH_EAST, Gdk::GRAVITY_NORTH_WEST, (GdkEvent *)ev);
        }
        button_current = -1;
        return false;
    });
    add_overlay(*menu_button);
    menu_button->set_no_show_all();
    add_menu_item(action);
    menu.get_style_context()->add_class("osd");
    menu.set_reserve_toggle_size(false);
}

void ActionButton::update_key_sequences()
{
    set_primary_action(action);
}

void ActionButton::add_action(ActionToolID act)
{
    menu_button->show();
    add_menu_item(act);
}

void ActionButton::set_primary_action(ActionToolID act)
{
    action = act;
    button->set_image_from_icon_name(get_icon(action), Gtk::ICON_SIZE_DND);
    std::string l = action_catalog.at(action).name;
    if (keys.count(action) && keys.at(action).size()) {
        l += " (" + keys.at(action) + ")";
    }
    button->set_tooltip_text(l);
}

Gtk::MenuItem &ActionButton::add_menu_item(ActionToolID act)
{
    auto it = Gtk::manage(new Gtk::MenuItem());
    auto box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 8));
    auto img = Gtk::manage(new Gtk::Image(get_icon(act), Gtk::ICON_SIZE_DND));
    box->pack_start(*img, false, false, 0);
    auto la = Gtk::manage(new Gtk::Label(action_catalog.at(act).name));
    la->set_xalign(0);
    box->pack_start(*la, true, true, 0);
    it->add(*box);
    it->show_all();
    it->signal_activate().connect([this, act] {
        set_primary_action(act);
        s_signal_clicked.emit(act);
    });
    menu.append(*it);
    return *it;
}

} // namespace horizon
