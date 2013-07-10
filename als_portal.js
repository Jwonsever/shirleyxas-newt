
function myCheckAuth() {
    $.newt_ajax({type: "GET",
    url: "/login",
    success: function(res){
        var tmpText = "";
        if (!res.auth) {
            myUsername = "invalid";
            $('#auth-spinner').hide();
            $("#logout-area").hide();
            $("#login-area").show();
        } else {
            myUsername = res.username.toLowerCase();
            myCheckAuthALS();
        }
    },
    error: function(request,testStatus,errorThrown) {
        console.log("Error "+errorThrown)
        $('#auth-spinner').hide();
        $("#logout-area").hide();
        $("#login-area").show();
    },
    });
}

function myCheckAuthALS() {
    $.alsapi_ajax({type: "GET",
    url: "/login",
    success: function(res){
        var tmpText = "";
        var myTmpUsername = "";
        if (res.auth) myTmpUsername = res.username.toLowerCase();
        if (!res.auth || myTmpUsername != myUsername) {
            myUsername = "invalid";
            $('#auth-spinner').hide();
            $("#logout-area").hide();
            $("#login-area").show();
            checkAuthCallback()
       } else {
            tmpText="<font color=white>Welcome "+myUsername+"&nbsp;&nbsp;&nbsp;</font>";
            $('#auth-spinner').hide();
            $("#username-area").html(tmpText);
            $("#username-area").trigger('create');
            $("#login-area").hide();
            $("#logout-area").show();
            checkAuthCallback()
        }
    },
    error: function(request,testStatus,errorThrown) {
        console.log("Error "+errorThrown)
        $('#auth-spinner').hide();
        $("#logout-area").hide();
        $("#login-area").show();
    },
    });
}

//function doAfterLogin() {
//    myCheckAuth();
//}

//function doAfterLogout() {
//    $('#auth-spinner').hide();
//    myCheckAuth();
//}

function doAfterFailure() {
    //alert("In doAfterFailure ")
    $('#auth-spinner').hide();
    myCheckAuth();
}

function openWindow(fileName) {
    OpenWindow=window.open("", "newwin", "status,resizable,scrollbars");
    OpenWindow.document.write("<html><title>"+fileName+"</title>")
    OpenWindow.document.write("<body>")
    OpenWindow.document.write("<img src='"+fileName+"' width='1000'/>")
    OpenWindow.document.write("</body></html>")
    OpenWindow.document.close()
    self.name="main"
}

var submission = function(){
    var username=$('#id_username').val();
    var password=$('#id_password').val();

    $('#auth-spinner').show();
    $('#login-area').hide();

    $.newt_ajax({
        type:'POST',
        url:"/auth",
        data:{'username':username, 'password':password},
        success:function(res, textStatus, jXHR) {
            //alert("Logged in Once")
            submissionALS()
        }
    });
}

var submissionALS = function(){
    var username=$('#id_username').val();
    var password=$('#id_password').val();

    $.alsapi_ajax({
        type:'POST',
        url:"/auth",
        data:{'username':username, 'password':password},
        success:function(res, textStatus, jXHR) {
            //alert("Logged in Twice")
            myCheckAuth()
        }
    });
}

var logout = function(){
    $.newt_ajax({url:"/logout",
        success:function() {
            logoutALS()
            //Set_Cookie( 'hide', '0', '', '/', '', '' );
    }});
}

var logoutALS = function(){
    $.alsapi_ajax({url:"/logout",
        success:function() {
            //Set_Cookie( 'hide', '0', '', '/', '', '' );
            myCheckAuth()
    }});
}

function listusers() {
    if (myUsername === "invalid") {
      $("#userlist").html("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Please Login.<br><br>");
    } else {
      $("#userlist").html('<center><img src=\"blue-ajax-loader.gif\"></center>');
      //alert("in list users")
      $.alsapi_ajax({type: "GET",
        url: "/usermanagement/seemap",
        success: function(res){
          var tmpText="<table class=table><tr><th>NERSC Username</th><th>ALS Username</th><th></th></tr>";
          $.each(res, function(i,item){
            tmpText+="<tr><td>"+item.nerscname+"</td><td>"+item.alsname+"</td>"
            tmpText+="<td><button class=\"btn btn-danger\" onclick=\"deletemap(\'"+item.nerscname+"\');\">Delete</button></td></tr>"
          });
          tmpText+="</table>";
          $("#userlist").html(tmpText);
        },
        error: function(res){
            $("#userlist").html("Failed<br><br>");
            console.log(res.error)
        }
      })
    }
}

function createmap() {
  $("#userlist").html('<center><img src=\"blue-ajax-loader.gif\"></center>');
  var nerscname = $("#nerscname").val()
  var alsname = $("#alsname").val()
  //alert("createmap "+nerscname+" "+alsname)
    $.alsapi_ajax({type: "GET",
        url: "/usermanagement/createmap?nerscname="+nerscname+"&alsname="+alsname,
        success: function(res){
            listusers()
        },
        error: function(res){
            console.log(res.error)
            listusers()
        }
    })
}

function deletemap(nerscname) {
  $("#userlist").html('<center><img src=\"blue-ajax-loader.gif\"></center>');
  //alert("createmap "+nerscname+" "+alsname)
    $.alsapi_ajax({type: "GET",
        url: "/usermanagement/deletemap?nerscname="+nerscname,
        success: function(res){
            listusers()
        },
        error: function(res){
            console.log(res.error)
        }
    })
}

function nWC(x) {
  return x.toString().replace(/\B(?=(\d{3})+(?!\d))/g, ",");
}

