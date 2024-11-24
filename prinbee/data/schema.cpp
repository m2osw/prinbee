// Copyright (c) 2019-2024  Made to Order Software Corp.  All Rights Reserved
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
 * \brief Database file implementation.
 *
 * Each table uses one or more files. Each file is handled by a dbfile
 * object and a corresponding set of blocks.
 *
 * \todo
 * The `version` field is not going to be cross instance compatible.
 * Any new instance of a database file gets a schema with version 1.0.
 * That version increases as modifications to the schema are being applied
 * (for example, as you add a new plugin to the Snap! environment of a
 * website, the content table is likely to be updated and get a newer
 * version).
 * \todo
 * The problem with this mechanism is that the exact same schema on two
 * different nodes will not always have the same version. If you create
 * a new node when another has a schema version 1.15, then the new node
 * gets the same schema, but the version is set to 1.0.
 * \todo
 * On day to day matters, this has no bearing, but it could be really
 * confusing to administrators. There are two possible solutions: have
 * the version assigned using communication and use the latest version
 * for that table, latest version across your entire set of nodes.
 * The other, which is much easier as it requires no inter-node
 * communication, is to calculate an MD5 sum of the schema. As long as
 * that calculation doesn't change across versions of Snap! Database
 * then we're all good (but I don't think we can ever guarantee such
 * a thing, so that solution becomes complicated in that sense).
 */

// self
//
#include    "prinbee/data/schema.h"

#include    "prinbee/data/convert.h"
#include    "prinbee/data/script.h"
#include    "prinbee/utils.h"


// snaplogger
//
#include    <snaplogger/message.h>


// snapdev
//
#include    <snapdev/tokenize_string.h>
#include    <snapdev/to_upper.h>


// C++
//
#include    <iostream>
#include    <type_traits>


// last include
//
#include    <snapdev/poison.h>



namespace prinbee
{



namespace
{



constexpr char const * const    g_column_scope = "column::";
constexpr char const * const    g_expiration_date = "expiration_date";
constexpr char const * const    g_index_scope = "index::";



//struct_description_t g_column_description[] =
//{
//    define_description(
//          FieldName("name")
//        , FieldType(struct_type_t::STRUCT_TYPE_P8STRING)
//    ),
//    define_description(
//          FieldName("column_id")
//        , FieldType(struct_type_t::STRUCT_TYPE_UINT16)
//    ),
//    define_description(
//          FieldName("type")
//        , FieldType(struct_type_t::STRUCT_TYPE_UINT16)
//    ),
//    define_description(
//          FieldName("flags=limited/required/blob/system/revision_type:2")
//        , FieldType(struct_type_t::STRUCT_TYPE_BITS32)
//    ),
//    define_description(
//          FieldName("encrypt_key_name")
//        , FieldType(struct_type_t::STRUCT_TYPE_P16STRING)
//    ),
//    define_description(
//          FieldName("default_value")
//        , FieldType(struct_type_t::STRUCT_TYPE_BUFFER32)
//    ),
//    define_description(
//          FieldName("minimum_value")
//        , FieldType(struct_type_t::STRUCT_TYPE_BUFFER32)
//    ),
//    define_description(
//          FieldName("maximum_value")
//        , FieldType(struct_type_t::STRUCT_TYPE_BUFFER32)
//    ),
//    define_description(
//          FieldName("minimum_length")
//        , FieldType(struct_type_t::STRUCT_TYPE_UINT32)
//    ),
//    define_description(
//          FieldName("maximum_length")
//        , FieldType(struct_type_t::STRUCT_TYPE_UINT32)
//    ),
//    define_description(
//          FieldName("validation")
//        , FieldType(struct_type_t::STRUCT_TYPE_BUFFER32)
//    ),
//    end_descriptions()
//};
//
//
//struct_description_t g_column_reference[] =
//{
//    define_description(
//          FieldName("column_id")
//        , FieldType(struct_type_t::STRUCT_TYPE_UINT16)
//    ),
//    end_descriptions()
//};
//
//
//struct_description_t g_sort_column[] =
//{
//    define_description(
//          FieldName("column_id")
//        , FieldType(struct_type_t::STRUCT_TYPE_UINT16)
//    ),
//    define_description(
//          FieldName("flags=descending/not_null")
//        , FieldType(struct_type_t::STRUCT_TYPE_BITS32)
//    ),
//    define_description(
//          FieldName("length")
//        , FieldType(struct_type_t::STRUCT_TYPE_UINT32)
//    ),
//    define_description(
//          FieldName("function")
//        , FieldType(struct_type_t::STRUCT_TYPE_BUFFER32)
//    ),
//    end_descriptions()
//};


//struct_description_t g_table_secondary_index[] =
//{
//    define_description(
//          FieldName("name")
//        , FieldType(struct_type_t::STRUCT_TYPE_P8STRING)
//    ),
//    define_description(
//          FieldName("flags=distributed")
//        , FieldType(struct_type_t::STRUCT_TYPE_BITS32)
//    ),
//    define_description(
//          FieldName("sort_columns")
//        , FieldType(struct_type_t::STRUCT_TYPE_ARRAY16)
//        , FieldSubDescription(g_sort_column)
//    ),
//    define_description(
//          FieldName("filter")
//        , FieldType(struct_type_t::STRUCT_TYPE_BUFFER32)
//    ),
//    end_descriptions()
//};




//struct_description_t g_table_description[] =
//{
//    define_description(
//          FieldName("schema_version")
//        , FieldType(struct_type_t::STRUCT_TYPE_VERSION)
//    ),
//    define_description(
//          FieldName("added_on")
//        , FieldType(struct_type_t::STRUCT_TYPE_TIME)
//    ),
//    define_description(
//          FieldName("name")
//        , FieldType(struct_type_t::STRUCT_TYPE_P8STRING)
//    ),
//    define_description(
//          FieldName("flags=temporary")
//        , FieldType(struct_type_t::STRUCT_TYPE_BITS64)
//    ),
//    define_description(
//          FieldName("block_size")
//        , FieldType(struct_type_t::STRUCT_TYPE_UINT32)
//    ),
//    define_description(
//          FieldName("model")
//        , FieldType(struct_type_t::STRUCT_TYPE_UINT8)
//    ),
//    define_description(
//          FieldName("primary_key")
//        , FieldType(struct_type_t::STRUCT_TYPE_ARRAY16)
//        , FieldSubDescription(g_column_reference)
//    ),
//    define_description(
//          FieldName("secondary_indexes")
//        , FieldType(struct_type_t::STRUCT_TYPE_ARRAY16)
//        , FieldSubDescription(g_table_secondary_index)
//    ),
//    define_description(
//          FieldName("columns")
//        , FieldType(struct_type_t::STRUCT_TYPE_ARRAY16)
//        , FieldSubDescription(g_column_description)
//    ),
//    end_descriptions()
//};



}
// no name namespace



index_type_t index_name_to_index_type(std::string const & name)
{
    if(name.empty())
    {
        return index_type_t::INDEX_TYPE_INVALID;
    }

    if(name.length() >= 2) switch(name[1])
    {
    case 'e':
        if(name == "_expiration")
        {
            return index_type_t::INDEX_TYPE_EXPIRATION;
        }
        break;

    case 'i':
        if(name == "_indirect")
        {
            return index_type_t::INDEX_TYPE_INDIRECT;
        }
        break;

    case 'p':
        if(name == "_primary")
        {
            return index_type_t::INDEX_TYPE_PRIMARY;
        }
        break;

    case 't':
        if(name == "_tree")
        {
            return index_type_t::INDEX_TYPE_TREE;
        }
        break;

    }

    return validate_name(name.c_str())
                ? index_type_t::INDEX_TYPE_SECONDARY
                : index_type_t::INDEX_TYPE_INVALID;
}


std::string index_type_to_index_name(index_type_t type)
{
    switch(type)
    {
    case index_type_t::INDEX_TYPE_INDIRECT:   return "indirect";
    case index_type_t::INDEX_TYPE_PRIMARY:    return "primary";
    case index_type_t::INDEX_TYPE_EXPIRATION: return "expiration";
    case index_type_t::INDEX_TYPE_TREE:       return "tree";
    case index_type_t::INDEX_TYPE_INVALID:    break;
    case index_type_t::INDEX_TYPE_SECONDARY:  break;
    }

    // not a system type, return an empty string even for the secondary type
    //
    return std::string();
}



struct model_and_name_t
{
    model_t const           f_model = model_t::TABLE_MODEL_CONTENT;
    char const * const      f_name = nullptr;

    bool operator < (model_and_name_t const & rhs) const
    {
        return strcmp(f_name, rhs.f_name) < 0;
    }
};

#define MODEL_AND_NAME(name)    { model_t::TABLE_MODEL_##name, #name }

model_and_name_t g_model_and_name[] =
{
    MODEL_AND_NAME(CONTENT),
    MODEL_AND_NAME(DATA),
    MODEL_AND_NAME(DEFAULT),
    MODEL_AND_NAME(LOG),
    MODEL_AND_NAME(QUEUE),
    MODEL_AND_NAME(SEQUENCIAL),
    MODEL_AND_NAME(SESSION),
    MODEL_AND_NAME(TREE)
};


model_t name_to_model(std::string const & name)
{
#ifdef _DEBUG
    // verify in debug because if not in order we cannot do a valid binary search
    //
    if(!std::is_sorted(g_model_and_name, g_model_and_name + std::size(g_model_and_name)))
    {
        throw logic_error("names in g_model_and_name are not in alphabetical order.");
    }
#endif

    if(name.empty())
    {
        return model_t::TABLE_MODEL_DEFAULT;
    }

    std::string const uc(snapdev::to_upper(name));

    // unfortunately, the standard library does not offer an algorithm
    // that returns the element that is equal with a complexity of
    // log2(N)+O(1) like we have here
    //
    int i(0);
    int j(std::size(g_model_and_name));
    while(i < j)
    {
        int const p((j - i) / 2 + i);
        int const r(uc.compare(g_model_and_name[p].f_name));
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
            return g_model_and_name[p].f_model;
        }
    }

    throw invalid_name(
              "unrecognized model \""
            + name
            + "\".");
}






