function diskUsage() {
    if (myUsername === "invalid") {
      $("#disk_usage").html("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Please Login.<br><br>");
    } else {
      $("#disk_usage").html('<center><img src=\"blue-ajax-loader.gif\"></center>');
      $.newt_ajax({type: "POST",
        url: "/command/hopper",
        data: {"executable": "/global/common/shared/usg/bin/mynersc_myprjquota_wrapper.sh"},
        success: function(res_raw){
          res = JSON.parse(res_raw.output); 
          percentused = parseInt(res['als']['Space']['Used']*100/res['als']['Space']['Quota']) 
          percentfree = 100 - percentused
          var tmpText = "<center><br><table width=80%><tr><td width=100%><div class=\"progress\">"
          tmpText+="<div class=\"bar bar-danger\" style=\"width: "+percentused+"%;\">"+ nWC(Math.round(res['als']['Space']['Used']))+" GB</div>"
          tmpText+="<div class=\"bar bar-success\" style=\"width: "+percentfree+"%;\">"+ nWC(Math.round(res['als']['Space']['Free']))+" GB</div>"
          tmpText+="</div></tr></td></table></center>"
          $("#disk_usage").html(tmpText);
        },
        error: function(res){
          $("#disk_usage").html("Query Failed<br><br>");
          console.log(res.error)
        }
      })
    }
}

