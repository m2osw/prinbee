// Copyright (c) 2024-2025  Made to Order Software Corp.  All Rights Reserved
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
 * | <function> ( <?> [, <?> ...] ) | left           | function call                                           |
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


// snaplogger
//
#include    <snaplogger/message.h>


// snapdev
//
#include    <snapdev/math.h>
#include    <snapdev/to_lower.h>


// C++
//
#include    <regex>


// last include
//
#include    <snapdev/poison.h>



namespace prinbee
{
namespace pbql
{



namespace
{



enum class function_t
{
    FUNCTION_ABS,
    FUNCTION_ACOS,
    FUNCTION_ACOSH,
    FUNCTION_ASIN,
    FUNCTION_ASINH,
    FUNCTION_ATAN,
    FUNCTION_ATAN2,
    FUNCTION_ATANH,
    FUNCTION_CBRT,
    FUNCTION_CEIL,
    FUNCTION_CONCAT,
    FUNCTION_COS,
    FUNCTION_COSH,
    FUNCTION_EXP,
    FUNCTION_EXPM1,
    FUNCTION_FLOOR,
    FUNCTION_HYPOT,
    FUNCTION_IMUL,
    FUNCTION_LENGTH,
    FUNCTION_LOG,
    FUNCTION_LOG1P,
    FUNCTION_LOG10,
    FUNCTION_LOG2,
    FUNCTION_MAX,
    FUNCTION_MIN,
    FUNCTION_POW,
    FUNCTION_RAND,
    FUNCTION_ROUND,
    FUNCTION_SIGN,
    FUNCTION_SIN,
    FUNCTION_SINH,
    FUNCTION_SQRT,
    FUNCTION_TAN,
    FUNCTION_TANH,
    FUNCTION_TRUNC,
};


struct expr_state
{
    lexer::pointer_t    f_lexer = lexer::pointer_t();
    node::pointer_t     f_node = node::pointer_t(); // current token
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
    node::pointer_t     parse_expr_cast_value(type_t cast_to);
    node::pointer_t     parse_expr_other();
    node::pointer_t     parse_expr_additive();
    node::pointer_t     parse_expr_multiplicative();
    node::pointer_t     parse_expr_exponentiation();
    node::pointer_t     parse_expr_unary();
    node::pointer_t     parse_expr_postfix();
    node::pointer_t     parse_expr_primary();