// required constructor for copying in the map
//
//schema_complex_type::schema_complex_type()
//{
//}


/** \brief Initialize a complex type from a configuration file.
 *
 * Once in a list of columns, a complex type becomes a
 * `STRUCT_TYPE_STRUCTURE`.
 *
 * \param[in] config  The configuration file (.ini) where this complex
 *                    type is defined.
 * \param[in] name  The name of the complex type being read.
 */
schema_complex_type::schema_complex_type(advgetopt::conf_file::pointer_t config, std::string const & name)
{
    if(name_to_struct_type(name) != INVALID_STRUCT_TYPE)
    {
        throw type_mismatch(
                  "the name of a complex type cannot be the name of a basic type; \""
                + name
                + "\" is not considered valid.");
    }

    f_name = name;

    std::string const section_name("type::" + name);

    // these parameters are optional; they remain empty if undefined
    //
    f_description = config->get_parameter(section_name + "::description");
    f_compare = config->get_parameter(section_name + "::compare");
    f_validation_script = config->get_parameter(section_name + "::validation_script");

    std::string const field_definitions(section_name + "::fields");
    std::string const enum_names(section_name + "::enum");

    bool const has_field_definitions(config->has_parameter(field_definitions));
    f_is_enum = config->has_parameter(enum_names);

    if(has_field_definitions
    && f_is_enum)
    {
        throw exclusive_fields("a complex type cannot have the \"fields\" and \"enum\" parameters defined together.");
    }

    if(f_is_enum)
    {
        // the enumeration type is optional, especially if the type is not an
        // enumeration (in which case it is ignored if defined)
        //
        std::string const enum_type_name(section_name + "::enum_type");
        if(config->has_parameter(enum_type_name))
        {
            f_enum_type = name_to_struct_type(config->get_parameter(enum_type_name));
            if(f_enum_type < struct_type_t::STRUCT_TYPE_INT8
            || f_enum_type > struct_type_t::STRUCT_TYPE_UINT64)
            {
                throw type_mismatch(
                          "an enum type must be an integer type from 8 to 64 bits; \""
                        + enum_type_name
                        + "\" is not considered valid.");
            }
        }

        advgetopt::string_list_t list;
        advgetopt::split_string(config->get_parameter(enum_names), list, {","});
        for(auto const & nv : list)
        {
            advgetopt::string_list_t name_value;
            advgetopt::split_string(nv, name_value, {" "});
            if(name_value.size() != 2)
            {
                throw type_mismatch(
                          "an enum definition must be a name and an integer separated by a space, not \""
                        + nv
                        + "\".");
            }
            auto const it_name(std::find_if(
                  f_fields.begin()
                , f_fields.end()
                , [&name_value](field_t const & f)
                {
                    return name_value[0] == f.f_name;
                }));
            if(it_name != f_fields.end())
            {
                throw type_mismatch(
                          "each name in an enum definition must be unique, found \""
                        + name_value[0]
                        + "\" twice.");
            }
            field_t enum_field;
            enum_field.f_name = name_value[0];
            enum_field.f_enum_value = convert_to_uint(name_value[1], 64);

            // TBD: do we want the values of an enum to be unique? at the moment
            //      I am thinking that yes and we are not offering the user to
            //      set the value anyway...
            auto const it_value(std::find_if(
                  f_fields.begin()
                , f_fields.end()
                , [&enum_field](field_t const & f)
                {
                    return enum_field.f_enum_value == f.f_enum_value;
                }));
            if(it_value != f_fields.end())
            {
                throw type_mismatch(
                          "each value in an enum definition must be unique, found \""
                        + name_value[1]
                        + "\" twice in \""
                        + it_value->f_name
                        + "\" and \""
                        + name_value[0]
                        + "\".");
            }

            f_fields.push_back(enum_field);
        }
    }
    else if(has_field_definitions)
    {
        advgetopt::string_list_t list;
        advgetopt::split_string(config->get_parameter(field_definitions), list, {","});
        for(auto const & nt : list)
        {
            advgetopt::string_list_t name_type;
            advgetopt::split_string(nt, name_type, {" "});
            if(name_type.size() != 2)
            {
                throw type_mismatch(
                          "a field definition must be a name and a type separated by a space, not \""
                        + nt
                        + "\".");
            }
            auto const it(std::find_if(
                  f_fields.begin()
                , f_fields.end()
                , [&name_type](field_t const & f)
                {
                    return name_type[0] == f.f_name;
                }));
            if(it != f_fields.end())
            {
                throw type_mismatch(
                          "each field name in an complex type must be unique, found \""
                        + name_type[0]
                        + "\" twice.");
            }

            // we do not yet have all the complex types so we cannot verify
            // their existance just yet (or whether a loop exists)
            //
            field_t field;
            field.f_name = name_type[0];
            field.f_type_name = name_type[1];
            f_fields.push_back(field);
        }
    }
    else
    {
        throw missing_parameter("a complex type must have \"fields=...\" or an \"enum=...\" definition.");
    }
}


//void schema_comple_type::verify_types(schema_complex_type::map_pointer_t complex_types)
//{
//    f_complex_types = complex_types;
//
//    f_type = name_to_struct_type(type_name);
//}


std::string schema_complex_type::get_name() const
{
    return f_name;
}


bool schema_complex_type::is_enum() const
{
    return f_is_enum;
}


std::size_t schema_complex_type::get_size() const
{
    return f_fields.size();
}


void schema_complex_type::set_type_name(int idx, std::string const & type_name)
{
    if(static_cast<std::size_t>(idx) >= f_fields.size())
    {
        throw out_of_range(
                "index ("
                + std::to_string(idx)
                + ") is too large for this complex type list of fields (max: "
                + std::to_string(f_fields.size())
                + ").");
    }

    if(type_name.empty())
    {
        throw invalid_name("the type name cannot be set to an empty string.");
    }

    // if the type_name is a complex type, then the following returns
    // the special value INVALID_STRUCT_TYPE so we need special handling
    // (i.e. you do not want to set the type name to STRUCTURE in that
    // case)
    //
    struct_type_t const type(name_to_struct_type(type_name));

    if(type == struct_type_t::STRUCT_TYPE_STRUCTURE)
    {
        throw invalid_name("the type name cannot be explicitly set to STRUCTURE; use the name of a complex type instead.");
    }

    if(type != INVALID_STRUCT_TYPE)
    {
        f_fields[idx].f_type = type;
    }
    else
    {
        f_fields[idx].f_type = struct_type_t::STRUCT_TYPE_STRUCTURE;
    }
    f_fields[idx].f_type_name = type_name;
}


std::string schema_complex_type::get_type_name(int idx) const
{
    if(static_cast<std::size_t>(idx) >= f_fields.size())
    {
        throw out_of_range(
                "index ("
                + std::to_string(idx)
                + ") is too large for this complex type list of fields (max: "
                + std::to_string(f_fields.size())
                + ").");
    }

    return f_fields[idx].f_type_name;
}


struct_type_t schema_complex_type::get_type(int idx) const
{
    if(static_cast<std::size_t>(idx) >= f_fields.size())
    {
        throw out_of_range(
                "index ("
                + std::to_string(idx)
                + ") is too large for this complex type list of fields (max: "
                + std::to_string(f_fields.size())
                + ").");
    }

    return f_fields[idx].f_type;
}


void schema_complex_type::set_type(int idx, struct_type_t type)
{
    if(static_cast<std::size_t>(idx) >= f_fields.size())
    {
        throw out_of_range(
                "index ("
                + std::to_string(idx)
                + ") is too large for this complex type list of fields (max: "
                + std::to_string(f_fields.size())
                + ").");
    }

    f_fields[idx].f_type = type;
    if(type != struct_type_t::STRUCT_TYPE_STRUCTURE)
    {
        f_fields[idx].f_type_name = to_string(type);
    }
}


std::int64_t schema_complex_type::get_enum_value(int idx) const
{
    if(static_cast<std::size_t>(idx) >= f_fields.size())
    {
        throw out_of_range(
                "index ("
                + std::to_string(idx)
                + ") is too large for this complex type list of fields (max: "
                + std::to_string(f_fields.size())
                + ").");
    }

    return f_fields[idx].f_enum_value;
}










