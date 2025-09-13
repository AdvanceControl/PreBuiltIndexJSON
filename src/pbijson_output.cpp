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
#include "pbijson_output.hpp"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void PreBuiltIndexJSONOutput::_bind_methods() {

	ClassDB::bind_integer_constant(get_class_static(), "ErrorType", "OK", OK);
	ClassDB::bind_integer_constant(get_class_static(), "ErrorType", "ERR_BUILT_IN_METHOD", ERR_BUILT_IN_METHOD);
	ClassDB::bind_integer_constant(get_class_static(), "ErrorType", "ERR_JSON_PARSE", ERR_JSON_PARSE);
	ClassDB::bind_integer_constant(get_class_static(), "ErrorType", "ERR_UNSUPPORTED_TYPE", ERR_UNSUPPORTED_TYPE);
	ClassDB::bind_integer_constant(get_class_static(), "ErrorType", "ERR_FILE_NOT_FILE", ERR_FILE_NOT_FILE);
	ClassDB::bind_integer_constant(get_class_static(), "ErrorType", "ERR_DATA_NOT_OPEN", ERR_DATA_NOT_OPEN);
	ClassDB::bind_integer_constant(get_class_static(), "ErrorType", "ERR_VALUE_PARSE", ERR_VALUE_PARSE);
	ClassDB::bind_integer_constant(get_class_static(), "ErrorType", "ERR_INVALID_PATH", ERR_INVALID_PATH);
	ClassDB::bind_integer_constant(get_class_static(), "ErrorType", "ERR_LINE_IN_JUMP_MARKER", ERR_LINE_IN_JUMP_MARKER);
	ClassDB::bind_integer_constant(get_class_static(), "ErrorType", "ERR_FILE_HEADER", ERR_FILE_HEADER);
	ClassDB::bind_integer_constant(get_class_static(), "ErrorType", "ERR_FILE_FORMAT", ERR_FORMAT);
    

    ClassDB::bind_method(D_METHOD("get_message"), &PreBuiltIndexJSONOutput::get_message);
    ClassDB::bind_method(D_METHOD("get_error_type"), &PreBuiltIndexJSONOutput::get_error_type);
    ClassDB::bind_method(D_METHOD("get_error"), &PreBuiltIndexJSONOutput::get_godot_error);
    ClassDB::bind_method(D_METHOD("get_data"), &PreBuiltIndexJSONOutput::get_data);
    ClassDB::bind_method(D_METHOD("get_line"), &PreBuiltIndexJSONOutput::get_line);
    ClassDB::bind_method(D_METHOD("has_line"), &PreBuiltIndexJSONOutput::has_line);
    ClassDB::bind_method(D_METHOD("has_message"), &PreBuiltIndexJSONOutput::has_message);
    ClassDB::bind_method(D_METHOD("has_data"), &PreBuiltIndexJSONOutput::has_data);

}


PreBuiltIndexJSONOutput::PreBuiltIndexJSONOutput() {
	clear();
}

PreBuiltIndexJSONOutput::PreBuiltIndexJSONOutput(ErrorType p_error_type) {
	clear();
	_error_type = p_error_type;
}

PreBuiltIndexJSONOutput::PreBuiltIndexJSONOutput(Error p_error) {
	clear();
	_error_type = ERR_BUILT_IN_METHOD;
	_godot_error = p_error;
}

PreBuiltIndexJSONOutput::PreBuiltIndexJSONOutput(ErrorType p_error_type, const String &p_message) {
	clear();
	_error_type = p_error_type;
	_message = p_message;
}

PreBuiltIndexJSONOutput::PreBuiltIndexJSONOutput(ErrorType p_error_type, const String &p_message, int p_line) {
	clear();
	_error_type = p_error_type;
	_message = p_message;
	_line = p_line;
}

PreBuiltIndexJSONOutput::PreBuiltIndexJSONOutput(const String &p_data) {
	clear();
	_error_type = OK;
	_data = p_data;
}

PreBuiltIndexJSONOutput::ErrorType PreBuiltIndexJSONOutput::get_error_type() const {
	return _error_type;
}

Error PreBuiltIndexJSONOutput::get_godot_error() const {
	if (_error_type != ERR_BUILT_IN_METHOD) {
		UtilityFunctions::printerr("get_godot_error() called on an incompatible error type.", __FUNCTION__, __FILE__, __LINE__);
		return Error::OK;
	}
	return _godot_error;
}

String PreBuiltIndexJSONOutput::get_data() const {
	return _data;
}

String PreBuiltIndexJSONOutput::get_message() const {
	if (!has_message()) {
		UtilityFunctions::printerr("get_message() called on an incompatible error type.", __FUNCTION__, __FILE__, __LINE__);
		return "";
	}
	return _message;
}

int PreBuiltIndexJSONOutput::get_line() const {
	if (!has_line()) {
		UtilityFunctions::printerr("get_line() called on an incompatible error type.", __FUNCTION__, __FILE__, __LINE__);
		return -1;
	}
	return _line;
}

bool PreBuiltIndexJSONOutput::has_message() const {
	switch (_error_type) {
		case ERR_BUILT_IN_METHOD:return false;
		case ERR_JSON_PARSE:return true;
		case ERR_UNSUPPORTED_TYPE:return true;
        case ERR_FILE_NOT_FILE:return false;
		case ERR_DATA_NOT_OPEN:return false;
		case ERR_VALUE_PARSE:return true;
		case ERR_INVALID_PATH:return true;
		case ERR_LINE_IN_JUMP_MARKER:return true;
		case ERR_FILE_HEADER:return true;
		case ERR_HASH:return true;
		case ERR_FORMAT:return true;
		default:
			return false;
	}
}

bool PreBuiltIndexJSONOutput::has_line() const {
	switch (_error_type) {
		case ERR_BUILT_IN_METHOD:return false;
		case ERR_JSON_PARSE:return true;
		case ERR_UNSUPPORTED_TYPE:return true;
        case ERR_FILE_NOT_FILE:return false;
		case ERR_DATA_NOT_OPEN:return false;
		case ERR_VALUE_PARSE:return true;
		case ERR_INVALID_PATH:return false;
		case ERR_LINE_IN_JUMP_MARKER:return false;
		case ERR_FILE_HEADER:return false;
		case ERR_HASH:return false;
		case ERR_FORMAT:return false;
		default:
			return false;
	}
}

bool PreBuiltIndexJSONOutput::has_data() const {
	return (_error_type == OK && !_data.is_empty());
}

void PreBuiltIndexJSONOutput::set_error_type(ErrorType p_error_type) {
	_error_type = p_error_type;
}

void PreBuiltIndexJSONOutput::set_godot_error(Error p_error) {
	_error_type = ERR_BUILT_IN_METHOD;
	_godot_error = p_error;
}

void PreBuiltIndexJSONOutput::set_data(const String &p_data) {
	_data = p_data;
}

void PreBuiltIndexJSONOutput::set_message(const String &p_message) {
	_message = p_message;
}

void PreBuiltIndexJSONOutput::set_line(int p_line) {
	_line = p_line;
}

void PreBuiltIndexJSONOutput::clear() {
	_error_type = OK;
	_godot_error = Error::OK;
	_data = "";
	_message = "";
	_line = -1;
}

void PreBuiltIndexJSONOutput::set_to(const Ref<PreBuiltIndexJSONOutput> &p_other) {
	if (p_other.is_null()) {
		clear();
		return;
	}
	this->_error_type = p_other->get_error_type();
	this->_godot_error = p_other->_godot_error;
	this->_data = p_other->_data;
	this->_message = p_other->_message;
	this->_line = p_other->_line;
}