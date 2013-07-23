/*
Started by Justin Patel
Lawrence Berkeley Laboratory
Molecular Foundry
06/2013 -> Present

Library for string-building via evaluation of symbolic document elements.
Essentially a text-replacement engine in an HTML context.
 */

TreeEval = {};

/*
 * Tree traversal.
 */
TreeEval.nodeValue = function(jq_elem, recursor_name) {
  // use base recursor if none specified
  if (typeof recursor_name === 'undefined') {
    recursor_name = 'base';
  }
  var recursor =  TreeEval.Recursors[recursor_name];

  // let node know it has been visited, so any callbacks can fire
  jq_elem.trigger('visit.TreeEval');

  // base cases
  var nodetype = recursor.nodetype(jq_elem);
  if (recursor.filter(jq_elem) === false) {
    return null;
  }
  if (recursor.isLeafNode(nodetype)) {
    return recursor.evaluateLeaf(jq_elem);
  }

  // recursion
  return recursor.evaluateInternal(jq_elem);
}



// Evaluation Contexts.
// Specify a context for recursion to happen.
// Determine what is being evaluated.
TreeEval.Contexts = {}

// HTML Context.
// Used for evaluating HTML elements.
TreeEval.Contexts['html'] = new Object();

TreeEval.Contexts['html']._meaningfulElems = 'input,select,div';

/*
 * Node progression.
 */
TreeEval.Contexts['html'].nextNode = function(jq_elem) {
  // for internal (non-leaf) nodes,
  // i.e. divs and selects (but not select multiples).
  var next_id = this.forward(jq_elem);
  // TODO: generalize this to skip all lists.
  if (jq_elem.prop('tagName').toLowerCase() != 'div') {
    if (next_id.length > 1) {
      next_id += '_';
    }
    next_id += jq_elem.val();
  }
  return $(next_id);
}

TreeEval.Contexts['html'].forward = function(jq_elem) {
  var next_id = '#' + jq_elem.prop('id');
  // alter forwarding as requested by elem
  for (var forwarder in this._Forwarders) {
    if (this._Forwarders.hasOwnProperty(forwarder)) {
      next_id = this._Forwarders[forwarder](next_id, jq_elem);
    }
  }
  return next_id;
}