schema_column::schema_column(
          schema_table::pointer_t table
        , advgetopt::conf_file::pointer_t config
        , std::string const & column_id)
    : f_schema_table(table)
{
    f_id = convert_to_uint(column_id, 16);
    if(f_id == 0)
    {
        throw invalid_number(
                  "a column identifier must be a 16 bit number except 0; \""
                + column_id
                + "\" is not valid.");
    }

    std::string const section_name(g_column_scope + column_id);

    std::string const column_field_name(section_name + "::name");
    if(!config->has_parameter(column_field_name))
    {
        throw missing_parameter(
                  "a \"name=...\" is mandatory for a column definition (id: "
                + column_id
                + ").");
    }
    f_name = config->get_parameter(column_field_name);
    if(!validate_name(f_name.c_str()))
    {
        throw invalid_name(
                  "\""
                + f_name
                + "\" is not a valid column name.");
    }
    if(f_name[0] == '_')
    {
        throw invalid_name(
                  "a user defined column name (\""
                + f_name
                + "\") cannot start with an underscore. This is reserved for system columns.");
    }

    std::string const column_field_type(section_name + "::type");
    if(!config->has_parameter(column_field_type))
    {
        throw missing_parameter(
                  "a \"type=...\" is mandatory in your \""
                + f_name
                + "\" column definition.");
    }
    std::string const & type_name(config->get_parameter(column_field_type));
    f_type = name_to_struct_type(type_name);
    if(f_type == INVALID_STRUCT_TYPE)
    {
        schema_complex_type::pointer_t ct(table->get_complex_type(type_name));
        if(ct == nullptr)
        {
            throw invalid_type(
                      "found unknown type \""
                    + type_name
                    + "\" in your \""
                    + f_name
                    + "\" column definition.");
        }

        // TODO: actually implement the complex type
        //       (at this time, I'm thinking that the way to do it is
        //       to use snapdev/brs.h and save each field that way in
        //       one binary blob)
        //
        throw not_yet_implemented(
                "full support for complex types not yet implemented");
    }

    // if the user defined an expiration date column, make sure it uses
    // the correct type otherwise that's a bug and it needs to be fixed
    //
    if(is_expiration_date_column())
    {
        switch(f_type)
        {
        case struct_type_t::STRUCT_TYPE_TIME:
        case struct_type_t::STRUCT_TYPE_MSTIME:
        case struct_type_t::STRUCT_TYPE_USTIME:
        case struct_type_t::STRUCT_TYPE_NSTIME:
            break;

        default:
            throw type_mismatch(
                    "the \"expiration_date\" column must be assigned a  valid time type (TIME, MSTIME, USTIME, NSTYPE); "
                  + to_string(f_type)
                  + " is not valid.");

        }
    }

    f_flags = 0;
    std::string const column_field_flags(section_name + "::flags");
    if(config->has_parameter(column_field_flags))
    {
        std::list<std::string> flags;
        snapdev::tokenize_string(flags, config->get_parameter(column_field_flags), { "," }, true);

        for(auto const & f : flags)
        {
            if(f == "blob")
            {
                f_flags |= COLUMN_FLAG_BLOB;
            }
            else if(f == "hidden")
            {
                f_flags |= COLUMN_FLAG_HIDDEN;
            }
            else if(f == "limited")
            {
                // limit display of this column by default because it
                // could be really large
                //
                f_flags |= COLUMN_FLAG_LIMITED;
            }
            else if(f == "required")
            {
                // i.e. column cannot be set to NULL; note that if a default
                //      exists then the column can be undefined and the
                //      default value gets used instead
                //
                f_flags |= COLUMN_FLAG_REQUIRED;
            }
            else if(f == "versioned")
            {
                // only versioned column generate a new row when updated
                //
                f_flags |= COLUMN_FLAG_VERSIONED;
            }
            else
            {
                throw invalid_name(
                          "column \""
                        + f_name
                        + "\" includes an unknown flag: \""
                        + f
                        + "\".");
            }
        }
    }

    std::string const column_field_encrypt(section_name + "::encrypt");
    f_encrypt_key_name = config->get_parameter(column_field_encrypt);

    std::string const column_field_description(section_name + "::description");
    f_description = config->get_parameter(column_field_description);

    std::string const column_field_default_value(section_name + "::default_value");
    if(config->has_parameter(column_field_default_value))
    {
        f_default_value = string_to_typed_buffer(f_type, config->get_parameter(column_field_default_value));
    }

    std::string const column_field_default_value_script(section_name + "::default_value_script");
    std::string const default_script(config->get_parameter(column_field_default_value_script));
    if(!default_script.empty())
    {
        // TODO: compile that script with as2js and save that in memory
        //
        //f_default_value_script = parse(default_script);
        throw not_yet_implemented("code -> f_default_value_script not yet implemented");
    }

    std::string const column_field_validation_script(section_name + "::validation_script");
    std::string code(config->get_parameter(column_field_validation_script));
    if(!code.empty())
    {
        // TODO: compile that script with as2js and save that in memory
        //
        //f_validation = parse(code);
        throw not_yet_implemented("code -> f_validation not yet implemented");
    }

    std::string const column_field_minimum_value(section_name + "::minimum_value");
    if(config->has_parameter(column_field_minimum_value))
    {
        f_minimum_value = string_to_typed_buffer(f_type, config->get_parameter(column_field_minimum_value));
    }

    std::string const column_field_maximum_value(section_name + "::maximum_value");
    if(config->has_parameter(column_field_maximum_value))
    {
        f_maximum_value = string_to_typed_buffer(f_type, config->get_parameter(column_field_maximum_value));
    }

    std::string const column_field_minimum_size(section_name + "::minimum_size");
    if(config->has_parameter(column_field_minimum_size))
    {
        f_minimum_size = convert_to_uint(config->get_parameter(column_field_minimum_size), 32);
    }

    std::string const column_field_maximum_size(section_name + "::maximum_size");
    if(config->has_parameter(column_field_maximum_size))
    {
        f_maximum_size = convert_to_uint(config->get_parameter(column_field_maximum_size), 32);
    }

    std::string const column_field_internal_size_limit(section_name + "::internal_size_limit");
    if(config->has_parameter(column_field_internal_size_limit))
    {
        f_internal_size_limit = convert_to_int(config->get_parameter(column_field_internal_size_limit), 32);
        if(f_internal_size_limit != -1 && f_internal_size_limit < 128)
        {
            throw type_mismatch(
                      "column \""
                    + table->get_name()
                    + "::column"
                    + column_id
                    + "::internal_size_limit\" has an invalid external size limit \""
                    + config->get_parameter(column_field_internal_size_limit)
                    + "\" value (not a number or too small).");
        }
    }
}


schema_column::schema_column(schema_table::pointer_t table, structure::pointer_t s)
    : f_schema_table(table)
{
    from_structure(s);
}


schema_column::schema_column(
              schema_table_pointer_t table
            , std::string const & name
            , struct_type_t type
            , flag32_t flags)
    : f_name(name)
    , f_type(type)
    , f_flags(flags)
    , f_schema_table(table)
{
    if((flags & COLUMN_FLAG_SYSTEM) == 0)
    {
        throw logic_error("only system columns can be created using this constructor.");
    }

    if(f_name.length() < 2
    || f_name[0] != '_')
    {
        throw logic_error("all system column names must start with an underscore.");
    }
}


void schema_column::from_structure(structure::pointer_t s)
{
    f_name = s->get_string("name");
    f_id = s->get_uinteger("column_id");
    f_type = static_cast<struct_type_t>(s->get_uinteger("type"));
    f_flags = s->get_uinteger("flags");
    f_encrypt_key_name = s->get_string("encrypt_key_name");
    f_internal_size_limit = s->get_integer("internal_size_limit");
    f_default_value = s->get_buffer("default_value");
    f_minimum_value = s->get_buffer("minimum_value");
    f_maximum_value = s->get_buffer("maximum_value");
    f_minimum_size = s->get_uinteger("minimum_size");
    f_maximum_size = s->get_uinteger("maximum_size");
    f_validation_script = s->get_buffer("validation_script");
}


/** \brief Return true if this column represents the "expiration_date" column.
 *
 * This function checks the name of the column. If the name is
 * "expiration_date", then the function returns true.
 *
 * \return true when the column name is "expiration_date".
 */
bool schema_column::is_expiration_date_column() const
{
    return f_name == g_expiration_date;
}


compare_t schema_column::compare(schema_column const & rhs) const
{
    compare_t result(compare_t::COMPARE_SCHEMA_EQUAL);

    if(f_name != rhs.f_name)
    {
        throw logic_error(
                  "the schema_column::compare() function can only be called"
                  " with two columns having the same name. You called it"
                  " with a column named \""
                + f_name
                + "\" and the other \""
                + rhs.f_name
                + "\".");
    }

    //f_column_id -- these are adjusted accordingly on a merge

    if(f_type != rhs.f_type)
    {
        return compare_t::COMPARE_SCHEMA_DIFFER;
    }

    // the LIMITED flag is just a display flag, it's really not important
    // still request for an update if changed by end user
    //
    if((f_flags & ~COLUMN_FLAG_LIMITED) != (rhs.f_flags & ~COLUMN_FLAG_LIMITED))
    {
        return compare_t::COMPARE_SCHEMA_DIFFER;
    }
    if(f_flags != rhs.f_flags)
    {
        result = compare_t::COMPARE_SCHEMA_UPDATE;
    }

    if(f_encrypt_key_name != rhs.f_encrypt_key_name)
    {
        return compare_t::COMPARE_SCHEMA_DIFFER;
    }

    if(f_default_value != rhs.f_default_value)
    {
        result = compare_t::COMPARE_SCHEMA_UPDATE;
    }

    if(f_minimum_value != rhs.f_minimum_value)
    {
        return compare_t::COMPARE_SCHEMA_DIFFER;
    }

    if(f_maximum_value != rhs.f_maximum_value)
    {
        return compare_t::COMPARE_SCHEMA_DIFFER;
    }

    if(f_minimum_size != rhs.f_minimum_size)
    {
        return compare_t::COMPARE_SCHEMA_DIFFER;
    }

    if(f_maximum_size != rhs.f_maximum_size)
    {
        return compare_t::COMPARE_SCHEMA_DIFFER;
    }

    // we can't do much better here, unfortunately
    // but if the script changes many things can be affected
    //
    if(f_validation_script != rhs.f_validation_script)
    {
        return compare_t::COMPARE_SCHEMA_DIFFER;
    }

    return result;
}


schema_table::pointer_t schema_column::get_table() const
{
    return f_schema_table.lock();
}


column_id_t schema_column::get_id() const
{
    return f_id;
}


void schema_column::set_id(column_id_t id)
{
    if(f_id != COLUMN_NULL)
    {
        throw id_already_assigned(
                  "this column already has an identifier ("
                + std::to_string(static_cast<int>(f_id))
                + ").");
    }

    f_id = id;
}


std::string schema_column::get_name() const
{
    return f_name;
}


struct_type_t schema_column::get_type() const
{
    return f_type;
}


flag32_t schema_column::get_flags() const
{
    return f_flags;
}


std::string schema_column::get_encrypt_key_name() const
{
    return f_encrypt_key_name;
}


std::int32_t schema_column::get_internal_size_limit() const
{
    return f_internal_size_limit;
}


buffer_t schema_column::get_default_value() const
{
    return f_default_value;
}


buffer_t schema_column::get_minimum_value() const
{
    return f_minimum_value;
}


buffer_t schema_column::get_maximum_value() const
{
    return f_maximum_value;
}


std::uint32_t schema_column::get_minimum_size() const
{
    return f_minimum_size;
}


std::uint32_t schema_column::get_maximum_size() const
{
    return f_maximum_size;
}


buffer_t schema_column::get_validation_script() const
{
    return f_validation_script;
}













