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
#include    "prinbee/names.h"
#include    "prinbee/utils.h"


// snaplogger
//
#include    <snaplogger/message.h>


// snapdev
//
#include    <snapdev/file_contents.h>
#include    <snapdev/to_string_literal.h>
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



// one file in a complex type definition
struct_description_t g_complex_enum_field_description[] =
{
    define_description( // name of the enum value
          FieldName(g_name_prinbee_fld_name)
        , FieldType(struct_type_t::STRUCT_TYPE_P8STRING)
    ),
    define_description( // the enum value
          FieldName(g_name_prinbee_fld_value)
        , FieldType(struct_type_t::STRUCT_TYPE_UINT64)
    ),
    end_descriptions()
};

// one complex enum definition
//
// Note: at the moment I do not see the need for a validation script
//       on an enumeration number since one is in pretty good control
//       of the enumeration values; the compare script can be used to
//       change the order (so not follow the integer value)
//
struct_description_t g_complex_enum_description[] =
{
    define_description( // name of the type
          FieldName(g_name_prinbee_fld_name)
        , FieldType(struct_type_t::STRUCT_TYPE_P8STRING)
    ),
    define_description( // human description
          FieldName(g_name_prinbee_fld_description)
        , FieldType(struct_type_t::STRUCT_TYPE_P32STRING)
    ),
    define_description( // compare script (as2js script as a string; returns -1, 0, 1)
          FieldName(g_name_prinbee_fld_compare_script)
        , FieldType(struct_type_t::STRUCT_TYPE_P32STRING)
    ),
    define_description( // if not VOID, type of enumeration (defaults to UINT16 if user does not define)
          FieldName(g_name_prinbee_fld_enum_type)
        , FieldType(struct_type_t::STRUCT_TYPE_UINT16)
        , FieldDefaultValue("VOID")
    ),
    define_description(
          FieldName(g_name_prinbee_fld_values)
        , FieldType(struct_type_t::STRUCT_TYPE_ARRAY16)
        , prinbee::FieldSubDescription(g_complex_enum_field_description)
    ),
    end_descriptions()
};

// one field in a complex type definition defining the field name & type
struct_description_t g_complex_type_field_description[] =
{
    define_description( // name of the field
          FieldName(g_name_prinbee_fld_name)
        , FieldType(struct_type_t::STRUCT_TYPE_P8STRING)
    ),
    define_description( // type of this field
          FieldName(g_name_prinbee_fld_type)
        , FieldType(struct_type_t::STRUCT_TYPE_P8STRING)
        , FieldDefaultValue("VOID")
    ),
    end_descriptions()
};

// one complex type definition
struct_description_t g_complex_type_description[] =
{
    define_description( // name of the type
          FieldName(g_name_prinbee_fld_name)
        , FieldType(struct_type_t::STRUCT_TYPE_P8STRING)
    ),
    define_description( // human description
          FieldName(g_name_prinbee_fld_description)
        , FieldType(struct_type_t::STRUCT_TYPE_P32STRING)
    ),
    define_description( // compare script (as2js script as a string; returns -1, 0, 1)
          FieldName(g_name_prinbee_fld_compare_script)
        , FieldType(struct_type_t::STRUCT_TYPE_P32STRING)
    ),
    define_description( // validation script (as2js script as a string; returns true, false)
          FieldName(g_name_prinbee_fld_validation_script)
        , FieldType(struct_type_t::STRUCT_TYPE_P32STRING)
    ),
    define_description(
          FieldName(g_name_prinbee_fld_fields)
        , FieldType(struct_type_t::STRUCT_TYPE_ARRAY16)
        , prinbee::FieldSubDescription(g_complex_type_field_description)
    ),
    end_descriptions()
};

// complex type definition, with all the complex types
struct_description_t g_complex_type_file_description[] =
{
    prinbee::define_description(
          prinbee::FieldName(g_system_field_name_magic)
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_MAGIC)
        , prinbee::FieldDefaultValue(prinbee::to_string(prinbee::dbtype_t::FILE_TYPE_COMPLEX_TYPE))
    ),
    prinbee::define_description(
          prinbee::FieldName(g_system_field_name_structure_version)
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_STRUCTURE_VERSION)
        , prinbee::FieldVersion(1, 0)
    ),
    prinbee::define_description(
          prinbee::FieldName(g_name_prinbee_fld_types)
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_ARRAY16)
        , prinbee::FieldSubDescription(g_complex_type_description)
    ),
    prinbee::define_description(
          prinbee::FieldName(g_name_prinbee_fld_enums)
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_ARRAY16)
        , prinbee::FieldSubDescription(g_complex_enum_description)
    ),
    end_descriptions()
};




struct_description_t g_column_description[] =
{
    define_description(
          FieldName(g_name_prinbee_fld_name)
        , FieldType(struct_type_t::STRUCT_TYPE_P8STRING)
    ),
    define_description(
          FieldName(g_name_prinbee_fld_column_id)
        , FieldType(struct_type_t::STRUCT_TYPE_UINT16)
    ),
    define_description(
          FieldName(g_name_prinbee_fld_type)
        , FieldType(struct_type_t::STRUCT_TYPE_UINT16)
    ),
    define_description(
            // "encrypted" -- whether this column is encrypted (and thus cannot
            //                be part of an index); use table ENCRYPTION KEY
            //
            // "limited" -- whether to display the entire column or not
            //
            // "required" -- NOT NULL
            //
            // "hidden" -- HIDDEN (clear using VISIBLE; whether the column
            //             is automatically included in results)
            // "blob" -- whether the data is part of the blob or not
            //
            // "system" -- column is a system column (TBD: the name start with
            //             '_' so we do not need to also have a flag?)
            //
            // "unique" -- column does not support duplicates
            //
            // "nulls" -- NULL for the purpose:
            //
            //            0 -- NULLS DISTINCT (default)
            //            1 -- NULLS NOT DISTINCT
            //            2 -- WITHOUT NULLS
            //
            // "revision_type" as defined in header is (which I think
            // represents how to handle a new revision, opposed to what
            // type of revisions are allowed in this column):
            //      global
            //      branch
            //      revision
            //
            // but maybe we need: OVERWRITTEN | VERSIONED [AND TRANSLATABLE] | TRANSLATABLE
            //   0 -- overwritten
            //   1 -- versioned
            //   2 -- translatable
            //   3 -- versioned & translatable
            //
          FieldName("flags=encrypted/limited/required/hidden/blob/system/revision_type:2/unique")
        , FieldType(struct_type_t::STRUCT_TYPE_BITS32)
    ),
    define_description( // DEFAULT <expression> (i.e. a script)
          FieldName(g_name_prinbee_fld_default_value)
        , FieldType(struct_type_t::STRUCT_TYPE_P32STRING)
    ),
    define_description(
          FieldName(g_name_prinbee_fld_minimum_value)
        , FieldType(struct_type_t::STRUCT_TYPE_BUFFER32)
    ),
    define_description(
          FieldName(g_name_prinbee_fld_maximum_value)
        , FieldType(struct_type_t::STRUCT_TYPE_BUFFER32)
    ),
    define_description(
          FieldName(g_name_prinbee_fld_minimum_length)
        , FieldType(struct_type_t::STRUCT_TYPE_UINT32)
    ),
    define_description(
          FieldName(g_name_prinbee_fld_maximum_length)
        , FieldType(struct_type_t::STRUCT_TYPE_UINT32)
    ),
    define_description( // CHECK (<expression>)
          FieldName(g_name_prinbee_fld_validation_script)
        , FieldType(struct_type_t::STRUCT_TYPE_P32STRING)
    ),
    end_descriptions()
};


struct_description_t g_column_reference[] =
{
    // Note: the PRIMARY KEY (column1, ..., columnN) does not offer a length
    //       parameter at the moment
    //
    define_description(
          FieldName(g_name_prinbee_fld_column_id)
        , FieldType(struct_type_t::STRUCT_TYPE_UINT16)
    ),
    end_descriptions()
};


struct_description_t g_sort_column[] =
{
    define_description( // the column identifier or 0 for expressions
          FieldName(g_name_prinbee_fld_column_id)
        , FieldType(struct_type_t::STRUCT_TYPE_UINT16)
    ),
    define_description(
            // "descending" -- flip the order for that one column
            //
            // "nulls" -- how to handle nulls:
            //
            //     0 - nulls are placed first (default -- NULLS FIRST)
            //     1 - nulls are placed last (NULLS LAST)
            //     2 - nulls are not included (WITHOUT NULLS)
            //
          FieldName("flags=descending/nulls:2")
        , FieldType(struct_type_t::STRUCT_TYPE_BITS32)
    ),
    define_description( // if column is a "buffer" or "string"
          FieldName(g_name_prinbee_fld_length)
        , FieldType(struct_type_t::STRUCT_TYPE_UINT32)
        , FieldDefaultValue(snapdev::integer_to_string_literal<SCHEMA_SORT_COLUMN_DEFAULT_LENGTH>)
    ),
    define_description( // for: ( <expression> ) [column_id is 0 when this is defined and vice versa]
          FieldName(g_name_prinbee_fld_key_script)
        , FieldType(struct_type_t::STRUCT_TYPE_P32STRING)
    ),
    end_descriptions()
};


struct_description_t g_table_secondary_index[] =
{
    define_description(
          FieldName(g_name_prinbee_fld_name)
        , FieldType(struct_type_t::STRUCT_TYPE_P8STRING)
    ),
    define_description(
            // "distributed" -- each server only handle a partial index
            //                  (from some determined low bound to a
            //                  determined high bound)
            //
            // "unique" -- column does not support duplicates
            //
            // "nulls" -- NULLS DISTINCT (0), NULLS NOT DISTINCT (1)
            //            (useful if "unique = 1")
            //
          FieldName("flags=distributed/unique/nulls")
        , FieldType(struct_type_t::STRUCT_TYPE_BITS32)
    ),
    define_description(
          FieldName(g_name_prinbee_fld_sort_columns)
        , FieldType(struct_type_t::STRUCT_TYPE_ARRAY16)
        , FieldSubDescription(g_sort_column)
    ),
    define_description( // WHERE <expression>
          FieldName(g_name_prinbee_fld_filter_script)
        , FieldType(struct_type_t::STRUCT_TYPE_P32STRING)
    ),
    define_description( // COMMENT <description>
          FieldName(g_name_prinbee_fld_description)
        , FieldType(struct_type_t::STRUCT_TYPE_P32STRING)
    ),
    end_descriptions()
};


