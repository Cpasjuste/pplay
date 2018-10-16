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

#ifndef MECHANIZE_HPP_INCLUDED
#define MECHANIZE_HPP_INCLUDED

#include <curl/curl.h>
#include <iostream>
#include <cstring>
#include <assert.h>
#include <vector>
#include <sstream>
#include <map>
#include "forms.hpp"
#include "links.hpp"
#include <errno.h>


///==================================THE BROWSER CLASS==============================///
class Browser {
    private:
        CURL *curl;
        FILE *filepipe;
        CURLcode res;
        static size_t write_to_string(void *curl, size_t size, size_t count, void *response);
        static size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream);
        /* init to NULL is important */
        bool writing_bytes            ;
        int  timeout                  ;
        bool fetching_links           ;
        bool fetching_forms           ;
        bool direct_form_post_        ;
        bool full_form_               ;
        struct curl_httppost *formpost;
        struct curl_httppost *lastptr;
        std::vector <std::string> history_;
        //might use that instead of always initializing the forms when opening the page
        //bool form_is_initialized      = false;
        void open_form(std::string url, int usertimeout);
        void convert_1_to_2(forms_class::form_class form_work_on_first,
                            forms_class::form_class2 &form_work_on_first2);
        void take_hidden(   forms_class::form_class form_work_on_first,
                            forms_class::form_class &form_we_need);
        class emails_class {
            public:
                //emails_class();
                //~emails_class();
                std::string operator[ ]  (int ite);
                int size()
                {
                    return all_emails.size();
                }
                void init(links_class links);
                std::string all();
            private:
                std::vector <std::string> all_emails;
            protected:
        };

    protected:
        std::string header_           ;
        std::string html_response     ;

    public:
        Browser();
        ~Browser();
        void init();
        void clean();
        bool error();
        std::string status();
        std::string info()
        {
            return header_;
        }
        forms_class forms;
        forms_class::form_class form;
        links_class links;
        emails_class emails;
        void select_form(int number_start_from_zero);
        void fetch_forms(bool allow);
        void fetch_links(bool allow);
        void submit(int timeout);
        void set_direct_form_post(bool direct,std::string url);
        std::string escape(std::string the_string);
        std::string unescape(std::string the_string);
        std::string get_first_root(bool start_from_host);
        void limit_speed(int limit);
        void limit_time(int limit);
        void set_http_tunel(bool allow);
        void set_proxy_login(std::string username, std::string passwd);
        void addheaders(std::string headers_to_add[2]);
        void adduseragent(std::string theuseragent);
        void addheaders(std::string header_to_add,std::string second_header_to_add);
        void addheaders(std::map<std::string, std::string> Headers);
        void addheaders(std::vector<std::string> Headers);
        void open(std::string url, int usertimeout,bool save_history);
        void open(std::string url, std::string post_data, int usertimeout);
        void open(std::string url, int usertimeout,std::string post_data);
        void open_novisit(std::string url, int usertimeout);
        void follow_link(std::string name_of_link_to_follow,int usertimeout);
        void set_handle_redirect(bool allow);
        void set_handle_gzip(bool allow);
        void set_handle_ssl(bool allow);
        void set_verbose(bool allow);
        void set_cookie(std::string cookies);
        void set_cookiejar(std::string cookiejar);
        void set_cookiejar();
        void set_dns(std::string dns_server);
        void set_proxy(bool allow);
        void set_proxy(std::string proxy, std::string type);
        void set_interface(std::string interface_name, long int port, long int max_port);
        void set_http_version_1_0(bool set_it);
        void write_bytes(std::string filename);
        std::string getcookies();
        void reload();
        std::string geturl();
        std::string title();
        bool intitle(std::string str);
        bool inresponse(std::string str);
        bool inurl(std::string str);
        std::string response();
        void head_request(bool allow);
        CURL *get_handle();
        void close();
        std::vector<std::string> get_history();
        void clear_history();
        void history();
        void back(int timeout);
        bool viewing_html();
};
///=================================================================================///



///==================================CONSTRUCTOR====================================///
Browser::Browser()
{
    header_                  = "";
    html_response            = "";
    formpost                 = NULL;
    lastptr                  = NULL;
    full_form_               = false;
    direct_form_post_        = false;
    writing_bytes            = false;
    timeout                  = 30;
    fetching_links           = true;
    fetching_forms           = true;

    //get a handler
    curl_global_cleanup();
    curl = curl_easy_init();
    //to init all
    curl_global_init(CURL_GLOBAL_ALL);
    //for ssl
    curl_global_init(CURL_GLOBAL_SSL);
    curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_TRY);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    //for authentification
    curl_easy_setopt(curl, CURLOPT_HTTPAUTH,  CURLAUTH_DIGEST|CURLAUTH_BASIC|CURLAUTH_ANYSAFE);
    curl_easy_setopt(curl, CURLOPT_PROXYAUTH,  CURLAUTH_ANYSAFE);
    curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");
    curl_easy_setopt(curl, CURLOPT_POSTREDIR, CURL_REDIR_POST_ALL);
    #ifdef windows
        //For W$ -- init winsock
        curl_global_init(CURL_GLOBAL_WIN32);
    #endif
}
///=================================================================================///