/** \brief Parse a column definition.
 *
 * This function parses the column definition and saves it in this object.
 * The definition has one required parameters, which is the column name,
 * all the other parameters are optional:
 *
 * 1. the name of column
 * 2. the length, which is a number written between parenthesis just after
 *    the column name; it defines a prefix to sort columns against; if not
 *    defined, the default of 256 is used, which may not work for your
 *    specific case
 * 3. "desc" keyword: sort this column in descending order
 * 4. "nulls_last" keyword: view NULL values as larger than anything else
 * 5. "without_nulls" keyword: if that column is NULL, filter out the whole
 *    row
 *
 * \code
 * <column_id>[(length)] [desc] [nulls_last|without_nulls]
 * \endcode
 *
 * \note
 * The "nulls_last" and "without_nulls" are mutually exclusive.
 */
void schema_sort_column::from_config(std::string const & column_definition)
{
    advgetopt::string_list_t parameters;
    advgetopt::split_string(column_definition, parameters, {" "});

    if(parameters.empty())
    {
        throw invalid_parameter("schema_sort_column::from_config() called with an empty column_definition.");
    }
    std::string::size_type const pos(parameters[0].find('('));
    if(pos == std::string::npos)
    {
        f_column_id = convert_to_uint(parameters[0], 16);
    }
    else
    {
        f_column_id = convert_to_uint(parameters[0].substr(0, pos), 16);
        parameters.insert(parameters.begin() + 1, parameters[0].substr(pos));
    }

    f_size = SCHEMA_SORT_COLUMN_DEFAULT_SIZE;
    std::size_t idx(1);
    if(parameters.size() >= 2
    && parameters[1][0] == '(')
    {
        // we have the length parameter
        //
        if(parameters[idx].back() != ')')
        {
            throw invalid_parameter("schema_sort_column::from_config() found a length parameter where the ')' is missing.");
        }

        //std::string const size(parameters[1].substr(1, parameters[1].length() - 2));
        std::string const size(parameters[1].begin() + 1, parameters[1].end() - 1);
        if(size.empty())
        {
            throw invalid_parameter("schema_sort_column::from_config() found a length parameter without an actual length.");
        }

        f_size = convert_to_uint(size, 32);
        if(f_size <= 0)
        {
            throw invalid_parameter(
                  "the length of a sort column must be at least 1. \""
                + size
                + "\" is not acceptable.");
        }

        idx = 2;
    }

    for(; idx < parameters.size(); ++idx)
    {
        if(parameters[idx] == "desc")
        {
            f_flags |= SCHEMA_SORT_COLUMN_DESCENDING;
        }
        else
        {
            f_flags &= ~SCHEMA_SORT_COLUMN_DESCENDING;
        }
        if(parameters[idx] == "nulls_last")
        {
            f_flags |= SCHEMA_SORT_COLUMN_PLACE_NULLS_LAST;
        }
        else
        {
            f_flags &= ~SCHEMA_SORT_COLUMN_PLACE_NULLS_LAST;
        }
        if(parameters[idx] == "without_nulls")
        {
            f_flags |= SCHEMA_SORT_COLUMN_WITHOUT_NULLS;
        }
        else
        {
            f_flags &= ~SCHEMA_SORT_COLUMN_WITHOUT_NULLS;
        }
    }

    if((f_flags & (SCHEMA_SORT_COLUMN_PLACE_NULLS_LAST | SCHEMA_SORT_COLUMN_WITHOUT_NULLS))
               == (SCHEMA_SORT_COLUMN_PLACE_NULLS_LAST | SCHEMA_SORT_COLUMN_WITHOUT_NULLS))
    {
        throw invalid_parameter(
              "schema_sort_column::from_config() forbids the use of \"nulls_last\" and \"without_nulls\" at the same time.");
    }
}


compare_t schema_sort_column::compare(schema_sort_column const & rhs) const
{
    compare_t result(compare_t::COMPARE_SCHEMA_EQUAL);

    if(f_column_id != rhs.f_column_id)
    {
        return compare_t::COMPARE_SCHEMA_DIFFER;
    }

    if(f_flags != rhs.f_flags)
    {
        return compare_t::COMPARE_SCHEMA_DIFFER;
    }

    if(f_size != rhs.f_size)
    {
        return compare_t::COMPARE_SCHEMA_DIFFER;
    }

    return result;
}


column_id_t schema_sort_column::get_column_id() const
{
    return f_column_id;
}


void schema_sort_column::set_column_id(column_id_t column_id)
{
    f_column_id = column_id;
}


flag32_t schema_sort_column::get_flags() const
{
    return f_flags;
}


void schema_sort_column::set_flags(flag32_t flags)
{
    f_flags = flags;
}


bool schema_sort_column::is_ascending() const
{
    return (f_flags & SCHEMA_SORT_COLUMN_WITHOUT_NULLS) == 0;
}


bool schema_sort_column::accept_null_columns() const
{
    return (f_flags & SCHEMA_SORT_COLUMN_WITHOUT_NULLS) == 0;
}


bool schema_sort_column::place_nulls_last() const
{
    return (f_flags & SCHEMA_SORT_COLUMN_PLACE_NULLS_LAST) == 0;
}


std::uint32_t schema_sort_column::get_size() const
{
    return f_size;
}


void schema_sort_column::set_size(std::uint32_t size)
{
    f_size = size;
}










void schema_secondary_index::from_config(
      advgetopt::conf_file::pointer_t config
    , std::string const & index_id)
{
    f_id = convert_to_uint(index_id, 32);
    if(f_id == 0)
    {
        throw invalid_number(
                  "an index identifier must be a 32 bit number except 0; \""
                + index_id
                + "\" is not valid.");
    }

    std::string const section_name(g_index_scope + index_id);

    std::string const index_field_name(section_name + "::name");
    if(!config->has_parameter(index_field_name))
    {
        throw missing_parameter(
                  "a \"name=...\" is mandatory for an index definition (id: "
                + index_id
                + ").");
    }
    f_name = config->get_parameter(index_field_name);
    if(!validate_name(f_name.c_str()))
    {
        throw invalid_name(
                  "\""
                + f_name
                + "\" is not a valid index name.");
    }
    if(f_name[0] == '_')
    {
        throw invalid_name(
                  "a user defined index name (\""
                + f_name
                + "\") cannot start with an underscore. This is reserved for system indexes.");
    }

    std::string const index_field_description(section_name + "::description");
    f_description = config->get_parameter(index_field_description);

    std::string const index_field_key_script(section_name + "::key_script");
    std::string const key_script(config->get_parameter(index_field_key_script));
    if(!key_script.empty())
    {
        // TODO: compile that script with as2js and save that in memory
        //
        //f_key_script = parse(key_script);
        throw not_yet_implemented("code -> f_key_script not yet implemented");
    }

    std::string const index_field_filter_script(section_name + "::filter_script");
    std::string const filter_script(config->get_parameter(index_field_filter_script));
    if(!filter_script.empty())
    {
        // TODO: compile that script with as2js and save that in memory
        //
        //f_filter_script = parse(filter_script);
        throw not_yet_implemented("code -> f_filter_script not yet implemented");
    }

    f_flags = 0;
    std::string const index_field_flags(section_name + "::flags");
    if(config->has_parameter(index_field_flags))
    {
        std::list<std::string> flags;
        snapdev::tokenize_string(flags, config->get_parameter(index_field_flags), { "," }, true);

        for(auto const & f : flags)
        {
            if(f == "without_nulls")
            {
                f_flags |= SECONDARY_INDEX_FLAG_WITHOUT_NULLS;
            }
            else if(f == "nulls_not_distinct")
            {
                f_flags |= SECONDARY_INDEX_FLAG_NULLS_NOT_DISTINCT;
            }
            else if(f == "distributed")
            {
                f_flags |= SECONDARY_INDEX_FLAG_DISTRIBUTED;
            }
            else
            {
                throw invalid_name(
                          "the user defined index \""
                        + f_name
                        + "\" includes an unknown flag: \""
                        + f
                        + "\".");
            }
        }
    }

    std::string const index_field_columns(section_name + "::columns");
    if(!config->has_parameter(index_field_columns))
    {
        throw missing_parameter(
                  "a user defined index name (\""
                + f_name
                + "\") must have a columns=... parameter.");
    }

    advgetopt::string_list_t columns;
    advgetopt::split_string(config->get_parameter(index_field_columns), columns, {","});
    for(auto const & c : columns)
    {
        schema_sort_column::pointer_t sort_column(std::make_shared<schema_sort_column>());
        sort_column->from_config(c);
        f_sort_columns.push_back(sort_column); // vector because these are sorted by user
    }
}


compare_t schema_secondary_index::compare(schema_secondary_index const & rhs) const
{
    compare_t result(compare_t::COMPARE_SCHEMA_EQUAL);

    if(f_name != rhs.f_name)
    {
        throw logic_error(
                  "the schema_secondary_index::compare() function can only be"
                  " called with two secondary indexes having the same name."
                  " You called it with a column named \""
                + f_name
                + "\" and the other \""
                + rhs.f_name
                + "\".");
    }

    size_t const lhs_max(get_column_count());
    size_t const rhs_max(rhs.get_column_count());
    if(lhs_max != rhs_max)
    {
        return compare_t::COMPARE_SCHEMA_DIFFER;
    }

    for(size_t idx(0); idx < lhs_max; ++idx)
    {
        compare_t const sc_compare(f_sort_columns[idx]->compare(*rhs.f_sort_columns[idx]));
        if(sc_compare == compare_t::COMPARE_SCHEMA_DIFFER)
        {
            return compare_t::COMPARE_SCHEMA_DIFFER;
        }
        else if(sc_compare == compare_t::COMPARE_SCHEMA_UPDATE)
        {
            result = compare_t::COMPARE_SCHEMA_UPDATE;
        }
    }

    if(f_filter != rhs.f_filter)
    {
        return compare_t::COMPARE_SCHEMA_DIFFER;
    }

    if(f_flags != rhs.f_flags)
    {
        return compare_t::COMPARE_SCHEMA_DIFFER;
    }

    return result;
}


index_id_t schema_secondary_index::get_id() const
{
    return f_id;
}


std::string schema_secondary_index::get_name() const
{
    return f_name;
}


void schema_secondary_index::set_name(std::string const & name)
{
    f_name = name;
}


flag32_t schema_secondary_index::get_flags() const
{
    return f_flags;
}


void schema_secondary_index::set_flags(flag32_t flags)
{
    f_flags = flags;
}


