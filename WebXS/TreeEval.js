/*
Started by Justin Patel
Lawrence Berkeley Laboratory
Molecular Foundry
06/2013 -> Present

API/Library for performing contextual tree evaluation.

Implemented for string-building via evaluation of symbolic document elements.
Essentially a text-replacement engine in an HTML context.
 */

TreeEval = {};

/*
 * Node Evaluation.
 */
TreeEval.nodeValue = function(node, recursor, context) {
  // use base recursor if none specified
  if (typeof recursor === 'undefined') {
    recursor =  this.Recursors['base'];
  }
  // use HTML context if none specified
  if (typeof context === 'undefined') {
    context =  this.Contexts['html'];
  }

  // let node know it has been visited, so any callbacks can fire
  context.visit(node);

  // base cases
  if (recursor.filter(node, context) === false) {
    return null;
  }
  if (recursor.isLeafNode(node, context)) {
    return recursor.evaluateLeaf(node, context);
  }

  // recursion
  return recursor.evaluateInternal(node, context);
}



/*
 * Evaluation Contexts.
 * Specify a context for recursion to happen.
 * Determine what is being evaluated.
 */
TreeEval.Contexts = {}


/*
 * HTML Context.
 * Used for evaluating HTML elements.
 */
TreeEval.Contexts['html'] = new Object();

TreeEval.Contexts['html']._meaningfulElems = 'input,select,div';


/*
 * Node type.
 * Determines the type of a node,
 * which will determine how it is evaluated.
 */
TreeEval.Contexts['html'].nodetype = function(jq_elem) {
  // base nodetype on tag name
  var nodetype = jq_elem.prop('tagName').toLowerCase();

  // modify nodetype based on element attributes, if necessary
  if (this._NodetypeMods.hasOwnProperty(nodetype)) {
    nodetype = this._NodetypeMods[nodetype](jq_elem, nodetype);
  }

  // statically override, if one is present
  if (jq_elem.attr('data-te-nodetype')) {
    nodetype = jq_elem.data('teNodetype');
  }

  return nodetype;
}

// attribute-based nodetype modifiers.
// nodetypes are based on tag, and modified by these based on attributes
TreeEval.Contexts['html']._NodetypeMods = {}

TreeEval.Contexts['html']._NodetypeMods['select'] = function(jq_elem, nodetype) {
  if (jq_elem.prop('multiple')) {
    nodetype += '_multiple';
  }
  return nodetype;
}


/*
 * Node visiting.
 */