    node::pointer_t     function_call(location const & l, function_t func, node::pointer_t params);
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
} // LCOV_EXCL_LINE


node::pointer_t expr_state::parse_expr_logical_or()
{
    node::pointer_t lhs(parse_expr_logical_and());
    while(f_node->get_token() == token_t::TOKEN_IDENTIFIER
       && f_node->get_string_upper() == "OR")
    {
        location l(f_node->get_location());
        f_node = f_lexer->get_next_token();
        node::pointer_t rhs(parse_expr_logical_and());

        bool const lliteral(lhs->is_literal(token_t::TOKEN_BOOLEAN));
        bool const rliteral(rhs->is_literal(token_t::TOKEN_BOOLEAN));

        bool const lvalue(lliteral
                        ? lhs->get_boolean_auto_convert()
                        : false);
        bool const rvalue(rliteral
                        ? rhs->get_boolean_auto_convert()
                        : false);
        bool const value(lvalue || rvalue);

        if(value
        || (lliteral && rliteral))
        {
            // WARNING: the 'value || ...' removes potential side effects
            //          of the other expression (but at the moment, I do
            //          not know of any such possible side effects in our
            //          expressions)
            //
            lhs = std::make_shared<node>(value
                                ? token_t::TOKEN_TRUE
                                : token_t::TOKEN_FALSE, l);
        }
        else if(lliteral)
        {
            lhs = rhs;
        }
        else if(!rliteral)
        {
            node::pointer_t n(std::make_shared<node>(token_t::TOKEN_LOGICAL_OR, l));
            n->insert_child(-1, lhs);
            n->insert_child(-1, rhs);
            lhs = n;
        }
    }
    return lhs;
} // LCOV_EXCL_LINE


node::pointer_t expr_state::parse_expr_logical_and()
{
    node::pointer_t lhs(parse_expr_logical_not());
    while(f_node->get_token() == token_t::TOKEN_IDENTIFIER
       && f_node->get_string_upper() == "AND")
    {
        location l(f_node->get_location());
        f_node = f_lexer->get_next_token();
        node::pointer_t rhs(parse_expr_logical_not());

        bool const lliteral(lhs->is_literal(token_t::TOKEN_BOOLEAN));
        bool const rliteral(rhs->is_literal(token_t::TOKEN_BOOLEAN));

        bool const lvalue(lliteral
                        ? lhs->get_boolean_auto_convert()
                        : true);
        bool const rvalue(rliteral
                        ? rhs->get_boolean_auto_convert()
                        : true);
        bool const value(lvalue && rvalue);

        if(!value
        || (lliteral && rliteral))
        {
            // WARNING: the 'value && ...' removes potential side effects
            //          of the other expression (but at the moment, I do
            //          not know of any such possible side effects in our
            //          expressions)
            //
            lhs = std::make_shared<node>(value
                                ? token_t::TOKEN_TRUE
                                : token_t::TOKEN_FALSE, l);
        }
        else if(lliteral)
        {
            lhs = rhs;
        }
        else if(!rliteral)
        {
            node::pointer_t n(std::make_shared<node>(token_t::TOKEN_LOGICAL_AND, l));
            n->insert_child(-1, lhs);
            n->insert_child(-1, rhs);
            lhs = n;
        }
    }
    return lhs;
} // LCOV_EXCL_LINE


node::pointer_t expr_state::parse_expr_logical_not()
{
    bool has_logical_not(false);
    bool logical_not(false);
    while(f_node->get_token() == token_t::TOKEN_IDENTIFIER
       && f_node->get_string_upper() == "NOT")
    {
        has_logical_not = true;
        f_node = f_lexer->get_next_token();
        logical_not = !logical_not;
    }
    node::pointer_t result(parse_expr_is());
    if(result->get_token() == token_t::TOKEN_NULL)
    {
        // in SQL: NULL = NOT NULL is true
        //         (although NULL IS NOT NULL is false...)
        //
        return result;
    }
    if(has_logical_not
    && result->is_literal(token_t::TOKEN_BOOLEAN))
    {
        bool const value(result->get_boolean_auto_convert());
        node::pointer_t n(std::make_shared<node>(
                value ^ logical_not
                    ? token_t::TOKEN_TRUE
                    : token_t::TOKEN_FALSE, f_node->get_location()));
        return n;
    }
    if(logical_not)
    {
        node::pointer_t n(std::make_shared<node>(token_t::TOKEN_LOGICAL_NOT, f_node->get_location()));
        n->insert_child(-1, result);
        return n;
    }
    if(has_logical_not)
    {
        // NOT NOT <something> is expected to return a Boolean
        //
        node::pointer_t n(std::make_shared<node>(token_t::TOKEN_LOGICAL_NOT, f_node->get_location()));
        node::pointer_t m(std::make_shared<node>(token_t::TOKEN_LOGICAL_NOT, f_node->get_location()));
        n->insert_child(-1, m);
        m->insert_child(-1, result);
        return n;
    }
    return result;
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
        if(f_node->get_token() != token_t::TOKEN_IDENTIFIER)
        {
            snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
            msg << f_node->get_location().get_location()
                << "expected one of TRUE, FALSE, NULL or DISTINCT after IS, not "
                << to_string(f_node->get_token())
                << ".";
            throw invalid_token(msg.str());
        }

        location l(f_node->get_location());
        std::string const keyword(f_node->get_string_upper());
        if(keyword == "TRUE")
        {
            f_node = f_lexer->get_next_token();
            if(result->is_literal())
            {
                result = std::make_shared<node>(result->get_token() == token_t::TOKEN_TRUE ^ negate
                                                    ? token_t::TOKEN_TRUE
                                                    : token_t::TOKEN_FALSE, l);
            }
            else if(negate)
            {
                node::pointer_t n(std::make_shared<node>(token_t::TOKEN_LOGICAL_NOT, l));
                n->insert_child(-1, result);
                result = n;
            }
            // else -- should we do (!!<expr>)?
        }
        else if(keyword == "FALSE")
        {
            f_node = f_lexer->get_next_token();
            if(result->is_literal())
            {
                result = std::make_shared<node>(result->get_token() == token_t::TOKEN_FALSE ^ negate
                                                    ? token_t::TOKEN_TRUE
                                                    : token_t::TOKEN_FALSE, l);
            }
            else if(!negate)
            {
                node::pointer_t n(std::make_shared<node>(token_t::TOKEN_LOGICAL_NOT, l));
                n->insert_child(-1, result);
                result = n;
            }
            // else -- should we do (!!<expr>)?
        }
        else if(keyword == "NULL")
        {
            f_node = f_lexer->get_next_token();
            if(result->is_literal())
            {
                result = std::make_shared<node>(result->get_token() == token_t::TOKEN_NULL ^ negate
                                                    ? token_t::TOKEN_TRUE
                                                    : token_t::TOKEN_FALSE, l);
            }
            else
            {
                node::pointer_t n(std::make_shared<node>(negate ? token_t::TOKEN_NOT_EQUAL : token_t::TOKEN_EQUAL, l));
                n->insert_child(-1, result);
                n->insert_child(-1, std::make_shared<node>(token_t::TOKEN_NULL, l));
                result = n;
            }
        }
        else if(keyword == "DISTINCT")
        {
            f_node = f_lexer->get_next_token();
            if(f_node->get_token() != token_t::TOKEN_IDENTIFIER
            || f_node->get_string_upper() != "FROM")
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                msg << f_node->get_location().get_location()
                    << "expected FROM after IS [NOT] DISTINCT.";
                throw invalid_token(msg.str());
            }
            f_node = f_lexer->get_next_token();
            throw not_yet_implemented("IS [NOT] DISTINCT FROM is not yet implemented.");
        }
        else
        {
            snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
            msg << f_node->get_location().get_location()
                << "expected one of TRUE, FALSE, NULL or DISTINCT after IS, not "
                << keyword
                << ".";
            throw invalid_token(msg.str());
        }
    }
    return result;
}


node::pointer_t expr_state::parse_expr_comparison()
{
    node::pointer_t lhs(parse_expr_matching());
    node::pointer_t rhs;
    for(;;)
    {
        token_t const compare_operator(f_node->get_token());
        switch(compare_operator)
        {
        case token_t::TOKEN_LESS:
        case token_t::TOKEN_LESS_EQUAL:
        case token_t::TOKEN_EQUAL:
        case token_t::TOKEN_GREATER:
        case token_t::TOKEN_GREATER_EQUAL:
        case token_t::TOKEN_NOT_EQUAL:
            {
                node::pointer_t n(f_node);
                f_node = f_lexer->get_next_token();
                rhs = parse_expr_matching();
                if(lhs->is_literal()
                && rhs->is_literal())
                {
                    // if lhs is NULL then we're done
                    //
                    if(lhs->get_token() != token_t::TOKEN_NULL)
                    {
                        if(rhs->get_token() == token_t::TOKEN_NULL)
                        {
                            lhs = rhs;
                        }
                        else
                        {
                            bool result(false);
                            if(lhs->is_literal(token_t::TOKEN_INTEGER)
                            && rhs->is_literal(token_t::TOKEN_INTEGER))
                            {
                                // compare as integers
                                //
                                int512_t const l(lhs->get_integer_auto_convert());
                                int512_t const r(rhs->get_integer_auto_convert());
                                switch(compare_operator)
                                {
                                case token_t::TOKEN_LESS:
                                    result = l < r;
                                    break;

                                case token_t::TOKEN_LESS_EQUAL:
                                    result = l <= r;
                                    break;

                                case token_t::TOKEN_EQUAL:
                                    result = l == r;
                                    break;

                                case token_t::TOKEN_GREATER:
                                    result = l > r;
                                    break;

                                case token_t::TOKEN_GREATER_EQUAL:
                                    result = l >= r;
                                    break;

                                case token_t::TOKEN_NOT_EQUAL:
                                    result = l != r;
                                    break;

                                default: // LCOV_EXCL_LINE
                                    throw logic_error("literal handling not properly implemented."); // LCOV_EXCL_LINE

                                }
                            }
                            else if(lhs->is_literal(token_t::TOKEN_NUMBER)
                                 && rhs->is_literal(token_t::TOKEN_NUMBER))
                            {
                                // compare as floating points
                                //
                                long double const l(lhs->get_floating_point_auto_convert());
                                long double const r(rhs->get_floating_point_auto_convert());
                                switch(compare_operator)
                                {
                                case token_t::TOKEN_LESS:
                                    result = l < r;
                                    break;

                                case token_t::TOKEN_LESS_EQUAL:
                                    result = l <= r;
                                    break;

                                case token_t::TOKEN_EQUAL:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
                                    result = l == r;
#pragma GCC diagnostic pop
                                    break;

                                case token_t::TOKEN_GREATER:
                                    result = l > r;
                                    break;

                                case token_t::TOKEN_GREATER_EQUAL:
                                    result = l >= r;
                                    break;

                                case token_t::TOKEN_NOT_EQUAL:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
                                    result = l != r;
#pragma GCC diagnostic pop
                                    break;

                                default: // LCOV_EXCL_LINE
                                    throw logic_error("literal handling not properly implemented."); // LCOV_EXCL_LINE

                                }
                            }
                            else if(lhs->is_literal(token_t::TOKEN_BOOLEAN)
                                 && rhs->is_literal(token_t::TOKEN_BOOLEAN))
                            {
                                // compare as boolean
                                //
                                bool const l(lhs->get_token() == token_t::TOKEN_TRUE);
                                bool const r(rhs->get_token() == token_t::TOKEN_TRUE);
                                switch(compare_operator)
                                {
                                case token_t::TOKEN_LESS:
                                    result = l < r;
                                    break;

                                case token_t::TOKEN_LESS_EQUAL:
                                    result = l <= r;
                                    break;

                                case token_t::TOKEN_EQUAL:
                                    result = l == r;
                                    break;

                                case token_t::TOKEN_GREATER:
                                    result = l > r;
                                    break;

                                case token_t::TOKEN_GREATER_EQUAL:
                                    result = l >= r;
                                    break;

                                case token_t::TOKEN_NOT_EQUAL:
                                    result = l != r;
                                    break;

                                default: // LCOV_EXCL_LINE
                                    throw logic_error("literal handling not properly implemented."); // LCOV_EXCL_LINE

                                }
                            }
                            else
                            {
                                // otherwise compare as strings
                                //
                                std::string const l(lhs->get_string_auto_convert());
                                std::string const r(rhs->get_string_auto_convert());
                                switch(compare_operator)
                                {
                                case token_t::TOKEN_LESS:
                                    result = l < r;
                                    break;

                                case token_t::TOKEN_LESS_EQUAL:
                                    result = l <= r;
                                    break;

                                case token_t::TOKEN_EQUAL:
                                    result = l == r;
                                    break;

                                case token_t::TOKEN_GREATER:
                                    result = l > r;
                                    break;

                                case token_t::TOKEN_GREATER_EQUAL:
                                    result = l >= r;
                                    break;

                                case token_t::TOKEN_NOT_EQUAL:
                                    result = l != r;
                                    break;

                                default: // LCOV_EXCL_LINE
                                    throw logic_error("literal handling not properly implemented."); // LCOV_EXCL_LINE

                                }
                            }
                            lhs = std::make_shared<node>(result
                                        ? token_t::TOKEN_TRUE
                                        : token_t::TOKEN_FALSE, lhs->get_location());
                        }
                    }
                }
                else
                {
                    n->insert_child(-1, lhs);
                    n->insert_child(-1, rhs);
                    lhs = n;
                }
            }
            break;

        default:
            return lhs;

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
        f_node = f_lexer->get_next_token();
    }
    if(f_node->get_token() == token_t::TOKEN_IDENTIFIER)
    {
        std::string const keyword(f_node->get_string_upper());
        if(keyword == "BETWEEN")
        {
            location const l(f_node->get_location());
            node::pointer_t lhs(result);

            // WARNING: here we have to make sure the next parse_expr_...()
            //          does not manage the "AND" keyword
            //
            f_node = f_lexer->get_next_token();
            node::pointer_t rhs1(parse_expr_other());

            if(f_node->get_token() != token_t::TOKEN_IDENTIFIER
            || f_node->get_string_upper() != "AND")
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                msg << f_node->get_location().get_location()
                    << "expected AND between the lower and higher bounds of [NOT] BETWEEN operator.";
                throw invalid_token(msg.str());
            }

            f_node = f_lexer->get_next_token();
            node::pointer_t rhs2(parse_expr_other());

            if(lhs->is_literal()
            && rhs1->is_literal()
            && rhs2->is_literal())
            {
                token_t r(token_t::TOKEN_FALSE);
                switch(lhs->get_token())
                {
                case token_t::TOKEN_STRING:
                    {
                        std::string const v(lhs->get_string());
                        std::string const b1(rhs1->get_string_auto_convert());
                        std::string const b2(rhs2->get_string_auto_convert());
                        if(v >= b1 && v <= b2)
                        {
                            r = token_t::TOKEN_TRUE;
                        }
                    }
                    break;

                case token_t::TOKEN_INTEGER:
                    {
                        int512_t const v(lhs->get_integer());
                        int512_t const b1(rhs1->get_integer_auto_convert());
                        int512_t const b2(rhs2->get_integer_auto_convert());
                        if(v >= b1 && v <= b2)
                        {
                            r = token_t::TOKEN_TRUE;
                        }
                    }
                    break;

                case token_t::TOKEN_FLOATING_POINT:
                    {
                        long double const v(lhs->get_floating_point());
                        long double const b1(rhs1->get_floating_point_auto_convert());
                        long double const b2(rhs2->get_floating_point_auto_convert());
                        if(v >= b1 && v <= b2)
                        {
                            r = token_t::TOKEN_TRUE;
                        }
                    }
                    break;

                case token_t::TOKEN_NULL:
                    r = token_t::TOKEN_NULL;
                    break;

                case token_t::TOKEN_TRUE:
                    {
                        bool const b1(rhs1->get_token() == token_t::TOKEN_TRUE);
                        bool const b2(rhs2->get_token() == token_t::TOKEN_TRUE);
                        if(true >= b1 && true <= b2)
                        {
                            r = token_t::TOKEN_TRUE;
                        }
                    }
                    break;

                case token_t::TOKEN_FALSE:
                    {
                        bool const b1(rhs1->get_token() == token_t::TOKEN_TRUE);
                        bool const b2(rhs2->get_token() == token_t::TOKEN_TRUE);
                        if(false >= b1 && false <= b2)
                        {
                            r = token_t::TOKEN_TRUE;
                        }
                    }
                    break;

                default: // LCOV_EXCL_LINE
                    throw logic_error("literal handling not properly implemented."); // LCOV_EXCL_LINE

                }
                if(negate)
                {
                    if(r == token_t::TOKEN_TRUE)
                    {
                        r = token_t::TOKEN_FALSE;
                    }
                    else if(r == token_t::TOKEN_FALSE)
                    {
                        r = token_t::TOKEN_TRUE;
                    }
                }
                result = std::make_shared<node>(r, l);
            }
            else
            {
                node::pointer_t n(std::make_shared<node>(token_t::TOKEN_BETWEEN, l));
                n->insert_child(-1, lhs);
                n->insert_child(-1, rhs1);
                n->insert_child(-1, rhs2);

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
            return result;
        }
        else if(keyword == "IN")
        {
            // TODO: not too what the right hand side would end up being
            //       in this case... (array, sub-select...)
            //
            throw not_yet_implemented("[NOT] IN is not yet implemented.");
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
            if(n != nullptr)
            {
                f_node = f_lexer->get_next_token();

                node::pointer_t lhs(result);

                // we expect a string for the pattern, so there is really
                // no need to check for the Boolean expressions
                //
                node::pointer_t rhs(parse_expr_other());

                if(lhs->is_literal()
                && rhs->is_literal())
                {
                    // we can test right now and transform this expression
                    // to TRUE or FALSE here
                    //
                    std::string const value(lhs->get_string_auto_convert());
                    std::string const regex(rhs->convert_like_pattern(rhs->get_string_auto_convert()));

                    std::regex re(regex, n->get_token() == token_t::TOKEN_ILIKE
                                            ? std::regex::ECMAScript | std::regex::icase
                                            : std::regex::ECMAScript);
                    bool const matches(std::regex_match(value, re));
                    result = std::make_shared<node>(matches ^ negate
                                ? token_t::TOKEN_TRUE
                                : token_t::TOKEN_FALSE, n->get_location());
                }
                else
                {
                    n->insert_child(-1, lhs);
                    n->insert_child(-1, rhs);

                    if(negate)
                    {
                        // Note: ... NOT {[I]LIKE | SIMILAR TO} ...
                        //       NOT ( ... {[I]LIKE | SIMILAR} ... )
                        //
                        result = std::make_shared<node>(token_t::TOKEN_LOGICAL_NOT, n->get_location());
                        result->insert_child(-1, n);
                    }
                    else
                    {
                        result = n;
                    }
                }
                return result;
            }
        }
    }

    if(negate)
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
#ifdef _DEBUG
    // LCOV_EXCL_START
    if(f_node->get_token() != token_t::TOKEN_OPEN_PARENTHESIS)
    {
        snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
        msg << f_node->get_location().get_location()
            << "expected '(' to start the list of parameters in a function call.";
        throw invalid_token(msg.str());
    }
    // LCOV_EXCL_STOP
#endif
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
            << "expected ')' to end the list of parameters in a function call; not "
            << to_string(f_node->get_token())
            << ".";
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
    f_node = f_lexer->get_next_token();
    return result;
}


node::pointer_t expr_state::parse_expr_cast_value(type_t cast_to)
{
    location const l(f_node->get_location());
    bool const has_parenthesis(f_node->get_token() == token_t::TOKEN_OPEN_PARENTHESIS);
    if(has_parenthesis)
    {
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
    result->set_integer(static_cast<std::int64_t>(cast_to));
    result->insert_child(-1, value);
    return result;
}


node::pointer_t expr_state::parse_expr_other()
{
    // this one is really strange since it can start with a primary like
    // expression (@ <?> and |/ <?> for example) however I had to move
    // the function calls to postfix so it would work properly
    //
    node::pointer_t result;
    location const l(f_node->get_location());

    //auto function_call = [&](std::string const & name, node::pointer_t params)
    //{
    //    result = std::make_shared<node>(token_t::TOKEN_FUNCTION_CALL, l);
    //    result->set_string(name);
    //    if(params->get_token() != token_t::TOKEN_LIST)
    //    {
    //        node::pointer_t list(std::make_shared<node>(token_t::TOKEN_LIST, params->get_location()));
    //        list->insert_child(-1, params);
    //        params = list;
    //    }
    //    result->insert_child(-1, params);
    //    return result;
    //};

    switch(f_node->get_token())
    {
    case token_t::TOKEN_ABSOLUTE_VALUE:
        f_node = f_lexer->get_next_token();
        return function_call(l, function_t::FUNCTION_ABS, parse_expr_other());

    case token_t::TOKEN_SQUARE_ROOT:
        f_node = f_lexer->get_next_token();
        return function_call(l, function_t::FUNCTION_SQRT, parse_expr_other());

    case token_t::TOKEN_CUBE_ROOT:
        f_node = f_lexer->get_next_token();
        return function_call(l, function_t::FUNCTION_CBRT, parse_expr_other());

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
            {
                node::pointer_t lhs(result);
                result = f_node;

                f_node = f_lexer->get_next_token();
                node::pointer_t rhs(parse_expr_additive());

                if(lhs->is_literal(token_t::TOKEN_NUMBER)
                && rhs->is_literal(token_t::TOKEN_NUMBER))
                {
                    int512_t const a(lhs->get_integer_auto_convert());
                    int512_t const b(rhs->get_integer_auto_convert());

                    int512_t r;
                    switch(result->get_token())
                    {
                    case token_t::TOKEN_BITWISE_AND:
                        r = a & b;
                        break;

                    case token_t::TOKEN_BITWISE_OR:
                        r = a | b;
                        break;

                    case token_t::TOKEN_BITWISE_XOR:
                        r = a ^ b;
                        break;

                    case token_t::TOKEN_SHIFT_LEFT:
                        r = a << b.f_value[0];
                        break;

                    case token_t::TOKEN_SHIFT_RIGHT:
                        r = a >> b.f_value[0];
                        break;

                    default: // LCOV_EXCL_LINE
                        throw logic_error("unsupported token in sub-switch (other integer)."); // LCOV_EXCL_LINE

                    }

                    if(lhs->get_token() != token_t::TOKEN_INTEGER)
                    {
                        lhs = std::make_shared<node>(token_t::TOKEN_INTEGER, lhs->get_location());
                    }
                    lhs->set_integer(r);

                    result = lhs;
                }
                else
                {
                    result->insert_child(-1, lhs);
                    result->insert_child(-1, rhs);
                }
            }
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
                    result = function_call(l, function_t::FUNCTION_CONCAT, params);
                }
            }
            break;

        case token_t::TOKEN_REGULAR_EXPRESSION:
        case token_t::TOKEN_UNMATCHED_REGULAR_EXPRESSION:
            // TBD: we could also create TOKEN_PERIOD + the new RegExp
            //      on the left handside and the test() call on the right
            //      handside--however, done in this way we can detect
            //      whether the two sides are string literal and if so
            //      change the expression in a Boolean at compile time
            //
            {
                bool const logical_not(f_node->get_token() == token_t::TOKEN_UNMATCHED_REGULAR_EXPRESSION);
                node::pointer_t lhs(result);
                //f_node->insert_child(-1, lhs);
                result = f_node;
                f_node = f_lexer->get_next_token();
                node::pointer_t rhs(parse_expr_additive());
                //result->insert_child(-1, rhs);
                if(lhs->is_literal()
                && rhs->is_literal())
                {
                    std::string const value(lhs->get_string_auto_convert());
                    std::string const regex(rhs->get_string_auto_convert());

                    std::regex re(regex);
                    bool const matches(std::regex_match(value, re));
                    result = std::make_shared<node>(matches ^ logical_not
                                ? token_t::TOKEN_TRUE
                                : token_t::TOKEN_FALSE, result->get_location());
                }
                else if(logical_not)
                {
                    node::pointer_t p(std::make_shared<node>(token_t::TOKEN_REGULAR_EXPRESSION, result->get_location()));
                    p->insert_child(-1, lhs);
                    p->insert_child(-1, rhs);

                    result = std::make_shared<node>(token_t::TOKEN_LOGICAL_NOT, result->get_location());
                    result->insert_child(-1, p);
                }
                else
                {
                    result->insert_child(-1, lhs);
                    result->insert_child(-1, rhs);
                }
            }
            break;

        default:
            return result;

        }
    }
}