bool schema_secondary_index::get_distributed_index() const
{
    return (f_flags & SECONDARY_INDEX_FLAG_DISTRIBUTED) != 0;
}


void schema_secondary_index::set_distributed_index(bool distributed)
{
    if(distributed)
    {
        f_flags |= SECONDARY_INDEX_FLAG_DISTRIBUTED;
    }
    else
    {
        f_flags &= ~SECONDARY_INDEX_FLAG_DISTRIBUTED;
    }
}


size_t schema_secondary_index::get_column_count() const
{
    return f_sort_columns.size();
}


schema_sort_column::pointer_t schema_secondary_index::get_sort_column(int idx) const
{
    if(static_cast<size_t>(idx) >= f_sort_columns.size())
    {
        throw out_of_range(
                  "index ("
                + std::to_string(idx)
                + ") is too large to pick a sort column from secondary index \""
                + f_name
                + "\".");
    }

    return f_sort_columns[idx];
}


void schema_secondary_index::add_sort_column(schema_sort_column::pointer_t sc)
{
    f_sort_columns.push_back(sc);
}


buffer_t schema_secondary_index::get_filter() const
{
    return f_filter;
}


void schema_secondary_index::set_filter(buffer_t const & filter)
{
    f_filter = filter;
}












schema_table::schema_table()
{
}


void schema_table::set_complex_types(schema_complex_type::map_pointer_t complex_types)
{
    f_complex_types = complex_types;
}


void schema_table::from_config(std::string const & name, std::string const & filename)
{
    // the list of sections are:
    //
    //    "table"                    from_config_load_table() & from_config_load_primary_key()
    //    "column::<identifier>"     from_config_load_columns()
    //    "index::<identifier>"      from_config_load_indexes()
    //
    // The table does not include complex type definitions; those are
    // defined in a separate file which is read prior to loading table
    // schemata; it is passed to the table by calling set_complex_types()
    // prior to this call
    //
    if(f_complex_types == nullptr)
    {
        throw logic_error("from_config() called before set_complex_types().");
    }

    from_config_name(name);
    from_config_version(filename);

    advgetopt::conf_file::pointer_t config(from_config_load_config());
    from_config_load_table(config);
    from_config_load_columns(config);
    from_config_load_primary_key(config);       // finishing up load_table() -- this part needs the list of columns
    from_config_load_indexes(config);
}


void schema_table::from_config_name(std::string const & name)
{
    // Note: this is already checked in the table_impl class, but we still
    //       want to make sure this name is valid
    //
    if(!validate_name(name.c_str()))
    {
        throw invalid_name(
                  "\""
                + name
                + "\" is not a valid table name.");
    }
    f_name = name;
}


void schema_table::from_config_version(std::string const & filename)
{
    std::string::size_type const pos(filename.rfind('-'));
    if(pos == std::string::npos)
    {
        throw type_mismatch(
                  "expected a dash + version in the table filename \""
                + filename
                + "\".");
    }
    char const * v(filename.c_str() + pos + 1);
    f_version = 0;
    while(*v >= '0' && *v <= '9')
    {
        f_version *= 10;
        f_version += *v - '0';
        ++v;
    }
    if(strcmp(v, ".ini") != 0)
    {
        throw type_mismatch(
                  "expected a dash + version followed by \".ini\" in the table filename \""
                + filename
                + "\".");
    }
    f_filename = filename;
}


advgetopt::conf_file::pointer_t schema_table::from_config_load_config()
{
    // load the .ini file
    //
    advgetopt::conf_file_setup setup(
          f_filename
        , advgetopt::line_continuation_t::line_continuation_unix
        , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
        , advgetopt::COMMENT_SHELL
        , advgetopt::SECTION_OPERATOR_INI_FILE | advgetopt::SECTION_OPERATOR_CPP);

    return advgetopt::conf_file::get_conf_file(setup);
}


void schema_table::from_config_load_table(advgetopt::conf_file::pointer_t config)
{
#if 0
    // I don't think this is still required; keeping for now as a reminder
    //
    bool const drop(x->attribute("drop") == "drop");
    if(drop)
    {
        // do not ever save a table when the DROP flag is set (actually
        // we want to delete the entire folder if it still exists!)
        //
        f_flags |= TABLE_FLAG_DROP;
        return;
    }
#endif

    // NAME
    {
        std::string const table_section_name("table::name");
        if(!config->has_parameter(table_section_name))
        {
            throw type_mismatch("the [table] section must have a name=... parameter.");
        }
        if(f_name != config->get_parameter(table_section_name))
        {
            throw type_mismatch(
                      "the table directory is \""
                    + f_name
                    + "\" and it was expected to match the [table] section name=... field which instead is set to \""
                    + config->get_parameter(table_section_name)
                    + "\".");
        }
    }

    // VERSION
    {
        std::string const table_section_version("table::version");
        if(!config->has_parameter(table_section_version))
        {
            throw type_mismatch("the [table] section must have a version=... parameter.");
        }
        std::string const ini_version(config->get_parameter(table_section_version));
        if(f_version != convert_to_uint(ini_version, 32))
        {
            throw type_mismatch(
                      "the table filename is \""
                    + f_filename
                    + "\" with version \""
                    + std::to_string(f_version)
                    + "\"; found version \""
                    + ini_version
                    + "\" in the .ini file; there is a mismatch.");
        }
    }

    // DESCRIPTION
    {
        std::string const table_section_description("table::description");
        if(config->has_parameter(table_section_description))
        {
            f_description = config->has_parameter(table_section_description);
        }
        else
        {
            f_description.clear();
        }
    }

    // REPLICATION
    {
        std::string const table_section_replication("table::replication");
        if(config->has_parameter(table_section_replication))
        {
            f_replication = convert_to_uint(config->get_parameter(table_section_replication), 8);
            if(f_replication == 0)
            {
                throw type_mismatch(
                          "table \""
                        + f_filename
                        + "\" has an invalid replication=... value: \""
                        + config->get_parameter(table_section_replication)
                        + "\". It is expected to be an integer from 1 to 255 included.");
            }
        }
        else
        {
            f_replication = TABLE_DEFAULT_REPLICATION;
        }
    }

    // MODEL
    {
        std::string const table_section_model("table::model");
        if(config->has_parameter(table_section_model))
        {
            f_model = name_to_model(config->get_parameter(table_section_model));
        }
        else
        {
            f_model = model_t::TABLE_MODEL_DEFAULT;
        }
    }

    // FLAGS
    {
        f_flags = flag64_t();
        std::string const table_section_flags("table::flags");
        if(config->has_parameter(table_section_flags))
        {
            advgetopt::string_list_t flags;
            advgetopt::split_string(config->get_parameter(table_section_flags), flags, {","});
            for(auto const & f : flags)
            {
                if(f == "secure")
                {
                    f_flags |= TABLE_FLAG_SECURE;
                }
                //else if(f == "temporary")
                //{
                //    f_flags |= TABLE_FLAG_TEMPORARY;
                //}
                else if(f == "translatable")
                {
                    f_flags |= TABLE_FLAG_TRANSLATABLE;
                }
                else if(f == "unlogged")
                {
                    f_flags |= TABLE_FLAG_UNLOGGED;
                }
                else
                {
                    throw unknown_parameter(
                              "table \""
                            + f_filename
                            + "\" has an unknown flag in its flags=... value: \""
                            + f
                            + "\".");
                }
            }
        }
    }

    // VERSIONED ROWS
    {
        std::string const table_section_versioned_rows("table::versioned_rows");
        if(config->has_parameter(table_section_versioned_rows))
        {
            f_versioned_rows = convert_to_uint(config->get_parameter(table_section_versioned_rows), 8);
            if(f_versioned_rows == 0)
            {
                throw type_mismatch(
                          "the table \""
                        + f_filename
                        + "\" has an invalid versioned rows \""
                        + config->get_parameter(table_section_versioned_rows)
                        + "\" value (not a number or too small or too large).");
            }
        }
        else
        {
            f_versioned_rows = 1;
        }
    }

    // BLOB LIMIT
    {
        std::string const table_section_blob_limit("table::blob_limit");
        if(config->has_parameter(table_section_blob_limit))
        {
            f_blob_limit = convert_to_uint(config->get_parameter(table_section_blob_limit), 32);
            if(f_blob_limit != 0 && f_blob_limit < 128)
            {
                throw type_mismatch(
                          "the table \""
                        + f_filename
                        + "\" has an invalid blob limit \""
                        + config->get_parameter(table_section_blob_limit)
                        + "\" value (not a number or too small).");
            }
        }
        else
        {
            f_blob_limit = 0;
        }
    }
}


