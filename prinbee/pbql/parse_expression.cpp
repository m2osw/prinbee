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
 * \brief The implementation of the parser::parse_expression() function.
 *
 * Parse an SQL expression (like most languages) is not such a simple
 * matter (plus SQL has only identifiers which change their meaning
 * depending on their location). This file implements the complex
 * SQL expression and converts it to an as2js expression (which is
 * more like JavaScript).
 *
 * The parser I use is written by hand so as usual I call functions
 * and process the right hand-side as required by the operator after
 * a call returns. The following describes the operators and how they
 * can be used and their precedence.
 *
 * \section precedence PBQL Operator Precedence
 *
 * The precedence of the PBQL operators is based on the SQL language.
 * This is very similar to most languages, only we support a few
 * unusual operators (compared to C/C++ languages).
 *
 * In the following table, the `<?>` represents an expression.
 *
 * \code
 * +--------------------------------+----------------+---------------------------------------------------------+
 * | Operator                       | Associativity  | Description                                             |
 * +--------------------------------+----------------+---------------------------------------------------------+
 * | <identifier>                   |                | primary literal identifier                              |
 * | <integer>                      |                | primary literal integer                                 |
 * | <floating point>               |                | primary literal floating point                          |
 * | <string>                       |                | primary literal string                                  |
 * | *                              |                | primary literal for "all fields"                        |
 * +--------------------------------+----------------+---------------------------------------------------------+
 * | <string> <string>              | left           | string concatenation                                    |
 * +--------------------------------+----------------+---------------------------------------------------------+
 * | ( <?> )                        | left           | expression grouping                                     |
 * +--------------------------------+----------------+---------------------------------------------------------+
 * | <?> . <name>                   | left           | context, table, column, or record name separator        |
 * | <?> . *                        | left           | all fields                                              |
 * +--------------------------------+----------------+---------------------------------------------------------+
 * | <?> :: <?>                     | left           | type cast (Postgres compatible)                         |
 * +--------------------------------+----------------+---------------------------------------------------------+
 * | <?> [ <?> ]                    | left           | array element access                                    |
 * +--------------------------------+----------------+---------------------------------------------------------+
 * | + <?>                          | right          | positive (unary plus)                                   |
 * | - <?>                          | right          | negative (unary minus)                                  |
 * +--------------------------------+----------------+---------------------------------------------------------+
 * | <?> ^ <?>                      | left           | exponentiation (warning: left associativity!)           |
 * +--------------------------------+----------------+---------------------------------------------------------+
 * | <?> * <?>                      | left           | multiplication                                          |
 * | <?> / <?>                      | left           | division                                                |
 * | <?> % <?>                      | left           | modulo                                                  |
 * +--------------------------------+----------------+---------------------------------------------------------+
 * | <?> + <?>                      | left           | addition                                                |
 * | <?> - <?>                      | left           | subtraction                                             |
 * +--------------------------------+----------------+---------------------------------------------------------+
 * | <?> ? <?>                      | left           | all other operators                                     |
 * |                                |                |                                                         |
 * | <type> <?>                     | left           | type cast                                               |
 * | <type>(<?>)                    | left           | type cast like a function                               |
 * | <function>(<?>[, <?> ...])     | left           | function call                                           |
 * | |/ <?>                         | right          | square root                                             |
 * | ||/ <?>                        | right          | cubic root                                              |
 * | @ <?>                          | right          | absolute value                                          |
 * | <?> ~ <?>                      | left           | match regular expression                                |
 * | <?> & <?>                      | left           | bitwise and                                             |
 * | <?> | <?>                      | left           | bitwise or                                              |
 * | <?> # <?>                      | left           | bitwise exclusive or                                    |
 * | <?> || <?>                     | left           | string concatenation                                    |
 * | <?> << <?>                     | left           | left shift                                              |
 * | <?> >> <?>                     | left           | right shift                                             |
 * +--------------------------------+----------------+---------------------------------------------------------+
 * | <?> [NOT] BETWEEN <?> AND <?>  | left           | range containement                                      |
 * | <?> [NOT] IN <?>               | left           | set membership                                          |
 * | <?> [NOT] LIKE <?>             | left           | string matching (case sensitive)                        |
 * | <?> [NOT] ILIKE <?>            | left           | string matching (case insensitive)                      |
 * | <?> [NOT] SIMILAR TO <?>       | left           | string matching (regular expression)                    |
 * +--------------------------------+----------------+---------------------------------------------------------+
 * | <?> = <?>                      | left           | equal to                                                |
 * | <?> <> <?>                     | left           | not equal to                                            |
 * | <?> < <?>                      | left           | less than                                               |
 * | <?> <= <?>                     | left           | less than or equal to                                   |
 * | <?> > <?>                      | left           | greater than                                            |
 * | <?> >= <?>                     | left           | greater than or equal to                                |
 * +--------------------------------+----------------+---------------------------------------------------------+
 * | <?> IS [NOT] TRUE              | left           | expression is (not) boolean TRUE                        |
 * | <?> IS [NOT] FALSE             | left           | expression is (not) boolean FALSE                       |
 * | <?> IS [NOT] NULL              | left           | expression is (not) NULL                                |
 * | <?> IS [NOT] DISTINCT FROM <?> | left           | expressions are (not) distinct                          |
 * +--------------------------------+----------------+---------------------------------------------------------+
 * | NOT <?>                        | right          | logical negation                                        |
 * +--------------------------------+----------------+---------------------------------------------------------+
 * | <?> AND <?>                    | left           | logical conjuction                                      |
 * +--------------------------------+----------------+---------------------------------------------------------+
 * | <?> OR <?>                     | left           | logical disjunction                                     |
 * +--------------------------------+----------------+---------------------------------------------------------+
 * | <?>, <?>                       | left           | list of expressions                                     |
 * +--------------------------------+----------------+---------------------------------------------------------+
 * \endcode
 *
 * **IMPORTANT NOTE 1:** The asterisk (*) has two meaning: the multiplication
 *                       operator (`7 * 3`) and the literal meaning all the
 *                       columns of a table (`table_name.*`) or all the fields
 *                       of a record.
 *
 * **IMPORTANT NOTE 2:** The comma is not a valid expression operator. It is
 *                       used to separate things such as expressions in a
 *                       SELECT statement, parameters to a function, etc.
 */