struct_description_t g_table_description[] =
{
    prinbee::define_description(
          prinbee::FieldName(g_system_field_name_magic)
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_MAGIC)
        , prinbee::FieldDefaultValue(prinbee::to_string(prinbee::dbtype_t::FILE_TYPE_SCHEMA))
    ),
    prinbee::define_description(
          prinbee::FieldName(g_system_field_name_structure_version)
        , prinbee::FieldType(prinbee::struct_type_t::STRUCT_TYPE_STRUCTURE_VERSION)
        , prinbee::FieldVersion(1, 0)
    ),
    define_description(
          FieldName(g_name_prinbee_fld_schema_version)
        , FieldType(struct_type_t::STRUCT_TYPE_UINT32) // just a number from 1 to ~4 billion
    ),
    define_description(
          FieldName(g_name_prinbee_fld_created_on)
        , FieldType(struct_type_t::STRUCT_TYPE_NSTIME)
    ),
    define_description(
          FieldName(g_name_prinbee_fld_last_updated_on)
        , FieldType(struct_type_t::STRUCT_TYPE_NSTIME)
    ),
    define_description(
          FieldName(g_name_prinbee_fld_name)
        , FieldType(struct_type_t::STRUCT_TYPE_P8STRING)
    ),
    define_description(
          // For now, I removed the "temporary" flag because I do not see how
          // to implement it nor how it would be used
          //
          FieldName("flags=logged/secure/translatable")
        , FieldType(struct_type_t::STRUCT_TYPE_BITS64)
    ),
    define_description(
          FieldName(g_name_prinbee_fld_model)
        , FieldType(struct_type_t::STRUCT_TYPE_UINT8)
    ),
    define_description(
          FieldName(g_name_prinbee_fld_replication)
        , FieldType(struct_type_t::STRUCT_TYPE_UINT8)
        , FieldDefaultValue("1")
    ),
    define_description(
          FieldName(g_name_prinbee_fld_description)
        , FieldType(struct_type_t::STRUCT_TYPE_P32STRING)
    ),
    define_description(
          FieldName(g_name_prinbee_fld_versioned_rows)
        , FieldType(struct_type_t::STRUCT_TYPE_VERSION) // max. number of major/minor versions (i.e. delete older versions when we reach these numbers)
    ),
    define_description(
          FieldName(g_name_prinbee_fld_blob_limit)
        , FieldType(struct_type_t::STRUCT_TYPE_UINT32) // size of blob at which we compress the data
    ),
    define_description(
          FieldName(g_name_prinbee_fld_blob_compressor)
        , FieldType(struct_type_t::STRUCT_TYPE_P8STRING) // compressor used to compress/decompress the blob
        , FieldDefaultValue("xz")
    ),
    define_description(
          FieldName(g_name_prinbee_fld_inline_limit)
        , FieldType(struct_type_t::STRUCT_TYPE_UINT32) // size of field before it gets saved to an external file
    ),
    define_description(
          FieldName(g_name_prinbee_fld_external_file_compressor)
        , FieldType(struct_type_t::STRUCT_TYPE_P8STRING) // compressor used to compress/decompress external files
        , FieldDefaultValue("xz")
    ),
    define_description( // ENCRYPTION KEY <name>
          FieldName(g_name_prinbee_fld_encrypt_key_name)
        , FieldType(struct_type_t::STRUCT_TYPE_P16STRING)
    ),
    define_description(
          FieldName(g_name_prinbee_fld_columns)
        , FieldType(struct_type_t::STRUCT_TYPE_ARRAY16)
        , FieldSubDescription(g_column_description)
    ),
    define_description(
          FieldName(g_name_prinbee_fld_primary_key)
        , FieldType(struct_type_t::STRUCT_TYPE_ARRAY16)
        , FieldSubDescription(g_column_reference)
    ),
    define_description(
          FieldName(g_name_prinbee_fld_secondary_indexes)
        , FieldType(struct_type_t::STRUCT_TYPE_ARRAY16)
        , FieldSubDescription(g_table_secondary_index)
    ),
    end_descriptions()
};



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






/** \brief Initialize a complex type from a .pb file.
 *
 * This function initializes a complex type definition from a structure.
 *
 * Once in a list of columns, a complex type becomes a
 * `STRUCT_TYPE_STRUCTURE`.
 *
 * \param[in] s  the structure from the .pb file with this complex
 *               type definition.
 * \param[in] is_enum  whether this is an enumeration (true) or not (false).
 */
schema_complex_type::schema_complex_type(structure::pointer_t s, bool is_enum)
{
    // first read common fields
    //
    f_name = s->get_string(g_name_prinbee_fld_name);
    if(name_to_struct_type(f_name) != INVALID_STRUCT_TYPE)
    {
        throw type_mismatch(
                  "the name of a complex type cannot be the name of a basic type; \""
                + f_name
                + "\" is not considered valid.");
    }
    f_description = s->get_string(g_name_prinbee_fld_description);
    f_compare_script = s->get_string(g_name_prinbee_fld_compare_script);

    if(is_enum)
    {
        // an enumeration has a type and a list of name=values
        //
        f_enum_type = name_to_struct_type(s->get_string(g_name_prinbee_fld_type));

        for(auto const & v : std::const_pointer_cast<structure>(s)->get_array(g_name_prinbee_fld_values))
        {
            field_t enum_field;
            enum_field.f_name = v->get_string(g_name_prinbee_fld_name);
            enum_field.f_enum_value = v->get_uinteger(g_name_prinbee_fld_value);

            // f_fields is a vector since we need to keep the fields in the
            // order the user defined them (do we, actually?)
            //
            auto const it_name(std::find_if(
                  f_fields.begin()
                , f_fields.end()
                , [&enum_field](field_t const & f)
                {
                    return enum_field.f_name == f.f_name;
                }));
            if(it_name != f_fields.end())
            {
                throw defined_twice(
                          "each name in an enum definition must be unique, found \""
                        + enum_field.f_name
                        + "\" twice.");
            }

            // TBD: do we want the values of an enum to be unique? at the moment
            //      I am thinking that yes and we are not offering the user to
            //      set the value anyway... (although we may, long term,
            //      want to allow the user to be able to define the value)
            //
            auto const it_value(std::find_if(
                  f_fields.begin()
                , f_fields.end()
                , [&enum_field](field_t const & f)
                {
                    return enum_field.f_enum_value == f.f_enum_value;
                }));
            if(it_value != f_fields.end())
            {
                throw defined_twice(
                          "each value in an enum definition must be unique, found \""
                        + std::to_string(enum_field.f_enum_value)
                        + "\" twice in \""
                        + it_value->f_name
                        + "\" and \""
                        + enum_field.f_name
                        + "\".");
            }

            f_fields.push_back(enum_field);
        }
    }
    else
    {
        // a type also has a validation script
        //
        f_validation_script = s->get_string(g_name_prinbee_fld_validation_script);

        for(auto const & f : std::const_pointer_cast<structure>(s)->get_array(g_name_prinbee_fld_fields))
        {
            // we do not yet have all the complex types so we cannot verify
            // their existence just yet (or whether a loop exists)
            //
            field_t type_field;
            type_field.f_name = f->get_string(g_name_prinbee_fld_name);
            type_field.f_type_name = f->get_uinteger(g_name_prinbee_fld_type);

            // f_fields is a vector since we need to keep the fields in the
            // order the user defined them (do we, actually?)
            //
            auto const it(std::find_if(
                  f_fields.begin()
                , f_fields.end()
                , [&type_field](field_t const & field)
                {
                    return type_field.f_name == field.f_name;
                }));
            if(it != f_fields.end())
            {
                throw defined_twice(
                          "each field name in a complex type must be unique, found \""
                        + type_field.f_name
                        + "\" twice.");
            }

            f_fields.push_back(type_field);
        }
    }
}


std::string schema_complex_type::get_name() const
{
    return f_name;
}