void schema_table::from_config_load_columns(advgetopt::conf_file::pointer_t config)
{
    // 2. add system columns (i.e. those columns do not appear in the
    //    .ini files)
    //
    //    _schema_version
    //    _oid
    //    _created_on
    //    _created_by
    //    _last_updated
    //    _last_updated_by
    //    _deleted_on
    //    _deleted_by
    //    _version
    //    _language
    //
    // _current_version -- this is actually a row parameter... and we are
    //                     expected to have at least 2 such parameters:
    //                     the "current version" presented to the user and
    //                     the "current default version"; on top of that,
    //                     there should be the "current draft" and
    //                     "current default draft" entries

    // schema version -- to know which schema to use to read the data
    //
    // [row specialized field, this is not saved as a column in the data file]
    //
    // this one is managed as a very special case instead; the version
    // is saved as the first 4 bytes of any one row; plus on a read we
    // always auto-convert the data to the latest schema version so
    // having such a column would not be useful (i.e. it would always
    // be the exact same value as far as the end user is concerned)
    //
    {
        auto c(std::make_shared<schema_column>(
                      shared_from_this()
                    , "_schema_version"
                    , struct_type_t::STRUCT_TYPE_UINT32
                    , COLUMN_FLAG_REQUIRED | COLUMN_FLAG_SYSTEM));

        f_columns_by_name[c->get_name()] = c;
    }

    // object identifier -- to place the rows in our primary index
    //
    // [row specialized field, this is not saved as a column in the data file]
    //
    // this field is not required here, but by having it, we can lose
    // the primary index and not lose anything since we have the information
    // here
    {
        auto c(std::make_shared<schema_column>(
                      shared_from_this()
                    , "_oid"
                    , struct_type_t::STRUCT_TYPE_OID
                    , COLUMN_FLAG_REQUIRED | COLUMN_FLAG_SYSTEM));

        f_columns_by_name[c->get_name()] = c;
    }

    // version of the data in this row
    //
    // [row specialized field, this is not saved as a column in the data file]
    //
    // --------------------------------------------------------- resume -----
    //
    // The implementation is most TBD TBD TBD still. The following is my
    // current talk about it. However, I think I got most of it laid out
    // as I think it will be easiest: use the version + language to generate
    // keys in separate branch and revision specific indexes, which is very
    // similar to what I've done in Cassandra. We also probably need two
    // fields: one to read a specific version and one to write a revision
    // which would get automatically updated to a new branch and/or revision.
    //
    // See also [but that function is wrong because murmur3 cannot include
    // the version & language]:
    // void row::generate_mumur3(buffer_t & murmur3, version_t version, std::string const language);
    //
    // ------------------------------------------------ long discussion -----
    //
    // How this will be implemented is not clear at this point--it will
    // only be for the `content` table (Note: in our Cassandra implementation
    // we had a `content`, a `branch` and a `revision` table);
    //
    // The version itself would not be saved as a column per se, instead
    // it would be a form of sub-index where the type of column defines
    // how a read is handle based on the version:
    //
    // `global` -- the version is ignored for all global fields
    // `branch` -- the fields are assigned the `major` version; so when the
    // version is 1.1 or 1.100, the data returned is the same; however, you
    // have two separate values for versions 1.55 and 3.2
    // `revision` -- the fields are assigned the full version
    // (`major.minor`); each piece of data depends 100% on the version
    //
    // So on a `commit()`, global fields are always overwritten, branches
    // are overwritten on a per `major` version and revisions only on a
    // per `major.minor` version.
    //
    // As far as the client is concerned, though, such rows have a version
    // column which clearly defines each column's value
    //
    // The `_version` in a row can be set to an existing version in the row.
    // If not defined, then no branch or revision are created at all. If
    // set to version `0.0`, then that means create a new revision in the
    // latest existing branch (i.e. no revision 0 exists, it's either `0.1`
    // or undefined). At this point, I do not have a good idea to also force
    // the creation of a new branch, unless we convert this field to a string.
    // Then we can use all sorts of characters for the purpose (which means
    // we may want two fields--one for write as a string and one for read):
    //
    // 1. `*.1` -- create a new branch with a first revision 1
    // 2. `L.*` -- create a new revision in the [L]atest branch
    // 3. `L.L` -- overwrite/update the latest branch and revision fields
    // 4. `0.*` -- create a new revision in specified branch (here branch `0`)
    //
    // **IMPORTANT:** The revision also makes use of a language. If the
    // "_language" column is not defined, then use "xx" as the default.
    //
    // Implementation Ideas: (right now I think #3 is what we must use
    // except that it prevents us from having a simple list of
    // `major:minor:language` sub-indexes which I think we need--so that
    // means we probably should use #2 instead)
    //
    // 1. add the major version along the column ID when saving a branch
    //    value (ID:major:value); in effect we end up with many more columns
    //    for the one same row, only we just read those that have a major
    //    that matches the `_version` field; similarly, the revision is
    //    defined as column ID, major, minor (ID:major:minor:value)
    // 2. the row has a `reference_t` to a "branch array"; that array is
    //    a set of `reference_t` that point to all the columns specific to
    //    that branch, the `major` version is the index in that table (we
    //    have a map, though (`major => reference_t`) so that way older
    //    branches can be deleted if/when necessary; the revisions would be
    //    managed in a similar way; the main row has a reference to an array
    //    which has a map defined as `major:minor:language => reference_t`
    //    and the reference points to all the columns assigned that specific
    //    revision
    // 3. full fledge indexes which make use of the row key + major version
    //    for branches and row key + major + minor version + language for
    //    revision and add two more indexes in our headers just for those two;
    //    NOTE: with a full fledge index we can distribute the data
    //    between computers; whether we want to do that is still TBD
    //
    // The main problem with (1) is that one row will grow tremendously and
    // that will probably be impossible to manage after a while (on reads as
    // well as sheer size of the row). (2) is great although it certainly
    // requires a lot more specialized management to maintain the arrays.
    // (3) is probably the best since we should be able to reuse much of the
    // code handling indexes which will just be the standard row key plus
    // the necessary version info (major or major:minor). This is what we
    // have in Cassandra although we have to manually handle all the revisions
    // in our Snap! code.
    //
    {
        auto c(std::make_shared<schema_column>(
                      shared_from_this()
                    , "_version"
                    , struct_type_t::STRUCT_TYPE_VERSION
                    , COLUMN_FLAG_SYSTEM));

        f_columns_by_name[c->get_name()] = c;
    }

    // language code used in the "body", "title", etc.
    //
    // [row specialized field, this is not saved as a column in the data file]
    //
    // [IMPORTANT: we use a language identifier to avoid using a string
    //             which would have a variable size]
    //
    // By default, we use a 2 letter code ISO-639-1 code, but this field
    // allows for any ISO encoding such as "en-us" and any macro language.
    // The low level system implementation doesn't care and won't verify
    // that the language is valid. We offer higher level functions to do
    // so if you'd like to verify before letting a user select a language.
    //
    // Use "xx" for an entry in a table that uses languages but does not
    // require one. Not defining the field means that no language is
    // specified. The system will automatically use "xx" for revisions.
    //
    {
        auto c(std::make_shared<schema_column>(
                      shared_from_this()
                    , "_language"
                    , struct_type_t::STRUCT_TYPE_UINT16
                    , COLUMN_FLAG_SYSTEM));

        f_columns_by_name[c->get_name()] = c;
    }

    // date when the row was created
    {
        auto c(std::make_shared<schema_column>(
                      shared_from_this()
                    , "_created_on"
                    , struct_type_t::STRUCT_TYPE_MSTIME
                    , COLUMN_FLAG_SYSTEM));

        f_columns_by_name[c->get_name()] = c;
    }

    // when the row was last updated
    {
        auto c(std::make_shared<schema_column>(
                      shared_from_this()
                    , "_last_updated"
                    , struct_type_t::STRUCT_TYPE_MSTIME
                    , COLUMN_FLAG_REQUIRED | COLUMN_FLAG_SYSTEM));

        f_columns_by_name[c->get_name()] = c;
    }

    // the date when it gets deleted automatically
    {
        auto c(std::make_shared<schema_column>(
                      shared_from_this()
                    , "_deleted_on"
                    , struct_type_t::STRUCT_TYPE_MSTIME
                    , COLUMN_FLAG_SYSTEM));

        f_columns_by_name[c->get_name()] = c;
    }

    // ID of user who created this row
    {
        auto c(std::make_shared<schema_column>(
                      shared_from_this()
                    , "_created_by"
                    , struct_type_t::STRUCT_TYPE_UINT64
                    , COLUMN_FLAG_SYSTEM));

        f_columns_by_name[c->get_name()] = c;
    }

    // ID of user who last updated this row
    {
        auto c(std::make_shared<schema_column>(
                      shared_from_this()
                    , "_last_updated_by"
                    , struct_type_t::STRUCT_TYPE_UINT64
                    , COLUMN_FLAG_SYSTEM));

        f_columns_by_name[c->get_name()] = c;
    }

    // ID of user who deleted this row
    {
        auto c(std::make_shared<schema_column>(
                      shared_from_this()
                    , "_deleted_by"
                    , struct_type_t::STRUCT_TYPE_UINT64
                    , COLUMN_FLAG_SYSTEM));

        f_columns_by_name[c->get_name()] = c;
    }

// this is not the right place for the current version concept; that would
// need to be in the corresponding index
//
// there are also several important concepts:
//
// 1. we have the latest version; usually called the "draft" because it is
//    likely not publicly available
//
// 2. we have the current version which is the public one
//
// 3. we also have a similar concept with the default value (i.e. the rows
//    where the major version is 0); so we have a current default value and
//    a "draft" (latest) default value
//
//    // current version
//    //
//    // this is another entry in link with the branch/revision concept, we
//    // need to display a page, we need to have a current version to display
//    // that page; problem here is we need one such version per language
//    //
//    // in the old Cassandra database we also have a latest version, which
//    // is also per language; this latest version gets used to create new
//    // revisions effectively
//    //
//    // finally, we had a 'last edited version' because if you were to edit
//    // and not save your editing, we wanted to save a version of the page
//    // attached to your user and that was a form of "floating" version
//    // (i.e. it was not yet assigned a full version/language pair)
//    //
//    // for now I leave this at that, but I think we'll need several more
//    // fields to manage the whole set of possibilities (although things
//    // such as the last edited page is per user so we can't just have one
//    // field? well... maybe we track the last 100 edits and delete anything
//    // that's too old and was not properly saved after that) -- the editing
//    // versions can be called "draft"; which could also make use of the
//    // language field to distinguish them: "<major>.<minor>::<language>-draft"
//    //
//    {
//        auto c(std::make_shared<schema_column>(
//                      shared_from_this()
//                    , "_current_version"
//                    , struct_type_t::STRUCT_TYPE_VERSION
//                    , COLUMN_FLAG_SYSTEM));
//
//        f_columns_by_name[c->name()] = c;
//    }

    // "_expiration_date" -- we actually do not need an expiration date
    // column, the user can create his own "expiration_date" column which
    // will automatically get picked up by the system; i.e. rows with
    // a column with that name will automatically be added to the expiration
    // index, nothing more to do and the programmer has the ability to choose
    // the precision and what the value should be (it's just like a standard
    // column) -- see is_expiration_date_column()

    // go through and load columns
    //
    advgetopt::conf_file::sections_t sections(config->get_sections());
    for(auto s : sections)
    {
        if(!s.starts_with(g_column_scope))
        {
            continue;
        }

        auto c(std::make_shared<schema_column>(
                      shared_from_this()
                    , config
                    , s.substr(strlen(g_column_scope))));

        f_columns_by_name[c->get_name()] = c;
        f_columns_by_id[c->get_id()] = c;
    }
}


