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
 * Tree Evaluation.
 * Evaluates the tree rooted at the given node.
 */
TreeEval.treeValue = function(node, evaluator, context) {
  // use base evaluator if none specified
  if (typeof evaluator === 'undefined') {
    evaluator =  this.Evaluators['base'];
  }
  // use HTML context if none specified
  if (typeof context === 'undefined') {
    context =  this.Contexts['base'];
  }

  // let node know it has been visited, so any callbacks can fire
  context.visit(node);

  // base cases
  if (evaluator.filter(node, context) === false) {
    return null;
  }
  if (evaluator.isLeafNode(node, context)) {
    return evaluator.evaluateLeaf(node, context);
  }

  // recursion
  return evaluator.evaluateInternal(node, context);
}



/*
 * Evaluation Contexts.
 * Specify a context for recursion to happen.
 * Determine what is being evaluated.
 */
TreeEval.Contexts = {}


/*
 * Base Context.
 * Used for evaluating HTML elements.
 */
TreeEval.Contexts['base'] = new Object();

TreeEval.Contexts['base']._meaningfulElems = 'input,select,div';


/*
 * Node evaluation.
 * Evaluates a single node, not the whole tree rooted at it.
 */
TreeEval.Contexts['base'].nodeValue = function(jq_elem) {
  return jq_elem.val();
}


/*
 * Node type.
 * Determines the type of a node,
 * which will determine how it is evaluated.
 */
TreeEval.Contexts['base'].nodetype = function(jq_elem) {
  // statically override nodetype, if one is present
  if (jq_elem.attr('data-te-nodetype')) {
    return jq_elem.data('teNodetype');
  }

  // look up nodetype generator from tag name.
  var tagName = jq_elem.prop('tagName').toLowerCase();
  if (this._NodetypeGens.hasOwnProperty(tagName)) {
    return this._NodetypeGens[tagName](jq_elem);
  } else {
    var msg = "TreeEval: Error: don't know nodetype for element of type: ";
    msg += tagName;
    msg += '.';
    console.log(msg);
  }
}

// tag-based nodetype generators.
TreeEval.Contexts['base']._NodetypeGens = {}

TreeEval.Contexts['base']._NodetypeGens['input'] = function(jq_elem) {
  var this_context = TreeEval.Contexts['base'];
  var type = jq_elem.attr('type');
  // TODO: add handlers for other input types here as it becomes necessary.
  switch(type) {
    default:
      return 'literal';
  }
}

TreeEval.Contexts['base']._NodetypeGens['select'] = function(jq_elem) {
  if (jq_elem.prop('multiple')) {
    return 'literal_list';
  } else {
    return 'fork';
  }
}

TreeEval.Contexts['base']._NodetypeGens['div'] = function(jq_elem) {
  var this_context = TreeEval.Contexts['base'];
  var listType = this_context._getListType(jq_elem);
  if (listType === null) {
    return 'pointer';
  } else {
    return listType;
  }
}

// Return the list type of a node,
// or null if it is not a list.
// Meant for divs.
TreeEval.Contexts['base']._getListType = function(jq_elem) {
  // return first matching list type
  var length = this._ListTypes.length;
  for (var listType in this._ListTypes) {
    if (this._ListTypes.hasOwnProperty(listType)) {
      if (jq_elem.data(listType)) {
        return this._ListTypes[listType];
      }
    }
  }
  return null;
}

// expected list types properties, mapped to pretties names for them.
TreeEval.Contexts['base']._ListTypes = {
  teListSlash: 'list_slash',
  teListComma: 'list_comma',
  teListConcat: 'list_concat',
  teListQuery: 'list_query',
  teListEquals: 'list_equals'
}

// Return whether a nodetype represents a list.
TreeEval.Contexts['base']._isListtype = function(nodetype) {
  return nodetype.length > 5 &&
         nodetype.substring(0, 5) === 'list_';
}

// If node overrides default leaf status, return the specified value.
// If node does not override, return null.
TreeEval.Contexts['base'].isLeafNodeOverride = function(jq_elem) {
  if (jq_elem.attr('data-te-is-leaf')) {
    return jq_elem.data('teIsLeaf');
  } else {
    return null;
  }
}


/*
 * Node visiting.
 */
TreeEval.Contexts['base'].visit = function(jq_elem) {
  jq_elem.trigger('visit.TreeEval');
}


/*
 * Node progression.
 */