bool schema_complex_type::is_enum() const
{
    return f_enum_type != struct_type_t::STRUCT_TYPE_VOID;
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


void schema_complex_type::load_complex_types(map_pointer_t complex_types, virtual_buffer::pointer_t b)
{
    if(complex_types == nullptr)
    {
        throw logic_error("load_complex_types() called with a null complex_types pointer.");
    }

    structure::pointer_t s(std::make_shared<prinbee::structure>(g_complex_type_file_description));
    s->set_virtual_buffer(b, 0);

    // we have two arrays to go through, the list of types (TYPE)
    // and enumerations (TYPE AS ENUM)
    //
    for(auto const & t : std::const_pointer_cast<structure>(s)->get_array(g_name_prinbee_fld_types))
    {
        schema_complex_type::pointer_t user_type(std::make_shared<schema_complex_type>(t, false));
        (*complex_types)[user_type->get_name()] = user_type;
    }

    for(auto const & e : std::const_pointer_cast<structure>(s)->get_array(g_name_prinbee_fld_enums))
    {
        schema_complex_type::pointer_t user_enum(std::make_shared<schema_complex_type>(e, true));
        (*complex_types)[user_enum->get_name()] = user_enum;
    }

    // now setup all the "field.f_type" fields properly
    //
    // the load above does not set that field because the
    // `f_complex_types->contains()` could fail unless we
    // load all the complex types first
    //
    for(auto & type : *complex_types)
    {
        if(type.second->is_enum())
        {
            continue;
        }

        std::size_t const size(type.second->get_size());
        for(std::size_t idx(0); idx < size; ++idx)
        {
            std::string const name(type.second->get_type_name(idx));
            struct_type_t const struct_type(name_to_struct_type(name));
            if(struct_type != INVALID_STRUCT_TYPE)
            {
                type.second->set_type(idx, struct_type);
            }
            else if(complex_types->contains(name))
            {
                // the type is a known complex type
                //
                type.second->set_type(idx, struct_type_t::STRUCT_TYPE_STRUCTURE);
            }
            else
            {
                throw type_not_found(
                        "basic or complex type named \""
                      + name
                      + "\" not known.");
            }
        }
    }
}








schema_column::schema_column(schema_table::pointer_t table)
    : f_schema_table(table)
{
}


//schema_column::schema_column(
//          schema_table::pointer_t table
//        , advgetopt::conf_file::pointer_t config
//        , std::string const & column_id)
//    : f_schema_table(table)
//{
//    f_id = convert_to_uint(column_id, 16);
//    if(f_id == 0)
//    {
//        throw invalid_number(
//                  "a column identifier must be a 16 bit number except 0; \""
//                + column_id
//                + "\" is not valid.");
//    }
//
//    std::string const section_name(g_column_scope + column_id);
//
//    std::string const column_field_name(section_name + "::name");
//    if(!config->has_parameter(column_field_name))
//    {
//        throw missing_parameter(
//                  "a \"name=...\" is mandatory for a column definition (id: "
//                + column_id
//                + ").");
//    }
//    f_name = config->get_parameter(column_field_name);
//    if(!validate_name(f_name.c_str()))
//    {
//        throw invalid_name(
//                  "\""
//                + f_name
//                + "\" is not a valid column name.");
//    }
//    if(f_name[0] == '_')
//    {
//        throw invalid_name(
//                  "a user defined column name (\""
//                + f_name
//                + "\") cannot start with an underscore. This is reserved for system columns.");
//    }
//
//    std::string const column_field_type(section_name + "::type");
//    if(!config->has_parameter(column_field_type))
//    {
//        throw missing_parameter(
//                  "a \"type=...\" is mandatory in your \""
//                + f_name
//                + "\" column definition.");
//    }
//    std::string const & type_name(config->get_parameter(column_field_type));
//    f_type = name_to_struct_type(type_name);
//    if(f_type == INVALID_STRUCT_TYPE)
//    {
//        schema_complex_type::pointer_t ct(table->get_complex_type(type_name));
//        if(ct == nullptr)
//        {
//            throw invalid_type(
//                      "found unknown type \""
//                    + type_name
//                    + "\" in your \""
//                    + f_name
//                    + "\" column definition.");
//        }
//
//        // TODO: actually implement the complex type
//        //       (at this time, I'm thinking that the way to do it is
//        //       to use snapdev/brs.h and save each field that way in
//        //       one binary blob)
//        //
//        throw not_yet_implemented(
//                "full support for complex types not yet implemented");
//    }
//
//    // if the user defined an expiration date column, make sure it uses
//    // the correct type otherwise that's a bug and it needs to be fixed
//    //
//    if(is_expiration_date_column())
//    {
//        switch(f_type)
//        {
//        case struct_type_t::STRUCT_TYPE_TIME:
//        case struct_type_t::STRUCT_TYPE_MSTIME:
//        case struct_type_t::STRUCT_TYPE_USTIME:
//        case struct_type_t::STRUCT_TYPE_NSTIME:
//            break;
//
//        default:
//            throw type_mismatch(
//                    "the \"expiration_date\" column must be assigned a  valid time type (TIME, MSTIME, USTIME, NSTYPE); "
//                  + to_string(f_type)
//                  + " is not valid.");
//
//        }
//    }
//
//    f_flags = 0;
//    std::string const column_field_flags(section_name + "::flags");
//    if(config->has_parameter(column_field_flags))
//    {
//        std::list<std::string> flags;
//        snapdev::tokenize_string(flags, config->get_parameter(column_field_flags), { "," }, true);
//
//        for(auto const & f : flags)
//        {
//            if(f == "blob")
//            {
//                f_flags |= COLUMN_FLAG_BLOB;
//            }
//            else if(f == "hidden")
//            {
//                f_flags |= COLUMN_FLAG_HIDDEN;
//            }
//            else if(f == "limited")
//            {
//                // limit display of this column by default because it
//                // could be really large
//                //
//                f_flags |= COLUMN_FLAG_LIMITED;
//            }
//            else if(f == "required")
//            {
//                // i.e. column cannot be set to NULL; note that if a default
//                //      exists then the column can be undefined and the
//                //      default value gets used instead
//                //
//                f_flags |= COLUMN_FLAG_REQUIRED;
//            }
//            else if(f == "versioned")
//            {
//                // only versioned column generate a new row when updated
//                //
//                f_flags |= COLUMN_FLAG_VERSIONED;
//            }
//            else
//            {
//                throw invalid_name(
//                          "column \""
//                        + f_name
//                        + "\" includes an unknown flag: \""
//                        + f
//                        + "\".");
//            }
//        }
//    }
//
//    std::string const column_field_encrypt(section_name + "::encrypt");
//    f_encrypt_key_name = config->get_parameter(column_field_encrypt);
//
//    std::string const column_field_description(section_name + "::description");
//    f_description = config->get_parameter(column_field_description);
//
//    std::string const column_field_default_value(section_name + "::default_value");
//    if(config->has_parameter(column_field_default_value))
//    {
//        f_default_value = string_to_typed_buffer(f_type, config->get_parameter(column_field_default_value));
//    }
//
//    std::string const column_field_default_value_script(section_name + "::default_value_script");
//    std::string const default_script(config->get_parameter(column_field_default_value_script));
//    if(!default_script.empty())
//    {
//        // TODO: compile that script with as2js and save that in memory
//        //
//        //f_default_value_script = parse(default_script);
//        throw not_yet_implemented("code -> f_default_value_script not yet implemented");
//    }
//
//    std::string const column_field_validation_script(section_name + "::validation_script");
//    std::string code(config->get_parameter(column_field_validation_script));
//    if(!code.empty())
//    {
//        // TODO: compile that script with as2js and save that in memory
//        //
//        //f_validation = parse(code);
//        throw not_yet_implemented("code -> f_validation not yet implemented");
//    }
//
//    std::string const column_field_minimum_value(section_name + "::minimum_value");
//    if(config->has_parameter(column_field_minimum_value))
//    {
//        f_minimum_value = string_to_typed_buffer(f_type, config->get_parameter(column_field_minimum_value));
//    }
//
//    std::string const column_field_maximum_value(section_name + "::maximum_value");
//    if(config->has_parameter(column_field_maximum_value))
//    {
//        f_maximum_value = string_to_typed_buffer(f_type, config->get_parameter(column_field_maximum_value));
//    }
//
//    std::string const column_field_minimum_size(section_name + "::minimum_size");
//    if(config->has_parameter(column_field_minimum_size))
//    {
//        f_minimum_size = convert_to_uint(config->get_parameter(column_field_minimum_size), 32);
//    }
//
//    std::string const column_field_maximum_size(section_name + "::maximum_size");
//    if(config->has_parameter(column_field_maximum_size))
//    {
//        f_maximum_size = convert_to_uint(config->get_parameter(column_field_maximum_size), 32);
//    }
//
//    std::string const column_field_internal_size_limit(section_name + "::internal_size_limit");
//    if(config->has_parameter(column_field_internal_size_limit))
//    {
//        f_internal_size_limit = convert_to_int(config->get_parameter(column_field_internal_size_limit), 32);
//        if(f_internal_size_limit != -1 && f_internal_size_limit < 128)
//        {
//            throw type_mismatch(
//                      "column \""
//                    + table->get_name()
//                    + "::column"
//                    + column_id
//                    + "::internal_size_limit\" has an invalid external size limit \""
//                    + config->get_parameter(column_field_internal_size_limit)
//                    + "\" value (not a number or too small).");
//        }
//    }
//}


// we should be able to do this using set_...() functions whenever we create
// a new column -- i.e. on a CREATE TABLE or ALTER TABLE
//schema_column::schema_column(
//              schema_table_pointer_t table
//            , std::string const & name
//            , struct_type_t type
//            , flag32_t flags)
//    : f_name(name)
//    , f_type(type)
//    , f_flags(flags)
//    , f_schema_table(table)
//{
//    if((flags & COLUMN_FLAG_SYSTEM) == 0)
//    {
//        throw logic_error("only system columns can be created using this constructor.");
//    }
//
//    if(f_name.length() < 2
//    || f_name[0] != '_')
//    {
//        throw logic_error("all system column names must start with an underscore.");
//    }
//}


void schema_column::from_binary(structure::pointer_t s)
{
    f_structure = s;

    f_name = f_structure->get_string(g_name_prinbee_fld_name);
    f_column_id = f_structure->get_uinteger(g_name_prinbee_fld_column_id);
    f_type = static_cast<struct_type_t>(f_structure->get_uinteger(g_name_prinbee_fld_type));
    //f_flags = f_structure->get_uinteger(g_name_prinbee_fld_flags); -- keep flags in structure
    //f_encrypt_key_name = f_structure->get_string(g_name_prinbee_fld_encrypt_key_name); -- moved to table
    //f_internal_size_limit = f_structure->get_integer(g_name_prinbee_fld_internal_size_limit); -- moved to table
    f_default_value = f_structure->get_buffer(g_name_prinbee_fld_default_value);
    f_minimum_value = f_structure->get_buffer(g_name_prinbee_fld_minimum_value);
    f_maximum_value = f_structure->get_buffer(g_name_prinbee_fld_maximum_value);
    f_minimum_length = f_structure->get_uinteger(g_name_prinbee_fld_minimum_length);
    f_maximum_length = f_structure->get_uinteger(g_name_prinbee_fld_maximum_length);
    f_validation_script = f_structure->get_string(g_name_prinbee_fld_validation_script);
    f_description = f_structure->get_string(g_name_prinbee_fld_description);
}


/** \brief Return true if this column represents the "expiration_date" column.
 *
 * This function checks the name of the column. If the name is
 * "expiration_date", then the function returns true.
 *
 * \note
 * The column is viewed as a user column but the system has intelligence
 * to consider the data of a row as out of date when NOW() > expiration_date
 * which makes a lot of things much more efficient.
 *
 * \return true when the column name is "expiration_date".
 */
bool schema_column::is_expiration_date_column() const
{
    return f_name == g_expiration_date;
}


//compare_t schema_column::compare(schema_column const & rhs) const
//{
//    compare_t result(compare_t::COMPARE_SCHEMA_EQUAL);
//
//    if(f_name != rhs.f_name)
//    {
//        throw logic_error(
//                  "the schema_column::compare() function can only be called"
//                  " with two columns having the same name. You called it"
//                  " with a column named \""
//                + f_name
//                + "\" and the other \""
//                + rhs.f_name
//                + "\".");
//    }
//
//    //f_column_id -- these are adjusted accordingly on a merge
//
//    if(f_type != rhs.f_type)
//    {
//        return compare_t::COMPARE_SCHEMA_DIFFER;
//    }
//
//    // the LIMITED flag is just a display flag, it's really not important
//    // still request for an update if changed by the end user
//    //
//    flag32_t const lhs_flags(f_structure->get_uinteger(g_name_prinbee_fld_flags));
//    flag32_t const rhs_flags(rhs.f_structure->get_uinteger(g_name_prinbee_fld_flags));
//    if((lhs_flags & ~COLUMN_FLAG_LIMITED) != (rhs_flags & ~COLUMN_FLAG_LIMITED))
//    {
//        return compare_t::COMPARE_SCHEMA_DIFFER;
//    }
//    if(lhs_flags != rhs_flags)
//    {
//        result = compare_t::COMPARE_SCHEMA_UPDATE;
//    }
//
//    if(f_encrypt_key_name != rhs.f_encrypt_key_name)
//    {
//        return compare_t::COMPARE_SCHEMA_DIFFER;
//    }
//
//    if(f_default_value != rhs.f_default_value)
//    {
//        result = compare_t::COMPARE_SCHEMA_UPDATE;
//    }
//
//    if(f_minimum_value != rhs.f_minimum_value)
//    {
//        return compare_t::COMPARE_SCHEMA_DIFFER;
//    }
//
//    if(f_maximum_value != rhs.f_maximum_value)
//    {
//        return compare_t::COMPARE_SCHEMA_DIFFER;
//    }
//
//    if(f_minimum_size != rhs.f_minimum_size)
//    {
//        return compare_t::COMPARE_SCHEMA_DIFFER;
//    }
//
//    if(f_maximum_size != rhs.f_maximum_size)
//    {
//        return compare_t::COMPARE_SCHEMA_DIFFER;
//    }
//
//    // we can't do much better here, unfortunately
//    // but if the script changes many things can be affected
//    //
//    if(f_validation_script != rhs.f_validation_script)
//    {
//        return compare_t::COMPARE_SCHEMA_DIFFER;
//    }
//
//    return result;
//}


schema_table::pointer_t schema_column::get_schema_table() const
{
    return f_schema_table.lock();
}


column_id_t schema_column::get_column_id() const
{
    return f_column_id;
}


void schema_column::set_column_id(column_id_t id)
{
    if(f_column_id != COLUMN_NULL)
    {
        throw id_already_assigned(
                  "this column already has an identifier ("
                + std::to_string(static_cast<int>(f_column_id))
                + ").");
    }
    if(id == COLUMN_NULL)
    {
        throw invalid_number("a column identifier cannot be set to NULL.");
    }

    f_column_id = id;
    f_structure->set_uinteger(g_name_prinbee_fld_column_id, id);
    get_schema_table()->modified();
}


std::string schema_column::get_name() const
{
    return f_name;
}


void schema_column::set_name(std::string const & name)
{
    if(f_name != name)
    {
        f_name = name;
        f_structure->set_string(g_name_prinbee_fld_name, name);
        get_schema_table()->modified();
    }
}


struct_type_t schema_column::get_type() const
{
    return f_type;
}


void schema_column::set_type(struct_type_t type)
{
    if(f_type != type)
    {
        f_type = type;
        f_structure->set_uinteger(g_name_prinbee_fld_type, static_cast<int>(type));
        get_schema_table()->modified();
    }
}


buffer_t schema_column::get_default_value() const
{
    return f_default_value;
}


void schema_column::set_default_value(buffer_t const & default_value)
{
    if(f_default_value != default_value)
    {
        f_default_value = default_value;
        f_structure->set_buffer(g_name_prinbee_fld_default_value, default_value);
        get_schema_table()->modified();
    }
}


buffer_t schema_column::get_minimum_value() const
{
    return f_minimum_value;
}


void schema_column::set_minimum_value(buffer_t const & value)
{
    if(f_minimum_value != value)
    {
        f_minimum_value = value;
        f_structure->set_buffer(g_name_prinbee_fld_minimum_value, value);
        get_schema_table()->modified();
    }
}


buffer_t schema_column::get_maximum_value() const
{
    return f_maximum_value;
}


void schema_column::set_maximum_value(buffer_t const & value)
{
    if(f_maximum_value != value)
    {
        f_maximum_value = value;
        f_structure->set_buffer(g_name_prinbee_fld_maximum_value, value);
        get_schema_table()->modified();
    }
}


std::uint32_t schema_column::get_minimum_length() const
{
    return f_minimum_length;
}


void schema_column::set_minimum_length(std::uint32_t length)
{
    if(f_minimum_length != length)
    {
        f_minimum_length = length;
        f_structure->set_uinteger(g_name_prinbee_fld_minimum_length, length);
        get_schema_table()->modified();
    }
}


std::uint32_t schema_column::get_maximum_length() const
{
    return f_maximum_length;
}


void schema_column::set_maximum_length(std::uint32_t length)
{
    if(f_maximum_length != length)
    {
        f_maximum_length = length;
        f_structure->set_uinteger(g_name_prinbee_fld_maximum_length, length);
        get_schema_table()->modified();
    }
}


std::string const & schema_column::get_validation_script() const
{
    return f_validation_script;
}


void schema_column::set_validation_script(std::string const & validation_script)
{
    if(f_validation_script != validation_script)
    {
        f_validation_script = validation_script;
        f_structure->set_string(g_name_prinbee_fld_validation_script, validation_script);
        get_schema_table()->modified();
    }
}


std::string const & schema_column::get_description() const
{
    return f_description;
}


void schema_column::set_description(std::string const & description)
{
    if(f_description != description)
    {
        f_description = description;
        f_structure->set_string(g_name_prinbee_fld_description, description);
        get_schema_table()->modified();
    }
}














schema_sort_column::schema_sort_column(schema_table::pointer_t t)
    : f_schema_table(t)
{
}


void schema_sort_column::from_binary(structure::pointer_t s)
{
    f_structure = s;

    f_column_id = f_structure->get_uinteger(g_name_prinbee_fld_column_id);
    //f_flags = f_structure->get_uinteger(g_name_prinbee_fld_flags);
    f_length = f_structure->get_uinteger(g_name_prinbee_fld_length);
    f_key_script = f_structure->get_uinteger(g_name_prinbee_fld_key_script);
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
//void schema_sort_column::from_config(std::string const & column_definition)
//{
//    advgetopt::string_list_t parameters;
//    advgetopt::split_string(column_definition, parameters, {" "});
//
//    if(parameters.empty())
//    {
//        throw invalid_parameter("schema_sort_column::from_config() called with an empty column_definition.");
//    }
//    std::string::size_type const pos(parameters[0].find('('));
//    if(pos == std::string::npos)
//    {
//        f_column_id = convert_to_uint(parameters[0], 16);
//    }
//    else
//    {
//        f_column_id = convert_to_uint(parameters[0].substr(0, pos), 16);
//        parameters.insert(parameters.begin() + 1, parameters[0].substr(pos));
//    }
//
//    f_length = SCHEMA_SORT_COLUMN_DEFAULT_LENGTH;
//    std::size_t idx(1);
//    if(parameters.size() >= 2
//    && parameters[1][0] == '(')
//    {
//        // we have the length parameter
//        //
//        if(parameters[idx].back() != ')')
//        {
//            throw invalid_parameter("schema_sort_column::from_config() found a length parameter where the ')' is missing.");
//        }
//
//        //std::string const size(parameters[1].substr(1, parameters[1].length() - 2));
//        std::string const size(parameters[1].begin() + 1, parameters[1].end() - 1);
//        if(size.empty())
//        {
//            throw invalid_parameter("schema_sort_column::from_config() found a length parameter without an actual length.");
//        }
//
//        f_size = convert_to_uint(size, 32);
//        if(f_size <= 0)
//        {
//            throw invalid_parameter(
//                  "the length of a sort column must be at least 1. \""
//                + size
//                + "\" is not acceptable.");
//        }
//
//        idx = 2;
//    }
//
//    for(; idx < parameters.size(); ++idx)
//    {
//        if(parameters[idx] == "desc")
//        {
//            f_flags |= SCHEMA_SORT_COLUMN_DESCENDING;
//        }
//        else
//        {
//            f_flags &= ~SCHEMA_SORT_COLUMN_DESCENDING;
//        }
//        if(parameters[idx] == "nulls_last")
//        {
//            f_flags |= SCHEMA_SORT_COLUMN_PLACE_NULLS_LAST;
//        }
//        else
//        {
//            f_flags &= ~SCHEMA_SORT_COLUMN_PLACE_NULLS_LAST;
//        }
//        if(parameters[idx] == "without_nulls")
//        {
//            f_flags |= SCHEMA_SORT_COLUMN_WITHOUT_NULLS;
//        }
//        else
//        {
//            f_flags &= ~SCHEMA_SORT_COLUMN_WITHOUT_NULLS;
//        }
//    }
//
//    if((f_flags & (SCHEMA_SORT_COLUMN_PLACE_NULLS_LAST | SCHEMA_SORT_COLUMN_WITHOUT_NULLS))
//               == (SCHEMA_SORT_COLUMN_PLACE_NULLS_LAST | SCHEMA_SORT_COLUMN_WITHOUT_NULLS))
//    {
//        throw invalid_parameter(
//              "schema_sort_column::from_config() forbids the use of \"nulls_last\" and \"without_nulls\" at the same time.");
//    }
//}


schema_table::pointer_t schema_sort_column::get_schema_table() const
{
    return f_schema_table.lock();
}


//compare_t schema_sort_column::compare(schema_sort_column const & rhs) const
//{
//    compare_t result(compare_t::COMPARE_SCHEMA_EQUAL);
//
//    if(f_column_id != rhs.f_column_id)
//    {
//        return compare_t::COMPARE_SCHEMA_DIFFER;
//    }
//
//    // to compare, retrieving the entire integer is a lot faster
//    //
//    if(f_structure->get_uinteger(g_name_prinbee_fld_flags) != rhs.f_structure->get_uinteger(g_name_prinbee_fld_flags))
//    {
//        return compare_t::COMPARE_SCHEMA_DIFFER;
//    }
//
//    if(f_size != rhs.f_size)
//    {
//        return compare_t::COMPARE_SCHEMA_DIFFER;
//    }
//
//    return result;
//}


column_id_t schema_sort_column::get_column_id() const
{
    return f_column_id;
}


void schema_sort_column::set_column_id(column_id_t column_id)
{
    if(f_column_id != column_id)
    {
        f_column_id = column_id;
        f_structure->set_uinteger(g_name_prinbee_fld_column_id, column_id);
        get_schema_table()->modified();
    }
}


bool schema_sort_column::is_descending() const
{
    return f_structure->get_bits("flags.descending") != 0;
}


void schema_sort_column::set_descending(bool descending)
{
    if(is_descending() != descending)
    {
        f_structure->set_bits("flags.descending", descending ? 1 : 0);
        get_schema_table()->modified();
    }
}


bool schema_sort_column::accept_null_columns() const
{
    return f_structure->get_bits("flags.nulls") != SCHEMA_SORT_COLUMN_WITHOUT_NULLS;
}


bool schema_sort_column::place_nulls_last() const
{
    return f_structure->get_bits("flags.nulls") != SCHEMA_SORT_COLUMN_NULLS_LAST;
}


void schema_sort_column::set_nulls(int mode)
{
    if(f_structure->get_bits("flags.nulls") != static_cast<std::uint64_t>(mode))
    {
        f_structure->set_bits("flags.nulls", mode);
        get_schema_table()->modified();
    }
}


std::uint32_t schema_sort_column::get_length() const
{
    return f_length;
}


void schema_sort_column::set_length(std::uint32_t length)
{
    if(f_length != length)
    {
        f_length = length;
        f_structure->set_uinteger(g_name_prinbee_fld_length, length);
        get_schema_table()->modified();
    }
}


std::string const & schema_sort_column::get_key_script() const
{
    return f_key_script;
}


void schema_sort_column::set_key_script(std::string const & script)
{
    if(f_key_script != script)
    {
        f_key_script = script;
        f_structure->set_string(g_name_prinbee_fld_key_script, script);
        get_schema_table()->modified();
    }
}










schema_secondary_index::schema_secondary_index(schema_table::pointer_t t)
    : f_schema_table(t)
{
}


void schema_secondary_index::from_binary(structure::pointer_t s)
{
    f_structure = s;

    f_name = f_structure->get_string(g_name_prinbee_fld_name);
    //f_flags = f_structure->get_uinteger(g_name_prinbee_fld_flags);
    f_filter_script = f_structure->get_string(g_name_prinbee_fld_filter_script); // used as the sorting key if not empty
    f_description = f_structure->get_string(g_name_prinbee_fld_description);

    auto const columns_field(f_structure->get_field(g_name_prinbee_fld_sort_columns));
    auto const columns_max(columns_field->size());
    for(std::remove_const<decltype(columns_max)>::type j(0); j < columns_max; ++j)
    {
        schema_sort_column::pointer_t sc(std::make_shared<schema_sort_column>(get_schema_table()));
        sc->from_binary((*columns_field)[j]);
        f_sort_columns.push_back(sc);
    }
}


//void schema_secondary_index::from_config(
//      advgetopt::conf_file::pointer_t config
//    , std::string const & index_id)
//{
//    f_id = convert_to_uint(index_id, 32);
//    if(f_id == 0)
//    {
//        throw invalid_number(
//                  "an index identifier must be a 32 bit number except 0; \""
//                + index_id
//                + "\" is not valid.");
//    }
//
//    std::string const section_name(g_index_scope + index_id);
//
//    std::string const index_field_name(section_name + "::name");
//    if(!config->has_parameter(index_field_name))
//    {
//        throw missing_parameter(
//                  "a \"name=...\" is mandatory for an index definition (id: "
//                + index_id
//                + ").");
//    }
//    f_name = config->get_parameter(index_field_name);
//    if(!validate_name(f_name.c_str()))
//    {
//        throw invalid_name(
//                  "\""
//                + f_name
//                + "\" is not a valid index name.");
//    }
//    if(f_name[0] == '_')
//    {
//        throw invalid_name(
//                  "a user defined index name (\""
//                + f_name
//                + "\") cannot start with an underscore. This is reserved for system indexes.");
//    }
//
//    std::string const index_field_description(section_name + "::description");
//    f_description = config->get_parameter(index_field_description);
//
//    std::string const index_field_key_script(section_name + "::key_script");
//    std::string const key_script(config->get_parameter(index_field_key_script));
//    if(!key_script.empty())
//    {
//        // TODO: compile that script with as2js and save that in memory
//        //
//        //f_key_script = parse(key_script);
//        throw not_yet_implemented("code -> f_key_script not yet implemented");
//    }
//
//    std::string const index_field_filter_script(section_name + "::filter_script");
//    std::string const filter_script(config->get_parameter(index_field_filter_script));
//    if(!filter_script.empty())
//    {
//        // TODO: compile that script with as2js and save that in memory
//        //
//        //f_filter_script = parse(filter_script);
//        throw not_yet_implemented("code -> f_filter_script not yet implemented");
//    }
//
//    f_flags = 0;
//    std::string const index_field_flags(section_name + "::flags");
//    if(config->has_parameter(index_field_flags))
//    {
//        std::list<std::string> flags;
//        snapdev::tokenize_string(flags, config->get_parameter(index_field_flags), { "," }, true);
//
//        for(auto const & f : flags)
//        {
//            if(f == "without_nulls")
//            {
//                f_flags |= SECONDARY_INDEX_FLAG_WITHOUT_NULLS;
//            }
//            else if(f == "nulls_not_distinct")
//            {
//                f_flags |= SECONDARY_INDEX_FLAG_NULLS_NOT_DISTINCT;
//            }
//            else if(f == "distributed")
//            {
//                f_flags |= SECONDARY_INDEX_FLAG_DISTRIBUTED;
//            }
//            else
//            {
//                throw invalid_name(
//                          "the user defined index \""
//                        + f_name
//                        + "\" includes an unknown flag: \""
//                        + f
//                        + "\".");
//            }
//        }
//    }
//
//    std::string const index_field_columns(section_name + "::columns");
//    if(!config->has_parameter(index_field_columns))
//    {
//        throw missing_parameter(
//                  "a user defined index name (\""
//                + f_name
//                + "\") must have a columns=... parameter.");
//    }
//
//    advgetopt::string_list_t columns;
//    advgetopt::split_string(config->get_parameter(index_field_columns), columns, {","});
//    for(auto const & c : columns)
//    {
//        schema_sort_column::pointer_t sort_column(std::make_shared<schema_sort_column>());
//        sort_column->from_config(c);
//        f_sort_columns.push_back(sort_column); // vector because these are sorted by user
//    }
//}


schema_table::pointer_t schema_secondary_index::get_schema_table() const
{
    return f_schema_table.lock();
}


//compare_t schema_secondary_index::compare(schema_secondary_index const & rhs) const
//{
//    compare_t result(compare_t::COMPARE_SCHEMA_EQUAL);
//
//    if(f_name != rhs.f_name)
//    {
//        throw logic_error(
//                  "the schema_secondary_index::compare() function can only be"
//                  " called with two secondary indexes having the same name."
//                  " You called it with an index named \""
//                + f_name
//                + "\" and the other \""
//                + rhs.f_name
//                + "\".");
//    }
//
//    std::size_t const lhs_max(get_column_count());
//    std::size_t const rhs_max(rhs.get_column_count());
//    if(lhs_max != rhs_max)
//    {
//        return compare_t::COMPARE_SCHEMA_DIFFER;
//    }
//
//    for(std::size_t idx(0); idx < lhs_max; ++idx)
//    {
//        compare_t const sc_compare(f_sort_columns[idx]->compare(*rhs.f_sort_columns[idx]));
//        if(sc_compare == compare_t::COMPARE_SCHEMA_DIFFER)
//        {
//            return compare_t::COMPARE_SCHEMA_DIFFER;
//        }
//        else if(sc_compare == compare_t::COMPARE_SCHEMA_UPDATE)
//        {
//            result = compare_t::COMPARE_SCHEMA_UPDATE;
//        }
//    }
//
//    if(f_filter_script != rhs.f_filter_script)
//    {
//        return compare_t::COMPARE_SCHEMA_DIFFER;
//    }
//
//    // to compare, retrieving the entire integer is a lot faster
//    //
//    if(f_structure->get_uinteger(g_name_prinbee_fld_flags) != rhs.f_structure->get_uinteger(g_name_prinbee_fld_flags))
//    {
//        return compare_t::COMPARE_SCHEMA_DIFFER;
//    }
//
//    return result;
//}


std::string const & schema_secondary_index::get_name() const
{
    return f_name;
}


void schema_secondary_index::set_name(std::string const & name)
{
    if(f_name != name)
    {
        f_name = name;
        f_structure->set_string(g_name_prinbee_fld_name, name);
        get_schema_table()->modified();
    }
}


bool schema_secondary_index::get_distributed_index() const
{
    return f_structure->get_bits("flags.distributed");
}


void schema_secondary_index::set_distributed_index(bool distributed)
{
    if(get_distributed_index() != distributed)
    {
        f_structure->set_bits("flags.distributed", distributed ? 1 : 0);
        get_schema_table()->modified();
    }
}


bool schema_secondary_index::get_unique_index() const
{
    return f_structure->get_bits("flags.unique");
}


void schema_secondary_index::set_unique_index(bool unique)
{
    if(get_unique_index() != unique)
    {
        f_structure->set_bits("flags.unique", unique ? 1 : 0);
        get_schema_table()->modified();
    }
}


bool schema_secondary_index::get_distinct_nulls() const
{
    return f_structure->get_bits("flags.nulls") == 0;
}


void schema_secondary_index::set_distinct_nulls(bool distinct_nulls)
{
    if(get_distinct_nulls() != distinct_nulls)
    {
        f_structure->set_bits("flags.nulls", distinct_nulls ? 0 : 1);
        get_schema_table()->modified();
    }
}


std::string const & schema_secondary_index::get_filter_script() const
{
    return f_filter_script;
}


void schema_secondary_index::set_filter_script(std::string const & filter_script)
{
    if(f_filter_script != filter_script)
    {
        f_filter_script = filter_script;
        f_structure->set_string(g_name_prinbee_fld_filter_script, filter_script);
        get_schema_table()->modified();
    }
}


std::size_t schema_secondary_index::get_column_count() const
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
                + "\" (max is "
                + std::to_string(f_sort_columns.size())
                + ").");
    }

    return f_sort_columns[idx];
}


