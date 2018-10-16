/*
Copyright (c) 2013, Patrick Louis <patrick at unixhub.net>
All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    1.  The author is informed of the use of his/her code. The author does not have to consent to the use; however he/she must be informed.
    2.  If the author wishes to know when his/her code is being used, it the duty of the author to provide a current email address at the top of his/her code, above or included in the copyright statement.
    3.  The author can opt out of being contacted, by not providing a form of contact in the copyright statement.
    4.  If any portion of the author’s code is used, credit must be given.
            a. For example, if the author’s code is being modified and/or redistributed in the form of a closed-source binary program, then the end user must still be made somehow aware that the author’s work has contributed to that program.
            b. If the code is being modified and/or redistributed in the form of code to be compiled, then the author’s name in the copyright statement is sufficient.
    5.  The following copyright statement must be included at the beginning of the code, regardless of binary form or source code form.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Except as contained in this notice, the name of a copyright holder shall not
be used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization of the copyright holder.
*/

#ifndef LINKS_HPP_INCLUDED
#define LINKS_HPP_INCLUDED

#include <cstring>
#include <iostream>
#include <vector>
#include <iomanip>
#include <map>
#include "regex.hpp"


class links_class;
class link_struct;
std::ostream &operator<<( std::ostream &flux, link_struct const& link_to_display  );

class link_struct
{
    public:
        std::string url()
        {
            return url_;
        }
        std::string name()
        {
            return name_;
        }
        std::string title()
        {
            return title_;
        }
        std::string target()
        {
            return target_;
        }

        std::string clas()
        {
            return class_;
        }

        std::string id()
        {
            return id_;
        }
    friend class links_class;
    friend std::ostream &operator<<( std::ostream &flux, link_struct const& link_to_display  );
    private:
    protected:
        std::string url_;
        std::string name_;
        std::string title_;
        std::string target_;
        std::string class_;
        std::string id_;
        void clear_link();
        void stream_it(std::ostream & flux) const;
};

class links_class
{
    public:
        //overload the << to cout all forms
        //overload [ ] to return
        int size();
        std::string all();
        link_struct operator[ ]  (int ite);
        void getlinks(std::string raw_input);
        void clear();
    private:
    protected:
        std::vector <link_struct> links_array;
};

///===============================clear the links array==============================///
void links_class::clear()
{
    links_array.clear();
}
///==================================================================================///

///=========================Return all the links=====================================///
std::string links_class::all()
{
    std::string output="";
    for(int i=0;i<size();i++) {
        output+=links_array[i].url();
        output+="\n";
    }
    return output;
}
///==================================================================================///

///=========Get all links in a raw_html_input and saves it in an array===============///
void links_class::getlinks(std::string raw_input)
{
    std::vector <std::string> temp_raw_links_container;
    //wrong!!! there thousands of a in a page
    //a better way to do that would be:
    //remove all comments
    //find href ... go backward until we find a 'a' // or maybe here untile we find '<'
    //then go backward ignoring spaces
    // the first char should be a '<' (we save that as the first index)
    //then go forward after the href until we find <
    //ignore spaces
    //if there's a slash or backslash
    //we continue ignoring spaces and if it's followed by a 'a'
    //and after the 'a' we have a ' ' or a '>' (we save that as the last index)
    //then voila!!
    get_from_intern(raw_input, "href","a", temp_raw_links_container);

    link_struct temp_link;
    for(unsigned int ii=0;ii<temp_raw_links_container.size();ii++) {
        temp_link.clear_link();
        temp_link.target_ = get_after_equal(temp_raw_links_container[ii], "target");
        temp_link.id_     = get_after_equal(temp_raw_links_container[ii], "id");
        temp_link.class_     = get_after_equal(temp_raw_links_container[ii], "class");
        temp_link.title_  = get_after_equal(temp_raw_links_container[ii], "title");
        temp_link.url_    = get_after_equal(temp_raw_links_container[ii], "href");
        temp_link.name_   = get_between_two_closed(temp_raw_links_container[ii],"a");
        links_array.push_back(temp_link);

    }
}
///==================================================================================///


///===============================EMPTY A LINK_struct================================///
void link_struct::clear_link()
{
    url_   = "";
    name_  = "";
    title_ = "";
    target_= "";
    id_    = "";
    class_ = "";
}
///==================================================================================///


///=======================Overloading of the streaming operator======================///
//return simply the url, the part the most used of a link
void link_struct::stream_it(std::ostream & flux) const
{
    flux << url_;
}
std::ostream &operator<<( std::ostream &flux, link_struct const& link_to_display  )
{
    link_to_display.stream_it(flux); // <- we change it here
    return flux;
}
///==================================================================================///


///==================Return the number of links found in a page======================///
int links_class::size()
{
    return (links_array.size());
}
///==================================================================================///


///======================Overloading of [] to return the link========================///
link_struct links_class::operator[ ]  (int ite)
{
    //return the link which has a url, name, title, target
    if((unsigned int)ite<links_array.size() && ite>-1)
        return (links_array[ite]);
    else {
        std::cerr<<"\n[!] No Such link, using the last link as default\n";
        return (links_array[links_array.size()-1]);
    }
}
///==================================================================================///

#endif // LINKS_HPP_INCLUDED