node::pointer_t expr_state::parse_expr_additive()
{
    node::pointer_t lhs(parse_expr_multiplicative());
    for(;;)
    {
        switch(f_node->get_token())
        {
        case token_t::TOKEN_PLUS:
        case token_t::TOKEN_MINUS:
            {
                node::pointer_t additive(f_node);
                f_node = f_lexer->get_next_token();
                node::pointer_t rhs(parse_expr_multiplicative());

                bool const lliteral(lhs->is_literal(token_t::TOKEN_NUMBER));
                bool const rliteral(rhs->is_literal(token_t::TOKEN_NUMBER));
                if(lliteral && rliteral)
                {
                    // do computation on the fly
                    //
                    if(lhs->is_literal(token_t::TOKEN_INTEGER)
                    && rhs->is_literal(token_t::TOKEN_INTEGER))
                    {
                        int512_t const a(lhs->get_integer_auto_convert());
                        int512_t const b(rhs->get_integer_auto_convert());

                        int512_t r;
                        switch(additive->get_token())
                        {
                        case token_t::TOKEN_PLUS:
                            r = a + b;
                            break;

                        case token_t::TOKEN_MINUS:
                            r = a - b;
                            break;

                        default: // LCOV_EXCL_LINE
                            throw logic_error("unsupported token in sub-switch (integer)."); // LCOV_EXCL_LINE

                        }

                        if(lhs->get_token() != token_t::TOKEN_INTEGER)
                        {
                            lhs = std::make_shared<node>(token_t::TOKEN_INTEGER, lhs->get_location());
                        }
                        lhs->set_integer(r);
                    }
                    else
                    {
                        long double const a(lhs->get_floating_point_auto_convert());
                        long double const b(rhs->get_floating_point_auto_convert());
                        long double r;
                        switch(additive->get_token())
                        {
                        case token_t::TOKEN_PLUS:
                            r = a + b;
                            break;

                        case token_t::TOKEN_MINUS:
                            r = a - b;
                            break;

                        default: // LCOV_EXCL_LINE
                            throw logic_error("unsupported token in sub-switch (floating point)."); // LCOV_EXCL_LINE

                        }
                        if(lhs->get_token() != token_t::TOKEN_FLOATING_POINT)
                        {
                            lhs = std::make_shared<node>(token_t::TOKEN_FLOATING_POINT, lhs->get_location());
                        }
                        lhs->set_floating_point(r);
                    }
                }
                else if((lhs->is_literal() && !lliteral)
                     || (rhs->is_literal() && !rliteral))
                {
                    snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                    msg << f_node->get_location().get_location()
                        << "the + and - binary operators expect numbers as input.";
                    throw invalid_token(msg.str());
                }
                else
                {
                    additive->insert_child(-1, lhs);
                    additive->insert_child(-1, rhs);
                    lhs = additive;
                }
            }
            break;

        default:
            return lhs;

        }
    }
    snapdev::NOT_REACHED();
} // LCOV_EXCL_LINE


