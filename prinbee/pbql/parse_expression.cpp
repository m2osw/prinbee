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
#include    <snapdev/to_lower.h>


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
    node::pointer_t     f_tree = node::pointer_t(); // resulting tree
    int                 f_temp = 0;

    node::pointer_t     parse_expr_list(); // for function calls only
    node::pointer_t     parse_expr_logical_or();
    node::pointer_t     parse_expr_logical_and();
    node::pointer_t     parse_expr_logical_not();
    node::pointer_t     parse_expr_is();
    node::pointer_t     parse_expr_comparison();
    node::pointer_t     parse_expr_matching();
    node::pointer_t     parse_expr_function_parameters(std::string const & keyword, int count);
    node::pointer_t     parse_expr_cast_value(std::string const & type_name);
    node::pointer_t     parse_expr_other();
    node::pointer_t     parse_expr_additive();
    node::pointer_t     parse_expr_multiplicative();
    node::pointer_t     parse_expr_exponentiation();
    node::pointer_t     parse_expr_unary();
    node::pointer_t     parse_expr_postfix();
    node::pointer_t     parse_expr_primary();
};


 
node::pointer_t expr_state::parse_expr_list()
{
    node::pointer_t result(std::make_shared<node>(token_t::TOKEN_LIST, f_node->get_location()));
    for(;;)
    {
        node::pointer_t n(parse_expr_logical_or());
        result->insert_child(-1, n);

        // repeat as long as we find commas
        //
        if(f_node->get_token() != token_t::TOKEN_COMMA)
        {
            return result;
        }
        f_node = f_lexer->get_next_token();
    }
}


node::pointer_t expr_state::parse_expr_logical_or()
{
    node::pointer_t result(parse_expr_logical_and());
    while(f_node->get_token() == token_t::TOKEN_IDENTIFIER
       && f_node->get_string_upper() == "OR")
    {
        node::pointer_t n(std::make_shared<node>(token_t::TOKEN_LOGICAL_OR, f_node->get_location()));
        n->insert_child(-1, result);
        f_node = f_lexer->get_next_token();
        n->insert_child(-1, parse_expr_logical_and());
        result = n;
    }
    return result;
}


node::pointer_t expr_state::parse_expr_logical_and()
{
    node::pointer_t result(parse_expr_logical_not());
    while(f_node->get_token() == token_t::TOKEN_IDENTIFIER
       && f_node->get_string_upper() == "AND")
    {
        node::pointer_t n(std::make_shared<node>(token_t::TOKEN_LOGICAL_AND, f_node->get_location()));
        n->insert_child(-1, result);
        f_node = f_lexer->get_next_token();
        n->insert_child(-1, parse_expr_logical_not());
        result = n;
    }
    return result;
}


node::pointer_t expr_state::parse_expr_logical_not()
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
        node::pointer_t n(std::make_shared<node>(token_t::TOKEN_LOGICAL_NOT, f_node->get_location()));
        n->insert_child(-1, parse_expr_is());
        return n;
    }
    return parse_expr_is();
}


