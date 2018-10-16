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

#ifndef REGEX_HPP_INCLUDED
#define REGEX_HPP_INCLUDED

#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <sstream>


bool remove_html_comment(std::string & html_response);
void remove_html_comments(std::string & html_response);
void lower_it(const std::string &income, std::string & outcome);
void upper_it(const std::string &income, std::string & outcome);
bool word_in(const std::string &the_string, const std::string &to_search);
std::string get_after_equal(std::string html_response, std::string seeking);
void replaceAll(std::string& str, const std::string& from, const std::string& to);
std::string get_between_two_closed(std::string raw_input,std::string seeking);
void get_between_two(std::string raw_input, std::string seeking, std::vector <std::string> & container);
void get_after_delimiter(std::string html_response, std::string seeking, std::vector <std::string> &form_container);
void get_from_intern(std::string raw_input, std::string word,std::string word2, std::vector <std::string> & container);
std::string  to_string(unsigned int integer);

///========================Conver a u integer to string==============================///
std::string to_string(unsigned int integer)
{
	std::ostringstream o;
	o << integer;
	return o.str();
}
///==================================================================================///

///===========================SPLIT A STRING FROM A DELIMITER========================///
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim))
        elems.push_back(item);
    return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}
///=================================================================================///

///==============================UPPER IT FROM STRING===============================///
void upper_it(const std::string &income, std::string & outcome)
{
    char c;
    int i=0;
    while (income[i]) {
        c        = income[i];
        outcome += toupper(c);
        i++;
    }
}
///=================================================================================///

///==============================LOWER IT FROM STRING===============================///
void lower_it(const std::string &income, std::string & outcome)
{
    char c;
    int i=0;
    while (income[i]) {
        c        = income[i];
        outcome += tolower(c);
        i++;
    }
}
///=================================================================================///

///===============================SEARCH FOR A WORD (any)===========================///
bool word_in(const std::string &the_string, const std::string &to_search)
{
    std::string temp;
    lower_it(the_string,temp);
    return (  temp.find(to_search)!=std::string::npos ||
              the_string.find(to_search)!=std::string::npos);
}
///=================================================================================///

///=================REPLACE SOMETHING IN A STRING WITH SOMETHING ELSE===============///
void replaceAll(std::string& str, const std::string& from, const std::string& to)
{
    if(from.empty())
        return;
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}
///=================================================================================///

///=================================REMOVE COMMENTS=================================///
//Remove 1 comments -> return true
//No more comments  -> return false
bool remove_html_comment(std::string & html_response)
{
    //save the start position of the start of a comment
    std::basic_string <char>::size_type position_start         = html_response.find ("<!--");
    std::basic_string <char>::size_type position_stop          = html_response.find ("-->");
    if (position_start != std::string::npos) {
        //now we need to check for the end of it
        //... if no end
        //then we remove all after the start of the comment
        if (position_stop != std::string::npos)
            html_response.erase (position_start, (position_stop-position_start)+3);
        else
            html_response.erase (position_start,html_response.length());
        return true;
    }
    //couldn't find any <!-- so no more comments in the html_response
    else
        return false;
}
void remove_html_comments(std::string & html_response)
{
    bool check_if_still_comments = true;
    while(check_if_still_comments)
        check_if_still_comments  = remove_html_comment(html_response);
}
///=================================================================================///