// self
//
#include    "prinbee/pbql/parser.h"

#include    "prinbee/data/schema.h"
#include    "prinbee/exception.h"


// libutf8
//
//#include    <libutf8/libutf8.h>


// snaplogger
//
#include    <snaplogger/message.h>


// snapdev
//
#include    <snapdev/escape_special_regex_characters.h>
#include    <snapdev/floating_point_to_string.h>
#include    <snapdev/to_upper.h>
#include    <snapdev/string_replace_many.h>
//#include    <snapdev/stream_fd.h>


//// C
////
//#include    <linux/fs.h>
//#include    <sys/ioctl.h>


// last include
//
#include    <snapdev/poison.h>



namespace prinbee
{
namespace pbql
{



namespace
{


struct expr_state
{
    lexer::pointer_t    f_lexer = lexer::pointer_t();
    node::pointer_t     f_node = node::pointer_t();
    int                 f_temp = 0;

    std::string         parse_expr_list(int & count); // for function calls only
    std::string         parse_expr_logical_or();
    std::string         parse_expr_logical_and();
    std::string         parse_expr_logical_not();
    std::string         parse_expr_is();
    std::string         parse_expr_comparison();
    std::string         parse_expr_matching();
    std::string         parse_expr_function(std::string const & keyword, int count, int & list_size);
    std::string         parse_expr_cast_value();
    std::string         parse_expr_other();
    std::string         parse_expr_additive();
    std::string         parse_expr_multiplicative();
    std::string         parse_expr_exponentiation();
    std::string         parse_expr_unary();
    std::string         parse_expr_postfix();
    std::string         parse_expr_primary();
};


std::string expr_state::parse_expr_list(int & count)
{
    count = 0;
    std::string result;
    for(;;)
    {
        ++count;
        result += parse_expr_logical_or();
        if(f_node->get_token() != token_t::TOKEN_COMMA)
        {
            return result;
        }
        result += ',';
        f_node = f_lexer->get_next_token();
    }
}


std::string expr_state::parse_expr_logical_or()
{
    std::string result(parse_expr_logical_and());
    while(f_node->get_token() == token_t::TOKEN_IDENTIFIER
       && f_node->get_string_upper() == "OR")
    {
        f_node = f_lexer->get_next_token();
        result += "||";
        result += parse_expr_logical_and();
    }
    return result;
}


std::string expr_state::parse_expr_logical_and()
{
    std::string result(parse_expr_logical_not());
    while(f_node->get_token() == token_t::TOKEN_IDENTIFIER
       && f_node->get_string_upper() == "AND")
    {
        f_node = f_lexer->get_next_token();
        result += "&&";
        result += parse_expr_logical_not();
    }
    return result;
}


std::string expr_state::parse_expr_logical_not()
{
    bool logical_not(false);
    while(f_node->get_token() == token_t::TOKEN_IDENTIFIER
       && f_node->get_string_upper() == "NOT")
    {
        f_node = f_lexer->get_next_token();
        logical_not = !logical_not;
    }
    if(logical_not)
    {
        return '!' + parse_expr_is(); // TODO: fix parenthesis since JavaScript '!' is unary
    }
    return parse_expr_is();
}


std::string expr_state::parse_expr_is()
{
    std::string result(parse_expr_comparison());
    while(f_node->get_token() == token_t::TOKEN_IDENTIFIER
       && f_node->get_string_upper() == "IS")
    {
        f_node = f_lexer->get_next_token();
        bool negate(false);
        if(f_node->get_token() == token_t::TOKEN_IDENTIFIER
        && f_node->get_string_upper() == "NOT")
        {
            negate = true;
            f_node = f_lexer->get_next_token();
        }
        if(f_node->get_token() == token_t::TOKEN_IDENTIFIER)
        {
            if(f_node->get_string_upper() == "TRUE")
            {
                // TBD: verify that we have a boolean?
                if(negate)
                {
                    result = "!(" + result + ')';
                }
            }
            else if(f_node->get_string_upper() == "FALSE")
            {
                // TBD: verify that we have a boolean?
                if(!negate)
                {
                    result = "!(" + result + ')';
                }
            }
            else if(f_node->get_string_upper() == "NULL")
            {
                result += negate ? '!' : '=';
                result += "=null";
            }
            else if(f_node->get_string_upper() == "DISTINCT")
            {
                f_node = f_lexer->get_next_token();
                if(f_node->get_token() != token_t::TOKEN_IDENTIFIER
                || f_node->get_string_upper() != "FROM")
                {
                    snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                    msg << f_node->get_location().get_location()
                        << "expected FROM after IS DISTINCT.";
                    throw invalid_token(msg.str());
                }
                throw not_yet_implemented("IS [NOT] DISTINCT FROM is not yet implemented");
            }
        }
    }
    return result;
}


std::string expr_state::parse_expr_comparison()
{
    std::string result(parse_expr_matching());
    for(;;)
    {
        switch(f_node->get_token())
        {
        case token_t::TOKEN_LESS:
            result += "<";
            result += parse_expr_matching();
            break;

        case token_t::TOKEN_LESS_EQUAL:
            result += "<=";
            result += parse_expr_matching();
            break;

        case token_t::TOKEN_EQUAL:
            result += "==";
            result += parse_expr_matching();
            break;

        case token_t::TOKEN_GREATER:
            result += ">";
            result += parse_expr_matching();
            break;

        case token_t::TOKEN_GREATER_EQUAL:
            result += ">=";
            result += parse_expr_matching();
            break;

        case token_t::TOKEN_NOT_EQUAL:
            result += "!=";
            result += parse_expr_matching();
            break;

        default:
            return result;

        }
    }
    snapdev::NOT_REACHED();
}


std::string expr_state::parse_expr_matching()
{
    std::string result(parse_expr_other());

    bool negate(false);
    if(f_node->get_token() == token_t::TOKEN_IDENTIFIER
    && f_node->get_string_upper() == "NOT")
    {
        negate = true;
    }
    if(f_node->get_token() == token_t::TOKEN_IDENTIFIER)
    {
        std::string const keyword(f_node->get_string_upper());
        if(keyword == "BETWEEN")
        {
            f_node = f_lexer->get_next_token();

            // WARNING: here we have to make sure the next parse_expr_...()
            //          does not manage the "AND" keyword
            //
            std::string const lowerbound(parse_expr_other());

            if(f_node->get_token() != token_t::TOKEN_IDENTIFIER
            || f_node->get_string_upper() != "AND")
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                msg << f_node->get_location().get_location()
                    << "expected AND between the lower and higher bounds of BETWEEN operator.";
                throw invalid_token(msg.str());
            }

            f_node = f_lexer->get_next_token();
            std::string const upperbound(parse_expr_other());

            ++f_temp;
            std::string temp("_t");
            temp += std::to_string(f_temp);

            std::string compare("(");
            compare += temp;
            compare += '=';
            compare += result;
            compare += ',';
            compare += temp;
            compare += negate ? "<" : ">=";
            compare += lowerbound;
            compare += negate ? "||" : "&&";
            compare += temp;
            compare += negate ? ">" : "<=";
            compare += upperbound;
            compare += ')';

            result.swap(compare);
        }
        else if(keyword == "IN")
        {
            // TODO: not too what the right hand side would end up being
            //       in this case... (array, sub-select...)
            //
            throw not_yet_implemented("[NOT] IN is not yet implemented");
        }
        else
        {
            bool const like(keyword == "LIKE");
            bool const ilike(keyword == "ILIKE");
            bool const similar(keyword == "SIMILAR");
            if(like || ilike || similar)
            {
                if(similar)
                {
                    f_node = f_lexer->get_next_token();
                    if(f_node->get_token() != token_t::TOKEN_IDENTIFIER
                    || f_node->get_string_upper() != "TO")
                    {
                        snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                        msg << f_node->get_location().get_location()
                            << "expected TO after the SIMILAR keyword.";
                        throw invalid_token(msg.str());
                    }
                }

                // we expect a string, so there is really no need to check
                // for the Boolean expressions
                //
                std::string const sql_pattern(parse_expr_other());
                std::string regex_pattern;
                char close('\0');
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
                    else if(similar)
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
                            goto copy_to_close;

                        case '[':
                            close = ']';
copy_to_close:
                            regex_pattern += c;
                            for(bool first(true);; first = false)
                            {
                                ++idx;
                                if(idx >= max)
                                {
                                    snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                                    msg << f_node->get_location().get_location()
                                        << "SIMILAR pattern missing closing bracket ('"
                                        << close
                                        << "' not found or the value within those brackets was empty).";
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

                        case '/':
                            // we need to escape the slash and the snapdev
                            // function does not do that for us
                            //
                            regex_pattern += "\\/";
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
                compare += result;
                compare += ')';

                result.swap(compare);
            }
        }
    }
    else if(negate)
    {
        snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
        msg << f_node->get_location().get_location()
            << "expected NOT in expression.";
        throw invalid_token(msg.str());
    }
    return result;
}


std::string expr_state::parse_expr_function(std::string const & keyword, int count, int & list_size)
{
    list_size = 0;
    f_node = f_lexer->get_next_token();
    if(f_node->get_token() != token_t::TOKEN_OPEN_PARENTHESIS)
    {
        snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
        msg << f_node->get_location().get_location()
            << "expected '(' to start the list of parameters in a function call.";
        throw invalid_token(msg.str());
    }
    f_node = f_lexer->get_next_token();
    std::string result;
    if(f_node->get_token() != token_t::TOKEN_CLOSE_PARENTHESIS)
    {
        result = parse_expr_list(list_size);
    }
    if(f_node->get_token() != token_t::TOKEN_CLOSE_PARENTHESIS)
    {
        snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
        msg << f_node->get_location().get_location()
            << "expected '(' to start the list of parameters in a function call.";
        throw invalid_token(msg.str());
    }
    if(count >= 0
    && count != list_size)
    {
        snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
        msg << f_node->get_location().get_location()
            << keyword
            << "() expected "
            << count
            << " parameter"
            << (count != 1 ? "s" : "")
            << ", found "
            << list_size
            << " instead.";
        throw invalid_parameter(msg.str());
    }
    return result;
}


std::string expr_state::parse_expr_cast_value()
{
    bool has_parenthesis(false);
    f_node = f_lexer->get_next_token();
    if(f_node->get_token() == token_t::TOKEN_OPEN_PARENTHESIS)
    {
        has_parenthesis = true;
        f_node = f_lexer->get_next_token();
    }
    std::string const result(parse_expr_logical_or());
    if(has_parenthesis)
    {
        if(f_node->get_token() != token_t::TOKEN_CLOSE_PARENTHESIS)
        {
            snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
            msg << f_node->get_location().get_location()
                << "type casting used '(' so a ')' was expected to end the casting expression.";
            throw invalid_parameter(msg.str());
        }
        f_node = f_lexer->get_next_token();
    }
    return result;
}


std::string expr_state::parse_expr_other()
{
    // this one is really strange since it can start with a primary like
    // expression (@ <?> and |/ <?> for example)
    //
    std::string result;
    switch(f_node->get_token())
    {
    case token_t::TOKEN_ABSOLUTE_VALUE:
        result += "Math.abs(";
        result += parse_expr_other();
        result += ")";
        return result;

    case token_t::TOKEN_SQUARE_ROOT:
        result += "Math.sqrt(";
        result += parse_expr_other();
        result += ")";
        return result;

    case token_t::TOKEN_CUBE_ROOT:
        result += "Math.cbrt(";
        result += parse_expr_other();
        result += ")";
        return result;

    case token_t::TOKEN_IDENTIFIER:
        // type case or function call
        //
        {
            int list_size(0);
            std::string keyword(f_node->get_string_upper());
            switch(keyword[0])
            {
            case 'A':
                if(keyword == "ABS")
                {
                    result += "Math.abs(";
                    result += parse_expr_function(keyword, 1, list_size);
                    result += ")";
                    return result;
                }
                if(keyword == "ACOS")
                {
                    result += "Math.acos(";
                    result += parse_expr_function(keyword, 1, list_size);
                    result += ")";
                    return result;
                }
                if(keyword == "ACOSH")
                {
                    result += "Math.acosh(";
                    result += parse_expr_function(keyword, 1, list_size);
                    result += ")";
                    return result;
                }
                if(keyword == "ASIN")
                {
                    result += "Math.asin(";
                    result += parse_expr_function(keyword, 1, list_size);
                    result += ")";
                    return result;
                }
                if(keyword == "ASINH")
                {
                    result += "Math.asinh(";
                    result += parse_expr_function(keyword, 1, list_size);
                    result += ")";
                    return result;
                }
                if(keyword == "ATAN")
                {
                    std::string const params(parse_expr_function(keyword, 1, list_size));
                    if(list_size == 1)
                    {
                        result += "Math.atan(";
                        result += params;
                        result += ")";
                    }
                    else if(list_size == 2)
                    {
                        result += "Math.atan2(";
                        result += params;
                        result += ")";
                    }
                    else
                    {
                        snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                        msg << f_node->get_location().get_location()
                            << "expected 1 or 2 parameters to ATAN(), found "
                            << list_size
                            << " instead.";
                        throw invalid_parameter(msg.str());
                    }
                    return result;
                }
                if(keyword == "ATANH")
                {
                    result += "Math.atanh(";
                    result += parse_expr_function(keyword, 1, list_size);
                    result += ")";
                    return result;
                }
                break;

            case 'B':
                if(keyword == "BIGINT")
                {
                    result += "new Integer(";
                    result += parse_expr_cast_value();
                    result += ')';
                    return result;
                }
                if(keyword == "BOOLEAN")
                {
                    result += "new Boolean(";
                    result += parse_expr_cast_value();
                    result += ')';
                    return result;
                }
                break;

            case 'C':
                if(keyword == "CBRT")
                {
                    result += "Math.cbrt(";
                    result += parse_expr_function(keyword, 1, list_size);
                    result += ")";
                    return result;
                }
                if(keyword == "CEIL")
                {
                    result += "Math.ceil(";
                    result += parse_expr_function(keyword, 1, list_size);
                    result += ")";
                    return result;
                }
                if(keyword == "CHAR")
                {
                    result += "new String(";
                    result += parse_expr_cast_value();
                    result += ')';
                    return result;
                }
                if(keyword == "COS")
                {
                    result += "Math.cos(";
                    result += parse_expr_function(keyword, 1, list_size);
                    result += ")";
                    return result;
                }
                if(keyword == "COSH")
                {
                    result += "Math.cosh(";
                    result += parse_expr_function(keyword, 1, list_size);
                    result += ")";
                    return result;
                }
                break;

            case 'D':
                if(keyword == "DOUBLE")
                {
                    f_node = f_lexer->get_next_token();
                    if(f_node->get_token() != token_t::TOKEN_IDENTIFIER
                    || f_node->get_string_upper() != "PRECISION")
                    {
                        snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                        msg << f_node->get_location().get_location()
                            << "expected DOUBLE to be followed by the word PRECISION.";
                        throw invalid_token(msg.str());
                    }
                    result += "new Number(";
                    result += parse_expr_cast_value();
                    result += ')';
                    return result;
                }
                break;

            case 'E':
                if(keyword == "EXP")
                {
                    result += "Math.exp(";
                    result += parse_expr_function(keyword, 1, list_size);
                    result += ")";
                    return result;
                }
                if(keyword == "EXPM1")
                {
                    result += "Math.expm1(";
                    result += parse_expr_function(keyword, 1, list_size);
                    result += ")";
                    return result;
                }
                break;

            case 'F':
                if(keyword == "FLOAT2"
                || keyword == "FLOAT4"
                || keyword == "FLOAT10")
                {
                    result += "new Number(";
                    result += parse_expr_cast_value();
                    result += ')';
                    return result;
                }
                if(keyword == "FLOOR")
                {
                    result += "Math.floor(";
                    result += parse_expr_function(keyword, 1, list_size);
                    result += ")";
                    return result;
                }
                if(keyword == "FROUND")
                {
                    result += "Math.fround(";
                    result += parse_expr_function(keyword, 1, list_size);
                    result += ")";
                    return result;
                }
                break;

            case 'H':
                if(keyword == "HYPOT")
                {
                    // with 0 parameters, the function returns 0
                    // with 1 parameter, the function returns that parameter as is
                    // with 2 or more, it returns the square root of the sum of the squares
                    //
                    result += "Math.hypot(";
                    result += parse_expr_function(keyword, -1, list_size);
                    result += ")";
                    return result;
                }
                break;

            case 'I':
                if(keyword == "INUL")
                {
                    result += "Math.imul(";
                    result += parse_expr_function(keyword, 2, list_size);
                    result += ")";
                    return result;
                }
                if(keyword == "INT"
                || keyword == "INT1"
                || keyword == "INT2"
                || keyword == "INT4"
                || keyword == "INT8"
                || keyword == "INT16"
                || keyword == "INT32"
                || keyword == "INT64"
                || keyword == "INTEGER")
                {
                    // TODO: add support for bigint for larger numbers can be
                    //       more than 64 bits
                    //
                    result += "new Integer(";
                    result += parse_expr_cast_value();
                    result += ')';
                    return result;
                }
                break;

            case 'L':
                if(keyword == "LOG")
                {
                    result += "Math.log(";
                    result += parse_expr_function(keyword, 1, list_size);
                    result += ")";
                    return result;
                }
                if(keyword == "LOG1P")
                {
                    result += "Math.log1p(";
                    result += parse_expr_function(keyword, 1, list_size);
                    result += ")";
                    return result;
                }
                if(keyword == "LOG10")
                {
                    result += "Math.log10(";
                    result += parse_expr_function(keyword, 1, list_size);
                    result += ")";
                    return result;
                }
                if(keyword == "LOG2")
                {
                    result += "Math.log2(";
                    result += parse_expr_function(keyword, 1, list_size);
                    result += ")";
                    return result;
                }
                break;

            case 'M':
                if(keyword == "MAX")
                {
                    result += "Math.max(";
                    result += parse_expr_function(keyword, -1, list_size);
                    result += ")";
                    return result;
                }
                if(keyword == "MIN")
                {
                    result += "Math.min(";
                    result += parse_expr_function(keyword, -1, list_size);
                    result += ")";
                    return result;
                }
                break;

            case 'P':
                if(keyword == "POW")
                {
                    result += "Math.pow(";
                    result += parse_expr_function(keyword, 2, list_size);
                    result += ")";
                    return result;
                }
                break;

            case 'R':
                if(keyword == "RAND")
                {
                    result += "Math.random(";
                    result += parse_expr_function(keyword, 0, list_size);
                    result += ")";
                    return result;
                }
                if(keyword == "REAL")
                {
                    result += "new Number(";
                    result += parse_expr_cast_value();
                    result += ')';
                    return result;
                }
                if(keyword == "ROUND")
                {
                    result += "Math.round(";
                    result += parse_expr_function(keyword, 1, list_size);
                    result += ")";
                    return result;
                }
                break;

            case 'S':
                if(keyword == "SIGN")
                {
                    result += "Math.sign(";
                    result += parse_expr_function(keyword, 1, list_size);
                    result += ")";
                    return result;
                }
                if(keyword == "SIN")
                {
                    result += "Math.sin(";
                    result += parse_expr_function(keyword, 1, list_size);
                    result += ")";
                    return result;
                }
                if(keyword == "SINH")
                {
                    result += "Math.sinh(";
                    result += parse_expr_function(keyword, 1, list_size);
                    result += ")";
                    return result;
                }
                if(keyword == "SMALLINT")
                {
                    result += "new Integer(";
                    result += parse_expr_cast_value();
                    result += ')';
                    return result;
                }
                if(keyword == "SQRT")
                {
                    result += "Math.sqrt(";
                    result += parse_expr_function(keyword, 1, list_size);
                    result += ")";
                    return result;
                }
                break;

            case 'T':
                if(keyword == "TAN")
                {
                    result += "Math.tan(";
                    result += parse_expr_function(keyword, 1, list_size);
                    result += ")";
                    return result;
                }
                if(keyword == "TANH")
                {
                    result += "Math.tanh(";
                    result += parse_expr_function(keyword, 1, list_size);
                    result += ")";
                    return result;
                }
                if(keyword == "TEXT")
                {
                    result += "new String(";
                    result += parse_expr_cast_value();
                    result += ')';
                    return result;
                }
                if(keyword == "TRUNC")
                {
                    result += "Math.trunc(";
                    result += parse_expr_function(keyword, 1, list_size);
                    result += ")";
                    return result;
                }
                break;

            case 'U':
                if(keyword == "UNSIGNED")
                {
                    f_node = f_lexer->get_next_token();
                    if(f_node->get_token() != token_t::TOKEN_IDENTIFIER)
                    {
                        snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                        msg << f_node->get_location().get_location()
                            << "expected an integer name to follow the UNSIGNED word.";
                        throw invalid_token(msg.str());
                    }
                    keyword = f_node->get_string_upper();
                    if(keyword == "BIGINT"
                    || keyword == "INT"
                    || keyword == "INT1"
                    || keyword == "INT2"
                    || keyword == "INT4"
                    || keyword == "INT8"
                    || keyword == "INT16"
                    || keyword == "INT32"
                    || keyword == "INT64"
                    || keyword == "INTEGER"
                    || keyword == "SMALLINT")
                    {
                        // TODO: add support for bigint for larger numbers can be
                        //       more than 64 bits
                        //
                        result += "new Integer(";
                        result += parse_expr_cast_value();
                        result += ')';
                        return result;
                    }
                }
                break;

            }
        }
        break;

    default:
        break;

    }

    result = parse_expr_additive();
    for(;;)
    {
        switch(f_node->get_token())
        {
        case token_t::TOKEN_BITWISE_AND:
            result += '&';
            f_node = f_lexer->get_next_token();
            result += parse_expr_additive();
            break;

        case token_t::TOKEN_BITWISE_OR:
            result += '|';
            f_node = f_lexer->get_next_token();
            result += parse_expr_additive();
            break;

        case token_t::TOKEN_BITWISE_XOR:
            result += '^';
            f_node = f_lexer->get_next_token();
            result += parse_expr_additive();
            break;

        case token_t::TOKEN_STRING_CONCAT:
            // TODO: optimize if we have multiple in a row by concatenating
            //       them all in one call
            //
            result = "String.concat(" + result;
            result += ',';
            f_node = f_lexer->get_next_token();
            result += parse_expr_additive();
            result += ')';
            break;

        case token_t::TOKEN_SHIFT_LEFT:
            result += "<<";
            f_node = f_lexer->get_next_token();
            result += parse_expr_additive();
            break;

        case token_t::TOKEN_SHIFT_RIGHT:
            // TODO: in JavaScript we need to use '>>>' to properly support
            //       unsigned integers
            //
            result += ">>";
            f_node = f_lexer->get_next_token();
            result += parse_expr_additive();
            break;

        case token_t::TOKEN_REGULAR_EXPRESSION:
            {
                f_node = f_lexer->get_next_token();

                std::string compare("new RegExp(");
                compare += parse_expr_additive();
                compare += ").test(";
                compare += result;
                compare += ')';

                result.swap(compare);
            }
            break;

        default:
            return result;

        }
    }
}


std::string expr_state::parse_expr_additive()
{
    std::string result(parse_expr_multiplicative());
    for(;;)
    {
        switch(f_node->get_token())
        {
        case token_t::TOKEN_PLUS:
            result += '+';
            result += parse_expr_multiplicative();
            break;

        case token_t::TOKEN_MINUS:
            result += '-';
            result += parse_expr_multiplicative();
            break;

        default:
            return result;

        }
    }
    snapdev::NOT_REACHED();
}


std::string expr_state::parse_expr_multiplicative()
{
    std::string result(parse_expr_exponentiation());
    for(;;)
    {
        switch(f_node->get_token())
        {
        case token_t::TOKEN_MULTIPLY:
            result += '*';
            result += parse_expr_exponentiation();
            break;

        case token_t::TOKEN_DIVIDE:
            result += '/';
            result += parse_expr_exponentiation();
            break;

        case token_t::TOKEN_MODULO:
            result += '%';
            result += parse_expr_exponentiation();
            break;

        default:
            return result;

        }
    }
    snapdev::NOT_REACHED();
}


std::string expr_state::parse_expr_exponentiation()
{
    std::string result(parse_expr_unary());
    for(;;)
    {
        switch(f_node->get_token())
        {
        case token_t::TOKEN_POWER:
            // in as2js the exponentiation is right to left (like in math/Ada)
            //
            result = '(' + result + "**";
            f_node = f_lexer->get_next_token();
            result += parse_expr_unary();
            result += ')';
            break;

        default:
            return result;

        }
    }
    snapdev::NOT_REACHED();
}


std::string expr_state::parse_expr_unary()
{
    bool negate(false);
    for(;;)
    {
        switch(f_node->get_token())
        {
        case token_t::TOKEN_PLUS:
            // the identity does nothing in SQL (as far as I know) so just
            // return the input as is
            //
            f_node = f_lexer->get_next_token();
            break;

        case token_t::TOKEN_MINUS:
            negate = !negate;
            f_node = f_lexer->get_next_token();
            break;

        default:
            if(negate)
            {
                return '-' + parse_expr_postfix();
            }
            return parse_expr_postfix();

        }
    }
    snapdev::NOT_REACHED();
}


std::string expr_state::parse_expr_postfix()
{
    bool found_all_fields(false);
    std::string result(parse_expr_primary());
    for(;;)
    {
        switch(f_node->get_token())
        {
        case token_t::TOKEN_PERIOD:
            if(found_all_fields)
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                msg << f_node->get_location().get_location()
                    << "no more '.' can be used after '.*'.";
                throw invalid_token(msg.str());
            }
            f_node = f_lexer->get_next_token();
            if(f_node->get_token() == token_t::TOKEN_MULTIPLY)
            {
                // special case were we want all the fields of a table,
                // record, etc.
                //
                // TBD: in JavaScript it means we return the object so we do
                //      nothing here?
                //
                f_node = f_lexer->get_next_token();
                found_all_fields = true;
            }
            else if(f_node->get_token() == token_t::TOKEN_IDENTIFIER)
            {
                result += '.';
                result += f_node->get_string_lower();
                f_node = f_lexer->get_next_token();
            }
            else
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                msg << f_node->get_location().get_location()
                    << "expected '*' or a field name after '.'.";
                throw invalid_token(msg.str());
            }
            break;

        case token_t::TOKEN_SCOPE:
            f_node = f_lexer->get_next_token();
            if(f_node->get_token() != token_t::TOKEN_IDENTIFIER)
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                msg << f_node->get_location().get_location()
                    << "a type name was expected after the '::' operator.";
                throw invalid_token(msg.str());
            }
            else
            {
                std::string keyword(f_node->get_string_upper());
                std::string const l(f_node->get_location().get_location());
                bool found(true);
                switch(keyword[0])
                {
                case 'B':
                    if(keyword == "BIGINT")
                    {
                        result = "new Integer(" + result;
                        result += ')';
                    }
                    else if(keyword == "BOOLEAN")
                    {
                        result = "new Boolean(" + result;
                        result += ')';
                    }
                    else
                    {
                        found = false;
                    }
                    break;

                case 'C':
                    if(keyword == "CHAR")
                    {
                        result = "new String(" + result;
                        result += ')';
                    }
                    else
                    {
                        found = false;
                    }
                    break;

                case 'D':
                    if(keyword == "DOUBLE")
                    {
                        f_node = f_lexer->get_next_token();
                        if(f_node->get_token() != token_t::TOKEN_IDENTIFIER
                        || f_node->get_string_upper() != "PRECISION")
                        {
                            snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                            msg << l
                                << "expected DOUBLE to be followed by the word PRECISION.";
                            throw invalid_token(msg.str());
                        }
                        result = "new Number(" + result;
                        result += ')';
                    }
                    else
                    {
                        found = false;
                    }
                    break;

                case 'I':
                    if(keyword == "INT"
                    || keyword == "INT1"
                    || keyword == "INT2"
                    || keyword == "INT4"
                    || keyword == "INT8"
                    || keyword == "INT16"
                    || keyword == "INT32"
                    || keyword == "INT64"
                    || keyword == "INTEGER")
                    {
                        result = "new Integer(" + result;
                        result += ')';
                    }
                    else
                    {
                        found = false;
                    }
                    break;

                case 'F':
                    if(keyword == "FLOAT2"
                    || keyword == "FLOAT4")
                    {
                        result = "new Number(" + result;
                        result += ')';
                    }
                    else
                    {
                        found = false;
                    }
                    break;

                case 'R':
                    if(keyword == "REAL")
                    {
                        result = "new Number(" + result;
                        result += ')';
                    }
                    else
                    {
                        found = false;
                    }
                    break;

                case 'S':
                    if(keyword == "SMALLINT")
                    {
                        result = "new Integer(" + result;
                        result += ')';
                    }
                    else
                    {
                        found = false;
                    }
                    break;

                case 'T':
                    if(keyword == "TEXT")
                    {
                        result = "new String(" + result;
                        result += ')';
                    }
                    else
                    {
                        found = false;
                    }
                    break;

                case 'U':
                    if(keyword == "UNSIGNED")
                    {
                        f_node = f_lexer->get_next_token();
                        if(f_node->get_token() != token_t::TOKEN_IDENTIFIER)
                        {
                            snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                            msg << f_node->get_location().get_location()
                                << "expected an integer name to follow the UNSIGNED word.";
                            throw invalid_token(msg.str());
                        }
                        keyword = f_node->get_string_upper();
                        if(keyword == "BIGINT"
                        || keyword == "INT"
                        || keyword == "INT1"
                        || keyword == "INT2"
                        || keyword == "INT4"
                        || keyword == "INT8"
                        || keyword == "INT16"
                        || keyword == "INT32"
                        || keyword == "INT64"
                        || keyword == "INTEGER"
                        || keyword == "SMALLINT")
                        {
                            result = "new Integer(" + result;
                            result += ')';
                        }
                        else
                        {
                            keyword = "UNSIGNED " + keyword;
                            found = false;
                        }
                    }
                    else
                    {
                        found = false;
                    }
                    break;

                default:
                    found = false;
                    break;

                }
                if(!found)
                {
                    snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                    msg << l
                        << "expected the name of a type after the '::' operator, found \""
                        << keyword
                        << "\" instead.";
                    throw invalid_token(msg.str());
                }
                f_node = f_lexer->get_next_token();
            }
            break;

        case token_t::TOKEN_OPEN_BRACKET:
            f_node = f_lexer->get_next_token();
            result += '[';
            result += parse_expr_logical_or();
            if(f_node->get_token() != token_t::TOKEN_CLOSE_BRACKET)
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                msg << f_node->get_location().get_location()
                    << "expected a closing square bracket (]), not "
                    << to_string(f_node->get_token())
                    << ".";
                throw invalid_token(msg.str());
            }
            result += ']';
            f_node = f_lexer->get_next_token();
            break;