///====================================DESTRUCTOR===================================///
Browser::~Browser()
{
    init();
    curl_easy_cleanup(curl);
    curl = nullptr;
    history_.clear();
    curl_global_cleanup();
}
///=================================================================================///


///=================================Close the Browser===============================///
void Browser::close()
{
    history_.clear();
    init();
}
///=================================================================================///


///======================INITIALIZE THE BROWSER AND VARIABLES=======================///
void Browser::init()
{
    //maybe we'll loose the cookies if we do that
    //curl                   = curl_easy_init();
    if(formpost!=NULL)
        curl_formfree(formpost);

    struct curl_slist *headers  = NULL;
    headers                     = curl_slist_append(headers, "Accept:");
    curl_slist_free_all(headers);
    curl_easy_reset(curl);

    html_response               = "";
    header_                     = "";

    forms.form_raw_container.clear();
    forms.all_forms.clear();
    form.clear();
    links.clear();
}
///=================================================================================///


///============================CLEAN BEFORE CLOSING==================================///
void Browser::clean()
{
    init();
    history_.clear();

    struct curl_slist *headers    = NULL;
    headers = curl_slist_append(headers, "Accept:");
    curl_slist_free_all(headers);

    curl_easy_cleanup(curl);
    curl    = curl_easy_init();
}
///=================================================================================///


///===========================HANDLE THE ERROR OUTPUT================================///
bool Browser::error()
{
    //Check the return code for errors
    if(res != CURLE_OK) {
        fprintf(stderr, "\n[!] There was an error while opening the page : %s\n",
                    curl_easy_strerror(res));
        return true;
    }
    return false;
}
///=================================================================================///


///=====================IF FALSE THEN NO FORMS OR LINKS ARE FETHED===================///
void Browser::fetch_forms(bool allow)
{
    fetching_forms = allow;
}
void Browser::fetch_links(bool allow)
{
    fetching_links = allow;
}
///=================================================================================///


