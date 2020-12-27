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

#ifndef FORMS_HPP_INCLUDED
#define FORMS_HPP_INCLUDED

#include <cstring>
#include <iostream>
#include <vector>
#include <map>
#include <assert.h>
#include "regex.hpp"



class forms_class;

class Browser;


///======================The class containing all forms in general===================///
class forms_class {
    public:
        class form_class;
        class form_class2;
        class input_struct;
        class select_struct;
        ///=========================class containing the inside of a form=====================///
        class textarea_struct {
            public:

                std::string value_;
                friend class forms_class;
                friend class form_class;
                std::string value()
                {
                    return value_;
                }
                std::string name()
                {
                    return name_;
                }
            protected:
                std::string name_;

        };
        class option {
            public:
                bool     selected_;
                bool selected()
                {
                    return selected_;
                }
                std::string value()
                {
                    return value_;
                }
                friend class select_struct;
                friend class form_class;
                friend class form_class2;
                friend class forms_class;
            protected:
                std::string value_;

        };
        class select_struct {
            public:
                //select_struct();
                //~select_struct();
                std::vector <option> options;
                void change_name(std::string new_name)
                {
                    name_ = new_name;
                }
                std::string name()
                {
                    return name_;
                }
                friend class form_class;
                friend class form_class2;
            protected:
                std::string name_;
            private:
        };
        class form_class2 {
            public:
                //form_class();
                //~form_class();
                // bytes tell the form which part is
                // suppose to be written as bytes
                std::vector < select_struct > select;
                std::vector < input_struct  > input;
                std::vector <textarea_struct> textarea;
                std::map <std::string, std::string> bytes_;
                std::string url()
                {
                    return url_;
                }
                std::string method()
                {
                    return method_;
                }
                bool multipart()
                {
                    return multipart_;
                }
                void clear();
                friend std::ostream &operator<<( std::ostream &flux, form_class2 const& form_to_display  );
                friend class input_struct;
                friend class forms_class;
                friend class Browser;
            protected:
                void stream_it(std::ostream & flux) const;
                std::string url_;
                std::string method_;
                bool multipart_;
            private:
        };
        class form_class: protected form_class2 {
            public:
                bool direct_post;
                form_class()
                {
                    direct_post   = false;
                    against_error = "";
                }
                //~form_class();
                // bytes tell the form which part is
                // suppose to be written as bytes
                std::vector < select_struct > select;
                std::vector < input_struct  > input;
                std::vector <textarea_struct> textarea;
                //bool fully_copied= false;
                std::map <std::string, std::string> bytes_;
                void bytes(std::string name, std::string content_type="")
                {
                    bytes_[name]=content_type;
                    //bytes_.push_back(temp_bytes);
                }

                std::string url()
                {
                    return url_;
                }
                std::string method()
                {
                    return method_;
                }
                bool multipart()
                {
                    return multipart_;
                }
                void clear();
                std::string *operator[ ]  (std::string name);
                friend std::ostream &operator<<( std::ostream &flux, form_class const& form_to_display  );
                friend class input_struct;
                friend class forms_class;
                friend class Browser;
            protected:
                form_class2 form_work_on;
                void stream_it(std::ostream & flux) const;
                std::string url_;
                std::string method_;
                bool multipart_;
                std::string against_error;
            private:
        };
        class input_struct {
            protected:
                std::string type_;
                std::string name_;
                std::string value_;
            public:
                friend std::string *forms_class::form_class::operator[ ]  (std::string name);
                friend void form_class::stream_it(std::ostream & flux) const;
                friend void form_class2::stream_it(std::ostream & flux) const;
                std::string name()
                {
                    return name_;
                }
                std::string type()
                {
                    return type_;
                }
                std::string value()
                {
                    return value_;
                }
                void change_name(std::string new_name)
                {
                    name_ = new_name;
                }
                void change_type(std::string new_type)
                {
                    type_ = new_type;
                }
                void change_value(std::string new_value)
                {
                    value_ = new_value;
                }
                //need to be able to change those values like that
                //*br.form["something"]="else";
                //or
                //br.form.input[i].change_value("else");
        };
        ///==================================================================================///
        std::vector <std::string> form_raw_container;
        forms_class(std::string whole_html);
        forms_class();
        ~forms_class();
        void initialize(std::string whole_html);
        std::string all();
        int size();
        std::vector <form_class> all_forms;
        form_class operator[ ]  (int ite);