node::pointer_t expr_state::parse_expr_multiplicative()
{
    node::pointer_t lhs(parse_expr_exponentiation());
    for(;;)
    {
        switch(f_node->get_token())
        {
        case token_t::TOKEN_MULTIPLY:
        case token_t::TOKEN_DIVIDE:
        case token_t::TOKEN_MODULO:
            {
                node::pointer_t multiplicative(f_node);
                f_node = f_lexer->get_next_token();
                node::pointer_t rhs(parse_expr_exponentiation());

                // TODO: fix case of empty string as in additive() above
                //
                if(lhs->is_literal(token_t::TOKEN_NUMBER)
                && rhs->is_literal(token_t::TOKEN_NUMBER))
                {
                    // do computation on the fly
                    //
                    if(lhs->is_literal(token_t::TOKEN_INTEGER)
                    && rhs->is_literal(token_t::TOKEN_INTEGER))
                    {
                        int512_t const a(lhs->get_integer_auto_convert());
                        int512_t const b(rhs->get_integer_auto_convert());

                        int512_t r;
                        switch(multiplicative->get_token())
                        {
                        case token_t::TOKEN_MULTIPLY:
                            r = a * b;
                            break;

                        case token_t::TOKEN_DIVIDE:
                            r = a / b;
                            break;

                        case token_t::TOKEN_MODULO:
                            r = a % b;
                            break;

                        default: // LCOV_EXCL_LINE
                            throw logic_error("unsupported token in sub-switch (integer)."); // LCOV_EXCL_LINE

                        }

                        if(lhs->get_token() != token_t::TOKEN_INTEGER)
                        {
                            lhs = std::make_shared<node>(token_t::TOKEN_INTEGER, lhs->get_location());
                        }
                        lhs->set_integer(r);
                    }
                    else
                    {
                        long double const a(lhs->get_floating_point_auto_convert());
                        long double const b(rhs->get_floating_point_auto_convert());
                        long double r;
                        switch(multiplicative->get_token())
                        {
                        case token_t::TOKEN_MULTIPLY:
                            r = a * b;
                            break;

                        case token_t::TOKEN_DIVIDE:
                            r = a / b;
                            break;

                        case token_t::TOKEN_MODULO:
                            r = fmodl(a, b);
                            break;

                        default: // LCOV_EXCL_LINE
                            throw logic_error("unsupported token in sub-switch (floating point)."); // LCOV_EXCL_LINE

                        }
                        if(lhs->get_token() != token_t::TOKEN_FLOATING_POINT)
                        {
                            lhs = std::make_shared<node>(token_t::TOKEN_FLOATING_POINT, lhs->get_location());
                        }
                        lhs->set_floating_point(r);
                    }
                }
                else
                {
                    multiplicative->insert_child(-1, lhs);
                    multiplicative->insert_child(-1, rhs);
                    lhs = multiplicative;
                }
            }
            break;

        default:
            return lhs;

        }
    }
    snapdev::NOT_REACHED();
} // LCOV_EXCL_LINE


node::pointer_t expr_state::parse_expr_exponentiation()
{
    node::pointer_t lhs(parse_expr_unary());
    for(;;)
    {
        switch(f_node->get_token())
        {
        case token_t::TOKEN_POWER:
            {
                // Note: in as2js the exponentiation is right to left
                //       (like in math/Ada); but here it's left to right!
                //
                node::pointer_t power(f_node);
                f_node = f_lexer->get_next_token();
                node::pointer_t rhs(parse_expr_unary());

                if(lhs->is_literal(token_t::TOKEN_NUMBER)
                && rhs->is_literal(token_t::TOKEN_NUMBER))
                {
                    // do computation on the fly
                    //
                    if(lhs->is_literal(token_t::TOKEN_INTEGER)
                    && rhs->is_literal(token_t::TOKEN_INTEGER))
                    {
                        int512_t const a(lhs->get_integer_auto_convert());
                        int512_t const b(rhs->get_integer_auto_convert());

                        // TODO: add support for a int512_t.pow() function
                        //       (see snapdev::pow() for a way to implement)
                        //
                        //int512_t const r(snapdev::pow(a, b));
                        std::int64_t const r(snapdev::pow(a.f_value[0], b.f_value[0]));

                        if(lhs->get_token() != token_t::TOKEN_INTEGER)
                        {
                            lhs = std::make_shared<node>(token_t::TOKEN_INTEGER, lhs->get_location());
                        }
                        lhs->set_integer(r);
                    }
                    else
                    {
                        long double const a(lhs->get_floating_point_auto_convert());
                        long double const b(rhs->get_floating_point_auto_convert());
                        long double const r = powl(a, b);
                        if(lhs->get_token() != token_t::TOKEN_FLOATING_POINT)
                        {
                            lhs = std::make_shared<node>(token_t::TOKEN_FLOATING_POINT, lhs->get_location());
                        }
                        lhs->set_floating_point(r);
                    }
                }
                else
                {
                    power->insert_child(-1, lhs);
                    power->insert_child(-1, rhs);
                    lhs = power;
                }
            }
            break;

        default:
            return lhs;

        }
    }
    snapdev::NOT_REACHED();
} // LCOV_EXCL_LINE


node::pointer_t expr_state::parse_expr_unary()
{
    node::pointer_t negate;
    bool convert(false);
    for(;;)
    {
        switch(f_node->get_token())
        {
        case token_t::TOKEN_PLUS:
            convert = true;
            break;

        case token_t::TOKEN_MINUS:
            convert = true;
            if(negate == nullptr)
            {
                negate = f_node;
            }
            else
            {
                negate.reset();
            }
            break;

        default:
            if(convert)
            {
                node::pointer_t n(parse_expr_postfix());
                if(n->is_literal(token_t::TOKEN_INTEGER))
                {
                    if(n->get_token() == token_t::TOKEN_INTEGER)
                    {
                        n->set_integer(negate == nullptr
                                        ? n->get_integer()
                                        : -n->get_integer());
                        return n;
                    }
                    else
                    {
                        node::pointer_t result(std::make_shared<node>(token_t::TOKEN_INTEGER, n->get_location()));
                        result->set_integer(negate == nullptr
                                                ? n->get_integer_auto_convert()
                                                : -n->get_integer_auto_convert());
                        return result;
                    }
                    snapdev::NOT_REACHED();
                }
                if(n->is_literal(token_t::TOKEN_FLOATING_POINT))
                {
                    if(n->get_token() == token_t::TOKEN_FLOATING_POINT)
                    {
                        n->set_floating_point(negate == nullptr
                                                ? n->get_floating_point()
                                                : -n->get_floating_point());
                        return n;
                    }
                    else
                    {
                        node::pointer_t result(std::make_shared<node>(token_t::TOKEN_FLOATING_POINT, n->get_location()));
                        result->set_floating_point(negate == nullptr
                                                    ? n->get_floating_point_auto_convert()
                                                    : -n->get_floating_point_auto_convert());
                        return result;
                    }
                    snapdev::NOT_REACHED();
                }
                if(n->get_token() == token_t::TOKEN_STRING)
                {
                    // this is a string that cannot be converted to a number
                    // so this fails
                    //
                    snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                    msg << f_node->get_location().get_location()
                        << "string \""
                        << n->get_string()
                        << "\" cannot be converted to a number.";
                    throw invalid_token(msg.str());
                }
                if(negate == nullptr)
                {
                    // the input is not a literal, but the output is
                    // expected to be a number so do a cast
                    //
                    // TBD: we may want to use "x*1" so it remains an integer
                    // if x is an integer instead of converting to floating point
                    //
                    node::pointer_t result(std::make_shared<node>(token_t::TOKEN_CAST, n->get_location()));
                    result->set_integer(static_cast<std::int64_t>(type_t::TYPE_FLOAT8)); // at the moment, all floats are FLOAT8 in as2js
                    result->insert_child(-1, n);
                    return result;
                }
                else
                {
                    negate->insert_child(-1, n);
                    return negate;
                }
                snapdev::NOT_REACHED();
            }
            return parse_expr_postfix();

        }
        f_node = f_lexer->get_next_token();
    }
    snapdev::NOT_REACHED();
}