TreeEval.Contexts['html'].visit = function(jq_elem) {
  jq_elem.trigger('visit.TreeEval');
}


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
TreeEval.Contexts['html'].childValues = function(jq_elem, recursor) {
  var childNodes = this.childNodes(jq_elem);

  // mutual recursion: call nodeValue() on each child to get all child parameters
  // essentially: values = map(this.nodeValue, childNodes)
  var length = childNodes.length;
  var values = new Array(length);
  for (var i = 0; i < length; i++) {
    values[i] = TreeEval.nodeValue($(childNodes[i]), recursor, this);
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
 * Node filtering (true => pass)
 */
TreeEval.Contexts['html'].Filters = {};

TreeEval.Contexts['html'].Filters['notUnchecked'] = function(jq_elem) {
  if (jq_elem.prop('tagName').toLowerCase() === 'input' &&
      jq_elem.prop('type') === 'checkbox') {
    return jq_elem.prop('checked');
  } else {
    return true;
  }
}

TreeEval.Contexts['html'].Filters['notSkipped'] = function(jq_elem) {
  return !(jq_elem.data('teSkip'));
}



/*
 * Evaluation recursors.
 * Determine the flow of recursion and how nodes are evaluated.
 * Library can be extended by creating new recursors.
 */
TreeEval.Recursors = {}

/*
 * Base recursor.
 * Defines base functionality.
 * Can be inherited in order to override certain aspects.
 */
TreeEval.Recursors['base'] = new Object();

/*
 * Determine if a node should be evaluated.
 */
TreeEval.Recursors['base'].filter = function(jq_elem, context) {
  return this._passesContextFilters(jq_elem, context)
}

// whether it passes the filters defined by the context
TreeEval.Recursors['base']._passesContextFilters = function(jq_elem, context) {
  // iterate over all filters and apply each one
  var curr_filter = null;
  for (var filter_key in context.Filters) {
    if (context.Filters.hasOwnProperty(filter_key)) {
      curr_filter = context.Filters[filter_key];
      if (!(curr_filter(jq_elem))) {
        return false;
      }
    }
  }
  return true;
}


/*
 * Determine if a node is a leaf node or an internal node.
 * Leaf nodes will be evaluated immediately (literally),
 * whereas internal nodes will be evaluated recursively.
 * Some nodes make sense only as one type.
 */
TreeEval.Recursors['base'].isLeafNode = function(jq_elem, context) {
  // determine default value for nodetype
  var nodetype = context.nodetype(jq_elem);
  var result = this._LeafNodeDefaults[nodetype];
  
  // dynamically override, if applicable
  if (this._LeafNodeOverrides.hasOwnProperty(nodetype)) {
    result = this._LeafNodeOverrides[nodetype](jq_elem, context);
  }

  // TODO: move the html into the HTML Context.
  // Create a function that returns the override, or null if there is none.
  // statically override, if one is present
  if (jq_elem.attr('data-te-is-leaf')) {
    result = jq_elem.data('teIsLeaf');
  }

  return result;
}

// default values for keys
TreeEval.Recursors['base']._LeafNodeDefaults = {
  select: false, // requires dynamic override anyway. TODO: figure out what to do about that.
  select_multiple: true,
  input: true,
  div: false
}

// dynamic overrides
TreeEval.Recursors['base']._LeafNodeOverrides = {}

TreeEval.Recursors['base']._LeafNodeOverrides['select'] = function(jq_elem, context) {
  var next_node = context.nextNode(jq_elem);
  // whether or not a matching child node is found. If not, this is a leaf.
  return next_node.length === 0;
}


/*
 * Evaluate a node as a leaf node.
 */
TreeEval.Recursors['base'].evaluateLeaf = function(jq_elem, context) {
  var nodetype = context.nodetype(jq_elem);
  if (this._LeafEvaluators.hasOwnProperty(nodetype)) {
    return this._LeafEvaluators[nodetype](jq_elem, context);
  } else {
    // move node.val() into <Context>.valueHere(node), or something.
    return jq_elem.val();
  }
}

// leaf node evaluators
TreeEval.Recursors['base']._LeafEvaluators = {}

TreeEval.Recursors['base']._LeafEvaluators['select_multiple'] = function(jq_elem, context) {
  var this_recursor = TreeEval.Recursors['base'];
  var selected = jq_elem.val();
  return this_recursor._sepAssemble(selected, ',');
}


/*
 * Evaluate a node as an internal node.
 */
TreeEval.Recursors['base'].evaluateInternal = function(jq_elem, context) {
  var nodetype = context.nodetype(jq_elem);
  if (this._InternalEvaluators.hasOwnProperty(nodetype)) {
    return this._InternalEvaluators[nodetype](jq_elem, context);
  } else {
    var msg = "TreeEval: Error: don't know how to recursively evaluate nodetype: ";
    msg += nodetype;
    msg += '.';
    console.log(msg);
  }
}

// internal node evaluators
TreeEval.Recursors['base']._InternalEvaluators = {}

TreeEval.Recursors['base']._InternalEvaluators['select'] = function(jq_elem, context) {
  var this_recursor = TreeEval.Recursors['base'];
  var next_node = context.nextNode(jq_elem);
  return TreeEval.nodeValue(next_node, this_recursor, context);
}

TreeEval.Recursors['base']._InternalEvaluators['div'] = function(jq_elem, context) {
  var this_recursor = TreeEval.Recursors['base'];
  var assemble = this_recursor._getAssembler(jq_elem, context);
  if (assemble !== null) {
    var children = context.childValues(jq_elem, this_recursor);
    return assemble(children);
  } else {
    // Not a list. try to forward it.
    // NOTE: this can cause an infinite loop if document is improperly formatted.
    // TODO: check that it has forwarding info.
    // If not, throw an error to avoid infinite loop.
    var next_node = context.nextNode(jq_elem);
    return TreeEval.nodeValue(next_node, this_recursor, context);
  }
}


/*
 * list assembly
 */
TreeEval.Recursors['base']._getAssembler = function(jq_elem, context) {
  var listType = context._getListType(jq_elem);
  if (listType !== null) {
      if (this._Assemblers.hasOwnProperty(listType)) {
        return this._Assemblers[listType];
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

// list assemblers
TreeEval.Recursors['base']._Assemblers = {}

TreeEval.Recursors['base']._Assemblers['teListSlash'] = function(values) {
  return TreeEval.Recursors['base']._sepAssemble(values, '/');
}

TreeEval.Recursors['base']._Assemblers['teListComma'] = function(values) {
  return TreeEval.Recursors['base']._sepAssemble(values, ',');
}

TreeEval.Recursors['base']._Assemblers['teListConcat'] = function(values) {
  return TreeEval.Recursors['base']._sepAssemble(values, '');
}

TreeEval.Recursors['base']._sepAssemble = function(values, separator) {
  var ret = '';
  var length = values.length;
  var value = null;
  for (var i = 0; i < length; i++) {
    value = values[i];
    if (value === null) {
      continue;
    }
    if (ret !== '') {
      ret += separator;
    }
    ret += value;
  }
  return ret;
}