node::pointer_t expr_state::parse_expr_is()
{
    node::pointer_t result(parse_expr_comparison());
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
                // TBD: verify that we have a Boolean?
                if(negate)
                {
                    node::pointer_t n(std::make_shared<node>(token_t::TOKEN_LOGICAL_NOT, f_node->get_location()));
                    n->insert_child(-1, result);
                    result = n;
                }
            }
            else if(f_node->get_string_upper() == "FALSE")
            {
                // TBD: verify that we have a Boolean?
                if(!negate)
                {
                    node::pointer_t n(std::make_shared<node>(token_t::TOKEN_LOGICAL_NOT, f_node->get_location()));
                    n->insert_child(-1, result);
                    result = n;
                }
            }
            else if(f_node->get_string_upper() == "NULL")
            {
                node::pointer_t n(std::make_shared<node>(negate ? token_t::TOKEN_NOT_EQUAL : token_t::TOKEN_EQUAL, f_node->get_location()));
                n->insert_child(-1, result);
                n->insert_child(-1, std::make_shared<node>(token_t::TOKEN_NULL, f_node->get_location()));
                result = n;
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


node::pointer_t expr_state::parse_expr_comparison()
{
    node::pointer_t result(parse_expr_matching());
    node::pointer_t n;
    for(;;)
    {
        switch(f_node->get_token())
        {
        case token_t::TOKEN_LESS:
        case token_t::TOKEN_LESS_EQUAL:
        case token_t::TOKEN_EQUAL:
        case token_t::TOKEN_GREATER:
        case token_t::TOKEN_GREATER_EQUAL:
        case token_t::TOKEN_NOT_EQUAL:
            n = std::make_shared<node>(f_node->get_token(), f_node->get_location());
            n->insert_child(-1, result);
            n->insert_child(-1, parse_expr_matching());
            result = n;
            break;

        default:
            return result;

        }
    }
    snapdev::NOT_REACHED();
}


node::pointer_t expr_state::parse_expr_matching()
{
    node::pointer_t result(parse_expr_other());

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
            node::pointer_t n(std::make_shared<node>(token_t::TOKEN_BETWEEN, f_node->get_location()));
            n->insert_child(-1, result);

            // WARNING: here we have to make sure the next parse_expr_...()
            //          does not manage the "AND" keyword
            //
            f_node = f_lexer->get_next_token();
            n->insert_child(-1, parse_expr_other());

            if(f_node->get_token() != token_t::TOKEN_IDENTIFIER
            || f_node->get_string_upper() != "AND")
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                msg << f_node->get_location().get_location()
                    << "expected AND between the lower and higher bounds of [NOT] BETWEEN operator.";
                throw invalid_token(msg.str());
            }

            f_node = f_lexer->get_next_token();
            n->insert_child(-1, parse_expr_other());

            if(negate)
            {
                // Note: ... NOT BETWEEN ... AND ...
                //       is equivalent to:
                //       NOT ( ... BETWEEN ... AND ... )
                //
                result = std::make_shared<node>(token_t::TOKEN_LOGICAL_NOT, n->get_location());
                result->insert_child(-1, n);
            }
            else
            {
                result = n;
            }
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
            node::pointer_t n;
            if(keyword == "LIKE")
            {
                n = std::make_shared<node>(token_t::TOKEN_LIKE, f_node->get_location());
            }
            else if(keyword == "ILIKE")
            {
                n = std::make_shared<node>(token_t::TOKEN_ILIKE, f_node->get_location());
            }
            else if(keyword == "SIMILAR")
            {
                n = std::make_shared<node>(token_t::TOKEN_SIMILAR, f_node->get_location());

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
            if(n != nullptr)
            {
                n->insert_child(-1, result);

                // we expect a string, so there is really no need to check
                // for the Boolean expressions
                //
                n->insert_child(-1, parse_expr_other());

                result = n;
            }
        }
    }
    else if(negate)
    {
        snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
        msg << f_node->get_location().get_location()
            << "expected NOT to be followed by BETWEEN, IN, LIKE, ILIKE, or SIMILAR TO.";
        throw invalid_token(msg.str());
    }
    return result;
}


node::pointer_t expr_state::parse_expr_function_parameters(std::string const & keyword, int count)
{
    f_node = f_lexer->get_next_token();
    if(f_node->get_token() != token_t::TOKEN_OPEN_PARENTHESIS)
    {
        snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
        msg << f_node->get_location().get_location()
            << "expected '(' to start the list of parameters in a function call.";
        throw invalid_token(msg.str());
    }
    f_node = f_lexer->get_next_token();
    node::pointer_t result;
    if(f_node->get_token() != token_t::TOKEN_CLOSE_PARENTHESIS)
    {
        result = parse_expr_list();
    }
    else
    {
        result = std::make_shared<node>(token_t::TOKEN_LIST, f_node->get_location());
    }
    if(f_node->get_token() != token_t::TOKEN_CLOSE_PARENTHESIS)
    {
        snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
        msg << f_node->get_location().get_location()
            << "expected '(' to start the list of parameters in a function call.";
        throw invalid_token(msg.str());
    }
    if(count >= 0
    && static_cast<std::size_t>(count) != result->get_children_size())
    {
        snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
        msg << f_node->get_location().get_location()
            << keyword
            << "() expected "
            << count
            << " parameter"
            << (count != 1 ? "s" : "")
            << ", found "
            << result->get_children_size()
            << " instead.";
        throw invalid_parameter(msg.str());
    }
    return result;
}


