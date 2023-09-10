#include <algorithm>
#include "parser.hpp"

c_kit_parser kit_parser;

std::vector<paint_kit> parser_skins;
std::vector<paint_kit> parser_gloves;

class CCStrike15ItemSchema;
class CCStrike15ItemSystem;
template <typename Key, typename Value>

struct node_t {
	int previous_id;
	int next_id;
	void* _unknown_ptr;
	int _unknown;
	Key key;
	Value value;
};
template <typename key, typename value>
struct head_t {
	node_t<key, value>* memory;
	int allocation_count;
	int grow_size;
	int start_element;
	int next_available;
	int _unknown;
	int last_element;
};

struct string_t {
	char* buffer;
	int capacity;
	int grow_size;
	int length;
};

struct paint_kit_t {
	int id;
	string_t name;
	string_t description;
	string_t item_name;
	string_t material_name;
	string_t image_inventory;
	char pad_0x0054[0x8C];
};

void* get_export(const char* module_name, const char* export_name) {

	HMODULE mod;
	while (!((mod = GetModuleHandleA(module_name))))
		Sleep(100);
	return reinterpret_cast<void*>(GetProcAddress(mod, export_name));
}

void c_kit_parser::setup() noexcept {
	const auto V_UCS2ToUTF8 = static_cast<int(*)(const wchar_t* ucs2, char* utf8, int len)>(get_export("vstdlib.dll", "V_UCS2ToUTF8"));
	const auto sig_address = util::FindSignature("client.dll", "56 8B F1 FF 76 08 E8 ? ? ? ? FF 76 0C");
	const auto item_system_offset = *reinterpret_cast<std::int32_t*>(sig_address + 1);
	const auto item_system_fn = reinterpret_cast<CCStrike15ItemSystem * (*)()>(sig_address + 5 + item_system_offset);
	const auto item_schema = reinterpret_cast<CCStrike15ItemSchema*>(std::uintptr_t(item_system_fn()) + sizeof(void*));
	{
		const auto get_paint_kit_definition_offset = *reinterpret_cast<std::int32_t*>(sig_address + 11 + 1);
		const auto get_paint_kit_definition_fn = reinterpret_cast<paint_kit_t * (__thiscall*)(CCStrike15ItemSchema*, int)>(sig_address + 11 + 5 + get_paint_kit_definition_offset);
		const auto start_element_offset = *reinterpret_cast<std::intptr_t*>(std::uintptr_t(get_paint_kit_definition_fn) + 8 + 2);
		const auto head_offset = start_element_offset - 12;
		const auto map_head = reinterpret_cast<head_t<int, paint_kit_t*>*>(std::uintptr_t(item_schema) + head_offset);
		for (auto i = 0; i <= map_head->last_element; ++i) {
			const auto paint_kit = map_head->memory[i].value;
			if (paint_kit->id == 9001)
				continue;
			const auto wide_name = m_localize()->Find(paint_kit->item_name.buffer + 1);
			char name[256];

			V_UCS2ToUTF8(wide_name, name, sizeof(name));
			if (paint_kit->id < 10000)
				parser_skins.push_back({ paint_kit->id, name });
			else
				parser_gloves.push_back({ paint_kit->id, name });
		}
		std::sort(parser_skins.begin(), parser_skins.end());
		std::sort(parser_gloves.begin(), parser_gloves.end());
	}

	printf("Kit Parser initialized!\n");
}