void schema_secondary_index::add_sort_column(schema_sort_column::pointer_t)
{
    throw not_yet_implemented("add_sort_column() -- need to implement that correctly...");
}












schema_table::schema_table()
    : f_structure(std::make_shared<structure>(g_table_description))
{
    f_structure->init_buffer();
}


void schema_table::set_complex_types(schema_complex_type::map_pointer_t complex_types)
{
    f_complex_types = complex_types;
}


//void schema_table::from_config(std::string const & name, std::string const & filename)
//{
//    // the list of sections are:
//    //
//    //    "table"                    from_config_load_table() & from_config_load_primary_key()
//    //    "column::<identifier>"     from_config_load_columns()
//    //    "index::<identifier>"      from_config_load_indexes()
//    //
//    // The table does not include complex type definitions; those are
//    // defined in a separate file which is read prior to loading table
//    // schemata; it is passed to the table by calling set_complex_types()
//    // prior to this call
//    //
//    if(f_complex_types == nullptr)
//    {
//        throw logic_error("from_config() called before set_complex_types().");
//    }
//
//    from_config_name(name);
//    from_config_version(filename);
//
//    advgetopt::conf_file::pointer_t config(from_config_load_config());
//    from_config_load_table(config);
//    from_config_load_columns(config);
//    from_config_load_primary_key(config);       // finishing up load_table() -- this part needs the list of columns
//    from_config_load_indexes(config);
//}
//
//
//void schema_table::from_config_name(std::string const & name)
//{
//    // Note: this is already checked in the table_impl class, but we still
//    //       want to make sure this name is valid
//    //
//    if(!validate_name(name.c_str()))
//    {
//        throw invalid_name(
//                  "\""
//                + name
//                + "\" is not a valid table name.");
//    }
//    f_name = name;
//}
//
//
//void schema_table::from_config_version(std::string const & filename)
//{
//    std::string::size_type const pos(filename.rfind('-'));
//    if(pos == std::string::npos)
//    {
//        throw type_mismatch(
//                  "expected a dash + version in the table filename \""
//                + filename
//                + "\".");
//    }
//    char const * v(filename.c_str() + pos + 1);
//    f_version = 0;
//    while(*v >= '0' && *v <= '9')
//    {
//        f_version *= 10;
//        f_version += *v - '0';
//        ++v;
//    }
//    if(strcmp(v, ".ini") != 0)
//    {
//        throw type_mismatch(
//                  "expected a dash + version followed by \".ini\" in the table filename \""
//                + filename
//                + "\".");
//    }
//    f_filename = filename;
//}
//
//
//advgetopt::conf_file::pointer_t schema_table::from_config_load_config()
//{
//    // load the .ini file
//    //
//    advgetopt::conf_file_setup setup(
//          f_filename
//        , advgetopt::line_continuation_t::line_continuation_unix
//        , advgetopt::ASSIGNMENT_OPERATOR_EQUAL
//        , advgetopt::COMMENT_SHELL
//        , advgetopt::SECTION_OPERATOR_INI_FILE | advgetopt::SECTION_OPERATOR_CPP);
//
//    return advgetopt::conf_file::get_conf_file(setup);
//}
//
//
//void schema_table::from_config_load_table(advgetopt::conf_file::pointer_t config)
//{
//#if 0
//    // I don't think this is still required; keeping for now as a reminder
//    //
//    bool const drop(x->attribute("drop") == "drop");
//    if(drop)
//    {
//        // do not ever save a table when the DROP flag is set (actually
//        // we want to delete the entire folder if it still exists!)
//        //
//        f_flags |= TABLE_FLAG_DROP;
//        return;
//    }
//#endif
//
//    // NAME
//    {
//        std::string const table_section_name("table::name");
//        if(!config->has_parameter(table_section_name))
//        {
//            throw type_mismatch("the [table] section must have a name=... parameter.");
//        }
//        if(f_name != config->get_parameter(table_section_name))
//        {
//            throw type_mismatch(
//                      "the table directory is \""
//                    + f_name
//                    + "\" and it was expected to match the [table] section name=... field which instead is set to \""
//                    + config->get_parameter(table_section_name)
//                    + "\".");
//        }
//    }
//
//    // VERSION
//    {
//        std::string const table_section_version("table::version");
//        if(!config->has_parameter(table_section_version))
//        {
//            throw type_mismatch("the [table] section must have a version=... parameter.");
//        }
//        std::string const ini_version(config->get_parameter(table_section_version));
//        if(f_version != convert_to_uint(ini_version, 32))
//        {
//            throw type_mismatch(
//                      "the table filename is \""
//                    + f_filename
//                    + "\" with version \""
//                    + std::to_string(f_version)
//                    + "\"; found version \""
//                    + ini_version
//                    + "\" in the .ini file; there is a mismatch.");
//        }
//    }
//
//    // DESCRIPTION
//    {
//        std::string const table_section_description("table::description");
//        if(config->has_parameter(table_section_description))
//        {
//            f_description = config->has_parameter(table_section_description);
//        }
//        else
//        {
//            f_description.clear();
//        }
//    }
//
//    // REPLICATION
//    {
//        std::string const table_section_replication("table::replication");
//        if(config->has_parameter(table_section_replication))
//        {
//            f_replication = convert_to_uint(config->get_parameter(table_section_replication), 8);
//            if(f_replication == 0)
//            {
//                throw type_mismatch(
//                          "table \""
//                        + f_filename
//                        + "\" has an invalid replication=... value: \""
//                        + config->get_parameter(table_section_replication)
//                        + "\". It is expected to be an integer from 1 to 255 included.");
//            }
//        }
//        else
//        {
//            f_replication = TABLE_DEFAULT_REPLICATION;
//        }
//    }
//
//    // MODEL
//    {
//        std::string const table_section_model("table::model");
//        if(config->has_parameter(table_section_model))
//        {
//            f_model = name_to_model(config->get_parameter(table_section_model));
//        }
//        else
//        {
//            f_model = model_t::TABLE_MODEL_DEFAULT;
//        }
//    }
//
//    // FLAGS
//    {
//        f_flags = flag64_t();
//        std::string const table_section_flags("table::flags");
//        if(config->has_parameter(table_section_flags))
//        {
//            advgetopt::string_list_t flags;
//            advgetopt::split_string(config->get_parameter(table_section_flags), flags, {","});
//            for(auto const & f : flags)
//            {
//                if(f == "secure")
//                {
//                    f_flags |= TABLE_FLAG_SECURE;
//                }
//                //else if(f == "temporary")
//                //{
//                //    f_flags |= TABLE_FLAG_TEMPORARY;
//                //}
//                else if(f == "translatable")
//                {
//                    f_flags |= TABLE_FLAG_TRANSLATABLE;
//                }
//                else if(f == "unlogged")
//                {
//                    f_flags |= TABLE_FLAG_UNLOGGED;
//                }
//                else
//                {
//                    throw unknown_parameter(
//                              "table \""
//                            + f_filename
//                            + "\" has an unknown flag in its flags=... value: \""
//                            + f
//                            + "\".");
//                }
//            }
//        }
//    }
//
//    // VERSIONED ROWS
//    {
//        std::string const table_section_versioned_rows("table::versioned_rows");
//        if(config->has_parameter(table_section_versioned_rows))
//        {
//            f_versioned_rows = convert_to_uint(config->get_parameter(table_section_versioned_rows), 8);
//            if(f_versioned_rows == 0)
//            {
//                throw type_mismatch(
//                          "the table \""
//                        + f_filename
//                        + "\" has an invalid versioned rows \""
//                        + config->get_parameter(table_section_versioned_rows)
//                        + "\" value (not a number or too small or too large).");
//            }
//        }
//        else
//        {
//            f_versioned_rows = 1;
//        }
//    }
//
//    // BLOB LIMIT
//    {
//        std::string const table_section_blob_limit("table::blob_limit");
//        if(config->has_parameter(table_section_blob_limit))
//        {
//            f_blob_limit = convert_to_uint(config->get_parameter(table_section_blob_limit), 32);
//            if(f_blob_limit != 0 && f_blob_limit < 128)
//            {
//                throw type_mismatch(
//                          "the table \""
//                        + f_filename
//                        + "\" has an invalid blob limit \""
//                        + config->get_parameter(table_section_blob_limit)
//                        + "\" value (not a number or too small).");
//            }
//        }
//        else
//        {
//            f_blob_limit = 0;
//        }
//    }
//}
//
//
//void schema_table::from_config_load_columns(advgetopt::conf_file::pointer_t config)
//{
//    // 2. add system columns (i.e. those columns do not appear in the
//    //    .ini files)
//    //
//    //    _schema_version
//    //    _oid
//    //    _created_on
//    //    _created_by
//    //    _last_updated
//    //    _last_updated_by
//    //    _deleted_on
//    //    _deleted_by
//    //    _version
//    //    _language
//    //
//    // _current_version -- this is actually a row parameter... and we are
//    //                     expected to have at least 2 such parameters:
//    //                     the "current version" presented to the user and
//    //                     the "current default version"; on top of that,
//    //                     there should be the "current draft" and
//    //                     "current default draft" entries
//
//    // schema version -- to know which schema to use to read the data
//    //
//    // [row specialized field, this is not saved as a column in the data file]
//    //
//    // this one is managed as a very special case instead; the version
//    // is saved as the first 4 bytes of any one row; plus on a read we
//    // always auto-convert the data to the latest schema version so
//    // having such a column would not be useful (i.e. it would always
//    // be the exact same value as far as the end user is concerned)
//    //
//    {
//        auto c(std::make_shared<schema_column>(
//                      shared_from_this()
//                    , "_schema_version"
//                    , struct_type_t::STRUCT_TYPE_UINT32
//                    , COLUMN_FLAG_REQUIRED | COLUMN_FLAG_SYSTEM));
//
//        f_columns_by_name[c->get_name()] = c;
//    }
//
//    // object identifier -- to place the rows in our primary index
//    //
//    // [row specialized field, this is not saved as a column in the data file]
//    //
//    // this field is not required here, but by having it, we can lose
//    // the primary index and not lose anything since we have the information
//    // here
//    {
//        auto c(std::make_shared<schema_column>(
//                      shared_from_this()
//                    , "_oid"
//                    , struct_type_t::STRUCT_TYPE_OID
//                    , COLUMN_FLAG_REQUIRED | COLUMN_FLAG_SYSTEM));
//
//        f_columns_by_name[c->get_name()] = c;
//    }
//
//    // version of the data in this row
//    //
//    // [row specialized field, this is not saved as a column in the data file]
//    //
//    // --------------------------------------------------------- resume -----
//    //
//    // The implementation is most TBD TBD TBD still. The following is my
//    // current talk about it. However, I think I got most of it laid out
//    // as I think it will be easiest: use the version + language to generate
//    // keys in separate branch and revision specific indexes, which is very
//    // similar to what I've done in Cassandra. We also probably need two
//    // fields: one to read a specific version and one to write a revision
//    // which would get automatically updated to a new branch and/or revision.
//    //
//    // See also [but that function is wrong because murmur3 cannot include
//    // the version & language]:
//    // void row::generate_mumur3(buffer_t & murmur3, version_t version, std::string const language);
//    //
//    // ------------------------------------------------ long discussion -----
//    //
//    // How this will be implemented is not clear at this point--it will
//    // only be for the `content` table (Note: in our Cassandra implementation
//    // we had a `content`, a `branch` and a `revision` table);
//    //
//    // The version itself would not be saved as a column per se, instead
//    // it would be a form of sub-index where the type of column defines
//    // how a read is handle based on the version:
//    //
//    // `global` -- the version is ignored for all global fields
//    // `branch` -- the fields are assigned the `major` version; so when the
//    // version is 1.1 or 1.100, the data returned is the same; however, you
//    // have two separate values for versions 1.55 and 3.2
//    // `revision` -- the fields are assigned the full version
//    // (`major.minor`); each piece of data depends 100% on the version
//    //
//    // So on a `commit()`, global fields are always overwritten, branches
//    // are overwritten on a per `major` version and revisions only on a
//    // per `major.minor` version.
//    //
//    // As far as the client is concerned, though, such rows have a version
//    // column which clearly defines each column's value
//    //
//    // The `_version` in a row can be set to an existing version in the row.
//    // If not defined, then no branch or revision are created at all. If
//    // set to version `0.0`, then that means create a new revision in the
//    // latest existing branch (i.e. no revision 0 exists, it's either `0.1`
//    // or undefined). At this point, I do not have a good idea to also force
//    // the creation of a new branch, unless we convert this field to a string.
//    // Then we can use all sorts of characters for the purpose (which means
//    // we may want two fields--one for write as a string and one for read):
//    //
//    // 1. `*.1` -- create a new branch with a first revision 1
//    // 2. `L.*` -- create a new revision in the [L]atest branch
//    // 3. `L.L` -- overwrite/update the latest branch and revision fields
//    // 4. `0.*` -- create a new revision in specified branch (here branch `0`)
//    //
//    // **IMPORTANT:** The revision also makes use of a language. If the
//    // "_language" column is not defined, then use "xx" as the default.
//    //
//    // Implementation Ideas: (right now I think #3 is what we must use
//    // except that it prevents us from having a simple list of
//    // `major:minor:language` sub-indexes which I think we need--so that
//    // means we probably should use #2 instead)
//    //
//    // 1. add the major version along the column ID when saving a branch
//    //    value (ID:major:value); in effect we end up with many more columns
//    //    for the one same row, only we just read those that have a major
//    //    that matches the `_version` field; similarly, the revision is
//    //    defined as column ID, major, minor (ID:major:minor:value)
//    // 2. the row has a `reference_t` to a "branch array"; that array is
//    //    a set of `reference_t` that point to all the columns specific to
//    //    that branch, the `major` version is the index in that table (we
//    //    have a map, though (`major => reference_t`) so that way older
//    //    branches can be deleted if/when necessary; the revisions would be
//    //    managed in a similar way; the main row has a reference to an array
//    //    which has a map defined as `major:minor:language => reference_t`
//    //    and the reference points to all the columns assigned that specific
//    //    revision
//    // 3. full fledge indexes which make use of the row key + major version
//    //    for branches and row key + major + minor version + language for
//    //    revision and add two more indexes in our headers just for those two;
//    //    NOTE: with a full fledge index we can distribute the data
//    //    between computers; whether we want to do that is still TBD
//    //
//    // The main problem with (1) is that one row will grow tremendously and
//    // that will probably be impossible to manage after a while (on reads as
//    // well as sheer size of the row). (2) is great although it certainly
//    // requires a lot more specialized management to maintain the arrays.
//    // (3) is probably the best since we should be able to reuse much of the
//    // code handling indexes which will just be the standard row key plus
//    // the necessary version info (major or major:minor). This is what we
//    // have in Cassandra although we have to manually handle all the revisions
//    // in our Snap! code.
//    //
//    {
//        auto c(std::make_shared<schema_column>(
//                      shared_from_this()
//                    , "_version"
//                    , struct_type_t::STRUCT_TYPE_VERSION
//                    , COLUMN_FLAG_SYSTEM));
//
//        f_columns_by_name[c->get_name()] = c;
//    }
//
//    // language code used in the "body", "title", etc.
//    //
//    // [row specialized field, this is not saved as a column in the data file]
//    //
//    // [IMPORTANT: we use a language identifier to avoid using a string
//    //             which would have a variable size]
//    //
//    // By default, we use a 2 letter code ISO-639-1 code, but this field
//    // allows for any ISO encoding such as "en-us" and any macro language.
//    // The low level system implementation doesn't care and won't verify
//    // that the language is valid. We offer higher level functions to do
//    // so if you'd like to verify before letting a user select a language.
//    //
//    // Use "xx" for an entry in a table that uses languages but does not
//    // require one. Not defining the field means that no language is
//    // specified. The system will automatically use "xx" for revisions.
//    //
//    {
//        auto c(std::make_shared<schema_column>(
//                      shared_from_this()
//                    , "_language"
//                    , struct_type_t::STRUCT_TYPE_UINT16
//                    , COLUMN_FLAG_SYSTEM));
//
//        f_columns_by_name[c->get_name()] = c;
//    }
//
//    // date when the row was created
//    {
//        auto c(std::make_shared<schema_column>(
//                      shared_from_this()
//                    , "_created_on"
//                    , struct_type_t::STRUCT_TYPE_MSTIME
//                    , COLUMN_FLAG_SYSTEM));
//
//        f_columns_by_name[c->get_name()] = c;
//    }
//
//    // when the row was last updated
//    {
//        auto c(std::make_shared<schema_column>(
//                      shared_from_this()
//                    , "_last_updated"
//                    , struct_type_t::STRUCT_TYPE_MSTIME
//                    , COLUMN_FLAG_REQUIRED | COLUMN_FLAG_SYSTEM));
//
//        f_columns_by_name[c->get_name()] = c;
//    }
//
//    // the date when it gets deleted automatically
//    {
//        auto c(std::make_shared<schema_column>(
//                      shared_from_this()
//                    , "_deleted_on"
//                    , struct_type_t::STRUCT_TYPE_MSTIME
//                    , COLUMN_FLAG_SYSTEM));
//
//        f_columns_by_name[c->get_name()] = c;
//    }
//
//    // ID of user who created this row
//    {
//        auto c(std::make_shared<schema_column>(
//                      shared_from_this()
//                    , "_created_by"
//                    , struct_type_t::STRUCT_TYPE_UINT64
//                    , COLUMN_FLAG_SYSTEM));
//
//        f_columns_by_name[c->get_name()] = c;
//    }
//
//    // ID of user who last updated this row
//    {
//        auto c(std::make_shared<schema_column>(
//                      shared_from_this()
//                    , "_last_updated_by"
//                    , struct_type_t::STRUCT_TYPE_UINT64
//                    , COLUMN_FLAG_SYSTEM));
//
//        f_columns_by_name[c->get_name()] = c;
//    }
//
//    // ID of user who deleted this row
//    {
//        auto c(std::make_shared<schema_column>(
//                      shared_from_this()
//                    , "_deleted_by"
//                    , struct_type_t::STRUCT_TYPE_UINT64
//                    , COLUMN_FLAG_SYSTEM));
//
//        f_columns_by_name[c->get_name()] = c;
//    }
//
//// this is not the right place for the current version concept; that would
//// need to be in the corresponding index
////
//// there are also several important concepts:
////
//// 1. we have the latest version; usually called the "draft" because it is
////    likely not publicly available
////
//// 2. we have the current version which is the public one
////
//// 3. we also have a similar concept with the default value (i.e. the rows
////    where the major version is 0); so we have a current default value and
////    a "draft" (latest) default value
////
////    // current version
////    //
////    // this is another entry in link with the branch/revision concept, we
////    // need to display a page, we need to have a current version to display
////    // that page; problem here is we need one such version per language
////    //
////    // in the old Cassandra database we also have a latest version, which
////    // is also per language; this latest version gets used to create new
////    // revisions effectively
////    //
////    // finally, we had a 'last edited version' because if you were to edit
////    // and not save your editing, we wanted to save a version of the page
////    // attached to your user and that was a form of "floating" version
////    // (i.e. it was not yet assigned a full version/language pair)
////    //
////    // for now I leave this at that, but I think we'll need several more
////    // fields to manage the whole set of possibilities (although things
////    // such as the last edited page is per user so we can't just have one
////    // field? well... maybe we track the last 100 edits and delete anything
////    // that's too old and was not properly saved after that) -- the editing
////    // versions can be called "draft"; which could also make use of the
////    // language field to distinguish them: "<major>.<minor>::<language>-draft"
////    //
////    {
////        auto c(std::make_shared<schema_column>(
////                      shared_from_this()
////                    , "_current_version"
////                    , struct_type_t::STRUCT_TYPE_VERSION
////                    , COLUMN_FLAG_SYSTEM));
////
////        f_columns_by_name[c->name()] = c;
////    }
//
//    // "_expiration_date" -- we actually do not need an expiration date
//    // column, the user can create his own "expiration_date" column which
//    // will automatically get picked up by the system; i.e. rows with
//    // a column with that name will automatically be added to the expiration
//    // index, nothing more to do and the programmer has the ability to choose
//    // the precision and what the value should be (it's just like a standard
//    // column) -- see is_expiration_date_column()
//
//    // go through and load columns
//    //
//    advgetopt::conf_file::sections_t sections(config->get_sections());
//    for(auto s : sections)
//    {
//        if(!s.starts_with(g_column_scope))
//        {
//            continue;
//        }
//
//        auto c(std::make_shared<schema_column>(
//                      shared_from_this()
//                    , config
//                    , s.substr(strlen(g_column_scope))));
//
//        f_columns_by_name[c->get_name()] = c;
//        f_columns_by_id[c->get_id()] = c;
//    }
//}
//
//
//void schema_table::from_config_load_primary_key(advgetopt::conf_file::pointer_t config)
//{
//    std::string const table_section_primary_key("table::primary_key");
//    if(!config->has_parameter(table_section_primary_key))
//    {
//        throw missing_parameter(
//                  "the table \""
//                + f_filename
//                + "\" must have a primary_key=... parameter defined.");
//    }
//    advgetopt::string_list_t primary_key_values;
//    advgetopt::split_string(config->get_parameter(table_section_primary_key), primary_key_values, { "," });
//    f_primary_key.clear();
//    for(auto const & id : primary_key_values)
//    {
//        column_id_t const column_id(convert_to_uint(id, 16));
//        if(f_columns_by_id.find(column_id) == f_columns_by_id.end())
//        {
//            throw column_not_found(
//                      "the table primary key in \""
//                    + f_filename
//                    + "\" references a column with identifier "
//                    + id
//                    + " which is not defined.");
//        }
//        f_primary_key.push_back(column_id);
//    }
//}
//
//
//void schema_table::from_config_load_indexes(advgetopt::conf_file::pointer_t config)
//{
//    advgetopt::conf_file::sections_t sections(config->get_sections());
//    for(auto s : sections)
//    {
//        if(!s.starts_with(g_index_scope))
//        {
//            continue;
//        }
//
//        auto index(std::make_shared<schema_secondary_index>());
//        index->from_config(
//                  config
//                , s.substr(strlen(g_index_scope)));
//
//        std::size_t const max(index->get_column_count());
//        for(std::size_t idx(0); idx < max; ++idx)
//        {
//            schema_sort_column::pointer_t sort_column(index->get_sort_column(idx));
//            column_id_t const column_id(sort_column->get_column_id());
//            if(f_columns_by_id.find(column_id) == f_columns_by_id.end())
//            {
//                throw column_not_found(
//                          "column with identifier \""
//                        + std::to_string(static_cast<int>(column_id))
//                        + "\" not found in table \""
//                        + f_name
//                        + "\".");
//            }
//        }
//
//        f_secondary_indexes[index->get_name()] = index;
//        f_secondary_indexes_by_id[index->get_id()] = index;
//    }
//}


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