node::pointer_t expr_state::parse_expr_postfix()
{
    bool found_all_fields(false);
    node::pointer_t result(parse_expr_primary());
    for(;;)
    {
        if(result->get_token() == token_t::TOKEN_TYPE)
        {
SNAP_LOG_WARNING << "we found a type followed by a '(' ... Result before cast evaluation: " << to_string(result->get_token()) << SNAP_LOG_SEND;
            result = parse_expr_cast_value(static_cast<type_t>(result->get_integer().f_value[0]));
SNAP_LOG_WARNING << "we found a type followed by a '(' ... Result type: " << to_string(result->get_token()) << SNAP_LOG_SEND;
SNAP_LOG_WARNING << "and we should be able to convert that CAST -> {" << result->to_as2js() << '}' << SNAP_LOG_SEND;
        }
        else switch(f_node->get_token())
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
            {
                node::pointer_t t;
                //location const l(f_node->get_location());
SNAP_LOG_WARNING << "location of result: " << result->get_location().get_location() << "..." << SNAP_LOG_SEND;
//SNAP_LOG_WARNING << "location of :: operator: " << l.get_location() << "..." << SNAP_LOG_SEND;
                f_node = f_lexer->get_next_token(); // skip the '::'
                if(f_node->get_token() == token_t::TOKEN_IDENTIFIER)
                {
                    t = parse_expr_primary();
                }
                if(t == nullptr || t->get_token() != token_t::TOKEN_TYPE)
                {
                    snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                    msg << (t == nullptr ? f_node->get_location().get_location() : t->get_location().get_location())
                        << "a type name was expected after the '::' operator, not "
                        << to_string(t == nullptr ? f_node->get_token() : t->get_token())
                        << (t == nullptr || t->get_token() != token_t::TOKEN_IDENTIFIER
                                ? (f_node->get_token() == token_t::TOKEN_IDENTIFIER
                                    ? " \"" + f_node->get_string() + '"'
                                    : "")
                                : " \"" + t->get_string() + '"')
                        << ".";
                    throw invalid_token(msg.str());
                }
                else
                {
                    node::pointer_t n(std::make_shared<node>(token_t::TOKEN_CAST, t->get_location()));
                    n->set_integer(t->get_integer());
                    n->insert_child(-1, result);
                    result = n;
                }
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

        case token_t::TOKEN_OPEN_PARENTHESIS:
            // function call
            //
            if(result->get_token() != token_t::TOKEN_IDENTIFIER)
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                msg << f_node->get_location().get_location()
                    << "unexpected opening parenthesis ('(') after token "
                    << to_string(result->get_token())
                    << ".";
                throw invalid_token(msg.str());
            }
            else
            {
                location loc(f_node->get_location());
                std::string const l(loc.get_location());

                std::string keyword(result->get_string_upper());
                bool found(true);
                //f_node = f_lexer->get_next_token(); -- do NOT skip parenthesis, function_call() does it for us
                //                                       and parse_expr_cast_value() counts on it to know whether a ')' is required
                switch(keyword[0])
                {
                case 'A':
                    if(keyword == "ABS")
                    {
                        result = function_call(loc, function_t::FUNCTION_ABS, parse_expr_function_parameters(keyword, 1));
                    }
                    else if(keyword == "ACOS")
                    {
                        result = function_call(loc, function_t::FUNCTION_ACOS, parse_expr_function_parameters(keyword, 1));
                    }
                    else if(keyword == "ACOSH")
                    {
                        result = function_call(loc, function_t::FUNCTION_ACOSH, parse_expr_function_parameters(keyword, 1));
                    }
                    else if(keyword == "ASIN")
                    {
                        result = function_call(loc, function_t::FUNCTION_ASIN, parse_expr_function_parameters(keyword, 1));
                    }
                    else if(keyword == "ASINH")
                    {
                        result = function_call(loc, function_t::FUNCTION_ASINH, parse_expr_function_parameters(keyword, 1));
                    }
                    else if(keyword == "ATAN")
                    {
                        node::pointer_t params(parse_expr_function_parameters(keyword, -1));
                        if(params->get_children_size() == 1)
                        {
                            result = function_call(loc, function_t::FUNCTION_ATAN, params);
                        }
                        else if(params->get_children_size() == 2)
                        {
                            result = function_call(loc, function_t::FUNCTION_ATAN2, params);
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
                    }
                    else if(keyword == "ATANH")
                    {
                        result = function_call(loc, function_t::FUNCTION_ATANH, parse_expr_function_parameters(keyword, 1));
                    }
                    else
                    {
                        found = false;
                    }
                    break;

                case 'C':
                    if(keyword == "CBRT")
                    {
                        result = function_call(loc, function_t::FUNCTION_CBRT, parse_expr_function_parameters(keyword, 1));
                    }
                    else if(keyword == "CEIL")
                    {
                        result = function_call(loc, function_t::FUNCTION_CEIL, parse_expr_function_parameters(keyword, 1));
                    }
                    else if(keyword == "COS")
                    {
                        result = function_call(loc, function_t::FUNCTION_COS, parse_expr_function_parameters(keyword, 1));
                    }
                    else if(keyword == "COSH")
                    {
                        result = function_call(loc, function_t::FUNCTION_COSH, parse_expr_function_parameters(keyword, 1));
                    }
                    else
                    {
                        found = false;
                    }
                    break;

                case 'E':
                    if(keyword == "EXP")
                    {
                        result = function_call(loc, function_t::FUNCTION_EXP, parse_expr_function_parameters(keyword, 1));
                    }
                    else if(keyword == "EXPM1")
                    {
                        result = function_call(loc, function_t::FUNCTION_EXPM1, parse_expr_function_parameters(keyword, 1));
                    }
                    else
                    {
                        found = false;
                    }
                    break;

                case 'F':
                    if(keyword == "FLOOR")
                    {
                        result = function_call(loc, function_t::FUNCTION_FLOOR, parse_expr_function_parameters(keyword, 1));
                    }
                    else
                    {
                        found = false;
                    }
                    break;

                case 'H':
                    if(keyword == "HYPOT")
                    {
                        // with 0 parameters, the function returns 0
                        // with 1 parameter, the function returns that parameter as is
                        // with 2 or more, it returns the square root of the sum of the squares
                        //
                        result = function_call(loc, function_t::FUNCTION_HYPOT, parse_expr_function_parameters(keyword, -1));
                    }
                    else
                    {
                        found = false;
                    }
                    break;

                case 'I':
                    if(keyword == "IMUL")
                    {
                        result = function_call(loc, function_t::FUNCTION_IMUL, parse_expr_function_parameters(keyword, 2));
                    }
                    else
                    {
                        found = false;
                    }
                    break;

                case 'L':
                    if(keyword == "LENGTH")
                    {
                        result = function_call(loc, function_t::FUNCTION_LENGTH, parse_expr_function_parameters(keyword, 1));
                    }
                    else if(keyword == "LOG")
                    {
                        result = function_call(loc, function_t::FUNCTION_LOG, parse_expr_function_parameters(keyword, 1));
                    }
                    else if(keyword == "LOG1P")
                    {
                        result = function_call(loc, function_t::FUNCTION_LOG1P, parse_expr_function_parameters(keyword, 1));
                    }
                    else if(keyword == "LOG10")
                    {
                        result = function_call(loc, function_t::FUNCTION_LOG10, parse_expr_function_parameters(keyword, 1));
                    }
                    else if(keyword == "LOG2")
                    {
                        result = function_call(loc, function_t::FUNCTION_LOG2, parse_expr_function_parameters(keyword, 1));
                    }
                    else
                    {
                        found = false;
                    }
                    break;

                case 'M':
                    if(keyword == "MAX")
                    {
                        result = function_call(loc, function_t::FUNCTION_MAX, parse_expr_function_parameters(keyword, -1));
                    }
                    else if(keyword == "MIN")
                    {
                        result = function_call(loc, function_t::FUNCTION_MIN, parse_expr_function_parameters(keyword, -1));
                    }
                    else
                    {
                        found = false;
                    }
                    break;

                case 'P':
                    if(keyword == "POW")
                    {
                        result = function_call(loc, function_t::FUNCTION_POW, parse_expr_function_parameters(keyword, 2));
                    }
                    else
                    {
                        found = false;
                    }
                    break;

                case 'R':
                    if(keyword == "RAND")
                    {
                        result = function_call(loc, function_t::FUNCTION_RAND, parse_expr_function_parameters(keyword, 0));
                    }
                    else if(keyword == "ROUND")
                    {
                        result = function_call(loc, function_t::FUNCTION_ROUND, parse_expr_function_parameters(keyword, 1));
                    }
                    else
                    {
                        found = false;
                    }
                    break;

                case 'S':
                    if(keyword == "SIGN")
                    {
                        result = function_call(loc, function_t::FUNCTION_SIGN, parse_expr_function_parameters(keyword, 1));
                    }
                    else if(keyword == "SIN")
                    {
                        result = function_call(loc, function_t::FUNCTION_SIN, parse_expr_function_parameters(keyword, 1));
                    }
                    else if(keyword == "SINH")
                    {
                        result = function_call(loc, function_t::FUNCTION_SINH, parse_expr_function_parameters(keyword, 1));
                    }
                    else if(keyword == "SQRT")
                    {
                        result = function_call(loc, function_t::FUNCTION_SQRT, parse_expr_function_parameters(keyword, 1));
                    }
                    else
                    {
                        found = false;
                    }
                    break;

                case 'T':
                    if(keyword == "TAN")
                    {
                        result = function_call(loc, function_t::FUNCTION_TAN, parse_expr_function_parameters(keyword, 1));
                    }
                    else if(keyword == "TANH")
                    {
                        result = function_call(loc, function_t::FUNCTION_TANH, parse_expr_function_parameters(keyword, 1));
                    }
                    else if(keyword == "TRUNC")
                    {
                        result = function_call(loc, function_t::FUNCTION_TRUNC, parse_expr_function_parameters(keyword, 1));
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
                    // TBD: at some point we may want to support any function
                    //      call or user defined type (i.e. for other as2js
                    //      functions and user defined functions)
                    //
                    snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                    msg << f_node->get_location().get_location()
                        << "unknown function "
                        << keyword
                        << "().";
                    throw type_not_found(msg.str());
                }
            }
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
            location const l(f_node->get_location());
            auto create_type_node = [this, &l](type_t type)
            {
                node::pointer_t t(std::make_shared<node>(token_t::TOKEN_TYPE, l));
                t->set_integer(static_cast<std::int64_t>(type));
                f_node = f_lexer->get_next_token();
                return t;
            }; // LCOV_EXCL_LINE

            std::string keyword(f_node->get_string_lower());
            switch(keyword[0])
            {
            case 'b':
                if(keyword == "bigint")
                {
                    return create_type_node(type_t::TYPE_INT8);
                }
                if(keyword == "boolean")
                {
                    return create_type_node(type_t::TYPE_BOOLEAN);
                }
                break;

            case 'c':
                if(keyword == "char")
                {
                    return create_type_node(type_t::TYPE_TEXT);
                }
                break;

            case 'd':
                if(keyword == "double")
                {
                    f_node = f_lexer->get_next_token();
                    if(f_node->get_token() != token_t::TOKEN_IDENTIFIER
                    || f_node->get_string_upper() != "PRECISION")
                    {
                        snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                        msg << l.get_location()
                            << "expected DOUBLE to be followed by the word PRECISION.";
                        throw invalid_token(msg.str());
                    }
                    return create_type_node(type_t::TYPE_FLOAT8);
                }
                break;

            case 'i':
                if(keyword == "int1")
                {
                    return create_type_node(type_t::TYPE_INT4);
                }
                if(keyword == "int2")
                {
                    return create_type_node(type_t::TYPE_INT2);
                }
                if(keyword == "int"
                || keyword == "int4"
                || keyword == "integer")
                {
                    return create_type_node(type_t::TYPE_INT4);
                }
                if(keyword == "int8")
                {
                    return create_type_node(type_t::TYPE_INT8);
                }
                if(keyword == "int16")
                {
                    return create_type_node(type_t::TYPE_INT16);
                }
                if(keyword == "int32")
                {
                    return create_type_node(type_t::TYPE_INT32);
                }
                if(keyword == "int64")
                {
                    return create_type_node(type_t::TYPE_INT64);
                }
                break;

            case 'f':
                if(keyword == "false")
                {
                    result = std::make_shared<node>(token_t::TOKEN_FALSE, f_node->get_location());
                    f_node = f_lexer->get_next_token();
                    return result;
                }
                if(keyword == "float4")
                {
                    return create_type_node(type_t::TYPE_FLOAT4);
                }
                if(keyword == "float8")
                {
                    return create_type_node(type_t::TYPE_FLOAT8);
                }
                if(keyword == "float10")
                {
                    return create_type_node(type_t::TYPE_FLOAT10);
                }
                break;

            case 'n':
                if(keyword == "null")
                {
                    result = std::make_shared<node>(token_t::TOKEN_NULL, f_node->get_location());
                    f_node = f_lexer->get_next_token();
                    return result;
                }
                break;

            case 'r':
                if(keyword == "real")
                {
                    return create_type_node(type_t::TYPE_FLOAT4);
                }
                break;

            case 's':
                if(keyword == "smallint")
                {
                    return create_type_node(type_t::TYPE_INT2);
                }
                break;

            case 't':
                if(keyword == "text")
                {
                    return create_type_node(type_t::TYPE_TEXT);
                }
                if(keyword == "true")
                {
                    result = std::make_shared<node>(token_t::TOKEN_TRUE, f_node->get_location());
                    f_node = f_lexer->get_next_token();
                    return result;
                }
                break;

            case 'u':
                if(keyword == "unsigned")
                {
                    f_node = f_lexer->get_next_token();
                    if(f_node->get_token() != token_t::TOKEN_IDENTIFIER)
                    {
                        snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                        msg << f_node->get_location().get_location()
                            << "expected an integer name to follow the UNSIGNED word (not a "
                            << to_string(f_node->get_token())
                            << ").";
                        throw invalid_token(msg.str());
                    }
                    keyword = f_node->get_string_upper();
                    if(keyword == "INT1")
                    {
                        return create_type_node(type_t::TYPE_UNSIGNED_INT1);
                    }
                    if(keyword == "SMALLINT"
                    || keyword == "INT2")
                    {
                        return create_type_node(type_t::TYPE_UNSIGNED_INT2);
                    }
                    if(keyword == "INT"
                    || keyword == "INTEGER"
                    || keyword == "INT4")
                    {
                        return create_type_node(type_t::TYPE_UNSIGNED_INT8);
                    }
                    if(keyword == "BIGINT"
                    || keyword == "INT8")
                    {
                        return create_type_node(type_t::TYPE_UNSIGNED_INT8);
                    }
                    if(keyword == "INT16")
                    {
                        return create_type_node(type_t::TYPE_UNSIGNED_INT16);
                    }
                    if(keyword == "INT32")
                    {
                        return create_type_node(type_t::TYPE_UNSIGNED_INT32);
                    }
                    if(keyword == "INT64")
                    {
                        return create_type_node(type_t::TYPE_UNSIGNED_INT64);
                    }

                    snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                    msg << f_node->get_location().get_location()
                        << "expected an integer name to follow the UNSIGNED word (not '"
                        << keyword
                        << "').";
                    throw invalid_token(msg.str());
                }
                break;

            }
            result = f_node;
            result->set_string(keyword); // save the lowercase version
SNAP_LOG_WARNING << "returning identifier '" << keyword << "'" << SNAP_LOG_SEND;
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


node::pointer_t expr_state::function_call(location const & l, function_t func, node::pointer_t params)
{
    // create result node
    //
    node::pointer_t result(std::make_shared<node>(token_t::TOKEN_FUNCTION_CALL, l));

    // make sure parameters are in a list
    //
    if(params->get_token() != token_t::TOKEN_LIST)
    {
        node::pointer_t list(std::make_shared<node>(token_t::TOKEN_LIST, params->get_location()));
        list->insert_child(-1, params);
        params = list;
    }

    // check whether our parameters are all literals
    //
    auto all_literals = [](node::pointer_t list, token_t match_type)
    {
        std::size_t const size(list->get_children_size());
        for(std::size_t idx(0); idx < size; ++idx)
        {
            node::pointer_t p(list->get_child(idx));
            if(!p->is_literal(match_type))
            {
                return false;
            }
        }
        return true;
    };

    node::pointer_t n;
    switch(func)
    {
    case function_t::FUNCTION_ABS:
        n = params->get_child(0);
        if(n->is_literal(token_t::TOKEN_INTEGER))
        {
            result = std::make_shared<node>(token_t::TOKEN_INTEGER, n->get_location());
            result->set_integer(n->get_integer_auto_convert().abs());
            return result;
        }
        if(n->is_literal(token_t::TOKEN_FLOATING_POINT))
        {
            result = std::make_shared<node>(token_t::TOKEN_FLOATING_POINT, n->get_location());
            result->set_floating_point(fabsl(n->get_floating_point_auto_convert()));
            return result;
        }
        if(n->get_token() == token_t::TOKEN_MINUS
        && n->get_children_size() == 1) // 1 -- it's a negate, 2 -- it's a subtraction, which we cannot optimize here
        {
            // abs(-n) => abs(n)
            //
            params->set_child(0, n->get_child(0));
        }
        result->set_string("Math.abs");
        break;

    case function_t::FUNCTION_ACOS:
        n = params->get_child(0);
        if(n->is_literal(token_t::TOKEN_NUMBER))
        {
            result = std::make_shared<node>(token_t::TOKEN_FLOATING_POINT, n->get_location());
            result->set_floating_point(acos(n->get_floating_point_auto_convert()));
            return result;
        }
        result->set_string("Math.acos");
        break;

    case function_t::FUNCTION_ACOSH:
        n = params->get_child(0);
        if(n->is_literal(token_t::TOKEN_NUMBER))
        {
            result = std::make_shared<node>(token_t::TOKEN_FLOATING_POINT, n->get_location());
            result->set_floating_point(acosh(n->get_floating_point_auto_convert()));
            return result;
        }
        result->set_string("Math.acosh");
        break;

    case function_t::FUNCTION_ASIN:
        n = params->get_child(0);
        if(n->is_literal(token_t::TOKEN_NUMBER))
        {
            result = std::make_shared<node>(token_t::TOKEN_FLOATING_POINT, n->get_location());
            result->set_floating_point(asin(n->get_floating_point_auto_convert()));
            return result;
        }
        result->set_string("Math.asin");
        break;

    case function_t::FUNCTION_ASINH:
        n = params->get_child(0);
        if(n->is_literal(token_t::TOKEN_NUMBER))
        {
            result = std::make_shared<node>(token_t::TOKEN_FLOATING_POINT, n->get_location());
            result->set_floating_point(asinh(n->get_floating_point_auto_convert()));
            return result;
        }
        result->set_string("Math.asinh");
        break;

    case function_t::FUNCTION_ATAN:
        n = params->get_child(0);
        if(n->is_literal(token_t::TOKEN_NUMBER))
        {
            result = std::make_shared<node>(token_t::TOKEN_FLOATING_POINT, n->get_location());
            result->set_floating_point(atan(n->get_floating_point_auto_convert()));
            return result;
        }
        result->set_string("Math.atan");
        break;

    case function_t::FUNCTION_ATAN2:
        n = params->get_child(0);
        if(n->is_literal(token_t::TOKEN_NUMBER))
        {
            node::pointer_t m(params->get_child(1));
            if(m->is_literal(token_t::TOKEN_NUMBER))
            {
                result = std::make_shared<node>(token_t::TOKEN_FLOATING_POINT, n->get_location());
                result->set_floating_point(atan2(n->get_floating_point_auto_convert(),
                                                 m->get_floating_point_auto_convert()));
                return result;
            }
        }
        result->set_string("Math.atan2");
        break;

    case function_t::FUNCTION_ATANH:
        n = params->get_child(0);
        if(n->is_literal(token_t::TOKEN_NUMBER))
        {
            result = std::make_shared<node>(token_t::TOKEN_FLOATING_POINT, n->get_location());
            result->set_floating_point(atanh(n->get_floating_point_auto_convert()));
            return result;
        }
        result->set_string("Math.atanh");
        break;

    case function_t::FUNCTION_CBRT:
        n = params->get_child(0);
        if(n->is_literal(token_t::TOKEN_NUMBER))
        {
            result = std::make_shared<node>(token_t::TOKEN_FLOATING_POINT, n->get_location());
            result->set_floating_point(cbrtl(n->get_floating_point_auto_convert()));
            return result;
        }
        result->set_string("Math.cbrt");
        break;

    case function_t::FUNCTION_CEIL:
        n = params->get_child(0);
        if(n->is_literal(token_t::TOKEN_INTEGER))
        {
            result = std::make_shared<node>(token_t::TOKEN_INTEGER, n->get_location());
            result->set_integer( n->get_integer_auto_convert());
            return result;
        }
        if(n->is_literal(token_t::TOKEN_FLOATING_POINT))
        {
            result = std::make_shared<node>(token_t::TOKEN_FLOATING_POINT, n->get_location());
            long double a(ceill(n->get_floating_point_auto_convert()));
            result->set_floating_point(a);
            return result;
        }
        result->set_string("Math.ceil");
        break;

    case function_t::FUNCTION_CONCAT:
        result->set_string("String.concat");
        break;

    case function_t::FUNCTION_COS:
        n = params->get_child(0);
        if(n->is_literal(token_t::TOKEN_NUMBER))
        {
            result = std::make_shared<node>(token_t::TOKEN_FLOATING_POINT, n->get_location());
            result->set_floating_point(cos(n->get_floating_point_auto_convert()));
            return result;
        }
        result->set_string("Math.cos");
        break;

    case function_t::FUNCTION_COSH:
        n = params->get_child(0);
        if(n->is_literal(token_t::TOKEN_NUMBER))
        {
            result = std::make_shared<node>(token_t::TOKEN_FLOATING_POINT, n->get_location());
            result->set_floating_point(cosh(n->get_floating_point_auto_convert()));
            return result;
        }
        result->set_string("Math.cosh");
        break;

    case function_t::FUNCTION_EXP:
        n = params->get_child(0);
        if(n->is_literal(token_t::TOKEN_NUMBER))
        {
            result = std::make_shared<node>(token_t::TOKEN_FLOATING_POINT, n->get_location());
            result->set_floating_point(exp(n->get_floating_point_auto_convert()));
            return result;
        }
        result->set_string("Math.exp");
        break;

    case function_t::FUNCTION_EXPM1:
        n = params->get_child(0);
        if(n->is_literal(token_t::TOKEN_NUMBER))
        {
            result = std::make_shared<node>(token_t::TOKEN_FLOATING_POINT, n->get_location());
            result->set_floating_point(expm1(n->get_floating_point_auto_convert()));
            return result;
        }
        result->set_string("Math.expm1");
        break;

    case function_t::FUNCTION_FLOOR:
        n = params->get_child(0);
        if(n->is_literal(token_t::TOKEN_INTEGER))
        {
            result = std::make_shared<node>(token_t::TOKEN_INTEGER, n->get_location());
            result->set_integer(n->get_integer_auto_convert());
            return result;
        }
        if(n->is_literal(token_t::TOKEN_FLOATING_POINT))
        {
            result = std::make_shared<node>(token_t::TOKEN_FLOATING_POINT, n->get_location());
            long double a(floorl(n->get_floating_point_auto_convert()));
            result->set_floating_point(a);
            return result;
        }
        result->set_string("Math.floor");
        break;

    case function_t::FUNCTION_HYPOT:
        if(params->get_children_size() == 0)
        {
            // a floating point node is 0.0 by default, so we can directly return it
            //
            return std::make_shared<node>(token_t::TOKEN_FLOATING_POINT, params->get_location());
        }
        else if(all_literals(params, token_t::TOKEN_NUMBER))
        {
            n = params->get_child(0);
            result = std::make_shared<node>(token_t::TOKEN_FLOATING_POINT, n->get_location());
            long double sum(0.0);
            std::size_t const size(params->get_children_size());
            for(std::size_t idx(0); idx < size; ++idx)
            {
                node::pointer_t p(params->get_child(idx));
                long double value(p->get_floating_point_auto_convert());
                sum += value * value;
            }
            result->set_floating_point(sqrtl(sum));
            return result;
        }
        if(params->get_children_size() == 1)
        {
            // this is much more efficient (|/ a ^ 2 = @ a)
            // (however, abs() of an integer will return an integer in as2js...)
            //
            result->set_string("Math.abs");
        }
        else
        {
            result->set_string("Math.hypot");
        }
        break;

    case function_t::FUNCTION_IMUL:
        n = params->get_child(0);
        if(n->is_literal(token_t::TOKEN_NUMBER))
        {
            node::pointer_t m(params->get_child(1));
            if(m->is_literal(token_t::TOKEN_NUMBER))
            {
                result = std::make_shared<node>(token_t::TOKEN_INTEGER, n->get_location());
                result->set_integer(n->get_integer_auto_convert() * m->get_integer_auto_convert());
                return result;
            }
        }
        result->set_string("Math.imul");
        break;

    case function_t::FUNCTION_LENGTH:
        n = params->get_child(0);
        if(n->is_literal())
        {
            result = std::make_shared<node>(token_t::TOKEN_INTEGER, n->get_location());
            result->set_integer(n->get_string_auto_convert().length());
            return result;
        }
        // in JavaScript, LENGTH is actually a field of a string
        //
        result = std::make_shared<node>(token_t::TOKEN_PERIOD, n->get_location());
        result->insert_child(-1, n);
        n = std::make_shared<node>(token_t::TOKEN_IDENTIFIER, n->get_location());
        n->set_string("length");
        result->insert_child(-1, n);
        break;

    case function_t::FUNCTION_LOG:
        n = params->get_child(0);
        if(n->is_literal(token_t::TOKEN_NUMBER))
        {
            result = std::make_shared<node>(token_t::TOKEN_FLOATING_POINT, n->get_location());
            result->set_floating_point(logl(n->get_floating_point_auto_convert()));
            return result;
        }
        result->set_string("Math.log");
        break;

    case function_t::FUNCTION_LOG1P:
        n = params->get_child(0);
        if(n->is_literal(token_t::TOKEN_NUMBER))
        {
            result = std::make_shared<node>(token_t::TOKEN_FLOATING_POINT, n->get_location());
            result->set_floating_point(log1pl(n->get_floating_point_auto_convert()));
            return result;
        }
        result->set_string("Math.log1p");
        break;

    case function_t::FUNCTION_LOG10:
        n = params->get_child(0);
        if(n->is_literal(token_t::TOKEN_NUMBER))
        {
            result = std::make_shared<node>(token_t::TOKEN_FLOATING_POINT, n->get_location());
            result->set_floating_point(log10l(n->get_floating_point_auto_convert()));
            return result;
        }
        result->set_string("Math.log10");
        break;

    case function_t::FUNCTION_LOG2:
        n = params->get_child(0);
        if(n->is_literal(token_t::TOKEN_NUMBER))
        {
            result = std::make_shared<node>(token_t::TOKEN_FLOATING_POINT, n->get_location());
            result->set_floating_point(log2l(n->get_floating_point_auto_convert()));
            return result;
        }
        result->set_string("Math.log2");
        break;

    case function_t::FUNCTION_MAX:
        if(params->get_children_size() == 0)
        {
            result = std::make_shared<node>(token_t::TOKEN_FLOATING_POINT, params->get_location());
            result->set_floating_point(-std::numeric_limits<long double>::infinity());
            return result;
        }
        else if(all_literals(params, token_t::TOKEN_NUMBER))
        {
            n = params->get_child(0);
            if(all_literals(params, token_t::TOKEN_INTEGER))
            {
                result = std::make_shared<node>(token_t::TOKEN_INTEGER, n->get_location());
                int512_t max;
                max.min();
                std::size_t const size(params->get_children_size());
                for(std::size_t idx(0); idx < size; ++idx)
                {
                    node::pointer_t p(params->get_child(idx));
                    int512_t const value(p->get_integer_auto_convert());
                    if(value > max)
                    {
                        max = value;
                    }
                }
                result->set_integer(max);
            }
            else
            {
                result = std::make_shared<node>(token_t::TOKEN_FLOATING_POINT, n->get_location());
                long double max(-std::numeric_limits<long double>::infinity());
                std::size_t const size(params->get_children_size());
                for(std::size_t idx(0); idx < size; ++idx)
                {
                    node::pointer_t p(params->get_child(idx));
                    long double value(p->get_floating_point_auto_convert());
                    if(value > max)
                    {
                        max = value;
                    }
                }
                result->set_floating_point(max);
            }
            return result;
        }
        result->set_string("Math.max");
        break;

    case function_t::FUNCTION_MIN:
        if(params->get_children_size() == 0)
        {
            result = std::make_shared<node>(token_t::TOKEN_FLOATING_POINT, params->get_location());
            result->set_floating_point(std::numeric_limits<long double>::infinity());
            return result;
        }
        else if(all_literals(params, token_t::TOKEN_NUMBER))
        {
            n = params->get_child(0);
            if(all_literals(params, token_t::TOKEN_INTEGER))
            {
                result = std::make_shared<node>(token_t::TOKEN_INTEGER, n->get_location());
                int512_t min;
                min.max();
                std::size_t const size(params->get_children_size());
                for(std::size_t idx(0); idx < size; ++idx)
                {
                    node::pointer_t p(params->get_child(idx));
                    int512_t const value(p->get_integer_auto_convert());
                    if(value < min)
                    {
                        min = value;
                    }
                }
                result->set_integer(min);
            }
            else
            {
                result = std::make_shared<node>(token_t::TOKEN_FLOATING_POINT, n->get_location());
                long double min(std::numeric_limits<long double>::infinity());
                std::size_t const size(params->get_children_size());
                for(std::size_t idx(0); idx < size; ++idx)
                {
                    node::pointer_t p(params->get_child(idx));
                    long double value(p->get_floating_point_auto_convert());
                    if(value < min)
                    {
                        min = value;
                    }
                }
                result->set_floating_point(min);
            }
            return result;
        }
        result->set_string("Math.min");
        break;

    case function_t::FUNCTION_POW:
        {
            n = params->get_child(0);
            node::pointer_t m(params->get_child(1));

            if(n->is_literal(token_t::TOKEN_NUMBER))
            {
                if(m->is_literal(token_t::TOKEN_NUMBER))
                {
                    if(n->is_literal(token_t::TOKEN_INTEGER)
                    && m->is_literal(token_t::TOKEN_INTEGER))
                    {
                        result = std::make_shared<node>(token_t::TOKEN_INTEGER, n->get_location());
                        int512_t const a(n->get_integer_auto_convert());
                        int512_t const b(m->get_integer_auto_convert());
                        // TODO: replace with int512_t::pow() once available
                        std::int64_t const r(snapdev::pow(a.f_value[0], b.f_value[0]));
                        result->set_integer(r);
                    }
                    else
                    {
                        result = std::make_shared<node>(token_t::TOKEN_FLOATING_POINT, n->get_location());
                        result->set_floating_point(powl(n->get_floating_point_auto_convert(),
                                                        m->get_floating_point_auto_convert()));
                    }
                    return result;
                }
            }

            // use the as2js '**' operator instead of the Math.pow() function
            //
            result = std::make_shared<node>(token_t::TOKEN_POWER, n->get_location());
            result->insert_child(-1, n);
            result->insert_child(-1, m);
            return result;
        }
        break;

    case function_t::FUNCTION_RAND:
        result->set_string("Math.rand");
        break;

    case function_t::FUNCTION_ROUND:
        n = params->get_child(0);
        if(n->is_literal(token_t::TOKEN_INTEGER))
        {
            result = std::make_shared<node>(token_t::TOKEN_INTEGER, n->get_location());
            result->set_integer(n->get_integer_auto_convert());
            return result;
        }
        if(n->is_literal(token_t::TOKEN_FLOATING_POINT))
        {
            result = std::make_shared<node>(token_t::TOKEN_INTEGER, n->get_location());
            result->set_integer(roundl(n->get_floating_point_auto_convert()));
            return result;
        }
        result->set_string("Math.round");
        break;

    case function_t::FUNCTION_SIGN:
        n = params->get_child(0);
        if(n->is_literal(token_t::TOKEN_INTEGER))
        {
            result = std::make_shared<node>(token_t::TOKEN_INTEGER, n->get_location());
            int512_t number(n->get_integer_auto_convert());
            if(!number.is_zero())
            {
                if(number.is_negative())
                {
                    number = -1;
                }
                else
                {
                    number = 1;
                }
            }
            result->set_integer(number);
            return result;
        }
        if(n->is_literal(token_t::TOKEN_FLOATING_POINT))
        {
            result = std::make_shared<node>(token_t::TOKEN_INTEGER, n->get_location());
            long double a(floorl(n->get_floating_point_auto_convert()));
            std::int64_t number(0);
            if(a < 0.0)
            {
                number = -1;
            }
            else if(a > 0.0)
            {
                number = 1;
            }
            result->set_integer(number);
            return result;
        }
        result->set_string("Math.sign");
        break;

    case function_t::FUNCTION_SIN:
        n = params->get_child(0);
        if(n->is_literal(token_t::TOKEN_NUMBER))
        {
            result = std::make_shared<node>(token_t::TOKEN_FLOATING_POINT, n->get_location());
            result->set_floating_point(sinl(n->get_floating_point_auto_convert()));
            return result;
        }
        result->set_string("Math.sin");
        break;

    case function_t::FUNCTION_SINH:
        n = params->get_child(0);
        if(n->is_literal(token_t::TOKEN_NUMBER))
        {
            result = std::make_shared<node>(token_t::TOKEN_FLOATING_POINT, n->get_location());
            result->set_floating_point(sinhl(n->get_floating_point_auto_convert()));
            return result;
        }
        result->set_string("Math.sinh");
        break;

    case function_t::FUNCTION_SQRT:
        n = params->get_child(0);
        if(n->is_literal(token_t::TOKEN_NUMBER))
        {
            result = std::make_shared<node>(token_t::TOKEN_FLOATING_POINT, n->get_location());
            result->set_floating_point(sqrtl(n->get_floating_point_auto_convert()));
            return result;
        }
        result->set_string("Math.sqrt");
        break;

    case function_t::FUNCTION_TAN:
        n = params->get_child(0);
        if(n->is_literal(token_t::TOKEN_NUMBER))
        {
            result = std::make_shared<node>(token_t::TOKEN_FLOATING_POINT, n->get_location());
            result->set_floating_point(tanl(n->get_floating_point_auto_convert()));
            return result;
        }
        result->set_string("Math.tan");
        break;

    case function_t::FUNCTION_TANH:
        n = params->get_child(0);
        if(n->is_literal(token_t::TOKEN_NUMBER))
        {
            result = std::make_shared<node>(token_t::TOKEN_FLOATING_POINT, n->get_location());
            result->set_floating_point(tanhl(n->get_floating_point_auto_convert()));
            return result;
        }
        result->set_string("Math.tanh");
        break;

    case function_t::FUNCTION_TRUNC:
        n = params->get_child(0);
        if(n->is_literal(token_t::TOKEN_INTEGER))
        {
            result = std::make_shared<node>(token_t::TOKEN_INTEGER, n->get_location());
            result->set_integer(n->get_integer_auto_convert());
            return result;
        }
        if(n->is_literal(token_t::TOKEN_FLOATING_POINT))
        {
            result = std::make_shared<node>(token_t::TOKEN_FLOATING_POINT, n->get_location());
            result->set_floating_point(truncl(n->get_floating_point_auto_convert()));
            return result;
        }
        result->set_string("Math.trunc");
        break;

    }

    // there was no optimization, save those parameters and return
    // the full function call
    //
    result->insert_child(-1, params);
    return result;
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
SNAP_LOG_WARNING << "--------------------------------------------------------- tree: BEGIN" << SNAP_LOG_SEND;
std::cerr << '\n' << tree->to_tree() << std::endl;
SNAP_LOG_WARNING << "--------------------------------------------------------- tree: END" << SNAP_LOG_SEND;
    std::string const result(tree->to_as2js());
    n = s.f_node;
    return result;
}



} // namespace pbql
} // namespace prinbee
// vim: ts=4 sw=4 et
