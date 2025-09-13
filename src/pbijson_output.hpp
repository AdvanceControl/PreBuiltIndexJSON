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

#include "godot_cpp/classes/ref_counted.hpp"
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/classes/global_constants.hpp>

using namespace godot;

class PreBuiltIndexJSONOutput : public RefCounted {
	GDCLASS(PreBuiltIndexJSONOutput, RefCounted)

public:
	enum ErrorType {
		OK = 0,
		ERR_BUILT_IN_METHOD,
		ERR_JSON_PARSE,
		ERR_UNSUPPORTED_TYPE,
        ERR_FILE_NOT_FILE,
		ERR_DATA_NOT_OPEN,
		ERR_VALUE_PARSE,
		ERR_INVALID_PATH,
		ERR_LINE_IN_JUMP_MARKER,
		ERR_FILE_HEADER,
		ERR_HASH,
		ERR_FORMAT,
	};

protected:
	static void _bind_methods();

private:
	ErrorType _error_type = OK;
	Error _godot_error = Error::OK;
	String _data;
	String _message;
	int _line = -1;

public:
	PreBuiltIndexJSONOutput();
	~PreBuiltIndexJSONOutput() override = default;

	// Constructors
	PreBuiltIndexJSONOutput(ErrorType p_error_type);
	PreBuiltIndexJSONOutput(Error p_error);
	PreBuiltIndexJSONOutput(ErrorType p_error_type, const String &p_message);
	PreBuiltIndexJSONOutput(ErrorType p_error_type, const String &p_message, int p_line);
	PreBuiltIndexJSONOutput(const String &p_data);

	// Getters
	ErrorType get_error_type() const;
	Error get_godot_error() const;
	String get_data() const;
	String get_message() const;
	int get_line() const;
	
	bool has_message() const;
	bool has_line() const;
	bool has_data() const;

	// Setters
	void set_error_type(ErrorType p_error_type);
	void set_godot_error(Error p_error);
	void set_data(const String &p_data);
	void set_message(const String &p_message);
	void set_line(int p_line);
	
	void clear();
	void set_to(const Ref<PreBuiltIndexJSONOutput> &p_other);
};

VARIANT_ENUM_CAST(PreBuiltIndexJSONOutput::ErrorType);