///==================THE OPEN PAGE WITH THE OVERLOADING=============================///
void Browser::open(std::string url, int usertimeout=20,bool save_history=true)
{
    init();
    timeout = usertimeout;
    assert(timeout>0);
    //set the url in the options
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str() );
    //Handle the response
    if(writing_bytes==false)
    {
        addheaders("Accept","text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
        addheaders("Connection" ,"keep-alive");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string );
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &html_response);
        curl_easy_setopt(curl, CURLOPT_WRITEHEADER, &header_);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT,   timeout );
    }
    else
    {
        assert(filepipe!=NULL);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, filepipe);
    }

    res = curl_easy_perform(curl);
    if(error())
    {
        std::cerr<<"\n";
    }
    if(writing_bytes==true)
        fclose(filepipe);
    //because we don't want to parse bytes for forms
    else
    {
        if(fetching_forms == true)
            forms.initialize(html_response);
        if(fetching_links == true)
        {
            links.getlinks(html_response);
            emails.init(links);
        }
    }
    //add current url to the history
    if(save_history)
        history_.push_back(geturl());
    full_form_                  = false;
    writing_bytes               = false;
    timeout                     = 20;
    direct_form_post_           = false;
}
void Browser::open_novisit(std::string url, int usertimeout=20)
{
    timeout = usertimeout;
    assert(timeout>0);
    //set the url in the options
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str() );
    //Handle the response
    if(writing_bytes==false)
    {
        addheaders("Accept","text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
        addheaders("Connection" ,"keep-alive");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string );
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &html_response);
        curl_easy_setopt(curl, CURLOPT_WRITEHEADER, &header_);
    }
    else
    {
        assert(filepipe!=NULL);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, filepipe);
    }
    curl_easy_setopt(curl, CURLOPT_TIMEOUT,   timeout );
    res = curl_easy_perform(curl);
    if(error())
    {
        std::cerr<<"\n";
    }
    if(writing_bytes==true)
    {
        fclose(filepipe);
        writing_bytes=false;
    }
    full_form_                  = false;
    writing_bytes               = false;
    timeout                     = 20;
    direct_form_post_           = false;
}
void Browser::open(std::string url, std::string post_data, int usertimeout=20)
{
    init();
    timeout = usertimeout;
    assert(timeout>0);
    //set the url in the options
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str() );
    //and set it as the options for curl
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
    //Handle the response
    if(writing_bytes==false)
    {
        addheaders("Accept","text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
        addheaders("Connection" ,"keep-alive");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string );
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &html_response);
        curl_easy_setopt(curl, CURLOPT_WRITEHEADER, &header_);
    }
    else
    {
        assert(filepipe!=NULL);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, filepipe);
    }
    curl_easy_setopt(curl, CURLOPT_TIMEOUT,   timeout );
    res = curl_easy_perform(curl);
    if(error())
    {
        std::cerr<<"\n===============================================\n";
    }
    if(writing_bytes==true)
        fclose(filepipe);
    //because we don't want to parse bytes for forms
    else
    {
        if(fetching_forms == true)
            forms.initialize(html_response);
        if(fetching_links == true)
        {
            links.getlinks(html_response);
            emails.init(links);
        }
    }
    history_.push_back(geturl());
    full_form_                  = false;
    writing_bytes               = false;
    timeout                     = 20;
    direct_form_post_           = false;
}
void Browser::open(std::string url, int usertimeout,std::string post_data)
{
    init();
    timeout = usertimeout;
    assert(timeout>0);
    //set the url in the options
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str() );
    //and set it as the options for curl
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
    //Handle the response
    if(writing_bytes==false)
    {
        addheaders("Accept","text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
        addheaders("Connection" ,"keep-alive");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string );
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &html_response);
        curl_easy_setopt(curl, CURLOPT_WRITEHEADER, &header_);
    }
    else
    {
        assert(filepipe!=NULL);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, filepipe);
    }
    curl_easy_setopt(curl, CURLOPT_TIMEOUT,   timeout );
    res = curl_easy_perform(curl);
    if(error())
    {
        std::cerr<<"\n===============================================\n";
    }
    if(writing_bytes==true)
        fclose(filepipe);
    //because we don't want to parse bytes for forms
    else
    {
        if(fetching_forms == true)
            forms.initialize(html_response);
        if(fetching_links == true)
        {
            links.getlinks(html_response);
            emails.init(links);
        }
    }
    history_.push_back(geturl());
    full_form_                  = false;
    writing_bytes               = false;
    timeout                     = 20;
    direct_form_post_           = false;
}
void Browser::open_form(std::string url, int usertimeout=20)
{
    timeout = usertimeout;
    assert(timeout>0);
    //set the url in the options
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str() );
    //Handle the response
    if(writing_bytes==false)
    {
        addheaders("Accept","text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
        addheaders("Connection" ,"keep-alive");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string );
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &html_response);
        curl_easy_setopt(curl, CURLOPT_WRITEHEADER, &header_);
    }
    else
    {
        assert(filepipe!=NULL);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, filepipe);
    }
    curl_easy_setopt(curl, CURLOPT_TIMEOUT,   timeout );
    res = curl_easy_perform(curl);
    if(error())
    {
        std::cerr<<"\n";
    }
    if(writing_bytes==true)
        fclose(filepipe);
    //because we don't want to parse bytes for forms
    else
    {
        if(fetching_forms == true)
            forms.initialize(html_response);
        if(fetching_links == true)
        {
            links.getlinks(html_response);
            emails.init(links);
        }
    }
    history_.push_back(geturl());
    full_form_                  = false;
    writing_bytes               = false;
    timeout                     = 20;
    direct_form_post_           = false;
}
///=================================================================================///


///=======================Select the form number to post later=======================///
void Browser::select_form(int number_start_from_zero)
{
    //starting from 0
    if(forms.size()>=number_start_from_zero-1) {
        //if we want to have the whole form copied
        //we will already have all the infos inside form
        //so no need for an intermediate form
        forms_class::form_class form_work_on_first  = forms[number_start_from_zero];

        //now convert this forms_class::form_class into a forms_class::form_class2
        forms_class::form_class2 form_work_on_first2;
        convert_1_to_2(form_work_on_first,form_work_on_first2);

        //place it in the form so it can know which one we are working with now
        form.form_work_on = form_work_on_first2;

        //copy only the hidden part of the form selected, the other parts must be selected by the user
        take_hidden(form_work_on_first,form);
    }
}
///=================================================================================///


