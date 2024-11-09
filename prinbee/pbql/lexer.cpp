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
 * The Pribee Query Language (PBQL) is an SQL-like language. This file
 * transforms the input data in tokens that the parser can then use to
 * create statements.
 *
 * The lexer supports tokens that include keywords (SELECT), identifiers
 * (column name), numbers (integers, floating points), operators (for
 * expressions; +, -, *, /, etc.).
 */

// self
//
#include    "prinbee/pbql/lexer.h"

#include    "prinbee/exception.h"


// libutf8
//
#include    <libutf8/libutf8.h>


// snaplogger
//
#include    <snaplogger/message.h>


// snapdev
//
#include    <snapdev/hexadecimal_string.h>
#include    <snapdev/not_reached.h>


// last include
//
#include    <snapdev/poison.h>



namespace prinbee
{
namespace pbql
{



void lexer::set_input(input::pointer_t & in)
{
    f_input = in;
}


node::pointer_t lexer::get_next_token()
{
    if(f_input == nullptr)
    {
        snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
        msg << "input missing.";
        throw logic_error(msg.str());
    }

    for(;;)
    {
        location const l(f_input->get_location());
        char32_t c(f_input->getc());
        switch(c)
        {
        case libutf8::EOS:
            {
                node::pointer_t n(std::make_shared<node>(token_t::TOKEN_EOF, l));
                return n;
            }

        case ' ':
        case '\n':
        case '\t':
        case '\f':
        case '\v':
            // skip white space silently
            break;

        case '%':
        case '&':
        case '(':
        case ')':
        case '*':
        case '+':
        case ',':
        case '.':
        case '/':
        case ';':
        case '@':
        case '^':
        case '=':
        case '~':
            {
                node::pointer_t n(std::make_shared<node>(static_cast<token_t>(c), l));
                return n;
            }

        case '#':
            if(l.get_line() == 1
            && l.get_column() == 1)
            {
                // if the '#' start the first line, view it as a comment
                //
                // in this case, we allow further lines to also use the '#'
                // to start a comment
                //
                for(;;)
                {
                    for(;;)
                    {
                        c = f_input->getc();
                        if(c == '\n' || c == libutf8::EOS)
                        {
                            break;
                        }
                    }
                    c = f_input->getc();
                    if(c != '#')
                    {
                        f_input->ungetc(c);
                        break;
                    }
                }
            }
            else
            {
                // this is an operator
                //
                node::pointer_t n(std::make_shared<node>(static_cast<token_t>(c), l));
                return n;
            }
            break;

        case '-':
            c = f_input->getc();
            if(c == '-')
            {
                // this is a comment, skip everything up to the next newline
                //
                for(;;)
                {
                    c = f_input->getc();
                    if(c == '\n' || c == libutf8::EOS)
                    {
                        break;
                    }
                }
            }
            else
            {
                f_input->ungetc(c);
                node::pointer_t n(std::make_shared<node>(token_t::TOKEN_MINUS, l));
                return n;
            }
            break;

        case '<':
            c = f_input->getc();
            if(c == '=')
            {
                node::pointer_t n(std::make_shared<node>(token_t::TOKEN_LESS_EQUAL, l));
                return n;
            }
            else if(c == '<')
            {
                node::pointer_t n(std::make_shared<node>(token_t::TOKEN_SHIFT_LEFT, l));
                return n;
            }
            else if(c == '>')
            {
                node::pointer_t n(std::make_shared<node>(token_t::TOKEN_NOT_EQUAL, l));
                return n;
            }
            else
            {
                f_input->ungetc(c);
                node::pointer_t n(std::make_shared<node>(token_t::TOKEN_LESS, l));
                return n;
            }
            break;

        case '>':
            c = f_input->getc();
            if(c == '=')
            {
                node::pointer_t n(std::make_shared<node>(token_t::TOKEN_GREATER_EQUAL, l));
                return n;
            }
            else if(c == '>')
            {
                node::pointer_t n(std::make_shared<node>(token_t::TOKEN_SHIFT_RIGHT, l));
                return n;
            }
            else
            {
                f_input->ungetc(c);
                node::pointer_t n(std::make_shared<node>(token_t::TOKEN_GREATER, l));
                return n;
            }
            break;

        case '|':
            c = f_input->getc();
            if(c == '/')
            {
                node::pointer_t n(std::make_shared<node>(token_t::TOKEN_SQUARE_ROOT, l));
                return n;
            }
            else if(c == '|')
            {
                c = f_input->getc();
                if(c == '/')
                {
                    node::pointer_t n(std::make_shared<node>(token_t::TOKEN_CUBE_ROOT, l));
                    return n;
                }
                else
                {
                    f_input->ungetc(c);
                    node::pointer_t n(std::make_shared<node>(token_t::TOKEN_STRING_CONCAT, l));
                    return n;
                }
            }
            else
            {
                f_input->ungetc(c);
                node::pointer_t n(std::make_shared<node>(token_t::TOKEN_BITWISE_OR, l));
                return n;
            }
            break;

        case '\'':
            {
                std::string s;
                for(;;)
                {
                    c = f_input->getc();
                    if(c == '\'')
                    {
                        break;
                    }
                    if(c == libutf8::EOS)
                    {
                        snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                        msg << "unclosed string.";
                        throw invalid_token(msg.str());
                    }
                    if(c == '\n')
                    {
                        snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                        msg << "string cannot include a newline or carriage return character.";
                        throw invalid_token(msg.str());
                    }
                    s += c;
                }

                node::pointer_t n(std::make_shared<node>(token_t::TOKEN_STRING, l));
                n->set_string(s);
                return n;
            }
            break;

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            {
                uint512_t value(0);
                if(c == '0')
                {
                    c = f_input->getc();
                    if(c == 'x' || c == 'X')
                    {
                        // hexadecimal number
                        //
                        bool found(false);
                        for(;;)
                        {
                            c = f_input->getc();
                            if(!snapdev::is_hexdigit(c))
                            {
                                f_input->ungetc(c);
                                break;
                            }
                            found = true;
                            value *= 16;
                            value += snapdev::hexdigit_to_number(c);
                        }
                        if(!found)
                        {
                            snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                            msg << "hexadecimal number needs at least one digit after the \"0x\".";
                            throw invalid_number(msg.str());
                        }
                        node::pointer_t n(std::make_shared<node>(token_t::TOKEN_INTEGER, l));
                        n->set_integer(value);
                        return n;
                    }
                    f_input->ungetc(c);
                    c = '0';
                }
                value += c - '0';
                for(;;)
                {
                    c = f_input->getc();
                    if(c < '0' || c > '9')
                    {
                        break;
                    }
                    value *= 10;
                    value += c - '0';
                }
                if(c == '.')
                {
                    std::string number(value.to_string());
                    number += '.';
                    for(;;)
                    {
                        c = f_input->getc();
                        if(c < '0' || c > '9')
                        {
                            break;
                        }
                        number += static_cast<char>(c);
                    }
                    if(c == 'e' || c == 'E')
                    {
                        number += 'e';
                        c = f_input->getc();
                        if(c == '+' || c == '-')
                        {
                            number += static_cast<char>(c);
                            c = f_input->getc();
                        }
                        while(c >= '0' && c <= '9')
                        {
                            number += static_cast<char>(c);
                            c = f_input->getc();
                        }
                    }
                    f_input->ungetc(c);
                    node::pointer_t n(std::make_shared<node>(token_t::TOKEN_FLOATING_POINT, l));
                    char * e(nullptr);
                    errno = 0;
                    long double fp(strtold(number.c_str(), &e));
                    if(errno != 0 
                    || (e != nullptr && *e != '\0'))
                    {
                        snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                        msg << "invalid floating point number ("
                            << number
                            << ").";
                        throw invalid_number(msg.str());
                    }
                    n->set_floating_point(fp);
                    return n;
                }
                // TODO: verify that 'c' is acceptable after a number
                //       (i.e. white space, operator...)
                //
                f_input->ungetc(c);

                node::pointer_t n(std::make_shared<node>(token_t::TOKEN_INTEGER, l));
                n->set_integer(value);
                return n;
            }
            break;

        default:
            if((c >= 'a' && c <= 'z')
            || (c >= 'A' && c <= 'Z')
            || c == '_')
            {
                // TODO: identifiers should probably support all UTF-8 letters
                //
                std::string identifier;
                do
                {
                    identifier += c;
                    c = f_input->getc();
                }
                while((c >= 'a' && c <= 'z')
                   || (c >= 'A' && c <= 'Z')
                   || (c >= '0' && c <= '9')
                   || c == '_');
                f_input->ungetc(c);

                node::pointer_t n(std::make_shared<node>(token_t::TOKEN_IDENTIFIER, l));
                n->set_string(identifier);
                return n;
            }
            else
            {
                snaplogger::message msg(snaplogger::severity_t::SEVERITY_FATAL);
                msg << "unexpected token ("
                    << libutf8::to_u8string(c)
                    << ").";
                throw unexpected_token(msg.str());
            }
            break;

        }
    }
    snapdev::NOT_REACHED();
}



} // namespace pbql
} // namespace prinbee
// vim: ts=4 sw=4 et
