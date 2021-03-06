/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "json.h"
#include "string_utils.h"
#include "temp_allocator.h"
#include "map.h"

namespace crown
{
namespace json
{
	static const char* next(const char* json, const char c = 0)
	{
		CE_ASSERT_NOT_NULL(json);

		if (c && c != *json)
		{
			CE_ASSERT(false, "Expected '%c' got '%c'", c, *json);
		}

		return ++json;
	}

	static const char* skip_string(const char* json)
	{
		CE_ASSERT_NOT_NULL(json);

		bool escaped = false;

		while ((*(json = next(json))) != 0)
		{
			if (*json == '"' && !escaped)
			{
				json = next(json);
				return json;
			}
			else if (*json == '\\') escaped = true;
			else escaped = false;
		}

		return json;
	}

	static const char* skip_value(const char* json)
	{
		CE_ASSERT_NOT_NULL(json);

		switch (*json)
		{
			case '"': json = skip_string(json); break;
			case '[': json = skip_block(json, '[', ']'); break;
			case '{': json = skip_block(json, '{', '}'); break;
			default: for (; *json != ',' && *json != '}' && *json != ']'; ++json) ; break;
		}

		return json;
	}

	JSONValueType::Enum type(const char* json)
	{
		CE_ASSERT_NOT_NULL(json);

		switch (*json)
		{
			case '"': return JSONValueType::STRING;
			case '{': return JSONValueType::OBJECT;
			case '[': return JSONValueType::ARRAY;
			case '-': return JSONValueType::NUMBER;
			default: return (isdigit(*json)) ? JSONValueType::NUMBER : (*json == 'n' ? JSONValueType::NIL : JSONValueType::BOOL);
		}
	}

	void parse_string(const char* json, DynamicString& string)
	{
		CE_ASSERT_NOT_NULL(json);

		if (*json == '"')
		{
			while (*(json = next(json)))
			{
				// Empty string
				if (*json == '"')
				{
					json = next(json);
					return;
				}
				else if (*json == '\\')
				{
					json = next(json);

					switch (*json)
					{
						case '"': string += '"'; break;
						case '\\': string += '\\'; break;
						case '/': string += '/'; break;
						case 'b': string += '\b'; break;
						case 'f': string += '\f'; break;
						case 'n': string += '\n'; break;
						case 'r': string += '\r'; break;
						case 't': string += '\t'; break;
						default:
						{
							CE_FATAL("Bad escape character");
							break;
						}
					}
				}
				else
				{
					string += *json;
				}
			}
		}

		CE_FATAL("Bad string");
	}

	double parse_number(const char* json)
	{
		CE_ASSERT_NOT_NULL(json);

		TempAllocator512 alloc;
	 	Array<char> number(alloc);

		if (*json == '-')
		{
			array::push_back(number, '-');
			json = next(json, '-');
		}
		while (isdigit(*json))
		{
			array::push_back(number, *json);
			json = next(json);
		}

		if (*json == '.')
		{
			array::push_back(number, '.');
			while ((*(json = next(json))) && isdigit(*json))
			{
				array::push_back(number, *json);
			}
		}

		if (*json == 'e' || *json == 'E')
		{
			array::push_back(number, *json);
			json = next(json);

			if (*json == '-' || *json == '+')
			{
				array::push_back(number, *json);
				json = next(json);
			}
			while (isdigit(*json))
			{
				array::push_back(number, *json);
				json = next(json);
			}
		}

		// Ensure null terminated
		array::push_back(number, '\0');
		return parse_double(array::begin(number));
	}

	bool parse_bool(const char* json)
	{
		CE_ASSERT_NOT_NULL(json);

		switch (*json)
		{
			case 't':
			{
				json = next(json, 't');
				json = next(json, 'r');
				json = next(json, 'u');
				json = next(json, 'e');
				return true;
			}
			case 'f':
			{
				json = next(json, 'f');
				json = next(json, 'a');
				json = next(json, 'l');
				json = next(json, 's');
				json = next(json, 'e');
				return false;
			}
			default:
			{
				CE_FATAL("Bad boolean");
				return false;
			}
		}
	}

	int32_t parse_int(const char* json)
	{
		return (int32_t) parse_number(json);
	}

	float parse_float(const char* json)
	{
		return (float) parse_number(json);
	}

	void parse_array(const char* json, Array<const char*>& array)
	{
		CE_ASSERT_NOT_NULL(json);

		if (*json == '[')
		{
			json = next(json, '[');
			json = skip_spaces(json);

			if (*json == ']')
			{
				json = next(json, ']');
				return;
			}

			while (*json)
			{
				array::push_back(array, json);

				json = skip_value(json);
				json = skip_spaces(json);

				if (*json == ']')
				{
					json = next(json, ']');
					return;
				}

				json = next(json, ',');
				json = skip_spaces(json);
			}
		}

		CE_FATAL("Bad array");
	}

	void parse_object(const char* json, Map<DynamicString, const char*>& object)
	{
		CE_ASSERT_NOT_NULL(json);

		if (*json == '{')
		{
			json = next(json, '{');

			json = skip_spaces(json);

			if (*json == '}')
			{
				next(json, '}');
				return;
			}

			while (*json)
			{
				DynamicString key;
				parse_string(json, key);

				json = skip_string(json);
				json = skip_spaces(json);
				json = next(json, ':');
				json = skip_spaces(json);

				map::set(object, key, json);

				json = skip_value(json);
				json = skip_spaces(json);

				if (*json == '}')
				{
					next(json, '}');
					return;
				}

				json = next(json, ',');
				json = skip_spaces(json);
			}
		}

		CE_FATAL("Bad object");
	}
} // namespace json
} // namespace crown