        default:
            return result;

        }
    }
    snapdev::NOT_REACHED();
}


std::string expr_state::parse_expr_primary()
{
    std::string result;
    switch(f_node->get_token())
    {
    case token_t::TOKEN_STRING:
        result = f_node->get_string();
        f_node = f_lexer->get_next_token();
        while(f_node->get_token() == token_t::TOKEN_STRING)
        {
            // SQL support a C-like string concatenation when two or more
            // strings are defined one after the other; to match the SQL
            // standard, we would need to make sure that each string is on
            // a different line (which would be easy since we can just check
            // the location of the node); however, in our case, we do not
            // need to follow the standard to the letter and skip on that part
            //
            result += f_node->get_string();
            f_node = f_lexer->get_next_token();
        }
        // add the quotes; also use double quotes and make sure to escape
        // them if any are present in the string
        //
        result = '"' + snapdev::string_replace_many(
                result
                , {
                    {"\"", "\\\""},
                    {"\b", "\\b"},
                    {"\f", "\\f"},
                    {"\n", "\\n"},  // the \n and \r are very important in strings otherwise the as2js lexer breaks
                    {"\r", "\\r"},
                    {"\t", "\\t"},
                    {"\v", "\\v"},
                }) + '"';
        return result;

    case token_t::TOKEN_IDENTIFIER:
        result = f_node->get_string_lower();
        // if result is "true" or "false" we already have a Boolean as expected!
        f_node = f_lexer->get_next_token();
        return result;

    case token_t::TOKEN_FLOATING_POINT:
        result = snapdev::floating_point_to_string<double, char>(f_node->get_floating_point());
        f_node = f_lexer->get_next_token();
        return result;

    case token_t::TOKEN_INTEGER:
        result = to_string(f_node->get_integer());
        f_node = f_lexer->get_next_token();
        return result;

    case token_t::TOKEN_MULTIPLY:
        // TODO: how do we really want to represent that one in our as2js expressions?
        //
        result = "ALL_FIELDS";
        f_node = f_lexer->get_next_token();
        return result;

    case token_t::TOKEN_OPEN_PARENTHESIS:
        f_node = f_lexer->get_next_token();
        result = parse_expr_logical_or();
        if(f_node->get_token() != token_t::TOKEN_CLOSE_PARENTHESIS)
        {
            snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
            msg << f_node->get_location().get_location()
                << "expected ')' to close the grouped expressions.";
            throw invalid_token(msg.str());
        }
        f_node = f_lexer->get_next_token();
        return result;

    default:
        snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
        msg << f_node->get_location().get_location()
            << "expected a primary token not "
            << to_string(f_node->get_token())
            << " (primary tokens are: string, number, true, false, identifier,"
               " '*', or an expression between parenthesis).";
        throw invalid_token(msg.str());

    }
    snapdev::NOT_REACHED(); // LCOV_EXCL_LINE
}



} // no name namespace


std::string parser::parse_expression(node::pointer_t & n)
{
    expr_state s
    {
        .f_lexer = f_lexer,
        .f_node = n,
    };
    std::string const result(s.parse_expr_logical_or());
    n = s.f_node;
    return result;
}



} // namespace pbql
} // namespace prinbee
// vim: ts=4 sw=4 et
