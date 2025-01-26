// Copyright (c) 2024  Made to Order Software Corp.  All Rights Reserved
//
// https://snapwebsites.org/project/prinbee
// contact@m2osw.com
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.


/** \file
 * \brief Lexer of the Prinbee Query Language.
 *
 * The Prinbee Query Language (PBQL) is an SQL-like language. This file
 * transforms the input data in tokens that the parser can then use to
 * create statements.
 *
 * The lexer supports tokens that include keywords (SELECT), identifiers
 * (column name), numbers (integers, floating points), operators (for
 * expressions; +, -, *, /, etc.).
 */

// self
//
#include    "prinbee/pbql/node.h"

#include    "prinbee/exception.h"


// snaplogger
//
#include    <snaplogger/message.h>


// snapdev
//
#include    <snapdev/escape_special_regex_characters.h>
#include    <snapdev/floating_point_to_string.h>
#include    <snapdev/not_reached.h>
#include    <snapdev/not_used.h>
#include    <snapdev/string_replace_many.h>
#include    <snapdev/safe_variable.h>
#include    <snapdev/to_lower.h>
#include    <snapdev/to_upper.h>


// last include
//
#include    <snapdev/poison.h>



namespace prinbee
{
namespace pbql
{



namespace
{



struct token_name_t
{
    token_t                 f_token = token_t::TOKEN_UNKNOWN;
    char const              f_name[32] = {};
};

#define TOKEN_NAME(name)    { token_t::TOKEN_##name, #name }
#define TOKEN_CHAR(name)    { token_t::TOKEN_##name, { static_cast<char>(token_t::TOKEN_##name), '\0' } }

struct token_name_t const g_token_names[] =
{
    // that one doesn't work because EOF gets transformed to -1 ahead of time
    //
    //TOKEN_NAME(EOF),
    { token_t::TOKEN_EOF, "EOF" },

    TOKEN_NAME(UNKNOWN),

    TOKEN_CHAR(BITWISE_XOR),
    TOKEN_CHAR(MODULO),
    TOKEN_CHAR(BITWISE_AND),
    TOKEN_CHAR(OPEN_PARENTHESIS),
    TOKEN_CHAR(CLOSE_PARENTHESIS),
    TOKEN_CHAR(MULTIPLY),
    TOKEN_CHAR(PLUS),
    TOKEN_CHAR(COMMA),
    TOKEN_CHAR(MINUS),
    TOKEN_CHAR(PERIOD),
    TOKEN_CHAR(DIVIDE),
    TOKEN_CHAR(COLON),
    TOKEN_CHAR(SEMI_COLON),
    TOKEN_CHAR(LESS),
    TOKEN_CHAR(EQUAL),
    TOKEN_CHAR(GREATER),
    TOKEN_CHAR(ABSOLUTE_VALUE),
    TOKEN_CHAR(OPEN_BRACKET),
    TOKEN_CHAR(CLOSE_BRACKET),
    TOKEN_CHAR(POWER),
    TOKEN_CHAR(BITWISE_OR),
    TOKEN_CHAR(REGULAR_EXPRESSION),

    //TOKEN_NAME(other), -- not a token

    TOKEN_NAME(IDENTIFIER),
    TOKEN_NAME(STRING),
    TOKEN_NAME(INTEGER),
    TOKEN_NAME(FLOATING_POINT),

    TOKEN_NAME(NOT_EQUAL),
    TOKEN_NAME(LESS_EQUAL),
    TOKEN_NAME(GREATER_EQUAL),
    TOKEN_NAME(SQUARE_ROOT),
    TOKEN_NAME(CUBE_ROOT),
    TOKEN_NAME(SCOPE),
    TOKEN_NAME(SHIFT_LEFT),
    TOKEN_NAME(SHIFT_RIGHT),
    TOKEN_NAME(UNMATCHED_REGULAR_EXPRESSION),
    TOKEN_NAME(STRING_CONCAT),

    TOKEN_NAME(ALL_FIELDS),
    TOKEN_NAME(AT),
    TOKEN_NAME(BETWEEN),
    TOKEN_NAME(CAST),
    TOKEN_NAME(FALSE),
    TOKEN_NAME(FUNCTION_CALL),
    TOKEN_NAME(ILIKE),
    TOKEN_NAME(LIKE),
    TOKEN_NAME(LIST),
    TOKEN_NAME(LOGICAL_OR),
    TOKEN_NAME(LOGICAL_AND),
    TOKEN_NAME(LOGICAL_NOT),
    { token_t::TOKEN_NULL, "NULL" },  // NULL is transformed to 0 ahead of time (see EOF above)
    TOKEN_NAME(TRUE),
    TOKEN_NAME(TYPE),