///======================take only the hidden fields=================================///
void Browser::take_hidden(forms_class::form_class form_work_on_first, forms_class::form_class &form_we_need)
{
    for(unsigned int ii=0;ii<form_work_on_first.input.size();ii++)
    {
        //if we have the type hidden we save it in form by default
        if( word_in(form_work_on_first.input[ii].type(),"hidden") )
        {
            forms_class::input_struct temp = form_work_on_first.input[ii];
            form_we_need.input.push_back(temp);
        }
    }
}
///=================================================================================///


///======================convert a second into a one=================================///
void Browser::convert_1_to_2(forms_class::form_class form_work_on_first
                            ,forms_class::form_class2 &form_work_on_first2)
{
        form_work_on_first2.select    = form_work_on_first.select;
        form_work_on_first2.input     = form_work_on_first.input;
        form_work_on_first2.textarea  = form_work_on_first.textarea;
        form_work_on_first2.bytes_    = form_work_on_first.bytes_;
        form_work_on_first2.url_      = form_work_on_first.url_;
        form_work_on_first2.method_   = form_work_on_first.method_;
        form_work_on_first2.multipart_= form_work_on_first.multipart_;
}
///=================================================================================///


///======Get the first root of the current  url starting from right or from left====///
std::string Browser::get_first_root(bool start_from_host)
{
    std::string temp_url   ="";
    int backward_it        = 1;
    temp_url               = geturl();
    bool https             = false;
    // meaning we just need to add the docuement to the hostname.
    //remove the http:// to not confuse the slashes
    replaceAll(temp_url,"http://","");
    if( word_in(temp_url,"https://") ) {
        https = true;
        replaceAll(temp_url,"https://","");
    }

    //now test if we are in a directory
    //meaning something like:
    //www.something.com/   or
    //www.somthing.com/blah.php or
    //www.something.com/else/somthing.php
    if( word_in(temp_url,"/")) {
        //just find the first / in the temp url
        if (start_from_host) {
            backward_it = temp_url.find("/");
            if (!https) temp_url = "http://"+ temp_url.substr(0,backward_it+1);
            else        temp_url = "https://"+temp_url.substr(0,backward_it+1);
            if (temp_url[temp_url.size()-1]=='/') return temp_url.substr(0,temp_url.size()-1);
            else return temp_url;
        }
        else {
            while(temp_url[temp_url.size()-backward_it]!='/')
                backward_it++;
            //here we are on the last slash
            //prepare the first root URL for the form
            if(form.url_[0]!='/')
                if(!https)
                    temp_url = "http://" + temp_url.substr(0,temp_url.size()-backward_it+1);
                else
                    temp_url = "https://" + temp_url.substr(0,temp_url.size()-backward_it+1);
            else
                if(!https)
                    temp_url = "http://" + temp_url.substr(0,temp_url.size()-backward_it);
                else
                    temp_url = "https://" + temp_url.substr(0,temp_url.size()-backward_it);
        }
    }
    //meaning we don't have any slash, we are in the top
    //dir , so something like:
    //www.blahblah.com
    else {
        //here we concatenate all we need in this way:
        //http://www.blahblah.com/formurl.php
        if(!https)
            temp_url = "http://" + temp_url + "/";
        else
            temp_url = "https://" + temp_url + "/";
    }

    return temp_url;

}
///=================================================================================///


