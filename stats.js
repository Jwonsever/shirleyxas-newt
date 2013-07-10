function getStats() {
    if (myUsername === "invalid") {
      $("#stats").html("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Please Login.<br><br>");
    } else {
      $("#stats").html('<center><img src=\"blue-ajax-loader.gif\"></center>');
      $.alsapi_ajax({type: "GET",
        url: "/hdf/stats",
        success: function(res){
          //alert("Success "+res['number'])
          totalFiles=res['number']
          var tmpText = "<table cellpadding=4>"
          tmpText += "<tr><td align=right>Number of Datasets:</td><td>"+totalFiles+"</td></tr>";
          tmpText += "</table>"
          $("#stats").html(tmpText);
        },
        error: function(res){
          $("#stats").html("Query Failed<br><br>");
          console.log(res.error)
        }
      })
    }
}