//WHAT IF WE HAVE FOUND SOMETHING VALUABLE BUT THAT IS BETWEEN QUOTES "" ??? PROBLEMS
//A BUG WILL CERTAINLLY HAPPENS BECAUSE WE BREAK THE HTML WHERE WE SHOULDN"T
//EVERYTHING DEPENDS ON IF THE HTML HAS "<" or ">" BETWEEN SOME QUOTES SOMEWHERE
//BUT WHO'S THE DUMBASS WHO WILL WRITE HTML LIKE THAT???
///========================RETURN "seeking *= *\"(.*)\""============================///
std::string get_after_equal(std::string html_response, std::string seeking)
{
    //0- Remove all comments because we don't want to search inside of them
    remove_html_comments(html_response);

    //1- lower it all but in a temporary string because we only want the indexes
    std::string html_lower;
    lower_it(html_response,html_lower);
    replaceAll(html_lower,"\n"," ");
    replaceAll(html_response,"\n"," ");
    //replace single quote with double quote to make the search easier/faster
    replaceAll(html_lower,"'","\"");

    //2- start the search at 0 for the begining of a form
    //   until we reach stage 3 which will turn STOP
    //   to TRUE
    /* some variables we'll use */
    /*related to the start */
    bool STOP      = false;
    int position   = 0;
    std::basic_string <char>::size_type first_index;
    /*related to the end */
    int forward_ite;
    while( STOP == false ) {
        // we didn't find an end yet
        //found_end = false;
        //2- start the search at 0 for the begining of a seeking
        first_index = html_lower.find(seeking.c_str(),position);

        //3- if first_index shows that the word seeking
        //   is not present in the html. WE STOP HERE
        if (first_index == std::string::npos)
            STOP = true;
        //4- we found a first instance of seeking BUT is it
        //   in the MIDDLE OF A WORD or before the '=' ?
        else {
            //maybe there's a word like aseeking (contains seeking)
            if(html_lower[first_index-1] == ' ') {
                // we go forward ignoring spaces until we reach a char
                forward_ite=seeking.length();
                while(html_lower[first_index+forward_ite]==' ')
                    forward_ite++;
                // if the char after the spaces is not '=' then we go back to
                // stage 2 (searching for the word seeking)
                position = first_index + seeking.length()-1;

                //we found the start
                if(html_lower[first_index+forward_ite] == '=') {
                    //5- So we found the begining of "seeking *="
                    //   now we need to ignore the spaces again
                    forward_ite++;
                    while(html_lower[first_index+forward_ite]==' ')
                        forward_ite++;

                    //we are here "seeking *= *"
                    //we should check if we have '"'
                    if(html_lower[first_index+forward_ite] == '"' || html_lower[first_index+forward_ite] == '\'') {
                        forward_ite++;
                        //we save the position of the start of the string here
                        position = first_index+forward_ite;
                        //we search for the next '"' that is not preceded by a backslash
                        while(html_lower[first_index+forward_ite] != '"'  &&
                               html_lower[first_index+forward_ite-1] != '\\')
                            forward_ite++;
                        //we found the end of the string
                        return html_response.substr(position,first_index+forward_ite-position);
                    }
                    ///MALFORMED HTML BUT WE SHOULD RETURN WHAT IS AFTER THE =
                    // we have "seeking *= *" not followed by a '"'
                    else {
                        //we save the position of the start of the string here
                        position = first_index+forward_ite;
                        //we search for the next ' '
                        while(html_lower[first_index+forward_ite] != ' ' && html_lower[first_index+forward_ite] != '>')
                            forward_ite++;
                        //we found the end of the string
                        return html_response.substr(position,first_index+forward_ite-position);
                    }
                }
            }
            //if it's a word containing seeking go backup and search again
            else
                position = first_index + seeking.length()-1;
        }
    }
    //in case of errors
    return "";
}
///=================================================================================///

///====GET  "< *seeking(.*)< */or\ *seeking" and append it to the form_container====///
void get_after_delimiter(std::string html_response, std::string seeking, std::vector <std::string> &form_container)
{
    //0- Remove all comments because we don't want to search inside of them
    remove_html_comments(html_response);

    //1- lower it all but in a temporary string because we only want the indexes
    std::string html_lower;
    lower_it(html_response,html_lower);

    //2- start the search at 0 for the begining of a form
    //   until we reach stage 3 which will turn STOP
    //   to TRUE
    /* some variables we'll use */
    /*related to the start */
    bool STOP      = false;
    int position       = 0;
    std::basic_string <char>::size_type first_index;
    /*related to the end */
    bool found_end = false;
    int backward_ite;
    std::basic_string <char>::size_type last_index;

    while( STOP == false ) {
        // we didn't find an end yet
        found_end = false;
        //2- start the search at 0 for the begining of a form
        first_index = html_lower.find(seeking.c_str(),position);

        //3- if first_index shows that the word "form"
        //   is not present then there's no more forms
        //   in the html. WE STOP HERE
        if (first_index == std::string::npos)
            STOP = true;

        //4- we found a first instance of "form" BUT is it
        //   in the MIDDLE OF A WORD or at the BEGINING OF
        //   A FORM?
        else {
            // we go backward ignoring spaces until we reach a char
            backward_ite=1;
            while(html_lower[first_index-backward_ite]==' ')
                backward_ite++;

            // if the char after the spaces is not "<" then we go back to
            // stage 2 (searching for the word "form")

            // we restart the search from where we stoped
            // after the "form"
            position = first_index + seeking.length();


            //we found the start of a form at first_index
            if(html_lower[first_index-backward_ite] == '<') {
                //5- So we found the begining of a form "< *form"
                //   now we need to find its end
                //   we search for a "form" after it
                position = first_index + seeking.length();

                //we search increasingly, meaning the position variable
                //change after all "form" found until it's a real end
                while( found_end == false ) {
                    last_index = html_lower.find(seeking.c_str(),position);

                    //OMG MALFORMED HTML the <form> is not close
                    //here this is to prevent infinity loop
                    if (last_index == std::string::npos) {
                        STOP = true;
                        break;
                    }

                    // we go backward ignoring spaces until we reach a char
                    backward_ite=1;
                    while(html_lower[last_index-backward_ite]==' ')
                        backward_ite++;

                    //The first char should be "/" or "\\"
                    //if not we search again for the end of the form
                    position = last_index + seeking.length();

                    if(html_lower[last_index-backward_ite] == '/' ||
                       html_lower[last_index-backward_ite] == '\\') {
                    //6- if there's a / or \\ then we go backward again ignoring spaces
                        backward_ite++;
                        while(html_lower[last_index-backward_ite]==' ')
                            backward_ite++;

                        //7- we found the start of a form and the end of it
                        if(html_lower[last_index-backward_ite]=='<') {
                            //let's add it to the container
                            form_container.push_back(  html_response.substr(first_index,last_index-first_index)  );
                            found_end=true;
                        }
                    }
                }
            }
        }
    }
}
///=================================================================================///

