	
function itemClickedCallback(file,group,phyloc,type) {

    $("#attributes").html("<center><img src='blue-ajax-loader.gif' width=40></center>")    

    //alert("Item clicked "+file+" "+group)

    if (type == 'file'){
        overview(phyloc, file)
        $("#hiddenattributes").hide()
        $("#option").show()
    } else if (type == 'group') {
        $('#hiddenattributes').hide()
        $("#option").show()
        getAttributes(phyloc, group)
    } else if (type == 'dataset') {
        image = group.slice(group.lastIndexOf('/') + 1)
        $('#hiddenattributes').hide()
        $("#option").hide()
        getImage(phyloc, group, image)
    }

}

function overview(file, fileName) {
    getAttributes(file,"/")
}

function datasetOverview(dataset) {

    $.alsapi_ajax({type: "GET",
        url: "/hdf/dataset"+dataset,
        success: function(res){
        
        },
        error: function(res){
            console.log(res.error)
        }
    })
}

function getAttributes(file, group) {
    imfirst = true;

    //alert(file)
    //alert("URL "+"/hdf/attributes"+file+"?group="+group)
 
    file = $.trim(file)
    group = $.trim(group)
    $.alsapi_ajax({type: "GET",
        url: "/hdf/attributes"+file+"?group="+group,
        success: function(res){

            myArray = ["facility", "end_station", "owner", "senergy", "obstime", "pxsize", "nangles", "archdir", "experimenter", "sdate", "nslices", "nrays", "arange", "errorFlag"]

            var myText = '<table width=100% border=0 padding=0><tr width=100%><td valign=top>'
            myText += '<div id="Header" style="font-size:30px">Information</div>'
            myText += '</td>'
            myText += '<td align=right>'
            myText += '<div id="dlbuttons">'
            myText += '<button id="download" class="btn" onClick=downloadURL("'+alsapi_base_url+'/hdf/download'+file+'") style="width: 200px">Download H5 File</button>'
            myText += '<div id="dummy" style="width:200px;height:5px;"></div>'
            myText += '<div id="downloading-raw">'
            if (group == "/") {
              myText += '<button id="download" class="btn" onClick=downloadZip("'+file+'","'+group+'","#downloading-raw","raw") style="width: 200px">Download Zip File</button>'
            } else {
              myText += '<button id="download" class="btn" onClick=downloadZip("'+file+'","'+group+'","#downloading-raw","raw") style="width: 200px">Download Group Images</button>'
            }
            myText += '</div>'
            myText += '</div>'
            myText += '<div id="progresscon-raw" class="progress progress-striped active" style="width: 200px;display:none;">'
            myText += '<div id="progressbar-raw" class="bar" style="width: 1%;"></div>'
            myText += '</div>'
            myText += '</td>'
            myText += '</tr></table>'

            if (group == "/") {
              myText += '<div id="images">'
              myText += '<h3>raw images</h3><div id="images-raw"></div>'
              myText += '</div>'
              //myText += '<div id="images-imgrec"></div>'
            }

            myText += '<table cellpadding=5>'
            myHidden = '<table cellpadding=5>'
            for (var x in res) {
                var myBool=0
                for (var y in myArray){
                    if (x == myArray[y] || res.group == "/"){
                        myText += '<tr><td align=right><b>' + x + ':</b></td><td align=left>' + wordwrap(res[x],50,'<br>\n',true) + '</td></tr>'
                        myBool=1
                        break;
                    }
                }
                if (myBool == 0) {
                    myHidden += '<tr><td align=right><b>' + x + ':</b></td><td align=left>' + wordwrap(res[x],50,'<br>',true) + '</td></tr>'
                }
            }

            myText += '</table>'
            myHidden += '</table>'

            $("#attributes").html(myText)
            $("#hiddenattributes").html(myHidden)
            
            imfirst_img = true;
            if (group == "/") {
              showImages(file,group,'raw')
              otherDatasetFiles(res.dataset)
            }


        },
        error: function(res){
            console.log(res.error)
        }
    })
}