void schema_table::from_config_load_primary_key(advgetopt::conf_file::pointer_t config)
{
    std::string const table_section_primary_key("table::primary_key");
    if(!config->has_parameter(table_section_primary_key))
    {
        throw missing_parameter(
                  "the table \""
                + f_filename
                + "\" must have a primary_key=... parameter defined.");
    }
    advgetopt::string_list_t primary_key_values;
    advgetopt::split_string(config->get_parameter(table_section_primary_key), primary_key_values, { "," });
    f_primary_key.clear();
    for(auto const & id : primary_key_values)
    {
        column_id_t const column_id(convert_to_uint(id, 16));
        if(f_columns_by_id.find(column_id) == f_columns_by_id.end())
        {
            throw column_not_found(
                      "the table primary key in \""
                    + f_filename
                    + "\" references a column with identifier "
                    + id
                    + " which is not defined.");
        }
        f_primary_key.push_back(column_id);
    }
}


void schema_table::from_config_load_indexes(advgetopt::conf_file::pointer_t config)
{
    advgetopt::conf_file::sections_t sections(config->get_sections());
    for(auto s : sections)
    {
        if(!s.starts_with(g_index_scope))
        {
            continue;
        }

        auto index(std::make_shared<schema_secondary_index>());
        index->from_config(
                  config
                , s.substr(strlen(g_index_scope)));

        std::size_t const max(index->get_column_count());
        for(std::size_t idx(0); idx < max; ++idx)
        {
            schema_sort_column::pointer_t sort_column(index->get_sort_column(idx));
            column_id_t const column_id(sort_column->get_column_id());
            if(f_columns_by_id.find(column_id) == f_columns_by_id.end())
            {
                throw column_not_found(
                          "column with identifier \""
                        + std::to_string(static_cast<int>(column_id))
                        + "\" not found in table \""
                        + f_name
                        + "\".");
            }
        }

        f_secondary_indexes[index->get_name()] = index;
        f_secondary_indexes_by_id[index->get_id()] = index;
    }
}


//void schema_table::load_extension(advgetopt::conf_file::pointer_t s)
//{
//    basic_xml::node::deque_t secondary_indexes;
//
//    for(auto child(e->first_child()); child != nullptr; child = child->next())
//    {
//        if(child->tag_name() == "schema")
//        {
//            process_columns(child);
//        }
//        else if(child->tag_name() == "secondary-index")
//        {
//            secondary_indexes.push_back(child);
//        }
//        else
//        {
//            // generate an error for unknown tags or ignore?
//            //
//            SNAP_LOG_WARNING
//                << "unknown tag \""
//                << child->tag_name()
//                << "\" within a <table-extension> tag ignored."
//                << SNAP_LOG_SEND;
//        }
//    }
//
//    process_secondary_indexes(secondary_indexes);
//}


//void schema_table::process_columns(advgetopt::conf_file::pointer_t column_definitions)
//{
//    for(auto column(column_definitions->first_child());
//        column != nullptr;
//        column = column->next())
//    {
//        auto c(std::make_shared<schema_column>(shared_from_this(), column));
//        if(f_columns_by_name.find(c->name()) != f_columns_by_name.end())
//        {
//            SNAP_LOG_WARNING
//                << "column \""
//                << f_name
//                << '.'
//                << c->name()
//                << "\" defined twice. Second definition ignored."
//                << SNAP_LOG_SEND;
//            continue;
//        }
//        f_columns_by_name[c->name()] = c;
//    }
//}


//void schema_table::process_secondary_indexes(advgetopt::conf_file::pointer_t secondary_indexes)
//{
//    for(auto const & si : secondary_indexes)
//    {
//        schema_secondary_index::pointer_t index(std::make_shared<schema_secondary_index>());
//        index->from_xml(si);
//        f_secondary_indexes[index->get_name()] = index;
//    }
//}


/** \brief Compare two schema tables.
 *
 * This operator let you know whether two schema descriptions are considered
 * equal or not.
 *
 * The compare ignores some fields and flags because equality implies that
 * the content of the table, as in the data being inserted, selected,
 * updated, and deleted is not going to be different between the two
 * different schema_table descriptions. However, we still want to overwrite
 * the newest version with the new version if it has some differences.
 *
 * The return value tells you whether some differences
 * (COMPARE_SCHEMA_UPDATE), or important changes (COMPARE_SCHEMA_DIFFER)
 * where found. If the schemas as the exact same, then the function says
 * they are equal (COMPARE_SCHEMA_EQUAL). Note that in most cases, we expect
 * the function to return COMPARE_SCHEMA_EQUAL since schemata should rarely
 * change.
 *
 * \param[in] rhs  The righ hand side to compare this schema.
 *
 * \return One of the compare_t enumeration values.
 */
compare_t schema_table::compare(schema_table const & rhs) const
{
    compare_t result(compare_t::COMPARE_SCHEMA_EQUAL);

    //f_version -- we calculate the version
    //f_added_on -- this is dynamically assigned on creation

    if(f_name != rhs.f_name)
    {
        return compare_t::COMPARE_SCHEMA_DIFFER;
    }

    if(f_flags != rhs.f_flags)
    {
        return compare_t::COMPARE_SCHEMA_DIFFER;
    }

    if(f_model != rhs.f_model)
    {
        result = compare_t::COMPARE_SCHEMA_UPDATE;
    }

    if(f_primary_key != rhs.f_primary_key)
    {
        return compare_t::COMPARE_SCHEMA_DIFFER;
    }

    for(schema_secondary_index::map_by_name_t::const_iterator
            it(f_secondary_indexes.cbegin());
            it != f_secondary_indexes.cend();
            ++it)
    {
        schema_secondary_index::pointer_t rhs_secondary_index(rhs.get_secondary_index(it->first));
        if(rhs_secondary_index == nullptr)
        {
            return compare_t::COMPARE_SCHEMA_DIFFER;
        }
        compare_t const r(it->second->compare(*rhs_secondary_index));
        if(r == compare_t::COMPARE_SCHEMA_DIFFER)
        {
            return compare_t::COMPARE_SCHEMA_DIFFER;
        }
        if(r == compare_t::COMPARE_SCHEMA_UPDATE)
        {
            result = compare_t::COMPARE_SCHEMA_UPDATE;
        }
    }

    // loop through the RHS in case we removed a secondary index
    //
    for(schema_secondary_index::map_by_name_t::const_iterator
            it(rhs.f_secondary_indexes.cbegin());
            it != rhs.f_secondary_indexes.cend();
            ++it)
    {
        if(get_secondary_index(it->first) == nullptr)
        {
            return compare_t::COMPARE_SCHEMA_DIFFER;
        }
    }

    //f_columns_by_id -- we only have to compare one map
    //                   so we ignore f_columns_by_id which has the same data
    //
    for(schema_column::map_by_name_t::const_iterator
            it(f_columns_by_name.cbegin());
            it != f_columns_by_name.cend();
            ++it)
    {
        schema_column::pointer_t rhs_column(rhs.get_column(it->first));
        if(rhs_column == nullptr)
        {
            // we could not find that column in the other schema,
            // so it's different
            //
            // TODO: make sure "renamed" columns are handled properly
            //       once we add that feature
            //
            return compare_t::COMPARE_SCHEMA_DIFFER;
        }
        compare_t r(it->second->compare(*rhs_column));
        if(r == compare_t::COMPARE_SCHEMA_DIFFER)
        {
            return compare_t::COMPARE_SCHEMA_DIFFER;
        }
        if(r == compare_t::COMPARE_SCHEMA_UPDATE)
        {
            result = compare_t::COMPARE_SCHEMA_UPDATE;
        }
    }

    // loop through the RHS in case we removed a column
    //
    for(schema_column::map_by_name_t::const_iterator
            it(rhs.f_columns_by_name.cbegin());
            it != rhs.f_columns_by_name.cend();
            ++it)
    {
        if(get_column(it->first) == nullptr)
        {
            // we could not find that column in the new schema,
            // so it's different
            //
            // TODO: make sure "renamed" columns are handled properly
            //       once we add that feature [use IDs instead of names?]
            //
            return compare_t::COMPARE_SCHEMA_DIFFER;
        }
    }

    //f_description -- totally ignored; that's just noise

    return result;
}