    TOKEN_NAME(BOOLEAN),
    TOKEN_NAME(NUMBER),

    //TOKEN_NAME(max) -- not a token
};



} // no name namespace



char const * cast_type_to_as2js_type(type_t const type)
{
    switch(type)
    {
    case type_t::TYPE_BOOLEAN:
        return "Boolean";

    case type_t::TYPE_INT1:
    case type_t::TYPE_INT2:
    case type_t::TYPE_INT4:
    case type_t::TYPE_INT8:
    case type_t::TYPE_INT16:
    case type_t::TYPE_INT32:
    case type_t::TYPE_INT64:
    case type_t::TYPE_UNSIGNED_INT1:
    case type_t::TYPE_UNSIGNED_INT2:
    case type_t::TYPE_UNSIGNED_INT4:
    case type_t::TYPE_UNSIGNED_INT8:
    case type_t::TYPE_UNSIGNED_INT16:
    case type_t::TYPE_UNSIGNED_INT32:
    case type_t::TYPE_UNSIGNED_INT64:
        return "Integer";

    case type_t::TYPE_FLOAT4:
    case type_t::TYPE_FLOAT8:
    case type_t::TYPE_FLOAT10:
        return "Number";

    case type_t::TYPE_TEXT:
        return "String";

    }

    return "undefined";
}


std::string to_string(token_t t, bool quote_char)
{
    std::uint32_t i(0);
    std::uint32_t j(std::size(g_token_names));
    while(i < j)
    {
        std::uint32_t p((j - i) / 2 + i);
        int const r(static_cast<int>(g_token_names[p].f_token) - static_cast<int>(t));
        if(r < 0)
        {
            i = p + 1;
        }
        else if(r > 0)
        {
            j = p;
        }
        else
        {
            if(quote_char && g_token_names[p].f_name[1] == '\0')
            {
                return std::string(1, '\'') + g_token_names[p].f_name + '\'';
            }
            return g_token_names[p].f_name;
        }
    }

    return std::string();
}



node::node(token_t token, location const & l)
    : f_token(token)
    , f_location(l)
{
    switch(f_token)
    {
    case token_t::TOKEN_EOF:
    case token_t::TOKEN_BITWISE_XOR:
    case token_t::TOKEN_MODULO:
    case token_t::TOKEN_BITWISE_AND:
    case token_t::TOKEN_OPEN_PARENTHESIS:
    case token_t::TOKEN_CLOSE_PARENTHESIS:
    case token_t::TOKEN_MULTIPLY:
    case token_t::TOKEN_PLUS:
    case token_t::TOKEN_COMMA:
    case token_t::TOKEN_MINUS:
    case token_t::TOKEN_PERIOD:
    case token_t::TOKEN_DIVIDE:
    case token_t::TOKEN_COLON:
    case token_t::TOKEN_SEMI_COLON:
    case token_t::TOKEN_EQUAL:
    case token_t::TOKEN_ABSOLUTE_VALUE:
    case token_t::TOKEN_OPEN_BRACKET:
    case token_t::TOKEN_CLOSE_BRACKET:
    case token_t::TOKEN_POWER:
    case token_t::TOKEN_BITWISE_OR:
    case token_t::TOKEN_REGULAR_EXPRESSION:
    case token_t::TOKEN_IDENTIFIER:
    case token_t::TOKEN_STRING:
    case token_t::TOKEN_INTEGER:
    case token_t::TOKEN_FLOATING_POINT:
    case token_t::TOKEN_NOT_EQUAL:
    case token_t::TOKEN_LESS:
    case token_t::TOKEN_LESS_EQUAL:
    case token_t::TOKEN_GREATER:
    case token_t::TOKEN_GREATER_EQUAL:
    case token_t::TOKEN_SQUARE_ROOT:
    case token_t::TOKEN_CUBE_ROOT:
    case token_t::TOKEN_SCOPE:
    case token_t::TOKEN_SHIFT_LEFT:
    case token_t::TOKEN_SHIFT_RIGHT:
    case token_t::TOKEN_UNMATCHED_REGULAR_EXPRESSION:
    case token_t::TOKEN_STRING_CONCAT:
    case token_t::TOKEN_ALL_FIELDS:
    case token_t::TOKEN_AT:
    case token_t::TOKEN_BETWEEN:
    case token_t::TOKEN_CAST:
    case token_t::TOKEN_FALSE:
    case token_t::TOKEN_FUNCTION_CALL:
    case token_t::TOKEN_ILIKE:
    case token_t::TOKEN_LIKE:
    case token_t::TOKEN_LIST:
    case token_t::TOKEN_LOGICAL_OR:
    case token_t::TOKEN_LOGICAL_AND:
    case token_t::TOKEN_LOGICAL_NOT:
    case token_t::TOKEN_NULL:
    case token_t::TOKEN_TRUE:
    case token_t::TOKEN_TYPE:
        break;

    default:
        throw invalid_token(
              "node created with an invalid token ("
            + std::to_string(static_cast<int>(f_token))
            + ").");

    }
}


token_t node::get_token() const
{
    return f_token;
}


location const & node::get_location() const
{
    return f_location;
}


bool node::is_literal(token_t match_type) const
{
    switch(f_token)
    {
    case token_t::TOKEN_STRING:
        if(match_type == token_t::TOKEN_NUMBER
        || match_type == token_t::TOKEN_FLOATING_POINT)
        {
            char * e(nullptr);
            errno = 0;
            snapdev::NOT_USED(strtold(f_string.c_str(), &e));
            return errno == 0
                && (e == nullptr || *e == '\0')
                && f_string.c_str() != e;
        }
        if(match_type == token_t::TOKEN_INTEGER)
        {
            // TBD: should we support binary, octal, hexadecimal notation too?
            //
            std::size_t const max(f_string.length());
            if(max == 0)
            {
                return false;
            }
            std::size_t idx(0);
            if(f_string[0] == '+'
            || f_string[0] == '-')
            {
                ++idx;
                if(idx >= max)
                {
                    return false;
                }
            }
            for(; idx < max; ++idx)
            {
                char c(f_string[idx]);
                if(c < '0' || c > '9')
                {
                    return false;
                }
            }
            return true;
        }
        if(match_type == token_t::TOKEN_BOOLEAN)
        {
            std::string const keyword(get_string_upper());
            if(keyword.length() <= 4
            && std::string("TRUE").starts_with(keyword))
            {
                return true;
            }
            if(keyword.length() <= 5
            && std::string("FALSE").starts_with(keyword))
            {
                return true;
            }
            return false;
        }
        return match_type == token_t::TOKEN_UNKNOWN || match_type == f_token;

    case token_t::TOKEN_INTEGER:
        return match_type == token_t::TOKEN_UNKNOWN || match_type == f_token || match_type == token_t::TOKEN_NUMBER || match_type == token_t::TOKEN_BOOLEAN;

    case token_t::TOKEN_FLOATING_POINT:
        return match_type == token_t::TOKEN_UNKNOWN || match_type == f_token || match_type == token_t::TOKEN_NUMBER || match_type == token_t::TOKEN_BOOLEAN;

    case token_t::TOKEN_NULL:
        return match_type == token_t::TOKEN_UNKNOWN || match_type == f_token;

    case token_t::TOKEN_TRUE:
    case token_t::TOKEN_FALSE:
        return match_type == token_t::TOKEN_UNKNOWN || match_type == token_t::TOKEN_BOOLEAN;

    default:
        return false;

    }
    snapdev::NOT_REACHED();
}


void node::set_string(std::string const & s)
{
    f_string = s;
}


std::string const & node::get_string() const
{
    return f_string;
}


std::string node::get_string_lower() const
{
    return snapdev::to_lower(f_string);
}


std::string node::get_string_upper() const
{
    return snapdev::to_upper(f_string);
}


std::string node::get_string_auto_convert() const
{
    switch(f_token)
    {
    case token_t::TOKEN_STRING:
        return f_string;

    case token_t::TOKEN_INTEGER:
        return to_string(f_integer);

    case token_t::TOKEN_FLOATING_POINT:
        return snapdev::floating_point_to_string<double, char>(f_floating_point);

    case token_t::TOKEN_NULL:
        return "null";

    case token_t::TOKEN_TRUE:
        return "true";

    case token_t::TOKEN_FALSE:
        return "false";

    default:
        throw logic_error("node is not a literal and it cannot be converted to a string.");

    }
}


bool node::get_boolean_auto_convert() const
{
    switch(f_token)
    {
    case token_t::TOKEN_STRING:
        {
            std::string const keyword(get_string_upper());
            if(keyword.length() <= 4
            && std::string("TRUE").starts_with(keyword))
            {
                return true;
            }
            if(keyword.length() <= 5
            && std::string("FALSE").starts_with(keyword))
            {
                return false;
            }
        }
        break;

    case token_t::TOKEN_INTEGER:
        return !f_integer.is_zero();

    case token_t::TOKEN_FLOATING_POINT:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
        return f_floating_point != 0.0;
#pragma GCC diagnostic pop

    case token_t::TOKEN_TRUE:
        return true;

    case token_t::TOKEN_FALSE:
        return false;

    default:
        break;

    }
    throw logic_error("node is not a literal representing a Boolean and as a result it cannot be converted to a Boolean.");
}


int512_t node::get_integer_auto_convert() const
{
    switch(f_token)
    {
    case token_t::TOKEN_STRING:
        // the f_integer parameter is not otherwise used when f_token is TOKEN_STRING
        //
        if(f_string.empty())
        {
            throw invalid_number(
                  "string \""
                + f_string
                + "\" does not represent a valid integer.");
        }
        const_cast<node *>(this)->f_integer.from_string(f_string);
        [[fallthrough]];
    case token_t::TOKEN_INTEGER:
        return f_integer;

    case token_t::TOKEN_FLOATING_POINT:
        // TODO: support "all" bits (implement that in int512.cpp--we already have the opposite)
        return static_cast<std::int64_t>(f_floating_point);

    default:
        throw logic_error("node is not a literal representing a number and it cannot be converted to an integer.");

    }
}


long double node::get_floating_point_auto_convert() const
{
    switch(f_token)
    {
    case token_t::TOKEN_STRING:
        // the f_floating_point parameter is not otherwise used when f_token is TOKEN_STRING
        //
        {
            char * e(nullptr);
            errno = 0;
            const_cast<node *>(this)->f_floating_point = strtold(f_string.c_str(), &e);
            if(errno != 0
            || (e != nullptr && *e != '\0')
            || f_string.c_str() == e)
            {
                throw invalid_number(
                      "string \""
                    + f_string
                    + "\" does not represent a valid floating point number.");
            }
        }
        [[fallthrough]];
    case token_t::TOKEN_FLOATING_POINT:
        return f_floating_point;

    case token_t::TOKEN_INTEGER:
        return f_integer.to_floating_point();

    default:
        throw logic_error("node is not a literal representing a number and it cannot be converted to a floating point.");

    }
}


void node::set_integer(int512_t i)
{
    f_integer = i;
}


int512_t node::get_integer()
{
    return f_integer;
}


void node::set_floating_point(long double d)
{
    f_floating_point = d;
}


long double node::get_floating_point()
{
    return f_floating_point;
}


node::pointer_t node::get_parent() const
{
    return f_parent.lock();
}


node::pointer_t node::get_child(int position) const
{
    if(static_cast<std::size_t>(position) >= f_children.size())
    {
        throw out_of_range("child " + std::to_string(position) + " does not exist.");
    }

    return f_children[position];
}


std::size_t node::get_children_size() const
{
    return f_children.size();
}


void node::set_child(int position, pointer_t child)
{
    // make sure it's not out of bounds
    //
    if(static_cast<std::size_t>(position) >= f_children.size())
    {
        throw out_of_range("child " + std::to_string(position) + " does not exist, it cannot be replaced.");
    }

    child->f_parent = shared_from_this();
    f_children[position] = child;
}


void node::insert_child(int position, pointer_t child)
{
    // if position is -1, do an append
    //
    if(static_cast<std::size_t>(position) == static_cast<std::size_t>(-1))
    {
        position = f_children.size();
    }

    // if end of list, just do an append
    //
    if(static_cast<std::size_t>(position) == f_children.size())
    {
        child->f_parent = shared_from_this();
        f_children.insert(f_children.end(), child);
        return;
    }

    // make sure it's not out of bounds
    //
    if(static_cast<std::size_t>(position) >= f_children.size())
    {
        throw out_of_range("child " + std::to_string(position) + " does not exist.");
    }

    child->f_parent = shared_from_this();
    f_children.insert(std::next(f_children.begin(), position), child);
}


std::string node::to_as2js() const
{
#ifdef _DEBUG
    static bool inside = false;
    if(inside)
    {
        throw logic_error("recursive_to_as2js() called to_as2js() directly."); // LCOV_EXCL_LINE
    }
    snapdev::safe_variable<bool> safe(inside, true, false);
#endif
    f_temp_counter = 0;
    return recursive_to_as2js();
}


std::string node::recursive_to_as2js() const
{
    switch(f_token)
    {
    case token_t::TOKEN_BITWISE_XOR:
        return f_children[0]->recursive_to_as2js() + "^" + f_children[1]->recursive_to_as2js();

    case token_t::TOKEN_MODULO:
        return f_children[0]->recursive_to_as2js() + "%" + f_children[1]->recursive_to_as2js();

    case token_t::TOKEN_BITWISE_AND:
        return f_children[0]->recursive_to_as2js() + "&" + f_children[1]->recursive_to_as2js();

    case token_t::TOKEN_MULTIPLY:
        return f_children[0]->recursive_to_as2js() + "*" + f_children[1]->recursive_to_as2js();

    case token_t::TOKEN_PLUS:
        if(f_children.size() == 2)
        {
            return f_children[0]->recursive_to_as2js() + '+' + f_children[1]->recursive_to_as2js();
        }
        else
        {
            throw logic_error("identity (+) operator found in the to_as2js() function."); // LCOV_EXCL_LINE
        }

    case token_t::TOKEN_MINUS:
        if(f_children.size() == 2)
        {
            return f_children[0]->recursive_to_as2js() + '-' + f_children[1]->recursive_to_as2js();
        }
        else
        {
            return '-' + f_children[0]->recursive_to_as2js();
        }

    case token_t::TOKEN_PERIOD:
        return f_children[0]->recursive_to_as2js() + "." + f_children[1]->recursive_to_as2js();

    case token_t::TOKEN_DIVIDE:
        return f_children[0]->recursive_to_as2js() + "/" + f_children[1]->recursive_to_as2js();

    case token_t::TOKEN_LESS:
        return f_children[0]->recursive_to_as2js() + "<" + f_children[1]->recursive_to_as2js();

    case token_t::TOKEN_EQUAL:
        return f_children[0]->recursive_to_as2js() + "==" + f_children[1]->recursive_to_as2js();

    case token_t::TOKEN_GREATER:
        return f_children[0]->recursive_to_as2js() + ">" + f_children[1]->recursive_to_as2js();

    case token_t::TOKEN_POWER:
        return '(' + f_children[0]->recursive_to_as2js() + "**" + f_children[1]->recursive_to_as2js() + ')';

    case token_t::TOKEN_BITWISE_OR:
        return f_children[0]->recursive_to_as2js() + "|" + f_children[1]->recursive_to_as2js();

    case token_t::TOKEN_REGULAR_EXPRESSION:
        return "new RegExp(" + f_children[1]->recursive_to_as2js() + ").test(" + f_children[0]->recursive_to_as2js() + ")";

    case token_t::TOKEN_IDENTIFIER:
        return f_string;

    case token_t::TOKEN_STRING:
        // add the quotes; also use double quotes and make sure to escape
        // them if any are present in the string; we also escape all the
        // special characters that can be added using E'...'
        //
        return '"' + snapdev::string_replace_many(
                f_string
                , {
                    {"\"", "\\\""},
                    {"\b", "\\b"},
                    {"\f", "\\f"},
                    {"\n", "\\n"},  // the \n and \r are very important in strings otherwise the as2js lexer breaks
                    {"\r", "\\r"},
                    {"\t", "\\t"},
                    {"\v", "\\v"},
                }) + '"';

    case token_t::TOKEN_INTEGER:
        return to_string(f_integer);

    case token_t::TOKEN_FLOATING_POINT:
        return snapdev::floating_point_to_string<double, char>(f_floating_point);

    case token_t::TOKEN_NOT_EQUAL:
        return f_children[0]->recursive_to_as2js() + "!=" + f_children[1]->recursive_to_as2js();

    case token_t::TOKEN_LESS_EQUAL:
        return f_children[0]->recursive_to_as2js() + "<=" + f_children[1]->recursive_to_as2js();

    case token_t::TOKEN_GREATER_EQUAL:
        return f_children[0]->recursive_to_as2js() + ">=" + f_children[1]->recursive_to_as2js();

    case token_t::TOKEN_SHIFT_LEFT:
        return f_children[0]->recursive_to_as2js() + "<<" + f_children[1]->recursive_to_as2js();

    case token_t::TOKEN_SHIFT_RIGHT:
        return f_children[0]->recursive_to_as2js() + ">>" + f_children[1]->recursive_to_as2js();

    case token_t::TOKEN_ALL_FIELDS:
        // TODO...
        return "ALL_FIELDS";

    case token_t::TOKEN_AT:
        return f_children[0]->recursive_to_as2js() + "[" + f_children[1]->recursive_to_as2js() + "]";

    case token_t::TOKEN_BETWEEN:
        {
            // (t = <value>, t >= <low_bound> && t <= <high_bound>)
            //
            std::string const temp(std::to_string(++f_temp_counter));
            return "(_t"
                 + temp
                 + "="
                 + f_children[0]->recursive_to_as2js()
                 + ",_t"
                 + temp
                 + ">="
                 + f_children[1]->recursive_to_as2js()
                 + "&&_t"
                 + temp
                 + "<="
                 + f_children[2]->recursive_to_as2js()
                 + ")";
        }

    case token_t::TOKEN_CAST:
        {
            std::string cast;
            if(static_cast<type_t>(f_integer.f_value[0]) == type_t::TYPE_BOOLEAN)
            {
                // This is much shorter in JavaScript
                //
                cast += "!!";
            }
            else
            {
                // TBD: for Number, we could look into using 1.0*(<expr>)?
                //
                cast += "new ";
                cast += cast_type_to_as2js_type(static_cast<type_t>(f_integer.f_value[0]));
            }
            cast += '(';
            cast += f_children[0]->recursive_to_as2js();
            cast += ')';
            return cast;
        }

    case token_t::TOKEN_FALSE:
        return "false";

    case token_t::TOKEN_FUNCTION_CALL:
        {
            std::string result(f_string);
            result += '(';
            char const * sep("");
            node::pointer_t list(f_children[0]);
            std::size_t const max(list->get_children_size());
            for(std::size_t idx(0); idx < max; ++idx)
            {
                node::pointer_t n(list->get_child(idx));
                result += sep;
                result += n->recursive_to_as2js();
                sep = ",";
            }
            result += ')';
            return result;
        }

    case token_t::TOKEN_LOGICAL_OR:
        return f_children[0]->recursive_to_as2js() + "||" + f_children[1]->recursive_to_as2js();

    case token_t::TOKEN_LOGICAL_AND:
        return f_children[0]->recursive_to_as2js() + "&&" + f_children[1]->recursive_to_as2js();

    case token_t::TOKEN_LOGICAL_NOT:
        return "!" + f_children[0]->recursive_to_as2js();

    case token_t::TOKEN_NULL:
        return "null";

    case token_t::TOKEN_TRUE:
        return "true";

    case token_t::TOKEN_ILIKE:
    case token_t::TOKEN_LIKE:
        {
            std::string regex_pattern;
            if(f_children[1]->is_literal())
            {
                regex_pattern += '"';
                regex_pattern += convert_like_pattern(f_children[1]->get_string_auto_convert());
                regex_pattern += '"';
            }
            else
            {
                regex_pattern = f_children[1]->recursive_to_as2js();
            }

            std::string compare("new RegExp(");
            compare += regex_pattern;
            if(f_token == token_t::TOKEN_ILIKE)
            {
                compare += ",\"i\"";
            }
            compare += ')';
            compare += ".test(";
            compare += f_children[0]->recursive_to_as2js();
            compare += ')';

            return compare;
        }
        break;

    default:
        throw invalid_token(
              "node with token type "
            + std::string(to_string(f_token))
            + (f_token == token_t::TOKEN_TYPE
                    ? " (" + to_string(f_integer) + ')'
                    : "")
            + " cannot directly be converted to as2js script.");

    }
}


std::string node::convert_like_pattern(std::string const sql_pattern) const
{
    // note: the C++ patterns do not require the '^' and '$' characters, but
    //       JavaScript does so make sure to include them
    //
    std::string regex_pattern(1, '^');
    for(auto c : sql_pattern)
    {
        // in a LIKE or ILIKE pattern, we may find '%' which denotes any
        // number of characters (i.e. ".*" in regex); the rest is copied
        // as is, although special characters need to be escaped
        //
        if(c == '%')
        {
            regex_pattern += ".*";
        }
        else
        {
            // escape character if required
            //
            char buf[2] = { c, '\0' };
            regex_pattern += snapdev::escape_special_regex_characters(std::string(buf));
        }
    }
    regex_pattern += '$';

    return regex_pattern;
} // LCOV_EXCL_LINE


std::string node::to_tree(int indent) const
{
    std::string result(indent * 2, ' ');

    result += to_string(f_token);

    switch(f_token)
    {
    case token_t::TOKEN_EOF:
    case token_t::TOKEN_UNKNOWN:
    case token_t::TOKEN_BITWISE_XOR:
    case token_t::TOKEN_MODULO:
    case token_t::TOKEN_BITWISE_AND:
    case token_t::TOKEN_OPEN_PARENTHESIS:
    case token_t::TOKEN_CLOSE_PARENTHESIS:
    case token_t::TOKEN_MULTIPLY:
    case token_t::TOKEN_PLUS:
    case token_t::TOKEN_COMMA:
    case token_t::TOKEN_MINUS:
    case token_t::TOKEN_PERIOD:
    case token_t::TOKEN_DIVIDE:
    case token_t::TOKEN_COLON:
    case token_t::TOKEN_SEMI_COLON:
    case token_t::TOKEN_LESS:
    case token_t::TOKEN_EQUAL:
    case token_t::TOKEN_GREATER:
    case token_t::TOKEN_ABSOLUTE_VALUE:
    case token_t::TOKEN_OPEN_BRACKET:
    case token_t::TOKEN_CLOSE_BRACKET:
    case token_t::TOKEN_POWER:
    case token_t::TOKEN_BITWISE_OR:
    case token_t::TOKEN_REGULAR_EXPRESSION:
    case token_t::TOKEN_other:
    case token_t::TOKEN_NOT_EQUAL:
    case token_t::TOKEN_LESS_EQUAL:
    case token_t::TOKEN_GREATER_EQUAL:
    case token_t::TOKEN_SQUARE_ROOT:
    case token_t::TOKEN_CUBE_ROOT:
    case token_t::TOKEN_SCOPE:
    case token_t::TOKEN_SHIFT_LEFT:
    case token_t::TOKEN_SHIFT_RIGHT:
    case token_t::TOKEN_UNMATCHED_REGULAR_EXPRESSION:
    case token_t::TOKEN_STRING_CONCAT:
    case token_t::TOKEN_ALL_FIELDS:
    case token_t::TOKEN_AT:
    case token_t::TOKEN_BETWEEN:
    case token_t::TOKEN_CAST:
    case token_t::TOKEN_FALSE:
    case token_t::TOKEN_ILIKE:
    case token_t::TOKEN_LIKE:
    case token_t::TOKEN_LIST:
    case token_t::TOKEN_LOGICAL_OR:
    case token_t::TOKEN_LOGICAL_AND:
    case token_t::TOKEN_LOGICAL_NOT:
    case token_t::TOKEN_NULL:
    case token_t::TOKEN_TRUE:
    case token_t::TOKEN_BOOLEAN:
    case token_t::TOKEN_NUMBER:
    case token_t::TOKEN_max:
        break;

    case token_t::TOKEN_FUNCTION_CALL:
    case token_t::TOKEN_STRING:
    case token_t::TOKEN_IDENTIFIER:
        result += " S:\"";
        result += f_string;
        result += '"';
        break;

    case token_t::TOKEN_INTEGER:
        result += " I:";
        result += f_integer.to_string();
        break;

    case token_t::TOKEN_FLOATING_POINT:
        result += " F:";
        result += std::to_string(f_floating_point);
        break;

    case token_t::TOKEN_TYPE:
        result += " T:";
        result += std::to_string(f_integer.f_value[0]);
        break;

    }
    result += '\n';

    for(auto const & c : f_children)
    {
        result += c->to_tree(indent + 2);
    }

    return result;
} // LCOV_EXCL_LINE



} // namespace pbql
} // namespace prinbee
// vim: ts=4 sw=4 et
