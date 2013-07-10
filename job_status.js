var page=1
var pagelimit=50

function createJobsTable() {
    if (myUsername === "invalid") {
      $("#jobstatus").html("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Please Login.<br><br>");
    } else {
      $("#jobstatus").html('<center><img src=\"blue-ajax-loader.gif\"></center>');
      //alert("in list users")
      $.alsapi_ajax({type: "GET",
        url: "/hdf/jobs?page="+page,
        success: function(res){
          var tmpText="<table class=table><tr wdith=100%><th width=49%>Dataset</th><th width=39%>Date</th><th width=9%>Status</th><th></th></tr></table>";
          tmpText+="<div class=\"accordion\" id=\"accordion2\">"
          var i=0
          $.each(res, function(i,item){
            i++
            tmpText+="<div class=\"accordion-group\">"
            if (item.status == "FAIL") {
              tmpText+="<div class=\"accordion-heading\" style=\"background-color:#f2dede;\">"
              tmpText+="<a class=\"accordion-toggle\" data-toggle=\"collapse\" data-parent=\"#accordion2\" href=\"#collapse"+i+"\">"
            } else if (item.status == "OK") {
              tmpText+="<div class=\"accordion-heading\" style=\"background-color:#dff0d8;\">"
              tmpText+="<a class=\"accordion-toggle\" data-toggle=\"collapse\" data-parent=\"#accordion2\" href=\"#collapse"+i+"\">"
            } else {
              tmpText+="<div class=\"accordion-heading\" style=\"background-color:#fcf8e3;\">"
              tmpText+="<a class=\"accordion-toggle\" data-toggle=\"collapse\" data-parent=\"#accordion2\" href=\"#collapse"+i+"\">"
            }
            tmpText+="<table width=100%><tr width=100%><td width=50%><h5>"+item.dataset+"</h5></td><td width=40%><h5>"+item.startdate+"</h5></td><td width=10%><h5>"+item.status+"</h5></td></tr></table>"
            tmpText+="</a>"
            tmpText+="</div>"
            tmpText+="<div id=\"collapse"+i+"\" class=\"accordion-body collapse\">"
            tmpText+="<div class=\"accordion-inner\">"
            tmpText+="<h5>Info:</h5><table cellpadding=5>"
            tmpText+="<tr><td align=right>Start Date:</td><td>"+item.startdate+"</td></tr>"
            tmpText+="<tr><td align=right>Raw File Size:</td><td>"+nWC(Math.round(item.h5RawFileSize/1024/1024))+" MB</td></tr>"
            tmpText+="<tr><td align=right>Job ID:</td><td>"+item.jobid+"</td></tr>"
            tmpText+="<tr><td align=right>Queue Time:</td><td>"+item.qtime+"</td></tr>"
            tmpText+="<tr><td align=right>Wall Time:</td><td>"+item.wtime+"</td></tr>"
            tmpText+="</table><h5>Progress:</h5><table cellpadding=1 width=100% valign=middle>"
            $.each(item.jobs, function(j,jtem){
              j++
              if (jtem.status == "FAIL") {
                tmpText+="<tr><td align=right>"+jtem.jobtype+":</td><td><font color=red>&nbsp;&nbsp;"+jtem.status+"</td><td width=80%></td></tr>"
              } else if (jtem.status == "OK") {
                qtimepercent=parseInt(jtem.qtime*100/(jtem.qtime+jtem.wtime))
                wtimepercent=100-qtimepercent
                ttimepercent=parseInt((jtem.wtime+jtem.qtime)*100/(item.wtime))+2
                ttime = jtem.qtime+jtem.wtime
                tmpText+="<tr><td align=right valign='top'>"+jtem.jobtype+":</td><td valign='top'>&nbsp;&nbsp;<font color=green>"+jtem.status+"</font></td>"
                tmpText+="<td valign=top>"+ttime+" sec</td><td width=80% valign='bottom'><div class=\"progress\" style=\"width: "+ttimepercent+"%;\">"
                tmpText+="<div class=\"bar bar-warning\" style=\"width: "+qtimepercent+"%;\"></div>"
                tmpText+="<div class=\"bar bar-success\" style=\"width: "+wtimepercent+"%;\"></div>"
                tmpText+="</div></td>"
                tmpText+="</tr>"
              } else {
                tmpText+="<tr><td align=right valign='top'>"+jtem.jobtype+":</td><td>&nbsp;&nbsp;Running<br>&nbsp;</td><td></td><td width=80%></td></tr>"
              }
            });
            tmpText+="</table>"
            tmpText+="</div></div></div>"
          });
          //tmpText+="<br><input id=\"clickMe\" type=\"button\" value=\"Older\" class=\"btn\" onClick=\"nextPage()\;\" />"
          tmpText+="<br><ul class=\"pager\">"
          tmpText+="<li class=\"previous\" onClick=\"nextPage()\;\"><a href=\"#\">&larr; Older</a></li>"
          tmpText+="<li class=\"next\" onClick=\"previousPage()\;\"><a href=\"#\">Newer &rarr;</a></li>"
          tmpText+="</ul>"
          tmpText+="</div>";
          $("#jobstatus").html(tmpText);
        },
        error: function(res){
            $("#jobstatus").html("Query Failed<br><br>");
            console.log(res.error)
        }
      })
    }
}

function nextPage() {
    page=page+1;
    createJobsTable();
}

function previousPage() {
    if (page != 1) {
      page=page-1;
      createJobsTable();
    }
}