// Node forwarders:
// IMPORTANT: the order of evaluation of these forwarders is intentional.
// These are not positioned arbitrarily.
TreeEval.Contexts['html']._Forwarders = {};
TreeEval.Contexts['html']._Forwarders['teForwardThrough'] = function (elem_id, jq_elem) {
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
TreeEval.Contexts['html']._Forwarders['teForwardWith'] = function (elem_id, jq_elem) {
  var forward_with = jq_elem.data('teForwardWith');
  if (forward_with) {
    // forward with an addition:
    // add a suffix to the id.
    elem_id += forward_with;
  }
  return elem_id;
}
TreeEval.Contexts['html']._Forwarders['teForwardTo'] = function (elem_id, jq_elem) {
  var forward_to = jq_elem.data('teForwardTo');
  if (forward_to) {
    // forward to an element:
    // replace the id with the specified destination.
    elem_id = '#' + forward_to;
  }
  return elem_id;
}


/*
 * List evaluation.
 */
TreeEval.Contexts['html'].childValues = function(jq_elem) {
  var childNodes = this.childNodes(jq_elem);

  // mutual recursion: call nodeValue() on each child to get all child parameters
  // essentially: values = map(this.nodeValue, childNodes)
  var length = childNodes.length;
  var values = new Array(length);
  for (var i = 0; i < length; i++) {
    values[i] = TreeEval.nodeValue($(childNodes[i]));
  }

  return values;
}
TreeEval.Contexts['html'].childNodes = function(jq_elem) {
  // list all the (meaningful) 1-deep child nodes of elem.
  // Do not filter them here;
  // they will be filtered upon evaluation, by the context.
  var children = jq_elem.children(this._meaningfulElems);
  return children;
}

TreeEval.Contexts['html']._getListType = function(jq_elem) {
  // return first matching list type
  var length = this._ListTypes.length;
  var listType = null;
  for (var i = 0; i < length; i++) {
    listType = this._ListTypes[i];
    if (jq_elem.data(listType)) {
      return listType;
    }
  }
  return null;
}
TreeEval.Contexts['html']._ListTypes = ['teListSlash',
                       'teListComma',
                       'teListConcat'];


/*
 * Node filters (true => pass)
 */
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



// Evaluation recursors.
// Determine the flow of recursion and how nodes are evaluated.
// Library can be extended by creating new recursors.
TreeEval.Recursors = {}

// Base recursor.
// Defines base functionality.
// Can be inherited in order to override certain aspects.
TreeEval.Recursors['base'] = new Object();

// Determines the type of a node,
// which will determine how it is evaluated.
TreeEval.Recursors['base'].nodetype = function(jq_elem) {
  // base nodetype on tag name
  var nodetype = jq_elem.prop('tagName').toLowerCase();

  // modify nodetype based on element attributes, if necessary
  if (this._NodetypeMods.hasOwnProperty(nodetype)) {
    nodetype = this._NodetypeMods[nodetype](jq_elem, nodetype);
  }

  // statically override, if one is present
  if (jq_elem.attr('data-te-nodetype')) {
    result = jq_elem.data('teNodetype');
  }
}
// attribute-based key modifiers for future lookup
// keys are based on tag, and modified by these based on attributes
TreeEval.Recursors['base']._NodetypeMods = {}
TreeEval.Recursors['base']._NodetypeMods['select'] = function(jq_elem, key) {
  if (jq_elem.prop('multiple')) {
    key += '_multiple';
  }
  return key;
}

// Determine if a node should be evaluated.
TreeEval.Recursors['base'].filter = function(jq_elem) {
  return this._passesContextFilters()
}
TreeEval.Recursors['base']._passesContextFilters = function(jq_elem) {
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

// Determine if a node is a leaf node or an internal node.
// Leaf nodes will be evaluated immediately (literally),
// whereas internal nodes will be evaluated recursively.
// Some nodes make sense only as one type.
TreeEval.Recursors['base'].isLeafNode = function(nodetype) {
  // default value for nodetype
  var result = this._LeafNodeDefaults[nodetype];
  
  // dynamically override, if applicable
  if (this._LeafNodeOverrides.hasOwnProperty(nodetype)) {
    result = this._LeafNodeOverrides[nodetype](jq_elem);
  }

  // statically override, if one is present
  if (jq_elem.attr('data-te-is-leaf')) {
    result = jq_elem.data('teIsLeaf');
  }

  return result;
}
// default values for keys
TreeEval.Context['base']._LeafNodeDefaults = {
  select: false, // requires dynamic override anyway. TODO: figure out what to do about that.
  select_multiple: true,
  input: true,
  div: false
}
// dynamic overrides
TreeEval.Context['base']._LeafNodeOverrides = {}
TreeEval.Context['base']._LeafNodeOverrides['select'] = function(jq_elem) {
  var next_node = TreeEval.nextNode(jq_elem);
  // whether or not a matching child node is found. If not, this is a leaf.
  return next_node.length === 0;
}

// Evaluate a node as a leaf node.
TreeEval.Recursors['base'].evaluateLeaf = function(jq_elem) {
  // TODO: perhaps pass this as a param to save second calculation?
  // perhaps add an optional parameter.
  var nodetype = this.nodetype(jq_elem);
  if (this._LeafEvaluators.hasOwnProperty(nodetype)) {
    return this._LeafEvaluators[nodetype](jq_elem);
  } else {
    return jq_elem.val();
  }
}
TreeEval.Recursors['base']._LeafEvaluators = {}
TreeEval.Recursors['base']._LeafEvaluators['select_multiple'] = function(jq_elem) {
  // TODO: why not just use sepAssemble()?
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

// Evaluate a node as an internal node.
TreeEval.Recursors['base'].evaluateInternal = function(jq_elem) {
  // TODO: perhaps pass this as a param to save second calculation?
  // perhaps add an optional parameter.
  var nodetype = this.nodetype(jq_elem);
  if (this._InternalEvaluators.hasOwnProperty(nodetype)) {
    return this._InternalEvaluators[nodetype](jq_elem);
  } else {
    var msg = "TreeEval: Error: don't know how to recursively evaluate nodetype: ";
    msg += nodetype;
    msg += '.';
    console.log(msg);
  }
}
TreeEval.Recursors['base']._InternalEvaluators = {}
TreeEval.Recursors['base']._InternalEvaluators['select'] = function(jq_elem) {
  var next_node = TreeEval.nextNode(jq_elem);
  return TreeEval.nodeValue(next_node);
}
TreeEval.Recursors['base']._InternalEvaluators['div'] = function(jq_elem) {
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
TreeEval.Recursors['base'].getAssembler = function(jq_elem) {
  var listType = TreeEval._getListType(jq_elem);
  if (listType !== null) {
      if (TreeEval._Assemblers.hasOwnProperty(listType)) {
        return TreeEval._Assemblers[listType];
      } else {
        var msg = "TreeEval: Error: don't know how to evaluate listType: ";
        msg += listType;
        msg += '.';
        console.log(msg);
      }
  } else {
    return null;
  }
}
// List assemblers:
TreeEval.Recursors['base']._Assemblers = {}
TreeEval.Recursors['base']._Assemblers['teListSlash'] = function(values) {
  return TreeEval._sepAssemble(values, '/');
}
TreeEval.Recursors['base']._Assemblers['teListComma'] = function(values) {
  return TreeEval._sepAssemble(values, ',');
}
TreeEval.Recursors['base']._Assemblers['teListConcat'] = function(values) {
  return TreeEval._sepAssemble(values, '');
}
TreeEval.Recursors['base']._sepAssemble = function(values, separator) {
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