///================GET "< *seeking(.*)>" and append it to a container===============///
void get_between_two(std::string raw_input, std::string seeking, std::vector <std::string> & container)
{
    //0- Remove all comments because we don't want to search inside of them
    remove_html_comments(raw_input);

    //1- lower it all but in a temporary string because we only want the indexes
    std::string raw_lower;
    lower_it(raw_input,raw_lower);

    //2- start the search at 0 for the begining of a form
    //   until we reach stage 3 which will turn STOP
    //   to TRUE
    /* some variables we'll use */
    /*related to the start */
    bool STOP      = false;
    unsigned int position       = 0;
    std::basic_string <char>::size_type first_index;
    /*related to the end */
    int backward_ite;

    while( STOP == false ) {
        // we didn't find an end yet
        //found_end = false;
        //2- start the search at 0 for the begining of a form
        first_index = raw_lower.find(seeking.c_str(),position);

        //3- if first_index shows that the word "form"
        //   is not present then there's no more forms
        //   in the html. WE STOP HERE
        if (first_index == std::string::npos)
            STOP = true;

        //4- we found a first instance of "form" BUT is it
        //   in the MIDDLE OF A WORD or at the BEGINING OF
        //   A FORM?
        else {
            // we go backward ignoring spaces until we reach a char
            backward_ite=1;
            while(raw_lower[first_index-backward_ite]==' ')
                backward_ite++;

            // if the char after the spaces is not "<" then we go back to
            // stage 2 (searching for the word "form")

            // we restart the search from where we stoped
            // after the "form"
            position = first_index + seeking.length();

            //we found the start of a form at first_index
            if(raw_lower[first_index-backward_ite] == '<') {
                //5- So we found the begining of a form "< *form"
                //   now we need to find its end
                //   we search for a ">" after it
                //   saves the pos and append it to the container
                position = first_index + seeking.length();

                //we search increasingly, meaning the position variable
                while( raw_lower[position] != '>' ) {
                    position++;

                    //OMG MALFORMED HTML the "< *seeking(.*)" is not close
                    //here this is to prevent infinity loop
                    if (position == raw_lower.length()-1) {
                        STOP = true;
                        break;
                    }
                }
                //here we should have "< *seeking(.*)>"
                container.push_back( raw_input.substr(first_index,position-first_index) );
            }
        }
    }
}
///=================================================================================///

///=====================GET ">(.*)< *[/|\] *seeking[ |>]" and throws a string============================///
// search for the first '>' from beg
// search for the first '<' from the end
//that's it! return :)
std::string get_between_two_closed(std::string raw_input,std::string seeking)
{
    //0- Remove all comments because we don't want to search inside of them
    remove_html_comments(raw_input);

    //1- lower it all but in a temporary string because we only want the indexes
    std::string raw_lower;
    lower_it(raw_input,raw_lower);

    /* some variables we'll use */
    /*related to the start */
    std::basic_string <char>::size_type first_index;
    unsigned int backward_ite;

    //find the first closing '>' saves it as the first index of the output string
    first_index = raw_lower.find('>',0)+1;

    if(first_index!=std::string::npos) {
        backward_ite = raw_lower.length()-1;

        while(raw_lower[backward_ite]!='<')
            backward_ite--;

        return raw_input.substr(first_index,backward_ite-first_index);
    }
    else
        //on error return empty string
        return "";

}
///=================================================================================///