function otherDatasetFiles(dataset) {
    //alert("In otherDatasetFiles "+dataset+" /hdf/dataset?dataset="+dataset)
    $.alsapi_ajax({type: "GET",
        url: "/hdf/dataset?dataset="+dataset,
        success: function(res){
           $.each(res, function(i,item){
              //alert("stage "+item.stage+" "+item.stage_date)
              //if (item.stage != "raw" && item.stage != "imgrec") {
              if (item.stage != "raw") {
                //alert("Found another set "+item.stage)
                var uniqueDate = item.stage_date.replace(':','')
                var myText = '<table width=100%><tr width=100%><td><h3>'+item.stage+' images: ('+item.stage_date.replace('Z','').replace('T',' ')+')</h3></td>'
                myText += '<td align=right>'
                myText += '<br><button class="btn" onClick=downloadURL("'+alsapi_base_url+'/hdf/download'+item.path+'") style="width: 200px">Download H5 File</button><br>'

                if (item.stage == "imgrec" || item.stage == "gridrec") {
                  myText += '<div id="render'+item.stage+uniqueDate+'"><button id="btnrender-'+item.stage+uniqueDate+'" class="btn" onClick=openWindowURL("volume-rendering/render.php?dataset='+item.name+'") style="width: 200px">Volume Rendering</button></div>'
                }

                //myText += '<div id="downloading-'+item.stage+uniqueDate+'"><button id="download-'+item.stage+uniqueDate+'" class="btn" onClick=downloadZip("'+item.path+'","/","#downloading-'+item.stage+uniqueDate+'","'+item.stage+uniqueDate+'") style="width: 200px">Download Zip File</button></div>'
                myText += '<div id="progresscon-'+item.stage+uniqueDate+'" class="progress progress-striped active" style="width:200px;display:none;">'
                myText += '<div id="progressbar-'+item.stage+uniqueDate+'" class="bar" style="width: 1%;"></div>'
                myText += '</div><br></td></tr></table>'
                myText += '<div id="images-'+item.stage+uniqueDate+'"><center><img src=blue-ajax-loader.gif width=35></center><br><br></div>'
                $("#images").append(myText)
                showImages(item.path,"/",item.stage+uniqueDate)
              } else if (item.stage == "imgrec") {
                showImages(item.path,"/",item.stage)
              }
           });
        },
        error: function(res){
            console.log(res.error)
        }
    })
}

function getImage(file, image, imageName) {
    $.alsapi_ajax({type: "POST",
        url: "/hdf/image"+file+"?group="+image,
        success: function(res){
            var mylocation = res.location
            var myname = res.filename
            var mythumb = res.thumbname
            var thumbSource = alsapi_base_url+"/hdf/downloadstaged/" + mylocation + "/" + mythumb
            var imageSource = alsapi_base_url+"/hdf/downloadstaged/" + mylocation + "/" + myname

            var i = 0;
            var str = '<table cellpadding=5>'
            for (var x in res) {
                if ( x != "thumbname" && x != "filename") {
                  str += '<tr><td align=right><b>' +x + ':</b></td><td align=left>' + wordwrap(res[x],50,'<br>',true) + '</td></tr>'
                  i++
                }
            }
            str += "</table>"

            //$("#attributes").html("<center><a href='javascript:void(0)'><img src='"+ thumbSource + "' onClick='openWindow(\"" + imageSource + "\")'/></a></center><br>" + str);
            //alert("In image attributes openModal(\""+res['name']+"\",\"" + imageSource + "\")")
            $("#attributes").html("<center><a href='javascript:void(0)'><img src='"+ thumbSource + "' onClick='openModal(\""+res['name']+"\",\"" + imageSource + "\")'/></a></center><br>" + str);
            //$("#attributes").html("<center><img src='"+ thumbSource + "' onClick='openModal(\""+res['name']+"\",\"" + imageSource + "\")'/></center><br>" + str);
        },
        error: function(res){
            alert(res.error)
        }
    });
}