/** \brief Read the schema from \p b.
 *
 * This function reads the schema of one table, including its columns, its
 * primary key definition, and if any, all of its secondary indexes.
 *
 * \param[in] b  The virtual buffer the data is being read from.
 */
void schema_table::from_binary(virtual_buffer::pointer_t b)
{
    f_structure->set_virtual_buffer(b, 0);

    f_version                  = f_structure->get_uinteger(g_name_prinbee_fld_schema_version);
    f_created_on               = f_structure->get_nstime(g_name_prinbee_fld_created_on);
    f_last_updated_on          = f_structure->get_nstime(g_name_prinbee_fld_last_updated_on);
    f_name                     = f_structure->get_string(g_name_prinbee_fld_name);
    //f_flags                    = f_structure->get_uinteger(g_name_prinbee_fld_flags);
    f_model                    = static_cast<model_t>(f_structure->get_uinteger(g_name_prinbee_fld_model));
    f_replication              = f_structure->get_uinteger(g_name_prinbee_fld_replication);
    f_description              = f_structure->get_string(g_name_prinbee_fld_description);
    f_versioned_rows           = f_structure->get_version(g_name_prinbee_fld_versioned_rows);
    f_blob_limit               = f_structure->get_uinteger(g_name_prinbee_fld_blob_limit);
    f_blob_compressor          = f_structure->get_string(g_name_prinbee_fld_blob_compressor);
    f_inline_limit             = f_structure->get_uinteger(g_name_prinbee_fld_inline_limit);
    f_external_file_compressor = f_structure->get_string(g_name_prinbee_fld_external_file_compressor);
    f_encryption_key_name      = f_structure->get_string(g_name_prinbee_fld_encrypt_key_name);

    {
        auto const field(f_structure->get_field(g_name_prinbee_fld_primary_key));
        auto const max(field->size());
        for(std::remove_const<decltype(max)>::type idx(0); idx < max; ++idx)
        {
            f_primary_key.push_back((*field)[idx]->get_uinteger(g_name_prinbee_fld_column_id));
        }
    }

    {
        auto const field(f_structure->get_field(g_name_prinbee_fld_secondary_indexes));
        auto const max(field->size());
        for(std::remove_const<decltype(max)>::type idx(0); idx < max; ++idx)
        {
            schema_secondary_index::pointer_t secondary_index(std::make_shared<schema_secondary_index>(shared_from_this()));
            secondary_index->from_binary((*field)[idx]);

            f_secondary_indexes[secondary_index->get_name()] = secondary_index;
        }
    }

    {
        auto field(f_structure->get_field(g_name_prinbee_fld_columns));
        auto const max(field->size());
        for(std::remove_const<decltype(max)>::type idx(0); idx < max; ++idx)
        {
            schema_column::pointer_t column(std::make_shared<schema_column>(shared_from_this()));
            column->from_binary((*field)[idx]);
            if(column->get_column_id() == 0)
            {
                throw id_missing(
                      "loaded column \""
                    + column->get_name()
                    + "\" from the database and its column identifier is 0.");
            }

            f_columns_by_name[column->get_name()] = column;
            f_columns_by_id[column->get_column_id()] = column;
        }
    }
}