///===================submit the form depending on GET or POST=======================///
void Browser::submit(int timeout=30)
{
    std::string temp_url="";
    //get out of the program if we don't have a post or a get in the form
    assert(word_in(form.method_,"get") || word_in(form.method_,"post") );

    //if the url is already complete
    if( word_in(form.url_,"http://")|| word_in(form.url_,"http://") )
        temp_url = form.url();
    //otherwise we add after the root of dir
    //meaning after the first /
    //or if there's not, we add a slash and append it -- But it might not always be true
    //example: /postsome/shit/here/
    else {
        //here we need to add to the hostname
        if (form.url_[0] == '/')
            temp_url = get_first_root(true)+form.url_;
        else
            temp_url = get_first_root(false)+form.url_;
    }

    //we have the url where we will post or get set correctly
    //now prepare the get or post to do and submit
    if( word_in(form.method_,"get")) {
        //if it's a get
        //append ? to action then add value1=avalue&value2=anothervalue
        //then open the link and decide if it writes to a file or not depending on the write_bytes
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
        temp_url += "?";

        //loop through all that:
        //select   :name(), options[selected(),value()]
        //input    :name(), type(), value()
        //textarea :name(), value()
        //bytes    :strings...

        //select
        for(unsigned int ii=0;ii<form.select.size();ii++)
            for(unsigned int jj=0;jj<form.select[ii].options.size();jj++)
                if(form.select[ii].options[jj].value()!="" &&form.select[ii].options[jj].selected()==true) {
                    temp_url+=escape(form.select[ii].name());
                    temp_url+="=";
                    temp_url+=escape(form.select[ii].options[jj].value());
                    temp_url+="&";
                }

        //input
        for(unsigned int ii=0;ii<form.input.size();ii++)
            if(form.input[ii].value()!="") {
                temp_url+=escape(form.input[ii].name());
                temp_url+="=";
                temp_url+=escape(form.input[ii].value());
                temp_url+="&";
            }

        //textarea
        for(unsigned int ii=0;ii<form.textarea.size();ii++)
            if(form.textarea[ii].value()!="") {
                temp_url+=escape(form.textarea[ii].name());
                temp_url+="=";
                temp_url+=escape(form.textarea[ii].value());
                temp_url+="&";
            }
        //stil use open because we can have write_bytes as a callback
        open(temp_url,timeout);

    }
    //FOR POSTS NOW!
    else if( word_in(form.method_,"post") ) {
        //if it's a post
        //we check what part of the form is pure bin

        //input -- here we can have the type FILE
        //which always imply uploading a file
        for(unsigned int ii=0;ii<form.input.size();ii++) {
            if(form.input[ii].value()!="") {
                //check if bytes and content type were specified
                for( std::map<std::string,std::string>::iterator inside_bytes=form.bytes_.begin();
                     inside_bytes!=form.bytes_.end();
                     ++inside_bytes) {
                    //if names of the input is inside the bytes
                    if( (*inside_bytes).first ==  form.input[ii].name() ) {
                        //if we didn't specified a content type
                        if( (*inside_bytes).second == "" ) {
                            // Fill in the file upload field
                            curl_formadd(&formpost,
                                     &lastptr,
                                     CURLFORM_COPYNAME, form.input[ii].name().c_str(),
                                     CURLFORM_FILE, form.input[ii].value().c_str(),
                                     CURLFORM_END);
                        }
                        //specified a content type
                        else {
                            // Fill in the file upload field with the content type
                            curl_formadd(&formpost,
                                     &lastptr,
                                     CURLFORM_COPYNAME, form.input[ii].name().c_str(),
                                     CURLFORM_FILE, form.input[ii].value().c_str(),
                                     CURLFORM_CONTENTTYPE, (*inside_bytes).second.c_str(),
                                     CURLFORM_END);
                        }
                    }
                }
                //

                //if we have the type file then it's a file:
                if( word_in(form.input[ii].type(),"file") ) {
                    // Fill in the file upload field
                    curl_formadd(&formpost,
                                 &lastptr,
                                 CURLFORM_COPYNAME, form.input[ii].name().c_str(),
                                 CURLFORM_FILE, form.input[ii].value().c_str(),
                                 CURLFORM_END);
                }
                //else if it's pure text
                else {
                    // Fill in the submit field too, even if this is rarely needed
                    curl_formadd(&formpost,
                                 &lastptr,
                                 CURLFORM_COPYNAME, form.input[ii].name().c_str(),
                                 CURLFORM_COPYCONTENTS, form.input[ii].value().c_str(),
                                 CURLFORM_END);
                }
            }
        }
        //select
        for(unsigned int ii=0;ii<form.select.size();ii++)
            for(unsigned int jj=0;jj<form.select[ii].options.size();jj++)
                if(form.select[ii].options[jj].value()!=""
                    && form.select[ii].options[jj].selected()==true) {
                    curl_formadd(&formpost,
                                 &lastptr,
                                 CURLFORM_COPYNAME, form.select[ii].name().c_str(),
                                 CURLFORM_COPYCONTENTS, form.select[ii].options[jj].value().c_str(),
                                 CURLFORM_END);
                }
        //textarea
        for(unsigned int ii=0;ii<form.textarea.size();ii++)
            if(form.textarea[ii].value()!="") {
                curl_formadd(&formpost,
                             &lastptr,
                             CURLFORM_COPYNAME, form.textarea[ii].name().c_str(),
                             CURLFORM_COPYCONTENTS, form.textarea[ii].value().c_str(),
                             CURLFORM_END);
            }

        //add our prepared formpost to the options
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

        //stil use open because we can have write_bytes as a callback
        open_form(temp_url,timeout);
    }

}
///=================================================================================///


///===============SAVE THE HTML OUTPUT INTO THE RESPONSE STRING=====================///
size_t Browser::write_to_string(void *curl, size_t size, size_t count, void *response)
{
  ((std::string*)response)->append((char*)curl, 0, size*count);
  return size*count;
}
///=================================================================================///


///====================SAVE THE OUTPUT IN A BIN FILE================================///
size_t Browser::write_data(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t written;
    written = fwrite(ptr, size, nmemb, stream);
    return written;
}
///=================================================================================///