    private:
    protected:
        form_class against_error_form;
        void filter_inside_form();
        void get_raw_inputs(std::string raw_form, std::vector <std::string> &raw_inputs_container);
        form_class crack(std::string form_part);
};
///==================================================================================///


///=======constructor/destruct of form_class - extracting and storing raw forms======///
forms_class::forms_class(std::string whole_html)
{
    //we get the raw forms inside the class
    form_raw_container.clear();
    all_forms.clear();
    get_after_delimiter(whole_html,"form ",form_raw_container);
    filter_inside_form();
}
void forms_class::initialize(std::string whole_html)
{
    form_raw_container.clear();
    all_forms.clear();
    //we get the raw forms inside the class
    get_after_delimiter(whole_html,"form",form_raw_container);
    filter_inside_form();
}
forms_class::forms_class()
{
};
forms_class::~forms_class()
{
}
///==================================================================================///


///========================return the total number of forms==========================///
int forms_class::size()
{
    return all_forms.size();
}
///==================================================================================///

///===================================clear the form=================================///
void forms_class::form_class::clear()
{
        select.clear();
        input.clear();
        textarea.clear();
        bytes_.clear();
        url_      = "";
        method_   = "";
        multipart_=false;
        form_work_on.clear();
}
void forms_class::form_class2::clear()
{
        select.clear();
        input.clear();
        textarea.clear();
        bytes_.clear();
        url_      = "";
        method_   = "";
        multipart_=false;
}
///==================================================================================///

///=======================Overloading of the streaming operator======================///
void forms_class::form_class::stream_it(std::ostream & flux) const
{
    std::string output = "";
    output +="--- FORM report. Uses ";
    output +=method_;
    output +=" to URL \"";
    output +=url_;
    output +="\"\n";
    if(multipart_==true)
            output+="--- type: multipart form upload\n";
    for(unsigned int j=0;j<textarea.size();j++) {
        output+="Textarea: NAME=\"";
        output+=textarea[j].name_;
        output+="\"\n";
    }

    for(unsigned int j=0;j<select.size();j++) {
        output+="Select: NAME=\"";
        output+=select[j].name_;
        output+="\"\n";
        for(unsigned int jj=0;jj<select[j].options.size();jj++) {
            output+="    Option VALUE=\"";
            output+=select[j].options[jj].value_;
            output+="\" ";
            if(select[j].options[jj].selected_)
                output+="(SELECTED)\n";
            else
                output+="\n";
        }
    }
    if(select.size()>0)
        output+="[end of select]\n";

    for(unsigned int j=0;j<input.size();j++) {
        if( input[j].name_ == " " ||  input[j].name_ == "" ) {
                output +="Button: \"";
                output += input[j].value_;
                output +="\"";
        }
        else {
                output +="Input: NAME=\""+ input[j].name_;
                if(input[j].value_!="")
                    output +="\" VALUE=\"" + input[j].value_;
                output +="\"";
        }

        output +=" (";
        output +=input[j].type_;
        output +=")\n";
    }
    output +="--- end of FORM\n";
    flux << output;
}
void forms_class::form_class2::stream_it(std::ostream & flux) const
{
    std::string output = "";
    output +="--- FORM report. Uses ";
    output +=method_;
    output +=" to URL \"";
    output +=url_;
    output +="\"\n";
    if(multipart_==true)
            output+="--- type: multipart form upload\n";
    for(unsigned int j=0;j<textarea.size();j++) {
        output+="Textarea: NAME=\"";
        output+=textarea[j].name_;
        output+="\"\n";
    }

    for(unsigned int j=0;j<select.size();j++) {
        output+="Select: NAME=\"";
        output+=select[j].name_;
        output+="\"\n";
        for(unsigned int jj=0;jj<select[j].options.size();jj++) {
            output+="    Option VALUE=\"";
            output+=select[j].options[jj].value_;
            output+="\" ";
            if(select[j].options[jj].selected_)
                output+="(SELECTED)\n";
            else
                output+="\n";
        }
    }
    if(select.size()>0)
        output+="[end of select]\n";

    for(unsigned int j=0;j<input.size();j++) {
        if( input[j].name_ == " " ||  input[j].name_ == "" ) {
                output +="Button: \"";
                output += input[j].value_;
                output +="\"";
        }
        else {
                output +="Input: NAME=\""+ input[j].name_;
                if(input[j].value_!="")
                    output +="\" VALUE=\"" + input[j].value_;
                output +="\"";
        }

        output +=" (";
        output +=input[j].type_;
        output +=")\n";
    }
    output +="--- end of FORM\n";
    flux << output;
}
std::ostream &operator<<( std::ostream &flux, forms_class::form_class const& form_to_display  )
{
    form_to_display.stream_it(flux); // <- we change it here
    return flux;
}
std::ostream &operator<<( std::ostream &flux, forms_class::form_class2 const& form_to_display  )
{
    form_to_display.stream_it(flux); // <- we change it here
    return flux;
}
///==================================================================================///


