/**
 * MIT License
 *
 * Copyright (c) 2025 AdvanceControl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/
#include "pbijson.hpp"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

// --- CacheManager Definition and Implementation ---
// We define it here to keep it private to the .cpp file
template <typename T>
class TypedCache {
private:
	Dictionary _cache;
public:
	bool has(const StringName &key) const { return _cache.has(key); }
	T get(const StringName &key) const { return _cache.get(key, T()); }
	void set(const StringName &key, const T &value) { _cache[key] = value; }
	bool erase(const StringName &key) { return _cache.erase(key); }
	void clear() { _cache.clear(); }
};

class CacheManager {
private:
	TypedCache<Variant> _value_cache;
	TypedCache<bool> _has_path_cache;
	TypedCache<int> _get_size_cache;
	TypedCache<PackedStringArray> _get_subpaths_cache;
	TypedCache<Array> _get_keys_cache;
public:
	void clear_all() {
		_value_cache.clear();
		_has_path_cache.clear();
		_get_size_cache.clear();
		_get_subpaths_cache.clear();
		_get_keys_cache.clear();
	}
	void clear_by_flag(PreBuiltIndexJSON::CacheFlags flag) {
		switch (flag) {
			case PreBuiltIndexJSON::VALUE_CACHE: _value_cache.clear(); break;
			case PreBuiltIndexJSON::HAS_PATH_CACHE: _has_path_cache.clear(); break;
			case PreBuiltIndexJSON::GET_SIZE_CACHE: _get_size_cache.clear(); break;
			case PreBuiltIndexJSON::GET_SUBPATHS_CACHE: _get_subpaths_cache.clear(); break;
			case PreBuiltIndexJSON::GET_KEYS_CACHE: _get_keys_cache.clear(); break;
			default: break;
		}
	}
	bool has(PreBuiltIndexJSON::CacheFlags flag, const StringName &key) const {
		switch (flag) {
			case PreBuiltIndexJSON::VALUE_CACHE: return _value_cache.has(key);
			case PreBuiltIndexJSON::HAS_PATH_CACHE: return _has_path_cache.has(key);
			case PreBuiltIndexJSON::GET_SIZE_CACHE: return _get_size_cache.has(key);
			case PreBuiltIndexJSON::GET_SUBPATHS_CACHE: return _get_subpaths_cache.has(key);
			case PreBuiltIndexJSON::GET_KEYS_CACHE: return _get_keys_cache.has(key);
			default: return false;
		}
	}
	bool erase(PreBuiltIndexJSON::CacheFlags flag, const StringName &key) {
		switch (flag) {
			case PreBuiltIndexJSON::VALUE_CACHE: return _value_cache.erase(key);
			case PreBuiltIndexJSON::HAS_PATH_CACHE: return _has_path_cache.erase(key);
			case PreBuiltIndexJSON::GET_SIZE_CACHE: return _get_size_cache.erase(key);
			case PreBuiltIndexJSON::GET_SUBPATHS_CACHE: return _get_subpaths_cache.erase(key);
			case PreBuiltIndexJSON::GET_KEYS_CACHE: return _get_keys_cache.erase(key);
			default: return false;
		}
	}
	template <typename T> void set(PreBuiltIndexJSON::CacheFlags flag, const StringName &key, const T &value) {
		if constexpr (std::is_same_v<T, Variant>) { if (flag == PreBuiltIndexJSON::VALUE_CACHE) _value_cache.set(key, value); } 
		else if constexpr (std::is_same_v<T, bool>) { if (flag == PreBuiltIndexJSON::HAS_PATH_CACHE) _has_path_cache.set(key, value); } 
		else if constexpr (std::is_same_v<T, int>) { if (flag == PreBuiltIndexJSON::GET_SIZE_CACHE) _get_size_cache.set(key, value); } 
		else if constexpr (std::is_same_v<T, PackedStringArray>) { if (flag == PreBuiltIndexJSON::GET_SUBPATHS_CACHE) _get_subpaths_cache.set(key, value); } 
		else if constexpr (std::is_same_v<T, Array>) { if (flag == PreBuiltIndexJSON::GET_KEYS_CACHE) _get_keys_cache.set(key, value); }
	}
	template <typename T> T get(PreBuiltIndexJSON::CacheFlags flag, const StringName &key) const {
		if constexpr (std::is_same_v<T, Variant>) { if (flag == PreBuiltIndexJSON::VALUE_CACHE) return _value_cache.get(key); } 
		else if constexpr (std::is_same_v<T, bool>) { if (flag == PreBuiltIndexJSON::HAS_PATH_CACHE) return _has_path_cache.get(key); } 
		else if constexpr (std::is_same_v<T, int>) { if (flag == PreBuiltIndexJSON::GET_SIZE_CACHE) return _get_size_cache.get(key); } 
		else if constexpr (std::is_same_v<T, PackedStringArray>) { if (flag == PreBuiltIndexJSON::GET_SUBPATHS_CACHE) return _get_subpaths_cache.get(key); } 
		else if constexpr (std::is_same_v<T, Array>) { if (flag == PreBuiltIndexJSON::GET_KEYS_CACHE) return _get_keys_cache.get(key); }
		return T();
	}
};

// --- PreBuiltIndexJSON Implementation ---

void PreBuiltIndexJSON::_bind_methods() {
	// ADD_PROPERTY(PropertyInfo(Variant::INT, "cache_flags", PROPERTY_HINT_FLAGS, "Value Cache,Path Existence Cache,Size Cache,Sub-paths Cache,Keys Cache"), "set_cache_flags", "get_cache_flags");
	ClassDB::bind_method(D_METHOD("build_from_string", "json_text"), &PreBuiltIndexJSON::build_from_string);
	ClassDB::bind_method(D_METHOD("build_from_file", "json_file_path", "target_path"), &PreBuiltIndexJSON::build_from_file);
	ClassDB::bind_method(D_METHOD("open_file", "path"), &PreBuiltIndexJSON::open_file);
	ClassDB::bind_method(D_METHOD("open_from_string", "data"), &PreBuiltIndexJSON::open_from_string);
	ClassDB::bind_method(D_METHOD("open_from_array", "data"), &PreBuiltIndexJSON::open_from_array);
	ClassDB::bind_method(D_METHOD("reload_file"), &PreBuiltIndexJSON::reload_file);
	ClassDB::bind_method(D_METHOD("get_value", "key_path", "default"), &PreBuiltIndexJSON::get_value, DEFVAL(Variant()));
    ClassDB::bind_method(D_METHOD("has_path", "key_path"), &PreBuiltIndexJSON::has_path);
    ClassDB::bind_method(D_METHOD("get_size", "key_path"), &PreBuiltIndexJSON::get_size);
    ClassDB::bind_method(D_METHOD("get_keys", "key_path"), &PreBuiltIndexJSON::get_keys);
    ClassDB::bind_method(D_METHOD("get_sub_paths", "key_path"), &PreBuiltIndexJSON::get_sub_paths);
	ClassDB::bind_method(D_METHOD("clear"), &PreBuiltIndexJSON::clear);
	ClassDB::bind_method(D_METHOD("close"), &PreBuiltIndexJSON::close);
	ClassDB::bind_method(D_METHOD("clear_caches"), &PreBuiltIndexJSON::clear_caches);
	ClassDB::bind_method(D_METHOD("clear_cache", "flag"), &PreBuiltIndexJSON::clear_cache);
	ClassDB::bind_method(D_METHOD("remove_from_cache", "flag", "key_path"), &PreBuiltIndexJSON::remove_from_cache);
	ClassDB::bind_method(D_METHOD("get_last_error"), &PreBuiltIndexJSON::get_last_error);
	ClassDB::bind_method(D_METHOD("is_data_loaded"), &PreBuiltIndexJSON::is_data_loaded);
	ClassDB::bind_method(D_METHOD("get_opened_file"), &PreBuiltIndexJSON::get_opened_file);
	ClassDB::bind_method(D_METHOD("is_cache_enabled", "flag"), &PreBuiltIndexJSON::is_cache_enabled);
	ClassDB::bind_method(D_METHOD("set_cache_enabled", "flag", "enabled"), &PreBuiltIndexJSON::set_cache_enabled);
    ClassDB::bind_method(D_METHOD("has_in_cache", "flag", "key_path"), &PreBuiltIndexJSON::has_in_cache);

	BIND_ENUM_CONSTANT(NONE);
	BIND_ENUM_CONSTANT(VALUE_CACHE);
	BIND_ENUM_CONSTANT(HAS_PATH_CACHE);
	BIND_ENUM_CONSTANT(GET_SIZE_CACHE);
	BIND_ENUM_CONSTANT(GET_SUBPATHS_CACHE);
	BIND_ENUM_CONSTANT(GET_KEYS_CACHE);
	BIND_ENUM_CONSTANT(ALL);
}

PreBuiltIndexJSON::PreBuiltIndexJSON() {
	_mutex.instantiate();
	_last_error.instantiate();
	_cache_manager = new CacheManager();
}

PreBuiltIndexJSON::~PreBuiltIndexJSON() {
	delete _cache_manager;
}

Ref<PreBuiltIndexJSONOutput> PreBuiltIndexJSON::build_from_file(const String &p_json_file, const String &p_target_path) {
	_mutex->lock();
	Ref<FileAccess> read_file = FileAccess::open(p_json_file, FileAccess::ModeFlags::READ);
	if (read_file.is_null()) {
		_last_error = Ref<PreBuiltIndexJSONOutput>(memnew(PreBuiltIndexJSONOutput(FileAccess::get_open_error())));
		_mutex->unlock();
		return _last_error;
	}
	Ref<PreBuiltIndexJSONOutput> output = build_from_string(read_file->get_as_text());
	if (output->get_error_type() != PreBuiltIndexJSONOutput::OK) {
		_last_error->set_to(output);
		_mutex->unlock();
		return _last_error;
	}
	Ref<FileAccess> write_file = FileAccess::open(p_target_path, FileAccess::ModeFlags::WRITE);
	if (write_file.is_null()) {
		_last_error = Ref<PreBuiltIndexJSONOutput>(memnew(PreBuiltIndexJSONOutput(FileAccess::get_open_error())));
		_mutex->unlock();
		return _last_error;
	}
	write_file->store_string(output->get_data());
	_last_error = Ref<PreBuiltIndexJSONOutput>(memnew(PreBuiltIndexJSONOutput(PreBuiltIndexJSONOutput::OK)));
	_mutex->unlock();
	return output;
}

Ref<PreBuiltIndexJSONOutput> PreBuiltIndexJSON::build_from_string(const String &p_json_text) {
	_mutex->lock();
	if (!_build_buffer.is_empty()) {
		_build_buffer.clear();
		UtilityFunctions::printerr("Build buffer was not empty. This may indicate a data race or unclean state.", __FUNCTION__, __FILE__, __LINE__);
	}
	Ref<JSON> json_parser = memnew(JSON);
	Error err = json_parser->parse(p_json_text);
	if (err != OK) {
		Ref<PreBuiltIndexJSONOutput> output = Ref<PreBuiltIndexJSONOutput>(memnew(PreBuiltIndexJSONOutput(PreBuiltIndexJSONOutput::ERR_JSON_PARSE, json_parser->get_error_message(), json_parser->get_error_line())));
		_mutex->unlock();
		return output;
	}
	Variant json_data = json_parser->get_data();
	if (json_data.get_type() != Variant::DICTIONARY && json_data.get_type() != Variant::ARRAY) {
		Ref<PreBuiltIndexJSONOutput> output = Ref<PreBuiltIndexJSONOutput>(memnew(PreBuiltIndexJSONOutput(PreBuiltIndexJSONOutput::ERR_UNSUPPORTED_TYPE, "Top-level JSON data must be a Dictionary or an Array.")));
		_mutex->unlock();
		return output;
	}
	Dictionary container_lines;
	_build_flat_index_recursive(json_data, 1, container_lines);
	_add_jump_marks_to_buffer(container_lines);
	String file_text = String("\n").join(_build_buffer);
	_build_buffer.clear();
	Ref<PreBuiltIndexJSONOutput> output = Ref<PreBuiltIndexJSONOutput>(memnew(PreBuiltIndexJSONOutput(file_text)));
	_mutex->unlock();
	return output;
}

void PreBuiltIndexJSON::_build_flat_index_recursive(const Variant &p_current_value, int p_depth, Dictionary &p_container_lines) {
	Variant::Type value_type = p_current_value.get_type();
	if (value_type == Variant::DICTIONARY) {
		Dictionary data_dict = p_current_value;
		Array sorted_keys = data_dict.keys();
		sorted_keys.sort();
		for (int i = 0; i < sorted_keys.size(); ++i) {
			const Variant& key_var = sorted_keys[i];
			Variant value = data_dict.get(key_var, Variant());
			String prefix = String::chr(DEPTH_MARKER).repeat(p_depth);
			String formatted_key = JSON::stringify(key_var);
			String line_header = prefix + formatted_key;
			_build_buffer.append(line_header);
			int64_t current_line_idx = _build_buffer.size() - 1;
			Variant::Type sub_value_type = value.get_type();
			if (sub_value_type == Variant::DICTIONARY || sub_value_type == Variant::ARRAY) {
				Dictionary dict;
				dict["depth"] = p_depth;
				dict["value"] = value;
				p_container_lines[current_line_idx] = dict;
				_build_flat_index_recursive(value, p_depth + 1, p_container_lines);
			} else {
				_build_buffer[current_line_idx] += String::chr(VALUE_SEPARATOR) + JSON::stringify(value);
			}
		}
	} else if (value_type == Variant::ARRAY) {
		Array data_array = p_current_value;
		for (int i = 0; i < data_array.size(); ++i) {
			Variant value = data_array[i];
			String prefix = String::chr(DEPTH_MARKER).repeat(p_depth);
			String formatted_key = String("[{0}]").format(Array::make(i));
			String line_header = prefix + formatted_key;
			_build_buffer.append(line_header);
			int64_t current_line_idx = _build_buffer.size() - 1;
			Variant::Type sub_value_type = value.get_type();
			if (sub_value_type == Variant::DICTIONARY || sub_value_type == Variant::ARRAY) {
				Dictionary dict;
				dict["depth"] = p_depth;
				dict["value"] = value;
				p_container_lines[current_line_idx] = dict;
				_build_flat_index_recursive(value, p_depth + 1, p_container_lines);
			} else {
				_build_buffer[current_line_idx] += String::chr(VALUE_SEPARATOR) + JSON::stringify(value);
			}
		}
	}
}

void PreBuiltIndexJSON::_add_jump_marks_to_buffer(Dictionary &p_container_lines) {
    Array sorted_keys = p_container_lines.keys();
    sorted_keys.sort();
    Dictionary line_counts;
    for (int64_t i = sorted_keys.size() - 1; i >= 0; --i) {
        int64_t line_idx = sorted_keys[i];
        Dictionary container_info = p_container_lines[line_idx];
        int depth = container_info["depth"];
        int descendant_count = 0;
        int64_t j = line_idx + 1;
        while (j < _build_buffer.size()) {
            const String& line = _build_buffer[j];
            int line_depth = _get_line_depth(line);
            if (line_depth <= depth) {
                break;
            }
            if (line_counts.has(j)) {
                int jump = line_counts[j];
                descendant_count += 1 + jump;
                j += 1 + jump;
            } else {
                descendant_count += 1;
                j += 1;
            }
        }
        if (descendant_count > 0) {
            line_counts[line_idx] = descendant_count;
            _build_buffer[line_idx] += String::chr(JUMP_MARKER_OPEN) + String::num_int64(descendant_count);
        } else {
            Variant empty_container = container_info["value"];
            _build_buffer[line_idx] += String::chr(VALUE_SEPARATOR) + JSON::stringify(empty_container);
        }
    }
}

Variant PreBuiltIndexJSON::get_value(const String &p_key_path, const Variant &p_default) const {
	_mutex->lock();
	_last_error->clear();
	const StringName key(p_key_path);
	if (is_cache_enabled(VALUE_CACHE) && _cache_manager->has(VALUE_CACHE, key)) {
		_mutex->unlock();
		return _cache_manager->get<Variant>(VALUE_CACHE, key);
	}
	if (!is_data_loaded()) {
		_last_error = Ref<PreBuiltIndexJSONOutput>(memnew(PreBuiltIndexJSONOutput(PreBuiltIndexJSONOutput::ERR_DATA_NOT_OPEN)));
		_mutex->unlock();
		return p_default;
	}
	PackedStringArray path_parts = _parse_escaped_path(p_key_path);
	Variant result;
	if (path_parts.is_empty() || (path_parts.size() == 1 && path_parts[0].is_empty())) {
		bool is_root_array = false;
		if (!_current_open_data.is_empty()) {
			is_root_array = _get_line_key_part(_current_open_data[0]).begins_with("[");
		}
		result = _rebuild_container_from_slice(_current_open_data, 1, is_root_array);
		if (is_cache_enabled(VALUE_CACHE)) {
			_cache_manager->set<Variant>(VALUE_CACHE, key, result);
		}
		_mutex->unlock();
		return result;
	}
	int current_line_idx = 0;
	int search_range_end = _current_open_data.size();
	bool is_parent_array = false;
	if (!_current_open_data.is_empty()) {
		is_parent_array = _get_line_key_part(_current_open_data[0]).begins_with("[");
	}
	for (int i = 0; i < path_parts.size(); ++i) {
		const String &part_to_find = path_parts[i];
		int expected_depth = i + 1;
		Dictionary find_result = _find_part_in_range(part_to_find, expected_depth, current_line_idx, search_range_end, is_parent_array, p_key_path);
		if (find_result.is_empty()) {
			if (_last_error->get_error_type() == PreBuiltIndexJSONOutput::OK) {
				_last_error = Ref<PreBuiltIndexJSONOutput>(memnew(PreBuiltIndexJSONOutput(PreBuiltIndexJSONOutput::ERR_INVALID_PATH, "Path part '" + part_to_find + "' not found.")));
			}
			_mutex->unlock();
			return p_default;
		}
		current_line_idx = find_result["line_idx"];
		const String &line_content = _current_open_data[current_line_idx];
		if (i == path_parts.size() - 1) {
			if (line_content.contains(String::chr(JUMP_MARKER_OPEN))) {
				int jump_count = line_content.split(String::chr(JUMP_MARKER_OPEN), true, 1)[1].to_int();
				PackedStringArray data_slice = _current_open_data.slice(current_line_idx + 1, current_line_idx + 1 + jump_count);
				bool is_target_array = false;
				if (!data_slice.is_empty()) {
					is_target_array = _get_line_key_part(data_slice[0]).begins_with("[");
				}
				result = _rebuild_container_from_slice(data_slice, expected_depth + 1, is_target_array);
			} else {
				result = _get_line_value(line_content, current_line_idx);
			}
			if (is_cache_enabled(VALUE_CACHE)) {
				_cache_manager->set<Variant>(VALUE_CACHE, key, result);
			}
			_mutex->unlock();
			return result;
		}
		if (!line_content.contains(String::chr(JUMP_MARKER_OPEN))) {
			_last_error = Ref<PreBuiltIndexJSONOutput>(memnew(PreBuiltIndexJSONOutput(PreBuiltIndexJSONOutput::ERR_INVALID_PATH, "Path expects a container, but found a value at part '" + part_to_find + "'.")));
			_mutex->unlock();
			return p_default;
		}
		int jump_count = line_content.split(String::chr(JUMP_MARKER_OPEN), true, 1)[1].to_int();
        if (current_line_idx + 1 < _current_open_data.size()) {
		    is_parent_array = _get_line_key_part(_current_open_data[current_line_idx + 1]).begins_with("[");
        } else {
            is_parent_array = false;
        }
		current_line_idx += 1;
		search_range_end = current_line_idx + jump_count;
	}
	_mutex->unlock();
	return p_default;
}

bool PreBuiltIndexJSON::has_path(const String &p_key_path) const {
	_mutex->lock();
	_last_error->clear();
    const StringName key(p_key_path);
	if (is_cache_enabled(HAS_PATH_CACHE) && _cache_manager->has(HAS_PATH_CACHE, key)) {
        _mutex->unlock();
		return _cache_manager->get<bool>(HAS_PATH_CACHE, key);
	}
	if (!is_data_loaded()) {
		_last_error = Ref<PreBuiltIndexJSONOutput>(memnew(PreBuiltIndexJSONOutput(PreBuiltIndexJSONOutput::ERR_DATA_NOT_OPEN)));
		_mutex->unlock();
		return false;
	}
	PackedStringArray path_parts = _parse_escaped_path(p_key_path);
	if (path_parts.is_empty() || (path_parts.size() == 1 && path_parts[0].is_empty())) {
		_mutex->unlock();
		return is_data_loaded();
	}
	int current_line_idx = 0;
	int search_range_end = _current_open_data.size();
	bool is_parent_array = _get_line_key_part(_current_open_data[0]).begins_with("[");
	bool result = false;
	for (int i = 0; i < path_parts.size(); ++i) {
		const String &part_to_find = path_parts[i];
		int expected_depth = i + 1;
		Dictionary find_result = _find_part_in_range(part_to_find, expected_depth, current_line_idx, search_range_end, is_parent_array, p_key_path);
		if (find_result.is_empty()) {
			result = false;
			if (is_cache_enabled(HAS_PATH_CACHE)) _cache_manager->set<bool>(HAS_PATH_CACHE, key, result);
			_mutex->unlock();
			return result;
		}
		if (i == path_parts.size() - 1) {
			result = true;
			if (is_cache_enabled(HAS_PATH_CACHE)) _cache_manager->set<bool>(HAS_PATH_CACHE, key, result);
			_mutex->unlock();
			return result;
		}
		current_line_idx = find_result["line_idx"];
		const String &line_content = _current_open_data[current_line_idx];
		if (!line_content.contains(String::chr(JUMP_MARKER_OPEN))) {
			result = false;
			if (is_cache_enabled(HAS_PATH_CACHE)) _cache_manager->set<bool>(HAS_PATH_CACHE, key, result);
			_mutex->unlock();
			return result;
		}
		int jump_count = line_content.split(String::chr(JUMP_MARKER_OPEN), true, 1)[1].to_int();
        if (current_line_idx + 1 < _current_open_data.size()) {
		    is_parent_array = _get_line_key_part(_current_open_data[current_line_idx + 1]).begins_with("[");
        } else {
            is_parent_array = false;
        }
		current_line_idx += 1;
		search_range_end = current_line_idx + jump_count;
	}
	result = false;
	if (is_cache_enabled(HAS_PATH_CACHE)) _cache_manager->set<bool>(HAS_PATH_CACHE, key, result);
	_mutex->unlock();
	return result;
}

int PreBuiltIndexJSON::get_size(const String &p_key_path) const {
	_mutex->lock();
    const StringName key(p_key_path);
	if (is_cache_enabled(GET_SIZE_CACHE) && _cache_manager->has(GET_SIZE_CACHE, key)) {
        _mutex->unlock();
		return _cache_manager->get<int>(GET_SIZE_CACHE, key);
	}
	Dictionary slice_info = _find_container_slice(p_key_path);
	int size = 0;
	if (!slice_info.is_empty()) {
		int start_idx = slice_info["start_idx"];
		int end_idx = slice_info["end_idx"];
		int child_depth = slice_info["child_depth"];
		for (int i = start_idx; i < end_idx; ++i) {
			const String &line = _current_open_data[i];
			if (_get_line_depth(line) == child_depth) {
				size++;
			}
		}
	}
	if (is_cache_enabled(GET_SIZE_CACHE)) _cache_manager->set<int>(GET_SIZE_CACHE, key, size);
	_mutex->unlock();
	return size;
}

Array PreBuiltIndexJSON::get_keys(const String &p_key_path) const {
	_mutex->lock();
    const StringName key(p_key_path);
	if (is_cache_enabled(GET_KEYS_CACHE) && _cache_manager->has(GET_KEYS_CACHE, key)) {
        _mutex->unlock();
		return _cache_manager->get<Array>(GET_KEYS_CACHE, key);
	}
	Dictionary slice_info = _find_container_slice(p_key_path);
	Array keys;
	if (!slice_info.is_empty()) {
		int start_idx = slice_info["start_idx"];
		int end_idx = slice_info["end_idx"];
		int child_depth = slice_info["child_depth"];
        Ref<JSON> json = memnew(JSON);
		for (int i = start_idx; i < end_idx; ++i) {
			const String &line = _current_open_data[i];
			if (_get_line_depth(line) == child_depth) {
				String key_part = _get_line_key_part(line);
				Variant parsed_key = json->parse_string(key_part);
				keys.append(parsed_key);
			}
		}
	}
	if (is_cache_enabled(GET_KEYS_CACHE)) _cache_manager->set<Array>(GET_KEYS_CACHE, key, keys);
	_mutex->unlock();
	return keys;
}

PackedStringArray PreBuiltIndexJSON::get_sub_paths(const String &p_key_path) const {
	_mutex->lock();
    const StringName key(p_key_path);
	if (is_cache_enabled(GET_SUBPATHS_CACHE) && _cache_manager->has(GET_SUBPATHS_CACHE, key)) {
        _mutex->unlock();
		return _cache_manager->get<PackedStringArray>(GET_SUBPATHS_CACHE, key);
	}
	Dictionary slice_info = _find_container_slice(p_key_path);
	PackedStringArray sub_paths;
	if (!slice_info.is_empty()) {
		Array path_stack;
		String base_path = p_key_path.rstrip("/");
		if (!base_path.is_empty()) {
			path_stack = base_path.split("/");
		}
		int base_depth = path_stack.size();
        Ref<JSON> json = memnew(JSON);
		for (int i = slice_info["start_idx"]; i < (int)slice_info["end_idx"]; ++i) {
			const String &line = _current_open_data[i];
			int current_depth = _get_line_depth(line);
			int relative_depth = current_depth - base_depth - 1;
			String key_part = _get_line_key_part(line);
			Variant parsed_key = json->parse_string(key_part);
			while (path_stack.size() > base_depth + relative_depth) {
				path_stack.pop_back();
			}
			path_stack.push_back(String(parsed_key));
			sub_paths.append(String("/").join(path_stack));
		}
	}
	if (is_cache_enabled(GET_SUBPATHS_CACHE)) _cache_manager->set<PackedStringArray>(GET_SUBPATHS_CACHE, key, sub_paths);
	_mutex->unlock();
	return sub_paths;
}

Ref<PreBuiltIndexJSONOutput> PreBuiltIndexJSON::open_file(const String &p_path) {
	_mutex->lock();
	clear_caches();
	Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::ModeFlags::READ);
	if (file.is_null()) {
		_last_error = Ref<PreBuiltIndexJSONOutput>(memnew(PreBuiltIndexJSONOutput(FileAccess::get_open_error())));
		_mutex->unlock();
		return _last_error;
	}
	_current_open_file = p_path;
	_current_open_data = file->get_as_text().split("\n", false);
	_remove_trailing_empty_line(_current_open_data);
	_last_error = Ref<PreBuiltIndexJSONOutput>(memnew(PreBuiltIndexJSONOutput(PreBuiltIndexJSONOutput::OK)));
	_mutex->unlock();
	return _last_error;
}

void PreBuiltIndexJSON::open_from_string(const String &p_data) {
	_mutex->lock();
	clear_caches();
	_current_open_file = "";
	_current_open_data = p_data.split("\n", false);
	_remove_trailing_empty_line(_current_open_data);
	_mutex->unlock();
}

void PreBuiltIndexJSON::open_from_array(const PackedStringArray &p_data) {
	_mutex->lock();
	clear_caches();
	_current_open_file = "";
	_current_open_data = p_data;
	_remove_trailing_empty_line(_current_open_data);
	_mutex->unlock();
}

Ref<PreBuiltIndexJSONOutput> PreBuiltIndexJSON::reload_file() {
	if (get_opened_file().is_empty()) {
		_last_error = Ref<PreBuiltIndexJSONOutput>(memnew(PreBuiltIndexJSONOutput(PreBuiltIndexJSONOutput::ERR_FILE_NOT_FILE)));
		return _last_error;
	}
	return open_file(_current_open_file);
}

void PreBuiltIndexJSON::clear() {
	_mutex->lock();
	_current_open_data.clear();
	_build_buffer.clear();
	_current_open_file = "";
	_last_error->clear();
	clear_caches();
	_mutex->unlock();
}

void PreBuiltIndexJSON::close() {
	_mutex->lock();
	_current_open_data.clear();
	_current_open_file = "";
	_mutex->unlock();
}

void PreBuiltIndexJSON::clear_caches() {
	_cache_manager->clear_all();
}

void PreBuiltIndexJSON::clear_cache(CacheFlags p_flag) {
	_cache_manager->clear_by_flag(p_flag);
}

bool PreBuiltIndexJSON::remove_from_cache(CacheFlags p_flag, const StringName &p_key_path) {
	return _cache_manager->erase(p_flag, p_key_path);
}

Ref<PreBuiltIndexJSONOutput> PreBuiltIndexJSON::get_last_error() const {
	return _last_error;
}

bool PreBuiltIndexJSON::is_data_loaded() const {
	return !_current_open_data.is_empty();
}

String PreBuiltIndexJSON::get_opened_file() const {
	return _current_open_file;
}

void PreBuiltIndexJSON::set_cache_flags(int p_flags) {
	cache_flags = static_cast<CacheFlags>(p_flags);
}

int PreBuiltIndexJSON::get_cache_flags() const {
	return static_cast<int>(cache_flags);
}

bool PreBuiltIndexJSON::is_cache_enabled(CacheFlags p_flag) const {
	return (cache_flags & p_flag) != 0;
}

void PreBuiltIndexJSON::set_cache_enabled(CacheFlags p_flag, bool p_enabled) {
	if (p_enabled) {
		cache_flags = static_cast<CacheFlags>(cache_flags | p_flag);
	} else {
		cache_flags = static_cast<CacheFlags>(cache_flags & ~p_flag);
	}
}

bool PreBuiltIndexJSON::has_in_cache(CacheFlags p_flag, const StringName &p_key_path) const {
	return _cache_manager->has(p_flag, p_key_path);
}

int PreBuiltIndexJSON::_get_line_depth(const String &p_line) const {
	int depth = 0;
	for (int i = 0; i < p_line.length(); ++i) {
		if (p_line[i] == DEPTH_MARKER) {
			depth++;
		} else {
			break;
		}
	}
	return depth;
}

void PreBuiltIndexJSON::_remove_trailing_empty_line(PackedStringArray &p_array) const {
	if (!p_array.is_empty() && p_array[p_array.size() - 1].is_empty()) {
		p_array.remove_at(p_array.size() - 1);
	}
}

PackedStringArray PreBuiltIndexJSON::_parse_escaped_path(const String &p_path) const {
	String path = p_path.rstrip("/");
	if (path.is_empty()) return PackedStringArray();
	PackedStringArray parts;
	String current_part;
	for (int i = 0; i < path.length(); ++i) {
		char32_t c = path[i];
		if (c == U'\\') {
			if (i + 1 < path.length()) {
				current_part += path[i+1];
				i++;
			} else {
				current_part += U'\\';
			}
		} else if (c == U'/') {
			parts.append(current_part);
			current_part = "";
		} else {
			current_part += c;
		}
	}
	parts.append(current_part);
	return parts;
}

Dictionary PreBuiltIndexJSON::_find_part_in_range(const String &p_part, int p_depth, int p_start_line, int p_end_line, bool p_is_parent_array, const String &p_full_path) const {
	String key_part_to_find;
	bool is_int = p_part.is_valid_int();
	if (p_is_parent_array) {
		if (!is_int) {
			_last_error = Ref<PreBuiltIndexJSONOutput>(memnew(PreBuiltIndexJSONOutput(PreBuiltIndexJSONOutput::ERR_INVALID_PATH, "Invalid path: An array can only be indexed by an integer. Got '" + p_part + "'. Full path: " + p_full_path)));
			return Dictionary();
		}
		key_part_to_find = String("[{0}]").format(Array::make(p_part.to_int()));
	} else {
		key_part_to_find = JSON::stringify(p_part);
	}
	String search_pattern = String::chr(DEPTH_MARKER).repeat(p_depth) + key_part_to_find;
	for (int i = p_start_line; i < p_end_line; ++i) {
		const String &line = _current_open_data[i];
		if (line.begins_with(search_pattern)) {
			if (line.length() > search_pattern.length()) {
				char32_t char_after_pattern = line[search_pattern.length()];
				if (char_after_pattern == JUMP_MARKER_OPEN || char_after_pattern == VALUE_SEPARATOR) {
					Dictionary result;
					result["line_idx"] = i;
					result["key_part"] = key_part_to_find;
					return result;
				}
			}
		}
	}
	return Dictionary();
}

String PreBuiltIndexJSON::_get_line_key_part(const String &p_line) const {
	int content_start = _get_line_depth(p_line);
	if (content_start >= p_line.length()) return "";
	char32_t first_char = p_line[content_start];
	int end_pos = -1;
	if (first_char == U'[') {
		end_pos = p_line.find("]", content_start);
	} else if (first_char == U'"') {
		int current_pos = content_start + 1;
		while (current_pos < p_line.length()) {
			if (p_line[current_pos] == U'"' && p_line[current_pos - 1] != U'\\') {
				end_pos = current_pos;
				break;
			}
			current_pos++;
		}
	} else {
		return "";
	}
	if (end_pos == -1) return "";
	return p_line.substr(content_start, end_pos - content_start + 1);
}

Variant PreBuiltIndexJSON::_get_line_value(const String &p_line, int p_line_number) const {
	int separator_pos = p_line.find(String::chr(VALUE_SEPARATOR));
	if (separator_pos == -1) return Variant();
	String value_str = p_line.substr(separator_pos + 1).strip_edges();
	Ref<JSON> json = memnew(JSON);
	Variant result = json->parse_string(value_str);
	// What if the value is actually an empty value?
	//if (result.get_type() == Variant::NIL) {
	//	_last_error = Ref<PreBuiltIndexJSONOutput>(memnew(PreBuiltIndexJSONOutput(PreBuiltIndexJSONOutput::ERR_VALUE_PARSE, "Value parsing error on line " + p_line + ". Raw value string: '" + value_str + "'")));
	//	return Variant();
	//}
	return result;
}

Variant PreBuiltIndexJSON::_rebuild_container_from_slice(const PackedStringArray &p_slice, int p_base_depth, bool p_is_array) const {
	if (_last_error.is_valid() && _last_error->get_error_type() != PreBuiltIndexJSONOutput::OK) return Variant();
	if (p_is_array) {
		Array new_array;
		for (int i = 0; i < p_slice.size(); ) {
			const String &line = p_slice[i];
			if (_get_line_depth(line) != p_base_depth) {
				i++;
				continue;
			}
			if (line.contains(String::chr(JUMP_MARKER_OPEN))) {
				int jump = line.split(String::chr(JUMP_MARKER_OPEN), true, 1)[1].to_int();
				if (i + 1 + jump > p_slice.size()) {
					_last_error = Ref<PreBuiltIndexJSONOutput>(memnew(PreBuiltIndexJSONOutput(PreBuiltIndexJSONOutput::ERR_LINE_IN_JUMP_MARKER, "Corrupted jump mark found in data slice.")));
					return Variant();
				}
				PackedStringArray sub_slice = p_slice.slice(i + 1, i + 1 + jump);
				bool is_sub_array = !sub_slice.is_empty() && _get_line_key_part(sub_slice[0]).begins_with("[");
				new_array.append(_rebuild_container_from_slice(sub_slice, p_base_depth + 1, is_sub_array));
				i += jump + 1;
			} else {
				new_array.append(_get_line_value(line, -1));
				i++;
			}
		}
		return new_array;
	} else {
		Dictionary new_dict;
        Ref<JSON> json = memnew(JSON);
		for (int i = 0; i < p_slice.size(); ) {
			const String &line = p_slice[i];
			if (_get_line_depth(line) != p_base_depth) {
				i++;
				continue;
			}
			Variant key = json->parse_string(_get_line_key_part(line));
			if (line.contains(String::chr(JUMP_MARKER_OPEN))) {
				int jump = line.split(String::chr(JUMP_MARKER_OPEN), true, 1)[1].to_int();
				if (i + 1 + jump > p_slice.size()) {
					_last_error = Ref<PreBuiltIndexJSONOutput>(memnew(PreBuiltIndexJSONOutput(PreBuiltIndexJSONOutput::ERR_LINE_IN_JUMP_MARKER, "Corrupted jump mark found in data slice.")));
					i++;
					continue;
				}
				PackedStringArray sub_slice = p_slice.slice(i + 1, i + 1 + jump);
				bool is_sub_array = !sub_slice.is_empty() && _get_line_key_part(sub_slice[0]).begins_with("[");
				new_dict[key] = _rebuild_container_from_slice(sub_slice, p_base_depth + 1, is_sub_array);
				i += jump + 1;
			} else {
				new_dict[key] = _get_line_value(line, -1);
				i++;
			}
		}
		return new_dict;
	}
}

Dictionary PreBuiltIndexJSON::_find_container_slice(const String &p_key_path) const {
	_last_error->clear();
	if (!is_data_loaded()) {
		_last_error = Ref<PreBuiltIndexJSONOutput>(memnew(PreBuiltIndexJSONOutput(PreBuiltIndexJSONOutput::ERR_DATA_NOT_OPEN)));
		return Dictionary();
	}
	PackedStringArray path_parts = _parse_escaped_path(p_key_path);
	if (path_parts.is_empty() || (path_parts.size() == 1 && path_parts[0].is_empty())) {
		Dictionary result;
		result["start_idx"] = 0;
		result["end_idx"] = _current_open_data.size();
		result["child_depth"] = 1;
		return result;
	}
	int current_line_idx = 0;
	int search_range_end = _current_open_data.size();
	bool is_parent_array = _get_line_key_part(_current_open_data[0]).begins_with("[");
	Dictionary find_result;
	for (int i = 0; i < path_parts.size(); ++i) {
		const String &part_to_find = path_parts[i];
		int expected_depth = i + 1;
		find_result = _find_part_in_range(part_to_find, expected_depth, current_line_idx, search_range_end, is_parent_array, p_key_path);
		if (find_result.is_empty()) {
			if (_last_error->get_error_type() == PreBuiltIndexJSONOutput::OK) {
				_last_error = Ref<PreBuiltIndexJSONOutput>(memnew(PreBuiltIndexJSONOutput(PreBuiltIndexJSONOutput::ERR_INVALID_PATH, "Path part '" + part_to_find + "' not found.")));
			}
			return Dictionary();
		}
		current_line_idx = find_result["line_idx"];
		const String &line_content = _current_open_data[current_line_idx];
		if (i < path_parts.size() - 1 && !line_content.contains(String::chr(JUMP_MARKER_OPEN))) {
			_last_error = Ref<PreBuiltIndexJSONOutput>(memnew(PreBuiltIndexJSONOutput(PreBuiltIndexJSONOutput::ERR_INVALID_PATH, "Path expects a container, but found a value at part '" + part_to_find + "'.")));
			return Dictionary();
		}
		if (line_content.contains(String::chr(JUMP_MARKER_OPEN))) {
			int jump_count = line_content.split(String::chr(JUMP_MARKER_OPEN), true, 1)[1].to_int();
			if (jump_count > 0 && current_line_idx + 1 < _current_open_data.size()) {
				is_parent_array = _get_line_key_part(_current_open_data[current_line_idx + 1]).begins_with("[");
			} else {
                is_parent_array = false;
            }
			current_line_idx += 1;
			search_range_end = current_line_idx + jump_count;
		} else {
			search_range_end = current_line_idx + 1;
		}
	}
	if (find_result.is_empty()) {
		return Dictionary();
	}
	const String &final_line_content = _current_open_data[find_result["line_idx"]];
	if (!final_line_content.contains(String::chr(JUMP_MARKER_OPEN))) return Dictionary();
	int jump = final_line_content.split(String::chr(JUMP_MARKER_OPEN), true, 1)[1].to_int();
	int start_idx = (int)find_result["line_idx"] + 1;
	int child_depth = path_parts.size() + 1;
	Dictionary result;
	result["start_idx"] = start_idx;
	result["end_idx"] = start_idx + jump;
	result["child_depth"] = child_depth;
	return result;
}