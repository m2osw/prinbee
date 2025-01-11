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
    char                    f_name[32] = {};
};

#define TOKEN_NAME(name)    { token_t::TOKEN_##name, #name }
#define TOKEN_CHAR(name)    { token_t::TOKEN_##name, { static_cast<char>(token_t::TOKEN_##name), '\0' } }

struct token_name_t g_token_names[] =
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
    TOKEN_NAME(SIMILAR),
    TOKEN_NAME(TRUE),

    TOKEN_NAME(BOOLEAN),
    TOKEN_NAME(NUMBER),

    //TOKEN_NAME(max) -- not a token
};



} // no name namespace



char const * to_string(token_t t)
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
            return g_token_names[p].f_name;
        }
    }

    return nullptr;
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
    case token_t::TOKEN_SIMILAR:
    case token_t::TOKEN_TRUE:
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
        return match_type == token_t::TOKEN_UNKNOWN || match_type == f_token;

    case token_t::TOKEN_INTEGER:
        return match_type == token_t::TOKEN_UNKNOWN || match_type == f_token || match_type == token_t::TOKEN_NUMBER;

    case token_t::TOKEN_FLOATING_POINT:
        return match_type == token_t::TOKEN_UNKNOWN || match_type == f_token || match_type == token_t::TOKEN_NUMBER;

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


void node::set_integer(uint512_t i)
{
    f_integer = i;
}


uint512_t node::get_integer()
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

    case token_t::TOKEN_MINUS:
        return '-' + f_children[0]->recursive_to_as2js();

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
        return "new " + f_string + "(" + f_children[0]->recursive_to_as2js() + ")";

    case token_t::TOKEN_FALSE:
        return "false";

    case token_t::TOKEN_FUNCTION_CALL:
        {
            std::string result(f_string);
            result += '(';
            char const * sep("");
            for(auto const & n : f_children)
            {
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
    case token_t::TOKEN_SIMILAR:
        {
            std::string const sql_pattern(f_children[1]->get_string());
            std::string regex_pattern;
            char close('\0');
            bool first(false);
            std::size_t const max(sql_pattern.size());
            for(std::size_t idx(0); idx < max; ++idx)
            {
                // % denotes any number of characters (i.e. ".*" in regex; only character supported by LIKE and ILIKE)
                // _ denotes one character (i.e. "." in regex)
                // | denotes alternation (either of two alternatives).
                // * denotes repetition of the previous item zero or more times.
                // + denotes repetition of the previous item one or more times.
                // ? denotes repetition of the previous item zero or one time.
                // {m} denotes repetition of the previous item exactly m times.
                // {m,} denotes repetition of the previous item m or more times.
                // {m,n} denotes repetition of the previous item at least m and not more than n times.
                // Parentheses () can be used to group items into a single logical item.
                // A bracket expression [...] specifies a character class, just as in POSIX regular expressions.

                char c(sql_pattern[idx]);
                if(c == '%')
                {
                    regex_pattern += ".*";
                }
                else if(f_token == token_t::TOKEN_SIMILAR)
                {
                    switch(c)
                    {
                    case '_':
                        regex_pattern += ".";
                        break;

                    case '|':   // Note: the following does not (currently)
                    case '*':   //       get verified here.
                    case '+':
                    case '?':
                    case '(':
                    case ')':
                        regex_pattern += c;
                        break;

                    case '{':
                        close = '}';
                        first = false;
                        goto copy_to_close;

                    case '[':
                        close = ']';
                        first = true;
copy_to_close:
                        regex_pattern += c;
                        for(;; first = false)
                        {
                            ++idx;
                            if(idx >= max)
                            {
                                snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                                msg << f_location.get_location()
                                    << "SIMILAR pattern missing closing bracket ('"
                                    << close
                                    << "' not found or there were no characters within those brackets).";
                                throw invalid_token(msg.str());
                            }
                            c = sql_pattern[idx];
                            regex_pattern += c;
                            if(!first
                            && c == close)
                            {
                                break;
                            }
                        }
                        break;

                    default:
                        // escape anything else as required
                        {
                            char buf[2] = { c, '\0' };
                            regex_pattern += snapdev::escape_special_regex_characters(std::string(buf));
                        }
                        break;

                    }
                }
                else
                {
                    // escape character if required
                    char buf[2] = { c, '\0' };
                    regex_pattern += snapdev::escape_special_regex_characters(std::string(buf));
                }
            }

            std::string compare("/");
            compare += regex_pattern;
            compare += "/.test(";
            compare += f_children[0]->recursive_to_as2js();
            compare += ')';

            return compare;
        }
        break;

    default:
        throw invalid_token(
              "node token type cannot be converted to as2js script ("
            + std::string(to_string(f_token))
            + ").");

    }
}



} // namespace pbql
} // namespace prinbee
// vim: ts=4 sw=4 et
