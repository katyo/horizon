#include "tool_map_symbol.hpp"
#include <iostream>
#include "core_schematic.hpp"
#include "imp_interface.hpp"
#include "entity.hpp"

namespace horizon {

	ToolMapSymbol::ToolMapSymbol(Core *c, ToolID tid): ToolBase(c, tid) {
		name = "Map Symbol";
	}

	bool ToolMapSymbol::can_begin() {
		return core.c;
	}

	ToolResponse ToolMapSymbol::begin(const ToolArgs &args) {
		std::cout << "tool map sym\n";
		Schematic *sch = core.c->get_schematic();
		//collect gates
		std::set<UUIDPath<2>> gates;
		for(const auto &it_component: sch->block->components) {
			for(const auto &it_gate: it_component.second.entity->gates) {
				gates.emplace(it_component.first, it_gate.first);
			}
		}

		for(auto &it_sheet: sch->sheets) {
			Sheet &sheet = it_sheet.second;

			for(const auto &it_sym: sheet.symbols) {
				const auto &sym = it_sym.second;
				if(gates.count({sym.component->uuid, sym.gate->uuid})) {
					gates.erase({sym.component->uuid, sym.gate->uuid});
				}
			}
		}
		UUID filter_uuid;
		if(core.c->selection.size() == 1) {
			if(core.c->selection.begin()->type == ObjectType::COMPONENT) {
				filter_uuid = core.c->selection.begin()->uuid;
			}
		}
		std::map<UUIDPath<2>, std::string> gates_out;
		for(const auto &it: gates) {
			Component *comp = &sch->block->components.at(it.at(0));
			const Gate *gate = &comp->entity->gates.at(it.at(1));
			if(!filter_uuid || filter_uuid==comp->uuid) {
				gates_out.emplace(std::make_pair(it, comp->refdes+gate->suffix));
			}
		}
		if(gates_out.size() == 0) {
			return ToolResponse::end();
		}

		UUIDPath<2> selected_gate;
		bool r;
		if(gates_out.size()>1) {
			std::tie(r, selected_gate) = imp->dialogs.map_symbol(gates_out);
			if(!r) {
				return ToolResponse::end();
			}
		}
		else {
			selected_gate = gates_out.begin()->first;
		}

		Component *comp = &sch->block->components.at(selected_gate.at(0));
		const Gate *gate = &comp->entity->gates.at(selected_gate.at(1));
		UUID selected_symbol;

		std::string query  = "SELECT symbols.uuid FROM symbols WHERE symbols.unit = ?";
		SQLite::Query q(core.r->m_pool->db, query);
		q.bind(1, gate->unit->uuid);
		int n = 0;
		while(q.step()) {
			selected_symbol = q.get<std::string>(0);
			n++;
		}

		if(n!=1) {
			std::tie(r, selected_symbol) = imp->dialogs.select_symbol(core.r->m_pool, gate->unit->uuid);
			if(!r) {
				return ToolResponse::end();
			}
		}

		const Symbol *sym = core.c->m_pool->get_symbol(selected_symbol);
		SchematicSymbol *schsym = core.c->insert_schematic_symbol(UUID::random(), sym);
		schsym->component = comp;
		schsym->gate = gate;
		schsym->placement.shift = args.coords;

		core.c->selection.clear();
		core.c->selection.emplace(schsym->uuid, ObjectType::SCHEMATIC_SYMBOL);
		core.c->commit();


		return ToolResponse::next(ToolID::MOVE);
	}
	ToolResponse ToolMapSymbol::update(const ToolArgs &args) {
		return ToolResponse();
	}
}
