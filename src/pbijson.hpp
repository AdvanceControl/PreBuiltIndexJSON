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
#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/mutex.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include "pbijson_output.hpp"

#include <type_traits> // For std::is_same_v

using namespace godot;

// Forward declaration
class CacheManager;

class PreBuiltIndexJSON : public RefCounted {
	GDCLASS(PreBuiltIndexJSON, RefCounted)

public:
	// CRITICAL FIX: Changed from `enum class` to a plain `enum` inside the class.
	// This makes it compatible with Godot's binding system while still namespacing it.
	enum CacheFlags {
		NONE             = 0,
		VALUE_CACHE      = 1 << 0,
		HAS_PATH_CACHE   = 1 << 1,
		GET_SIZE_CACHE   = 1 << 2,
		GET_SUBPATHS_CACHE = 1 << 3,
		GET_KEYS_CACHE   = 1 << 4,
		ALL              = (1 << 5) - 1,
	};

protected:
	static void _bind_methods();

private:
	const char32_t DEPTH_MARKER = U':';
	const char32_t VALUE_SEPARATOR = U'>';
	const char32_t JUMP_MARKER_OPEN = U'<';
	
	Ref<Mutex> _mutex;
	String _current_open_file;
	PackedStringArray _current_open_data;
	PackedStringArray _build_buffer;
	
	CacheFlags cache_flags = ALL;

	class CacheManager* _cache_manager;
	mutable Ref<PreBuiltIndexJSONOutput> _last_error;

	void _build_flat_index_recursive(const Variant &p_current_value, int p_depth, Dictionary &p_container_lines);
	void _add_jump_marks_to_buffer(Dictionary &p_container_lines);
	int _get_line_depth(const String &p_line) const;
	String _get_line_key_part(const String &p_line) const;
	Variant _get_line_value(const String &p_line, int p_line_number) const;
	PackedStringArray _parse_escaped_path(const String &p_path) const;
	Dictionary _find_part_in_range(const String &p_part, int p_depth, int p_start_line, int p_end_line, bool p_is_parent_array, const String &p_full_path) const;
	Variant _rebuild_container_from_slice(const PackedStringArray &p_slice, int p_base_depth, bool p_is_array) const;
    Dictionary _find_container_slice(const String &p_key_path) const;
    void _remove_trailing_empty_line(PackedStringArray &p_array) const;
	Ref<PreBuiltIndexJSONOutput> _open_data(const PackedStringArray &p_data,const bool &ignore_hash = false);

	String _generate_file_header(const Dictionary &data);
	Dictionary _parse_header(const String &p_line);
	Ref<PreBuiltIndexJSONOutput> _build(const String &p_json_text);
public:
	PreBuiltIndexJSON();
	~PreBuiltIndexJSON() override;

	// Build methods
	Ref<PreBuiltIndexJSONOutput> build_from_string(const String &p_json_text);
	Ref<PreBuiltIndexJSONOutput> build_from_file(const String &p_json_file);
	Ref<PreBuiltIndexJSONOutput> build_from_file_to(const String &p_json_file, const String &p_target_path);

	// Data loading methods
	Ref<PreBuiltIndexJSONOutput> open_file(const String &p_path,const bool &ignore_hash = false);
	Ref<PreBuiltIndexJSONOutput> open_from_string(const String &p_data,const bool &ignore_hash = false);
	Ref<PreBuiltIndexJSONOutput> open_from_array(const PackedStringArray &p_data,const bool &ignore_hash = false);
	Ref<PreBuiltIndexJSONOutput> reload_file(const bool &ignore_hash = false);

	// Data query methods
	Variant get_value(const String &p_key_path, const Variant &p_default = Variant()) const;
    bool has_path(const String &p_key_path) const;
    int get_size(const String &p_key_path) const;
    Array get_keys(const String &p_key_path) const;
    PackedStringArray get_sub_paths(const String &p_key_path) const;

	// State and cache management
	void clear();
	void close();
	void clear_caches();
	void clear_cache(CacheFlags p_flag);
	bool remove_from_cache(CacheFlags p_flag, const StringName &p_key_path);
	
	// Getters & Setters for properties
	Ref<PreBuiltIndexJSONOutput> get_last_error() const;
	bool is_data_loaded() const;
	String get_opened_file() const;

	void set_cache_flags(int p_flags);
	int get_cache_flags() const;
	bool is_cache_enabled(CacheFlags p_flag) const;
	void set_cache_enabled(CacheFlags p_flag, bool p_enabled);
    bool has_in_cache(CacheFlags p_flag, const StringName &p_key_path) const;

	
	static String get_pbijson_format();
};

// Now that the class is defined, we can add the macro.
VARIANT_ENUM_CAST(PreBuiltIndexJSON::CacheFlags);