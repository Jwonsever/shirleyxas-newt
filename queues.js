function createJobsTable() {
    if (myUsername === "invalid") {
      $("#jobstatus").html("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Please Login.<br><br>");
    } else {
      $("#userlist").html('<center><img src=\"blue-ajax-loader.gif\"></center>');
      //alert("in list users")
      $.alsapi_ajax({type: "GET",
        url: "/hdf/jobs?page=1&pagelimit=200",
        success: function(res){
          var myText = "";
          var myArray = new Array();
          var j=0;
          $.each(res, function(i,item){
            if ('qtime' in item) {
              //myText+=i+" "+j+" "+item['startdate'].replace("T"," ").replace("Z","")+" "+item['qtime']+"<br>"
              myArray[j]=new Array(item['startdate'].replace("T"," ").replace("Z",""),parseInt(item['qtime']))
              j++
            }
          });
          myText+="<div id='plot'></div>"
          $("#jobstatus").html(myText);
          makePlot(myArray)
        },
        error: function(res){
            $("#jobstatus").html("Query Failed<br><br>");
            console.log(res.error)
        }
      })
    }
}

function makePlot(myArray) {
    $('#plot').html(" ");
    $('#plot').trigger('create');
    var rangePlot = $.jqplot('plot', [myArray] , {
        title: 'Queue Time Over Time',
        seriesDefaults: {    
                  showLine:false
                },
        axes: {
            xaxis: {
                renderer:$.jqplot.DateAxisRenderer,
                //tickOptions:{formatString:'%m %d'},
                //min: myArray[0][0],
                //tickInterval: '2 hours',
                //pad: 0
            },
            yaxis: {
                //pad: 1.1,
                //min: 0,
                tickOptions: {
                  formatString: '%d'
                }
            }
        }
    });
}