///===========================PREPARE FOR WRITING BYTES==============================///
void Browser::write_bytes(std::string filename)
{
    filepipe      = fopen(filename.c_str(),"wb");

    assert(filepipe!=NULL);
    //an error occured when opening the file for writting
    if(filepipe==NULL)
        fprintf (stderr, "[!] error writting the file: %s\n", strerror (errno));
    writing_bytes = true;
}
///=================================================================================///


///=========================LET US SEE THAT RESPONSE=================================///
////THIS MIGHT BECOME A CLASS IN THE FUTURE////
std::string Browser::response()
{
    return html_response;
}
///=================================================================================///


///==========================SET A USER-AGENT FOR THE BROWSER========================///
void Browser::adduseragent(std::string theuseragent)
{
    curl_easy_setopt(curl, CURLOPT_USERAGENT, theuseragent.c_str());
}
///=================================================================================///


///========================ADD HEADERS WITH OVERLOADING==============================///
void Browser::addheaders(std::string headers_to_add[2])
{
    std::string toaddhead = headers_to_add[0]+":"+headers_to_add[1];
    struct curl_slist *headers    = NULL;
    headers = curl_slist_append(headers, toaddhead.c_str());
    curl_slist_free_all(headers);
}
void Browser::addheaders(std::string header_to_add,std::string second_header_to_add)
{
    std::string toaddhead = header_to_add+":"+second_header_to_add;
    struct curl_slist *headers    = NULL;
    headers = curl_slist_append(headers, toaddhead.c_str());
    curl_slist_free_all(headers);
}
void Browser::addheaders(std::map<std::string, std::string> Headers)
{
    std::string toaddhead;
    struct curl_slist *headers    = NULL;
    for( std::map<std::string,std::string>::iterator ii=Headers.begin(); ii!=Headers.end(); ++ii) {
        toaddhead =  (*ii).first+ ":" + (*ii).second;
        headers = curl_slist_append(headers, toaddhead.c_str());
    }
    curl_slist_free_all(headers);
}
void Browser::addheaders(std::vector<std::string> Headers)
{
    std::string toaddhead;
    struct curl_slist *headers    = NULL;
    for(unsigned int i=0; i < Headers.size(); i+=2) {
        toaddhead =  Headers[i]+ ":" + Headers[i+1];
        headers = curl_slist_append(headers, toaddhead.c_str());
    }
    curl_slist_free_all(headers);
}
///=================================================================================///

///===============Follow a link in the page based on the name=======================///
void Browser::follow_link(std::string name_of_link_to_follow, int usertimeout=20)
{
    if(links.size()==0) {
        std::cerr<<"\n[!] No links found\n";
        return;
    }
    std::string to_follow="";
    for(int i=0;i<links.size();i++)
        if(links[i].name()==name_of_link_to_follow) {
            to_follow=links[i].url();
            break;
        }

    if(to_follow=="") {
        std::cerr<<"\n[!] No such link in the page found\n";
        return;
    }
    //if the link is already a complete one we open it directly else we add the site root
    if(word_in(to_follow,"http://"))
        open(to_follow,usertimeout);
    else {
        //here we are on the last slash
        if(to_follow[0]!='/')
            open(get_first_root(false)+to_follow);
        else
            open(get_first_root(true)+to_follow);
    }
}
///=================================================================================///

///====================RETURN TRUE IF THE STRING IS IN REPSONSE=====================///
bool Browser::inresponse(std::string str)
{
    if (html_response.find (str) != std::string::npos)
        return true;
    else
        return false;
}
///=================================================================================///


///================return the title of the page <title>TITLE</title>=================///
std::string Browser::title()
{
    std::vector <std::string> title_container;
    get_after_delimiter(html_response,"title",title_container);
    if(title_container.size()>0) {
        int start = title_container[0].find(">")+1;
        int stop  = title_container[0].find("<");
        return title_container[0].substr(start,stop-start);
    }
    return "";
}
///=================================================================================///


///======================return true if a string is in the title=====================///
bool Browser::intitle(std::string str)
{
    std::string current_title = title();
    if (current_title.find (str) != std::string::npos)
        return true;
    else
        return false;
}
///=================================================================================///


///=======================get all the cookies received in a string===================///
std::string Browser::getcookies()
{
    std::string allcookies = "";
    //set to 0 and not to NULL
    curl_slist * cookies   = 0 ;
    curl_easy_getinfo(curl,CURLINFO_COOKIELIST, &cookies);
    curl_slist * cur = cookies;
    while (cur) {
        allcookies+= cur->data;
        cur        = cur->next;
    }
    curl_slist_free_all(cookies);
    curl_slist_free_all(cur);
    return allcookies;
}
///=================================================================================///


