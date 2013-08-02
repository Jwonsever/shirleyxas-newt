// newt.js

var newt_server="https://newt.nersc.gov";
var newt_base_path="/newt";
var newt_base_url=newt_server+newt_base_path;
var stayInDOM = true;

var newt_user={
    username:null,
    auth:false,
    session_lifetime:0
}

var IE = document.all?true:false;

jQuery.extend({
   newt_ajax : function(s) {
       var myurl;
       // Do some sanity checking on URL - strip off https hostname and leading /newt
       // Do we want to check for newt_base_url??
       if (myurl=s.url.match(/^https?:\/\/[^\/]*(\/.*)/)) {
           s.url=myurl[1];           
       }
       
       // TODO: Match against newt_base_path
       var re=new RegExp("^"+newt_base_path+"(.*)");
       if (myurl=s.url.match(re)) {
           s.url=myurl[1];
       }

       // Set contentType string to json for "store" queries
       if (s.url.match(/^\/store/) && s.data) {
           s.contentType="application/json; charset=utf-8";           
       }

       if (!s.url.match(/^\/file.*view=read$/) && !s.url.match(/^\/status.*view=read$/)) {
           s.dataType='json';
       }
          
       // FIXME: Deal width jquery issue that calls success on "0" error code

       // Merge the existing settings with NEWT boilerplate options
       var ajax_settings=$.extend({},s,{
              url : newt_base_url+s.url,
              beforeSend : function(xhr){
                      // make cross-site requests
                      xhr.withCredentials = true;

              },
              crossDomain : true,
              cache : false,
              // For jquery 1.5
              xhrFields: { withCredentials: true },
          });
       $.ajax(ajax_settings);                   
   }
});

