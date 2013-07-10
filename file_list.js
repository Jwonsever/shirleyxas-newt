var searchURL = ""
var number = 10
var numAdd = 50
var tooFew = false
var active_item = ""

function alsFileSearch(numBegin,numEnd) {
    checkALSAuth();
    if (numBegin == 0) {
      $("#file-list").html("")
    } else {
      $("#li-more").remove()
    }
    $("#files-loading").show()
    $.alsapi_ajax({
                url: "/hdf/search?search="+searchURL,
                success: function(data){
                    //alert("Success!")
                    var tmpText = ""
                    var ifile=0;
                    $.each(data, function(i,item){
                        //alert("Result! "+number+" "+item.parent+" "+item.name+" ")
                        var filePath = item.path
                        var displayName=getDisplayName(item.dataset,"/")
// JRD DISPLAY item.dataset here instead
                        if (item.name.indexOf(".h5") != -1 && ifile < numEnd && ifile >= numBegin && item.name.indexOf(".zip") == -1){
                                tmpText += "<li id='li-"+item.name+"/'><a href='javascript:expandSubgroups(\""+item.name+"\",\"/\",\""+item.parent+"/"+item.name+"\",0,"+number+",\"file\",\""+displayName+"\")'>"+displayName+"</a></li>"
                                //tmpText += "<li id='li-"+item.name+"/'><a href='javascript:expandSubgroups(\""+item.name+"\",\"/\",\""+item.parent+"/"+item.name+"\",0,"+number+",\"file\",\"Name\")'>"+displayName+"</a></li>"
                        } else if (ifile == numEnd) {
                            var nfilesNew = numEnd+numAdd;
                            tmpText += "<li id='li-more'><a href=\"javascript:alsFileSearch("+numEnd+","+nfilesNew+")\">More...</a></li>"
                        }
                        ifile++;
                    });
                    $("#files-loading").hide()
                    $("#file-list").append(tmpText)
                },
                error: function(res){
                    //alert(res.error)
                    console.log("CheckAuth error: " + res.error);
                    $("#files-loading").hide()
                    $("#file-list").html("Error")
                }
    });
}

function fileSearch(){
    //alert("In File Search");
    searchURL=$('#file-search-box').val();
    alsFileSearch(0,number);
}

function datasetCallback(file,group,phyloc,numBegin,numEnd,type){
    //alert("datasetCallback "+"/hdf"+phyloc+"?group="+group+" "+numBegin+" "+numEnd);
    itemClickedCallback(file,group,phyloc,type);
}

function expandSubgroups(file,group,phyloc,numBegin,numEnd,type,outDisplayName){

    //alert("expandSubgroups")
    //var outDisplayName = getDisplayName(file,group)
    $('li[id="'+active_item+'"]').removeClass("active")
    $('li[id="li-'+file+group+'"]').addClass("active")
    active_item="li-"+file+group

    if (numBegin == 0) {
      var tmpText = outDisplayName+"<br><center><img src=blue-ajax-loader.gif width=35></center>"
      $('li[id="li-'+file+group+'"]').html(tmpText)
    }

    $.alsapi_ajax({type: "GET",
        url: "/hdf"+phyloc+"?group="+group,
        success: function(res){
            //alert("success "+res.length);
            var tmpText="<a href='javascript:collapseSubgroups(\""+file+"\",\""+group+"\",\""+phyloc+"\",\""+type+"\",\""+outDisplayName+"\")'>"+outDisplayName+"</a>";
            var numChildren;
            var incomplete = false;

            if (res.length > 0) {
              tmpText += "<ul class=\"nav nav-list\">";
              $.each(res, function(i,item){
                  if ( i < numEnd) {
                    var myType = 'group'
                    if (res[i].type == 'dataset') myType = 'dataset'
                    if (res[i].parent == "/") {
                      var displayName = getDisplayName(file,res[i].parent+res[i].name)
                      tmpText += "<li id='li-"+file+res[i].parent+res[i].name+"'><a href='javascript:expandSubgroups(\""+file+"\",\""+res[i].parent+res[i].name+"\",\""+phyloc+"\",0,"+number+",\""+myType+"\",\""+displayName+"\")'>"+displayName+"</a></li>";
                    } else {
                      var displayName = getDisplayName(file,res[i].parent+"/"+res[i].name)
                      if (myType != 'dataset') {
                        tmpText += "<li id='li-"+file+res[i].parent+"/"+res[i].name+"'><a href='javascript:expandSubgroups(\""+file+"\",\""+res[i].parent+"/"+res[i].name+"\",\""+phyloc+"\",0,"+number+",\""+myType+"\",\""+displayName+"\")'>"+displayName+"</a></li>";
                        //tmpText += "<li id='li-"+file+res[i].parent+"/"+res[i].name+"'><a href='javascript:expandSubgroups(\""+file+"\",\""+res[i].parent+"/"+res[i].name+"\",\""+phyloc+"\",0,"+number+",\""+myType+"\",\"None\")'>"+displayName+"</a></li>";
                      } else {
                        tmpText += "<li id='li-"+file+res[i].parent+"/"+res[i].name+"'><a href='javascript:datasetCallback(\""+file+"\",\""+res[i].parent+"/"+res[i].name+"\",\""+phyloc+"\",0,"+number+",\""+myType+"\")'>"+displayName+"</a></li>";
                      }
                    }
                  } else if (i == numEnd) {
                    incomplete = true
                  }
                  numChildren = i
              })
              if (incomplete) {
                  var newnumber = 2 * numEnd
                  tmpText += "<li><a href='javascript:expandSubgroups(\""+file+"\",\""+group+"\",\""+phyloc+"\","+numEnd+","+newnumber+",\""+type+"\",\"More...\")'>More...</a>";
              }
              tmpText += "</ul>";

              $('li[id="li-'+file+group+'"]').html(tmpText)
            } else {
              tmpText = "<a href='javascript:expandSubgroups(\""+file+"\",\""+group+"\",\""+phyloc+"\","+0+","+number+",\""+type+"\",\""+outDisplayName+"\")'>"+outDisplayName+"</a>"
              $('li[id="li-'+file+group+'"]').html(tmpText)
            }

        }, error: function(res){
            alert(res.error)
            console.log("CheckAuth error: " + res.error);
        }
    });

    itemClickedCallback(file,group,phyloc,type);

}

function collapseSubgroups(file,group,phyloc,type,outDisplayName){
    //var outDisplayName = getDisplayName(file,group)
    var tmpText="<a href='javascript:expandSubgroups(\""+file+"\",\""+group+"\",\""+phyloc+"\",0,"+number+",\""+type+"\",\""+outDisplayName+"\")'>"+outDisplayName+"</a>";
    $('li[id="li-'+file+group+'"]').html(tmpText)

    //itemClickedCallback(file,group,phyloc,type);

}

function getDisplayName(file,group) {
  //console.log("display name "+group+"END")
  var outName;
  if (group == '/') {
    outName = file
  } else {
    outName = group.substring(group.lastIndexOf("/")+1,group.length)
  }

  outNameFull = outName;

  if (outName.length > 36) {
    //outName = outName.substring(0,10)+"...."+outName.substring(outName.length-9,outName.length)
    outName = outName.substring(0,32)+"<br>&nbsp;&nbsp;"+outName.substring(32,outName.length)
    //outName = "<span rel=\"tooltip\" title=\""+outNameFull+"\">"+outName.substring(0,10)+"...."+outName.substring(outName.length-9,outName.length)+"</span>"
    //outName = "<span rel=\"tooltip\" title=\""+outNameFull+"\">"+outName+"</span>"
  }

  return outName
}