///==========================allow redirection======================================///
void Browser::set_handle_redirect(bool allow)
{
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, allow);
    curl_easy_setopt(curl, CURLOPT_AUTOREFERER, allow);
}
///=================================================================================///


///============================set verbose to true==================================///
void Browser::set_verbose(bool allow)
{
    curl_easy_setopt(curl, CURLOPT_VERBOSE, allow);
}
///=================================================================================///


///============================set gzip encoding to true=============================///
void Browser::set_handle_gzip(bool allow)
{
    if(allow)
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip");
    else
        //remember that this one needs curl to be compiled with zlib
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "deflate");
    curl_easy_setopt(curl, CURLOPT_TRANSFER_ENCODING, allow);
}
///=================================================================================///


///==========================add cookies to the browser==============================///
void Browser::set_cookie(std::string cookies)
{
    curl_easy_setopt(curl, CURLOPT_COOKIE, cookies.c_str());
}
///=================================================================================///


///========================set the output of the cookies=============================///
void Browser::set_cookiejar(std::string cookiejar)
{
    curl_easy_setopt(curl, CURLOPT_COOKIEJAR, cookiejar.c_str());
}
void Browser::set_cookiejar()
{
    curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "-");
}
///=================================================================================///


///========================simply reload the page===================================///
void Browser::reload()
{
    if(geturl().length()>4) {
        std::string current_page = geturl();
        //don't reload a page that isn't a page
        assert(current_page.length()>7);
        open(current_page);
    }
    else
        std::cerr<<"\n[!] No page has been open yet\n";
}
///=================================================================================///


///=========================to set the verify peer to false=========================///
void Browser::set_handle_ssl(bool allow)
{
    if(!allow)
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_NONE);
    else
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_TRY);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, allow);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, allow);
}
///=================================================================================///


///============================Set a custom dns server===============================///
void Browser::set_dns(std::string dns_server)
{
    curl_easy_setopt(curl, CURLOPT_DNS_SERVERS, dns_server.c_str());
}
///=================================================================================///


///================Set a proxy (proxy:port) or unset it with the bool false=========///
void Browser::set_proxy(std::string proxy, std::string type="http")
{
    if(type=="http") {
        curl_easy_setopt(curl, CURLOPT_PROXY, proxy.c_str());
        curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP );
    }
    else if(type=="socks4") {
        curl_easy_setopt(curl, CURLOPT_PROXY, proxy.c_str());
        curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS4  );
    }
    else if(type=="socks5") {
        curl_easy_setopt(curl, CURLOPT_PROXY, proxy.c_str());
        curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5 );
    }
    else if(type=="socks4a") {
        curl_easy_setopt(curl, CURLOPT_PROXY, CURLPROXY_SOCKS4A );
    }
    else {
        curl_easy_setopt(curl, CURLOPT_PROXY, proxy.c_str());
        curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP );
    }
}
void Browser::set_proxy(bool allow)
{
    if(allow)
        std::cerr<<"\n[!] Need to pass the arguments \n";
    if(!allow)
        curl_easy_setopt(curl, CURLOPT_PROXY, "" );
}
///=================================================================================///


///======================HTTP tunneling through http proxy===========================///
void Browser::set_http_tunel(bool allow)
{
    curl_easy_setopt(curl, CURLOPT_HTTPPROXYTUNNEL, allow);
}
///=================================================================================///


///======================proxy Login identification=================================///
void Browser::set_proxy_login(std::string username, std::string passwd)
{
    std::string concat = username+":"+passwd;
    curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, concat.c_str());
}
///=================================================================================///


///===========Set the interface to use with a port or maybe a port range============///
void Browser::set_interface(std::string interface_name, long int port=80, long int max_port=80)
{
        curl_easy_setopt(curl, CURLOPT_INTERFACE, interface_name.c_str() );
        curl_easy_setopt(curl, CURLOPT_LOCALPORT, port );
        if(port<80)
            std::cerr<<"\n[!] Remember: With great power comes great responsabilities\n";
        if(max_port>80) {
            max_port = port + max_port-port+1;
            curl_easy_setopt(curl, CURLOPT_LOCALPORTRANGE, max_port );
        }
}
///=================================================================================///


///========================get the status response code=============================///
std::string Browser::status()
{
    int response_int;
    std::ostringstream response_str;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_int);
    response_str<<response_int;
    response_str.str()="";
    response_str.clear();
    return response_str.str();
}
///=================================================================================///