node::pointer_t expr_state::parse_expr_cast_value(std::string const & type_name)
{
    bool has_parenthesis(false);
    location const l(f_node->get_location());
    f_node = f_lexer->get_next_token();
    if(f_node->get_token() == token_t::TOKEN_OPEN_PARENTHESIS)
    {
        has_parenthesis = true;
        f_node = f_lexer->get_next_token();
    }
    node::pointer_t value(parse_expr_logical_or());
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
    node::pointer_t result(std::make_shared<node>(token_t::TOKEN_CAST, l));
    result->set_string(type_name);
    result->insert_child(-1, value);
    return result;
}


node::pointer_t expr_state::parse_expr_other()
{
    // this one is really strange since it can start with a primary like
    // expression (@ <?> and |/ <?> for example)
    //
    node::pointer_t result;
    location const l(f_node->get_location());

    auto function_call = [&](std::string const & name, node::pointer_t params)
    {
        result = std::make_shared<node>(token_t::TOKEN_FUNCTION_CALL, l);
        result->set_string(name);
        result->insert_child(-1, params);
        return result;
    };

    switch(f_node->get_token())
    {
    case token_t::TOKEN_ABSOLUTE_VALUE:
        return function_call("Math.abs", parse_expr_other());

    case token_t::TOKEN_SQUARE_ROOT:
        return function_call("Math.sqrt", parse_expr_other());

    case token_t::TOKEN_CUBE_ROOT:
        return function_call("Math.cbrt", parse_expr_other());

    case token_t::TOKEN_IDENTIFIER:
        // type cast or function call
        {
            int parameter_count(-2);
            std::string keyword(f_node->get_string_upper());
            switch(keyword[0])
            {
            case 'A':
                if(keyword == "ABS"
                || keyword == "ACOS"
                || keyword == "ACOSH"
                || keyword == "ASIN"
                || keyword == "ASINH"
                || keyword == "ATANH")
                {
                    parameter_count = 1;
                }
                else if(keyword == "ATAN")
                {
                    node::pointer_t params(parse_expr_function_parameters(keyword, -1));
                    if(params->get_children_size() == 1)
                    {
                        return function_call("Math.atan", params);
                    }
                    else if(params->get_children_size() == 2)
                    {
                        return function_call("Math.atan2", params);
                    }
                    else
                    {
                        snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                        msg << f_node->get_location().get_location()
                            << "expected 1 or 2 parameters to ATAN(), found "
                            << params->get_children_size()
                            << " instead.";
                        throw invalid_parameter(msg.str());
                    }
                    snapdev::NOT_REACHED();
                }
                break;

            case 'B':
                if(keyword == "BIGINT")
                {
                    return parse_expr_cast_value("Integer");
                }
                else if(keyword == "BOOLEAN")
                {
                    return parse_expr_cast_value("Boolean");
                }
                break;

            case 'C':
                if(keyword == "CBRT"
                || keyword == "CEIL"
                || keyword == "COS"
                || keyword == "COSH")
                {
                    parameter_count = 1;
                }
                else if(keyword == "CHAR")
                {
                    return parse_expr_cast_value("String");
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
                    return parse_expr_cast_value("Number");
                }
                break;

            case 'E':
                if(keyword == "EXP"
                || keyword == "EXPM1")
                {
                    parameter_count = 1;
                }
                break;

            case 'F':
                if(keyword == "FLOOR"
                || keyword == "FROUND")
                {
                    parameter_count = 1;
                }
                else if(keyword == "FLOAT2"
                     || keyword == "FLOAT4"
                     || keyword == "FLOAT10")
                {
                    return parse_expr_cast_value("Number");
                }
                break;

            case 'H':
                if(keyword == "HYPOT")
                {
                    // with 0 parameters, the function returns 0
                    // with 1 parameter, the function returns that parameter as is
                    // with 2 or more, it returns the square root of the sum of the squares
                    //
                    parameter_count = -1;
                }
                break;

            case 'I':
                if(keyword == "INUL")
                {
                    parameter_count = 1;
                }
                else if(keyword == "INT"
                     || keyword == "INT1"
                     || keyword == "INT2"
                     || keyword == "INT4"
                     || keyword == "INT8"
                     || keyword == "INT16"
                     || keyword == "INT32"
                     || keyword == "INT64"
                     || keyword == "INTEGER")
                {
                    // TODO: add support for bigint in as2js so larger numbers
                    //       can be more than 64 bits
                    //
                    return parse_expr_cast_value("Integer");
                }
                break;

            case 'L':
                if(keyword == "LOG"
                || keyword == "LOG1P"
                || keyword == "LOG10"
                || keyword == "LOG2")
                {
                    parameter_count = 1;
                }
                break;

            case 'M':
                if(keyword == "MAX"
                || keyword == "MIN")
                {
                    parameter_count = -1;
                }
                break;

            case 'P':
                if(keyword == "POW")
                {
                    parameter_count = 2;
                }
                break;

            case 'R':
                if(keyword == "RAND")
                {
                    parameter_count = 0;
                }
                else if(keyword == "REAL")
                {
                    return parse_expr_cast_value("Number");
                }
                else if(keyword == "ROUND")
                {
                    parameter_count = 1;
                }
                break;

            case 'S':
                if(keyword == "SIGN"
                || keyword == "SIN"
                || keyword == "SINH"
                || keyword == "SQRT")
                {
                    parameter_count = 1;
                }
                else if(keyword == "SMALLINT")
                {
                    return parse_expr_cast_value("Integer");
                }
                break;

            case 'T':
                if(keyword == "TAN"
                || keyword == "TANH"
                || keyword == "TRUNC")
                {
                    parameter_count = 1;
                }
                else if(keyword == "TEXT")
                {
                    return parse_expr_cast_value("String");
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
                        // TODO: add support for bigint so larger numbers
                        //       can be more than 64 bits
                        //
                        return parse_expr_cast_value("Integer");
                    }
                }
                break;

            }
            if(parameter_count != -2)
            {
                return function_call(
                              "Math." + snapdev::to_lower(keyword)
                            , parse_expr_function_parameters(keyword, parameter_count));
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
        case token_t::TOKEN_BITWISE_OR:
        case token_t::TOKEN_BITWISE_XOR:
        case token_t::TOKEN_SHIFT_LEFT:
        case token_t::TOKEN_SHIFT_RIGHT:
            f_node->insert_child(-1, result);
            result = f_node;
            f_node = f_lexer->get_next_token();
            result->insert_child(-1, parse_expr_additive());
            break;

        case token_t::TOKEN_STRING_CONCAT:
            {
                node::pointer_t params(std::make_shared<node>(token_t::TOKEN_LIST, l));
                params->insert_child(-1, result);
                for(;;)
                {
                    f_node = f_lexer->get_next_token();
                    node::pointer_t n(parse_expr_additive());
                    if(params->get_children_size() > 0
                    && n->is_literal())
                    {
                        node::pointer_t p(params->get_child(params->get_children_size() - 1));
                        if(p->is_literal())
                        {
                            std::string const concatenated(p->get_string_auto_convert() + n->get_string_auto_convert());
                            if(p->get_token() == token_t::TOKEN_STRING)
                            {
                                // already a string, we can do it in place
                                //
                                p->set_string(concatenated);
                            }
                            else
                            {
                                // the existing parameter is not a string,
                                // create a new node to replace it and
                                // save the result there
                                //
                                node::pointer_t str(std::make_shared<node>(token_t::TOKEN_STRING, p->get_location()));
                                str->set_string(concatenated);
                                params->set_child(params->get_children_size() - 1, str);
                            }
                        }
                        else
                        {
                            params->insert_child(-1, n);
                        }
                    }
                    else
                    {
                        params->insert_child(-1, n);
                    }
                    if(f_node->get_token() != token_t::TOKEN_STRING_CONCAT)
                    {
                        break;
                    }
                }
                if(params->get_children_size() == 1)
                {
                    // all were merged, we do not need to keep a complicated
                    // function call when we can just use a literal string
                    //
                    result = params->get_child(0);
                }
                else
                {
                    result = function_call("String.concat", params);
                }
            }
            break;

        case token_t::TOKEN_REGULAR_EXPRESSION:
            // TBD: we could also create TOKEN_PERIOD + the new RegExpr
            //      on the left handside and the test() call on the right
            //      handside--however, done in this way we can detect
            //      whether the two sides are string literal and if so
            //      change the expression in a Boolean at compile time
            //
            f_node->insert_child(-1, result);
            result = f_node;
            f_node = f_lexer->get_next_token();
            result->insert_child(-1, parse_expr_additive());
            break;

        default:
            return result;

        }
    }
}