virtual_buffer::pointer_t schema_table::to_binary() const
{
    // we do not have to do anything special since we always keep the
    // structure up to date whenever we do a set_...()
    //
    // the start offset is not important here (always zero)
    //
    reference_t start_offset;
    return f_structure->get_virtual_buffer(start_offset);

//    structure::pointer_t s(std::make_shared<structure>(g_table_description));
//    s->init_buffer();
//    s->set_uinteger("schema_version", f_version);
//    s->set_large_integer("created_on", f_created_on);
//    s->set_large_integer("last_updated_on", f_last_updated_on);
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
//    reference_t start_offset(0);
//    return s->get_virtual_buffer(start_offset);
}


void schema_table::modified()
{
    f_last_updated_on = snapdev::now();
}


/** \brief Compare two table schemata.
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
 * where found. If the schemata are the exact same, then the function says
 * they are equal (COMPARE_SCHEMA_EQUAL). Note that in most cases, we expect
 * the function to return COMPARE_SCHEMA_EQUAL since schemata should rarely
 * change.
 *
 * \param[in] rhs  The right hand side to compare this schema.
 *
 * \return One of the compare_t enumeration values.
 */
// this was a good idea, but at the same time it makes things much more
// complicated; now we just view all versions of a schema as different
// and always update the data to the latest--that means if you just update
// a description then you waste a huge amount of processing time, but in
// the end, the implementation is much easier and we could also check the
// output of a row and avoid a write if equal to the existing row... just
// update the schema version; that will be much easier
//compare_t schema_table::compare(schema_table const & rhs) const
//{
//    compare_t result(compare_t::COMPARE_SCHEMA_EQUAL);
//
//    //f_version -- we calculate the version
//    //f_created_on -- this is dynamically assigned on creation
//    //f_last_updated_on -- this is dynamically assigned on updates
//
//    if(f_name != rhs.f_name)
//    {
//        return compare_t::COMPARE_SCHEMA_DIFFER;
//    }
//
//    if(f_structure->get_uinteger(g_name_prinbee_fld_flags) == rhs.f_structure->get_uinteger(g_name_prinbee_fld_flags))
//    {
//        return compare_t::COMPARE_SCHEMA_DIFFER;
//    }
//
//    if(f_model != rhs.f_model)
//    {
//        result = compare_t::COMPARE_SCHEMA_UPDATE;
//    }
//
//    if(f_primary_key != rhs.f_primary_key)
//    {
//        return compare_t::COMPARE_SCHEMA_DIFFER;
//    }
//
//    for(schema_secondary_index::map_by_name_t::const_iterator
//            it(f_secondary_indexes.cbegin());
//            it != f_secondary_indexes.cend();
//            ++it)
//    {
//        schema_secondary_index::pointer_t rhs_secondary_index(rhs.get_secondary_index(it->first));
//        if(rhs_secondary_index == nullptr)
//        {
//            return compare_t::COMPARE_SCHEMA_DIFFER;
//        }
//        compare_t const r(it->second->compare(*rhs_secondary_index));
//        if(r == compare_t::COMPARE_SCHEMA_DIFFER)
//        {
//            return compare_t::COMPARE_SCHEMA_DIFFER;
//        }
//        if(r == compare_t::COMPARE_SCHEMA_UPDATE)
//        {
//            result = compare_t::COMPARE_SCHEMA_UPDATE;
//        }
//    }
//
//    // loop through the RHS in case we removed a secondary index
//    //
//    for(schema_secondary_index::map_by_name_t::const_iterator
//            it(rhs.f_secondary_indexes.cbegin());
//            it != rhs.f_secondary_indexes.cend();
//            ++it)
//    {
//        if(get_secondary_index(it->first) == nullptr)
//        {
//            return compare_t::COMPARE_SCHEMA_DIFFER;
//        }
//    }
//
//    //f_columns_by_id -- we only have to compare one map
//    //                   so we ignore f_columns_by_id which has the same data
//    //
//    for(schema_column::map_by_name_t::const_iterator
//            it(f_columns_by_name.cbegin());
//            it != f_columns_by_name.cend();
//            ++it)
//    {
//        schema_column::pointer_t rhs_column(rhs.get_column(it->first));
//        if(rhs_column == nullptr)
//        {
//            // we could not find that column in the other schema,
//            // so it's different
//            //
//            // TODO: make sure "renamed" columns are handled properly
//            //       once we add that feature
//            //
//            return compare_t::COMPARE_SCHEMA_DIFFER;
//        }
//        compare_t r(it->second->compare(*rhs_column));
//        if(r == compare_t::COMPARE_SCHEMA_DIFFER)
//        {
//            return compare_t::COMPARE_SCHEMA_DIFFER;
//        }
//        if(r == compare_t::COMPARE_SCHEMA_UPDATE)
//        {
//            result = compare_t::COMPARE_SCHEMA_UPDATE;
//        }
//    }
//
//    // loop through the RHS in case we removed a column
//    //
//    for(schema_column::map_by_name_t::const_iterator
//            it(rhs.f_columns_by_name.cbegin());
//            it != rhs.f_columns_by_name.cend();
//            ++it)
//    {
//        if(get_column(it->first) == nullptr)
//        {
//            // we could not find that column in the new schema,
//            // so it's different
//            //
//            // TODO: make sure "renamed" columns are handled properly
//            //       once we add that feature [use IDs instead of names?]
//            //
//            return compare_t::COMPARE_SCHEMA_DIFFER;
//        }
//    }
//
//    //f_description -- totally ignored; that's just noise
//
//    return result;
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


snapdev::timespec_ex schema_table::get_created_on() const
{
    return f_created_on;
}


snapdev::timespec_ex schema_table::get_last_updated_on() const
{
    return f_last_updated_on;
}


std::string schema_table::get_name() const
{
    return f_name;
}


model_t schema_table::get_model() const
{
    return f_model;
}


bool schema_table::is_logged() const
{
    return f_structure->get_bits("flags.logged") != 0;
}


void schema_table::set_logged(bool logged)
{
    f_structure->set_bits("flags.logged", logged ? 1 : 0);
}


bool schema_table::is_secure() const
{
    return f_structure->get_bits("flags.secure") != 0;
}


void schema_table::set_secure(bool secure)
{
    f_structure->set_bits("flags.secure", secure ? 1 : 0);
}


bool schema_table::is_translatable() const
{
    return f_structure->get_bits("flags.translatable") != 0;
}


void schema_table::set_translatable(bool translatable)
{
    f_structure->set_bits("flags.translatable", translatable ? 1 : 0);
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