///================Print all the forms with all their data inside====================///
std::string forms_class::all()
{
    std::string output = "";
    for(unsigned int i =0;i<all_forms.size();i++) {
        output +="--- FORM report. Uses ";
        output +=all_forms[i].method_;
        output +=" to URL \"";
        output +=all_forms[i].url_;
        output +="\"\n";

        if(all_forms[i].multipart_==true)
            output+="--- type: multipart form upload\n";

        for(unsigned int j=0;j<all_forms[i].textarea.size();j++) {
            output+="Textarea: NAME=\"";
            output+=all_forms[i].textarea[j].name();
            output+="\"\n";
        }

        for(unsigned int j=0;j<all_forms[i].select.size();j++) {
            output+="Select: NAME=\"";
            output+=all_forms[i].select[j].name();
            output+="\"\n";
            for(unsigned int jj=0;jj<all_forms[i].select[j].options.size();jj++) {
                output+="    Option VALUE=\"";
                output+=all_forms[i].select[j].options[jj].value();
                output+="\" ";
                if(all_forms[i].select[j].options[jj].selected())
                    output+="(SELECTED)\n";
                else
                    output+="\n";
            }
        }
        if(all_forms[i].select.size()>0)
            output+="[end of select]\n";

        for(unsigned int j=0;j<all_forms[i].input.size();j++) {
            if( all_forms[i].input[j].name() == " " ||  all_forms[i].input[j].name() == "" ) {
                output +="Button: \"";
                output += all_forms[i].input[j].value();
                output +="\"";
            }
            else {
                output +="Input: NAME=\""+ all_forms[i].input[j].name();
                if(all_forms[i].input[j].value()!="" && all_forms[i].input[j].value()!=" ")
                    output +="\" VALUE=\"" + all_forms[i].input[j].value();
                output +="\"";
            }
            output +=" (";
            output +=all_forms[i].input[j].type();
            output +=")\n";
        }
        output +="--- end of FORM\n";
    }
    return output;
}
///==================================================================================///


