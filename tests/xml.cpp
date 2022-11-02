// Copyright (c) 2019-2022  Made to Order Software Corp.  All Rights Reserved
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

// self
//
#include    "main.h"


// prinbee
//
#include    <prinbee/exception.h>
#include    <prinbee/data/xml.h>


// C++
//
#include    <fstream>


// C
//
#include    <sys/stat.h>
#include    <sys/types.h>



namespace
{


std::string get_folder_name()
{
    std::string const xml_path(SNAP_CATCH2_NAMESPACE::g_tmp_dir() + "/xml");

    if(mkdir(xml_path.c_str(), 0700) != 0)
    {
        if(errno != EEXIST)
        {
            perror(("could not create directory \"" + xml_path + "\"").c_str());
            return std::string();
        }
    }

    return xml_path;
}


}



CATCH_TEST_CASE("XML Basics", "[xml]")
{
    CATCH_START_SECTION("empty")
    {
        std::string const xml;

        std::string const xml_path(get_folder_name());
        std::string const filename(xml_path + "/empty.xml");

        // create empty file
        {
            std::ofstream f;
            f.open(filename);
            CATCH_REQUIRE(f.is_open());
        }

        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::xml(filename)
                , prinbee::unexpected_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee: File \""
                        + filename
                        + "\" cannot be empty or include anything other than a processor tag and comments before the root tag.", true));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("empty root tag")
    {
        std::string const xml;

        std::string const xml_path(get_folder_name());
        std::string const filename(xml_path + "/empty-tag.xml");

        // create file with one empty tag
        {
            std::ofstream f;
            f.open(filename);
            CATCH_REQUIRE(f.is_open());
            f << "<empty/>";
        }

        CATCH_REQUIRE_THROWS_MATCHES(
                  prinbee::xml(filename)
                , prinbee::unexpected_token
                , Catch::Matchers::ExceptionMessage(
                          "prinbee: File \""
                        + filename
                        + "\" root tag cannot be an empty tag.", true));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("empty root")
    {
        std::string const xml;

        std::string const xml_path(get_folder_name());
        std::string const filename(xml_path + "/empty-tag.xml");

        // create file with one empty tag
        {
            std::ofstream f;
            f.open(filename);
            CATCH_REQUIRE(f.is_open());
            f << "<empty></empty>";
        }

        prinbee::xml x(filename);
        prinbee::xml_node::pointer_t root(x.root());
        CATCH_REQUIRE(root != nullptr);
        CATCH_REQUIRE(root->tag_name() == "empty");
        CATCH_REQUIRE(root->text().empty());
        CATCH_REQUIRE(root->all_attributes().empty());
        CATCH_REQUIRE(root->parent() == nullptr);
        CATCH_REQUIRE(root->first_child() == nullptr);
        CATCH_REQUIRE(root->last_child() == nullptr);
        CATCH_REQUIRE(root->next() == nullptr);
        CATCH_REQUIRE(root->previous() == nullptr);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("empty root with preprocessor")
    {
        std::string const xml;

        std::string const xml_path(get_folder_name());
        std::string const filename(xml_path + "/empty-tag.xml");

        // create file with one empty tag
        {
            std::ofstream f;
            f.open(filename);
            CATCH_REQUIRE(f.is_open());
            f << "<?xml version=\"1.0\"?><still-empty></still-empty>";
        }

        prinbee::xml x(filename);
        prinbee::xml_node::pointer_t root(x.root());
        CATCH_REQUIRE(root != nullptr);
        CATCH_REQUIRE(root->tag_name() == "still-empty");
        CATCH_REQUIRE(root->text().empty());
        CATCH_REQUIRE(root->all_attributes().empty());
        CATCH_REQUIRE(root->parent() == nullptr);
        CATCH_REQUIRE(root->first_child() == nullptr);
        CATCH_REQUIRE(root->last_child() == nullptr);
        CATCH_REQUIRE(root->next() == nullptr);
        CATCH_REQUIRE(root->previous() == nullptr);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("empty root with comment & preprocessor")
    {
        std::string const xml;

        std::string const xml_path(get_folder_name());
        std::string const filename(xml_path + "/quite-empty.xml");

        // create file with one empty tag
        {
            std::ofstream f;
            f.open(filename);
            CATCH_REQUIRE(f.is_open());
            f << "<!-- name='rotor' --><?xml version=\"1.0\"?><quite-empty></quite-empty>";
        }

        prinbee::xml x(filename);
        prinbee::xml_node::pointer_t root(x.root());
        CATCH_REQUIRE(root != nullptr);
        CATCH_REQUIRE(root->tag_name() == "quite-empty");
        CATCH_REQUIRE(root->text().empty());
        CATCH_REQUIRE(root->all_attributes().empty());
        CATCH_REQUIRE(root->parent() == nullptr);
        CATCH_REQUIRE(root->first_child() == nullptr);
        CATCH_REQUIRE(root->last_child() == nullptr);
        CATCH_REQUIRE(root->next() == nullptr);
        CATCH_REQUIRE(root->previous() == nullptr);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("empty root with comment & preprocessor & attributes")
    {
        std::string const xml;

        std::string const xml_path(get_folder_name());
        std::string const filename(xml_path + "/root-attributes.xml");

        // create file with one empty tag
        {
            std::ofstream f;
            f.open(filename);
            CATCH_REQUIRE(f.is_open());
            f << "<!--\n"
                 "name='next level'\n"
                 "-->\n"
                 "\n"
                 "<?xml version=\"1.0\"?>\n"
                 "<root-canal quite=\"quite\" size='123' very=\"true\">"
                 " \t \t \t "
                 "</root-canal>";
        }

        prinbee::xml x(filename);
        prinbee::xml_node::pointer_t root(x.root());
        CATCH_REQUIRE(root != nullptr);
        CATCH_REQUIRE(root->tag_name() == "root-canal");
        CATCH_REQUIRE(root->text() == " \t \t \t ");
        CATCH_REQUIRE(root->all_attributes().size() == 3);
        CATCH_REQUIRE(root->attribute("quite") == "quite");
        CATCH_REQUIRE(root->attribute("size") == "123");
        CATCH_REQUIRE(root->attribute("very") == "true");
        CATCH_REQUIRE(root->parent() == nullptr);
        CATCH_REQUIRE(root->first_child() == nullptr);
        CATCH_REQUIRE(root->last_child() == nullptr);
        CATCH_REQUIRE(root->next() == nullptr);
        CATCH_REQUIRE(root->previous() == nullptr);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("entities test")
    {
        std::string const xml;

        std::string const xml_path(get_folder_name());
        std::string const filename(xml_path + "/entities.xml");

        // create file with one empty tag
        {
            std::ofstream f;
            f.open(filename);
            CATCH_REQUIRE(f.is_open());
            f << "<!--\n"
                 "name='entitie&#x73;'\n"
                 "-->\n"
                 "\n"
                 "<?xml version=\"1.0\"?>\n"
                 "<entity-a-gogo quite=\"&#x71;uit&#101;\" size='1&#x32;3'"
                        " very=\"&quot;true&quot;\" special-entry=\"&quot;&lt;it&apos;s special &amp; weird&gt;&quot;\">"
                 "</entity-a-gogo>";
        }

        prinbee::xml x(filename);
        prinbee::xml_node::pointer_t root(x.root());
        CATCH_REQUIRE(root != nullptr);
        CATCH_REQUIRE(root->tag_name() == "entity-a-gogo");
        CATCH_REQUIRE(root->all_attributes().size() == 4);
        CATCH_REQUIRE(root->attribute("quite") == "quite");
        CATCH_REQUIRE(root->attribute("size") == "123");
        CATCH_REQUIRE(root->attribute("very") == "\"true\"");
        CATCH_REQUIRE(root->attribute("special-entry") == "\"<it's special & weird>\"");
        CATCH_REQUIRE(root->parent() == nullptr);
        CATCH_REQUIRE(root->first_child() == nullptr);
        CATCH_REQUIRE(root->last_child() == nullptr);
        CATCH_REQUIRE(root->next() == nullptr);
        CATCH_REQUIRE(root->previous() == nullptr);
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("XML Tree", "[xml]")
{
    CATCH_START_SECTION("tree")
    {
        std::string const xml;

        std::string const xml_path(get_folder_name());
        std::string const filename(xml_path + "/tree.xml");

        // create empty file
        {
            std::ofstream f;
            f.open(filename);
            CATCH_REQUIRE(f.is_open());
            f << "<root>"
                  << "<parent>"
                      << "<child>DATA 1</child>"
                      << "<child>DATA 2</child>"
                      << "<child>DATA 3</child>"
                  << "</parent>"
              << "</root>";
        }

        prinbee::xml x(filename);
        prinbee::xml_node::pointer_t root(x.root());
        CATCH_REQUIRE(root != nullptr);
        CATCH_REQUIRE(root->parent() == nullptr);
        prinbee::xml_node::pointer_t parent_node(root->first_child());
        CATCH_REQUIRE(parent_node != nullptr);
        CATCH_REQUIRE(root->last_child() == parent_node);
        CATCH_REQUIRE(root->next() == nullptr);
        CATCH_REQUIRE(root->previous() == nullptr);

        CATCH_REQUIRE(parent_node->parent() == root);
        prinbee::xml_node::pointer_t child1_node(parent_node->first_child());
        prinbee::xml_node::pointer_t child2_node(child1_node->next());
        prinbee::xml_node::pointer_t child3_node(child2_node->next());
        CATCH_REQUIRE(child1_node != nullptr);
        CATCH_REQUIRE(child2_node != nullptr);
        CATCH_REQUIRE(child3_node != nullptr);
        CATCH_REQUIRE(parent_node->last_child() == child3_node);
        CATCH_REQUIRE(parent_node->next() == nullptr);
        CATCH_REQUIRE(parent_node->previous() == nullptr);

        CATCH_REQUIRE(child1_node->parent() == parent_node);
        CATCH_REQUIRE(child2_node->parent() == parent_node);
        CATCH_REQUIRE(child3_node->parent() == parent_node);

        CATCH_REQUIRE(child1_node->first_child() == nullptr);
        CATCH_REQUIRE(child2_node->first_child() == nullptr);
        CATCH_REQUIRE(child3_node->first_child() == nullptr);

        CATCH_REQUIRE(child1_node->last_child() == nullptr);
        CATCH_REQUIRE(child2_node->last_child() == nullptr);
        CATCH_REQUIRE(child3_node->last_child() == nullptr);

        CATCH_REQUIRE(child1_node->text() == "DATA 1");
        CATCH_REQUIRE(child2_node->text() == "DATA 2");
        CATCH_REQUIRE(child3_node->text() == "DATA 3");

        CATCH_REQUIRE(child1_node->next() == child2_node);
        CATCH_REQUIRE(child2_node->previous() == child1_node);

        CATCH_REQUIRE(child2_node->next() == child3_node);
        CATCH_REQUIRE(child3_node->previous() == child2_node);

        CATCH_REQUIRE(child3_node->next() == nullptr);
        CATCH_REQUIRE(child1_node->previous() == nullptr);
    }
    CATCH_END_SECTION()
}



// vim: ts=4 sw=4 et
