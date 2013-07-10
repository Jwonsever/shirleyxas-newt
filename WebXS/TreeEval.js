/*
Started by Justin Patel
Lawrence Berkeley Laboratory
Molecular Foundry
06/2013 -> Present

Library for string-building via evaluation of symbolic document elements.
Essentially a text-replacement engine in an HTML context.
 */

/* 
 * Generic stuff.
 */
TreeEval = {};
TreeEval._meaningfulElems = 'input,select,div';

/*
 * Tree traversal.
 */
TreeEval.nodeValue = function(jq_elem) {
  var type = jq_elem.prop('tagName').toLowerCase();
  // base case
  if (TreeEval._isLeafNode(jq_elem)) {
    return TreeEval._leafNodeVal(jq_elem);
  }
  // recursion
  switch(type) {
    case 'div':
      return TreeEval._divValue(jq_elem);
    case 'select':
      return TreeEval._selectValue(jq_elem);
    default:
      alert("error: don't know how to recursively evaluate: " + type + '.');
      break;
  }
}
TreeEval._isLeafNode = function(jq_elem) {
  switch(jq_elem.prop('tagName').toLowerCase()) {
    case 'select':
      var next_node = TreeEval.nextNode(jq_elem);
      // whether or not a matching child node is found. If not, this is a leaf.
      return next_node.length === 0;
    case 'input':
      return true;
    case 'div':
      return false;
  }
}

TreeEval.nextNode = function(jq_elem) {
  // for internal (non-leaf) nodes,
  // i.e. divs and selects (but not select multiples).
  var next_id = TreeEval.forward(jq_elem);
  // TODO: generalize this to skip all lists.
  if (jq_elem.prop('tagName').toLowerCase() != 'div') {
    if (next_id.length > 1) {
      next_id += '_';
    }
    next_id += jq_elem.val();
  }
  return $(next_id);
}

/*
 * Non-recursive node evaluation.
 */
TreeEval._leafNodeVal = function(jq_elem) {
  // perhaps pass this as a param to save second calculation?
  var type = jq_elem.prop('tagName').toLowerCase();
  if (!(TreeEval.passesFilters(jq_elem))) {
    // filtered-out leaf nodes evaluate empty
    return '';
  } else if (type === 'select' &&
      jq_elem.prop('multiple')) {
    // if it's a select multiple, make it into a comma-separated list string
    return TreeEval._selectMultipleVal(jq_elem);
  } else {
    return jq_elem.val();
  }
}

TreeEval._selectMultipleVal = function(jq_elem) {
  // leaf node
  var result = '';
  var selected = jq_elem.val();
  var length = selected.length;
  for (var i = 0; i < length; i++) {
    if (i !== 0) {
      result += ',';
    }
    result += selected[i];
  }
  return result;
}

/*
 * Recursive node evaluation.
 */
TreeEval._selectValue = function(jq_elem) {
  // we already tested that this is not a leaf node.
  var next_node = TreeEval.nextNode(jq_elem);
  return TreeEval.nodeValue(next_node);
}

TreeEval._divValue = function(jq_elem) {
  var assemble = TreeEval.getAssembler(jq_elem);
  if (assemble) {
    var children = TreeEval.childValues(jq_elem);
    return assemble(children);
  } else {
    // Not a list. try to forward it.
    // NOTE: this can cause an infinite loop if document is improperly formatted.
    // TODO: check that it has forwarding info.
    // If not, throw an error to avoid infinite loop.
    var next_node = TreeEval.nextNode(jq_elem);
    return TreeEval.nodeValue(next_node);
  }
}
TreeEval.childValues = function(jq_elem) {
  var childNodes = TreeEval.childNodes(jq_elem);

  // mutual recursion: call nodeValue() on each child to get all child parameters
  // essentially: values = map(TreeEval.nodeValue, childNodes)
  var length = childNodes.length;
  var values = new Array(length);
  for (var i = 0; i < length; i++) {
    values[i] = TreeEval.nodeValue($(childNodes[i]));
  }

  return values;
}
TreeEval.childNodes = function(jq_elem) {
  // list all the (meaningful) 1-deep child nodes of elem.
  var children = jq_elem.children(TreeEval._meaningfulElems);
  return children.filter(function() {
      return TreeEval.passesFilters($(this));
      });
}

