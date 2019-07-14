#pragma once
#include "entity.hpp"
#include "package.hpp"
#include "util/uuid.hpp"
#include "util/uuid_provider.hpp"
#include <fstream>
#include <map>
#include <vector>

namespace horizon {
using json = nlohmann::json;

class Part : public UUIDProvider {
private:
    Part(const UUID &uu, const json &j, Pool &pool);
    const std::string empty;

public:
    class PadMapItem {
    public:
        PadMapItem(const Gate *g, const Pin *p) : gate(g), pin(p)
        {
        }
        uuid_ptr<const Gate> gate;
        uuid_ptr<const Pin> pin;
    };
    Part(const UUID &uu);

    static Part new_from_file(const std::string &filename, Pool &pool);
    UUID uuid;

    enum class Attribute { MPN, VALUE, MANUFACTURER, DATASHEET, DESCRIPTION };
    std::map<Attribute, std::pair<bool, std::string>> attributes;
    std::map<UUID, std::string> orderable_MPNs;
    const std::string &get_attribute(Attribute a) const;
    const std::pair<bool, std::string> &get_attribute_pair(Attribute a) const;

    const std::string &get_MPN() const;
    const std::string &get_value() const;
    const std::string &get_manufacturer() const;
    const std::string &get_datasheet() const;
    const std::string &get_description() const;
    std::set<std::string> get_tags() const;
    UUID get_model() const;

    std::set<std::string> tags;
    bool inherit_tags = false;
    uuid_ptr<const Entity> entity;
    uuid_ptr<const Package> package;
    UUID model;
    bool inherit_model = true;
    uuid_ptr<const Part> base;

    void update_refs(Pool &pool);
    UUID get_uuid() const;

    std::map<std::string, std::string> parametric;

    std::map<UUID, PadMapItem> pad_map;
    json serialize() const;
};
} // namespace horizon