function wordwrap( str, width, brk, cut ) {

    brk = brk || '\n';
    width = width || 75;
    cut = cut || false;

    if (!str) { return str; }

    var regex = '.{1,' +width+ '}(\\s|$)' + (cut ? '|.{' +width+ '}|.+$' : '|\\S+?(\\s|$)');

    return str.match( RegExp(regex, 'g') ).join( brk );

}

function openModal(imageName,imageSource) {

    //alert("In openModal "+imageSource);
    $('#imageModalLabel').html(imageName);
    $('#imageModalBody').html("<img src=\""+imageSource+"\">");
    $('#imageModal').modal('show');

}

function openWindow(imageName,imageSource) {
  myWindow = window.open('', 'header', 'menubar=0', 'toolbar=0');
  myWindow.document.write("<img src=\""+imageSource+"\">");
}

$("#option").change(function() {
  //alert("option changed");
  if ($('#watch-me').is(':checked')) {
    //alert("checked!")
    $('#hiddenattributes').show();
  } else {
    //alert("Not checked!")
    $('#hiddenattributes').hide();
  }
});

//$(function () {
//  $("[rel='tooltip']").tooltip();
//});

var downloadURL = function downloadURL(url)
{
    //alert(url)
    var iframe;
    iframe = document.getElementById("hiddenDownloader");
    if (iframe === null)
    {
        iframe = document.createElement('iframe');
        iframe.id = "hiddenDownloader";
        iframe.style.visibility = 'hidden';
        document.body.appendChild(iframe);
    }
    iframe.src = url;
}

function downloadZipWrap(filePath, group,myDiv,myStage) {
    if (!imfirst) {
        downloadZip(filePath,group,myDiv,myStage)
    }
}

function showImagesWrap(filePath, group, imgtype) {
    if (!imfirst_img) {
        showImages(filePath,group,imgtype)
    }
}

function showImages(filePath, group, imgtype) {

    console.log('CreatingGal '+filePath)

    var file = $.trim(filePath)
    group = $.trim(group)
    if (imfirst_img) {
      var loaderText="<center><img src=blue-ajax-loader.gif width=35></center><br><br>"
      $("#images-"+imgtype).html(loaderText)
      $("#images-"+imgtype).trigger('create')
    }
    imfirst_img = false;
    $.alsapi_ajax({type: "GET",
    url: "/hdf/creategallery"+file+"?group="+group,
    success: function(res){
        console.log("showImages success "+res.status.state)
        if (res.status.state == 'Running'){
            var myText = "<center><img src=blue-ajax-loader.gif width=35></center><br><br>";
            $("#images-"+imgtype).html(myText)
            //var bar = document.getElementById("imageprogressbar-"+imgtype);
            //$("#imagesprogresscon-"+imgtype).show();
            //bar.style.width = res.status.complete +"%";
            if (res.status.complete < 100) {
               setTimeout(function() { showImagesWrap(file,group,imgtype)}, 2000);
            }
        } else if (res.status.state == 'Complete'){
            //var myText = "Gallery complete "+res.img_url
            var myText = '<ul class="bxslider" id=\"slider-'+imgtype+'\">';
            var myimage = res.img_url+"/animated.gif";
            //alert(myimage)
            nitems = res.nitems;
            if (nitems > 10) {
              nitems=10
              myText += '<li><img src="'+myimage+'" /></li>';
            }
            for (j = 1 ; j <= nitems ; j++) {
              myimage = res.img_url+"/img"+j+"_thumb.png";
              myText += '<li><a href=\'javascript:void(0)\'><img src="'+myimage+'" onClick=\'downloadURL(\"'+res.img_url+'/img'+j+'.tif'+'\")\'/></a></li>';
            }
            myText += '</ul>';

            $("#images-"+imgtype).html(myText)

            var slider;

            if (imgtype == "raw" || imgtype.indexOf("norm") != -1) {
              slider=$('#slider-'+imgtype).bxSlider({
                infiniteLoop: false,
                mode: 'horizontal',
                slideMargin: 5
              });
            } else {
              slider=$('#slider-'+imgtype).bxSlider({
                infiniteLoop: false,
                mode: 'vertical',
                slideMargin: 5
              });
            }

            slider.goToSlide(0);

        } else if (res.status.state == 'Staging') {
            var myText = "Staging File From Tape <center><img src=blue-ajax-loader.gif width=35></center><br><br>";
            $("#images-"+imgtype).html(myText)
            //var bar = document.getElementById("imageprogressbar-"+imgtype);
            //$("#rawimagesprogresscon").show();
            //bar.style.width = "100%";
            if (res.status.complete < 100) {
               setTimeout(function() { showImagesWrap(file,group,imgtype)}, 10000);
            }           
        } else if (res.status.state == 'Unknown') {
            if (res.status.complete < 100) {
               setTimeout(function() { showImagesWrap(file,group,imgtype)}, 2000);
            }           
        } else {
            if (res.status.complete < 100) {
               setTimeout(function() { showImagesWrap(file,group,imgtype)}, 2000);
            }           
        }
    },error: function(res){
        if (!imfirst_img) {
          setTimeout(function() { showImagesWrap(filePath,group,imgtype)}, 10000);
        }
    }})    
}