/*
 * Node forwarding
 */
TreeEval.forward = function(jq_elem) {
  var next_id = '#' + jq_elem.prop('id');
  // alter forwarding as requested by elem
  for (var forwarder in TreeEval._Forwarders) {
    if (TreeEval._Forwarders.hasOwnProperty(forwarder)) {
      next_id = TreeEval._Forwarders[forwarder](next_id, jq_elem);
    }
  }
  return next_id;
}

// Node forwarders:
// IMPORTANT: the order of evaluation of these forwarders is intentional.
// These are not positioned arbitrarily.
TreeEval._Forwarders = {};
TreeEval._Forwarders['teForwardThrough'] = function (elem_id, jq_elem) {
  if (jq_elem.data('teForwardThrough')) {
    // forward through this element:
    // drop the last part of the id
    var index = elem_id.lastIndexOf('_');
    if (index > 0) {
      elem_id = elem_id.substr(0, index);
    } else {
      elem_id = '#';
    }
  }
  return elem_id;
} 
TreeEval._Forwarders['teForwardWith'] = function (elem_id, jq_elem) {
  var forward_with = jq_elem.data('teForwardWith');
  if (forward_with) {
    // forward with an addition:
    // add a suffix to the id.
    elem_id += forward_with;
  }
  return elem_id;
}
TreeEval._Forwarders['teForwardTo'] = function (elem_id, jq_elem) {
  var forward_to = jq_elem.data('teForwardTo');
  if (forward_to) {
    // forward to an element:
    // replace the id with the specified destination.
    elem_id = '#' + forward_to;
  }
  return elem_id;
}

/*
 * Node filtering
 */
TreeEval.passesFilters = function(jq_elem) {
  // iterate over all filters and apply each one
  var curr_filter = null;
  for (var filter_key in TreeEval._Filters) {
    if (TreeEval._Filters.hasOwnProperty(filter_key)) {
      curr_filter = TreeEval._Filters[filter_key];
      if (!(curr_filter(jq_elem))) {
        return false;
      }
    }
  }
  return true;
}

// Node filterers (true => pass):
TreeEval._Filters = {};
TreeEval._Filters['notUnchecked'] = function(jq_elem) {
  if (jq_elem.prop('tagName').toLowerCase() === 'input' &&
      jq_elem.prop('type') === 'checkbox') {
    return jq_elem.prop('checked');
  } else {
    return true;
  }
}
TreeEval._Filters['notSkipped'] = function(jq_elem) {
  return !(jq_elem.data('teSkip'));
}

/*
 * List assembly
 */
TreeEval.getAssembler = function(jq_elem) {
  // iterate over type->assembler combinations
  for (var listType in TreeEval._Assemblers) {
    if (TreeEval._Assemblers.hasOwnProperty(listType)) {
      if (jq_elem.data(listType)) {
        return TreeEval._Assemblers[listType];
      }
    }
  }
  return null;
}

// List assemblers:
TreeEval._Assemblers = {}
TreeEval._Assemblers['teListSlash'] = function(values) {
  return TreeEval._sepAssemble(values, '/');
}
TreeEval._Assemblers['teListComma'] = function(values) {
  return TreeEval._sepAssemble(values, ',');
}
TreeEval._Assemblers['teListConcat'] = function(values) {
  return TreeEval._sepAssemble(values, '');
}
TreeEval._sepAssemble = function(values, separator) {
  var ret = '';
  var length = values.length;
  for (var i = 0; i < length; i++) {
    if (i != 0) {
      ret += separator;
    }
    ret += values[i];
  }
  return ret;
}