TreeEval.Contexts['base'].nextNode = function(jq_elem) {
  // for internal (non-leaf) nodes,
  // i.e. divs and selects (but not select multiples).
  var next_id = this.forward(jq_elem);
  // TODO: generalize this to skip all lists.
  if (jq_elem.prop('tagName').toLowerCase() != 'div') {
    if (next_id.length > 1) {
      next_id += '_';
    }
    next_id += this.nodeValue(jq_elem);
  }
  return $(next_id);
}

TreeEval.Contexts['base'].forward = function(jq_elem) {
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
TreeEval.Contexts['base']._Forwarders = {};

TreeEval.Contexts['base']._Forwarders['teForwardThrough'] = function (elem_id, jq_elem) {
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

TreeEval.Contexts['base']._Forwarders['teForwardWith'] = function (elem_id, jq_elem) {
  var forward_with = jq_elem.data('teForwardWith');
  if (forward_with) {
    // forward with an addition:
    // add a suffix to the id.
    elem_id += forward_with;
  }
  return elem_id;
}

TreeEval.Contexts['base']._Forwarders['teForwardTo'] = function (elem_id, jq_elem) {
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
TreeEval.Contexts['base'].childNodes = function(jq_elem) {
  // list all the (meaningful) 1-deep child nodes of elem.
  // Do not filter them here;
  // they will be filtered upon evaluation, by the context.
  // TODO: change that. Might have to add logic to leaf
  // node detection to account for this.
  var jq_children = jq_elem.children(this._meaningfulElems);

  // turn one jquery object of multiple elements into
  // an array of multiple jquery objects, each with one element.
  // This way they can be indexed by evaluators and evaluated
  // without having to do context-specific reboxing.
  var num_children = jq_children.length;
  var children = new Array(num_children);
  for (var i = 0; i < num_children; i++) {
    children[i] = $(jq_children[i]);
  }

  return children;
}


/*
 * Node filtering (true => pass)
 */
TreeEval.Contexts['base'].Filters = {};

TreeEval.Contexts['base'].Filters['notSkipped'] = function(jq_elem) {
  return !(jq_elem.data('teSkip'));
}



/*
 * Evaluators.
 * Determine the flow of recursion and how nodes are evaluated.
 * Library can be extended by creating new evaluators.
 */
TreeEval.Evaluators = {}

/*
 * Base evaluator.
 * Defines base functionality.
 * Can be inherited in order to override certain aspects.
 */
TreeEval.Evaluators['base'] = new Object();

/*
 * Determine if a node should be evaluated.
 */
TreeEval.Evaluators['base'].filter = function(node, context) {
  // TODO: refactor this. Shouldn't be able to access nodes that the
  // context has filtered out. Make childNodes filter based on context's filters,
  // so that evaluator can never even see what the context thinks is definitely
  // not to be evaluated.
  return this._passesContextFilters(node, context)
}

// whether it passes the filters defined by the context
TreeEval.Evaluators['base']._passesContextFilters = function(node, context) {
  // iterate over all filters and apply each one
  var curr_filter = null;
  for (var filter_key in context.Filters) {
    if (context.Filters.hasOwnProperty(filter_key)) {
      curr_filter = context.Filters[filter_key];
      if (!(curr_filter(node))) {
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
TreeEval.Evaluators['base'].isLeafNode = function(node, context) {
  // determine default value for nodetype
  var nodetype = context.nodetype(node);
  var result = this._LeafNodeDefaults[nodetype];
  
  // dynamically override, if applicable
  if (this._LeafNodeOverrides.hasOwnProperty(nodetype)) {
    result = this._LeafNodeOverrides[nodetype](node, context);
  }

  // statically override, if one is present
  var override = context.isLeafNodeOverride(node);
  if (override !== null) {
    result = override;
  }

  return result;
}

// default values for keys
TreeEval.Evaluators['base']._LeafNodeDefaults = {
  literal: true,
  literal_list: true,
  fork: false, // requires dynamic override anyway. TODO: figure out what to do about that.
  pointer: false,
  list_slash: false, // TODO: generalize this. will be a lot of repeating with more list types.
  list_comma: false,
  list_concat: false,
  list_query: false,
  list_equals: false
}

// dynamic (runtime) overrides
TreeEval.Evaluators['base']._LeafNodeOverrides = {}

TreeEval.Evaluators['base']._LeafNodeOverrides['fork'] = function(node, context) {
  var next_node = context.nextNode(node);
  // whether or not a matching child node is found. If not, this is a leaf.
  return next_node.length === 0;
}


/*
 * Evaluate a node as a leaf node.
 */
TreeEval.Evaluators['base'].evaluateLeaf = function(node, context) {
  var nodetype = context.nodetype(node);
  if (this._LeafEvaluators.hasOwnProperty(nodetype)) {
    return this._LeafEvaluators[nodetype](node, context);
  } else {
    // by default, treat it as a literal
    return this._LeafEvaluators['literal'](node, context);
  }
}

// leaf node evaluators
TreeEval.Evaluators['base']._LeafEvaluators = {}

TreeEval.Evaluators['base']._LeafEvaluators['literal'] = function(node, context) {
  return context.nodeValue(node);
}

TreeEval.Evaluators['base']._LeafEvaluators['literal_list'] = function(node, context) {
  var this_evaluator = TreeEval.Evaluators['base'];
  var selected = context.nodeValue(node);
  return this_evaluator._sepAssemble(selected, ',');
}


/*
 * Evaluate a node as an internal node.
 */
TreeEval.Evaluators['base'].evaluateInternal = function(node, context) {
  var nodetype = context.nodetype(node);
  if (context._isListtype(nodetype)) {
    return this._evaluateList(node, context);
  } else if (this._InternalEvaluators.hasOwnProperty(nodetype)) {
    return this._InternalEvaluators[nodetype](node, context);
  } else {
    var msg = "TreeEval: Error: don't know how to recursively evaluate nodetype: ";
    msg += nodetype;
    msg += '.';
    console.log(msg);
  }
}

// internal node evaluators
TreeEval.Evaluators['base']._InternalEvaluators = {}

TreeEval.Evaluators['base']._InternalEvaluators['fork'] = function(node, context) {
  var this_evaluator = TreeEval.Evaluators['base'];
  var next_node = context.nextNode(node);
  return this_evaluator._treeValue(next_node, context);
}

TreeEval.Evaluators['base']._InternalEvaluators['pointer'] = function(node, context) {
  var this_evaluator = TreeEval.Evaluators['base'];
  // NOTE: this can probably cause an infinite loop if document is improperly formatted.
  // TODO: check that it has forwarding info.
  // If not, throw an error to avoid infinite loop.
  var next_node = context.nextNode(node);
  return this_evaluator._treeValue(next_node, context);
}

TreeEval.Evaluators['base']._evaluateList = function(node, context) {
  var this_evaluator = TreeEval.Evaluators['base'];
  var listType = context.nodetype(node);

  var assemble = this_evaluator._getAssembler(listType);

  var children = context.childNodes(node);
  var num_children = children.length;
  var childValues = new Array(num_children);
  for (var i = 0; i < num_children; i++) {
    childValues[i] = this_evaluator._treeValue(children[i], context);
  }

  return assemble(childValues);
}

// wrapper for tree evaluation that chooses an evaluator based on the nodetype
// of the node to be evaluated.
TreeEval.Evaluators['base']._treeValue = function(node, context) {
  var evaluator = null;
  var nodetype = context.nodetype(node);
  if (this._EvaluatorPrefs.hasOwnProperty(nodetype)) {
    evaluator = this._evaluatorPrefs[nodetype];
  } else {
    evaluator = this;
  }

  return TreeEval.treeValue(node, evaluator, context);
}

// preferences for which evaluator to use for a given nodetype.
TreeEval.Evaluators['base']._EvaluatorPrefs = {
}


/*
 * list assembly
 */
TreeEval.Evaluators['base']._getAssembler = function(listType) {
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
TreeEval.Evaluators['base']._Assemblers = {}

TreeEval.Evaluators['base']._Assemblers['list_slash'] = function(values) {
  var this_evaluator = TreeEval.Evaluators['base'];
  return this_evaluator._sepAssemble(values, '/');
}

TreeEval.Evaluators['base']._Assemblers['list_comma'] = function(values) {
  var this_evaluator = TreeEval.Evaluators['base'];
  return this_evaluator._sepAssemble(values, ',');
}

TreeEval.Evaluators['base']._Assemblers['list_concat'] = function(values) {
  var this_evaluator = TreeEval.Evaluators['base'];
  return this_evaluator._sepAssemble(values, '');
}
TreeEval.Evaluators['base']._Assemblers['list_query'] = function(values) {
  var this_evaluator = TreeEval.Evaluators['base'];
  var ret = '?';
  ret +=  this_evaluator._sepAssemble(values, '&');
  return ret;
}
TreeEval.Evaluators['base']._Assemblers['list_equals'] = function(values) {
  var this_evaluator = TreeEval.Evaluators['base'];
  return this_evaluator._sepAssemble(values, '=');
}

TreeEval.Evaluators['base']._sepAssemble = function(values, separator) {
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