function downloadZip(filePath, group, myDiv, myStage) {
    console.log("In DZ "+filePath+" "+group+" myDiv "+myDiv)
    var file = $.trim(filePath)
    group = $.trim(group)
    //alert("/hdf/createzip"+file+"?group="+group)
    if (imfirst) {
      var loaderText="<center><img src=blue-ajax-loader.gif width=35></center>"
      $(myDiv).html(loaderText)
    }
    imfirst = false;
    $.alsapi_ajax({type: "GET",
    url: "/hdf/createzip"+file+"?group="+group,
    success: function(res){
        console.log("DZ success "+res.status.state)
        //alert("DZ Success "+res)
        //alert("DZ Success "+res.status.state)
        if (res.status.state == 'Running'){
            var myText = "";
            $(myDiv).html(myText)
            var bar = document.getElementById("progressbar-"+myStage);
            $("#progresscon-"+myStage).show();
            bar.style.width = res.status.complete +"%";
            if (res.status.complete < 100) {
               setTimeout(function() { downloadZipWrap(file,group,myDiv,myStage)}, 2000);
            }
        } else if (res.status.state == 'Complete'){
            var myText = "Conversion Complete"
            downloadURL(alsapi_base_url+"/hdf/downloadstaged/"+res.location)
            $(myDiv).html(myText)
            try {
                $("#progresscon-"+myStage).hide();
            } catch (err) {
            }
        } else if (res.status.state == 'Staging') {
            var myText = "Staging File From Tape";
            $(myDiv).html(myText)
            var bar = document.getElementById("progressbar-"+myStage);
            $("#progresscon-"+myStage).show();
            bar.style.width = "100%";
            if (res.status.complete < 100) {
               setTimeout(function() { downloadZipWrap(file,group,myDiv,myStage)}, 10000);
            }           
        } else {
            //var myText = "Staging File From Tape";
            //$("#downloading").html(myText)
            if (res.status.complete < 100) {
               setTimeout(function() { downloadZipWrap(file,group,myDiv,myStage)}, 2000);
            }           
        }
    },error: function(res){
        //alert("DZ Error ")
        if (!imfirst) {
          setTimeout(function() { downloadZipWrap(filePath,group,myDiv,myStage)}, 10000);
        }
    }})
}