node::pointer_t expr_state::parse_expr_additive()
{
    node::pointer_t result(parse_expr_multiplicative());
    for(;;)
    {
        switch(f_node->get_token())
        {
        case token_t::TOKEN_PLUS:
        case token_t::TOKEN_MINUS:
            f_node->insert_child(-1, result);
            result = f_node;
            f_node = f_lexer->get_next_token();
            result->insert_child(-1, parse_expr_multiplicative());
            break;

        default:
            return result;

        }
    }
    snapdev::NOT_REACHED();
}


node::pointer_t expr_state::parse_expr_multiplicative()
{
    node::pointer_t result(parse_expr_exponentiation());
    for(;;)
    {
        switch(f_node->get_token())
        {
        case token_t::TOKEN_MULTIPLY:
        case token_t::TOKEN_DIVIDE:
        case token_t::TOKEN_MODULO:
            f_node->insert_child(-1, result);
            result = f_node;
            f_node = f_lexer->get_next_token();
            result->insert_child(-1, parse_expr_exponentiation());
            break;

        default:
            return result;

        }
    }
    snapdev::NOT_REACHED();
}


node::pointer_t expr_state::parse_expr_exponentiation()
{
    node::pointer_t result(parse_expr_unary());
    for(;;)
    {
        switch(f_node->get_token())
        {
        case token_t::TOKEN_POWER:
            // Note: in as2js the exponentiation is right to left
            //       (like in math/Ada); but here it's left to right!
            //
            f_node->insert_child(-1, result);
            result = f_node;
            f_node = f_lexer->get_next_token();
            result->insert_child(-1, parse_expr_unary());
            break;

        default:
            return result;

        }
    }
    snapdev::NOT_REACHED();
}


