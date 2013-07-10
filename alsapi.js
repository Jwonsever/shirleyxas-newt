// alsapi.js

var alsapi_server="https://portal-auth.nersc.gov";
var alsapi_base_path="/als2";
var alsapi_base_url=alsapi_server+alsapi_base_path;
var stayInDOM = true;

var IE = document.all?true:false;

jQuery.extend({
   alsapi_ajax : function(s) {
       var myurl;
       // Do some sanity checking on URL - strip off https hostname and leading /alsapi
       // Do we want to check for alsapi_base_url??
       if (myurl=s.url.match(/^https?:\/\/[^\/]*(\/.*)/)) {
           s.url=myurl[1];           
       }
       
       // TODO: Match against alsapi_base_path
       var re=new RegExp("^"+alsapi_base_path+"(.*)");
       if (myurl=s.url.match(re)) {
           s.url=myurl[1];
       }

       // Set contentType string to json for "store" queries
       if (s.url.match(/^\/store/) && s.data) {
           s.contentType="application/json; charset=utf-8";           
       }

       if (!s.url.match(/^\/file.*view=read$/)) {
           s.dataType='json';
       }
          
       // FIXME: Deal width jquery issue that calls success on "0" error code

       //alert("In alsapi "+alsapi_base_url+s.url);

       // Merge the existing settings with NEWT boilerplate options
       var ajax_settings=$.extend({},s,{
              url : alsapi_base_url+s.url,
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

function checkALSAuth(){
        $.alsapi_ajax({type: "GET",
                url: "/auth",async: true,
                success: function(res){
                        if (!res.auth) {
                          alert("Not logged in to ALS")
                        }
                        else {
                          var myALSUsername = res.username.toLowerCase();
                          //alert("Logged in to ALS as "+myALSUsername)
                        }
                },
                error: function(res,testStatus,errorThrown) {
                        console.log("CheckAuth error: " + testStatus + "  " + errorThrown + " " + res.error);
                },
        });
}
