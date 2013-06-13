
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