node::pointer_t expr_state::parse_expr_unary()
{
    node::pointer_t result;
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
            if(result == nullptr)
            {
                result = f_node;
            }
            else
            {
                result.reset();
            }
            f_node = f_lexer->get_next_token();
            break;

        default:
            if(result != nullptr)
            {
                result->insert_child(-1, parse_expr_postfix());
                return result;
            }
            return parse_expr_postfix();

        }
    }
    snapdev::NOT_REACHED();
}


node::pointer_t expr_state::parse_expr_postfix()
{
    bool found_all_fields(false);
    node::pointer_t result(parse_expr_primary());
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
            f_node->insert_child(-1, result);
            result = f_node;
            f_node = f_lexer->get_next_token();
            if(f_node->get_token() == token_t::TOKEN_MULTIPLY)
            {
                // special case were we want all the fields of a table,
                // record, etc.
                //
                result->insert_child(-1, std::make_shared<node>(token_t::TOKEN_ALL_FIELDS, f_node->get_location()));
                f_node = f_lexer->get_next_token();
                found_all_fields = true;
            }
            else if(f_node->get_token() == token_t::TOKEN_IDENTIFIER)
            {
                f_node->set_string(f_node->get_string_lower());
                result->insert_child(-1, f_node);
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
                location loc(f_node->get_location());
                std::string const l(loc.get_location());
                auto cast_value = [&](char const * type_name)
                {
                    node::pointer_t n(std::make_shared<node>(token_t::TOKEN_CAST, loc));
                    n->set_string(type_name);
                    n->insert_child(-1, result);
                    return n;
                };
                std::string keyword(f_node->get_string_upper());
                bool found(true);
                switch(keyword[0])
                {
                case 'B':
                    if(keyword == "BIGINT")
                    {
                        result = cast_value("Integer");
                    }
                    else if(keyword == "BOOLEAN")
                    {
                        result = cast_value("Boolean");
                    }
                    else
                    {
                        found = false;
                    }
                    break;

                case 'C':
                    if(keyword == "CHAR")
                    {
                        result = cast_value("String");
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
                        result = cast_value("Number");
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
                        result = cast_value("Integer");
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
                        result = cast_value("Number");
                    }
                    else
                    {
                        found = false;
                    }
                    break;

                case 'R':
                    if(keyword == "REAL")
                    {
                        result = cast_value("Number");
                    }
                    else
                    {
                        found = false;
                    }
                    break;

                case 'S':
                    if(keyword == "SMALLINT")
                    {
                        result = cast_value("Integer");
                    }
                    else
                    {
                        found = false;
                    }
                    break;

                case 'T':
                    if(keyword == "TEXT")
                    {
                        result = cast_value("String");
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
                            result = cast_value("Integer");
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
            f_node = std::make_shared<node>(token_t::TOKEN_AT, f_node->get_location());
            f_node->insert_child(-1, result);
            result = f_node;
            f_node = f_lexer->get_next_token();
            result->insert_child(-1, parse_expr_logical_or());
            if(f_node->get_token() != token_t::TOKEN_CLOSE_BRACKET)
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                msg << f_node->get_location().get_location()
                    << "expected a closing square bracket (]), not "
                    << to_string(f_node->get_token())
                    << ".";
                throw invalid_token(msg.str());
            }
            f_node = f_lexer->get_next_token();
            break;

        default:
            return result;

        }
    }
    snapdev::NOT_REACHED();
}