///=========================yet another weird regex==================================///
// "(< *word2 .* word *=.*< *[/|\] *word2 *>)"
//here we must find all the links word = href ; word2=a
//the search if based on word and not on word2 for efficiency because
//word is less common than word2
// "(< *a .* href *=.*< *[/|\]] *a *>)"
void get_from_intern(std::string raw_input, std::string word,std::string word2, std::vector <std::string> & container)
{

    // Remove all comments because we don't want to search inside of them
    remove_html_comments(raw_input);

    //lower it all but in a temporary string because we only want the indexes
    std::string raw_lower;
    lower_it(raw_input,raw_lower);

    //start the search from 0
    //some variables we'll use
    //related to the start
    //position is the variable that
    //decides from where the search are done
    unsigned int position   = 0;
    std::basic_string <char>::size_type first_index = 0;
    std::basic_string <char>::size_type middle_index = 0;
    unsigned int forward_ite;
    int backward_ite;
    bool have_found=false;

    while(middle_index!=std::string::npos && position<=raw_lower.length()) {
        //we find the word in the middle
        //" word"
        middle_index = raw_lower.find(" "+word,position);
        if (middle_index==std::string::npos) break;
        position     = middle_index+2;
        backward_ite = 1;
        forward_ite  = 0;
        have_found   = false;

        //ignore the spaces after word
        //" word *"
        forward_ite+=word.length()+1;
        while(raw_lower[middle_index+forward_ite]==' ')
            forward_ite++;

        position = middle_index+forward_ite;

        //continue only if after the spaces we have '='
        //" word *="
        if(raw_lower[middle_index+forward_ite]=='=') {

            //place the cursor after the =
            forward_ite++;

            ///WRONG!!! we can make it faster by only searching for the first '<'
            ///before " word" then go forward to confirm it's a word2
            ///no exception occurs with this method
            //we found the first '<' before href
            //"<.* word *="
            while(raw_lower[middle_index-backward_ite]!='<')
                backward_ite++;
            //save this position as the first index for later use
            //if it's a good tag
            first_index = middle_index-backward_ite;

            //go forward after the '<' and check if we have 'a '
            backward_ite--;
            //here we ignore the spaces
            while(raw_lower[middle_index-backward_ite]==' ')
                backward_ite--;
            //verification of the a tag 'a '
            if(raw_lower[middle_index-backward_ite]==word2[0] && raw_lower[middle_index-backward_ite+1]==' ') {
                //"< *word2 .* word *="
                //we now move forward after the =
                //"< *word2 .* word *=.*<"
                ///here maybe we don't find an end
                ///maybe we can search for /a
                while(have_found==false) {
                    while(raw_lower[middle_index+forward_ite]!='<' &&
                          (raw_lower[middle_index+forward_ite+1]!=' ' || raw_lower[middle_index+forward_ite+1]!='/'|| raw_lower[middle_index+forward_ite+1]!='\\')
                         )
                        forward_ite++;
                    //place the cursor after the <
                    //"< *word2 .* word *=.*<"
                    forward_ite++;


                    //ignore all spaces
                    //"< *word2 .* word *=.*< *"
                    while(raw_lower[middle_index+forward_ite]==' ')
                        forward_ite++;

                    //continue only if we have a backslash or slash
                    //"< *word2 .* word *=.*< *[/|\]"
                    if(raw_lower[middle_index+forward_ite]=='/'||raw_lower[middle_index+forward_ite]=='\\') {
                        forward_ite++;
                        //ignore the spaces
                        //"< *word2 .* word *=.*< *[/|\] *"
                        while(raw_lower[middle_index+forward_ite]==' ')
                            forward_ite++;

                        //continue if we have the end of the tag
                        //"< *word2 .* word *=.*< *[/|\] *word2"
                        if(raw_lower[middle_index+forward_ite]==word2[0]) {
                            //place ourself after word2
                            //and ignore spaces
                            //"< *word2 .* word *=.*< *[/|\] *word2 *"
                            forward_ite+=word2.length();
                            while(raw_lower[middle_index+forward_ite]==' ')
                                forward_ite++;

                            //if we have '>' then it's finally the end of the tag
                            //"< *word2 .* word *=.*< *[/|\] *word2 *>"
                            if(raw_lower[middle_index+forward_ite]=='>') {
                                have_found=true;
                                //append the regex to the container
                                container.push_back( raw_input.substr(first_index,middle_index+forward_ite-first_index+1) );
                            }
                        }
                    }
                }
            }
            /////////==-here end same as exception-==//////////
        }
    }
}
///==================================================================================///
#endif // REGEX_HPP_INCLUDED
