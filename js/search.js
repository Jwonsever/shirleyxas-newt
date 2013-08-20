//JS Functions related to searching and scraping databases.

// query the selected REST API.
var lastSearchResult = '';
function restQuery() {
  var dest = $('#searchResults');

  dest.html('working...');
  var root = $('#searchLocationChosen');
  var url = TreeEval.treeValue(root);

  var end = url.substring(url.lastIndexOf('/') + 1,
                          url.length);

  // only send SDFs to JSmol.
  var result = '';
  switch(end) {
    case 'SDF':
	url += "?record_type=3d"; // Grab 3d 
      $.get(url, function(data, status) {
        lastSearchResult = data;
        result = '<p id="searchResult" ';
        result += ' style="display:none">';
        result += 'The structure is now available for display.</p>';
        dest.html(result);
        $('#searchResult').fadeIn();
      }).fail(function(xhr, textStatus, errorThrown) {
        result = 'An error occurred with the request:<br><pre>';
        result += errorThrown;
        result += '<br><br>';
        result += xhr.responseText;
        result += '</pre>';
        dest.html(result);
      });
      break;
    default:
      result = '<a id="searchResult" ';
      result += 'style="display:none" ';
      result += 'target="_blank" href="';
      result += url;
      if (end == 'PNG') {
        result += '">View Image</a><br>';
      } else {
        result += '">Open Result</a><br>';
      }
      dest.html(result);
      $('#searchResult').fadeIn();
      break;
  }
}

// display the result of the last search result in JSmol.
function displaySearchResult(applet) {
    var script = "zap;set echo top left;font echo 16;echo \"Loading and Optimizing\";";
    Jmol.script(applet, script);
    script = "try{set useMinimizationThread true;load INLINE '" + lastSearchResult + "'; " //Fom before 3d records: minimize STEPS 300 addHydrogens;"
	+ "}catch(e){;}";
    Jmol.script(applet, script);
    Jmol.script(applet, "javascript DSRcallback()");
}

// organizes scraping operations
Scrape = {}
Scrape.scraperPath = 'scrape/scrape.cgi';
Scrape.getInputs = function(form) {
  var selector = '#' + form.id + ' table input';
  return $(selector);
}
Scrape.validateInputs = function(form, db_name) {
  var inputs = Scrape.getInputs(form);

  var valid = true;
  var message = '';

  // must provide at least one search term
  var termGiven = false;
  for (var i = 0; i < inputs.length; i++) {
    if (inputs[i].value.length > 0) {
      termGiven = true;
    }
  }
  if (!termGiven) {
    message = 'Please enter at least one search term.';
    valid = false;
  }

  if (valid) {
    Scrape.scrape(form, db_name);
  } else {
    alert(message);
  }
}
Scrape.scrape = function(form, db_name) {
  var url = Scrape.scraperPath + '?';
  url += 'db=' + db_name;
  
  var inputs = Scrape.getInputs(form);
  var input = null;
  for (var i = 0; i < inputs.length; i++) {
    input = inputs[i];
    if (input.value.length > 0) {
      url += '&';
      url += input.name + '=' + input.value;
    }
  }

  $.get(url,
        Scrape.handleResult);
  $('#searchResults').html('working... (could take up to 30 seconds)');
}
Scrape.handleResult = function(data, status) {
  $('#searchResults').html('<pre>' + data + '</pre>');
}


// Search area switching functions.
function mutexShow(shownId, mutexClass) {
    // if nothing to show, no errors will be thrown. Thanks jquery!
    $('.' + mutexClass).hide();
    $('#' + shownId).show();
}

function switchToSearchLocation(loc) {
    mutexShow(loc, 'searchTermSet');
}

// organizes PubChem switching
// TODO: make this less hideous
PubChem = {
    Compound: {},
    Substance: {},
    Assay: {},
}
PubChem.chooseDomain = function(dom) {
  dom = 'PubChem_' + dom; 
  mutexShow(dom, 'PubChem_domain');
}
PubChem._chooseNamespace = function(namespace, domain) {
  // some namespaces don't have anything to show. This is okay.
  namespace = 'PubChem_' + domain + '_' + namespace;
  mutexShow(namespace, 'PubChem_' + domain + '_namespace');
}
PubChem._chooseOperation = function(op, domain) {
  op = 'PubChem_' + domain + '_op_' + op;
  mutexShow(op, 'PubChem_' + domain + '_op');
}
PubChem.Compound.chooseNamespace = function(namespace) {
  PubChem._chooseNamespace(namespace, 'compound');
}
PubChem.Compound.chooseOperation = function(op) {
  PubChem._chooseOperation(op, 'compound');
}
PubChem.Substance.chooseNamespace = function(namespace) {
  PubChem._chooseNamespace(namespace, 'substance');
}
PubChem.Substance.chooseOperation = function(op) {
  PubChem._chooseOperation(op, 'substance');
}
PubChem.Assay.chooseNamespace = function(namespace) {
  PubChem._chooseNamespace(namespace, 'assay');
}
PubChem.Assay.chooseOperation = function(op) {
  PubChem._chooseOperation(op, 'assay');
}