///======================OPTIONS RELATED TO THE CONGESTION==========================///
//in bytes normally, now * 1000 so in kbs
void Browser::limit_speed(int limit)
{
    assert(limit>0 && limit<30000);
    limit = limit *1000;
    if(limit>0 && limit<30000) {
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, limit);
        curl_easy_setopt(curl, CURLOPT_MAX_SEND_SPEED_LARGE, limit);
        curl_easy_setopt(curl, CURLOPT_MAX_RECV_SPEED_LARGE, limit);
    }
    else
        std::cerr<<"\n[!] Can't set the time limit \n";
}
//relevant only if limit_speed is set up
//in seconds
void Browser::limit_time(int limit)
{
    assert(limit>0);
    if(limit>0)
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, limit);
    else
        std::cerr<<"\n[!] Can't set the limit \n";
}
///=================================================================================///


///=============================Escape/Unescape strings==============================///
std::string Browser::escape(std::string the_string)
{
    std::string output;
    output = curl_easy_escape( curl, the_string.c_str(), 0 );
    return output;
}
std::string Browser::unescape(std::string the_string)
{
    std::string output;
    output = curl_easy_escape( curl, the_string.c_str(), 0 );
    return output;
}
///=================================================================================///


///===========================do a header only request===============================///
void Browser::head_request(bool allow)
{
    curl_easy_setopt(curl, CURLOPT_NOBODY, allow);
}
///=================================================================================///


///===========return a curl handle to use outside of the class======================///
CURL *Browser::get_handle()
{
    return &curl;
}
///=================================================================================///


///====Use this function to directly create a form out of the blue to post it=======///
void Browser::set_direct_form_post(bool direct,std::string url = "")
{
    if(url=="")
        form.direct_post           = false;
    else
        form.direct_post           = direct;

    //if it's a direct post we set the url and the method
    if(form.direct_post==true) {
        form.url_    = url;
        form.method_ = "POST";
    }
}
///=================================================================================///


///===========================Use HTTP 1.0 instead of 1.1===========================///
void Browser::set_http_version_1_0(bool set_it)
{
    if(set_it)
        curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
    else
        curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
}
///=================================================================================///


///=========================Clear the history accumulated over time==================///
void Browser::clear_history()
{
    history_.clear();
}

std::vector<std::string> Browser::get_history()
{
    return history_;
}

///=================================================================================///


///===========================Show the current history===============================///
void Browser::history()
{
    for(unsigned int ii=0;ii<history_.size();ii++)
        std::cout<<history_[ii]<<"\n";
}
///=================================================================================///


///==============================return the current url=============================///
std::string Browser::geturl()
{
    char * current_url;
    curl_easy_getinfo(curl,CURLINFO_EFFECTIVE_URL, &current_url);
    return current_url;
}
///=================================================================================///


///==============================check if in url====================================///
bool Browser::inurl(std::string str)
{
    std::string current_url = geturl();
    if(current_url=="")
        std::cerr<<"\n[!] No page in history\n";
    if (current_url.find (str) != std::string::npos)
        return true;
    else
        return false;
}
///=================================================================================///


///============================Go back in history===================================///
void Browser::back(int timeout=20)
{
    assert(history_.size()>0);
    //remove the last page
    //visit the page before the last page we opened
    //but don't save that we visited it
    history_.pop_back();
    open(history_[history_.size()-1],timeout,false);

}
///=================================================================================///


///===========return true if html response is not empty=============================///
bool Browser::viewing_html()
{
    if(response().length()>12)
        return true;
    else
        return false;
}
///=================================================================================///

///===============Init the emails from the links already fetched====================///
void Browser::emails_class::init(links_class links)
{
    //clear all the emails in the page before adding the new ones
    all_emails.clear();

    //loop through the links to see if there's contact informations
    for(int i=0; i<links.size(); i++)
        //search for the word : "mailto:"
        if(word_in(links[i].url(), "mailto:")) {
            std::string fake_temp_shit = links[i].url();
            replaceAll(fake_temp_shit,"mailto:","");
            all_emails.push_back(fake_temp_shit);
        }
}
///=================================================================================///

///==========================return the contact information=========================///
std::string Browser::emails_class::operator[ ]  (int ite)
{
    if(ite>size()) {
        std::cerr<<"\n[!] No Such Email in the page\n";
        return "";
    }
    return all_emails[ite];
}
///=================================================================================///

std::string Browser::emails_class::all()
{
    std::string output="";
    for(int kk=0;kk<size();kk++) {
        output+=all_emails[kk];
        output+= "\n";
    }
    if(output=="")
        std::cerr<<"\n[!] No Emails in the page\n";
    return output;
}

#endif // MECHANIZE_HPP_INCLUDED