///================Overloading of the [ ] in form to fill by the name ===============///
std::string *forms_class::form_class::operator[ ]  (std::string name)
{
    //search in the form we want what is the type of the [name]
    //then push_back this on the form we want to post (original
    //is completelly empty) and return a pointer to the "value"
    //of [name] we just pushed to this form we will post/get with
    //this way we can only add what is present in the form by default


    //find the type:
    //loop through all that:
    //select   :name(), options[selected(),value()]
    //input    :name(), type(), value()
    //textarea :name(), value()
    //bytes    :strings...

    if(direct_post==false) {
        if(url_=="")
            url_=form_work_on.url();
        if(method_=="")
            method_=form_work_on.method();

        for(unsigned int ii=0;ii<form_work_on.input.size();ii++)
            if(form_work_on.input[ii].name()==name) {
                //it is an input, we need to push_back an input
                //with the same name and infos
                input_struct temp_input;
                temp_input.name_ = name;
                temp_input.type_ = form_work_on.input[ii].type();
                temp_input.value_ = "";
                input.push_back(temp_input);
                //return a pointer to the value of the input we just pushed
                assert(&input[input.size()-1].value_!=NULL);
                return &input[input.size()-1].value_;
            }

        for(unsigned int ii=0;ii<form_work_on.textarea.size();ii++)
            if(form_work_on.textarea[ii].name()==name) {
                //it is a textarea, we need to push_back a textarea
                //with the same name and infos
                textarea_struct temp_text;
                temp_text.name_=form_work_on.textarea[ii].name();
                temp_text.value_="";
                textarea.push_back(temp_text);
                //return a pointer to the value of the textarea we just pushed
                assert(&textarea[textarea.size()-1].value_!=NULL);
                return &textarea[textarea.size()-1].value_;
            }

        for(unsigned int ii=0;ii<form_work_on.select.size();ii++)
            if(form_work_on.select[ii].name()==name) {
                //it is a select, we need to push_back a
                //select if the select doesn't exist,
                //otherwise we only push_back the option

                //loop inside to see if we already have this select
                for(unsigned int loop_in=0;loop_in<select.size();loop_in++) {
                    //we already have it so we only need to
                    //push_back a new option to it
                    if(select[loop_in].name()==name) {
                        option temp_option;
                        temp_option.selected_ = true;
                        temp_option.value_    = "";
                        select[loop_in].options.push_back(temp_option);
                        assert(&select[loop_in].options[   select[loop_in].options.size()-1  ].value_!=NULL);
                        return &select[loop_in].options[   select[loop_in].options.size()-1  ].value_;
                    }
                }
                //if we don't have it we create on with the info of
                //the form we are working on
                select_struct temp_select;
                temp_select.name_ = name;
                //set the options
                option temp_option;
                temp_option.selected_ = true;
                temp_option.value_    = "";
                //add our option to the new select
                temp_select.options.push_back(temp_option);
                //push_back the new select
                select.push_back(temp_select);
                assert(&select[select.size()-1].options[   select[select.size()-1].options.size()-1  ].value_!=NULL);
                return &select[select.size()-1].options[   select[select.size()-1].options.size()-1  ].value_;
            }

    }
    //if it's a direct post we consider everything as an input
    else {
        input_struct temp_input;
        temp_input.name_ = name;
        temp_input.type_ = "text";
        temp_input.value_ = "";
        input.push_back(temp_input);
        //return a pointer to the value of the input we just pushed
        assert(&input[input.size()-1].value_!=NULL);
        return &input[input.size()-1].value_;
    }

    std::cerr<<"\n[!] No Such Name inside this form\n";
    //pointer to zero instead of NULL, this is safer against buffer overflows
    //or we can return a pointer to a temp string because we are doing that with an equal
    //return 0;
    return &against_error;
}
///=================================================================================///


///=====overloadind of operator [] to loop through forms(hopefully it will work)=====///
forms_class::form_class forms_class::operator[ ]  (int ite)
{
    if( (unsigned int) ite>=all_forms.size() || ite<0) {
        if(all_forms.size()!=0) {
            std::cerr<<"\n[!] No Such form, using the first form as default\n";
            return all_forms[0];
        }
        else {
            std::cerr<<"\n[!] No form at all seen\n";
            return against_error_form;
        }
    }
    return all_forms[ite];
}
///==================================================================================///


///=================fill the class form_class with all the data=====================///
void forms_class::filter_inside_form()
{
    for(unsigned int ii=0; ii < form_raw_container.size(); ii++) {
        ///maybe we can fork those in the background... or make it multithreaded but it will not be multiplatform
        form_class cracked_form  = crack(form_raw_container[ii]);
        all_forms.push_back(cracked_form);
    }
}
///=================================================================================///