//void schema_table::from_binary(virtual_buffer::pointer_t b)
//{
//    structure::pointer_t s(std::make_shared<structure>(g_table_description));
//
//    s->set_virtual_buffer(b, 0);
//
//    f_version  = s->get_uinteger("schema_version");
//    f_added_on = s->get_integer("added_on");
//    f_name     = s->get_string("name");
//    f_flags    = s->get_uinteger("flags");
//    f_model    = static_cast<model_t>(s->get_uinteger("model"));
//
//    {
//        auto const field(s->get_field("primary_key"));
//        auto const max(field->size());
//        for(std::remove_const<decltype(max)>::type idx(0); idx < max; ++idx)
//        {
//            f_primary_key.push_back((*field)[idx]->get_uinteger("column_id"));
//        }
//    }
//
//    {
//        auto const field(s->get_field("secondary_indexes"));
//        auto const max(field->size());
//        for(std::remove_const<decltype(max)>::type idx(0); idx < max; ++idx)
//        {
//            schema_secondary_index::pointer_t secondary_index(std::make_shared<schema_secondary_index>());
//
//            secondary_index->set_index_name((*field)[idx]->get_string("name"));
//            secondary_index->set_flags((*field)[idx]->get_uinteger("flags"));
//
//            auto const columns_field((*field)[idx]->get_field("sort_columns"));
//            auto const columns_max(columns_field->size());
//            for(std::remove_const<decltype(columns_max)>::type j(0); j < columns_max; ++j)
//            {
//                schema_sort_column::pointer_t sc(std::make_shared<schema_sort_column>());
//                sc->set_column_id((*columns_field)[j]->get_uinteger("column_id"));
//                sc->set_flags((*columns_field)[j]->get_uinteger("flags"));
//                sc->set_length((*columns_field)[j]->get_uinteger("length"));
//                secondary_index->add_sort_column(sc);
//            }
//
//            secondary_index->set_filter((*field)[idx]->get_buffer("filter"));
//
//            f_secondary_indexes[secondary_index->get_name()] = secondary_index;
//        }
//    }
//
//    {
//        auto field(s->get_field("columns"));
//        auto const max(field->size());
//        for(std::remove_const<decltype(max)>::type idx(0); idx < max; ++idx)
//        {
//            schema_column::pointer_t column(std::make_shared<schema_column>(shared_from_this(), (*field)[idx]));
//            if(column->column_id() == 0)
//            {
//                throw id_missing(
//                      "loaded column \""
//                    + column->name()
//                    + "\" from the database and its column identifier is 0.");
//            }
//
//            f_columns_by_name[column->name()] = column;
//            f_columns_by_id[column->column_id()] = column;
//        }
//    }
//}
//
//
//virtual_buffer::pointer_t schema_table::to_binary() const
//{
//    structure::pointer_t s(std::make_shared<structure>(g_table_description));
//    s->init_buffer();
//    s->set_uinteger("schema_version", f_version.to_binary());
//    s->set_integer("added_on", f_added_on);
//    s->set_string("name", f_name);
//    s->set_uinteger("flags", f_flags);
//    s->set_uinteger("block_size", f_block_size);
//    s->set_uinteger("model", static_cast<uint8_t>(f_model));
//
//    {
//        auto const max(f_primary_key.size());
//        for(std::remove_const<decltype(max)>::type i(0); i < max; ++i)
//        {
//            structure::pointer_t column_id_structure(s->new_array_item("primary_key"));
//            column_id_structure->set_uinteger("column_id", f_primary_key[i]);
//        }
//    }
//
//    {
//        for(auto it(f_secondary_indexes.cbegin());
//                 it != f_secondary_indexes.cend();
//                 ++it)
//        {
//            structure::pointer_t secondary_index_structure(s->new_array_item("secondary_indexes"));
//            secondary_index_structure->set_string("name", it->second->get_name());
//            secondary_index_structure->set_uinteger("flags", it->second->get_flags());
//            secondary_index_structure->set_buffer("filter", it->second->get_filter());
//
//            auto const jmax(it->second->get_column_count());
//            for(std::remove_const<decltype(jmax)>::type j(0); j < jmax; ++j)
//            {
//                structure::pointer_t sort_column_structure(secondary_index_structure->new_array_item("sort_columns"));
//                schema_sort_column::pointer_t sc(it->second->get_sort_column(j));
//                sort_column_structure->set_uinteger("column_id", sc->get_column_id());
//                sort_column_structure->set_uinteger("flags", sc->get_flags());
//                sort_column_structure->set_uinteger("length", sc->get_length());
//            }
//        }
//    }
//
//    {
//        for(auto const & col : f_columns_by_id)
//        {
//            structure::pointer_t column_description(s->new_array_item("columns"));
//            column_description->set_string("name", col.second->name());
//            column_description->set_uinteger("column_id", col.second->column_id());
//            column_description->set_uinteger("type", static_cast<uint16_t>(col.second->type()));
//            column_description->set_uinteger("flags", col.second->flags());
//            column_description->set_string("encrypt_key_name", col.second->encrypt_key_name());
//            column_description->set_buffer("default_value", col.second->default_value());
//            column_description->set_buffer("minimum_value", col.second->minimum_value());
//            column_description->set_buffer("maximum_value", col.second->maximum_value());
//            column_description->set_uinteger("minimum_length", col.second->minimum_length());
//            column_description->set_uinteger("maximum_length", col.second->maximum_length());
//            column_description->set_buffer("validation", col.second->validation());
//        }
//    }
//
//    // we know it is zero so we ignore that one anyay
//    //
//    uint64_t start_offset(0);
//    return s->get_virtual_buffer(start_offset);
//}


schema_version_t schema_table::get_schema_version() const
{
    return f_version;
}


/** \brief Set the version of the schema.
 *
 * This function is used only internally to set the version of the schema.
 * By default, all schemata are assigned version 1.0 on a read. However,
 * it may later be determined that this is an updated version of the
 * schema for a given table. In that case, the table will know what its
 * current version is (i.e. the latest version of the schema in that
 * table). Using that version + 1 is going to determine the new schema
 * version for this table and that's what gets assigned here.
 *
 * There are no other reasons to set the schema version. So you most
 * certainly never ever need to call this function ever.
 *
 * \param[in] version  The new schema version.
 */
void schema_table::set_schema_version(schema_version_t version)
{
    f_version = version;
}


time_t schema_table::get_added_on() const
{
    return f_added_on;
}


std::string schema_table::get_name() const
{
    return f_name;
}


model_t schema_table::get_model() const
{
    return f_model;
}


bool schema_table::is_secure() const
{
    return (f_flags & TABLE_FLAG_SECURE) != 0;
}


column_ids_t schema_table::get_primary_key() const
{
    return f_primary_key;
}


// we now immediately have the IDs and thus there is nothing to assign later
//void schema_table::assign_column_ids(schema_table::pointer_t existing_schema)
//{
//    if(!f_columns_by_id.empty())
//    {
//        return;
//    }
//
//    // if we have an existing schema, the same columns must be given the
//    // exact same identifier or else it would all break
//    //
//    if(existing_schema != nullptr)
//    {
//        for(auto c : f_columns_by_name)
//        {
//            if(c.second->get_id() != 0)
//            {
//                throw logic_error(
//                          "column \""
//                        + f_name
//                        + "."
//                        + c.second->get_name()
//                        + "\" was already given an identifier: "
//                        + std::to_string(static_cast<int>(c.second->get_id()))
//                        + ".");
//            }
//
//            schema_column::pointer_t e(existing_schema->get_column(c.first));
//            if(e != nullptr)
//            {
//                // keep the same identifier as in the source schema
//                //
//                c.second->set_id(e->get_id());
//                f_columns_by_id[e->get_id()] = c.second;
//            }
//        }
//    }
//
//    // in case new columns were added, we want to give them a new identifier
//    // also in case old columns were removed, we can reuse their identifier
//    //
//    // Note: that works because each row has a reference to to the schema
//    //       that was used when we created it and that means the column
//    //       identifiers will be attached to the correct column
//    //
//    column_id_t id(1);
//    for(auto c : f_columns_by_name)
//    {
//        if(c.second->get_id() != 0)
//        {
//            continue;
//        }
//
//        while(f_columns_by_id.find(id) != f_columns_by_id.end())
//        {
//            ++id;
//        }
//
//        c.second->set_id(id);
//        f_columns_by_id[id] = c.second;
//        ++id;
//    }
//
//    // the identifiers can now be used to define the row keys
//    //
//    for(auto const & id : f_primary_key)
//    {
//        schema_column::pointer_t c(get_column(id));
//        if(c == nullptr)
//        {
//            throw column_not_found(
//                      "all columns referenced in primary_key=... of table \""
//                    + f_name
//                    + "\" must exist. We could not find a column with identifier \""
//                    + std::to_string(id)
//                    + "\".");
//        }
//        if(c->get_name() == g_oid_column)
//        {
//            throw invalid_parameter(
//                      "the \""
//                    + c->get_name()
//                    + "\" column (with id: "
//                    + std::to_string(id)
//                    + ") is not acceptable as part of the primary key.");
//        }
//    }
//
//    // and the secondary indexes can also be defined
//    //
//    for(auto const & index : f_secondary_indexes)
//    {
//        size_t const max(index.second->get_column_count());
//        for(size_t idx(0); idx < max; ++idx)
//        {
//            schema_sort_column::pointer_t sc(index.second->get_sort_column(idx));
//            std::string const n(sc->get_column_name());
//            schema_column::pointer_t c(column(n));
//            if(c == nullptr)
//            {
//                throw invalid_xml(
//                          "a column referenced in the secondary-index of table \""
//                        + f_name
//                        + "\" must exist. We could not find \""
//                        + f_name
//                        + "."
//                        + n
//                        + "\".");
//            }
//            if(c->get_id() == 0)
//            {
//                throw logic_error(
//                          "somehow column \""
//                        + f_name
//                        + "."
//                        + n
//                        + "\" still has no identifier.");
//            }
//            sc->set_id(c->get_id());
//        }
//    }
//}


/** \brief Whether this schema includes an expiration date.
 *
 * The "expiration_date" column is used to expire a row. If the date in that
 * column is less than `now` then the row is considered expired. The row will
 * not be returned to you and will eventually get removed from the database
 * by one of our backend processes.
 *
 * The "expiration_date" is optional and in most cases not defined. This
 * function returns true if that table has that column.
 *
 * \return true if the table has an expiration date column.
 */
bool schema_table::has_expiration_date_column() const
{
    return f_columns_by_name.find(g_expiration_date) != f_columns_by_name.end();
}


schema_column::pointer_t schema_table::get_expiration_date_column() const
{
    return get_column(g_expiration_date);
}


schema_column::pointer_t schema_table::get_column(std::string const & name) const
{
    auto it(f_columns_by_name.find(name));
    if(it == f_columns_by_name.end())
    {
        return schema_column::pointer_t();
    }
    return it->second;
}


schema_column::pointer_t schema_table::get_column(column_id_t id) const
{
    auto it(f_columns_by_id.find(id));
    if(it == f_columns_by_id.end())
    {
        return schema_column::pointer_t();
    }
    return it->second;
}


schema_column::map_by_id_t schema_table::get_columns_by_id() const
{
    return f_columns_by_id;
}


schema_column::map_by_name_t schema_table::get_columns_by_name() const
{
    return f_columns_by_name;
}


schema_secondary_index::pointer_t schema_table::get_secondary_index(std::string const & name) const
{
    auto const & it(f_secondary_indexes.find(name));
    if(it != f_secondary_indexes.end())
    {
        return it->second;
    }

    return schema_secondary_index::pointer_t();
}


schema_complex_type::pointer_t schema_table::get_complex_type(std::string const & name) const
{
    auto const & it(f_complex_types->find(name));
    if(it != f_complex_types->end())
    {
        return it->second;
    }

    return schema_complex_type::pointer_t();
}


std::string schema_table::get_description() const
{
    return f_description;
}


char const * get_expiration_date_column_name()
{
    return g_expiration_date;
}



} // namespace prinbee
// vim: ts=4 sw=4 et