node::pointer_t expr_state::parse_expr_primary()
{
    node::pointer_t result;
    switch(f_node->get_token())
    {
    case token_t::TOKEN_STRING:
        result = f_node;
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
            result->set_string(result->get_string() + f_node->get_string());
            f_node = f_lexer->get_next_token();
        }
        return result;

    case token_t::TOKEN_IDENTIFIER:
        {
            std::string const keyword(f_node->get_string_lower());
            if(keyword == "true")
            {
                result = std::make_shared<node>(token_t::TOKEN_TRUE, f_node->get_location());
            }
            else if(keyword == "false")
            {
                result = std::make_shared<node>(token_t::TOKEN_FALSE, f_node->get_location());
            }
            else
            {
                result = f_node;
                result->set_string(keyword); // save the lowercase version
            }
            f_node = f_lexer->get_next_token();
            return result;
        }

    case token_t::TOKEN_FLOATING_POINT:
        //result = snapdev::floating_point_to_string<double, char>(f_node->get_floating_point());
        result = f_node;
        f_node = f_lexer->get_next_token();
        return result;

    case token_t::TOKEN_INTEGER:
        //result = to_string(f_node->get_integer());
        result = f_node;
        f_node = f_lexer->get_next_token();
        return result;

    case token_t::TOKEN_MULTIPLY:
        result = std::make_shared<node>(token_t::TOKEN_ALL_FIELDS, f_node->get_location());
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
    node::pointer_t tree(s.parse_expr_logical_or());
    std::string const result(tree->to_as2js());
    n = s.f_node;
    return result;
}



} // namespace pbql
} // namespace prinbee
// vim: ts=4 sw=4 et