///================Create a form from the raw form==================================///
forms_class::form_class forms_class::crack(std::string form_part)
{
    form_class my_new_form;

    //get the basic stuff of a form
    ///we can fork that
    my_new_form.url_    = get_after_equal(form_part,"action");
    ///we can fork that
    std::string temp_en= get_after_equal(form_part,"enctype");
    if(temp_en=="multipart/form-data")
        my_new_form.multipart_=true;
    else
        my_new_form.multipart_=false;
    upper_it(get_after_equal(form_part,"method"),my_new_form.method_);
    if(my_new_form.method_==" " || my_new_form.method_=="")
        my_new_form.method_="GET";

    //search for textarea if the keyword "textarea " is there
    //filter the rest in a foo -- get the name
    if( word_in(form_part,"textarea ") ) {
        ///we can fork that
        //we need to store all the textarea in an array of struct
        //containing the name and an empty value so we
        //can fill them up later
        std::vector <std::string> textarea_raw_container;
        std::string temporary_str;
        get_after_delimiter(form_part,"textarea", textarea_raw_container);
        for(unsigned int ii=0;ii<textarea_raw_container.size();ii++) {
            temporary_str = get_after_equal(textarea_raw_container[ii],"name");
            textarea_struct temporary_textarea;
            temporary_textarea.name_ = temporary_str;
            temporary_textarea.value_= "";
            my_new_form.textarea.push_back(temporary_textarea);
        }
    }

    //get the SELECT only if the keyword "select " is there
    //filter inside select in a foo -- name,options(value,selected)
    if( word_in(form_part,"select ") ) {
        //we need to have what is between < *select(.*) < */ * select *>
        //and store it in a raw container
        std::vector <std::string> select_raw_container;
        std::string temporary_str2;
        get_after_delimiter(form_part,"select", select_raw_container);
        for(unsigned int ii=0;ii<select_raw_container.size();ii++) {
            temporary_str2 = get_after_equal(select_raw_container[ii],"name");
            select_struct temporary_select;
            temporary_select.change_name(temporary_str2);
            //now we should also get all the options in an option container
            //then parse inside them to get the name and if they are selected
            //split with "< *option(.*)>" then check if " selected " is inside
            //take the value=""
            std::vector <std::string> select_options_container;
            get_between_two(select_raw_container[ii],"option",select_options_container);

            //we need to get each options
            option temporary_options;
            for(unsigned int jj=0;jj<select_options_container.size();jj++) {
                //if the word " selected" is inside we consider that this option is
                //selected
                if(word_in(select_options_container[jj]," selected"))
                    temporary_options.selected_ = true;
                else
                    temporary_options.selected_ = false;

                temporary_options.value_ = get_after_equal(select_options_container[jj],"value");
                temporary_select.options.push_back(temporary_options);
            }
            my_new_form.select.push_back(temporary_select);
        }

    }

    //continue with the inputs
    //need to form_part.split("input") and get_after_equal for each
    std::vector < std::string > temp_raw_input_container;
    //split all input and store them inside temp_raw_input_container
    get_raw_inputs(form_part, temp_raw_input_container);
    for(unsigned int i=0;i<temp_raw_input_container.size();i++) {
        std::string Name  = get_after_equal(temp_raw_input_container[i],"name");
        std::string Value = get_after_equal(temp_raw_input_container[i],"value");
        std::string Type  = get_after_equal(temp_raw_input_container[i],"type");

        input_struct temp_input_struct;
        temp_input_struct.change_name(Name);
        temp_input_struct.change_value(Value);
        temp_input_struct.change_type(Type);
        //if(temp_input_struct.name()!="" && temp_input_struct.name()!=" " && temp_input_struct.type()!="")
        my_new_form.input.push_back(temp_input_struct);
    }
    return my_new_form;
}
///=================================================================================///


///============Split inputs from form -- if not inputs return enpty string==========///
void forms_class::get_raw_inputs(std::string raw_form, std::vector <std::string> &raw_inputs_container)
{
    remove_html_comments(raw_form);
    std::string raw_form_lower;
    lower_it(raw_form,raw_form_lower);
    replaceAll(raw_form_lower,"\n"," ");
    replaceAll(raw_form,"\n"," ");

    unsigned int first_index=0;
    unsigned int last_index ;
    int backward_ite=1;
    int loop_index;
    bool find_first_index = false;

    //save the index of the first input
    while(find_first_index ==false && first_index!=std::string::npos) {
        first_index = raw_form_lower.find("input ",first_index);
        backward_ite= 1;
        //we go backward after the word input ignoring spaces
        while(raw_form_lower[first_index-backward_ite]==' ')
            backward_ite++;
        //the first char before input must be '<'
        if(raw_form_lower[first_index-backward_ite]=='<')
            find_first_index = true;
    }

    //more then one input
    while(raw_form_lower.find("input ",first_index+3)!= std::string::npos) {
        find_first_index = false;
        backward_ite     = 1;
        loop_index       = first_index;
        while(find_first_index==false  && last_index!=std::string::npos) {
            //save the last index
            last_index       = raw_form_lower.find("input ",loop_index+3);
            while(raw_form_lower[last_index-backward_ite]==' ')
                backward_ite++;
            //the first char before input must be '<'
            if(raw_form_lower[last_index-backward_ite]=='<')
                find_first_index = true;
            loop_index = last_index+3;
        }

        //append to the container the input we just found
        raw_inputs_container.push_back(raw_form.substr(first_index,last_index-first_index+1));

        //restart the search from the end
        first_index=last_index;
    }

    //there was only one input -- eliminating the form that has
    //no inputs at all
    if(first_index!=std::string::npos)
        raw_inputs_container.push_back(raw_form.substr(first_index,raw_form.length()-first_index));
}
///=================================================================================///


#endif // FORMS_HPP_INCLUDED
