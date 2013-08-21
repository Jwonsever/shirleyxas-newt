// BH  3/25/2013 11:10:21 AM -- jQuery.iecors.js moved to JSmolCore.js
// BH 11/28/2012 7:23:24 PM fixed binary ArrayBuffer loading for Mac/Safari
// BH 11/3/2012 11:33:54 PM added "binary" dataType for PNG, PNGJ, ZIP, GZIP files. Preliminary only
// BH 11/9/2012 5:59:21 AM includes jQuery.iecors.js
/*!
 * jQuery JavaScript Library v1.7.2
 * http://jquery.com/
 *
 * Copyright 2011, John Resig
 * Dual licensed under the MIT or GPL Version 2 licenses.
 * http://jquery.org/license
 *
 * Includes Sizzle.js
 * http://sizzlejs.com/
 * Copyright 2011, The Dojo Foundation
 * Released under the MIT, BSD, and GPL Licenses.
 *
 * Date: Wed Mar 21 12:46:34 2012 -0700
 */
(function( window, undefined ) {

// Use the correct document accordingly with window argument (sandbox)
var document = window.document,
	navigator = window.navigator,
	location = window.location;
var jQuery = (function() {

// Define a local copy of jQuery
var jQuery = function( selector, context ) {
		// The jQuery object is actually just the init constructor 'enhanced'
		return new jQuery.fn.init( selector, context, rootjQuery );
	},

	// Map over jQuery in case of overwrite
	_jQuery = window.jQuery,

	// Map over the $ in case of overwrite
	_$ = window.$,

	// A central reference to the root jQuery(document)
	rootjQuery,

	// A simple way to check for HTML strings or ID strings
	// Prioritize #id over <tag> to avoid XSS via location.hash (#9521)
	quickExpr = /^(?:[^#<]*(<[\w\W]+>)[^>]*$|#([\w\-]*)$)/,

	// Check if a string has a non-whitespace character in it
	rnotwhite = /\S/,

	// Used for trimming whitespace
	trimLeft = /^\s+/,
	trimRight = /\s+$/,

	// Match a standalone tag
	rsingleTag = /^<(\w+)\s*\/?>(?:<\/\1>)?$/,

	// JSON RegExp
	rvalidchars = /^[\],:{}\s]*$/,
	rvalidescape = /\\(?:["\\\/bfnrt]|u[0-9a-fA-F]{4})/g,
	rvalidtokens = /"[^"\\\n\r]*"|true|false|null|-?\d+(?:\.\d*)?(?:[eE][+\-]?\d+)?/g,
	rvalidbraces = /(?:^|:|,)(?:\s*\[)+/g,

	// Useragent RegExp
	rwebkit = /(webkit)[ \/]([\w.]+)/,
	ropera = /(opera)(?:.*version)?[ \/]([\w.]+)/,
	rmsie = /(msie) ([\w.]+)/,
	rmozilla = /(mozilla)(?:.*? rv:([\w.]+))?/,

	// Matches dashed string for camelizing
	rdashAlpha = /-([a-z]|[0-9])/ig,
	rmsPrefix = /^-ms-/,

	// Used by jQuery.camelCase as callback to replace()
	fcamelCase = function( all, letter ) {
		return ( letter + "" ).toUpperCase();
	},

	// Keep a UserAgent string for use with jQuery.browser
	userAgent = navigator.userAgent,

	// For matching the engine and version of the browser
	browserMatch,

	// The deferred used on DOM ready
	readyList,

	// The ready event handler
	DOMContentLoaded,

	// Save a reference to some core methods
	toString = Object.prototype.toString,
	hasOwn = Object.prototype.hasOwnProperty,
	push = Array.prototype.push,
	slice = Array.prototype.slice,
	trim = String.prototype.trim,
	indexOf = Array.prototype.indexOf,

	// [[Class]] -> type pairs
	class2type = {};

jQuery.fn = jQuery.prototype = {
	constructor: jQuery,
	init: function( selector, context, rootjQuery ) {
		var match, elem, ret, doc;

		// Handle $(""), $(null), or $(undefined)
		if ( !selector ) {
			return this;
		}

		// Handle $(DOMElement)
		if ( selector.nodeType ) {
			this.context = this[0] = selector;
			this.length = 1;
			return this;
		}

		// The body element only exists once, optimize finding it
		if ( selector === "body" && !context && document.body ) {
			this.context = document;
			this[0] = document.body;
			this.selector = selector;
			this.length = 1;
			return this;
		}

		// Handle HTML strings
		if ( typeof selector === "string" ) {
			// Are we dealing with HTML string or an ID?
			if ( selector.charAt(0) === "<" && selector.charAt( selector.length - 1 ) === ">" && selector.length >= 3 ) {
				// Assume that strings that start and end with <> are HTML and skip the regex check
				match = [ null, selector, null ];

			} else {
				match = quickExpr.exec( selector );
			}

			// Verify a match, and that no context was specified for #id
			if ( match && (match[1] || !context) ) {

				// HANDLE: $(html) -> $(array)
				if ( match[1] ) {
					context = context instanceof jQuery ? context[0] : context;
					doc = ( context ? context.ownerDocument || context : document );

					// If a single string is passed in and it's a single tag
					// just do a createElement and skip the rest
					ret = rsingleTag.exec( selector );

					if ( ret ) {
						if ( jQuery.isPlainObject( context ) ) {
							selector = [ document.createElement( ret[1] ) ];
							jQuery.fn.attr.call( selector, context, true );

						} else {
							selector = [ doc.createElement( ret[1] ) ];
						}

					} else {
						ret = jQuery.buildFragment( [ match[1] ], [ doc ] );
						selector = ( ret.cacheable ? jQuery.clone(ret.fragment) : ret.fragment ).childNodes;
					}

					return jQuery.merge( this, selector );

				// HANDLE: $("#id")
				} else {
					elem = document.getElementById( match[2] );

					// Check parentNode to catch when Blackberry 4.6 returns
					// nodes that are no longer in the document #6963
					if ( elem && elem.parentNode ) {
						// Handle the case where IE and Opera return items
						// by name instead of ID
						if ( elem.id !== match[2] ) {
							return rootjQuery.find( selector );
						}

						// Otherwise, we inject the element directly into the jQuery object
						this.length = 1;
						this[0] = elem;
					}

					this.context = document;
					this.selector = selector;
					return this;
				}

			// HANDLE: $(expr, $(...))
			} else if ( !context || context.jquery ) {
				return ( context || rootjQuery ).find( selector );

			// HANDLE: $(expr, context)
			// (which is just equivalent to: $(context).find(expr)
			} else {
				return this.constructor( context ).find( selector );
			}

		// HANDLE: $(function)
		// Shortcut for document ready
		} else if ( jQuery.isFunction( selector ) ) {
			return rootjQuery.ready( selector );
		}

		if ( selector.selector !== undefined ) {
			this.selector = selector.selector;
			this.context = selector.context;
		}

		return jQuery.makeArray( selector, this );
	},

	// Start with an empty selector
	selector: "",

	// The current version of jQuery being used
	jquery: "1.7.2",

	// The default length of a jQuery object is 0
	length: 0,

	// The number of elements contained in the matched element set
	size: function() {
		return this.length;
	},

	toArray: function() {
		return slice.call( this, 0 );
	},

	// Get the Nth element in the matched element set OR
	// Get the whole matched element set as a clean array
	get: function( num ) {
		return num == null ?

			// Return a 'clean' array
			this.toArray() :

			// Return just the object
			( num < 0 ? this[ this.length + num ] : this[ num ] );
	},

	// Take an array of elements and push it onto the stack
	// (returning the new matched element set)
	pushStack: function( elems, name, selector ) {
		// Build a new jQuery matched element set
		var ret = this.constructor();

		if ( jQuery.isArray( elems ) ) {
			push.apply( ret, elems );

		} else {
			jQuery.merge( ret, elems );
		}

		// Add the old object onto the stack (as a reference)
		ret.prevObject = this;

		ret.context = this.context;

		if ( name === "find" ) {
			ret.selector = this.selector + ( this.selector ? " " : "" ) + selector;
		} else if ( name ) {
			ret.selector = this.selector + "." + name + "(" + selector + ")";
		}

		// Return the newly-formed element set
		return ret;
	},

	// Execute a callback for every element in the matched set.
	// (You can seed the arguments with an array of args, but this is
	// only used internally.)
	each: function( callback, args ) {
		return jQuery.each( this, callback, args );
	},

	ready: function( fn ) {
		// Attach the listeners
		jQuery.bindReady();

		// Add the callback
		readyList.add( fn );

		return this;
	},

	eq: function( i ) {
		i = +i;
		return i === -1 ?
			this.slice( i ) :
			this.slice( i, i + 1 );
	},

	first: function() {
		return this.eq( 0 );
	},

	last: function() {
		return this.eq( -1 );
	},

	slice: function() {
		return this.pushStack( slice.apply( this, arguments ),
			"slice", slice.call(arguments).join(",") );
	},

	map: function( callback ) {
		return this.pushStack( jQuery.map(this, function( elem, i ) {
			return callback.call( elem, i, elem );
		}));
	},

	end: function() {
		return this.prevObject || this.constructor(null);
	},

	// For internal use only.
	// Behaves like an Array's method, not like a jQuery method.
	push: push,
	sort: [].sort,
	splice: [].splice
};

// Give the init function the jQuery prototype for later instantiation
jQuery.fn.init.prototype = jQuery.fn;

jQuery.extend = jQuery.fn.extend = function() {
	var options, name, src, copy, copyIsArray, clone,
		target = arguments[0] || {},
		i = 1,
		length = arguments.length,
		deep = false;

	// Handle a deep copy situation
	if ( typeof target === "boolean" ) {
		deep = target;
		target = arguments[1] || {};
		// skip the boolean and the target
		i = 2;
	}

	// Handle case when target is a string or something (possible in deep copy)
	if ( typeof target !== "object" && !jQuery.isFunction(target) ) {
		target = {};
	}

	// extend jQuery itself if only one argument is passed
	if ( length === i ) {
		target = this;
		--i;
	}

	for ( ; i < length; i++ ) {
		// Only deal with non-null/undefined values
		if ( (options = arguments[ i ]) != null ) {
			// Extend the base object
			for ( name in options ) {
				src = target[ name ];
				copy = options[ name ];

				// Prevent never-ending loop
				if ( target === copy ) {
					continue;
				}

				// Recurse if we're merging plain objects or arrays
				if ( deep && copy && ( jQuery.isPlainObject(copy) || (copyIsArray = jQuery.isArray(copy)) ) ) {
					if ( copyIsArray ) {
						copyIsArray = false;
						clone = src && jQuery.isArray(src) ? src : [];

					} else {
						clone = src && jQuery.isPlainObject(src) ? src : {};
					}

					// Never move original objects, clone them
					target[ name ] = jQuery.extend( deep, clone, copy );

				// Don't bring in undefined values
				} else if ( copy !== undefined ) {
					target[ name ] = copy;
				}
			}
		}
	}

	// Return the modified object
	return target;
};

jQuery.extend({
	noConflict: function( deep ) {
		if ( window.$ === jQuery ) {
			window.$ = _$;
		}

		if ( deep && window.jQuery === jQuery ) {
			window.jQuery = _jQuery;
		}

		return jQuery;
	},

	// Is the DOM ready to be used? Set to true once it occurs.
	isReady: false,

	// A counter to track how many items to wait for before
	// the ready event fires. See #6781
	readyWait: 1,

	// Hold (or release) the ready event
	holdReady: function( hold ) {
		if ( hold ) {
			jQuery.readyWait++;
		} else {
			jQuery.ready( true );
		}
	},

	// Handle when the DOM is ready
	ready: function( wait ) {
		// Either a released hold or an DOMready/load event and not yet ready
		if ( (wait === true && !--jQuery.readyWait) || (wait !== true && !jQuery.isReady) ) {
			// Make sure body exists, at least, in case IE gets a little overzealous (ticket #5443).
			if ( !document.body ) {
				return setTimeout( jQuery.ready, 1 );
			}

			// Remember that the DOM is ready
			jQuery.isReady = true;

			// If a normal DOM Ready event fired, decrement, and wait if need be
			if ( wait !== true && --jQuery.readyWait > 0 ) {
				return;
			}

			// If there are functions bound, to execute
			readyList.fireWith( document, [ jQuery ] );

			// Trigger any bound ready events
			if ( jQuery.fn.trigger ) {
				jQuery( document ).trigger( "ready" ).off( "ready" );
			}
		}
	},

	bindReady: function() {
		if ( readyList ) {
			return;
		}

		readyList = jQuery.Callbacks( "once memory" );

		// Catch cases where $(document).ready() is called after the
		// browser event has already occurred.
		if ( document.readyState === "complete" ) {
			// Handle it asynchronously to allow scripts the opportunity to delay ready
			return setTimeout( jQuery.ready, 1 );
		}

		// Mozilla, Opera and webkit nightlies currently support this event
		if ( document.addEventListener ) {
			// Use the handy event callback
			document.addEventListener( "DOMContentLoaded", DOMContentLoaded, false );

			// A fallback to window.onload, that will always work
			window.addEventListener( "load", jQuery.ready, false );

		// If IE event model is used
		} else if ( document.attachEvent ) {
			// ensure firing before onload,
			// maybe late but safe also for iframes
			document.attachEvent( "onreadystatechange", DOMContentLoaded );

			// A fallback to window.onload, that will always work
			window.attachEvent( "onload", jQuery.ready );

			// If IE and not a frame
			// continually check to see if the document is ready
			var toplevel = false;

			try {
				toplevel = window.frameElement == null;
			} catch(e) {}

			if ( document.documentElement.doScroll && toplevel ) {
				doScrollCheck();
			}
		}
	},

	// See test/unit/core.js for details concerning isFunction.
	// Since version 1.3, DOM methods and functions like alert
	// aren't supported. They return false on IE (#2968).
	isFunction: function( obj ) {
		return jQuery.type(obj) === "function";
	},

	isArray: Array.isArray || function( obj ) {
		return jQuery.type(obj) === "array";
	},

	isWindow: function( obj ) {
		return obj != null && obj == obj.window;
	},

	isNumeric: function( obj ) {
		return !isNaN( parseFloat(obj) ) && isFinite( obj );
	},

	type: function( obj ) {
		return obj == null ?
			String( obj ) :
			class2type[ toString.call(obj) ] || "object";
	},

	isPlainObject: function( obj ) {
		// Must be an Object.
		// Because of IE, we also have to check the presence of the constructor property.
		// Make sure that DOM nodes and window objects don't pass through, as well
		if ( !obj || jQuery.type(obj) !== "object" || obj.nodeType || jQuery.isWindow( obj ) ) {
			return false;
		}

		try {
			// Not own constructor property must be Object
			if ( obj.constructor &&
				!hasOwn.call(obj, "constructor") &&
				!hasOwn.call(obj.constructor.prototype, "isPrototypeOf") ) {
				return false;
			}
		} catch ( e ) {
			// IE8,9 Will throw exceptions on certain host objects #9897
			return false;
		}

		// Own properties are enumerated firstly, so to speed up,
		// if last one is own, then all properties are own.

		var key;
		for ( key in obj ) {}

		return key === undefined || hasOwn.call( obj, key );
	},

	isEmptyObject: function( obj ) {
		for ( var name in obj ) {
			return false;
		}
		return true;
	},

	error: function( msg ) {
		throw new Error( msg );
	},

	parseJSON: function( data ) {
		if ( typeof data !== "string" || !data ) {
			return null;
		}

		// Make sure leading/trailing whitespace is removed (IE can't handle it)
		data = jQuery.trim( data );

		// Attempt to parse using the native JSON parser first
		if ( window.JSON && window.JSON.parse ) {
			return window.JSON.parse( data );
		}

		// Make sure the incoming data is actual JSON
		// Logic borrowed from http://json.org/json2.js
		if ( rvalidchars.test( data.replace( rvalidescape, "@" )
			.replace( rvalidtokens, "]" )
			.replace( rvalidbraces, "")) ) {

			return ( new Function( "return " + data ) )();

		}
		jQuery.error( "Invalid JSON: " + data );
	},

	// Cross-browser xml parsing
	parseXML: function( data ) {
		if ( typeof data !== "string" || !data ) {
			return null;
		}
		var xml, tmp;
		try {
			if ( window.DOMParser ) { // Standard
				tmp = new DOMParser();
				xml = tmp.parseFromString( data , "text/xml" );
			} else { // IE
				xml = new ActiveXObject( "Microsoft.XMLDOM" );
				xml.async = "false";
				xml.loadXML( data );
			}
		} catch( e ) {
			xml = undefined;
		}
		if ( !xml || !xml.documentElement || xml.getElementsByTagName( "parsererror" ).length ) {
			jQuery.error( "Invalid XML: " + data );
		}
		return xml;
	},

	noop: function() {},

	// Evaluates a script in a global context
	// Workarounds based on findings by Jim Driscoll
	// http://weblogs.java.net/blog/driscoll/archive/2009/09/08/eval-javascript-global-context
	globalEval: function( data ) {
		if ( data && rnotwhite.test( data ) ) {
			// We use execScript on Internet Explorer
			// We use an anonymous function so that context is window
			// rather than jQuery in Firefox
			( window.execScript || function( data ) {
				window[ "eval" ].call( window, data );
			} )( data );
		}
	},

	// Convert dashed to camelCase; used by the css and data modules
	// Microsoft forgot to hump their vendor prefix (#9572)
	camelCase: function( string ) {
		return string.replace( rmsPrefix, "ms-" ).replace( rdashAlpha, fcamelCase );
	},

	nodeName: function( elem, name ) {
		return elem.nodeName && elem.nodeName.toUpperCase() === name.toUpperCase();
	},

	// args is for internal usage only
	each: function( object, callback, args ) {
		var name, i = 0,
			length = object.length,
			isObj = length === undefined || jQuery.isFunction( object );

		if ( args ) {
			if ( isObj ) {
				for ( name in object ) {
					if ( callback.apply( object[ name ], args ) === false ) {
						break;
					}
				}
			} else {
				for ( ; i < length; ) {
					if ( callback.apply( object[ i++ ], args ) === false ) {
						break;
					}
				}
			}

		// A special, fast, case for the most common use of each
		} else {
			if ( isObj ) {
				for ( name in object ) {
					if ( callback.call( object[ name ], name, object[ name ] ) === false ) {
						break;
					}
				}
			} else {
				for ( ; i < length; ) {
					if ( callback.call( object[ i ], i, object[ i++ ] ) === false ) {
						break;
					}
				}
			}
		}

		return object;
	},

	// Use native String.trim function wherever possible
	trim: trim ?
		function( text ) {
			return text == null ?
				"" :
				trim.call( text );
		} :

		// Otherwise use our own trimming functionality
		function( text ) {
			return text == null ?
				"" :
				text.toString().replace( trimLeft, "" ).replace( trimRight, "" );
		},

	// results is for internal usage only
	makeArray: function( array, results ) {
		var ret = results || [];

		if ( array != null ) {
			// The window, strings (and functions) also have 'length'
			// Tweaked logic slightly to handle Blackberry 4.7 RegExp issues #6930
			var type = jQuery.type( array );

			if ( array.length == null || type === "string" || type === "function" || type === "regexp" || jQuery.isWindow( array ) ) {
				push.call( ret, array );
			} else {
				jQuery.merge( ret, array );
			}
		}

		return ret;
	},

	inArray: function( elem, array, i ) {
		var len;

		if ( array ) {
			if ( indexOf ) {
				return indexOf.call( array, elem, i );
			}

			len = array.length;
			i = i ? i < 0 ? Math.max( 0, len + i ) : i : 0;

			for ( ; i < len; i++ ) {
				// Skip accessing in sparse arrays
				if ( i in array && array[ i ] === elem ) {
					return i;
				}
			}
		}

		return -1;
	},

	merge: function( first, second ) {
		var i = first.length,
			j = 0;

		if ( typeof second.length === "number" ) {
			for ( var l = second.length; j < l; j++ ) {
				first[ i++ ] = second[ j ];
			}

		} else {
			while ( second[j] !== undefined ) {
				first[ i++ ] = second[ j++ ];
			}
		}

		first.length = i;

		return first;
	},

	grep: function( elems, callback, inv ) {
		var ret = [], retVal;
		inv = !!inv;

		// Go through the array, only saving the items
		// that pass the validator function
		for ( var i = 0, length = elems.length; i < length; i++ ) {
			retVal = !!callback( elems[ i ], i );
			if ( inv !== retVal ) {
				ret.push( elems[ i ] );
			}
		}

		return ret;
	},

	// arg is for internal usage only
	map: function( elems, callback, arg ) {
		var value, key, ret = [],
			i = 0,
			length = elems.length,
			// jquery objects are treated as arrays
			isArray = elems instanceof jQuery || length !== undefined && typeof length === "number" && ( ( length > 0 && elems[ 0 ] && elems[ length -1 ] ) || length === 0 || jQuery.isArray( elems ) ) ;

		// Go through the array, translating each of the items to their
		if ( isArray ) {
			for ( ; i < length; i++ ) {
				value = callback( elems[ i ], i, arg );

				if ( value != null ) {
					ret[ ret.length ] = value;
				}
			}

		// Go through every key on the object,
		} else {
			for ( key in elems ) {
				value = callback( elems[ key ], key, arg );

				if ( value != null ) {
					ret[ ret.length ] = value;
				}
			}
		}

		// Flatten any nested arrays
		return ret.concat.apply( [], ret );
	},

	// A global GUID counter for objects
	guid: 1,

	// Bind a function to a context, optionally partially applying any
	// arguments.
	proxy: function( fn, context ) {
		if ( typeof context === "string" ) {
			var tmp = fn[ context ];
			context = fn;
			fn = tmp;
		}

		// Quick check to determine if target is callable, in the spec
		// this throws a TypeError, but we will just return undefined.
		if ( !jQuery.isFunction( fn ) ) {
			return undefined;
		}

		// Simulated bind
		var args = slice.call( arguments, 2 ),
			proxy = function() {
				return fn.apply( context, args.concat( slice.call( arguments ) ) );
			};

		// Set the guid of unique handler to the same of original handler, so it can be removed
		proxy.guid = fn.guid = fn.guid || proxy.guid || jQuery.guid++;

		return proxy;
	},

	// Mutifunctional method to get and set values to a collection
	// The value/s can optionally be executed if it's a function
	access: function( elems, fn, key, value, chainable, emptyGet, pass ) {
		var exec,
			bulk = key == null,
			i = 0,
			length = elems.length;

		// Sets many values
		if ( key && typeof key === "object" ) {
			for ( i in key ) {
				jQuery.access( elems, fn, i, key[i], 1, emptyGet, value );
			}
			chainable = 1;

		// Sets one value
		} else if ( value !== undefined ) {
			// Optionally, function values get executed if exec is true
			exec = pass === undefined && jQuery.isFunction( value );

			if ( bulk ) {
				// Bulk operations only iterate when executing function values
				if ( exec ) {
					exec = fn;
					fn = function( elem, key, value ) {
						return exec.call( jQuery( elem ), value );
					};

				// Otherwise they run against the entire set
				} else {
					fn.call( elems, value );
					fn = null;
				}
			}

			if ( fn ) {
				for (; i < length; i++ ) {
					fn( elems[i], key, exec ? value.call( elems[i], i, fn( elems[i], key ) ) : value, pass );
				}
			}

			chainable = 1;
		}

		return chainable ?
			elems :

			// Gets
			bulk ?
				fn.call( elems ) :
				length ? fn( elems[0], key ) : emptyGet;
	},

	now: function() {
		return ( new Date() ).getTime();
	},

	// Use of jQuery.browser is frowned upon.
	// More details: http://docs.jquery.com/Utilities/jQuery.browser
	uaMatch: function( ua ) {
		ua = ua.toLowerCase();

		var match = rwebkit.exec( ua ) ||
			ropera.exec( ua ) ||
			rmsie.exec( ua ) ||
			ua.indexOf("compatible") < 0 && rmozilla.exec( ua ) ||
			[];

		return { browser: match[1] || "", version: match[2] || "0" };
	},

	sub: function() {
		function jQuerySub( selector, context ) {
			return new jQuerySub.fn.init( selector, context );
		}
		jQuery.extend( true, jQuerySub, this );
		jQuerySub.superclass = this;
		jQuerySub.fn = jQuerySub.prototype = this();
		jQuerySub.fn.constructor = jQuerySub;
		jQuerySub.sub = this.sub;
		jQuerySub.fn.init = function init( selector, context ) {
			if ( context && context instanceof jQuery && !(context instanceof jQuerySub) ) {
				context = jQuerySub( context );
			}

			return jQuery.fn.init.call( this, selector, context, rootjQuerySub );
		};
		jQuerySub.fn.init.prototype = jQuerySub.fn;
		var rootjQuerySub = jQuerySub(document);
		return jQuerySub;
	},

	browser: {}
});

// Populate the class2type map
jQuery.each("Boolean Number String Function Array Date RegExp Object".split(" "), function(i, name) {
	class2type[ "[object " + name + "]" ] = name.toLowerCase();
});

browserMatch = jQuery.uaMatch( userAgent );
if ( browserMatch.browser ) {
	jQuery.browser[ browserMatch.browser ] = true;
	jQuery.browser.version = browserMatch.version;
}

// Deprecated, use jQuery.browser.webkit instead
if ( jQuery.browser.webkit ) {
	jQuery.browser.safari = true;
}

// IE doesn't match non-breaking spaces with \s
if ( rnotwhite.test( "\xA0" ) ) {
	trimLeft = /^[\s\xA0]+/;
	trimRight = /[\s\xA0]+$/;
}

// All jQuery objects should point back to these
rootjQuery = jQuery(document);

// Cleanup functions for the document ready method
if ( document.addEventListener ) {
	DOMContentLoaded = function() {
		document.removeEventListener( "DOMContentLoaded", DOMContentLoaded, false );
		jQuery.ready();
	};

} else if ( document.attachEvent ) {
	DOMContentLoaded = function() {
		// Make sure body exists, at least, in case IE gets a little overzealous (ticket #5443).
		if ( document.readyState === "complete" ) {
			document.detachEvent( "onreadystatechange", DOMContentLoaded );
			jQuery.ready();
		}
	};
}

// The DOM ready check for Internet Explorer
function doScrollCheck() {
	if ( jQuery.isReady ) {
		return;
	}

	try {
		// If IE is used, use the trick by Diego Perini
		// http://javascript.nwbox.com/IEContentLoaded/
		document.documentElement.doScroll("left");
	} catch(e) {
		setTimeout( doScrollCheck, 1 );
		return;
	}

	// and execute any waiting functions
	jQuery.ready();
}

return jQuery;

})();


// String to Object flags format cache
var flagsCache = {};

// Convert String-formatted flags into Object-formatted ones and store in cache
function createFlags( flags ) {
	var object = flagsCache[ flags ] = {},
		i, length;
	flags = flags.split( /\s+/ );
	for ( i = 0, length = flags.length; i < length; i++ ) {
		object[ flags[i] ] = true;
	}
	return object;
}

/*
 * Create a callback list using the following parameters:
 *
 *	flags:	an optional list of space-separated flags that will change how
 *			the callback list behaves
 *
 * By default a callback list will act like an event callback list and can be
 * "fired" multiple times.
 *
 * Possible flags:
 *
 *	once:			will ensure the callback list can only be fired once (like a Deferred)
 *
 *	memory:			will keep track of previous values and will call any callback added
 *					after the list has been fired right away with the latest "memorized"
 *					values (like a Deferred)
 *
 *	unique:			will ensure a callback can only be added once (no duplicate in the list)
 *
 *	stopOnFalse:	interrupt callings when a callback returns false
 *
 */
jQuery.Callbacks = function( flags ) {

	// Convert flags from String-formatted to Object-formatted
	// (we check in cache first)
	flags = flags ? ( flagsCache[ flags ] || createFlags( flags ) ) : {};

	var // Actual callback list
		list = [],
		// Stack of fire calls for repeatable lists
		stack = [],
		// Last fire value (for non-forgettable lists)
		memory,
		// Flag to know if list was already fired
		fired,
		// Flag to know if list is currently firing
		firing,
		// First callback to fire (used internally by add and fireWith)
		firingStart,
		// End of the loop when firing
		firingLength,
		// Index of currently firing callback (modified by remove if needed)
		firingIndex,
		// Add one or several callbacks to the list
		add = function( args ) {
			var i,
				length,
				elem,
				type,
				actual;
			for ( i = 0, length = args.length; i < length; i++ ) {
				elem = args[ i ];
				type = jQuery.type( elem );
				if ( type === "array" ) {
					// Inspect recursively
					add( elem );
				} else if ( type === "function" ) {
					// Add if not in unique mode and callback is not in
					if ( !flags.unique || !self.has( elem ) ) {
						list.push( elem );
					}
				}
			}
		},
		// Fire callbacks
		fire = function( context, args ) {
			args = args || [];
			memory = !flags.memory || [ context, args ];
			fired = true;
			firing = true;
			firingIndex = firingStart || 0;
			firingStart = 0;
			firingLength = list.length;
			for ( ; list && firingIndex < firingLength; firingIndex++ ) {
				if ( list[ firingIndex ].apply( context, args ) === false && flags.stopOnFalse ) {
					memory = true; // Mark as halted
					break;
				}
			}
			firing = false;
			if ( list ) {
				if ( !flags.once ) {
					if ( stack && stack.length ) {
						memory = stack.shift();
						self.fireWith( memory[ 0 ], memory[ 1 ] );
					}
				} else if ( memory === true ) {
					self.disable();
				} else {
					list = [];
				}
			}
		},
		// Actual Callbacks object
		self = {
			// Add a callback or a collection of callbacks to the list
			add: function() {
				if ( list ) {
					var length = list.length;
					add( arguments );
					// Do we need to add the callbacks to the
					// current firing batch?
					if ( firing ) {
						firingLength = list.length;
					// With memory, if we're not firing then
					// we should call right away, unless previous
					// firing was halted (stopOnFalse)
					} else if ( memory && memory !== true ) {
						firingStart = length;
						fire( memory[ 0 ], memory[ 1 ] );
					}
				}
				return this;
			},
			// Remove a callback from the list
			remove: function() {
				if ( list ) {
					var args = arguments,
						argIndex = 0,
						argLength = args.length;
					for ( ; argIndex < argLength ; argIndex++ ) {
						for ( var i = 0; i < list.length; i++ ) {
							if ( args[ argIndex ] === list[ i ] ) {
								// Handle firingIndex and firingLength
								if ( firing ) {
									if ( i <= firingLength ) {
										firingLength--;
										if ( i <= firingIndex ) {
											firingIndex--;
										}
									}
								}
								// Remove the element
								list.splice( i--, 1 );
								// If we have some unicity property then
								// we only need to do this once
								if ( flags.unique ) {
									break;
								}
							}
						}
					}
				}
				return this;
			},
			// Control if a given callback is in the list
			has: function( fn ) {
				if ( list ) {
					var i = 0,
						length = list.length;
					for ( ; i < length; i++ ) {
						if ( fn === list[ i ] ) {
							return true;
						}
					}
				}
				return false;
			},
			// Remove all callbacks from the list
			empty: function() {
				list = [];
				return this;
			},
			// Have the list do nothing anymore
			disable: function() {
				list = stack = memory = undefined;
				return this;
			},
			// Is it disabled?
			disabled: function() {
				return !list;
			},
			// Lock the list in its current state
			lock: function() {
				stack = undefined;
				if ( !memory || memory === true ) {
					self.disable();
				}
				return this;
			},
			// Is it locked?
			locked: function() {
				return !stack;
			},
			// Call all callbacks with the given context and arguments
			fireWith: function( context, args ) {
				if ( stack ) {
					if ( firing ) {
						if ( !flags.once ) {
							stack.push( [ context, args ] );
						}
					} else if ( !( flags.once && memory ) ) {
						fire( context, args );
					}
				}
				return this;
			},
			// Call all the callbacks with the given arguments
			fire: function() {
				self.fireWith( this, arguments );
				return this;
			},
			// To know if the callbacks have already been called at least once
			fired: function() {
				return !!fired;
			}
		};

	return self;
};




var // Static reference to slice
	sliceDeferred = [].slice;

jQuery.extend({

	Deferred: function( func ) {
		var doneList = jQuery.Callbacks( "once memory" ),
			failList = jQuery.Callbacks( "once memory" ),
			progressList = jQuery.Callbacks( "memory" ),
			state = "pending",
			lists = {
				resolve: doneList,
				reject: failList,
				notify: progressList
			},
			promise = {
				done: doneList.add,
				fail: failList.add,
				progress: progressList.add,

				state: function() {
					return state;
				},

				// Deprecated
				isResolved: doneList.fired,
				isRejected: failList.fired,

				then: function( doneCallbacks, failCallbacks, progressCallbacks ) {
					deferred.done( doneCallbacks ).fail( failCallbacks ).progress( progressCallbacks );
					return this;
				},
				always: function() {
					deferred.done.apply( deferred, arguments ).fail.apply( deferred, arguments );
					return this;
				},
				pipe: function( fnDone, fnFail, fnProgress ) {
					return jQuery.Deferred(function( newDefer ) {
						jQuery.each( {
							done: [ fnDone, "resolve" ],
							fail: [ fnFail, "reject" ],
							progress: [ fnProgress, "notify" ]
						}, function( handler, data ) {
							var fn = data[ 0 ],
								action = data[ 1 ],
								returned;
							if ( jQuery.isFunction( fn ) ) {
								deferred[ handler ](function() {
									returned = fn.apply( this, arguments );
									if ( returned && jQuery.isFunction( returned.promise ) ) {
										returned.promise().then( newDefer.resolve, newDefer.reject, newDefer.notify );
									} else {
										newDefer[ action + "With" ]( this === deferred ? newDefer : this, [ returned ] );
									}
								});
							} else {
								deferred[ handler ]( newDefer[ action ] );
							}
						});
					}).promise();
				},
				// Get a promise for this deferred
				// If obj is provided, the promise aspect is added to the object
				promise: function( obj ) {
					if ( obj == null ) {
						obj = promise;
					} else {
						for ( var key in promise ) {
							obj[ key ] = promise[ key ];
						}
					}
					return obj;
				}
			},
			deferred = promise.promise({}),
			key;

		for ( key in lists ) {
			deferred[ key ] = lists[ key ].fire;
			deferred[ key + "With" ] = lists[ key ].fireWith;
		}

		// Handle state
		deferred.done( function() {
			state = "resolved";
		}, failList.disable, progressList.lock ).fail( function() {
			state = "rejected";
		}, doneList.disable, progressList.lock );

		// Call given func if any
		if ( func ) {
			func.call( deferred, deferred );
		}

		// All done!
		return deferred;
	},

	// Deferred helper
	when: function( firstParam ) {
		var args = sliceDeferred.call( arguments, 0 ),
			i = 0,
			length = args.length,
			pValues = new Array( length ),
			count = length,
			pCount = length,
			deferred = length <= 1 && firstParam && jQuery.isFunction( firstParam.promise ) ?
				firstParam :
				jQuery.Deferred(),
			promise = deferred.promise();
		function resolveFunc( i ) {
			return function( value ) {
				args[ i ] = arguments.length > 1 ? sliceDeferred.call( arguments, 0 ) : value;
				if ( !( --count ) ) {
					deferred.resolveWith( deferred, args );
				}
			};
		}
		function progressFunc( i ) {
			return function( value ) {
				pValues[ i ] = arguments.length > 1 ? sliceDeferred.call( arguments, 0 ) : value;
				deferred.notifyWith( promise, pValues );
			};
		}
		if ( length > 1 ) {
			for ( ; i < length; i++ ) {
				if ( args[ i ] && args[ i ].promise && jQuery.isFunction( args[ i ].promise ) ) {
					args[ i ].promise().then( resolveFunc(i), deferred.reject, progressFunc(i) );
				} else {
					--count;
				}
			}
			if ( !count ) {
				deferred.resolveWith( deferred, args );
			}
		} else if ( deferred !== firstParam ) {
			deferred.resolveWith( deferred, length ? [ firstParam ] : [] );
		}
		return promise;
	}
});




jQuery.support = (function() {

	var support,
		all,
		a,
		select,
		opt,
		input,
		fragment,
		tds,
		events,
		eventName,
		i,
		isSupported,
		div = document.createElement( "div" ),
		documentElement = document.documentElement;

	// Preliminary tests
	div.setAttribute("className", "t");
	div.innerHTML = "   <link/><table></table><a href='/a' style='top:1px;float:left;opacity:.55;'>a</a><input type='checkbox'/>";

	all = div.getElementsByTagName( "*" );
	a = div.getElementsByTagName( "a" )[ 0 ];

	// Can't get basic test support
	if ( !all || !all.length || !a ) {
		return {};
	}

	// First batch of supports tests
	select = document.createElement( "select" );
	opt = select.appendChild( document.createElement("option") );
	input = div.getElementsByTagName( "input" )[ 0 ];

	support = {
		// IE strips leading whitespace when .innerHTML is used
		leadingWhitespace: ( div.firstChild.nodeType === 3 ),

		// Make sure that tbody elements aren't automatically inserted
		// IE will insert them into empty tables
		tbody: !div.getElementsByTagName("tbody").length,

		// Make sure that link elements get serialized correctly by innerHTML
		// This requires a wrapper element in IE
		htmlSerialize: !!div.getElementsByTagName("link").length,

		// Get the style information from getAttribute
		// (IE uses .cssText instead)
		style: /top/.test( a.getAttribute("style") ),

		// Make sure that URLs aren't manipulated
		// (IE normalizes it by default)
		hrefNormalized: ( a.getAttribute("href") === "/a" ),

		// Make sure that element opacity exists
		// (IE uses filter instead)
		// Use a regex to work around a WebKit issue. See #5145
		opacity: /^0.55/.test( a.style.opacity ),

		// Verify style float existence
		// (IE uses styleFloat instead of cssFloat)
		cssFloat: !!a.style.cssFloat,

		// Make sure that if no value is specified for a checkbox
		// that it defaults to "on".
		// (WebKit defaults to "" instead)
		checkOn: ( input.value === "on" ),

		// Make sure that a selected-by-default option has a working selected property.
		// (WebKit defaults to false instead of true, IE too, if it's in an optgroup)
		optSelected: opt.selected,

		// Test setAttribute on camelCase class. If it works, we need attrFixes when doing get/setAttribute (ie6/7)
		getSetAttribute: div.className !== "t",

		// Tests for enctype support on a form(#6743)
		enctype: !!document.createElement("form").enctype,

		// Makes sure cloning an html5 element does not cause problems
		// Where outerHTML is undefined, this still works
		html5Clone: document.createElement("nav").cloneNode( true ).outerHTML !== "<:nav></:nav>",

		// Will be defined later
		submitBubbles: true,
		changeBubbles: true,
		focusinBubbles: false,
		deleteExpando: true,
		noCloneEvent: true,
		inlineBlockNeedsLayout: false,
		shrinkWrapBlocks: false,
		reliableMarginRight: true,
		pixelMargin: true
	};

	// jQuery.boxModel DEPRECATED in 1.3, use jQuery.support.boxModel instead
	jQuery.boxModel = support.boxModel = (document.compatMode === "CSS1Compat");

	// Make sure checked status is properly cloned
	input.checked = true;
	support.noCloneChecked = input.cloneNode( true ).checked;

	// Make sure that the options inside disabled selects aren't marked as disabled
	// (WebKit marks them as disabled)
	select.disabled = true;
	support.optDisabled = !opt.disabled;

	// Test to see if it's possible to delete an expando from an element
	// Fails in Internet Explorer
	try {
		delete div.test;
	} catch( e ) {
		support.deleteExpando = false;
	}

	if ( !div.addEventListener && div.attachEvent && div.fireEvent ) {
		div.attachEvent( "onclick", function() {
			// Cloning a node shouldn't copy over any
			// bound event handlers (IE does this)
			support.noCloneEvent = false;
		});
		div.cloneNode( true ).fireEvent( "onclick" );
	}

	// Check if a radio maintains its value
	// after being appended to the DOM
	input = document.createElement("input");
	input.value = "t";
	input.setAttribute("type", "radio");
	support.radioValue = input.value === "t";

	input.setAttribute("checked", "checked");

	// #11217 - WebKit loses check when the name is after the checked attribute
	input.setAttribute( "name", "t" );

	div.appendChild( input );
	fragment = document.createDocumentFragment();
	fragment.appendChild( div.lastChild );

	// WebKit doesn't clone checked state correctly in fragments
	support.checkClone = fragment.cloneNode( true ).cloneNode( true ).lastChild.checked;

	// Check if a disconnected checkbox will retain its checked
	// value of true after appended to the DOM (IE6/7)
	support.appendChecked = input.checked;

	fragment.removeChild( input );
	fragment.appendChild( div );

	// Technique from Juriy Zaytsev
	// http://perfectionkills.com/detecting-event-support-without-browser-sniffing/
	// We only care about the case where non-standard event systems
	// are used, namely in IE. Short-circuiting here helps us to
	// avoid an eval call (in setAttribute) which can cause CSP
	// to go haywire. See: https://developer.mozilla.org/en/Security/CSP
	if ( div.attachEvent ) {
		for ( i in {
			submit: 1,
			change: 1,
			focusin: 1
		}) {
			eventName = "on" + i;
			isSupported = ( eventName in div );
			if ( !isSupported ) {
				div.setAttribute( eventName, "return;" );
				isSupported = ( typeof div[ eventName ] === "function" );
			}
			support[ i + "Bubbles" ] = isSupported;
		}
	}

	fragment.removeChild( div );

	// Null elements to avoid leaks in IE
	fragment = select = opt = div = input = null;

	// Run tests that need a body at doc ready
	jQuery(function() {
		var container, outer, inner, table, td, offsetSupport,
			marginDiv, conMarginTop, style, html, positionTopLeftWidthHeight,
			paddingMarginBorderVisibility, paddingMarginBorder,
			body = document.getElementsByTagName("body")[0];

		if ( !body ) {
			// Return for frameset docs that don't have a body
			return;
		}

		conMarginTop = 1;
		paddingMarginBorder = "padding:0;margin:0;border:";
		positionTopLeftWidthHeight = "position:absolute;top:0;left:0;width:1px;height:1px;";
		paddingMarginBorderVisibility = paddingMarginBorder + "0;visibility:hidden;";
		style = "style='" + positionTopLeftWidthHeight + paddingMarginBorder + "5px solid #000;";
		html = "<div " + style + "display:block;'><div style='" + paddingMarginBorder + "0;display:block;overflow:hidden;'></div></div>" +
			"<table " + style + "' cellpadding='0' cellspacing='0'>" +
			"<tr><td></td></tr></table>";

		container = document.createElement("div");
		container.style.cssText = paddingMarginBorderVisibility + "width:0;height:0;position:static;top:0;margin-top:" + conMarginTop + "px";
		body.insertBefore( container, body.firstChild );

		// Construct the test element
		div = document.createElement("div");
		container.appendChild( div );

		// Check if table cells still have offsetWidth/Height when they are set
		// to display:none and there are still other visible table cells in a
		// table row; if so, offsetWidth/Height are not reliable for use when
		// determining if an element has been hidden directly using
		// display:none (it is still safe to use offsets if a parent element is
		// hidden; don safety goggles and see bug #4512 for more information).
		// (only IE 8 fails this test)
		div.innerHTML = "<table><tr><td style='" + paddingMarginBorder + "0;display:none'></td><td>t</td></tr></table>";
		tds = div.getElementsByTagName( "td" );
		isSupported = ( tds[ 0 ].offsetHeight === 0 );

		tds[ 0 ].style.display = "";
		tds[ 1 ].style.display = "none";

		// Check if empty table cells still have offsetWidth/Height
		// (IE <= 8 fail this test)
		support.reliableHiddenOffsets = isSupported && ( tds[ 0 ].offsetHeight === 0 );

		// Check if div with explicit width and no margin-right incorrectly
		// gets computed margin-right based on width of container. For more
		// info see bug #3333
		// Fails in WebKit before Feb 2011 nightlies
		// WebKit Bug 13343 - getComputedStyle returns wrong value for margin-right
		if ( window.getComputedStyle ) {
			div.innerHTML = "";
			marginDiv = document.createElement( "div" );
			marginDiv.style.width = "0";
			marginDiv.style.marginRight = "0";
			div.style.width = "2px";
			div.appendChild( marginDiv );
			support.reliableMarginRight =
				( parseInt( ( window.getComputedStyle( marginDiv, null ) || { marginRight: 0 } ).marginRight, 10 ) || 0 ) === 0;
		}

		if ( typeof div.style.zoom !== "undefined" ) {
			// Check if natively block-level elements act like inline-block
			// elements when setting their display to 'inline' and giving
			// them layout
			// (IE < 8 does this)
			div.innerHTML = "";
			div.style.width = div.style.padding = "1px";
			div.style.border = 0;
			div.style.overflow = "hidden";
			div.style.display = "inline";
			div.style.zoom = 1;
			support.inlineBlockNeedsLayout = ( div.offsetWidth === 3 );

			// Check if elements with layout shrink-wrap their children
			// (IE 6 does this)
			div.style.display = "block";
			div.style.overflow = "visible";
			div.innerHTML = "<div style='width:5px;'></div>";
			support.shrinkWrapBlocks = ( div.offsetWidth !== 3 );
		}

		div.style.cssText = positionTopLeftWidthHeight + paddingMarginBorderVisibility;
		div.innerHTML = html;

		outer = div.firstChild;
		inner = outer.firstChild;
		td = outer.nextSibling.firstChild.firstChild;

		offsetSupport = {
			doesNotAddBorder: ( inner.offsetTop !== 5 ),
			doesAddBorderForTableAndCells: ( td.offsetTop === 5 )
		};

		inner.style.position = "fixed";
		inner.style.top = "20px";

		// safari subtracts parent border width here which is 5px
		offsetSupport.fixedPosition = ( inner.offsetTop === 20 || inner.offsetTop === 15 );
		inner.style.position = inner.style.top = "";

		outer.style.overflow = "hidden";
		outer.style.position = "relative";

		offsetSupport.subtractsBorderForOverflowNotVisible = ( inner.offsetTop === -5 );
		offsetSupport.doesNotIncludeMarginInBodyOffset = ( body.offsetTop !== conMarginTop );

		if ( window.getComputedStyle ) {
			div.style.marginTop = "1%";
			support.pixelMargin = ( window.getComputedStyle( div, null ) || { marginTop: 0 } ).marginTop !== "1%";
		}

		if ( typeof container.style.zoom !== "undefined" ) {
			container.style.zoom = 1;
		}

		body.removeChild( container );
		marginDiv = div = container = null;

		jQuery.extend( support, offsetSupport );
	});

	return support;
})();




var rbrace = /^(?:\{.*\}|\[.*\])$/,
	rmultiDash = /([A-Z])/g;

jQuery.extend({
	cache: {},

	// Please use with caution
	uuid: 0,

	// Unique for each copy of jQuery on the page
	// Non-digits removed to match rinlinejQuery
	expando: "jQuery" + ( jQuery.fn.jquery + Math.random() ).replace( /\D/g, "" ),

	// The following elements throw uncatchable exceptions if you
	// attempt to add expando properties to them.
	noData: {
		"embed": true,
		// Ban all objects except for Flash (which handle expandos)
		"object": "clsid:D27CDB6E-AE6D-11cf-96B8-444553540000",
		"applet": true
	},

	hasData: function( elem ) {
		elem = elem.nodeType ? jQuery.cache[ elem[jQuery.expando] ] : elem[ jQuery.expando ];
		return !!elem && !isEmptyDataObject( elem );
	},

	data: function( elem, name, data, pvt /* Internal Use Only */ ) {
		if ( !jQuery.acceptData( elem ) ) {
			return;
		}

		var privateCache, thisCache, ret,
			internalKey = jQuery.expando,
			getByName = typeof name === "string",

			// We have to handle DOM nodes and JS objects differently because IE6-7
			// can't GC object references properly across the DOM-JS boundary
			isNode = elem.nodeType,

			// Only DOM nodes need the global jQuery cache; JS object data is
			// attached directly to the object so GC can occur automatically
			cache = isNode ? jQuery.cache : elem,

			// Only defining an ID for JS objects if its cache already exists allows
			// the code to shortcut on the same path as a DOM node with no cache
			id = isNode ? elem[ internalKey ] : elem[ internalKey ] && internalKey,
			isEvents = name === "events";

		// Avoid doing any more work than we need to when trying to get data on an
		// object that has no data at all
		if ( (!id || !cache[id] || (!isEvents && !pvt && !cache[id].data)) && getByName && data === undefined ) {
			return;
		}

		if ( !id ) {
			// Only DOM nodes need a new unique ID for each element since their data
			// ends up in the global cache
			if ( isNode ) {
				elem[ internalKey ] = id = ++jQuery.uuid;
			} else {
				id = internalKey;
			}
		}

		if ( !cache[ id ] ) {
			cache[ id ] = {};

			// Avoids exposing jQuery metadata on plain JS objects when the object
			// is serialized using JSON.stringify
			if ( !isNode ) {
				cache[ id ].toJSON = jQuery.noop;
			}
		}

		// An object can be passed to jQuery.data instead of a key/value pair; this gets
		// shallow copied over onto the existing cache
		if ( typeof name === "object" || typeof name === "function" ) {
			if ( pvt ) {
				cache[ id ] = jQuery.extend( cache[ id ], name );
			} else {
				cache[ id ].data = jQuery.extend( cache[ id ].data, name );
			}
		}

		privateCache = thisCache = cache[ id ];

		// jQuery data() is stored in a separate object inside the object's internal data
		// cache in order to avoid key collisions between internal data and user-defined
		// data.
		if ( !pvt ) {
			if ( !thisCache.data ) {
				thisCache.data = {};
			}

			thisCache = thisCache.data;
		}

		if ( data !== undefined ) {
			thisCache[ jQuery.camelCase( name ) ] = data;
		}

		// Users should not attempt to inspect the internal events object using jQuery.data,
		// it is undocumented and subject to change. But does anyone listen? No.
		if ( isEvents && !thisCache[ name ] ) {
			return privateCache.events;
		}

		// Check for both converted-to-camel and non-converted data property names
		// If a data property was specified
		if ( getByName ) {

			// First Try to find as-is property data
			ret = thisCache[ name ];

			// Test for null|undefined property data
			if ( ret == null ) {

				// Try to find the camelCased property
				ret = thisCache[ jQuery.camelCase( name ) ];
			}
		} else {
			ret = thisCache;
		}

		return ret;
	},

	removeData: function( elem, name, pvt /* Internal Use Only */ ) {
		if ( !jQuery.acceptData( elem ) ) {
			return;
		}

		var thisCache, i, l,

			// Reference to internal data cache key
			internalKey = jQuery.expando,

			isNode = elem.nodeType,

			// See jQuery.data for more information
			cache = isNode ? jQuery.cache : elem,

			// See jQuery.data for more information
			id = isNode ? elem[ internalKey ] : internalKey;

		// If there is already no cache entry for this object, there is no
		// purpose in continuing
		if ( !cache[ id ] ) {
			return;
		}

		if ( name ) {

			thisCache = pvt ? cache[ id ] : cache[ id ].data;

			if ( thisCache ) {

				// Support array or space separated string names for data keys
				if ( !jQuery.isArray( name ) ) {

					// try the string as a key before any manipulation
					if ( name in thisCache ) {
						name = [ name ];
					} else {

						// split the camel cased version by spaces unless a key with the spaces exists
						name = jQuery.camelCase( name );
						if ( name in thisCache ) {
							name = [ name ];
						} else {
							name = name.split( " " );
						}
					}
				}

				for ( i = 0, l = name.length; i < l; i++ ) {
					delete thisCache[ name[i] ];
				}

				// If there is no data left in the cache, we want to continue
				// and let the cache object itself get destroyed
				if ( !( pvt ? isEmptyDataObject : jQuery.isEmptyObject )( thisCache ) ) {
					return;
				}
			}
		}

		// See jQuery.data for more information
		if ( !pvt ) {
			delete cache[ id ].data;

			// Don't destroy the parent cache unless the internal data object
			// had been the only thing left in it
			if ( !isEmptyDataObject(cache[ id ]) ) {
				return;
			}
		}

		// Browsers that fail expando deletion also refuse to delete expandos on
		// the window, but it will allow it on all other JS objects; other browsers
		// don't care
		// Ensure that `cache` is not a window object #10080
		if ( jQuery.support.deleteExpando || !cache.setInterval ) {
			delete cache[ id ];
		} else {
			cache[ id ] = null;
		}

		// We destroyed the cache and need to eliminate the expando on the node to avoid
		// false lookups in the cache for entries that no longer exist
		if ( isNode ) {
			// IE does not allow us to delete expando properties from nodes,
			// nor does it have a removeAttribute function on Document nodes;
			// we must handle all of these cases
			if ( jQuery.support.deleteExpando ) {
				delete elem[ internalKey ];
			} else if ( elem.removeAttribute ) {
				elem.removeAttribute( internalKey );
			} else {
				elem[ internalKey ] = null;
			}
		}
	},

	// For internal use only.
	_data: function( elem, name, data ) {
		return jQuery.data( elem, name, data, true );
	},

	// A method for determining if a DOM node can handle the data expando
	acceptData: function( elem ) {
		if ( elem.nodeName ) {
			var match = jQuery.noData[ elem.nodeName.toLowerCase() ];

			if ( match ) {
				return !(match === true || elem.getAttribute("classid") !== match);
			}
		}

		return true;
	}
});

jQuery.fn.extend({
	data: function( key, value ) {
		var parts, part, attr, name, l,
			elem = this[0],
			i = 0,
			data = null;

		// Gets all values
		if ( key === undefined ) {
			if ( this.length ) {
				data = jQuery.data( elem );

				if ( elem.nodeType === 1 && !jQuery._data( elem, "parsedAttrs" ) ) {
					attr = elem.attributes;
					for ( l = attr.length; i < l; i++ ) {
						name = attr[i].name;

						if ( name.indexOf( "data-" ) === 0 ) {
							name = jQuery.camelCase( name.substring(5) );

							dataAttr( elem, name, data[ name ] );
						}
					}
					jQuery._data( elem, "parsedAttrs", true );
				}
			}

			return data;
		}

		// Sets multiple values
		if ( typeof key === "object" ) {
			return this.each(function() {
				jQuery.data( this, key );
			});
		}

		parts = key.split( ".", 2 );
		parts[1] = parts[1] ? "." + parts[1] : "";
		part = parts[1] + "!";

		return jQuery.access( this, function( value ) {

			if ( value === undefined ) {
				data = this.triggerHandler( "getData" + part, [ parts[0] ] );

				// Try to fetch any internally stored data first
				if ( data === undefined && elem ) {
					data = jQuery.data( elem, key );
					data = dataAttr( elem, key, data );
				}

				return data === undefined && parts[1] ?
					this.data( parts[0] ) :
					data;
			}

			parts[1] = value;
			this.each(function() {
				var self = jQuery( this );

				self.triggerHandler( "setData" + part, parts );
				jQuery.data( this, key, value );
				self.triggerHandler( "changeData" + part, parts );
			});
		}, null, value, arguments.length > 1, null, false );
	},

	removeData: function( key ) {
		return this.each(function() {
			jQuery.removeData( this, key );
		});
	}
});

function dataAttr( elem, key, data ) {
	// If nothing was found internally, try to fetch any
	// data from the HTML5 data-* attribute
	if ( data === undefined && elem.nodeType === 1 ) {

		var name = "data-" + key.replace( rmultiDash, "-$1" ).toLowerCase();

		data = elem.getAttribute( name );

		if ( typeof data === "string" ) {
			try {
				data = data === "true" ? true :
				data === "false" ? false :
				data === "null" ? null :
				jQuery.isNumeric( data ) ? +data :
					rbrace.test( data ) ? jQuery.parseJSON( data ) :
					data;
			} catch( e ) {}

			// Make sure we set the data so it isn't changed later
			jQuery.data( elem, key, data );

		} else {
			data = undefined;
		}
	}

	return data;
}

// checks a cache object for emptiness
function isEmptyDataObject( obj ) {
	for ( var name in obj ) {

		// if the public data object is empty, the private is still empty
		if ( name === "data" && jQuery.isEmptyObject( obj[name] ) ) {
			continue;
		}
		if ( name !== "toJSON" ) {
			return false;
		}
	}

	return true;
}




function handleQueueMarkDefer( elem, type, src ) {
	var deferDataKey = type + "defer",
		queueDataKey = type + "queue",
		markDataKey = type + "mark",
		defer = jQuery._data( elem, deferDataKey );
	if ( defer &&
		( src === "queue" || !jQuery._data(elem, queueDataKey) ) &&
		( src === "mark" || !jQuery._data(elem, markDataKey) ) ) {
		// Give room for hard-coded callbacks to fire first
		// and eventually mark/queue something else on the element
		setTimeout( function() {
			if ( !jQuery._data( elem, queueDataKey ) &&
				!jQuery._data( elem, markDataKey ) ) {
				jQuery.removeData( elem, deferDataKey, true );
				defer.fire();
			}
		}, 0 );
	}
}

jQuery.extend({

	_mark: function( elem, type ) {
		if ( elem ) {
			type = ( type || "fx" ) + "mark";
			jQuery._data( elem, type, (jQuery._data( elem, type ) || 0) + 1 );
		}
	},

	_unmark: function( force, elem, type ) {
		if ( force !== true ) {
			type = elem;
			elem = force;
			force = false;
		}
		if ( elem ) {
			type = type || "fx";
			var key = type + "mark",
				count = force ? 0 : ( (jQuery._data( elem, key ) || 1) - 1 );
			if ( count ) {
				jQuery._data( elem, key, count );
			} else {
				jQuery.removeData( elem, key, true );
				handleQueueMarkDefer( elem, type, "mark" );
			}
		}
	},

	queue: function( elem, type, data ) {
		var q;
		if ( elem ) {
			type = ( type || "fx" ) + "queue";
			q = jQuery._data( elem, type );

			// Speed up dequeue by getting out quickly if this is just a lookup
			if ( data ) {
				if ( !q || jQuery.isArray(data) ) {
					q = jQuery._data( elem, type, jQuery.makeArray(data) );
				} else {
					q.push( data );
				}
			}
			return q || [];
		}
	},

	dequeue: function( elem, type ) {
		type = type || "fx";

		var queue = jQuery.queue( elem, type ),
			fn = queue.shift(),
			hooks = {};

		// If the fx queue is dequeued, always remove the progress sentinel
		if ( fn === "inprogress" ) {
			fn = queue.shift();
		}

		if ( fn ) {
			// Add a progress sentinel to prevent the fx queue from being
			// automatically dequeued
			if ( type === "fx" ) {
				queue.unshift( "inprogress" );
			}

			jQuery._data( elem, type + ".run", hooks );
			fn.call( elem, function() {
				jQuery.dequeue( elem, type );
			}, hooks );
		}

		if ( !queue.length ) {
			jQuery.removeData( elem, type + "queue " + type + ".run", true );
			handleQueueMarkDefer( elem, type, "queue" );
		}
	}
});

jQuery.fn.extend({
	queue: function( type, data ) {
		var setter = 2;

		if ( typeof type !== "string" ) {
			data = type;
			type = "fx";
			setter--;
		}

		if ( arguments.length < setter ) {
			return jQuery.queue( this[0], type );
		}

		return data === undefined ?
			this :
			this.each(function() {
				var queue = jQuery.queue( this, type, data );

				if ( type === "fx" && queue[0] !== "inprogress" ) {
					jQuery.dequeue( this, type );
				}
			});
	},
	dequeue: function( type ) {
		return this.each(function() {
			jQuery.dequeue( this, type );
		});
	},
	// Based off of the plugin by Clint Helfers, with permission.
	// http://blindsignals.com/index.php/2009/07/jquery-delay/
	delay: function( time, type ) {
		time = jQuery.fx ? jQuery.fx.speeds[ time ] || time : time;
		type = type || "fx";

		return this.queue( type, function( next, hooks ) {
			var timeout = setTimeout( next, time );
			hooks.stop = function() {
				clearTimeout( timeout );
			};
		});
	},
	clearQueue: function( type ) {
		return this.queue( type || "fx", [] );
	},
	// Get a promise resolved when queues of a certain type
	// are emptied (fx is the type by default)
	promise: function( type, object ) {
		if ( typeof type !== "string" ) {
			object = type;
			type = undefined;
		}
		type = type || "fx";
		var defer = jQuery.Deferred(),
			elements = this,
			i = elements.length,
			count = 1,
			deferDataKey = type + "defer",
			queueDataKey = type + "queue",
			markDataKey = type + "mark",
			tmp;
		function resolve() {
			if ( !( --count ) ) {
				defer.resolveWith( elements, [ elements ] );
			}
		}
		while( i-- ) {
			if (( tmp = jQuery.data( elements[ i ], deferDataKey, undefined, true ) ||
					( jQuery.data( elements[ i ], queueDataKey, undefined, true ) ||
						jQuery.data( elements[ i ], markDataKey, undefined, true ) ) &&
					jQuery.data( elements[ i ], deferDataKey, jQuery.Callbacks( "once memory" ), true ) )) {
				count++;
				tmp.add( resolve );
			}
		}
		resolve();
		return defer.promise( object );
	}
});




var rclass = /[\n\t\r]/g,
	rspace = /\s+/,
	rreturn = /\r/g,
	rtype = /^(?:button|input)$/i,
	rfocusable = /^(?:button|input|object|select|textarea)$/i,
	rclickable = /^a(?:rea)?$/i,
	rboolean = /^(?:autofocus|autoplay|async|checked|controls|defer|disabled|hidden|loop|multiple|open|readonly|required|scoped|selected)$/i,
	getSetAttribute = jQuery.support.getSetAttribute,
	nodeHook, boolHook, fixSpecified;

jQuery.fn.extend({
	attr: function( name, value ) {
		return jQuery.access( this, jQuery.attr, name, value, arguments.length > 1 );
	},

	removeAttr: function( name ) {
		return this.each(function() {
			jQuery.removeAttr( this, name );
		});
	},

	prop: function( name, value ) {
		return jQuery.access( this, jQuery.prop, name, value, arguments.length > 1 );
	},

	removeProp: function( name ) {
		name = jQuery.propFix[ name ] || name;
		return this.each(function() {
			// try/catch handles cases where IE balks (such as removing a property on window)
			try {
				this[ name ] = undefined;
				delete this[ name ];
			} catch( e ) {}
		});
	},

	addClass: function( value ) {
		var classNames, i, l, elem,
			setClass, c, cl;

		if ( jQuery.isFunction( value ) ) {
			return this.each(function( j ) {
				jQuery( this ).addClass( value.call(this, j, this.className) );
			});
		}

		if ( value && typeof value === "string" ) {
			classNames = value.split( rspace );

			for ( i = 0, l = this.length; i < l; i++ ) {
				elem = this[ i ];

				if ( elem.nodeType === 1 ) {
					if ( !elem.className && classNames.length === 1 ) {
						elem.className = value;

					} else {
						setClass = " " + elem.className + " ";

						for ( c = 0, cl = classNames.length; c < cl; c++ ) {
							if ( !~setClass.indexOf( " " + classNames[ c ] + " " ) ) {
								setClass += classNames[ c ] + " ";
							}
						}
						elem.className = jQuery.trim( setClass );
					}
				}
			}
		}

		return this;
	},

	removeClass: function( value ) {
		var classNames, i, l, elem, className, c, cl;

		if ( jQuery.isFunction( value ) ) {
			return this.each(function( j ) {
				jQuery( this ).removeClass( value.call(this, j, this.className) );
			});
		}

		if ( (value && typeof value === "string") || value === undefined ) {
			classNames = ( value || "" ).split( rspace );

			for ( i = 0, l = this.length; i < l; i++ ) {
				elem = this[ i ];

				if ( elem.nodeType === 1 && elem.className ) {
					if ( value ) {
						className = (" " + elem.className + " ").replace( rclass, " " );
						for ( c = 0, cl = classNames.length; c < cl; c++ ) {
							className = className.replace(" " + classNames[ c ] + " ", " ");
						}
						elem.className = jQuery.trim( className );

					} else {
						elem.className = "";
					}
				}
			}
		}

		return this;
	},

	toggleClass: function( value, stateVal ) {
		var type = typeof value,
			isBool = typeof stateVal === "boolean";

		if ( jQuery.isFunction( value ) ) {
			return this.each(function( i ) {
				jQuery( this ).toggleClass( value.call(this, i, this.className, stateVal), stateVal );
			});
		}

		return this.each(function() {
			if ( type === "string" ) {
				// toggle individual class names
				var className,
					i = 0,
					self = jQuery( this ),
					state = stateVal,
					classNames = value.split( rspace );

				while ( (className = classNames[ i++ ]) ) {
					// check each className given, space seperated list
					state = isBool ? state : !self.hasClass( className );
					self[ state ? "addClass" : "removeClass" ]( className );
				}

			} else if ( type === "undefined" || type === "boolean" ) {
				if ( this.className ) {
					// store className if set
					jQuery._data( this, "__className__", this.className );
				}

				// toggle whole className
				this.className = this.className || value === false ? "" : jQuery._data( this, "__className__" ) || "";
			}
		});
	},

	hasClass: function( selector ) {
		var className = " " + selector + " ",
			i = 0,
			l = this.length;
		for ( ; i < l; i++ ) {
			if ( this[i].nodeType === 1 && (" " + this[i].className + " ").replace(rclass, " ").indexOf( className ) > -1 ) {
				return true;
			}
		}

		return false;
	},

	val: function( value ) {
		var hooks, ret, isFunction,
			elem = this[0];

		if ( !arguments.length ) {
			if ( elem ) {
				hooks = jQuery.valHooks[ elem.type ] || jQuery.valHooks[ elem.nodeName.toLowerCase() ];

				if ( hooks && "get" in hooks && (ret = hooks.get( elem, "value" )) !== undefined ) {
					return ret;
				}

				ret = elem.value;

				return typeof ret === "string" ?
					// handle most common string cases
					ret.replace(rreturn, "") :
					// handle cases where value is null/undef or number
					ret == null ? "" : ret;
			}

			return;
		}

		isFunction = jQuery.isFunction( value );

		return this.each(function( i ) {
			var self = jQuery(this), val;

			if ( this.nodeType !== 1 ) {
				return;
			}

			if ( isFunction ) {
				val = value.call( this, i, self.val() );
			} else {
				val = value;
			}

			// Treat null/undefined as ""; convert numbers to string
			if ( val == null ) {
				val = "";
			} else if ( typeof val === "number" ) {
				val += "";
			} else if ( jQuery.isArray( val ) ) {
				val = jQuery.map(val, function ( value ) {
					return value == null ? "" : value + "";
				});
			}

			hooks = jQuery.valHooks[ this.type ] || jQuery.valHooks[ this.nodeName.toLowerCase() ];

			// If set returns undefined, fall back to normal setting
			if ( !hooks || !("set" in hooks) || hooks.set( this, val, "value" ) === undefined ) {
				this.value = val;
			}
		});
	}
});

jQuery.extend({
	valHooks: {
		option: {
			get: function( elem ) {
				// attributes.value is undefined in Blackberry 4.7 but
				// uses .value. See #6932
				var val = elem.attributes.value;
				return !val || val.specified ? elem.value : elem.text;
			}
		},
		select: {
			get: function( elem ) {
				var value, i, max, option,
					index = elem.selectedIndex,
					values = [],
					options = elem.options,
					one = elem.type === "select-one";

				// Nothing was selected
				if ( index < 0 ) {
					return null;
				}

				// Loop through all the selected options
				i = one ? index : 0;
				max = one ? index + 1 : options.length;
				for ( ; i < max; i++ ) {
					option = options[ i ];

					// Don't return options that are disabled or in a disabled optgroup
					if ( option.selected && (jQuery.support.optDisabled ? !option.disabled : option.getAttribute("disabled") === null) &&
							(!option.parentNode.disabled || !jQuery.nodeName( option.parentNode, "optgroup" )) ) {

						// Get the specific value for the option
						value = jQuery( option ).val();

						// We don't need an array for one selects
						if ( one ) {
							return value;
						}

						// Multi-Selects return an array
						values.push( value );
					}
				}

				// Fixes Bug #2551 -- select.val() broken in IE after form.reset()
				if ( one && !values.length && options.length ) {
					return jQuery( options[ index ] ).val();
				}

				return values;
			},

			set: function( elem, value ) {
				var values = jQuery.makeArray( value );

				jQuery(elem).find("option").each(function() {
					this.selected = jQuery.inArray( jQuery(this).val(), values ) >= 0;
				});

				if ( !values.length ) {
					elem.selectedIndex = -1;
				}
				return values;
			}
		}
	},

	attrFn: {
		val: true,
		css: true,
		html: true,
		text: true,
		data: true,
		width: true,
		height: true,
		offset: true
	},

	attr: function( elem, name, value, pass ) {
		var ret, hooks, notxml,
			nType = elem.nodeType;

		// don't get/set attributes on text, comment and attribute nodes
		if ( !elem || nType === 3 || nType === 8 || nType === 2 ) {
			return;
		}

		if ( pass && name in jQuery.attrFn ) {
			return jQuery( elem )[ name ]( value );
		}

		// Fallback to prop when attributes are not supported
		if ( typeof elem.getAttribute === "undefined" ) {
			return jQuery.prop( elem, name, value );
		}

		notxml = nType !== 1 || !jQuery.isXMLDoc( elem );

		// All attributes are lowercase
		// Grab necessary hook if one is defined
		if ( notxml ) {
			name = name.toLowerCase();
			hooks = jQuery.attrHooks[ name ] || ( rboolean.test( name ) ? boolHook : nodeHook );
		}

		if ( value !== undefined ) {

			if ( value === null ) {
				jQuery.removeAttr( elem, name );
				return;

			} else if ( hooks && "set" in hooks && notxml && (ret = hooks.set( elem, value, name )) !== undefined ) {
				return ret;

			} else {
				elem.setAttribute( name, "" + value );
				return value;
			}

		} else if ( hooks && "get" in hooks && notxml && (ret = hooks.get( elem, name )) !== null ) {
			return ret;

		} else {

			ret = elem.getAttribute( name );

			// Non-existent attributes return null, we normalize to undefined
			return ret === null ?
				undefined :
				ret;
		}
	},

	removeAttr: function( elem, value ) {
		var propName, attrNames, name, l, isBool,
			i = 0;

		if ( value && elem.nodeType === 1 ) {
			attrNames = value.toLowerCase().split( rspace );
			l = attrNames.length;

			for ( ; i < l; i++ ) {
				name = attrNames[ i ];

				if ( name ) {
					propName = jQuery.propFix[ name ] || name;
					isBool = rboolean.test( name );

					// See #9699 for explanation of this approach (setting first, then removal)
					// Do not do this for boolean attributes (see #10870)
					if ( !isBool ) {
						jQuery.attr( elem, name, "" );
					}
					elem.removeAttribute( getSetAttribute ? name : propName );

					// Set corresponding property to false for boolean attributes
					if ( isBool && propName in elem ) {
						elem[ propName ] = false;
					}
				}
			}
		}
	},

	attrHooks: {
		type: {
			set: function( elem, value ) {
				// We can't allow the type property to be changed (since it causes problems in IE)
				if ( rtype.test( elem.nodeName ) && elem.parentNode ) {
					jQuery.error( "type property can't be changed" );
				} else if ( !jQuery.support.radioValue && value === "radio" && jQuery.nodeName(elem, "input") ) {
					// Setting the type on a radio button after the value resets the value in IE6-9
					// Reset value to it's default in case type is set after value
					// This is for element creation
					var val = elem.value;
					elem.setAttribute( "type", value );
					if ( val ) {
						elem.value = val;
					}
					return value;
				}
			}
		},
		// Use the value property for back compat
		// Use the nodeHook for button elements in IE6/7 (#1954)
		value: {
			get: function( elem, name ) {
				if ( nodeHook && jQuery.nodeName( elem, "button" ) ) {
					return nodeHook.get( elem, name );
				}
				return name in elem ?
					elem.value :
					null;
			},
			set: function( elem, value, name ) {
				if ( nodeHook && jQuery.nodeName( elem, "button" ) ) {
					return nodeHook.set( elem, value, name );
				}
				// Does not return so that setAttribute is also used
				elem.value = value;
			}
		}
	},

	propFix: {
		tabindex: "tabIndex",
		readonly: "readOnly",
		"for": "htmlFor",
		"class": "className",
		maxlength: "maxLength",
		cellspacing: "cellSpacing",
		cellpadding: "cellPadding",
		rowspan: "rowSpan",
		colspan: "colSpan",
		usemap: "useMap",
		frameborder: "frameBorder",
		contenteditable: "contentEditable"
	},

	prop: function( elem, name, value ) {
		var ret, hooks, notxml,
			nType = elem.nodeType;

		// don't get/set properties on text, comment and attribute nodes
		if ( !elem || nType === 3 || nType === 8 || nType === 2 ) {
			return;
		}

		notxml = nType !== 1 || !jQuery.isXMLDoc( elem );

		if ( notxml ) {
			// Fix name and attach hooks
			name = jQuery.propFix[ name ] || name;
			hooks = jQuery.propHooks[ name ];
		}

		if ( value !== undefined ) {
			if ( hooks && "set" in hooks && (ret = hooks.set( elem, value, name )) !== undefined ) {
				return ret;

			} else {
				return ( elem[ name ] = value );
			}

		} else {
			if ( hooks && "get" in hooks && (ret = hooks.get( elem, name )) !== null ) {
				return ret;

			} else {
				return elem[ name ];
			}
		}
	},

	propHooks: {
		tabIndex: {
			get: function( elem ) {
				// elem.tabIndex doesn't always return the correct value when it hasn't been explicitly set
				// http://fluidproject.org/blog/2008/01/09/getting-setting-and-removing-tabindex-values-with-javascript/
				var attributeNode = elem.getAttributeNode("tabindex");

				return attributeNode && attributeNode.specified ?
					parseInt( attributeNode.value, 10 ) :
					rfocusable.test( elem.nodeName ) || rclickable.test( elem.nodeName ) && elem.href ?
						0 :
						undefined;
			}
		}
	}
});

// Add the tabIndex propHook to attrHooks for back-compat (different case is intentional)
jQuery.attrHooks.tabindex = jQuery.propHooks.tabIndex;

// Hook for boolean attributes
boolHook = {
	get: function( elem, name ) {
		// Align boolean attributes with corresponding properties
		// Fall back to attribute presence where some booleans are not supported
		var attrNode,
			property = jQuery.prop( elem, name );
		return property === true || typeof property !== "boolean" && ( attrNode = elem.getAttributeNode(name) ) && attrNode.nodeValue !== false ?
			name.toLowerCase() :
			undefined;
	},
	set: function( elem, value, name ) {
		var propName;
		if ( value === false ) {
			// Remove boolean attributes when set to false
			jQuery.removeAttr( elem, name );
		} else {
			// value is true since we know at this point it's type boolean and not false
			// Set boolean attributes to the same name and set the DOM property
			propName = jQuery.propFix[ name ] || name;
			if ( propName in elem ) {
				// Only set the IDL specifically if it already exists on the element
				elem[ propName ] = true;
			}

			elem.setAttribute( name, name.toLowerCase() );
		}
		return name;
	}
};

// IE6/7 do not support getting/setting some attributes with get/setAttribute
if ( !getSetAttribute ) {

	fixSpecified = {
		name: true,
		id: true,
		coords: true
	};

	// Use this for any attribute in IE6/7
	// This fixes almost every IE6/7 issue
	nodeHook = jQuery.valHooks.button = {
		get: function( elem, name ) {
			var ret;
			ret = elem.getAttributeNode( name );
			return ret && ( fixSpecified[ name ] ? ret.nodeValue !== "" : ret.specified ) ?
				ret.nodeValue :
				undefined;
		},
		set: function( elem, value, name ) {
			// Set the existing or create a new attribute node
			var ret = elem.getAttributeNode( name );
			if ( !ret ) {
				ret = document.createAttribute( name );
				elem.setAttributeNode( ret );
			}
			return ( ret.nodeValue = value + "" );
		}
	};

	// Apply the nodeHook to tabindex
	jQuery.attrHooks.tabindex.set = nodeHook.set;

	// Set width and height to auto instead of 0 on empty string( Bug #8150 )
	// This is for removals
	jQuery.each([ "width", "height" ], function( i, name ) {
		jQuery.attrHooks[ name ] = jQuery.extend( jQuery.attrHooks[ name ], {
			set: function( elem, value ) {
				if ( value === "" ) {
					elem.setAttribute( name, "auto" );
					return value;
				}
			}
		});
	});

	// Set contenteditable to false on removals(#10429)
	// Setting to empty string throws an error as an invalid value
	jQuery.attrHooks.contenteditable = {
		get: nodeHook.get,
		set: function( elem, value, name ) {
			if ( value === "" ) {
				value = "false";
			}
			nodeHook.set( elem, value, name );
		}
	};
}


// Some attributes require a special call on IE
if ( !jQuery.support.hrefNormalized ) {
	jQuery.each([ "href", "src", "width", "height" ], function( i, name ) {
		jQuery.attrHooks[ name ] = jQuery.extend( jQuery.attrHooks[ name ], {
			get: function( elem ) {
				var ret = elem.getAttribute( name, 2 );
				return ret === null ? undefined : ret;
			}
		});
	});
}

if ( !jQuery.support.style ) {
	jQuery.attrHooks.style = {
		get: function( elem ) {
			// Return undefined in the case of empty string
			// Normalize to lowercase since IE uppercases css property names
			return elem.style.cssText.toLowerCase() || undefined;
		},
		set: function( elem, value ) {
			return ( elem.style.cssText = "" + value );
		}
	};
}

// Safari mis-reports the default selected property of an option
// Accessing the parent's selectedIndex property fixes it
if ( !jQuery.support.optSelected ) {
	jQuery.propHooks.selected = jQuery.extend( jQuery.propHooks.selected, {
		get: function( elem ) {
			var parent = elem.parentNode;

			if ( parent ) {
				parent.selectedIndex;

				// Make sure that it also works with optgroups, see #5701
				if ( parent.parentNode ) {
					parent.parentNode.selectedIndex;
				}
			}
			return null;
		}
	});
}

// IE6/7 call enctype encoding
if ( !jQuery.support.enctype ) {
	jQuery.propFix.enctype = "encoding";
}

// Radios and checkboxes getter/setter
if ( !jQuery.support.checkOn ) {
	jQuery.each([ "radio", "checkbox" ], function() {
		jQuery.valHooks[ this ] = {
			get: function( elem ) {
				// Handle the case where in Webkit "" is returned instead of "on" if a value isn't specified
				return elem.getAttribute("value") === null ? "on" : elem.value;
			}
		};
	});
}
jQuery.each([ "radio", "checkbox" ], function() {
	jQuery.valHooks[ this ] = jQuery.extend( jQuery.valHooks[ this ], {
		set: function( elem, value ) {
			if ( jQuery.isArray( value ) ) {
				return ( elem.checked = jQuery.inArray( jQuery(elem).val(), value ) >= 0 );
			}
		}
	});
});




var rformElems = /^(?:textarea|input|select)$/i,
	rtypenamespace = /^([^\.]*)?(?:\.(.+))?$/,
	rhoverHack = /(?:^|\s)hover(\.\S+)?\b/,
	rkeyEvent = /^key/,
	rmouseEvent = /^(?:mouse|contextmenu)|click/,
	rfocusMorph = /^(?:focusinfocus|focusoutblur)$/,
	rquickIs = /^(\w*)(?:#([\w\-]+))?(?:\.([\w\-]+))?$/,
	quickParse = function( selector ) {
		var quick = rquickIs.exec( selector );
		if ( quick ) {
			//   0  1    2   3
			// [ _, tag, id, class ]
			quick[1] = ( quick[1] || "" ).toLowerCase();
			quick[3] = quick[3] && new RegExp( "(?:^|\\s)" + quick[3] + "(?:\\s|$)" );
		}
		return quick;
	},
	quickIs = function( elem, m ) {
		var attrs = elem.attributes || {};
		return (
			(!m[1] || elem.nodeName.toLowerCase() === m[1]) &&
			(!m[2] || (attrs.id || {}).value === m[2]) &&
			(!m[3] || m[3].test( (attrs[ "class" ] || {}).value ))
		);
	},
	hoverHack = function( events ) {
		return jQuery.event.special.hover ? events : events.replace( rhoverHack, "mouseenter$1 mouseleave$1" );
	};

/*
 * Helper functions for managing events -- not part of the public interface.
 * Props to Dean Edwards' addEvent library for many of the ideas.
 */
jQuery.event = {

	add: function( elem, types, handler, data, selector ) {

		var elemData, eventHandle, events,
			t, tns, type, namespaces, handleObj,
			handleObjIn, quick, handlers, special;

		// Don't attach events to noData or text/comment nodes (allow plain objects tho)
		if ( elem.nodeType === 3 || elem.nodeType === 8 || !types || !handler || !(elemData = jQuery._data( elem )) ) {
			return;
		}

		// Caller can pass in an object of custom data in lieu of the handler
		if ( handler.handler ) {
			handleObjIn = handler;
			handler = handleObjIn.handler;
			selector = handleObjIn.selector;
		}

		// Make sure that the handler has a unique ID, used to find/remove it later
		if ( !handler.guid ) {
			handler.guid = jQuery.guid++;
		}

		// Init the element's event structure and main handler, if this is the first
		events = elemData.events;
		if ( !events ) {
			elemData.events = events = {};
		}
		eventHandle = elemData.handle;
		if ( !eventHandle ) {
			elemData.handle = eventHandle = function( e ) {
				// Discard the second event of a jQuery.event.trigger() and
				// when an event is called after a page has unloaded
				return typeof jQuery !== "undefined" && (!e || jQuery.event.triggered !== e.type) ?
					jQuery.event.dispatch.apply( eventHandle.elem, arguments ) :
					undefined;
			};
			// Add elem as a property of the handle fn to prevent a memory leak with IE non-native events
			eventHandle.elem = elem;
		}

		// Handle multiple events separated by a space
		// jQuery(...).bind("mouseover mouseout", fn);
		types = jQuery.trim( hoverHack(types) ).split( " " );
		for ( t = 0; t < types.length; t++ ) {

			tns = rtypenamespace.exec( types[t] ) || [];
			type = tns[1];
			namespaces = ( tns[2] || "" ).split( "." ).sort();

			// If event changes its type, use the special event handlers for the changed type
			special = jQuery.event.special[ type ] || {};

			// If selector defined, determine special event api type, otherwise given type
			type = ( selector ? special.delegateType : special.bindType ) || type;

			// Update special based on newly reset type
			special = jQuery.event.special[ type ] || {};

			// handleObj is passed to all event handlers
			handleObj = jQuery.extend({
				type: type,
				origType: tns[1],
				data: data,
				handler: handler,
				guid: handler.guid,
				selector: selector,
				quick: selector && quickParse( selector ),
				namespace: namespaces.join(".")
			}, handleObjIn );

			// Init the event handler queue if we're the first
			handlers = events[ type ];
			if ( !handlers ) {
				handlers = events[ type ] = [];
				handlers.delegateCount = 0;

				// Only use addEventListener/attachEvent if the special events handler returns false
				if ( !special.setup || special.setup.call( elem, data, namespaces, eventHandle ) === false ) {
					// Bind the global event handler to the element
					if ( elem.addEventListener ) {
						elem.addEventListener( type, eventHandle, false );

					} else if ( elem.attachEvent ) {
						elem.attachEvent( "on" + type, eventHandle );
					}
				}
			}

			if ( special.add ) {
				special.add.call( elem, handleObj );

				if ( !handleObj.handler.guid ) {
					handleObj.handler.guid = handler.guid;
				}
			}

			// Add to the element's handler list, delegates in front
			if ( selector ) {
				handlers.splice( handlers.delegateCount++, 0, handleObj );
			} else {
				handlers.push( handleObj );
			}

			// Keep track of which events have ever been used, for event optimization
			jQuery.event.global[ type ] = true;
		}

		// Nullify elem to prevent memory leaks in IE
		elem = null;
	},

	global: {},

	// Detach an event or set of events from an element
	remove: function( elem, types, handler, selector, mappedTypes ) {

		var elemData = jQuery.hasData( elem ) && jQuery._data( elem ),
			t, tns, type, origType, namespaces, origCount,
			j, events, special, handle, eventType, handleObj;

		if ( !elemData || !(events = elemData.events) ) {
			return;
		}

		// Once for each type.namespace in types; type may be omitted
		types = jQuery.trim( hoverHack( types || "" ) ).split(" ");
		for ( t = 0; t < types.length; t++ ) {
			tns = rtypenamespace.exec( types[t] ) || [];
			type = origType = tns[1];
			namespaces = tns[2];

			// Unbind all events (on this namespace, if provided) for the element
			if ( !type ) {
				for ( type in events ) {
					jQuery.event.remove( elem, type + types[ t ], handler, selector, true );
				}
				continue;
			}

			special = jQuery.event.special[ type ] || {};
			type = ( selector? special.delegateType : special.bindType ) || type;
			eventType = events[ type ] || [];
			origCount = eventType.length;
			namespaces = namespaces ? new RegExp("(^|\\.)" + namespaces.split(".").sort().join("\\.(?:.*\\.)?") + "(\\.|$)") : null;

			// Remove matching events
			for ( j = 0; j < eventType.length; j++ ) {
				handleObj = eventType[ j ];

				if ( ( mappedTypes || origType === handleObj.origType ) &&
					 ( !handler || handler.guid === handleObj.guid ) &&
					 ( !namespaces || namespaces.test( handleObj.namespace ) ) &&
					 ( !selector || selector === handleObj.selector || selector === "**" && handleObj.selector ) ) {
					eventType.splice( j--, 1 );

					if ( handleObj.selector ) {
						eventType.delegateCount--;
					}
					if ( special.remove ) {
						special.remove.call( elem, handleObj );
					}
				}
			}

			// Remove generic event handler if we removed something and no more handlers exist
			// (avoids potential for endless recursion during removal of special event handlers)
			if ( eventType.length === 0 && origCount !== eventType.length ) {
				if ( !special.teardown || special.teardown.call( elem, namespaces ) === false ) {
					jQuery.removeEvent( elem, type, elemData.handle );
				}

				delete events[ type ];
			}
		}

		// Remove the expando if it's no longer used
		if ( jQuery.isEmptyObject( events ) ) {
			handle = elemData.handle;
			if ( handle ) {
				handle.elem = null;
			}

			// removeData also checks for emptiness and clears the expando if empty
			// so use it instead of delete
			jQuery.removeData( elem, [ "events", "handle" ], true );
		}
	},

	// Events that are safe to short-circuit if no handlers are attached.
	// Native DOM events should not be added, they may have inline handlers.
	customEvent: {
		"getData": true,
		"setData": true,
		"changeData": true
	},

	trigger: function( event, data, elem, onlyHandlers ) {
		// Don't do events on text and comment nodes
		if ( elem && (elem.nodeType === 3 || elem.nodeType === 8) ) {
			return;
		}

		// Event object or event type
		var type = event.type || event,
			namespaces = [],
			cache, exclusive, i, cur, old, ontype, special, handle, eventPath, bubbleType;

		// focus/blur morphs to focusin/out; ensure we're not firing them right now
		if ( rfocusMorph.test( type + jQuery.event.triggered ) ) {
			return;
		}

		if ( type.indexOf( "!" ) >= 0 ) {
			// Exclusive events trigger only for the exact event (no namespaces)
			type = type.slice(0, -1);
			exclusive = true;
		}

		if ( type.indexOf( "." ) >= 0 ) {
			// Namespaced trigger; create a regexp to match event type in handle()
			namespaces = type.split(".");
			type = namespaces.shift();
			namespaces.sort();
		}

		if ( (!elem || jQuery.event.customEvent[ type ]) && !jQuery.event.global[ type ] ) {
			// No jQuery handlers for this event type, and it can't have inline handlers
			return;
		}

		// Caller can pass in an Event, Object, or just an event type string
		event = typeof event === "object" ?
			// jQuery.Event object
			event[ jQuery.expando ] ? event :
			// Object literal
			new jQuery.Event( type, event ) :
			// Just the event type (string)
			new jQuery.Event( type );

		event.type = type;
		event.isTrigger = true;
		event.exclusive = exclusive;
		event.namespace = namespaces.join( "." );
		event.namespace_re = event.namespace? new RegExp("(^|\\.)" + namespaces.join("\\.(?:.*\\.)?") + "(\\.|$)") : null;
		ontype = type.indexOf( ":" ) < 0 ? "on" + type : "";

		// Handle a global trigger
		if ( !elem ) {

			// TODO: Stop taunting the data cache; remove global events and always attach to document
			cache = jQuery.cache;
			for ( i in cache ) {
				if ( cache[ i ].events && cache[ i ].events[ type ] ) {
					jQuery.event.trigger( event, data, cache[ i ].handle.elem, true );
				}
			}
			return;
		}

		// Clean up the event in case it is being reused
		event.result = undefined;
		if ( !event.target ) {
			event.target = elem;
		}

		// Clone any incoming data and prepend the event, creating the handler arg list
		data = data != null ? jQuery.makeArray( data ) : [];
		data.unshift( event );

		// Allow special events to draw outside the lines
		special = jQuery.event.special[ type ] || {};
		if ( special.trigger && special.trigger.apply( elem, data ) === false ) {
			return;
		}

		// Determine event propagation path in advance, per W3C events spec (#9951)
		// Bubble up to document, then to window; watch for a global ownerDocument var (#9724)
		eventPath = [[ elem, special.bindType || type ]];
		if ( !onlyHandlers && !special.noBubble && !jQuery.isWindow( elem ) ) {

			bubbleType = special.delegateType || type;
			cur = rfocusMorph.test( bubbleType + type ) ? elem : elem.parentNode;
			old = null;
			for ( ; cur; cur = cur.parentNode ) {
				eventPath.push([ cur, bubbleType ]);
				old = cur;
			}

			// Only add window if we got to document (e.g., not plain obj or detached DOM)
			if ( old && old === elem.ownerDocument ) {
				eventPath.push([ old.defaultView || old.parentWindow || window, bubbleType ]);
			}
		}

		// Fire handlers on the event path
		for ( i = 0; i < eventPath.length && !event.isPropagationStopped(); i++ ) {

			cur = eventPath[i][0];
			event.type = eventPath[i][1];

			handle = ( jQuery._data( cur, "events" ) || {} )[ event.type ] && jQuery._data( cur, "handle" );
			if ( handle ) {
				handle.apply( cur, data );
			}
			// Note that this is a bare JS function and not a jQuery handler
			handle = ontype && cur[ ontype ];
			if ( handle && jQuery.acceptData( cur ) && handle.apply( cur, data ) === false ) {
				event.preventDefault();
			}
		}
		event.type = type;

		// If nobody prevented the default action, do it now
		if ( !onlyHandlers && !event.isDefaultPrevented() ) {

			if ( (!special._default || special._default.apply( elem.ownerDocument, data ) === false) &&
				!(type === "click" && jQuery.nodeName( elem, "a" )) && jQuery.acceptData( elem ) ) {

				// Call a native DOM method on the target with the same name name as the event.
				// Can't use an .isFunction() check here because IE6/7 fails that test.
				// Don't do default actions on window, that's where global variables be (#6170)
				// IE<9 dies on focus/blur to hidden element (#1486)
				if ( ontype && elem[ type ] && ((type !== "focus" && type !== "blur") || event.target.offsetWidth !== 0) && !jQuery.isWindow( elem ) ) {

					// Don't re-trigger an onFOO event when we call its FOO() method
					old = elem[ ontype ];

					if ( old ) {
						elem[ ontype ] = null;
					}

					// Prevent re-triggering of the same event, since we already bubbled it above
					jQuery.event.triggered = type;
					elem[ type ]();
					jQuery.event.triggered = undefined;

					if ( old ) {
						elem[ ontype ] = old;
					}
				}
			}
		}

		return event.result;
	},

	dispatch: function( event ) {

		// Make a writable jQuery.Event from the native event object
		event = jQuery.event.fix( event || window.event );


		var handlers = ( (jQuery._data( this, "events" ) || {} )[ event.type ] || []),
			delegateCount = handlers.delegateCount,
			args = [].slice.call( arguments, 0 ),
			run_all = !event.exclusive && !event.namespace,
			special = jQuery.event.special[ event.type ] || {},
			handlerQueue = [],
			i, j, cur, jqcur, ret, selMatch, matched, matches, handleObj, sel, related;


		// Use the fix-ed jQuery.Event rather than the (read-only) native event
		args[0] = event;
		event.delegateTarget = this;

		// Call the preDispatch hook for the mapped type, and let it bail if desired
		if ( special.preDispatch && special.preDispatch.call( this, event ) === false ) {
			return;
		}

		// Determine handlers that should run if there are delegated events
		// Avoid non-left-click bubbling in Firefox (#3861)
		if ( delegateCount && !(event.button && event.type === "click") ) {

			// Pregenerate a single jQuery object for reuse with .is()
			jqcur = jQuery(this);
			jqcur.context = this.ownerDocument || this;

			for ( cur = event.target; cur != this; cur = cur.parentNode || this ) {

				// Don't process events on disabled elements (#6911, #8165)
				if ( cur.disabled !== true ) {
					selMatch = {};
					matches = [];
					jqcur[0] = cur;
					for ( i = 0; i < delegateCount; i++ ) {
						handleObj = handlers[ i ];
						sel = handleObj.selector;

						if ( selMatch[ sel ] === undefined ) {
							selMatch[ sel ] = (
								handleObj.quick ? quickIs( cur, handleObj.quick ) : jqcur.is( sel )
							);
						}
						if ( selMatch[ sel ] ) {
							matches.push( handleObj );
						}
					}
					if ( matches.length ) {
						handlerQueue.push({ elem: cur, matches: matches });
					}
				}
			}
		}

		// Add the remaining (directly-bound) handlers
		if ( handlers.length > delegateCount ) {
			handlerQueue.push({ elem: this, matches: handlers.slice( delegateCount ) });
		}

		// Run delegates first; they may want to stop propagation beneath us
		for ( i = 0; i < handlerQueue.length && !event.isPropagationStopped(); i++ ) {
			matched = handlerQueue[ i ];
			event.currentTarget = matched.elem;

			for ( j = 0; j < matched.matches.length && !event.isImmediatePropagationStopped(); j++ ) {
				handleObj = matched.matches[ j ];

				// Triggered event must either 1) be non-exclusive and have no namespace, or
				// 2) have namespace(s) a subset or equal to those in the bound event (both can have no namespace).
				if ( run_all || (!event.namespace && !handleObj.namespace) || event.namespace_re && event.namespace_re.test( handleObj.namespace ) ) {

					event.data = handleObj.data;
					event.handleObj = handleObj;

					ret = ( (jQuery.event.special[ handleObj.origType ] || {}).handle || handleObj.handler )
							.apply( matched.elem, args );

					if ( ret !== undefined ) {
						event.result = ret;
						if ( ret === false ) {
							event.preventDefault();
							event.stopPropagation();
						}
					}
				}
			}
		}

		// Call the postDispatch hook for the mapped type
		if ( special.postDispatch ) {
			special.postDispatch.call( this, event );
		}

		return event.result;
	},

	// Includes some event props shared by KeyEvent and MouseEvent
	// *** attrChange attrName relatedNode srcElement  are not normalized, non-W3C, deprecated, will be removed in 1.8 ***
	props: "attrChange attrName relatedNode srcElement altKey bubbles cancelable ctrlKey currentTarget eventPhase metaKey relatedTarget shiftKey target timeStamp view which".split(" "),

	fixHooks: {},

	keyHooks: {
		props: "char charCode key keyCode".split(" "),
		filter: function( event, original ) {

			// Add which for key events
			if ( event.which == null ) {
				event.which = original.charCode != null ? original.charCode : original.keyCode;
			}

			return event;
		}
	},

	mouseHooks: {
		props: "button buttons clientX clientY fromElement offsetX offsetY pageX pageY screenX screenY toElement".split(" "),
		filter: function( event, original ) {
			var eventDoc, doc, body,
				button = original.button,
				fromElement = original.fromElement;

			// Calculate pageX/Y if missing and clientX/Y available
			if ( event.pageX == null && original.clientX != null ) {
				eventDoc = event.target.ownerDocument || document;
				doc = eventDoc.documentElement;
				body = eventDoc.body;

				event.pageX = original.clientX + ( doc && doc.scrollLeft || body && body.scrollLeft || 0 ) - ( doc && doc.clientLeft || body && body.clientLeft || 0 );
				event.pageY = original.clientY + ( doc && doc.scrollTop  || body && body.scrollTop  || 0 ) - ( doc && doc.clientTop  || body && body.clientTop  || 0 );
			}

			// Add relatedTarget, if necessary
			if ( !event.relatedTarget && fromElement ) {
				event.relatedTarget = fromElement === event.target ? original.toElement : fromElement;
			}

			// Add which for click: 1 === left; 2 === middle; 3 === right
			// Note: button is not normalized, so don't use it
			if ( !event.which && button !== undefined ) {
				event.which = ( button & 1 ? 1 : ( button & 2 ? 3 : ( button & 4 ? 2 : 0 ) ) );
			}

			return event;
		}
	},

	fix: function( event ) {
		if ( event[ jQuery.expando ] ) {
			return event;
		}

		// Create a writable copy of the event object and normalize some properties
		var i, prop,
			originalEvent = event,
			fixHook = jQuery.event.fixHooks[ event.type ] || {},
			copy = fixHook.props ? this.props.concat( fixHook.props ) : this.props;

		event = jQuery.Event( originalEvent );

		for ( i = copy.length; i; ) {
			prop = copy[ --i ];
			event[ prop ] = originalEvent[ prop ];
		}

		// Fix target property, if necessary (#1925, IE 6/7/8 & Safari2)
		if ( !event.target ) {
			event.target = originalEvent.srcElement || document;
		}

		// Target should not be a text node (#504, Safari)
		if ( event.target.nodeType === 3 ) {
			event.target = event.target.parentNode;
		}

		// For mouse/key events; add metaKey if it's not there (#3368, IE6/7/8)
		if ( event.metaKey === undefined ) {
			event.metaKey = event.ctrlKey;
		}

		return fixHook.filter? fixHook.filter( event, originalEvent ) : event;
	},

	special: {
		ready: {
			// Make sure the ready event is setup
			setup: jQuery.bindReady
		},

		load: {
			// Prevent triggered image.load events from bubbling to window.load
			noBubble: true
		},

		focus: {
			delegateType: "focusin"
		},
		blur: {
			delegateType: "focusout"
		},

		beforeunload: {
			setup: function( data, namespaces, eventHandle ) {
				// We only want to do this special case on windows
				if ( jQuery.isWindow( this ) ) {
					this.onbeforeunload = eventHandle;
				}
			},

			teardown: function( namespaces, eventHandle ) {
				if ( this.onbeforeunload === eventHandle ) {
					this.onbeforeunload = null;
				}
			}
		}
	},

	simulate: function( type, elem, event, bubble ) {
		// Piggyback on a donor event to simulate a different one.
		// Fake originalEvent to avoid donor's stopPropagation, but if the
		// simulated event prevents default then we do the same on the donor.
		var e = jQuery.extend(
			new jQuery.Event(),
			event,
			{ type: type,
				isSimulated: true,
				originalEvent: {}
			}
		);
		if ( bubble ) {
			jQuery.event.trigger( e, null, elem );
		} else {
			jQuery.event.dispatch.call( elem, e );
		}
		if ( e.isDefaultPrevented() ) {
			event.preventDefault();
		}
	}
};

// Some plugins are using, but it's undocumented/deprecated and will be removed.
// The 1.7 special event interface should provide all the hooks needed now.
jQuery.event.handle = jQuery.event.dispatch;

jQuery.removeEvent = document.removeEventListener ?
	function( elem, type, handle ) {
		if ( elem.removeEventListener ) {
			elem.removeEventListener( type, handle, false );
		}
	} :
	function( elem, type, handle ) {
		if ( elem.detachEvent ) {
			elem.detachEvent( "on" + type, handle );
		}
	};

jQuery.Event = function( src, props ) {
	// Allow instantiation without the 'new' keyword
	if ( !(this instanceof jQuery.Event) ) {
		return new jQuery.Event( src, props );
	}

	// Event object
	if ( src && src.type ) {
		this.originalEvent = src;
		this.type = src.type;

		// Events bubbling up the document may have been marked as prevented
		// by a handler lower down the tree; reflect the correct value.
		this.isDefaultPrevented = ( src.defaultPrevented || src.returnValue === false ||
			src.getPreventDefault && src.getPreventDefault() ) ? returnTrue : returnFalse;

	// Event type
	} else {
		this.type = src;
	}

	// Put explicitly provided properties onto the event object
	if ( props ) {
		jQuery.extend( this, props );
	}

	// Create a timestamp if incoming event doesn't have one
	this.timeStamp = src && src.timeStamp || jQuery.now();

	// Mark it as fixed
	this[ jQuery.expando ] = true;
};

function returnFalse() {
	return false;
}
function returnTrue() {
	return true;
}

// jQuery.Event is based on DOM3 Events as specified by the ECMAScript Language Binding
// http://www.w3.org/TR/2003/WD-DOM-Level-3-Events-20030331/ecma-script-binding.html
jQuery.Event.prototype = {
	preventDefault: function() {
		this.isDefaultPrevented = returnTrue;

		var e = this.originalEvent;
		if ( !e ) {
			return;
		}

		// if preventDefault exists run it on the original event
		if ( e.preventDefault ) {
			e.preventDefault();

		// otherwise set the returnValue property of the original event to false (IE)
		} else {
			e.returnValue = false;
		}
	},
	stopPropagation: function() {
		this.isPropagationStopped = returnTrue;

		var e = this.originalEvent;
		if ( !e ) {
			return;
		}
		// if stopPropagation exists run it on the original event
		if ( e.stopPropagation ) {
			e.stopPropagation();
		}
		// otherwise set the cancelBubble property of the original event to true (IE)
		e.cancelBubble = true;
	},
	stopImmediatePropagation: function() {
		this.isImmediatePropagationStopped = returnTrue;
		this.stopPropagation();
	},
	isDefaultPrevented: returnFalse,
	isPropagationStopped: returnFalse,
	isImmediatePropagationStopped: returnFalse
};

// Create mouseenter/leave events using mouseover/out and event-time checks
jQuery.each({
	mouseenter: "mouseover",
	mouseleave: "mouseout"
}, function( orig, fix ) {
	jQuery.event.special[ orig ] = {
		delegateType: fix,
		bindType: fix,

		handle: function( event ) {
			var target = this,
				related = event.relatedTarget,
				handleObj = event.handleObj,
				selector = handleObj.selector,
				ret;

			// For mousenter/leave call the handler if related is outside the target.
			// NB: No relatedTarget if the mouse left/entered the browser window
			if ( !related || (related !== target && !jQuery.contains( target, related )) ) {
				event.type = handleObj.origType;
				ret = handleObj.handler.apply( this, arguments );
				event.type = fix;
			}
			return ret;
		}
	};
});

// IE submit delegation
if ( !jQuery.support.submitBubbles ) {

	jQuery.event.special.submit = {
		setup: function() {
			// Only need this for delegated form submit events
			if ( jQuery.nodeName( this, "form" ) ) {
				return false;
			}

			// Lazy-add a submit handler when a descendant form may potentially be submitted
			jQuery.event.add( this, "click._submit keypress._submit", function( e ) {
				// Node name check avoids a VML-related crash in IE (#9807)
				var elem = e.target,
					form = jQuery.nodeName( elem, "input" ) || jQuery.nodeName( elem, "button" ) ? elem.form : undefined;
				if ( form && !form._submit_attached ) {
					jQuery.event.add( form, "submit._submit", function( event ) {
						event._submit_bubble = true;
					});
					form._submit_attached = true;
				}
			});
			// return undefined since we don't need an event listener
		},
		
		postDispatch: function( event ) {
			// If form was submitted by the user, bubble the event up the tree
			if ( event._submit_bubble ) {
				delete event._submit_bubble;
				if ( this.parentNode && !event.isTrigger ) {
					jQuery.event.simulate( "submit", this.parentNode, event, true );
				}
			}
		},

		teardown: function() {
			// Only need this for delegated form submit events
			if ( jQuery.nodeName( this, "form" ) ) {
				return false;
			}

			// Remove delegated handlers; cleanData eventually reaps submit handlers attached above
			jQuery.event.remove( this, "._submit" );
		}
	};
}

// IE change delegation and checkbox/radio fix
if ( !jQuery.support.changeBubbles ) {

	jQuery.event.special.change = {

		setup: function() {

			if ( rformElems.test( this.nodeName ) ) {
				// IE doesn't fire change on a check/radio until blur; trigger it on click
				// after a propertychange. Eat the blur-change in special.change.handle.
				// This still fires onchange a second time for check/radio after blur.
				if ( this.type === "checkbox" || this.type === "radio" ) {
					jQuery.event.add( this, "propertychange._change", function( event ) {
						if ( event.originalEvent.propertyName === "checked" ) {
							this._just_changed = true;
						}
					});
					jQuery.event.add( this, "click._change", function( event ) {
						if ( this._just_changed && !event.isTrigger ) {
							this._just_changed = false;
							jQuery.event.simulate( "change", this, event, true );
						}
					});
				}
				return false;
			}
			// Delegated event; lazy-add a change handler on descendant inputs
			jQuery.event.add( this, "beforeactivate._change", function( e ) {
				var elem = e.target;

				if ( rformElems.test( elem.nodeName ) && !elem._change_attached ) {
					jQuery.event.add( elem, "change._change", function( event ) {
						if ( this.parentNode && !event.isSimulated && !event.isTrigger ) {
							jQuery.event.simulate( "change", this.parentNode, event, true );
						}
					});
					elem._change_attached = true;
				}
			});
		},

		handle: function( event ) {
			var elem = event.target;

			// Swallow native change events from checkbox/radio, we already triggered them above
			if ( this !== elem || event.isSimulated || event.isTrigger || (elem.type !== "radio" && elem.type !== "checkbox") ) {
				return event.handleObj.handler.apply( this, arguments );
			}
		},

		teardown: function() {
			jQuery.event.remove( this, "._change" );

			return rformElems.test( this.nodeName );
		}
	};
}

// Create "bubbling" focus and blur events
if ( !jQuery.support.focusinBubbles ) {
	jQuery.each({ focus: "focusin", blur: "focusout" }, function( orig, fix ) {

		// Attach a single capturing handler while someone wants focusin/focusout
		var attaches = 0,
			handler = function( event ) {
				jQuery.event.simulate( fix, event.target, jQuery.event.fix( event ), true );
			};

		jQuery.event.special[ fix ] = {
			setup: function() {
				if ( attaches++ === 0 ) {
					document.addEventListener( orig, handler, true );
				}
			},
			teardown: function() {
				if ( --attaches === 0 ) {
					document.removeEventListener( orig, handler, true );
				}
			}
		};
	});
}

jQuery.fn.extend({

	on: function( types, selector, data, fn, /*INTERNAL*/ one ) {
		var origFn, type;

		// Types can be a map of types/handlers
		if ( typeof types === "object" ) {
			// ( types-Object, selector, data )
			if ( typeof selector !== "string" ) { // && selector != null
				// ( types-Object, data )
				data = data || selector;
				selector = undefined;
			}
			for ( type in types ) {
				this.on( type, selector, data, types[ type ], one );
			}
			return this;
		}

		if ( data == null && fn == null ) {
			// ( types, fn )
			fn = selector;
			data = selector = undefined;
		} else if ( fn == null ) {
			if ( typeof selector === "string" ) {
				// ( types, selector, fn )
				fn = data;
				data = undefined;
			} else {
				// ( types, data, fn )
				fn = data;
				data = selector;
				selector = undefined;
			}
		}
		if ( fn === false ) {
			fn = returnFalse;
		} else if ( !fn ) {
			return this;
		}

		if ( one === 1 ) {
			origFn = fn;
			fn = function( event ) {
				// Can use an empty set, since event contains the info
				jQuery().off( event );
				return origFn.apply( this, arguments );
			};
			// Use same guid so caller can remove using origFn
			fn.guid = origFn.guid || ( origFn.guid = jQuery.guid++ );
		}
		return this.each( function() {
			jQuery.event.add( this, types, fn, data, selector );
		});
	},
	one: function( types, selector, data, fn ) {
		return this.on( types, selector, data, fn, 1 );
	},
	off: function( types, selector, fn ) {
		if ( types && types.preventDefault && types.handleObj ) {
			// ( event )  dispatched jQuery.Event
			var handleObj = types.handleObj;
			jQuery( types.delegateTarget ).off(
				handleObj.namespace ? handleObj.origType + "." + handleObj.namespace : handleObj.origType,
				handleObj.selector,
				handleObj.handler
			);
			return this;
		}
		if ( typeof types === "object" ) {
			// ( types-object [, selector] )
			for ( var type in types ) {
				this.off( type, selector, types[ type ] );
			}
			return this;
		}
		if ( selector === false || typeof selector === "function" ) {
			// ( types [, fn] )
			fn = selector;
			selector = undefined;
		}
		if ( fn === false ) {
			fn = returnFalse;
		}
		return this.each(function() {
			jQuery.event.remove( this, types, fn, selector );
		});
	},

	bind: function( types, data, fn ) {
		return this.on( types, null, data, fn );
	},
	unbind: function( types, fn ) {
		return this.off( types, null, fn );
	},

	live: function( types, data, fn ) {
		jQuery( this.context ).on( types, this.selector, data, fn );
		return this;
	},
	die: function( types, fn ) {
		jQuery( this.context ).off( types, this.selector || "**", fn );
		return this;
	},

	delegate: function( selector, types, data, fn ) {
		return this.on( types, selector, data, fn );
	},
	undelegate: function( selector, types, fn ) {
		// ( namespace ) or ( selector, types [, fn] )
		return arguments.length == 1? this.off( selector, "**" ) : this.off( types, selector, fn );
	},

	trigger: function( type, data ) {
		return this.each(function() {
			jQuery.event.trigger( type, data, this );
		});
	},
	triggerHandler: function( type, data ) {
		if ( this[0] ) {
			return jQuery.event.trigger( type, data, this[0], true );
		}
	},

	toggle: function( fn ) {
		// Save reference to arguments for access in closure
		var args = arguments,
			guid = fn.guid || jQuery.guid++,
			i = 0,
			toggler = function( event ) {
				// Figure out which function to execute
				var lastToggle = ( jQuery._data( this, "lastToggle" + fn.guid ) || 0 ) % i;
				jQuery._data( this, "lastToggle" + fn.guid, lastToggle + 1 );

				// Make sure that clicks stop
				event.preventDefault();

				// and execute the function
				return args[ lastToggle ].apply( this, arguments ) || false;
			};

		// link all the functions, so any of them can unbind this click handler
		toggler.guid = guid;
		while ( i < args.length ) {
			args[ i++ ].guid = guid;
		}

		return this.click( toggler );
	},

	hover: function( fnOver, fnOut ) {
		return this.mouseenter( fnOver ).mouseleave( fnOut || fnOver );
	}
});

jQuery.each( ("blur focus focusin focusout load resize scroll unload click dblclick " +
	"mousedown mouseup mousemove mouseover mouseout mouseenter mouseleave " +
	"change select submit keydown keypress keyup error contextmenu").split(" "), function( i, name ) {

	// Handle event binding
	jQuery.fn[ name ] = function( data, fn ) {
		if ( fn == null ) {
			fn = data;
			data = null;
		}

		return arguments.length > 0 ?
			this.on( name, null, data, fn ) :
			this.trigger( name );
	};

	if ( jQuery.attrFn ) {
		jQuery.attrFn[ name ] = true;
	}

	if ( rkeyEvent.test( name ) ) {
		jQuery.event.fixHooks[ name ] = jQuery.event.keyHooks;
	}

	if ( rmouseEvent.test( name ) ) {
		jQuery.event.fixHooks[ name ] = jQuery.event.mouseHooks;
	}
});



/*!
 * Sizzle CSS Selector Engine
 *  Copyright 2011, The Dojo Foundation
 *  Released under the MIT, BSD, and GPL Licenses.
 *  More information: http://sizzlejs.com/
 */
(function(){

var chunker = /((?:\((?:\([^()]+\)|[^()]+)+\)|\[(?:\[[^\[\]]*\]|['"][^'"]*['"]|[^\[\]'"]+)+\]|\\.|[^ >+~,(\[\\]+)+|[>+~])(\s*,\s*)?((?:.|\r|\n)*)/g,
	expando = "sizcache" + (Math.random() + '').replace('.', ''),
	done = 0,
	toString = Object.prototype.toString,
	hasDuplicate = false,
	baseHasDuplicate = true,
	rBackslash = /\\/g,
	rReturn = /\r\n/g,
	rNonWord = /\W/;

// Here we check if the JavaScript engine is using some sort of
// optimization where it does not always call our comparision
// function. If that is the case, discard the hasDuplicate value.
//   Thus far that includes Google Chrome.
[0, 0].sort(function() {
	baseHasDuplicate = false;
	return 0;
});

var Sizzle = function( selector, context, results, seed ) {
	results = results || [];
	context = context || document;

	var origContext = context;

	if ( context.nodeType !== 1 && context.nodeType !== 9 ) {
		return [];
	}

	if ( !selector || typeof selector !== "string" ) {
		return results;
	}

	var m, set, checkSet, extra, ret, cur, pop, i,
		prune = true,
		contextXML = Sizzle.isXML( context ),
		parts = [],
		soFar = selector;

	// Reset the position of the chunker regexp (start from head)
	do {
		chunker.exec( "" );
		m = chunker.exec( soFar );

		if ( m ) {
			soFar = m[3];

			parts.push( m[1] );

			if ( m[2] ) {
				extra = m[3];
				break;
			}
		}
	} while ( m );

	if ( parts.length > 1 && origPOS.exec( selector ) ) {

		if ( parts.length === 2 && Expr.relative[ parts[0] ] ) {
			set = posProcess( parts[0] + parts[1], context, seed );

		} else {
			set = Expr.relative[ parts[0] ] ?
				[ context ] :
				Sizzle( parts.shift(), context );

			while ( parts.length ) {
				selector = parts.shift();

				if ( Expr.relative[ selector ] ) {
					selector += parts.shift();
				}

				set = posProcess( selector, set, seed );
			}
		}

	} else {
		// Take a shortcut and set the context if the root selector is an ID
		// (but not if it'll be faster if the inner selector is an ID)
		if ( !seed && parts.length > 1 && context.nodeType === 9 && !contextXML &&
				Expr.match.ID.test(parts[0]) && !Expr.match.ID.test(parts[parts.length - 1]) ) {

			ret = Sizzle.find( parts.shift(), context, contextXML );
			context = ret.expr ?
				Sizzle.filter( ret.expr, ret.set )[0] :
				ret.set[0];
		}

		if ( context ) {
			ret = seed ?
				{ expr: parts.pop(), set: makeArray(seed) } :
				Sizzle.find( parts.pop(), parts.length === 1 && (parts[0] === "~" || parts[0] === "+") && context.parentNode ? context.parentNode : context, contextXML );

			set = ret.expr ?
				Sizzle.filter( ret.expr, ret.set ) :
				ret.set;

			if ( parts.length > 0 ) {
				checkSet = makeArray( set );

			} else {
				prune = false;
			}

			while ( parts.length ) {
				cur = parts.pop();
				pop = cur;

				if ( !Expr.relative[ cur ] ) {
					cur = "";
				} else {
					pop = parts.pop();
				}

				if ( pop == null ) {
					pop = context;
				}

				Expr.relative[ cur ]( checkSet, pop, contextXML );
			}

		} else {
			checkSet = parts = [];
		}
	}

	if ( !checkSet ) {
		checkSet = set;
	}

	if ( !checkSet ) {
		Sizzle.error( cur || selector );
	}

	if ( toString.call(checkSet) === "[object Array]" ) {
		if ( !prune ) {
			results.push.apply( results, checkSet );

		} else if ( context && context.nodeType === 1 ) {
			for ( i = 0; checkSet[i] != null; i++ ) {
				if ( checkSet[i] && (checkSet[i] === true || checkSet[i].nodeType === 1 && Sizzle.contains(context, checkSet[i])) ) {
					results.push( set[i] );
				}
			}

		} else {
			for ( i = 0; checkSet[i] != null; i++ ) {
				if ( checkSet[i] && checkSet[i].nodeType === 1 ) {
					results.push( set[i] );
				}
			}
		}

	} else {
		makeArray( checkSet, results );
	}

	if ( extra ) {
		Sizzle( extra, origContext, results, seed );
		Sizzle.uniqueSort( results );
	}

	return results;
};

Sizzle.uniqueSort = function( results ) {
	if ( sortOrder ) {
		hasDuplicate = baseHasDuplicate;
		results.sort( sortOrder );

		if ( hasDuplicate ) {
			for ( var i = 1; i < results.length; i++ ) {
				if ( results[i] === results[ i - 1 ] ) {
					results.splice( i--, 1 );
				}
			}
		}
	}

	return results;
};

Sizzle.matches = function( expr, set ) {
	return Sizzle( expr, null, null, set );
};

Sizzle.matchesSelector = function( node, expr ) {
	return Sizzle( expr, null, null, [node] ).length > 0;
};

Sizzle.find = function( expr, context, isXML ) {
	var set, i, len, match, type, left;

	if ( !expr ) {
		return [];
	}

	for ( i = 0, len = Expr.order.length; i < len; i++ ) {
		type = Expr.order[i];

		if ( (match = Expr.leftMatch[ type ].exec( expr )) ) {
			left = match[1];
			match.splice( 1, 1 );

			if ( left.substr( left.length - 1 ) !== "\\" ) {
				match[1] = (match[1] || "").replace( rBackslash, "" );
				set = Expr.find[ type ]( match, context, isXML );

				if ( set != null ) {
					expr = expr.replace( Expr.match[ type ], "" );
					break;
				}
			}
		}
	}

	if ( !set ) {
		set = typeof context.getElementsByTagName !== "undefined" ?
			context.getElementsByTagName( "*" ) :
			[];
	}

	return { set: set, expr: expr };
};

Sizzle.filter = function( expr, set, inplace, not ) {
	var match, anyFound,
		type, found, item, filter, left,
		i, pass,
		old = expr,
		result = [],
		curLoop = set,
		isXMLFilter = set && set[0] && Sizzle.isXML( set[0] );

	while ( expr && set.length ) {
		for ( type in Expr.filter ) {
			if ( (match = Expr.leftMatch[ type ].exec( expr )) != null && match[2] ) {
				filter = Expr.filter[ type ];
				left = match[1];

				anyFound = false;

				match.splice(1,1);

				if ( left.substr( left.length - 1 ) === "\\" ) {
					continue;
				}

				if ( curLoop === result ) {
					result = [];
				}

				if ( Expr.preFilter[ type ] ) {
					match = Expr.preFilter[ type ]( match, curLoop, inplace, result, not, isXMLFilter );

					if ( !match ) {
						anyFound = found = true;

					} else if ( match === true ) {
						continue;
					}
				}

				if ( match ) {
					for ( i = 0; (item = curLoop[i]) != null; i++ ) {
						if ( item ) {
							found = filter( item, match, i, curLoop );
							pass = not ^ found;

							if ( inplace && found != null ) {
								if ( pass ) {
									anyFound = true;

								} else {
									curLoop[i] = false;
								}

							} else if ( pass ) {
								result.push( item );
								anyFound = true;
							}
						}
					}
				}

				if ( found !== undefined ) {
					if ( !inplace ) {
						curLoop = result;
					}

					expr = expr.replace( Expr.match[ type ], "" );

					if ( !anyFound ) {
						return [];
					}

					break;
				}
			}
		}

		// Improper expression
		if ( expr === old ) {
			if ( anyFound == null ) {
				Sizzle.error( expr );

			} else {
				break;
			}
		}

		old = expr;
	}

	return curLoop;
};

Sizzle.error = function( msg ) {
	throw new Error( "Syntax error, unrecognized expression: " + msg );
};

/**
 * Utility function for retreiving the text value of an array of DOM nodes
 * @param {Array|Element} elem
 */
var getText = Sizzle.getText = function( elem ) {
    var i, node,
		nodeType = elem.nodeType,
		ret = "";

	if ( nodeType ) {
		if ( nodeType === 1 || nodeType === 9 || nodeType === 11 ) {
			// Use textContent || innerText for elements
			if ( typeof elem.textContent === 'string' ) {
				return elem.textContent;
			} else if ( typeof elem.innerText === 'string' ) {
				// Replace IE's carriage returns
				return elem.innerText.replace( rReturn, '' );
			} else {
				// Traverse it's children
				for ( elem = elem.firstChild; elem; elem = elem.nextSibling) {
					ret += getText( elem );
				}
			}
		} else if ( nodeType === 3 || nodeType === 4 ) {
			return elem.nodeValue;
		}
	} else {

		// If no nodeType, this is expected to be an array
		for ( i = 0; (node = elem[i]); i++ ) {
			// Do not traverse comment nodes
			if ( node.nodeType !== 8 ) {
				ret += getText( node );
			}
		}
	}
	return ret;
};

var Expr = Sizzle.selectors = {
	order: [ "ID", "NAME", "TAG" ],

	match: {
		ID: /#((?:[\w\u00c0-\uFFFF\-]|\\.)+)/,
		CLASS: /\.((?:[\w\u00c0-\uFFFF\-]|\\.)+)/,
		NAME: /\[name=['"]*((?:[\w\u00c0-\uFFFF\-]|\\.)+)['"]*\]/,
		ATTR: /\[\s*((?:[\w\u00c0-\uFFFF\-]|\\.)+)\s*(?:(\S?=)\s*(?:(['"])(.*?)\3|(#?(?:[\w\u00c0-\uFFFF\-]|\\.)*)|)|)\s*\]/,
		TAG: /^((?:[\w\u00c0-\uFFFF\*\-]|\\.)+)/,
		CHILD: /:(only|nth|last|first)-child(?:\(\s*(even|odd|(?:[+\-]?\d+|(?:[+\-]?\d*)?n\s*(?:[+\-]\s*\d+)?))\s*\))?/,
		POS: /:(nth|eq|gt|lt|first|last|even|odd)(?:\((\d*)\))?(?=[^\-]|$)/,
		PSEUDO: /:((?:[\w\u00c0-\uFFFF\-]|\\.)+)(?:\((['"]?)((?:\([^\)]+\)|[^\(\)]*)+)\2\))?/
	},

	leftMatch: {},

	attrMap: {
		"class": "className",
		"for": "htmlFor"
	},

	attrHandle: {
		href: function( elem ) {
			return elem.getAttribute( "href" );
		},
		type: function( elem ) {
			return elem.getAttribute( "type" );
		}
	},

	relative: {
		"+": function(checkSet, part){
			var isPartStr = typeof part === "string",
				isTag = isPartStr && !rNonWord.test( part ),
				isPartStrNotTag = isPartStr && !isTag;

			if ( isTag ) {
				part = part.toLowerCase();
			}

			for ( var i = 0, l = checkSet.length, elem; i < l; i++ ) {
				if ( (elem = checkSet[i]) ) {
					while ( (elem = elem.previousSibling) && elem.nodeType !== 1 ) {}

					checkSet[i] = isPartStrNotTag || elem && elem.nodeName.toLowerCase() === part ?
						elem || false :
						elem === part;
				}
			}

			if ( isPartStrNotTag ) {
				Sizzle.filter( part, checkSet, true );
			}
		},

		">": function( checkSet, part ) {
			var elem,
				isPartStr = typeof part === "string",
				i = 0,
				l = checkSet.length;

			if ( isPartStr && !rNonWord.test( part ) ) {
				part = part.toLowerCase();

				for ( ; i < l; i++ ) {
					elem = checkSet[i];

					if ( elem ) {
						var parent = elem.parentNode;
						checkSet[i] = parent.nodeName.toLowerCase() === part ? parent : false;
					}
				}

			} else {
				for ( ; i < l; i++ ) {
					elem = checkSet[i];

					if ( elem ) {
						checkSet[i] = isPartStr ?
							elem.parentNode :
							elem.parentNode === part;
					}
				}

				if ( isPartStr ) {
					Sizzle.filter( part, checkSet, true );
				}
			}
		},

		"": function(checkSet, part, isXML){
			var nodeCheck,
				doneName = done++,
				checkFn = dirCheck;

			if ( typeof part === "string" && !rNonWord.test( part ) ) {
				part = part.toLowerCase();
				nodeCheck = part;
				checkFn = dirNodeCheck;
			}

			checkFn( "parentNode", part, doneName, checkSet, nodeCheck, isXML );
		},

		"~": function( checkSet, part, isXML ) {
			var nodeCheck,
				doneName = done++,
				checkFn = dirCheck;

			if ( typeof part === "string" && !rNonWord.test( part ) ) {
				part = part.toLowerCase();
				nodeCheck = part;
				checkFn = dirNodeCheck;
			}

			checkFn( "previousSibling", part, doneName, checkSet, nodeCheck, isXML );
		}
	},

	find: {
		ID: function( match, context, isXML ) {
			if ( typeof context.getElementById !== "undefined" && !isXML ) {
				var m = context.getElementById(match[1]);
				// Check parentNode to catch when Blackberry 4.6 returns
				// nodes that are no longer in the document #6963
				return m && m.parentNode ? [m] : [];
			}
		},

		NAME: function( match, context ) {
			if ( typeof context.getElementsByName !== "undefined" ) {
				var ret = [],
					results = context.getElementsByName( match[1] );

				for ( var i = 0, l = results.length; i < l; i++ ) {
					if ( results[i].getAttribute("name") === match[1] ) {
						ret.push( results[i] );
					}
				}

				return ret.length === 0 ? null : ret;
			}
		},

		TAG: function( match, context ) {
			if ( typeof context.getElementsByTagName !== "undefined" ) {
				return context.getElementsByTagName( match[1] );
			}
		}
	},
	preFilter: {
		CLASS: function( match, curLoop, inplace, result, not, isXML ) {
			match = " " + match[1].replace( rBackslash, "" ) + " ";

			if ( isXML ) {
				return match;
			}

			for ( var i = 0, elem; (elem = curLoop[i]) != null; i++ ) {
				if ( elem ) {
					if ( not ^ (elem.className && (" " + elem.className + " ").replace(/[\t\n\r]/g, " ").indexOf(match) >= 0) ) {
						if ( !inplace ) {
							result.push( elem );
						}

					} else if ( inplace ) {
						curLoop[i] = false;
					}
				}
			}

			return false;
		},

		ID: function( match ) {
			return match[1].replace( rBackslash, "" );
		},

		TAG: function( match, curLoop ) {
			return match[1].replace( rBackslash, "" ).toLowerCase();
		},

		CHILD: function( match ) {
			if ( match[1] === "nth" ) {
				if ( !match[2] ) {
					Sizzle.error( match[0] );
				}

				match[2] = match[2].replace(/^\+|\s*/g, '');

				// parse equations like 'even', 'odd', '5', '2n', '3n+2', '4n-1', '-n+6'
				var test = /(-?)(\d*)(?:n([+\-]?\d*))?/.exec(
					match[2] === "even" && "2n" || match[2] === "odd" && "2n+1" ||
					!/\D/.test( match[2] ) && "0n+" + match[2] || match[2]);

				// calculate the numbers (first)n+(last) including if they are negative
				match[2] = (test[1] + (test[2] || 1)) - 0;
				match[3] = test[3] - 0;
			}
			else if ( match[2] ) {
				Sizzle.error( match[0] );
			}

			// TODO: Move to normal caching system
			match[0] = done++;

			return match;
		},

		ATTR: function( match, curLoop, inplace, result, not, isXML ) {
			var name = match[1] = match[1].replace( rBackslash, "" );

			if ( !isXML && Expr.attrMap[name] ) {
				match[1] = Expr.attrMap[name];
			}

			// Handle if an un-quoted value was used
			match[4] = ( match[4] || match[5] || "" ).replace( rBackslash, "" );

			if ( match[2] === "~=" ) {
				match[4] = " " + match[4] + " ";
			}

			return match;
		},

		PSEUDO: function( match, curLoop, inplace, result, not ) {
			if ( match[1] === "not" ) {
				// If we're dealing with a complex expression, or a simple one
				if ( ( chunker.exec(match[3]) || "" ).length > 1 || /^\w/.test(match[3]) ) {
					match[3] = Sizzle(match[3], null, null, curLoop);

				} else {
					var ret = Sizzle.filter(match[3], curLoop, inplace, true ^ not);

					if ( !inplace ) {
						result.push.apply( result, ret );
					}

					return false;
				}

			} else if ( Expr.match.POS.test( match[0] ) || Expr.match.CHILD.test( match[0] ) ) {
				return true;
			}

			return match;
		},

		POS: function( match ) {
			match.unshift( true );

			return match;
		}
	},

	filters: {
		enabled: function( elem ) {
			return elem.disabled === false && elem.type !== "hidden";
		},

		disabled: function( elem ) {
			return elem.disabled === true;
		},

		checked: function( elem ) {
			return elem.checked === true;
		},

		selected: function( elem ) {
			// Accessing this property makes selected-by-default
			// options in Safari work properly
			if ( elem.parentNode ) {
				elem.parentNode.selectedIndex;
			}

			return elem.selected === true;
		},

		parent: function( elem ) {
			return !!elem.firstChild;
		},

		empty: function( elem ) {
			return !elem.firstChild;
		},

		has: function( elem, i, match ) {
			return !!Sizzle( match[3], elem ).length;
		},

		header: function( elem ) {
			return (/h\d/i).test( elem.nodeName );
		},

		text: function( elem ) {
			var attr = elem.getAttribute( "type" ), type = elem.type;
			// IE6 and 7 will map elem.type to 'text' for new HTML5 types (search, etc)
			// use getAttribute instead to test this case
			return elem.nodeName.toLowerCase() === "input" && "text" === type && ( attr === type || attr === null );
		},

		radio: function( elem ) {
			return elem.nodeName.toLowerCase() === "input" && "radio" === elem.type;
		},

		checkbox: function( elem ) {
			return elem.nodeName.toLowerCase() === "input" && "checkbox" === elem.type;
		},

		file: function( elem ) {
			return elem.nodeName.toLowerCase() === "input" && "file" === elem.type;
		},

		password: function( elem ) {
			return elem.nodeName.toLowerCase() === "input" && "password" === elem.type;
		},

		submit: function( elem ) {
			var name = elem.nodeName.toLowerCase();
			return (name === "input" || name === "button") && "submit" === elem.type;
		},

		image: function( elem ) {
			return elem.nodeName.toLowerCase() === "input" && "image" === elem.type;
		},

		reset: function( elem ) {
			var name = elem.nodeName.toLowerCase();
			return (name === "input" || name === "button") && "reset" === elem.type;
		},

		button: function( elem ) {
			var name = elem.nodeName.toLowerCase();
			return name === "input" && "button" === elem.type || name === "button";
		},

		input: function( elem ) {
			return (/input|select|textarea|button/i).test( elem.nodeName );
		},

		focus: function( elem ) {
			return elem === elem.ownerDocument.activeElement;
		}
	},
	setFilters: {
		first: function( elem, i ) {
			return i === 0;
		},

		last: function( elem, i, match, array ) {
			return i === array.length - 1;
		},

		even: function( elem, i ) {
			return i % 2 === 0;
		},

		odd: function( elem, i ) {
			return i % 2 === 1;
		},

		lt: function( elem, i, match ) {
			return i < match[3] - 0;
		},

		gt: function( elem, i, match ) {
			return i > match[3] - 0;
		},

		nth: function( elem, i, match ) {
			return match[3] - 0 === i;
		},

		eq: function( elem, i, match ) {
			return match[3] - 0 === i;
		}
	},
	filter: {
		PSEUDO: function( elem, match, i, array ) {
			var name = match[1],
				filter = Expr.filters[ name ];

			if ( filter ) {
				return filter( elem, i, match, array );

			} else if ( name === "contains" ) {
				return (elem.textContent || elem.innerText || getText([ elem ]) || "").indexOf(match[3]) >= 0;

			} else if ( name === "not" ) {
				var not = match[3];

				for ( var j = 0, l = not.length; j < l; j++ ) {
					if ( not[j] === elem ) {
						return false;
					}
				}

				return true;

			} else {
				Sizzle.error( name );
			}
		},

		CHILD: function( elem, match ) {
			var first, last,
				doneName, parent, cache,
				count, diff,
				type = match[1],
				node = elem;

			switch ( type ) {
				case "only":
				case "first":
					while ( (node = node.previousSibling) ) {
						if ( node.nodeType === 1 ) {
							return false;
						}
					}

					if ( type === "first" ) {
						return true;
					}

					node = elem;

					/* falls through */
				case "last":
					while ( (node = node.nextSibling) ) {
						if ( node.nodeType === 1 ) {
							return false;
						}
					}

					return true;

				case "nth":
					first = match[2];
					last = match[3];

					if ( first === 1 && last === 0 ) {
						return true;
					}

					doneName = match[0];
					parent = elem.parentNode;

					if ( parent && (parent[ expando ] !== doneName || !elem.nodeIndex) ) {
						count = 0;

						for ( node = parent.firstChild; node; node = node.nextSibling ) {
							if ( node.nodeType === 1 ) {
								node.nodeIndex = ++count;
							}
						}

						parent[ expando ] = doneName;
					}

					diff = elem.nodeIndex - last;

					if ( first === 0 ) {
						return diff === 0;

					} else {
						return ( diff % first === 0 && diff / first >= 0 );
					}
			}
		},

		ID: function( elem, match ) {
			return elem.nodeType === 1 && elem.getAttribute("id") === match;
		},

		TAG: function( elem, match ) {
			return (match === "*" && elem.nodeType === 1) || !!elem.nodeName && elem.nodeName.toLowerCase() === match;
		},

		CLASS: function( elem, match ) {
			return (" " + (elem.className || elem.getAttribute("class")) + " ")
				.indexOf( match ) > -1;
		},

		ATTR: function( elem, match ) {
			var name = match[1],
				result = Sizzle.attr ?
					Sizzle.attr( elem, name ) :
					Expr.attrHandle[ name ] ?
					Expr.attrHandle[ name ]( elem ) :
					elem[ name ] != null ?
						elem[ name ] :
						elem.getAttribute( name ),
				value = result + "",
				type = match[2],
				check = match[4];

			return result == null ?
				type === "!=" :
				!type && Sizzle.attr ?
				result != null :
				type === "=" ?
				value === check :
				type === "*=" ?
				value.indexOf(check) >= 0 :
				type === "~=" ?
				(" " + value + " ").indexOf(check) >= 0 :
				!check ?
				value && result !== false :
				type === "!=" ?
				value !== check :
				type === "^=" ?
				value.indexOf(check) === 0 :
				type === "$=" ?
				value.substr(value.length - check.length) === check :
				type === "|=" ?
				value === check || value.substr(0, check.length + 1) === check + "-" :
				false;
		},

		POS: function( elem, match, i, array ) {
			var name = match[2],
				filter = Expr.setFilters[ name ];

			if ( filter ) {
				return filter( elem, i, match, array );
			}
		}
	}
};

var origPOS = Expr.match.POS,
	fescape = function(all, num){
		return "\\" + (num - 0 + 1);
	};

for ( var type in Expr.match ) {
	Expr.match[ type ] = new RegExp( Expr.match[ type ].source + (/(?![^\[]*\])(?![^\(]*\))/.source) );
	Expr.leftMatch[ type ] = new RegExp( /(^(?:.|\r|\n)*?)/.source + Expr.match[ type ].source.replace(/\\(\d+)/g, fescape) );
}
// Expose origPOS
// "global" as in regardless of relation to brackets/parens
Expr.match.globalPOS = origPOS;

var makeArray = function( array, results ) {
	array = Array.prototype.slice.call( array, 0 );

	if ( results ) {
		results.push.apply( results, array );
		return results;
	}

	return array;
};

// Perform a simple check to determine if the browser is capable of
// converting a NodeList to an array using builtin methods.
// Also verifies that the returned array holds DOM nodes
// (which is not the case in the Blackberry browser)
try {
	Array.prototype.slice.call( document.documentElement.childNodes, 0 )[0].nodeType;

// Provide a fallback method if it does not work
} catch( e ) {
	makeArray = function( array, results ) {
		var i = 0,
			ret = results || [];

		if ( toString.call(array) === "[object Array]" ) {
			Array.prototype.push.apply( ret, array );

		} else {
			if ( typeof array.length === "number" ) {
				for ( var l = array.length; i < l; i++ ) {
					ret.push( array[i] );
				}

			} else {
				for ( ; array[i]; i++ ) {
					ret.push( array[i] );
				}
			}
		}

		return ret;
	};
}

var sortOrder, siblingCheck;

if ( document.documentElement.compareDocumentPosition ) {
	sortOrder = function( a, b ) {
		if ( a === b ) {
			hasDuplicate = true;
			return 0;
		}

		if ( !a.compareDocumentPosition || !b.compareDocumentPosition ) {
			return a.compareDocumentPosition ? -1 : 1;
		}

		return a.compareDocumentPosition(b) & 4 ? -1 : 1;
	};

} else {
	sortOrder = function( a, b ) {
		// The nodes are identical, we can exit early
		if ( a === b ) {
			hasDuplicate = true;
			return 0;

		// Fallback to using sourceIndex (in IE) if it's available on both nodes
		} else if ( a.sourceIndex && b.sourceIndex ) {
			return a.sourceIndex - b.sourceIndex;
		}

		var al, bl,
			ap = [],
			bp = [],
			aup = a.parentNode,
			bup = b.parentNode,
			cur = aup;

		// If the nodes are siblings (or identical) we can do a quick check
		if ( aup === bup ) {
			return siblingCheck( a, b );

		// If no parents were found then the nodes are disconnected
		} else if ( !aup ) {
			return -1;

		} else if ( !bup ) {
			return 1;
		}

		// Otherwise they're somewhere else in the tree so we need
		// to build up a full list of the parentNodes for comparison
		while ( cur ) {
			ap.unshift( cur );
			cur = cur.parentNode;
		}

		cur = bup;

		while ( cur ) {
			bp.unshift( cur );
			cur = cur.parentNode;
		}

		al = ap.length;
		bl = bp.length;

		// Start walking down the tree looking for a discrepancy
		for ( var i = 0; i < al && i < bl; i++ ) {
			if ( ap[i] !== bp[i] ) {
				return siblingCheck( ap[i], bp[i] );
			}
		}

		// We ended someplace up the tree so do a sibling check
		return i === al ?
			siblingCheck( a, bp[i], -1 ) :
			siblingCheck( ap[i], b, 1 );
	};

	siblingCheck = function( a, b, ret ) {
		if ( a === b ) {
			return ret;
		}

		var cur = a.nextSibling;

		while ( cur ) {
			if ( cur === b ) {
				return -1;
			}

			cur = cur.nextSibling;
		}

		return 1;
	};
}

// Check to see if the browser returns elements by name when
// querying by getElementById (and provide a workaround)
(function(){
	// We're going to inject a fake input element with a specified name
	var form = document.createElement("div"),
		id = "script" + (new Date()).getTime(),
		root = document.documentElement;

	form.innerHTML = "<a name='" + id + "'/>";

	// Inject it into the root element, check its status, and remove it quickly
	root.insertBefore( form, root.firstChild );

	// The workaround has to do additional checks after a getElementById
	// Which slows things down for other browsers (hence the branching)
	if ( document.getElementById( id ) ) {
		Expr.find.ID = function( match, context, isXML ) {
			if ( typeof context.getElementById !== "undefined" && !isXML ) {
				var m = context.getElementById(match[1]);

				return m ?
					m.id === match[1] || typeof m.getAttributeNode !== "undefined" && m.getAttributeNode("id").nodeValue === match[1] ?
						[m] :
						undefined :
					[];
			}
		};

		Expr.filter.ID = function( elem, match ) {
			var node = typeof elem.getAttributeNode !== "undefined" && elem.getAttributeNode("id");

			return elem.nodeType === 1 && node && node.nodeValue === match;
		};
	}

	root.removeChild( form );

	// release memory in IE
	root = form = null;
})();

(function(){
	// Check to see if the browser returns only elements
	// when doing getElementsByTagName("*")

	// Create a fake element
	var div = document.createElement("div");
	div.appendChild( document.createComment("") );

	// Make sure no comments are found
	if ( div.getElementsByTagName("*").length > 0 ) {
		Expr.find.TAG = function( match, context ) {
			var results = context.getElementsByTagName( match[1] );

			// Filter out possible comments
			if ( match[1] === "*" ) {
				var tmp = [];

				for ( var i = 0; results[i]; i++ ) {
					if ( results[i].nodeType === 1 ) {
						tmp.push( results[i] );
					}
				}

				results = tmp;
			}

			return results;
		};
	}

	// Check to see if an attribute returns normalized href attributes
	div.innerHTML = "<a href='#'></a>";

	if ( div.firstChild && typeof div.firstChild.getAttribute !== "undefined" &&
			div.firstChild.getAttribute("href") !== "#" ) {

		Expr.attrHandle.href = function( elem ) {
			return elem.getAttribute( "href", 2 );
		};
	}

	// release memory in IE
	div = null;
})();

if ( document.querySelectorAll ) {
	(function(){
		var oldSizzle = Sizzle,
			div = document.createElement("div"),
			id = "__sizzle__";

		div.innerHTML = "<p class='TEST'></p>";

		// Safari can't handle uppercase or unicode characters when
		// in quirks mode.
		if ( div.querySelectorAll && div.querySelectorAll(".TEST").length === 0 ) {
			return;
		}

		Sizzle = function( query, context, extra, seed ) {
			context = context || document;

			// Only use querySelectorAll on non-XML documents
			// (ID selectors don't work in non-HTML documents)
			if ( !seed && !Sizzle.isXML(context) ) {
				// See if we find a selector to speed up
				var match = /^(\w+$)|^\.([\w\-]+$)|^#([\w\-]+$)/.exec( query );

				if ( match && (context.nodeType === 1 || context.nodeType === 9) ) {
					// Speed-up: Sizzle("TAG")
					if ( match[1] ) {
						return makeArray( context.getElementsByTagName( query ), extra );

					// Speed-up: Sizzle(".CLASS")
					} else if ( match[2] && Expr.find.CLASS && context.getElementsByClassName ) {
						return makeArray( context.getElementsByClassName( match[2] ), extra );
					}
				}

				if ( context.nodeType === 9 ) {
					// Speed-up: Sizzle("body")
					// The body element only exists once, optimize finding it
					if ( query === "body" && context.body ) {
						return makeArray( [ context.body ], extra );

					// Speed-up: Sizzle("#ID")
					} else if ( match && match[3] ) {
						var elem = context.getElementById( match[3] );

						// Check parentNode to catch when Blackberry 4.6 returns
						// nodes that are no longer in the document #6963
						if ( elem && elem.parentNode ) {
							// Handle the case where IE and Opera return items
							// by name instead of ID
							if ( elem.id === match[3] ) {
								return makeArray( [ elem ], extra );
							}

						} else {
							return makeArray( [], extra );
						}
					}

					try {
						return makeArray( context.querySelectorAll(query), extra );
					} catch(qsaError) {}

				// qSA works strangely on Element-rooted queries
				// We can work around this by specifying an extra ID on the root
				// and working up from there (Thanks to Andrew Dupont for the technique)
				// IE 8 doesn't work on object elements
				} else if ( context.nodeType === 1 && context.nodeName.toLowerCase() !== "object" ) {
					var oldContext = context,
						old = context.getAttribute( "id" ),
						nid = old || id,
						hasParent = context.parentNode,
						relativeHierarchySelector = /^\s*[+~]/.test( query );

					if ( !old ) {
						context.setAttribute( "id", nid );
					} else {
						nid = nid.replace( /'/g, "\\$&" );
					}
					if ( relativeHierarchySelector && hasParent ) {
						context = context.parentNode;
					}

					try {
						if ( !relativeHierarchySelector || hasParent ) {
							return makeArray( context.querySelectorAll( "[id='" + nid + "'] " + query ), extra );
						}

					} catch(pseudoError) {
					} finally {
						if ( !old ) {
							oldContext.removeAttribute( "id" );
						}
					}
				}
			}

			return oldSizzle(query, context, extra, seed);
		};

		for ( var prop in oldSizzle ) {
			Sizzle[ prop ] = oldSizzle[ prop ];
		}

		// release memory in IE
		div = null;
	})();
}

(function(){
	var html = document.documentElement,
		matches = html.matchesSelector || html.mozMatchesSelector || html.webkitMatchesSelector || html.msMatchesSelector;

	if ( matches ) {
		// Check to see if it's possible to do matchesSelector
		// on a disconnected node (IE 9 fails this)
		var disconnectedMatch = !matches.call( document.createElement( "div" ), "div" ),
			pseudoWorks = false;

		try {
			// This should fail with an exception
			// Gecko does not error, returns false instead
			matches.call( document.documentElement, "[test!='']:sizzle" );

		} catch( pseudoError ) {
			pseudoWorks = true;
		}

		Sizzle.matchesSelector = function( node, expr ) {
			// Make sure that attribute selectors are quoted
			expr = expr.replace(/\=\s*([^'"\]]*)\s*\]/g, "='$1']");

			if ( !Sizzle.isXML( node ) ) {
				try {
					if ( pseudoWorks || !Expr.match.PSEUDO.test( expr ) && !/!=/.test( expr ) ) {
						var ret = matches.call( node, expr );

						// IE 9's matchesSelector returns false on disconnected nodes
						if ( ret || !disconnectedMatch ||
								// As well, disconnected nodes are said to be in a document
								// fragment in IE 9, so check for that
								node.document && node.document.nodeType !== 11 ) {
							return ret;
						}
					}
				} catch(e) {}
			}

			return Sizzle(expr, null, null, [node]).length > 0;
		};
	}
})();

(function(){
	var div = document.createElement("div");

	div.innerHTML = "<div class='test e'></div><div class='test'></div>";

	// Opera can't find a second classname (in 9.6)
	// Also, make sure that getElementsByClassName actually exists
	if ( !div.getElementsByClassName || div.getElementsByClassName("e").length === 0 ) {
		return;
	}

	// Safari caches class attributes, doesn't catch changes (in 3.2)
	div.lastChild.className = "e";

	if ( div.getElementsByClassName("e").length === 1 ) {
		return;
	}

	Expr.order.splice(1, 0, "CLASS");
	Expr.find.CLASS = function( match, context, isXML ) {
		if ( typeof context.getElementsByClassName !== "undefined" && !isXML ) {
			return context.getElementsByClassName(match[1]);
		}
	};

	// release memory in IE
	div = null;
})();

function dirNodeCheck( dir, cur, doneName, checkSet, nodeCheck, isXML ) {
	for ( var i = 0, l = checkSet.length; i < l; i++ ) {
		var elem = checkSet[i];

		if ( elem ) {
			var match = false;

			elem = elem[dir];

			while ( elem ) {
				if ( elem[ expando ] === doneName ) {
					match = checkSet[elem.sizset];
					break;
				}

				if ( elem.nodeType === 1 && !isXML ){
					elem[ expando ] = doneName;
					elem.sizset = i;
				}

				if ( elem.nodeName.toLowerCase() === cur ) {
					match = elem;
					break;
				}

				elem = elem[dir];
			}

			checkSet[i] = match;
		}
	}
}

function dirCheck( dir, cur, doneName, checkSet, nodeCheck, isXML ) {
	for ( var i = 0, l = checkSet.length; i < l; i++ ) {
		var elem = checkSet[i];

		if ( elem ) {
			var match = false;

			elem = elem[dir];

			while ( elem ) {
				if ( elem[ expando ] === doneName ) {
					match = checkSet[elem.sizset];
					break;
				}

				if ( elem.nodeType === 1 ) {
					if ( !isXML ) {
						elem[ expando ] = doneName;
						elem.sizset = i;
					}

					if ( typeof cur !== "string" ) {
						if ( elem === cur ) {
							match = true;
							break;
						}

					} else if ( Sizzle.filter( cur, [elem] ).length > 0 ) {
						match = elem;
						break;
					}
				}

				elem = elem[dir];
			}

			checkSet[i] = match;
		}
	}
}

if ( document.documentElement.contains ) {
	Sizzle.contains = function( a, b ) {
		return a !== b && (a.contains ? a.contains(b) : true);
	};

} else if ( document.documentElement.compareDocumentPosition ) {
	Sizzle.contains = function( a, b ) {
		return !!(a.compareDocumentPosition(b) & 16);
	};

} else {
	Sizzle.contains = function() {
		return false;
	};
}

Sizzle.isXML = function( elem ) {
	// documentElement is verified for cases where it doesn't yet exist
	// (such as loading iframes in IE - #4833)
	var documentElement = (elem ? elem.ownerDocument || elem : 0).documentElement;

	return documentElement ? documentElement.nodeName !== "HTML" : false;
};

var posProcess = function( selector, context, seed ) {
	var match,
		tmpSet = [],
		later = "",
		root = context.nodeType ? [context] : context;

	// Position selectors must be done after the filter
	// And so must :not(positional) so we move all PSEUDOs to the end
	while ( (match = Expr.match.PSEUDO.exec( selector )) ) {
		later += match[0];
		selector = selector.replace( Expr.match.PSEUDO, "" );
	}

	selector = Expr.relative[selector] ? selector + "*" : selector;

	for ( var i = 0, l = root.length; i < l; i++ ) {
		Sizzle( selector, root[i], tmpSet, seed );
	}

	return Sizzle.filter( later, tmpSet );
};

// EXPOSE
// Override sizzle attribute retrieval
Sizzle.attr = jQuery.attr;
Sizzle.selectors.attrMap = {};
jQuery.find = Sizzle;
jQuery.expr = Sizzle.selectors;
jQuery.expr[":"] = jQuery.expr.filters;
jQuery.unique = Sizzle.uniqueSort;
jQuery.text = Sizzle.getText;
jQuery.isXMLDoc = Sizzle.isXML;
jQuery.contains = Sizzle.contains;


})();


var runtil = /Until$/,
	rparentsprev = /^(?:parents|prevUntil|prevAll)/,
	// Note: This RegExp should be improved, or likely pulled from Sizzle
	rmultiselector = /,/,
	isSimple = /^.[^:#\[\.,]*$/,
	slice = Array.prototype.slice,
	POS = jQuery.expr.match.globalPOS,
	// methods guaranteed to produce a unique set when starting from a unique set
	guaranteedUnique = {
		children: true,
		contents: true,
		next: true,
		prev: true
	};

jQuery.fn.extend({
	find: function( selector ) {
		var self = this,
			i, l;

		if ( typeof selector !== "string" ) {
			return jQuery( selector ).filter(function() {
				for ( i = 0, l = self.length; i < l; i++ ) {
					if ( jQuery.contains( self[ i ], this ) ) {
						return true;
					}
				}
			});
		}

		var ret = this.pushStack( "", "find", selector ),
			length, n, r;

		for ( i = 0, l = this.length; i < l; i++ ) {
			length = ret.length;
			jQuery.find( selector, this[i], ret );

			if ( i > 0 ) {
				// Make sure that the results are unique
				for ( n = length; n < ret.length; n++ ) {
					for ( r = 0; r < length; r++ ) {
						if ( ret[r] === ret[n] ) {
							ret.splice(n--, 1);
							break;
						}
					}
				}
			}
		}

		return ret;
	},

	has: function( target ) {
		var targets = jQuery( target );
		return this.filter(function() {
			for ( var i = 0, l = targets.length; i < l; i++ ) {
				if ( jQuery.contains( this, targets[i] ) ) {
					return true;
				}
			}
		});
	},

	not: function( selector ) {
		return this.pushStack( winnow(this, selector, false), "not", selector);
	},

	filter: function( selector ) {
		return this.pushStack( winnow(this, selector, true), "filter", selector );
	},

	is: function( selector ) {
		return !!selector && (
			typeof selector === "string" ?
				// If this is a positional selector, check membership in the returned set
				// so $("p:first").is("p:last") won't return true for a doc with two "p".
				POS.test( selector ) ?
					jQuery( selector, this.context ).index( this[0] ) >= 0 :
					jQuery.filter( selector, this ).length > 0 :
				this.filter( selector ).length > 0 );
	},

	closest: function( selectors, context ) {
		var ret = [], i, l, cur = this[0];

		// Array (deprecated as of jQuery 1.7)
		if ( jQuery.isArray( selectors ) ) {
			var level = 1;

			while ( cur && cur.ownerDocument && cur !== context ) {
				for ( i = 0; i < selectors.length; i++ ) {

					if ( jQuery( cur ).is( selectors[ i ] ) ) {
						ret.push({ selector: selectors[ i ], elem: cur, level: level });
					}
				}

				cur = cur.parentNode;
				level++;
			}

			return ret;
		}

		// String
		var pos = POS.test( selectors ) || typeof selectors !== "string" ?
				jQuery( selectors, context || this.context ) :
				0;

		for ( i = 0, l = this.length; i < l; i++ ) {
			cur = this[i];

			while ( cur ) {
				if ( pos ? pos.index(cur) > -1 : jQuery.find.matchesSelector(cur, selectors) ) {
					ret.push( cur );
					break;

				} else {
					cur = cur.parentNode;
					if ( !cur || !cur.ownerDocument || cur === context || cur.nodeType === 11 ) {
						break;
					}
				}
			}
		}

		ret = ret.length > 1 ? jQuery.unique( ret ) : ret;

		return this.pushStack( ret, "closest", selectors );
	},

	// Determine the position of an element within
	// the matched set of elements
	index: function( elem ) {

		// No argument, return index in parent
		if ( !elem ) {
			return ( this[0] && this[0].parentNode ) ? this.prevAll().length : -1;
		}

		// index in selector
		if ( typeof elem === "string" ) {
			return jQuery.inArray( this[0], jQuery( elem ) );
		}

		// Locate the position of the desired element
		return jQuery.inArray(
			// If it receives a jQuery object, the first element is used
			elem.jquery ? elem[0] : elem, this );
	},

	add: function( selector, context ) {
		var set = typeof selector === "string" ?
				jQuery( selector, context ) :
				jQuery.makeArray( selector && selector.nodeType ? [ selector ] : selector ),
			all = jQuery.merge( this.get(), set );

		return this.pushStack( isDisconnected( set[0] ) || isDisconnected( all[0] ) ?
			all :
			jQuery.unique( all ) );
	},

	andSelf: function() {
		return this.add( this.prevObject );
	}
});

// A painfully simple check to see if an element is disconnected
// from a document (should be improved, where feasible).
function isDisconnected( node ) {
	return !node || !node.parentNode || node.parentNode.nodeType === 11;
}

jQuery.each({
	parent: function( elem ) {
		var parent = elem.parentNode;
		return parent && parent.nodeType !== 11 ? parent : null;
	},
	parents: function( elem ) {
		return jQuery.dir( elem, "parentNode" );
	},
	parentsUntil: function( elem, i, until ) {
		return jQuery.dir( elem, "parentNode", until );
	},
	next: function( elem ) {
		return jQuery.nth( elem, 2, "nextSibling" );
	},
	prev: function( elem ) {
		return jQuery.nth( elem, 2, "previousSibling" );
	},
	nextAll: function( elem ) {
		return jQuery.dir( elem, "nextSibling" );
	},
	prevAll: function( elem ) {
		return jQuery.dir( elem, "previousSibling" );
	},
	nextUntil: function( elem, i, until ) {
		return jQuery.dir( elem, "nextSibling", until );
	},
	prevUntil: function( elem, i, until ) {
		return jQuery.dir( elem, "previousSibling", until );
	},
	siblings: function( elem ) {
		return jQuery.sibling( ( elem.parentNode || {} ).firstChild, elem );
	},
	children: function( elem ) {
		return jQuery.sibling( elem.firstChild );
	},
	contents: function( elem ) {
		return jQuery.nodeName( elem, "iframe" ) ?
			elem.contentDocument || elem.contentWindow.document :
			jQuery.makeArray( elem.childNodes );
	}
}, function( name, fn ) {
	jQuery.fn[ name ] = function( until, selector ) {
		var ret = jQuery.map( this, fn, until );

		if ( !runtil.test( name ) ) {
			selector = until;
		}

		if ( selector && typeof selector === "string" ) {
			ret = jQuery.filter( selector, ret );
		}

		ret = this.length > 1 && !guaranteedUnique[ name ] ? jQuery.unique( ret ) : ret;

		if ( (this.length > 1 || rmultiselector.test( selector )) && rparentsprev.test( name ) ) {
			ret = ret.reverse();
		}

		return this.pushStack( ret, name, slice.call( arguments ).join(",") );
	};
});

jQuery.extend({
	filter: function( expr, elems, not ) {
		if ( not ) {
			expr = ":not(" + expr + ")";
		}

		return elems.length === 1 ?
			jQuery.find.matchesSelector(elems[0], expr) ? [ elems[0] ] : [] :
			jQuery.find.matches(expr, elems);
	},

	dir: function( elem, dir, until ) {
		var matched = [],
			cur = elem[ dir ];

		while ( cur && cur.nodeType !== 9 && (until === undefined || cur.nodeType !== 1 || !jQuery( cur ).is( until )) ) {
			if ( cur.nodeType === 1 ) {
				matched.push( cur );
			}
			cur = cur[dir];
		}
		return matched;
	},

	nth: function( cur, result, dir, elem ) {
		result = result || 1;
		var num = 0;

		for ( ; cur; cur = cur[dir] ) {
			if ( cur.nodeType === 1 && ++num === result ) {
				break;
			}
		}

		return cur;
	},

	sibling: function( n, elem ) {
		var r = [];

		for ( ; n; n = n.nextSibling ) {
			if ( n.nodeType === 1 && n !== elem ) {
				r.push( n );
			}
		}

		return r;
	}
});

// Implement the identical functionality for filter and not
function winnow( elements, qualifier, keep ) {

	// Can't pass null or undefined to indexOf in Firefox 4
	// Set to 0 to skip string check
	qualifier = qualifier || 0;

	if ( jQuery.isFunction( qualifier ) ) {
		return jQuery.grep(elements, function( elem, i ) {
			var retVal = !!qualifier.call( elem, i, elem );
			return retVal === keep;
		});

	} else if ( qualifier.nodeType ) {
		return jQuery.grep(elements, function( elem, i ) {
			return ( elem === qualifier ) === keep;
		});

	} else if ( typeof qualifier === "string" ) {
		var filtered = jQuery.grep(elements, function( elem ) {
			return elem.nodeType === 1;
		});

		if ( isSimple.test( qualifier ) ) {
			return jQuery.filter(qualifier, filtered, !keep);
		} else {
			qualifier = jQuery.filter( qualifier, filtered );
		}
	}

	return jQuery.grep(elements, function( elem, i ) {
		return ( jQuery.inArray( elem, qualifier ) >= 0 ) === keep;
	});
}




function createSafeFragment( document ) {
	var list = nodeNames.split( "|" ),
	safeFrag = document.createDocumentFragment();

	if ( safeFrag.createElement ) {
		while ( list.length ) {
			safeFrag.createElement(
				list.pop()
			);
		}
	}
	return safeFrag;
}

var nodeNames = "abbr|article|aside|audio|bdi|canvas|data|datalist|details|figcaption|figure|footer|" +
		"header|hgroup|mark|meter|nav|output|progress|section|summary|time|video",
	rinlinejQuery = / jQuery\d+="(?:\d+|null)"/g,
	rleadingWhitespace = /^\s+/,
	rxhtmlTag = /<(?!area|br|col|embed|hr|img|input|link|meta|param)(([\w:]+)[^>]*)\/>/ig,
	rtagName = /<([\w:]+)/,
	rtbody = /<tbody/i,
	rhtml = /<|&#?\w+;/,
	rnoInnerhtml = /<(?:script|style)/i,
	rnocache = /<(?:script|object|embed|option|style)/i,
	rnoshimcache = new RegExp("<(?:" + nodeNames + ")[\\s/>]", "i"),
	// checked="checked" or checked
	rchecked = /checked\s*(?:[^=]|=\s*.checked.)/i,
	rscriptType = /\/(java|ecma)script/i,
	rcleanScript = /^\s*<!(?:\[CDATA\[|\-\-)/,
	wrapMap = {
		option: [ 1, "<select multiple='multiple'>", "</select>" ],
		legend: [ 1, "<fieldset>", "</fieldset>" ],
		thead: [ 1, "<table>", "</table>" ],
		tr: [ 2, "<table><tbody>", "</tbody></table>" ],
		td: [ 3, "<table><tbody><tr>", "</tr></tbody></table>" ],
		col: [ 2, "<table><tbody></tbody><colgroup>", "</colgroup></table>" ],
		area: [ 1, "<map>", "</map>" ],
		_default: [ 0, "", "" ]
	},
	safeFragment = createSafeFragment( document );

wrapMap.optgroup = wrapMap.option;
wrapMap.tbody = wrapMap.tfoot = wrapMap.colgroup = wrapMap.caption = wrapMap.thead;
wrapMap.th = wrapMap.td;

// IE can't serialize <link> and <script> tags normally
if ( !jQuery.support.htmlSerialize ) {
	wrapMap._default = [ 1, "div<div>", "</div>" ];
}

jQuery.fn.extend({
	text: function( value ) {
		return jQuery.access( this, function( value ) {
			return value === undefined ?
				jQuery.text( this ) :
				this.empty().append( ( this[0] && this[0].ownerDocument || document ).createTextNode( value ) );
		}, null, value, arguments.length );
	},

	wrapAll: function( html ) {
		if ( jQuery.isFunction( html ) ) {
			return this.each(function(i) {
				jQuery(this).wrapAll( html.call(this, i) );
			});
		}

		if ( this[0] ) {
			// The elements to wrap the target around
			var wrap = jQuery( html, this[0].ownerDocument ).eq(0).clone(true);

			if ( this[0].parentNode ) {
				wrap.insertBefore( this[0] );
			}

			wrap.map(function() {
				var elem = this;

				while ( elem.firstChild && elem.firstChild.nodeType === 1 ) {
					elem = elem.firstChild;
				}

				return elem;
			}).append( this );
		}

		return this;
	},

	wrapInner: function( html ) {
		if ( jQuery.isFunction( html ) ) {
			return this.each(function(i) {
				jQuery(this).wrapInner( html.call(this, i) );
			});
		}

		return this.each(function() {
			var self = jQuery( this ),
				contents = self.contents();

			if ( contents.length ) {
				contents.wrapAll( html );

			} else {
				self.append( html );
			}
		});
	},

	wrap: function( html ) {
		var isFunction = jQuery.isFunction( html );

		return this.each(function(i) {
			jQuery( this ).wrapAll( isFunction ? html.call(this, i) : html );
		});
	},

	unwrap: function() {
		return this.parent().each(function() {
			if ( !jQuery.nodeName( this, "body" ) ) {
				jQuery( this ).replaceWith( this.childNodes );
			}
		}).end();
	},

	append: function() {
		return this.domManip(arguments, true, function( elem ) {
			if ( this.nodeType === 1 ) {
				this.appendChild( elem );
			}
		});
	},

	prepend: function() {
		return this.domManip(arguments, true, function( elem ) {
			if ( this.nodeType === 1 ) {
				this.insertBefore( elem, this.firstChild );
			}
		});
	},

	before: function() {
		if ( this[0] && this[0].parentNode ) {
			return this.domManip(arguments, false, function( elem ) {
				this.parentNode.insertBefore( elem, this );
			});
		} else if ( arguments.length ) {
			var set = jQuery.clean( arguments );
			set.push.apply( set, this.toArray() );
			return this.pushStack( set, "before", arguments );
		}
	},

	after: function() {
		if ( this[0] && this[0].parentNode ) {
			return this.domManip(arguments, false, function( elem ) {
				this.parentNode.insertBefore( elem, this.nextSibling );
			});
		} else if ( arguments.length ) {
			var set = this.pushStack( this, "after", arguments );
			set.push.apply( set, jQuery.clean(arguments) );
			return set;
		}
	},

	// keepData is for internal use only--do not document
	remove: function( selector, keepData ) {
		for ( var i = 0, elem; (elem = this[i]) != null; i++ ) {
			if ( !selector || jQuery.filter( selector, [ elem ] ).length ) {
				if ( !keepData && elem.nodeType === 1 ) {
					jQuery.cleanData( elem.getElementsByTagName("*") );
					jQuery.cleanData( [ elem ] );
				}

				if ( elem.parentNode ) {
					elem.parentNode.removeChild( elem );
				}
			}
		}

		return this;
	},

	empty: function() {
		for ( var i = 0, elem; (elem = this[i]) != null; i++ ) {
			// Remove element nodes and prevent memory leaks
			if ( elem.nodeType === 1 ) {
				jQuery.cleanData( elem.getElementsByTagName("*") );
			}

			// Remove any remaining nodes
			while ( elem.firstChild ) {
				elem.removeChild( elem.firstChild );
			}
		}

		return this;
	},

	clone: function( dataAndEvents, deepDataAndEvents ) {
		dataAndEvents = dataAndEvents == null ? false : dataAndEvents;
		deepDataAndEvents = deepDataAndEvents == null ? dataAndEvents : deepDataAndEvents;

		return this.map( function () {
			return jQuery.clone( this, dataAndEvents, deepDataAndEvents );
		});
	},

	html: function( value ) {
		return jQuery.access( this, function( value ) {
			var elem = this[0] || {},
				i = 0,
				l = this.length;

			if ( value === undefined ) {
				return elem.nodeType === 1 ?
					elem.innerHTML.replace( rinlinejQuery, "" ) :
					null;
			}


			if ( typeof value === "string" && !rnoInnerhtml.test( value ) &&
				( jQuery.support.leadingWhitespace || !rleadingWhitespace.test( value ) ) &&
				!wrapMap[ ( rtagName.exec( value ) || ["", ""] )[1].toLowerCase() ] ) {

				value = value.replace( rxhtmlTag, "<$1></$2>" );

				try {
					for (; i < l; i++ ) {
						// Remove element nodes and prevent memory leaks
						elem = this[i] || {};
						if ( elem.nodeType === 1 ) {
							jQuery.cleanData( elem.getElementsByTagName( "*" ) );
							elem.innerHTML = value;
						}
					}

					elem = 0;

				// If using innerHTML throws an exception, use the fallback method
				} catch(e) {}
			}

			if ( elem ) {
				this.empty().append( value );
			}
		}, null, value, arguments.length );
	},

	replaceWith: function( value ) {
		if ( this[0] && this[0].parentNode ) {
			// Make sure that the elements are removed from the DOM before they are inserted
			// this can help fix replacing a parent with child elements
			if ( jQuery.isFunction( value ) ) {
				return this.each(function(i) {
					var self = jQuery(this), old = self.html();
					self.replaceWith( value.call( this, i, old ) );
				});
			}

			if ( typeof value !== "string" ) {
				value = jQuery( value ).detach();
			}

			return this.each(function() {
				var next = this.nextSibling,
					parent = this.parentNode;

				jQuery( this ).remove();

				if ( next ) {
					jQuery(next).before( value );
				} else {
					jQuery(parent).append( value );
				}
			});
		} else {
			return this.length ?
				this.pushStack( jQuery(jQuery.isFunction(value) ? value() : value), "replaceWith", value ) :
				this;
		}
	},

	detach: function( selector ) {
		return this.remove( selector, true );
	},

	domManip: function( args, table, callback ) {
		var results, first, fragment, parent,
			value = args[0],
			scripts = [];

		// We can't cloneNode fragments that contain checked, in WebKit
		if ( !jQuery.support.checkClone && arguments.length === 3 && typeof value === "string" && rchecked.test( value ) ) {
			return this.each(function() {
				jQuery(this).domManip( args, table, callback, true );
			});
		}

		if ( jQuery.isFunction(value) ) {
			return this.each(function(i) {
				var self = jQuery(this);
				args[0] = value.call(this, i, table ? self.html() : undefined);
				self.domManip( args, table, callback );
			});
		}

		if ( this[0] ) {
			parent = value && value.parentNode;

			// If we're in a fragment, just use that instead of building a new one
			if ( jQuery.support.parentNode && parent && parent.nodeType === 11 && parent.childNodes.length === this.length ) {
				results = { fragment: parent };

			} else {
				results = jQuery.buildFragment( args, this, scripts );
			}

			fragment = results.fragment;

			if ( fragment.childNodes.length === 1 ) {
				first = fragment = fragment.firstChild;
			} else {
				first = fragment.firstChild;
			}

			if ( first ) {
				table = table && jQuery.nodeName( first, "tr" );

				for ( var i = 0, l = this.length, lastIndex = l - 1; i < l; i++ ) {
					callback.call(
						table ?
							root(this[i], first) :
							this[i],
						// Make sure that we do not leak memory by inadvertently discarding
						// the original fragment (which might have attached data) instead of
						// using it; in addition, use the original fragment object for the last
						// item instead of first because it can end up being emptied incorrectly
						// in certain situations (Bug #8070).
						// Fragments from the fragment cache must always be cloned and never used
						// in place.
						results.cacheable || ( l > 1 && i < lastIndex ) ?
							jQuery.clone( fragment, true, true ) :
							fragment
					);
				}
			}

			if ( scripts.length ) {
				jQuery.each( scripts, function( i, elem ) {
					if ( elem.src ) {
						jQuery.ajax({
							type: "GET",
							global: false,
							url: elem.src,
							async: false,
							dataType: "script"
						});
					} else {
						jQuery.globalEval( ( elem.text || elem.textContent || elem.innerHTML || "" ).replace( rcleanScript, "/*$0*/" ) );
					}

					if ( elem.parentNode ) {
						elem.parentNode.removeChild( elem );
					}
				});
			}
		}

		return this;
	}
});

function root( elem, cur ) {
	return jQuery.nodeName(elem, "table") ?
		(elem.getElementsByTagName("tbody")[0] ||
		elem.appendChild(elem.ownerDocument.createElement("tbody"))) :
		elem;
}

function cloneCopyEvent( src, dest ) {

	if ( dest.nodeType !== 1 || !jQuery.hasData( src ) ) {
		return;
	}

	var type, i, l,
		oldData = jQuery._data( src ),
		curData = jQuery._data( dest, oldData ),
		events = oldData.events;

	if ( events ) {
		delete curData.handle;
		curData.events = {};

		for ( type in events ) {
			for ( i = 0, l = events[ type ].length; i < l; i++ ) {
				jQuery.event.add( dest, type, events[ type ][ i ] );
			}
		}
	}

	// make the cloned public data object a copy from the original
	if ( curData.data ) {
		curData.data = jQuery.extend( {}, curData.data );
	}
}

function cloneFixAttributes( src, dest ) {
	var nodeName;

	// We do not need to do anything for non-Elements
	if ( dest.nodeType !== 1 ) {
		return;
	}

	// clearAttributes removes the attributes, which we don't want,
	// but also removes the attachEvent events, which we *do* want
	if ( dest.clearAttributes ) {
		dest.clearAttributes();
	}

	// mergeAttributes, in contrast, only merges back on the
	// original attributes, not the events
	if ( dest.mergeAttributes ) {
		dest.mergeAttributes( src );
	}

	nodeName = dest.nodeName.toLowerCase();

	// IE6-8 fail to clone children inside object elements that use
	// the proprietary classid attribute value (rather than the type
	// attribute) to identify the type of content to display
	if ( nodeName === "object" ) {
		dest.outerHTML = src.outerHTML;

	} else if ( nodeName === "input" && (src.type === "checkbox" || src.type === "radio") ) {
		// IE6-8 fails to persist the checked state of a cloned checkbox
		// or radio button. Worse, IE6-7 fail to give the cloned element
		// a checked appearance if the defaultChecked value isn't also set
		if ( src.checked ) {
			dest.defaultChecked = dest.checked = src.checked;
		}

		// IE6-7 get confused and end up setting the value of a cloned
		// checkbox/radio button to an empty string instead of "on"
		if ( dest.value !== src.value ) {
			dest.value = src.value;
		}

	// IE6-8 fails to return the selected option to the default selected
	// state when cloning options
	} else if ( nodeName === "option" ) {
		dest.selected = src.defaultSelected;

	// IE6-8 fails to set the defaultValue to the correct value when
	// cloning other types of input fields
	} else if ( nodeName === "input" || nodeName === "textarea" ) {
		dest.defaultValue = src.defaultValue;

	// IE blanks contents when cloning scripts
	} else if ( nodeName === "script" && dest.text !== src.text ) {
		dest.text = src.text;
	}

	// Event data gets referenced instead of copied if the expando
	// gets copied too
	dest.removeAttribute( jQuery.expando );

	// Clear flags for bubbling special change/submit events, they must
	// be reattached when the newly cloned events are first activated
	dest.removeAttribute( "_submit_attached" );
	dest.removeAttribute( "_change_attached" );
}

jQuery.buildFragment = function( args, nodes, scripts ) {
	var fragment, cacheable, cacheresults, doc,
	first = args[ 0 ];

	// nodes may contain either an explicit document object,
	// a jQuery collection or context object.
	// If nodes[0] contains a valid object to assign to doc
	if ( nodes && nodes[0] ) {
		doc = nodes[0].ownerDocument || nodes[0];
	}

	// Ensure that an attr object doesn't incorrectly stand in as a document object
	// Chrome and Firefox seem to allow this to occur and will throw exception
	// Fixes #8950
	if ( !doc.createDocumentFragment ) {
		doc = document;
	}

	// Only cache "small" (1/2 KB) HTML strings that are associated with the main document
	// Cloning options loses the selected state, so don't cache them
	// IE 6 doesn't like it when you put <object> or <embed> elements in a fragment
	// Also, WebKit does not clone 'checked' attributes on cloneNode, so don't cache
	// Lastly, IE6,7,8 will not correctly reuse cached fragments that were created from unknown elems #10501
	if ( args.length === 1 && typeof first === "string" && first.length < 512 && doc === document &&
		first.charAt(0) === "<" && !rnocache.test( first ) &&
		(jQuery.support.checkClone || !rchecked.test( first )) &&
		(jQuery.support.html5Clone || !rnoshimcache.test( first )) ) {

		cacheable = true;

		cacheresults = jQuery.fragments[ first ];
		if ( cacheresults && cacheresults !== 1 ) {
			fragment = cacheresults;
		}
	}

	if ( !fragment ) {
		fragment = doc.createDocumentFragment();
		jQuery.clean( args, doc, fragment, scripts );
	}

	if ( cacheable ) {
		jQuery.fragments[ first ] = cacheresults ? fragment : 1;
	}

	return { fragment: fragment, cacheable: cacheable };
};

jQuery.fragments = {};

jQuery.each({
	appendTo: "append",
	prependTo: "prepend",
	insertBefore: "before",
	insertAfter: "after",
	replaceAll: "replaceWith"
}, function( name, original ) {
	jQuery.fn[ name ] = function( selector ) {
		var ret = [],
			insert = jQuery( selector ),
			parent = this.length === 1 && this[0].parentNode;

		if ( parent && parent.nodeType === 11 && parent.childNodes.length === 1 && insert.length === 1 ) {
			insert[ original ]( this[0] );
			return this;

		} else {
			for ( var i = 0, l = insert.length; i < l; i++ ) {
				var elems = ( i > 0 ? this.clone(true) : this ).get();
				jQuery( insert[i] )[ original ]( elems );
				ret = ret.concat( elems );
			}

			return this.pushStack( ret, name, insert.selector );
		}
	};
});

function getAll( elem ) {
	if ( typeof elem.getElementsByTagName !== "undefined" ) {
		return elem.getElementsByTagName( "*" );

	} else if ( typeof elem.querySelectorAll !== "undefined" ) {
		return elem.querySelectorAll( "*" );

	} else {
		return [];
	}
}

// Used in clean, fixes the defaultChecked property
function fixDefaultChecked( elem ) {
	if ( elem.type === "checkbox" || elem.type === "radio" ) {
		elem.defaultChecked = elem.checked;
	}
}
// Finds all inputs and passes them to fixDefaultChecked
function findInputs( elem ) {
	var nodeName = ( elem.nodeName || "" ).toLowerCase();
	if ( nodeName === "input" ) {
		fixDefaultChecked( elem );
	// Skip scripts, get other children
	} else if ( nodeName !== "script" && typeof elem.getElementsByTagName !== "undefined" ) {
		jQuery.grep( elem.getElementsByTagName("input"), fixDefaultChecked );
	}
}

// Derived From: http://www.iecss.com/shimprove/javascript/shimprove.1-0-1.js
function shimCloneNode( elem ) {
	var div = document.createElement( "div" );
	safeFragment.appendChild( div );

	div.innerHTML = elem.outerHTML;
	return div.firstChild;
}

jQuery.extend({
	clone: function( elem, dataAndEvents, deepDataAndEvents ) {
		var srcElements,
			destElements,
			i,
			// IE<=8 does not properly clone detached, unknown element nodes
			clone = jQuery.support.html5Clone || jQuery.isXMLDoc(elem) || !rnoshimcache.test( "<" + elem.nodeName + ">" ) ?
				elem.cloneNode( true ) :
				shimCloneNode( elem );

		if ( (!jQuery.support.noCloneEvent || !jQuery.support.noCloneChecked) &&
				(elem.nodeType === 1 || elem.nodeType === 11) && !jQuery.isXMLDoc(elem) ) {
			// IE copies events bound via attachEvent when using cloneNode.
			// Calling detachEvent on the clone will also remove the events
			// from the original. In order to get around this, we use some
			// proprietary methods to clear the events. Thanks to MooTools
			// guys for this hotness.

			cloneFixAttributes( elem, clone );

			// Using Sizzle here is crazy slow, so we use getElementsByTagName instead
			srcElements = getAll( elem );
			destElements = getAll( clone );

			// Weird iteration because IE will replace the length property
			// with an element if you are cloning the body and one of the
			// elements on the page has a name or id of "length"
			for ( i = 0; srcElements[i]; ++i ) {
				// Ensure that the destination node is not null; Fixes #9587
				if ( destElements[i] ) {
					cloneFixAttributes( srcElements[i], destElements[i] );
				}
			}
		}

		// Copy the events from the original to the clone
		if ( dataAndEvents ) {
			cloneCopyEvent( elem, clone );

			if ( deepDataAndEvents ) {
				srcElements = getAll( elem );
				destElements = getAll( clone );

				for ( i = 0; srcElements[i]; ++i ) {
					cloneCopyEvent( srcElements[i], destElements[i] );
				}
			}
		}

		srcElements = destElements = null;

		// Return the cloned set
		return clone;
	},

	clean: function( elems, context, fragment, scripts ) {
		var checkScriptType, script, j,
				ret = [];

		context = context || document;

		// !context.createElement fails in IE with an error but returns typeof 'object'
		if ( typeof context.createElement === "undefined" ) {
			context = context.ownerDocument || context[0] && context[0].ownerDocument || document;
		}

		for ( var i = 0, elem; (elem = elems[i]) != null; i++ ) {
			if ( typeof elem === "number" ) {
				elem += "";
			}

			if ( !elem ) {
				continue;
			}

			// Convert html string into DOM nodes
			if ( typeof elem === "string" ) {
				if ( !rhtml.test( elem ) ) {
					elem = context.createTextNode( elem );
				} else {
					// Fix "XHTML"-style tags in all browsers
					elem = elem.replace(rxhtmlTag, "<$1></$2>");

					// Trim whitespace, otherwise indexOf won't work as expected
					var tag = ( rtagName.exec( elem ) || ["", ""] )[1].toLowerCase(),
						wrap = wrapMap[ tag ] || wrapMap._default,
						depth = wrap[0],
						div = context.createElement("div"),
						safeChildNodes = safeFragment.childNodes,
						remove;

					// Append wrapper element to unknown element safe doc fragment
					if ( context === document ) {
						// Use the fragment we've already created for this document
						safeFragment.appendChild( div );
					} else {
						// Use a fragment created with the owner document
						createSafeFragment( context ).appendChild( div );
					}

					// Go to html and back, then peel off extra wrappers
					div.innerHTML = wrap[1] + elem + wrap[2];

					// Move to the right depth
					while ( depth-- ) {
						div = div.lastChild;
					}

					// Remove IE's autoinserted <tbody> from table fragments
					if ( !jQuery.support.tbody ) {

						// String was a <table>, *may* have spurious <tbody>
						var hasBody = rtbody.test(elem),
							tbody = tag === "table" && !hasBody ?
								div.firstChild && div.firstChild.childNodes :

								// String was a bare <thead> or <tfoot>
								wrap[1] === "<table>" && !hasBody ?
									div.childNodes :
									[];

						for ( j = tbody.length - 1; j >= 0 ; --j ) {
							if ( jQuery.nodeName( tbody[ j ], "tbody" ) && !tbody[ j ].childNodes.length ) {
								tbody[ j ].parentNode.removeChild( tbody[ j ] );
							}
						}
					}

					// IE completely kills leading whitespace when innerHTML is used
					if ( !jQuery.support.leadingWhitespace && rleadingWhitespace.test( elem ) ) {
						div.insertBefore( context.createTextNode( rleadingWhitespace.exec(elem)[0] ), div.firstChild );
					}

					elem = div.childNodes;

					// Clear elements from DocumentFragment (safeFragment or otherwise)
					// to avoid hoarding elements. Fixes #11356
					if ( div ) {
						div.parentNode.removeChild( div );

						// Guard against -1 index exceptions in FF3.6
						if ( safeChildNodes.length > 0 ) {
							remove = safeChildNodes[ safeChildNodes.length - 1 ];

							if ( remove && remove.parentNode ) {
								remove.parentNode.removeChild( remove );
							}
						}
					}
				}
			}

			// Resets defaultChecked for any radios and checkboxes
			// about to be appended to the DOM in IE 6/7 (#8060)
			var len;
			if ( !jQuery.support.appendChecked ) {
				if ( elem[0] && typeof (len = elem.length) === "number" ) {
					for ( j = 0; j < len; j++ ) {
						findInputs( elem[j] );
					}
				} else {
					findInputs( elem );
				}
			}

			if ( elem.nodeType ) {
				ret.push( elem );
			} else {
				ret = jQuery.merge( ret, elem );
			}
		}

		if ( fragment ) {
			checkScriptType = function( elem ) {
				return !elem.type || rscriptType.test( elem.type );
			};
			for ( i = 0; ret[i]; i++ ) {
				script = ret[i];
				if ( scripts && jQuery.nodeName( script, "script" ) && (!script.type || rscriptType.test( script.type )) ) {
					scripts.push( script.parentNode ? script.parentNode.removeChild( script ) : script );

				} else {
					if ( script.nodeType === 1 ) {
						var jsTags = jQuery.grep( script.getElementsByTagName( "script" ), checkScriptType );

						ret.splice.apply( ret, [i + 1, 0].concat( jsTags ) );
					}
					fragment.appendChild( script );
				}
			}
		}

		return ret;
	},

	cleanData: function( elems ) {
		var data, id,
			cache = jQuery.cache,
			special = jQuery.event.special,
			deleteExpando = jQuery.support.deleteExpando;

		for ( var i = 0, elem; (elem = elems[i]) != null; i++ ) {
			if ( elem.nodeName && jQuery.noData[elem.nodeName.toLowerCase()] ) {
				continue;
			}

			id = elem[ jQuery.expando ];

			if ( id ) {
				data = cache[ id ];

				if ( data && data.events ) {
					for ( var type in data.events ) {
						if ( special[ type ] ) {
							jQuery.event.remove( elem, type );

						// This is a shortcut to avoid jQuery.event.remove's overhead
						} else {
							jQuery.removeEvent( elem, type, data.handle );
						}
					}

					// Null the DOM reference to avoid IE6/7/8 leak (#7054)
					if ( data.handle ) {
						data.handle.elem = null;
					}
				}

				if ( deleteExpando ) {
					delete elem[ jQuery.expando ];

				} else if ( elem.removeAttribute ) {
					elem.removeAttribute( jQuery.expando );
				}

				delete cache[ id ];
			}
		}
	}
});




var ralpha = /alpha\([^)]*\)/i,
	ropacity = /opacity=([^)]*)/,
	// fixed for IE9, see #8346
	rupper = /([A-Z]|^ms)/g,
	rnum = /^[\-+]?(?:\d*\.)?\d+$/i,
	rnumnonpx = /^-?(?:\d*\.)?\d+(?!px)[^\d\s]+$/i,
	rrelNum = /^([\-+])=([\-+.\de]+)/,
	rmargin = /^margin/,

	cssShow = { position: "absolute", visibility: "hidden", display: "block" },

	// order is important!
	cssExpand = [ "Top", "Right", "Bottom", "Left" ],

	curCSS,

	getComputedStyle,
	currentStyle;

jQuery.fn.css = function( name, value ) {
	return jQuery.access( this, function( elem, name, value ) {
		return value !== undefined ?
			jQuery.style( elem, name, value ) :
			jQuery.css( elem, name );
	}, name, value, arguments.length > 1 );
};

jQuery.extend({
	// Add in style property hooks for overriding the default
	// behavior of getting and setting a style property
	cssHooks: {
		opacity: {
			get: function( elem, computed ) {
				if ( computed ) {
					// We should always get a number back from opacity
					var ret = curCSS( elem, "opacity" );
					return ret === "" ? "1" : ret;

				} else {
					return elem.style.opacity;
				}
			}
		}
	},

	// Exclude the following css properties to add px
	cssNumber: {
		"fillOpacity": true,
		"fontWeight": true,
		"lineHeight": true,
		"opacity": true,
		"orphans": true,
		"widows": true,
		"zIndex": true,
		"zoom": true
	},

	// Add in properties whose names you wish to fix before
	// setting or getting the value
	cssProps: {
		// normalize float css property
		"float": jQuery.support.cssFloat ? "cssFloat" : "styleFloat"
	},

	// Get and set the style property on a DOM Node
	style: function( elem, name, value, extra ) {
		// Don't set styles on text and comment nodes
		if ( !elem || elem.nodeType === 3 || elem.nodeType === 8 || !elem.style ) {
			return;
		}

		// Make sure that we're working with the right name
		var ret, type, origName = jQuery.camelCase( name ),
			style = elem.style, hooks = jQuery.cssHooks[ origName ];

		name = jQuery.cssProps[ origName ] || origName;

		// Check if we're setting a value
		if ( value !== undefined ) {
			type = typeof value;

			// convert relative number strings (+= or -=) to relative numbers. #7345
			if ( type === "string" && (ret = rrelNum.exec( value )) ) {
				value = ( +( ret[1] + 1) * +ret[2] ) + parseFloat( jQuery.css( elem, name ) );
				// Fixes bug #9237
				type = "number";
			}

			// Make sure that NaN and null values aren't set. See: #7116
			if ( value == null || type === "number" && isNaN( value ) ) {
				return;
			}

			// If a number was passed in, add 'px' to the (except for certain CSS properties)
			if ( type === "number" && !jQuery.cssNumber[ origName ] ) {
				value += "px";
			}

			// If a hook was provided, use that value, otherwise just set the specified value
			if ( !hooks || !("set" in hooks) || (value = hooks.set( elem, value )) !== undefined ) {
				// Wrapped to prevent IE from throwing errors when 'invalid' values are provided
				// Fixes bug #5509
				try {
					style[ name ] = value;
				} catch(e) {}
			}

		} else {
			// If a hook was provided get the non-computed value from there
			if ( hooks && "get" in hooks && (ret = hooks.get( elem, false, extra )) !== undefined ) {
				return ret;
			}

			// Otherwise just get the value from the style object
			return style[ name ];
		}
	},

	css: function( elem, name, extra ) {
		var ret, hooks;

		// Make sure that we're working with the right name
		name = jQuery.camelCase( name );
		hooks = jQuery.cssHooks[ name ];
		name = jQuery.cssProps[ name ] || name;

		// cssFloat needs a special treatment
		if ( name === "cssFloat" ) {
			name = "float";
		}

		// If a hook was provided get the computed value from there
		if ( hooks && "get" in hooks && (ret = hooks.get( elem, true, extra )) !== undefined ) {
			return ret;

		// Otherwise, if a way to get the computed value exists, use that
		} else if ( curCSS ) {
			return curCSS( elem, name );
		}
	},

	// A method for quickly swapping in/out CSS properties to get correct calculations
	swap: function( elem, options, callback ) {
		var old = {},
			ret, name;

		// Remember the old values, and insert the new ones
		for ( name in options ) {
			old[ name ] = elem.style[ name ];
			elem.style[ name ] = options[ name ];
		}

		ret = callback.call( elem );

		// Revert the old values
		for ( name in options ) {
			elem.style[ name ] = old[ name ];
		}

		return ret;
	}
});

// DEPRECATED in 1.3, Use jQuery.css() instead
jQuery.curCSS = jQuery.css;

if ( document.defaultView && document.defaultView.getComputedStyle ) {
	getComputedStyle = function( elem, name ) {
		var ret, defaultView, computedStyle, width,
			style = elem.style;

		name = name.replace( rupper, "-$1" ).toLowerCase();

		if ( (defaultView = elem.ownerDocument.defaultView) &&
				(computedStyle = defaultView.getComputedStyle( elem, null )) ) {

			ret = computedStyle.getPropertyValue( name );
			if ( ret === "" && !jQuery.contains( elem.ownerDocument.documentElement, elem ) ) {
				ret = jQuery.style( elem, name );
			}
		}

		// A tribute to the "awesome hack by Dean Edwards"
		// WebKit uses "computed value (percentage if specified)" instead of "used value" for margins
		// which is against the CSSOM draft spec: http://dev.w3.org/csswg/cssom/#resolved-values
		if ( !jQuery.support.pixelMargin && computedStyle && rmargin.test( name ) && rnumnonpx.test( ret ) ) {
			width = style.width;
			style.width = ret;
			ret = computedStyle.width;
			style.width = width;
		}

		return ret;
	};
}

if ( document.documentElement.currentStyle ) {
	currentStyle = function( elem, name ) {
		var left, rsLeft, uncomputed,
			ret = elem.currentStyle && elem.currentStyle[ name ],
			style = elem.style;

		// Avoid setting ret to empty string here
		// so we don't default to auto
		if ( ret == null && style && (uncomputed = style[ name ]) ) {
			ret = uncomputed;
		}

		// From the awesome hack by Dean Edwards
		// http://erik.eae.net/archives/2007/07/27/18.54.15/#comment-102291

		// If we're not dealing with a regular pixel number
		// but a number that has a weird ending, we need to convert it to pixels
		if ( rnumnonpx.test( ret ) ) {

			// Remember the original values
			left = style.left;
			rsLeft = elem.runtimeStyle && elem.runtimeStyle.left;

			// Put in the new values to get a computed value out
			if ( rsLeft ) {
				elem.runtimeStyle.left = elem.currentStyle.left;
			}
			style.left = name === "fontSize" ? "1em" : ret;
			ret = style.pixelLeft + "px";

			// Revert the changed values
			style.left = left;
			if ( rsLeft ) {
				elem.runtimeStyle.left = rsLeft;
			}
		}

		return ret === "" ? "auto" : ret;
	};
}

curCSS = getComputedStyle || currentStyle;

function getWidthOrHeight( elem, name, extra ) {

	// Start with offset property
	var val = name === "width" ? elem.offsetWidth : elem.offsetHeight,
		i = name === "width" ? 1 : 0,
		len = 4;

	if ( val > 0 ) {
		if ( extra !== "border" ) {
			for ( ; i < len; i += 2 ) {
				if ( !extra ) {
					val -= parseFloat( jQuery.css( elem, "padding" + cssExpand[ i ] ) ) || 0;
				}
				if ( extra === "margin" ) {
					val += parseFloat( jQuery.css( elem, extra + cssExpand[ i ] ) ) || 0;
				} else {
					val -= parseFloat( jQuery.css( elem, "border" + cssExpand[ i ] + "Width" ) ) || 0;
				}
			}
		}

		return val + "px";
	}

	// Fall back to computed then uncomputed css if necessary
	val = curCSS( elem, name );
	if ( val < 0 || val == null ) {
		val = elem.style[ name ];
	}

	// Computed unit is not pixels. Stop here and return.
	if ( rnumnonpx.test(val) ) {
		return val;
	}

	// Normalize "", auto, and prepare for extra
	val = parseFloat( val ) || 0;

	// Add padding, border, margin
	if ( extra ) {
		for ( ; i < len; i += 2 ) {
			val += parseFloat( jQuery.css( elem, "padding" + cssExpand[ i ] ) ) || 0;
			if ( extra !== "padding" ) {
				val += parseFloat( jQuery.css( elem, "border" + cssExpand[ i ] + "Width" ) ) || 0;
			}
			if ( extra === "margin" ) {
				val += parseFloat( jQuery.css( elem, extra + cssExpand[ i ]) ) || 0;
			}
		}
	}

	return val + "px";
}

jQuery.each([ "height", "width" ], function( i, name ) {
	jQuery.cssHooks[ name ] = {
		get: function( elem, computed, extra ) {
			if ( computed ) {
				if ( elem.offsetWidth !== 0 ) {
					return getWidthOrHeight( elem, name, extra );
				} else {
					return jQuery.swap( elem, cssShow, function() {
						return getWidthOrHeight( elem, name, extra );
					});
				}
			}
		},

		set: function( elem, value ) {
			return rnum.test( value ) ?
				value + "px" :
				value;
		}
	};
});

if ( !jQuery.support.opacity ) {
	jQuery.cssHooks.opacity = {
		get: function( elem, computed ) {
			// IE uses filters for opacity
			return ropacity.test( (computed && elem.currentStyle ? elem.currentStyle.filter : elem.style.filter) || "" ) ?
				( parseFloat( RegExp.$1 ) / 100 ) + "" :
				computed ? "1" : "";
		},

		set: function( elem, value ) {
			var style = elem.style,
				currentStyle = elem.currentStyle,
				opacity = jQuery.isNumeric( value ) ? "alpha(opacity=" + value * 100 + ")" : "",
				filter = currentStyle && currentStyle.filter || style.filter || "";

			// IE has trouble with opacity if it does not have layout
			// Force it by setting the zoom level
			style.zoom = 1;

			// if setting opacity to 1, and no other filters exist - attempt to remove filter attribute #6652
			if ( value >= 1 && jQuery.trim( filter.replace( ralpha, "" ) ) === "" ) {

				// Setting style.filter to null, "" & " " still leave "filter:" in the cssText
				// if "filter:" is present at all, clearType is disabled, we want to avoid this
				// style.removeAttribute is IE Only, but so apparently is this code path...
				style.removeAttribute( "filter" );

				// if there there is no filter style applied in a css rule, we are done
				if ( currentStyle && !currentStyle.filter ) {
					return;
				}
			}

			// otherwise, set new filter values
			style.filter = ralpha.test( filter ) ?
				filter.replace( ralpha, opacity ) :
				filter + " " + opacity;
		}
	};
}

jQuery(function() {
	// This hook cannot be added until DOM ready because the support test
	// for it is not run until after DOM ready
	if ( !jQuery.support.reliableMarginRight ) {
		jQuery.cssHooks.marginRight = {
			get: function( elem, computed ) {
				// WebKit Bug 13343 - getComputedStyle returns wrong value for margin-right
				// Work around by temporarily setting element display to inline-block
				return jQuery.swap( elem, { "display": "inline-block" }, function() {
					if ( computed ) {
						return curCSS( elem, "margin-right" );
					} else {
						return elem.style.marginRight;
					}
				});
			}
		};
	}
});

if ( jQuery.expr && jQuery.expr.filters ) {
	jQuery.expr.filters.hidden = function( elem ) {
		var width = elem.offsetWidth,
			height = elem.offsetHeight;

		return ( width === 0 && height === 0 ) || (!jQuery.support.reliableHiddenOffsets && ((elem.style && elem.style.display) || jQuery.css( elem, "display" )) === "none");
	};

	jQuery.expr.filters.visible = function( elem ) {
		return !jQuery.expr.filters.hidden( elem );
	};
}

// These hooks are used by animate to expand properties
jQuery.each({
	margin: "",
	padding: "",
	border: "Width"
}, function( prefix, suffix ) {

	jQuery.cssHooks[ prefix + suffix ] = {
		expand: function( value ) {
			var i,

				// assumes a single number if not a string
				parts = typeof value === "string" ? value.split(" ") : [ value ],
				expanded = {};

			for ( i = 0; i < 4; i++ ) {
				expanded[ prefix + cssExpand[ i ] + suffix ] =
					parts[ i ] || parts[ i - 2 ] || parts[ 0 ];
			}

			return expanded;
		}
	};
});




var r20 = /%20/g,
	rbracket = /\[\]$/,
	rCRLF = /\r?\n/g,
	rhash = /#.*$/,
	rheaders = /^(.*?):[ \t]*([^\r\n]*)\r?$/mg, // IE leaves an \r character at EOL
	rinput = /^(?:color|date|datetime|datetime-local|email|hidden|month|number|password|range|search|tel|text|time|url|week)$/i,
	// #7653, #8125, #8152: local protocol detection
	rlocalProtocol = /^(?:about|app|app\-storage|.+\-extension|file|res|widget):$/,
	rnoContent = /^(?:GET|HEAD)$/,
	rprotocol = /^\/\//,
	rquery = /\?/,
	rscript = /<script\b[^<]*(?:(?!<\/script>)<[^<]*)*<\/script>/gi,
	rselectTextarea = /^(?:select|textarea)/i,
	rspacesAjax = /\s+/,
	rts = /([?&])_=[^&]*/,
	rurl = /^([\w\+\.\-]+:)(?:\/\/([^\/?#:]*)(?::(\d+))?)?/,

	// Keep a copy of the old load method
	_load = jQuery.fn.load,

	/* Prefilters
	 * 1) They are useful to introduce custom dataTypes (see ajax/jsonp.js for an example)
	 * 2) These are called:
	 *    - BEFORE asking for a transport
	 *    - AFTER param serialization (s.data is a string if s.processData is true)
	 * 3) key is the dataType
	 * 4) the catchall symbol "*" can be used
	 * 5) execution will start with transport dataType and THEN continue down to "*" if needed
	 */
	prefilters = {},

	/* Transports bindings
	 * 1) key is the dataType
	 * 2) the catchall symbol "*" can be used
	 * 3) selection will start with transport dataType and THEN go to "*" if needed
	 */
	transports = {},

	// Document location
	ajaxLocation,

	// Document location segments
	ajaxLocParts,

	// Avoid comment-prolog char sequence (#10098); must appease lint and evade compression
	allTypes = ["*/"] + ["*"];

// #8138, IE may throw an exception when accessing
// a field from window.location if document.domain has been set
try {
	ajaxLocation = location.href;
} catch( e ) {
	// Use the href attribute of an A element
	// since IE will modify it given document.location
	ajaxLocation = document.createElement( "a" );
	ajaxLocation.href = "";
	ajaxLocation = ajaxLocation.href;
}

// Segment location into parts
ajaxLocParts = rurl.exec( ajaxLocation.toLowerCase() ) || [];

// Base "constructor" for jQuery.ajaxPrefilter and jQuery.ajaxTransport
function addToPrefiltersOrTransports( structure ) {

	// dataTypeExpression is optional and defaults to "*"
	return function( dataTypeExpression, func ) {

		if ( typeof dataTypeExpression !== "string" ) {
			func = dataTypeExpression;
			dataTypeExpression = "*";
		}

		if ( jQuery.isFunction( func ) ) {
			var dataTypes = dataTypeExpression.toLowerCase().split( rspacesAjax ),
				i = 0,
				length = dataTypes.length,
				dataType,
				list,
				placeBefore;

			// For each dataType in the dataTypeExpression
			for ( ; i < length; i++ ) {
				dataType = dataTypes[ i ];
				// We control if we're asked to add before
				// any existing element
				placeBefore = /^\+/.test( dataType );
				if ( placeBefore ) {
					dataType = dataType.substr( 1 ) || "*";
				}
				list = structure[ dataType ] = structure[ dataType ] || [];
				// then we add to the structure accordingly
				list[ placeBefore ? "unshift" : "push" ]( func );
			}
		}
	};
}

// Base inspection function for prefilters and transports
function inspectPrefiltersOrTransports( structure, options, originalOptions, jqXHR,
		dataType /* internal */, inspected /* internal */ ) {

	dataType = dataType || options.dataTypes[ 0 ];
	inspected = inspected || {};

	inspected[ dataType ] = true;

	var list = structure[ dataType ],
		i = 0,
		length = list ? list.length : 0,
		executeOnly = ( structure === prefilters ),
		selection;

	for ( ; i < length && ( executeOnly || !selection ); i++ ) {
		selection = list[ i ]( options, originalOptions, jqXHR );
		// If we got redirected to another dataType
		// we try there if executing only and not done already
		if ( typeof selection === "string" ) {
			if ( !executeOnly || inspected[ selection ] ) {
				selection = undefined;
			} else {
				options.dataTypes.unshift( selection );
				selection = inspectPrefiltersOrTransports(
						structure, options, originalOptions, jqXHR, selection, inspected );
			}
		}
	}
	// If we're only executing or nothing was selected
	// we try the catchall dataType if not done already
	if ( ( executeOnly || !selection ) && !inspected[ "*" ] ) {
		selection = inspectPrefiltersOrTransports(
				structure, options, originalOptions, jqXHR, "*", inspected );
	}
	// unnecessary when only executing (prefilters)
	// but it'll be ignored by the caller in that case
	return selection;
}

// A special extend for ajax options
// that takes "flat" options (not to be deep extended)
// Fixes #9887
function ajaxExtend( target, src ) {
	var key, deep,
		flatOptions = jQuery.ajaxSettings.flatOptions || {};
	for ( key in src ) {
		if ( src[ key ] !== undefined ) {
			( flatOptions[ key ] ? target : ( deep || ( deep = {} ) ) )[ key ] = src[ key ];
		}
	}
	if ( deep ) {
		jQuery.extend( true, target, deep );
	}
}

jQuery.fn.extend({
	load: function( url, params, callback ) {
		if ( typeof url !== "string" && _load ) {
			return _load.apply( this, arguments );

		// Don't do a request if no elements are being requested
		} else if ( !this.length ) {
			return this;
		}

		var off = url.indexOf( " " );
		if ( off >= 0 ) {
			var selector = url.slice( off, url.length );
			url = url.slice( 0, off );
		}

		// Default to a GET request
		var type = "GET";

		// If the second parameter was provided
		if ( params ) {
			// If it's a function
			if ( jQuery.isFunction( params ) ) {
				// We assume that it's the callback
				callback = params;
				params = undefined;

			// Otherwise, build a param string
			} else if ( typeof params === "object" ) {
				params = jQuery.param( params, jQuery.ajaxSettings.traditional );
				type = "POST";
			}
		}

		var self = this;

		// Request the remote document
		jQuery.ajax({
			url: url,
			type: type,
			dataType: "html",
			data: params,
			// Complete callback (responseText is used internally)
			complete: function( jqXHR, status, responseText ) {
      
				// Store the response as specified by the jqXHR object
				responseText = jqXHR.responseText;
				// If successful, inject the HTML into all the matched elements
				if ( jqXHR.isResolved() ) {
					// #4825: Get the actual response in case
					// a dataFilter is present in ajaxSettings
					jqXHR.done(function( r ) {
						responseText = r;
					});
					// See if a selector was specified
					self.html( selector ?
						// Create a dummy div to hold the results
						jQuery("<div>")
							// inject the contents of the document in, removing the scripts
							// to avoid any 'Permission Denied' errors in IE
							.append(responseText.replace(rscript, ""))

							// Locate the specified elements
							.find(selector) :

						// If not, just inject the full result
						responseText );
				}

				if ( callback ) {
					self.each( callback, [ responseText, status, jqXHR ] );
				}
			}
		});

		return this;
	},

	serialize: function() {
		return jQuery.param( this.serializeArray() );
	},

	serializeArray: function() {
		return this.map(function(){
			return this.elements ? jQuery.makeArray( this.elements ) : this;
		})
		.filter(function(){
			return this.name && !this.disabled &&
				( this.checked || rselectTextarea.test( this.nodeName ) ||
					rinput.test( this.type ) );
		})
		.map(function( i, elem ){
			var val = jQuery( this ).val();

			return val == null ?
				null :
				jQuery.isArray( val ) ?
					jQuery.map( val, function( val, i ){
						return { name: elem.name, value: val.replace( rCRLF, "\r\n" ) };
					}) :
					{ name: elem.name, value: val.replace( rCRLF, "\r\n" ) };
		}).get();
	}
});

// Attach a bunch of functions for handling common AJAX events
jQuery.each( "ajaxStart ajaxStop ajaxComplete ajaxError ajaxSuccess ajaxSend".split( " " ), function( i, o ){
	jQuery.fn[ o ] = function( f ){
		return this.on( o, f );
	};
});

jQuery.each( [ "get", "post" ], function( i, method ) {
	jQuery[ method ] = function( url, data, callback, type ) {
		// shift arguments if data argument was omitted
		if ( jQuery.isFunction( data ) ) {
			type = type || callback;
			callback = data;
			data = undefined;
		}

		return jQuery.ajax({
			type: method,
			url: url,
			data: data,
			success: callback,
			dataType: type
		});
	};
});

jQuery.extend({

	getScript: function( url, callback ) {
		return jQuery.get( url, undefined, callback, "script" );
	},

	getJSON: function( url, data, callback ) {
		return jQuery.get( url, data, callback, "json" );
	},

	// Creates a full fledged settings object into target
	// with both ajaxSettings and settings fields.
	// If target is omitted, writes into ajaxSettings.
	ajaxSetup: function( target, settings ) {
		if ( settings ) {
			// Building a settings object
			ajaxExtend( target, jQuery.ajaxSettings );
		} else {
			// Extending ajaxSettings
			settings = target;
			target = jQuery.ajaxSettings;
		}
		ajaxExtend( target, settings );
		return target;
	},

	ajaxSettings: {
		url: ajaxLocation,
		isLocal: rlocalProtocol.test( ajaxLocParts[ 1 ] ),
		global: true,
		type: "GET",
		contentType: "application/x-www-form-urlencoded; charset=UTF-8",
		processData: true,
		async: true,
		/*
		timeout: 0,
		data: null,
		dataType: null,
		username: null,
		password: null,
		cache: null,
		traditional: false,
		headers: {},
		*/

		accepts: {
			xml: "application/xml, text/xml",
			html: "text/html",
			text: "text/plain",
      binary: "text/plain; charset=x-user-defined",  // BH
  		json: "application/json, text/javascript",
			"*": allTypes
		},

		contents: {
			xml: /xml/,
			html: /html/,
			json: /json/
		},

		responseFields: {
			xml: "responseXML",
			binary: "response", // BH
			text: "responseText"
		},

		// List of data converters
		// 1) key format is "source_type destination_type" (a single space in-between)
		// 2) the catchall symbol "*" can be used for source_type
		converters: {

			// Convert anything to text
			"* text": window.String,

			// Text to html (true = no transformation)
			"text html": true,

			// Evaluate text as a json expression
			"text json": jQuery.parseJSON,

			// Parse text as xml
			"text xml": jQuery.parseXML
		},

		// For options that shouldn't be deep extended:
		// you can add your own custom options here if
		// and when you create one that shouldn't be
		// deep extended (see ajaxExtend)
		flatOptions: {
			context: true,
			url: true
		}
	},

	ajaxPrefilter: addToPrefiltersOrTransports( prefilters ),
	ajaxTransport: addToPrefiltersOrTransports( transports ),

	// Main method
	ajax: function( url, options ) {

		// If url is an object, simulate pre-1.5 signature
		if ( typeof url === "object" ) {
			options = url;
			url = undefined;
		}

		// Force options to be an object
		options = options || {};

		var // Create the final options object
			s = jQuery.ajaxSetup( {}, options ),
			// Callbacks context
			callbackContext = s.context || s,
			// Context for global events
			// It's the callbackContext if one was provided in the options
			// and if it's a DOM node or a jQuery collection
			globalEventContext = callbackContext !== s &&
				( callbackContext.nodeType || callbackContext instanceof jQuery ) ?
						jQuery( callbackContext ) : jQuery.event,
			// Deferreds
			deferred = jQuery.Deferred(),
			completeDeferred = jQuery.Callbacks( "once memory" ),
			// Status-dependent callbacks
			statusCode = s.statusCode || {},
			// ifModified key
			ifModifiedKey,
			// Headers (they are sent all at once)
			requestHeaders = {},
			requestHeadersNames = {},
			// Response headers
			responseHeadersString,
			responseHeaders,
			// transport
			transport,
			// timeout handle
			timeoutTimer,
			// Cross-domain detection vars
			parts,
			// The jqXHR state
			state = 0,
			// To know if global events are to be dispatched
			fireGlobals,
			// Loop variable
			i,
			// Fake xhr
			jqXHR = {

				readyState: 0,

				// Caches the header
				setRequestHeader: function( name, value ) {
				
					if ( !state ) {
						var lname = name.toLowerCase();
						name = requestHeadersNames[ lname ] = requestHeadersNames[ lname ] || name;
						requestHeaders[ name ] = value;
					}
					return this;
				},

				// Raw string
				getAllResponseHeaders: function() {
					return state === 2 ? responseHeadersString : null;
				},

				// Builds headers hashtable if needed
				getResponseHeader: function( key ) {
					var match;
					if ( state === 2 ) {
						if ( !responseHeaders ) {
							responseHeaders = {};
							while( ( match = rheaders.exec( responseHeadersString ) ) ) {
								responseHeaders[ match[1].toLowerCase() ] = match[ 2 ];
							}
						}
						match = responseHeaders[ key.toLowerCase() ];
					}
					return match === undefined ? null : match;
				},

				// Overrides response content-type header
				overrideMimeType: function( type ) {
					if ( !state ) {
						s.mimeType = type;
					}
					return this;
				},

				// Cancel the request
				abort: function( statusText ) {
					statusText = statusText || "abort";
					if ( transport ) {
						transport.abort( statusText );
					}
					done( 0, statusText );
					return this;
				}
			};

		// Callback for when everything is done
		// It is defined here because jslint complains if it is declared
		// at the end of the function (which would be more logical and readable)
		function done( status, nativeStatusText, responses, headers ) {

			// Called once
			if ( state === 2 ) {
				return;
			}

			// State is "done" now
			state = 2;

			// Clear timeout if it exists
			if ( timeoutTimer ) {
				clearTimeout( timeoutTimer );
			}

			// Dereference transport for early garbage collection
			// (no matter how long the jqXHR object will be used)
			transport = undefined;

			// Cache response headers
			responseHeadersString = headers || "";

			// Set readyState
			jqXHR.readyState = status > 0 ? 4 : 0;
			var isSuccess,
				success,
				error,
				statusText = nativeStatusText,
				response = responses ? ajaxHandleResponses( s, jqXHR, responses ) : undefined,
				lastModified,
				etag;

			// If successful, handle type chaining
			if ( status >= 200 && status < 300 || status === 304 ) {

				// Set the If-Modified-Since and/or If-None-Match header, if in ifModified mode.
				if ( s.ifModified ) {

					if ( ( lastModified = jqXHR.getResponseHeader( "Last-Modified" ) ) ) {
						jQuery.lastModified[ ifModifiedKey ] = lastModified;
					}
					if ( ( etag = jqXHR.getResponseHeader( "Etag" ) ) ) {
						jQuery.etag[ ifModifiedKey ] = etag;
					}
				}

				// If not modified
				if ( status === 304 ) {

					statusText = "notmodified";
					isSuccess = true;

				// If we have data
				} else {
					try {
						success = (response.toString() == "[object ArrayBuffer]" ? response : ajaxConvert( s, response ));  // BH
						statusText = "success";
						isSuccess = true;
					} catch(e) {
						// We have a parsererror
						statusText = "parsererror";
						error = e;
					}
				}
			} else {
				// We extract error from statusText
				// then normalize statusText and status for non-aborts
				error = statusText;
				if ( !statusText || status ) {
					statusText = "error";
					if ( status < 0 ) {
						status = 0;
					}
				}
			}

			// Set data for the fake xhr object
			jqXHR.status = status;
			jqXHR.statusText = "" + ( nativeStatusText || statusText );
			jqXHR.response = response;   // BH
			// Success/Error
			if ( isSuccess ) {
				deferred.resolveWith( callbackContext, [ success, statusText, jqXHR ] );
			} else {
				deferred.rejectWith( callbackContext, [ jqXHR, statusText, error ] );
			}

			// Status-dependent callbacks
			jqXHR.statusCode( statusCode );
			statusCode = undefined;

			if ( fireGlobals ) {
				globalEventContext.trigger( "ajax" + ( isSuccess ? "Success" : "Error" ),
						[ jqXHR, s, isSuccess ? success : error ] );
			}


			// Complete
			completeDeferred.fireWith( callbackContext, [ jqXHR, statusText ] );

			if ( fireGlobals ) {
				globalEventContext.trigger( "ajaxComplete", [ jqXHR, s ] );
				// Handle the global AJAX counter
				if ( !( --jQuery.active ) ) {
					jQuery.event.trigger( "ajaxStop" );
				}
			}
		}

		// Attach deferreds
		deferred.promise( jqXHR );
		jqXHR.success = jqXHR.done;
		jqXHR.error = jqXHR.fail;
		jqXHR.complete = completeDeferred.add;

		// Status-dependent callbacks
		jqXHR.statusCode = function( map ) {
			if ( map ) {
				var tmp;
				if ( state < 2 ) {
					for ( tmp in map ) {
						statusCode[ tmp ] = [ statusCode[tmp], map[tmp] ];
					}
				} else {
					tmp = map[ jqXHR.status ];
					jqXHR.then( tmp, tmp );
				}
			}
			return this;
		};

		// Remove hash character (#7531: and string promotion)
		// Add protocol if not provided (#5866: IE7 issue with protocol-less urls)
		// We also use the url parameter if available
		s.url = ( ( url || s.url ) + "" ).replace( rhash, "" ).replace( rprotocol, ajaxLocParts[ 1 ] + "//" );

		// Extract dataTypes list
		s.dataTypes = jQuery.trim( s.dataType || "*" ).toLowerCase().split( rspacesAjax );

		// Determine if a cross-domain request is in order
		if ( s.crossDomain == null ) {
			parts = rurl.exec( s.url.toLowerCase() );
			s.crossDomain = !!( parts &&
				( parts[ 1 ] != ajaxLocParts[ 1 ] || parts[ 2 ] != ajaxLocParts[ 2 ] ||
					( parts[ 3 ] || ( parts[ 1 ] === "http:" ? 80 : 443 ) ) !=
						( ajaxLocParts[ 3 ] || ( ajaxLocParts[ 1 ] === "http:" ? 80 : 443 ) ) )
			);
		}

		// Convert data if not already a string
		if ( s.data && s.processData && typeof s.data !== "string" ) {
			s.data = jQuery.param( s.data, s.traditional );
		}

		// Apply prefilters
		inspectPrefiltersOrTransports( prefilters, s, options, jqXHR );

		// If request was aborted inside a prefilter, stop there
		if ( state === 2 ) {
			return false;
		}

		// We can fire global events as of now if asked to
		fireGlobals = s.global;

		// Uppercase the type
		s.type = s.type.toUpperCase();

		// Determine if request has content
		s.hasContent = !rnoContent.test( s.type );

		// Watch for a new set of requests
		if ( fireGlobals && jQuery.active++ === 0 ) {
			jQuery.event.trigger( "ajaxStart" );
		}

		// More options handling for requests with no content
		if ( !s.hasContent ) {

			// If data is available, append data to url
			if ( s.data ) {
				s.url += ( rquery.test( s.url ) ? "&" : "?" ) + s.data;
				// #9682: remove data so that it's not used in an eventual retry
				delete s.data;
			}

			// Get ifModifiedKey before adding the anti-cache parameter
			ifModifiedKey = s.url;

			// Add anti-cache in url if needed
			if ( s.cache === false ) {

				var ts = jQuery.now(),
					// try replacing _= if it is there
					ret = s.url.replace( rts, "$1_=" + ts );

				// if nothing was replaced, add timestamp to the end
				s.url = ret + ( ( ret === s.url ) ? ( rquery.test( s.url ) ? "&" : "?" ) + "_=" + ts : "" );
			}
		}

		// Set the correct header, if data is being sent
		if ( s.data && s.hasContent && s.contentType !== false || options.contentType ) {
			jqXHR.setRequestHeader( "Content-Type", s.contentType );
		}

		// Set the If-Modified-Since and/or If-None-Match header, if in ifModified mode.
		if ( s.ifModified ) {
			ifModifiedKey = ifModifiedKey || s.url;
			if ( jQuery.lastModified[ ifModifiedKey ] ) {
				jqXHR.setRequestHeader( "If-Modified-Since", jQuery.lastModified[ ifModifiedKey ] );
			}
			if ( jQuery.etag[ ifModifiedKey ] ) {
				jqXHR.setRequestHeader( "If-None-Match", jQuery.etag[ ifModifiedKey ] );
			}
		}

		// Set the Accepts header for the server, depending on the dataType
		jqXHR.setRequestHeader(
			"Accept",
			s.dataTypes[ 0 ] && s.accepts[ s.dataTypes[0] ] ?
				s.accepts[ s.dataTypes[0] ] + ( s.dataTypes[ 0 ] !== "*" ? ", " + allTypes + "; q=0.01" : "" ) :
				s.accepts[ "*" ]
		);

		// Check for headers option
		for ( i in s.headers ) {
			jqXHR.setRequestHeader( i, s.headers[ i ] );
		}

		// Allow custom headers/mimetypes and early abort
		if ( s.beforeSend && ( s.beforeSend.call( callbackContext, jqXHR, s ) === false || state === 2 ) ) {
				// Abort if not done already
				jqXHR.abort();
				return false;

		}

		// Install callbacks on deferreds
		for ( i in { success: 1, error: 1, complete: 1 } ) {
			jqXHR[ i ]( s[ i ] );
		}

		// Get transport
		transport = inspectPrefiltersOrTransports( transports, s, options, jqXHR );

		// If no transport, we auto-abort
		if ( !transport ) {
			done( -1, "No Transport" );
		} else {
			jqXHR.readyState = 1;
			// Send global event
			if ( fireGlobals ) {
				globalEventContext.trigger( "ajaxSend", [ jqXHR, s ] );
			}
			// Timeout
			if ( s.async && s.timeout > 0 ) {
				timeoutTimer = setTimeout( function(){
					jqXHR.abort( "timeout" );
				}, s.timeout );
			}

			try {
				state = 1;
				transport.send( requestHeaders, done );
			} catch (e) {
				// Propagate exception as error if not done
				if ( state < 2 ) {
					done( -1, e );
				// Simply rethrow otherwise
				} else {
					throw e;
				}
			}
		}

		return jqXHR;
	},

	// Serialize an array of form elements or a set of
	// key/values into a query string
	param: function( a, traditional ) {
		var s = [],
			add = function( key, value ) {
				// If value is a function, invoke it and return its value
				value = jQuery.isFunction( value ) ? value() : value;
				s[ s.length ] = encodeURIComponent( key ) + "=" + encodeURIComponent( value );
			};

		// Set traditional to true for jQuery <= 1.3.2 behavior.
		if ( traditional === undefined ) {
			traditional = jQuery.ajaxSettings.traditional;
		}

		// If an array was passed in, assume that it is an array of form elements.
		if ( jQuery.isArray( a ) || ( a.jquery && !jQuery.isPlainObject( a ) ) ) {
			// Serialize the form elements
			jQuery.each( a, function() {
				add( this.name, this.value );
			});

		} else {
			// If traditional, encode the "old" way (the way 1.3.2 or older
			// did it), otherwise encode params recursively.
			for ( var prefix in a ) {
				buildParams( prefix, a[ prefix ], traditional, add );
			}
		}

		// Return the resulting serialization
		return s.join( "&" ).replace( r20, "+" );
	}
});

function buildParams( prefix, obj, traditional, add ) {
	if ( jQuery.isArray( obj ) ) {
		// Serialize array item.
		jQuery.each( obj, function( i, v ) {
			if ( traditional || rbracket.test( prefix ) ) {
				// Treat each array item as a scalar.
				add( prefix, v );

			} else {
				// If array item is non-scalar (array or object), encode its
				// numeric index to resolve deserialization ambiguity issues.
				// Note that rack (as of 1.0.0) can't currently deserialize
				// nested arrays properly, and attempting to do so may cause
				// a server error. Possible fixes are to modify rack's
				// deserialization algorithm or to provide an option or flag
				// to force array serialization to be shallow.
				buildParams( prefix + "[" + ( typeof v === "object" ? i : "" ) + "]", v, traditional, add );
			}
		});

	} else if ( !traditional && jQuery.type( obj ) === "object" ) {
		// Serialize object item.
		for ( var name in obj ) {
			buildParams( prefix + "[" + name + "]", obj[ name ], traditional, add );
		}

	} else {
		// Serialize scalar item.
		add( prefix, obj );
	}
}

// This is still on the jQuery object... for now
// Want to move this to jQuery.ajax some day
jQuery.extend({

	// Counter for holding the number of active queries
	active: 0,

	// Last-Modified header cache for next request
	lastModified: {},
	etag: {}

});

/* Handles responses to an ajax request:
 * - sets all responseXXX fields accordingly
 * - finds the right dataType (mediates between content-type and expected dataType)
 * - returns the corresponding response
 */
function ajaxHandleResponses( s, jqXHR, responses ) {

	var contents = s.contents,
		dataTypes = s.dataTypes,
		responseFields = s.responseFields,
		ct,
		type,
		finalDataType,
		firstDataType;

	// Fill responseXXX fields
	for ( type in responseFields ) {
		if ( type in responses ) {
			jqXHR[ responseFields[type] ] = responses[ type ];
		}
	}

	// Remove auto dataType and get content-type in the process
	while( dataTypes[ 0 ] === "*" ) {
		dataTypes.shift();
		if ( ct === undefined ) {
			ct = s.mimeType || jqXHR.getResponseHeader( "content-type" );
		}
	}

	// Check if we're dealing with a known content-type
	if ( ct ) {
		for ( type in contents ) {
			if ( contents[ type ] && contents[ type ].test( ct ) ) {
				dataTypes.unshift( type );
				break;
			}
		}
	}

	// Check to see if we have a response for the expected dataType
	if ( dataTypes[ 0 ] in responses ) {
		finalDataType = dataTypes[ 0 ];
	} else {
		// Try convertible dataTypes
		for ( type in responses ) {
			if ( !dataTypes[ 0 ] || s.converters[ type + " " + dataTypes[0] ] ) {
				finalDataType = type;
				break;
			}
			if ( !firstDataType ) {
				firstDataType = type;
			}
		}
		// Or just use first one
		finalDataType = finalDataType || firstDataType;
	}

	// If we found a dataType
	// We add the dataType to the list if needed
	// and return the corresponding response
	if ( finalDataType ) {
		if ( finalDataType !== dataTypes[ 0 ] ) {
			dataTypes.unshift( finalDataType );
		}
		return responses[ finalDataType ];
	}
}

// Chain conversions given the request and the original response
function ajaxConvert( s, response ) {
	// Apply the dataFilter if provided
	if ( s.dataFilter ) {
		response = s.dataFilter( response, s.dataType );
	}

	var dataTypes = s.dataTypes,
		converters = {},
		i,
		key,
		length = dataTypes.length,
		tmp,
		// Current and previous dataTypes
		current = dataTypes[ 0 ],
		prev,
		// Conversion expression
		conversion,
		// Conversion function
		conv,
		// Conversion functions (transitive conversion)
		conv1,
		conv2;

	// For each dataType in the chain
	for ( i = 1; i < length; i++ ) {

		// Create converters map
		// with lowercased keys
		if ( i === 1 ) {
			for ( key in s.converters ) {
				if ( typeof key === "string" ) {
					converters[ key.toLowerCase() ] = s.converters[ key ];
				}
			}
		}

		// Get the dataTypes
		prev = current;
		current = dataTypes[ i ];

		// If current is auto dataType, update it to prev
		if ( current === "*" ) {
			current = prev;
		// If no auto and dataTypes are actually different
		} else if ( prev !== "*" && prev !== current ) {

			// Get the converter
			conversion = prev + " " + current;
			conv = converters[ conversion ] || converters[ "* " + current ];

			// If there is no direct converter, search transitively
			if ( !conv ) {
				conv2 = undefined;
				for ( conv1 in converters ) {
					tmp = conv1.split( " " );
					if ( tmp[ 0 ] === prev || tmp[ 0 ] === "*" ) {
						conv2 = converters[ tmp[1] + " " + current ];
						if ( conv2 ) {
							conv1 = converters[ conv1 ];
							if ( conv1 === true ) {
								conv = conv2;
							} else if ( conv2 === true ) {
								conv = conv1;
							}
							break;
						}
					}
				}
			}
			// If we found no converter, dispatch an error
			if ( !( conv || conv2 ) ) {
				jQuery.error( "No conversion from " + conversion.replace(" "," to ") );
			}
			// If found converter is not an equivalence
			if ( conv !== true ) {
				// Convert with 1 or 2 converters accordingly
				response = conv ? conv( response ) : conv2( conv1(response) );
			}
		}
	}
	return response;
}




var jsc = jQuery.now(),
	jsre = /(\=)\?(&|$)|\?\?/i;

// Default jsonp settings
jQuery.ajaxSetup({
	jsonp: "callback",
	jsonpCallback: function() {
		return jQuery.expando + "_" + ( jsc++ );
	}
});

// Detect, normalize options and install callbacks for jsonp requests
jQuery.ajaxPrefilter( "json jsonp", function( s, originalSettings, jqXHR ) {

	var inspectData = ( typeof s.data === "string" ) && /^application\/x\-www\-form\-urlencoded/.test( s.contentType );

	if ( s.dataTypes[ 0 ] === "jsonp" ||
		s.jsonp !== false && ( jsre.test( s.url ) ||
				inspectData && jsre.test( s.data ) ) ) {

		var responseContainer,
			jsonpCallback = s.jsonpCallback =
				jQuery.isFunction( s.jsonpCallback ) ? s.jsonpCallback() : s.jsonpCallback,
			previous = window[ jsonpCallback ],
			url = s.url,
			data = s.data,
			replace = "$1" + jsonpCallback + "$2";

		if ( s.jsonp !== false ) {
			url = url.replace( jsre, replace );
			if ( s.url === url ) {
				if ( inspectData ) {
					data = data.replace( jsre, replace );
				}
				if ( s.data === data ) {
					// Add callback manually
					url += (/\?/.test( url ) ? "&" : "?") + s.jsonp + "=" + jsonpCallback;
				}
			}
		}

		s.url = url;
		s.data = data;

		// Install callback
		window[ jsonpCallback ] = function( response ) {
			responseContainer = [ response ];
		};

		// Clean-up function
		jqXHR.always(function() {
			// Set callback back to previous value
			window[ jsonpCallback ] = previous;
			// Call if it was a function and we have a response
			if ( responseContainer && jQuery.isFunction( previous ) ) {
				window[ jsonpCallback ]( responseContainer[ 0 ] );
			}
		});

		// Use data converter to retrieve json after script execution
		s.converters["script json"] = function() {
			if ( !responseContainer ) {
				jQuery.error( jsonpCallback + " was not called" );
			}
			return responseContainer[ 0 ];
		};

		// force json dataType
		s.dataTypes[ 0 ] = "json";

		// Delegate to script
		return "script";
	}
});




// Install script dataType
jQuery.ajaxSetup({
	accepts: {
		script: "text/javascript, application/javascript, application/ecmascript, application/x-ecmascript"
	},
	contents: {
		script: /javascript|ecmascript/
	},
	converters: {
		"text script": function( text ) {
			jQuery.globalEval( text );
			return text;
		}
	}
});

// Handle cache's special case and global
jQuery.ajaxPrefilter( "script", function( s ) {
	if ( s.cache === undefined ) {
		s.cache = false;
	}
	if ( s.crossDomain ) {
		s.type = "GET";
		s.global = false;
	}
});

// Bind script tag hack transport
jQuery.ajaxTransport( "script", function(s) {

	// This transport only deals with cross domain requests
	if ( s.crossDomain ) {

		var script,
			head = document.head || document.getElementsByTagName( "head" )[0] || document.documentElement;

		return {

			send: function( _, callback ) {
          
				script = document.createElement( "script" );

				script.async = "async";

				if ( s.scriptCharset ) {
					script.charset = s.scriptCharset;
				}

				script.src = s.url;

				// Attach handlers for all browsers
				script.onload = script.onreadystatechange = function( _, isAbort ) {

					if ( isAbort || !script.readyState || /loaded|complete/.test( script.readyState ) ) {

						// Handle memory leak in IE
						script.onload = script.onreadystatechange = null;

						// Remove the script
						if ( head && script.parentNode ) {
							head.removeChild( script );
						}

						// Dereference the script
						script = undefined;

						// Callback if not abort
						if ( !isAbort ) {
							callback( 200, "success" );
						}
					}
				};
				// Use insertBefore instead of appendChild  to circumvent an IE6 bug.
				// This arises when a base node is used (#2709 and #4378).
				head.insertBefore( script, head.firstChild );
			},

			abort: function() {
				if ( script ) {
					script.onload( 0, 1 );
				}
			}
		};
	}
});




var // #5280: Internet Explorer will keep connections alive if we don't abort on unload
	xhrOnUnloadAbort = window.ActiveXObject ? function() {
		// Abort all pending requests
		for ( var key in xhrCallbacks ) {
			xhrCallbacks[ key ]( 0, 1 );
		}
	} : false,
	xhrId = 0,
	xhrCallbacks;

// Functions to create xhrs
function createStandardXHR() {
	try {
		return new window.XMLHttpRequest();
	} catch( e ) {}
}

function createActiveXHR() {
	try {
		return new window.ActiveXObject( "Microsoft.XMLHTTP" );
	} catch( e ) {}
}

// Create the request object
// (This is still attached to ajaxSettings for backward compatibility)
jQuery.ajaxSettings.xhr = window.ActiveXObject ?
	/* Microsoft failed to properly
	 * implement the XMLHttpRequest in IE7 (can't request local files),
	 * so we use the ActiveXObject when it is available
	 * Additionally XMLHttpRequest can be disabled in IE7/IE8 so
	 * we need a fallback.
	 */
	function() {
		return !this.isLocal && createStandardXHR() || createActiveXHR();
	} :
	// For all other browsers, use the standard XMLHttpRequest object
	createStandardXHR;

// Determine support properties
(function( xhr ) {
	jQuery.extend( jQuery.support, {
		ajax: !!xhr,
		cors: !!xhr && ( "withCredentials" in xhr )
	});
})( jQuery.ajaxSettings.xhr() );

// Create transport if the browser can provide an xhr
if ( jQuery.support.ajax ) {
      
	jQuery.ajaxTransport(function( s ) {
		// Cross domain only allowed if supported through XMLHttpRequest
		if ( !s.crossDomain || jQuery.support.cors ) {

			var callback;
			return {
				send: function( headers, complete ) {

					// Get a new xhr
					var xhr = s.xhr(),
						handle,
						i;

					// Open the socket
					// Passing null username, generates a login popup on Opera (#2865)
					if ( s.username ) {
						xhr.open( s.type, s.url, s.async, s.username, s.password );
					} else {
          	xhr.open( s.type, s.url, s.async );
					}

					// Apply custom fields if provided
					if ( s.xhrFields ) {
						for ( i in s.xhrFields ) {
							xhr[ i ] = s.xhrFields[ i ];
						}
					}

					// Override mime type if needed
					if ( s.mimeType && xhr.overrideMimeType ) {
						xhr.overrideMimeType( s.mimeType );
					}

					// X-Requested-With header
					// For cross-domain requests, seeing as conditions for a preflight are
					// akin to a jigsaw puzzle, we simply never set it to be sure.
					// (it can always be set on a per-request basis or even using ajaxSetup)
					// For same-domain requests, won't change header if already provided.
					if ( !s.crossDomain && !headers["X-Requested-With"] ) {
						headers[ "X-Requested-With" ] = "XMLHttpRequest";
					}

if (s.dataType == "binary") {
  if (xhr.hasOwnProperty("responseType")) {
    xhr.responseType = "arraybuffer";
  } else if (xhr.overrideMimeType) {
    xhr.overrideMimeType('text/plain; charset=x-user-defined');
  }
}

//xxxss= s

					// Need an extra try/catch for cross domain requests in Firefox 3
					try {
						for ( i in headers ) {
            //alert("setting header " + headers[i])
            //xxxss += headers[i] 
							xhr.setRequestHeader( i, headers[ i ] );
						}
					} catch( _ ) {}



					// Do send the request
					// This may raise an exception which is actually
					// handled in jQuery.ajax (so no try/catch here)
					xhr.send( ( s.hasContent && s.data ) || null );

					// Listener
					callback = function( _, isAbort ) {

						var status,
							statusText,
							responseHeaders,
							responses,
							xml;

						// Firefox throws exceptions when accessing properties
						// of an xhr when a network error occured
						// http://helpful.knobs-dials.com/index.php/Component_returned_failure_code:_0x80040111_(NS_ERROR_NOT_AVAILABLE)
						try {

							// Was never called and is aborted or complete
							if ( callback && ( isAbort || xhr.readyState === 4 ) ) {

								// Only called once
								callback = undefined;

								// Do not keep as active anymore
								if ( handle ) {
									xhr.onreadystatechange = jQuery.noop;
									if ( xhrOnUnloadAbort ) {
										delete xhrCallbacks[ handle ];
									}
								}

								// If it's an abort
								if ( isAbort ) {
	
									// Abort it manually if needed
									if ( xhr.readyState !== 4 ) {
										xhr.abort();
									}
								} else {
                	status = xhr.status;
									responseHeaders = xhr.getAllResponseHeaders();
									responses = {};
                  
									// BH: Safari in binary mode fails here 
									try {
										xml = xhr.responseXML;
										// Construct response list
										if ( xml && xml.documentElement /* #4958 */ ) {
											responses.xml = xml;
										}
									} catch( _ ) {
									}
									
									// When requesting binary data, IE6-9 will throw an exception
									// on any attempt to access responseText (#11426)
									try {
										responses.text = xhr.responseText;
              		} catch( _ ) {
									}
									// When requesting binary data, IE6-9 will throw an exception
									// on any attempt to access responseText (#11426)
									try {
										responses.data = xhr.response;
									} catch( _ ) {
									}
                  
                  
									// Firefox throws an exception when accessing
									// statusText for faulty cross-domain requests
									try {
										statusText = xhr.statusText;
									} catch( e ) {
										// We normalize with Webkit giving an empty statusText
										statusText = "";
									}
                  

									// Filter status for non standard behaviors

									// If the request is local and we have data: assume a success
									// (success with no data won't get notified, that's the best we
									// can do given current implementations)
									if ( !status && s.isLocal && !s.crossDomain ) {
										status = responses.text ? 200 : 404;
									// IE - #1450: sometimes returns 1223 when it should be 204
									} else if ( status === 1223 ) {
										status = 204;
									}
								}
							}
						} catch( firefoxAccessException ) {
							if ( !isAbort ) {
								complete( -1, firefoxAccessException );
							}
						}

						// Call complete if needed
						if ( responses ) {
							complete( status, statusText, responses, responseHeaders );
						}
					};

					// if we're in sync mode or it's in cache
					// and has been retrieved directly (IE6 & IE7)
					// we need to manually fire the callback
					if ( !s.async || xhr.readyState === 4 ) {
          	callback();
					} else {
						handle = ++xhrId;
						if ( xhrOnUnloadAbort ) {
							// Create the active xhrs callbacks list if needed
							// and attach the unload handler
							if ( !xhrCallbacks ) {
								xhrCallbacks = {};
								jQuery( window ).unload( xhrOnUnloadAbort );
							}
							// Add to list of active xhrs callbacks
							xhrCallbacks[ handle ] = callback;
						}
						xhr.onreadystatechange = callback;
					}
				},

				abort: function() {
					if ( callback ) {
						callback(0,1);
					}
				}
			};
		}
	});
}




var elemdisplay = {},
	iframe, iframeDoc,
	rfxtypes = /^(?:toggle|show|hide)$/,
	rfxnum = /^([+\-]=)?([\d+.\-]+)([a-z%]*)$/i,
	timerId,
	fxAttrs = [
		// height animations
		[ "height", "marginTop", "marginBottom", "paddingTop", "paddingBottom" ],
		// width animations
		[ "width", "marginLeft", "marginRight", "paddingLeft", "paddingRight" ],
		// opacity animations
		[ "opacity" ]
	],
	fxNow;

jQuery.fn.extend({
	show: function( speed, easing, callback ) {
		var elem, display;

		if ( speed || speed === 0 ) {
			return this.animate( genFx("show", 3), speed, easing, callback );

		} else {
			for ( var i = 0, j = this.length; i < j; i++ ) {
				elem = this[ i ];

				if ( elem.style ) {
					display = elem.style.display;

					// Reset the inline display of this element to learn if it is
					// being hidden by cascaded rules or not
					if ( !jQuery._data(elem, "olddisplay") && display === "none" ) {
						display = elem.style.display = "";
					}

					// Set elements which have been overridden with display: none
					// in a stylesheet to whatever the default browser style is
					// for such an element
					if ( (display === "" && jQuery.css(elem, "display") === "none") ||
						!jQuery.contains( elem.ownerDocument.documentElement, elem ) ) {
						jQuery._data( elem, "olddisplay", defaultDisplay(elem.nodeName) );
					}
				}
			}

			// Set the display of most of the elements in a second loop
			// to avoid the constant reflow
			for ( i = 0; i < j; i++ ) {
				elem = this[ i ];

				if ( elem.style ) {
					display = elem.style.display;

					if ( display === "" || display === "none" ) {
						elem.style.display = jQuery._data( elem, "olddisplay" ) || "";
					}
				}
			}

			return this;
		}
	},

	hide: function( speed, easing, callback ) {
		if ( speed || speed === 0 ) {
			return this.animate( genFx("hide", 3), speed, easing, callback);

		} else {
			var elem, display,
				i = 0,
				j = this.length;

			for ( ; i < j; i++ ) {
				elem = this[i];
				if ( elem.style ) {
					display = jQuery.css( elem, "display" );

					if ( display !== "none" && !jQuery._data( elem, "olddisplay" ) ) {
						jQuery._data( elem, "olddisplay", display );
					}
				}
			}

			// Set the display of the elements in a second loop
			// to avoid the constant reflow
			for ( i = 0; i < j; i++ ) {
				if ( this[i].style ) {
					this[i].style.display = "none";
				}
			}

			return this;
		}
	},

	// Save the old toggle function
	_toggle: jQuery.fn.toggle,

	toggle: function( fn, fn2, callback ) {
		var bool = typeof fn === "boolean";

		if ( jQuery.isFunction(fn) && jQuery.isFunction(fn2) ) {
			this._toggle.apply( this, arguments );

		} else if ( fn == null || bool ) {
			this.each(function() {
				var state = bool ? fn : jQuery(this).is(":hidden");
				jQuery(this)[ state ? "show" : "hide" ]();
			});

		} else {
			this.animate(genFx("toggle", 3), fn, fn2, callback);
		}

		return this;
	},

	fadeTo: function( speed, to, easing, callback ) {
		return this.filter(":hidden").css("opacity", 0).show().end()
					.animate({opacity: to}, speed, easing, callback);
	},

	animate: function( prop, speed, easing, callback ) {
		var optall = jQuery.speed( speed, easing, callback );

		if ( jQuery.isEmptyObject( prop ) ) {
			return this.each( optall.complete, [ false ] );
		}

		// Do not change referenced properties as per-property easing will be lost
		prop = jQuery.extend( {}, prop );

		function doAnimation() {
			// XXX 'this' does not always have a nodeName when running the
			// test suite

			if ( optall.queue === false ) {
				jQuery._mark( this );
			}

			var opt = jQuery.extend( {}, optall ),
				isElement = this.nodeType === 1,
				hidden = isElement && jQuery(this).is(":hidden"),
				name, val, p, e, hooks, replace,
				parts, start, end, unit,
				method;

			// will store per property easing and be used to determine when an animation is complete
			opt.animatedProperties = {};

			// first pass over propertys to expand / normalize
			for ( p in prop ) {
				name = jQuery.camelCase( p );
				if ( p !== name ) {
					prop[ name ] = prop[ p ];
					delete prop[ p ];
				}

				if ( ( hooks = jQuery.cssHooks[ name ] ) && "expand" in hooks ) {
					replace = hooks.expand( prop[ name ] );
					delete prop[ name ];

					// not quite $.extend, this wont overwrite keys already present.
					// also - reusing 'p' from above because we have the correct "name"
					for ( p in replace ) {
						if ( ! ( p in prop ) ) {
							prop[ p ] = replace[ p ];
						}
					}
				}
			}

			for ( name in prop ) {
				val = prop[ name ];
				// easing resolution: per property > opt.specialEasing > opt.easing > 'swing' (default)
				if ( jQuery.isArray( val ) ) {
					opt.animatedProperties[ name ] = val[ 1 ];
					val = prop[ name ] = val[ 0 ];
				} else {
					opt.animatedProperties[ name ] = opt.specialEasing && opt.specialEasing[ name ] || opt.easing || 'swing';
				}

				if ( val === "hide" && hidden || val === "show" && !hidden ) {
					return opt.complete.call( this );
				}

				if ( isElement && ( name === "height" || name === "width" ) ) {
					// Make sure that nothing sneaks out
					// Record all 3 overflow attributes because IE does not
					// change the overflow attribute when overflowX and
					// overflowY are set to the same value
					opt.overflow = [ this.style.overflow, this.style.overflowX, this.style.overflowY ];

					// Set display property to inline-block for height/width
					// animations on inline elements that are having width/height animated
					if ( jQuery.css( this, "display" ) === "inline" &&
							jQuery.css( this, "float" ) === "none" ) {

						// inline-level elements accept inline-block;
						// block-level elements need to be inline with layout
						if ( !jQuery.support.inlineBlockNeedsLayout || defaultDisplay( this.nodeName ) === "inline" ) {
							this.style.display = "inline-block";

						} else {
							this.style.zoom = 1;
						}
					}
				}
			}

			if ( opt.overflow != null ) {
				this.style.overflow = "hidden";
			}

			for ( p in prop ) {
				e = new jQuery.fx( this, opt, p );
				val = prop[ p ];

				if ( rfxtypes.test( val ) ) {

					// Tracks whether to show or hide based on private
					// data attached to the element
					method = jQuery._data( this, "toggle" + p ) || ( val === "toggle" ? hidden ? "show" : "hide" : 0 );
					if ( method ) {
						jQuery._data( this, "toggle" + p, method === "show" ? "hide" : "show" );
						e[ method ]();
					} else {
						e[ val ]();
					}

				} else {
					parts = rfxnum.exec( val );
					start = e.cur();

					if ( parts ) {
						end = parseFloat( parts[2] );
						unit = parts[3] || ( jQuery.cssNumber[ p ] ? "" : "px" );

						// We need to compute starting value
						if ( unit !== "px" ) {
							jQuery.style( this, p, (end || 1) + unit);
							start = ( (end || 1) / e.cur() ) * start;
							jQuery.style( this, p, start + unit);
						}

						// If a +=/-= token was provided, we're doing a relative animation
						if ( parts[1] ) {
							end = ( (parts[ 1 ] === "-=" ? -1 : 1) * end ) + start;
						}

						e.custom( start, end, unit );

					} else {
						e.custom( start, val, "" );
					}
				}
			}

			// For JS strict compliance
			return true;
		}

		return optall.queue === false ?
			this.each( doAnimation ) :
			this.queue( optall.queue, doAnimation );
	},

	stop: function( type, clearQueue, gotoEnd ) {
		if ( typeof type !== "string" ) {
			gotoEnd = clearQueue;
			clearQueue = type;
			type = undefined;
		}
		if ( clearQueue && type !== false ) {
			this.queue( type || "fx", [] );
		}

		return this.each(function() {
			var index,
				hadTimers = false,
				timers = jQuery.timers,
				data = jQuery._data( this );

			// clear marker counters if we know they won't be
			if ( !gotoEnd ) {
				jQuery._unmark( true, this );
			}

			function stopQueue( elem, data, index ) {
				var hooks = data[ index ];
				jQuery.removeData( elem, index, true );
				hooks.stop( gotoEnd );
			}

			if ( type == null ) {
				for ( index in data ) {
					if ( data[ index ] && data[ index ].stop && index.indexOf(".run") === index.length - 4 ) {
						stopQueue( this, data, index );
					}
				}
			} else if ( data[ index = type + ".run" ] && data[ index ].stop ){
				stopQueue( this, data, index );
			}

			for ( index = timers.length; index--; ) {
				if ( timers[ index ].elem === this && (type == null || timers[ index ].queue === type) ) {
					if ( gotoEnd ) {

						// force the next step to be the last
						timers[ index ]( true );
					} else {
						timers[ index ].saveState();
					}
					hadTimers = true;
					timers.splice( index, 1 );
				}
			}

			// start the next in the queue if the last step wasn't forced
			// timers currently will call their complete callbacks, which will dequeue
			// but only if they were gotoEnd
			if ( !( gotoEnd && hadTimers ) ) {
				jQuery.dequeue( this, type );
			}
		});
	}

});

// Animations created synchronously will run synchronously
function createFxNow() {
	setTimeout( clearFxNow, 0 );
	return ( fxNow = jQuery.now() );
}

function clearFxNow() {
	fxNow = undefined;
}

// Generate parameters to create a standard animation
function genFx( type, num ) {
	var obj = {};

	jQuery.each( fxAttrs.concat.apply([], fxAttrs.slice( 0, num )), function() {
		obj[ this ] = type;
	});

	return obj;
}

// Generate shortcuts for custom animations
jQuery.each({
	slideDown: genFx( "show", 1 ),
	slideUp: genFx( "hide", 1 ),
	slideToggle: genFx( "toggle", 1 ),
	fadeIn: { opacity: "show" },
	fadeOut: { opacity: "hide" },
	fadeToggle: { opacity: "toggle" }
}, function( name, props ) {
	jQuery.fn[ name ] = function( speed, easing, callback ) {
		return this.animate( props, speed, easing, callback );
	};
});

jQuery.extend({
	speed: function( speed, easing, fn ) {
		var opt = speed && typeof speed === "object" ? jQuery.extend( {}, speed ) : {
			complete: fn || !fn && easing ||
				jQuery.isFunction( speed ) && speed,
			duration: speed,
			easing: fn && easing || easing && !jQuery.isFunction( easing ) && easing
		};

		opt.duration = jQuery.fx.off ? 0 : typeof opt.duration === "number" ? opt.duration :
			opt.duration in jQuery.fx.speeds ? jQuery.fx.speeds[ opt.duration ] : jQuery.fx.speeds._default;

		// normalize opt.queue - true/undefined/null -> "fx"
		if ( opt.queue == null || opt.queue === true ) {
			opt.queue = "fx";
		}

		// Queueing
		opt.old = opt.complete;

		opt.complete = function( noUnmark ) {
			if ( jQuery.isFunction( opt.old ) ) {
				opt.old.call( this );
			}

			if ( opt.queue ) {
				jQuery.dequeue( this, opt.queue );
			} else if ( noUnmark !== false ) {
				jQuery._unmark( this );
			}
		};

		return opt;
	},

	easing: {
		linear: function( p ) {
			return p;
		},
		swing: function( p ) {
			return ( -Math.cos( p*Math.PI ) / 2 ) + 0.5;
		}
	},

	timers: [],

	fx: function( elem, options, prop ) {
		this.options = options;
		this.elem = elem;
		this.prop = prop;

		options.orig = options.orig || {};
	}

});

jQuery.fx.prototype = {
	// Simple function for setting a style value
	update: function() {
		if ( this.options.step ) {
			this.options.step.call( this.elem, this.now, this );
		}

		( jQuery.fx.step[ this.prop ] || jQuery.fx.step._default )( this );
	},

	// Get the current size
	cur: function() {
		if ( this.elem[ this.prop ] != null && (!this.elem.style || this.elem.style[ this.prop ] == null) ) {
			return this.elem[ this.prop ];
		}

		var parsed,
			r = jQuery.css( this.elem, this.prop );
		// Empty strings, null, undefined and "auto" are converted to 0,
		// complex values such as "rotate(1rad)" are returned as is,
		// simple values such as "10px" are parsed to Float.
		return isNaN( parsed = parseFloat( r ) ) ? !r || r === "auto" ? 0 : r : parsed;
	},

	// Start an animation from one number to another
	custom: function( from, to, unit ) {
		var self = this,
			fx = jQuery.fx;

		this.startTime = fxNow || createFxNow();
		this.end = to;
		this.now = this.start = from;
		this.pos = this.state = 0;
		this.unit = unit || this.unit || ( jQuery.cssNumber[ this.prop ] ? "" : "px" );

		function t( gotoEnd ) {
			return self.step( gotoEnd );
		}

		t.queue = this.options.queue;
		t.elem = this.elem;
		t.saveState = function() {
			if ( jQuery._data( self.elem, "fxshow" + self.prop ) === undefined ) {
				if ( self.options.hide ) {
					jQuery._data( self.elem, "fxshow" + self.prop, self.start );
				} else if ( self.options.show ) {
					jQuery._data( self.elem, "fxshow" + self.prop, self.end );
				}
			}
		};

		if ( t() && jQuery.timers.push(t) && !timerId ) {
			timerId = setInterval( fx.tick, fx.interval );
		}
	},

	// Simple 'show' function
	show: function() {
		var dataShow = jQuery._data( this.elem, "fxshow" + this.prop );

		// Remember where we started, so that we can go back to it later
		this.options.orig[ this.prop ] = dataShow || jQuery.style( this.elem, this.prop );
		this.options.show = true;

		// Begin the animation
		// Make sure that we start at a small width/height to avoid any flash of content
		if ( dataShow !== undefined ) {
			// This show is picking up where a previous hide or show left off
			this.custom( this.cur(), dataShow );
		} else {
			this.custom( this.prop === "width" || this.prop === "height" ? 1 : 0, this.cur() );
		}

		// Start by showing the element
		jQuery( this.elem ).show();
	},

	// Simple 'hide' function
	hide: function() {
		// Remember where we started, so that we can go back to it later
		this.options.orig[ this.prop ] = jQuery._data( this.elem, "fxshow" + this.prop ) || jQuery.style( this.elem, this.prop );
		this.options.hide = true;

		// Begin the animation
		this.custom( this.cur(), 0 );
	},

	// Each step of an animation
	step: function( gotoEnd ) {
		var p, n, complete,
			t = fxNow || createFxNow(),
			done = true,
			elem = this.elem,
			options = this.options;

		if ( gotoEnd || t >= options.duration + this.startTime ) {
			this.now = this.end;
			this.pos = this.state = 1;
			this.update();

			options.animatedProperties[ this.prop ] = true;

			for ( p in options.animatedProperties ) {
				if ( options.animatedProperties[ p ] !== true ) {
					done = false;
				}
			}

			if ( done ) {
				// Reset the overflow
				if ( options.overflow != null && !jQuery.support.shrinkWrapBlocks ) {

					jQuery.each( [ "", "X", "Y" ], function( index, value ) {
						elem.style[ "overflow" + value ] = options.overflow[ index ];
					});
				}

				// Hide the element if the "hide" operation was done
				if ( options.hide ) {
					jQuery( elem ).hide();
				}

				// Reset the properties, if the item has been hidden or shown
				if ( options.hide || options.show ) {
					for ( p in options.animatedProperties ) {
						jQuery.style( elem, p, options.orig[ p ] );
						jQuery.removeData( elem, "fxshow" + p, true );
						// Toggle data is no longer needed
						jQuery.removeData( elem, "toggle" + p, true );
					}
				}

				// Execute the complete function
				// in the event that the complete function throws an exception
				// we must ensure it won't be called twice. #5684

				complete = options.complete;
				if ( complete ) {

					options.complete = false;
					complete.call( elem );
				}
			}

			return false;

		} else {
			// classical easing cannot be used with an Infinity duration
			if ( options.duration == Infinity ) {
				this.now = t;
			} else {
				n = t - this.startTime;
				this.state = n / options.duration;

				// Perform the easing function, defaults to swing
				this.pos = jQuery.easing[ options.animatedProperties[this.prop] ]( this.state, n, 0, 1, options.duration );
				this.now = this.start + ( (this.end - this.start) * this.pos );
			}
			// Perform the next step of the animation
			this.update();
		}

		return true;
	}
};

jQuery.extend( jQuery.fx, {
	tick: function() {
		var timer,
			timers = jQuery.timers,
			i = 0;

		for ( ; i < timers.length; i++ ) {
			timer = timers[ i ];
			// Checks the timer has not already been removed
			if ( !timer() && timers[ i ] === timer ) {
				timers.splice( i--, 1 );
			}
		}

		if ( !timers.length ) {
			jQuery.fx.stop();
		}
	},

	interval: 13,

	stop: function() {
		clearInterval( timerId );
		timerId = null;
	},

	speeds: {
		slow: 600,
		fast: 200,
		// Default speed
		_default: 400
	},

	step: {
		opacity: function( fx ) {
			jQuery.style( fx.elem, "opacity", fx.now );
		},

		_default: function( fx ) {
			if ( fx.elem.style && fx.elem.style[ fx.prop ] != null ) {
				fx.elem.style[ fx.prop ] = fx.now + fx.unit;
			} else {
				fx.elem[ fx.prop ] = fx.now;
			}
		}
	}
});

// Ensure props that can't be negative don't go there on undershoot easing
jQuery.each( fxAttrs.concat.apply( [], fxAttrs ), function( i, prop ) {
	// exclude marginTop, marginLeft, marginBottom and marginRight from this list
	if ( prop.indexOf( "margin" ) ) {
		jQuery.fx.step[ prop ] = function( fx ) {
			jQuery.style( fx.elem, prop, Math.max(0, fx.now) + fx.unit );
		};
	}
});

if ( jQuery.expr && jQuery.expr.filters ) {
	jQuery.expr.filters.animated = function( elem ) {
		return jQuery.grep(jQuery.timers, function( fn ) {
			return elem === fn.elem;
		}).length;
	};
}

// Try to restore the default display value of an element
function defaultDisplay( nodeName ) {

	if ( !elemdisplay[ nodeName ] ) {

		var body = document.body,
			elem = jQuery( "<" + nodeName + ">" ).appendTo( body ),
			display = elem.css( "display" );
		elem.remove();

		// If the simple way fails,
		// get element's real default display by attaching it to a temp iframe
		if ( display === "none" || display === "" ) {
			// No iframe to use yet, so create it
			if ( !iframe ) {
				iframe = document.createElement( "iframe" );
				iframe.frameBorder = iframe.width = iframe.height = 0;
			}

			body.appendChild( iframe );

			// Create a cacheable copy of the iframe document on first call.
			// IE and Opera will allow us to reuse the iframeDoc without re-writing the fake HTML
			// document to it; WebKit & Firefox won't allow reusing the iframe document.
			if ( !iframeDoc || !iframe.createElement ) {
				iframeDoc = ( iframe.contentWindow || iframe.contentDocument ).document;
				iframeDoc.write( ( jQuery.support.boxModel ? "<!doctype html>" : "" ) + "<html><body>" );
				iframeDoc.close();
			}

			elem = iframeDoc.createElement( nodeName );

			iframeDoc.body.appendChild( elem );

			display = jQuery.css( elem, "display" );
			body.removeChild( iframe );
		}

		// Store the correct default display
		elemdisplay[ nodeName ] = display;
	}

	return elemdisplay[ nodeName ];
}




var getOffset,
	rtable = /^t(?:able|d|h)$/i,
	rroot = /^(?:body|html)$/i;

if ( "getBoundingClientRect" in document.documentElement ) {
	getOffset = function( elem, doc, docElem, box ) {
		try {
			box = elem.getBoundingClientRect();
		} catch(e) {}

		// Make sure we're not dealing with a disconnected DOM node
		if ( !box || !jQuery.contains( docElem, elem ) ) {
			return box ? { top: box.top, left: box.left } : { top: 0, left: 0 };
		}

		var body = doc.body,
			win = getWindow( doc ),
			clientTop  = docElem.clientTop  || body.clientTop  || 0,
			clientLeft = docElem.clientLeft || body.clientLeft || 0,
			scrollTop  = win.pageYOffset || jQuery.support.boxModel && docElem.scrollTop  || body.scrollTop,
			scrollLeft = win.pageXOffset || jQuery.support.boxModel && docElem.scrollLeft || body.scrollLeft,
			top  = box.top  + scrollTop  - clientTop,
			left = box.left + scrollLeft - clientLeft;

		return { top: top, left: left };
	};

} else {
	getOffset = function( elem, doc, docElem ) {
		var computedStyle,
			offsetParent = elem.offsetParent,
			prevOffsetParent = elem,
			body = doc.body,
			defaultView = doc.defaultView,
			prevComputedStyle = defaultView ? defaultView.getComputedStyle( elem, null ) : elem.currentStyle,
			top = elem.offsetTop,
			left = elem.offsetLeft;

		while ( (elem = elem.parentNode) && elem !== body && elem !== docElem ) {
			if ( jQuery.support.fixedPosition && prevComputedStyle.position === "fixed" ) {
				break;
			}

			computedStyle = defaultView ? defaultView.getComputedStyle(elem, null) : elem.currentStyle;
			top  -= elem.scrollTop;
			left -= elem.scrollLeft;

			if ( elem === offsetParent ) {
				top  += elem.offsetTop;
				left += elem.offsetLeft;

				if ( jQuery.support.doesNotAddBorder && !(jQuery.support.doesAddBorderForTableAndCells && rtable.test(elem.nodeName)) ) {
					top  += parseFloat( computedStyle.borderTopWidth  ) || 0;
					left += parseFloat( computedStyle.borderLeftWidth ) || 0;
				}

				prevOffsetParent = offsetParent;
				offsetParent = elem.offsetParent;
			}

			if ( jQuery.support.subtractsBorderForOverflowNotVisible && computedStyle.overflow !== "visible" ) {
				top  += parseFloat( computedStyle.borderTopWidth  ) || 0;
				left += parseFloat( computedStyle.borderLeftWidth ) || 0;
			}

			prevComputedStyle = computedStyle;
		}

		if ( prevComputedStyle.position === "relative" || prevComputedStyle.position === "static" ) {
			top  += body.offsetTop;
			left += body.offsetLeft;
		}

		if ( jQuery.support.fixedPosition && prevComputedStyle.position === "fixed" ) {
			top  += Math.max( docElem.scrollTop, body.scrollTop );
			left += Math.max( docElem.scrollLeft, body.scrollLeft );
		}

		return { top: top, left: left };
	};
}

jQuery.fn.offset = function( options ) {
	if ( arguments.length ) {
		return options === undefined ?
			this :
			this.each(function( i ) {
				jQuery.offset.setOffset( this, options, i );
			});
	}

	var elem = this[0],
		doc = elem && elem.ownerDocument;

	if ( !doc ) {
		return null;
	}

	if ( elem === doc.body ) {
		return jQuery.offset.bodyOffset( elem );
	}

	return getOffset( elem, doc, doc.documentElement );
};

jQuery.offset = {

	bodyOffset: function( body ) {
		var top = body.offsetTop,
			left = body.offsetLeft;

		if ( jQuery.support.doesNotIncludeMarginInBodyOffset ) {
			top  += parseFloat( jQuery.css(body, "marginTop") ) || 0;
			left += parseFloat( jQuery.css(body, "marginLeft") ) || 0;
		}

		return { top: top, left: left };
	},

	setOffset: function( elem, options, i ) {
		var position = jQuery.css( elem, "position" );

		// set position first, in-case top/left are set even on static elem
		if ( position === "static" ) {
			elem.style.position = "relative";
		}

		var curElem = jQuery( elem ),
			curOffset = curElem.offset(),
			curCSSTop = jQuery.css( elem, "top" ),
			curCSSLeft = jQuery.css( elem, "left" ),
			calculatePosition = ( position === "absolute" || position === "fixed" ) && jQuery.inArray("auto", [curCSSTop, curCSSLeft]) > -1,
			props = {}, curPosition = {}, curTop, curLeft;

		// need to be able to calculate position if either top or left is auto and position is either absolute or fixed
		if ( calculatePosition ) {
			curPosition = curElem.position();
			curTop = curPosition.top;
			curLeft = curPosition.left;
		} else {
			curTop = parseFloat( curCSSTop ) || 0;
			curLeft = parseFloat( curCSSLeft ) || 0;
		}

		if ( jQuery.isFunction( options ) ) {
			options = options.call( elem, i, curOffset );
		}

		if ( options.top != null ) {
			props.top = ( options.top - curOffset.top ) + curTop;
		}
		if ( options.left != null ) {
			props.left = ( options.left - curOffset.left ) + curLeft;
		}

		if ( "using" in options ) {
			options.using.call( elem, props );
		} else {
			curElem.css( props );
		}
	}
};


jQuery.fn.extend({

	position: function() {
		if ( !this[0] ) {
			return null;
		}

		var elem = this[0],

		// Get *real* offsetParent
		offsetParent = this.offsetParent(),

		// Get correct offsets
		offset       = this.offset(),
		parentOffset = rroot.test(offsetParent[0].nodeName) ? { top: 0, left: 0 } : offsetParent.offset();

		// Subtract element margins
		// note: when an element has margin: auto the offsetLeft and marginLeft
		// are the same in Safari causing offset.left to incorrectly be 0
		offset.top  -= parseFloat( jQuery.css(elem, "marginTop") ) || 0;
		offset.left -= parseFloat( jQuery.css(elem, "marginLeft") ) || 0;

		// Add offsetParent borders
		parentOffset.top  += parseFloat( jQuery.css(offsetParent[0], "borderTopWidth") ) || 0;
		parentOffset.left += parseFloat( jQuery.css(offsetParent[0], "borderLeftWidth") ) || 0;

		// Subtract the two offsets
		return {
			top:  offset.top  - parentOffset.top,
			left: offset.left - parentOffset.left
		};
	},

	offsetParent: function() {
		return this.map(function() {
			var offsetParent = this.offsetParent || document.body;
			while ( offsetParent && (!rroot.test(offsetParent.nodeName) && jQuery.css(offsetParent, "position") === "static") ) {
				offsetParent = offsetParent.offsetParent;
			}
			return offsetParent;
		});
	}
});


// Create scrollLeft and scrollTop methods
jQuery.each( {scrollLeft: "pageXOffset", scrollTop: "pageYOffset"}, function( method, prop ) {
	var top = /Y/.test( prop );

	jQuery.fn[ method ] = function( val ) {
		return jQuery.access( this, function( elem, method, val ) {
			var win = getWindow( elem );

			if ( val === undefined ) {
				return win ? (prop in win) ? win[ prop ] :
					jQuery.support.boxModel && win.document.documentElement[ method ] ||
						win.document.body[ method ] :
					elem[ method ];
			}

			if ( win ) {
				win.scrollTo(
					!top ? val : jQuery( win ).scrollLeft(),
					 top ? val : jQuery( win ).scrollTop()
				);

			} else {
				elem[ method ] = val;
			}
		}, method, val, arguments.length, null );
	};
});

function getWindow( elem ) {
	return jQuery.isWindow( elem ) ?
		elem :
		elem.nodeType === 9 ?
			elem.defaultView || elem.parentWindow :
			false;
}




// Create width, height, innerHeight, innerWidth, outerHeight and outerWidth methods
jQuery.each( { Height: "height", Width: "width" }, function( name, type ) {
	var clientProp = "client" + name,
		scrollProp = "scroll" + name,
		offsetProp = "offset" + name;

	// innerHeight and innerWidth
	jQuery.fn[ "inner" + name ] = function() {
		var elem = this[0];
		return elem ?
			elem.style ?
			parseFloat( jQuery.css( elem, type, "padding" ) ) :
			this[ type ]() :
			null;
	};

	// outerHeight and outerWidth
	jQuery.fn[ "outer" + name ] = function( margin ) {
		var elem = this[0];
		return elem ?
			elem.style ?
			parseFloat( jQuery.css( elem, type, margin ? "margin" : "border" ) ) :
			this[ type ]() :
			null;
	};

	jQuery.fn[ type ] = function( value ) {
		return jQuery.access( this, function( elem, type, value ) {
			var doc, docElemProp, orig, ret;

			if ( jQuery.isWindow( elem ) ) {
				// 3rd condition allows Nokia support, as it supports the docElem prop but not CSS1Compat
				doc = elem.document;
				docElemProp = doc.documentElement[ clientProp ];
				return jQuery.support.boxModel && docElemProp ||
					doc.body && doc.body[ clientProp ] || docElemProp;
			}

			// Get document width or height
			if ( elem.nodeType === 9 ) {
				// Either scroll[Width/Height] or offset[Width/Height], whichever is greater
				doc = elem.documentElement;

				// when a window > document, IE6 reports a offset[Width/Height] > client[Width/Height]
				// so we can't use max, as it'll choose the incorrect offset[Width/Height]
				// instead we use the correct client[Width/Height]
				// support:IE6
				if ( doc[ clientProp ] >= doc[ scrollProp ] ) {
					return doc[ clientProp ];
				}

				return Math.max(
					elem.body[ scrollProp ], doc[ scrollProp ],
					elem.body[ offsetProp ], doc[ offsetProp ]
				);
			}

			// Get width or height on the element
			if ( value === undefined ) {
				orig = jQuery.css( elem, type );
				ret = parseFloat( orig );
				return jQuery.isNumeric( ret ) ? ret : orig;
			}

			// Set the width or height on the element
			jQuery( elem ).css( type, value );
		}, type, value, arguments.length, null );
	};
});




// Expose jQuery to the global object
window.jQuery = window.$ = jQuery;

// Expose jQuery as an AMD module, but only for AMD loaders that
// understand the issues with loading multiple versions of jQuery
// in a page that all might call define(). The loader will indicate
// they have special allowances for multiple jQuery versions by
// specifying define.amd.jQuery = true. Register as a named module,
// since jQuery can be concatenated with other files that may use define,
// but not use a proper concatenation script that understands anonymous
// AMD modules. A named AMD is safest and most robust way to register.
// Lowercase jquery is used because AMD module names are derived from
// file names, and jQuery is normally delivered in a lowercase file name.
// Do this after creating the global so that if an AMD module wants to call
// noConflict to hide this version of jQuery, it will work.
if ( typeof define === "function" && define.amd && define.amd.jQuery ) {
	define( "jquery", [], function () { return jQuery; } );
}



})( window );

// JSmolCore.js -- Jmol core capability  3/22/2013 5:52:54 PM 

// see JSmolApi.js for public user-interface. All these are private functions

// BH 3/22/2013 5:53:02 PM: Adds noscript option, JSmol.min.core.js
// BH 1/17/2013 5:20:44 PM: Fixed problem with console not getting initial position if no first click
// 1/13/2013 BH: Fixed MSIE not-reading-local-files problem.
// 11/28/2012 BH: Fixed MacOS Safari binary ArrayBuffer problem
// 11/21/2012 BH: restructuring of files as JS... instead of J...
// 11/20/2012 BH: MSIE9 cannot do a synchronous file load cross-domain. See Jmol._getFileData
// 11/4/2012 BH: RCSB REST format change "<structureId>" to "<dimStructure.structureId>"
// 9/13/2012 BH: JmolCore.js chfanges for JSmol doAjax() method -- _3ata()
// 6/12/2012 BH: JmolApi.js: adds Jmol.setInfo(applet, info, isShown) -- third parameter optional 
// 6/12/2012 BH: JmolApi.js: adds Jmol.getInfo(applet) 
// 6/12/2012 BH: JmolApplet.js: Fixes for MSIE 8
// 6/5/2012  BH: fixes problem with Jmol "javascript" command not working and getPropertyAsArray not working
// 6/4/2012  BH: corrects problem with MSIE requiring mouse-hover to activate applet
// 5/31/2012 BH: added JSpecView interface and api -- see JmolJSV.js
//               also changed "jmolJarPath" to just "jarPath"
//               jmolJarFile->jarFile, jmolIsSigned->isSigned, jmolReadyFunction->readyFunction
//               also corrects a double-loading issue
// 5/14/2012 BH: added AJAX queue for ChemDoodle option with multiple canvases 
// 8/12/2012 BH: adds support for MSIE xdr cross-domain request (jQuery.iecors.js)

// allows Jmol applets to be created on a page with more flexibility and extendability
// provides an object-oriented interface for JSpecView and syncing of Jmol/JSpecView


// JSmoljQuery modifies standard jQuery to include binary file transfer
// If you are using jQuery already on your page and you do not need any
// binary file transfer, you can  

// required/optional libraries (preferably in the following order):

//		JSmoljQuery.js      -- required for binary file transfer; otherwise standard jQuery should be OK
//		JSmolCore.js      -- required;
//		JSmolApplet.js    -- required; internal functions for _Applet and _Image; must be after JmolCore
//		JSmolControls.js  -- optional; internal functions for buttons, links, menus, etc.; must be after JmolCore
//		JSmolApi.js       -- required; all user functions; must be after JmolCore
//    JSmolTHREE.js     -- WebGL library required for JSmolGLmol.js
//    JSmolGLmol.js     -- WebGL version of JSmol.
//		JSmolJSV.js       -- optional; for creating and interacting with a JSpecView applet 
//                          (requires JSpecViewApplet.jar or JSpecViewAppletSigned.jar
//    JSmol.js

// Allows Jmol-like objects to be displayed on Java-challenged (iPad/iPhone)
// or applet-challenged (Android/iPhone) platforms, with automatic switching to 

// For your installation, you should consider putting JmolData.jar and jsmol.php 
// on your own server. Nothing more than these two files is needed on the server, and this 
// allows more options for MSIE and Chrome when working with cross-domain files (such as RCSB or pubChem) 

// The NCI and RCSB databases are accessed via direct AJAX if available (xhr2/xdr).


if(typeof(jQuery)=="undefined") alert("Note -- JSmoljQuery is required for JSmol, but it's not defined.")


Jmol = (function(document) {
	return {
		_jmolInfo: {
			userAgent:navigator.userAgent, 
			version: version = 'Jmol-JSO 13.0'
		},
		_allowedJmolSize: [25, 2048, 300],   // min, max, default (pixels)
    /*  By setting the Jmol.allowedJmolSize[] variable in the webpage
        before calling Jmol.getApplet(), limits for applet size can be overriden.
        2048 standard for GeoWall (http://geowall.geo.lsa.umich.edu/home.html)
    */		
		_applets: {},
		_asynchronous: true,
		_ajaxQueue: [],
		db: {
			_databasePrefixes: "$=:",
			_fileLoadScript: ";if (_loadScript = '' && defaultLoadScript == '' && _filetype == 'Pdb') { select protein or nucleic;cartoons Only;color structure; select * };",
			_nciLoadScript: ";n = ({molecule=1}.length < {molecule=2}.length ? 2 : 1); select molecule=n;display selected;center selected;",
			_pubChemLoadScript: "",
			_DirectDatabaseCalls:{
				"cactus.nci.nih.gov": "%URL",
				"www.rcsb.org": "%URL",
				"pubchem.ncbi.nlm.nih.gov":"%URL",
				"$": "http://cactus.nci.nih.gov/chemical/structure/%FILE/file?format=sdf&get3d=True",
				"$$": "http://cactus.nci.nih.gov/chemical/structure/%FILE/file?format=sdf",
				"=": "http://www.rcsb.org/pdb/files/%FILE.pdb",
				"==": "http://www.rcsb.org/pdb/files/ligand/%FILE.cif",
				":": "http://pubchem.ncbi.nlm.nih.gov/rest/pug/compound/%FILE/SDF?record_type=3d"
			},
			_restQueryUrl: "http://www.rcsb.org/pdb/rest/search",
			_restQueryXml: "<orgPdbQuery><queryType>org.pdb.query.simple.AdvancedKeywordQuery</queryType><description>Text Search</description><keywords>QUERY</keywords></orgPdbQuery>",
			_restReportUrl: "http://www.pdb.org/pdb/rest/customReport?pdbids=IDLIST&customReportColumns=structureId,structureTitle"
		},
		_debugAlert: false,
		_document: document,
		_execLog: "",
		_execStack: [],
		_isMsie: (navigator.userAgent.toLowerCase().indexOf("msie") >= 0),
		_isXHTML: false,
		_lastAppletID: null,
    _mousePageX: null,
		_serverUrl: "http://chemapps.stolaf.edu/jmol/jsmol.jsmol.php",
    _touching: false,
		_XhtmlElement: null,
		_XhtmlAppendChild: false
	}
})(document);


(function (Jmol, $) {

// this library is organized into the following sections:

  // jQuery interface
  // protected variables
  // feature detection
  // AJAX-related core functionality
  // applet start-up functionality
  // misc core functionality
  // mouse events


  ////////////////////// jQuery interface ///////////////////////
  
  // hooks to jQuery -- if you have a different AJAX tool, feel free to adapt.
  // There should be no other references to jQuery in all the JSmol libraries.

  Jmol.$ = function(appletOrId, what) {
		return $("#" + appletOrId._id + (what ? "_" + what : ""));
	}	

  Jmol.$ajax = function (info) {
    return $.ajax(info);
  }

  Jmol.$attr = function (id, a, val) {
    return $("#" + id).attr(a, val);
  }
  
  Jmol.$bind = function(id, list, f) {
    return (f ? $(id).bind(list, f) : $(id).unbind(list));
  }

  Jmol.$html = function(id, html) {
    return $("#" + id).html(html);
  }
   
  Jmol.$offset = function (id) {
    return $("#" + id).offset();
  }
  
  Jmol.$on = function (evt, f) {
    return $(window).on(evt, f);
  }

  Jmol.$resize = function (f) {
    return $(window).resize(f);
  }
  
  Jmol.$submit = function(form) {
    return $("#" + form).submit();
  }

  Jmol.$val = function (id, v) {
    return $("#" + id).val(v);
  }
  
  /*
   * jQuery outside events - v1.1 - 3/16/2010
   * http://benalman.com/projects/jquery-outside-events-plugin/
   * 
   * Copyright (c) 2010 "Cowboy" Ben Alman
   * Dual licensed under the MIT and GPL licenses.
   * http://benalman.com/about/license/
   * 
   * modified by Bob Hanson to streamline and to add parameter reference to actual jQuery event
   *   
   */
  ;(function($,c,b){
  $.map("click mousemove mouseup touchmove touchend".split(" "),function(d){a(d)});
  function a(g){
  	var e=g+b;
  	var d=$(),h=g+"."+e+"-special-event";
  	$.event.special[e]={
  		setup:function(){
  			d=d.add(this);
  			if(d.length===1){
  				$(c).bind(h,f)
  			}
  		},
  		teardown:function(){
  			d=d.not(this);
  			if(d.length===0){
  				$(c).unbind(h)
  			}
  		},
  		add:function(i){
  			var j=i.handler;
  			i.handler=function(l,k){
  				l.target=k;
  				j.apply(this,arguments)
  			}
  		}
  	};
  	function f(ev){
  		$(d).each(function(){
  			var j=$(this);
  			if(this!==ev.target&&!j.has(ev.target).length){
  	//BH: adds ev to pass that along to our handler as well.
  				j.triggerHandler(e,[ev.target, ev])
  			}
  		}
  		)
  	}
  }
  })(jQuery,document,"outjsmol");
  
  // source: https://github.com/dkastner/jquery.iecors
  // author: Derek Kastner dkastner@gmail.com http://dkastner.github.com
  
  // MSIE cross-domain request
  
  ;(function( jQuery ) {
  
    // Create the request object
    // (This is still attached to ajaxSettings for backward compatibility)
    jQuery.ajaxSettings.xdr = function() {
      return (window.XDomainRequest ? new window.XDomainRequest() : null);
    };
  
    // Determine support properties
    (function( xdr ) {
      jQuery.extend( jQuery.support, { iecors: !!xdr });
    })( jQuery.ajaxSettings.xdr() );
  
    // Create transport if the browser can provide an xdr
    if ( jQuery.support.iecors ) {
  
      jQuery.ajaxTransport(function( s ) {
        var callback;
        return {
          send: function( headers, complete ) {
            var xdr = s.xdr();
            xdr.onload = function() {          
              var headers = { 'Content-Type': xdr.contentType };
              complete(200, 'OK', { text: xdr.responseText }, headers);
            };
  
  
            // Apply custom fields if provided
  					if ( s.xhrFields ) {
              xdr.onerror = s.xhrFields.error;
              xdr.ontimeout = s.xhrFields.timeout;
  					}
  //if (!xdr.onerror)xdr.onerror=function(a){alert("xdr error:" + a)}
  //if (!xdr.onprogress)xdr.onprogress=function(a,b,c){alert("xdr progress:" + this.responseText)}
  //if (!xdr.ontimeout)xdr.ontimeout=function(a){alert("xdr timeout:" + a)}
  
  // note that xdr is not synchronous
  
  
            xdr.open( s.type, s.url );
  
            // XDR has no method for setting headers O_o
  
            xdr.send( ( s.hasContent && s.data ) || null );
            
          },
  
          abort: function() {
          
            xdr.abort();
          }
        };
      });
    }
  })( jQuery );
  // end of jQuery.iecors
  
  ////////////// protected variables ///////////
  

  Jmol._clearVars = function() {
    // only on page closing -- appears to improve garbage collection
    
    delete jQuery;
    delete $;
    delete Jmol;

    if (!java)return;	

    delete J;
    delete JZ;
    delete java;
    delete Clazz;
    delete JavaObject;
    delete bhtest;
    delete xxxbhparams;
    delete xxxShowParams;
    delete c$;
    delete d$;
    delete w$;      
    delete $_A;
    delete $_AB;
    delete $_AC;
    delete $_AD;
    delete $_AF;
    delete $_AI;
    delete $_AL;
    delete $_AS;
    delete $_Ab;
    delete $_B;
    delete $_C;
    delete $_D;
    delete $_E;
    delete $_F;
    delete $_G;
    delete $_H;
    delete $_I;
    delete $_J;
    delete $_K;
    delete $_L;
    delete $_M;
    delete $_N;
    delete $_O;
    delete $_P;
    delete $_Q;
    delete $_R;
    delete $_S;
    delete $_T;
    delete $_U;
    delete $_V;
    delete $_W;
    delete $_X;
    delete $_Y;
    delete $_Z;
    delete $_k;
    delete $_s;
    delete $t$;
  }
  
  ////////////// feature detection ///////////////
  
  Jmol.featureDetection = (function(document, window) {
		
		var features = {};
		features.ua = navigator.userAgent.toLowerCase()
		
		features.os = function(){
			var osList = ["linux","unix","mac","win"]
			var i = osList.length;
			
			while (i--){
				if (features.ua.indexOf(osList[i])!=-1) return osList[i]
			}
			return "unknown";
		}
		
		features.browser = function(){
			var ua = features.ua;
			var browserList = ["konqueror","webkit","omniweb","opera","webtv","icab","msie","mozilla"];
			for (var i = 0; i < browserList.length; i++)
				if (ua.indexOf(browserList[i])>=0) 
					return browserList[i];
			return "unknown";
		}
		features.browserName = features.browser();
	  features.browserVersion= parseFloat(features.ua.substring(features.ua.indexOf(features.browserName)+features.browserName.length+1));
	  
		features.supportsXhr2 = function() {return ($.support.cors || $.support.iecors)}
    features.allowDestroy = (features.browserName != "msie");
    features.allowHTML5 = (features.browserName != "msie" || navigator.appVersion.indexOf("MSIE 8") < 0);
    
    //alert(features.allowHTML5 + " " + features.browserName + " " +  navigator.appVersion)
    
    features.getDefaultLanguage = function() {
      return navigator.language || navigator.userLanguage || "en-US";
    };
    
		features._webGLtest = 0;
		
		features.supportsWebGL = function() {
			if (!Jmol.featureDetection._webGLtest) { 
				var canvas;
				Jmol.featureDetection._webGLtest = ( 
					window.WebGLRenderingContext 
						&& ((canvas = document.createElement("canvas")).getContext("webgl") 
							|| canvas.getContext("experimental-webgl")) ? 1 : -1);
			}
			return (Jmol.featureDetection._webGLtest > 0);
		};
		
    features.supportsLocalization = function() {
     //<meta charset="utf-8">                                     
      var metas = document.getElementsByTagName('meta'); 
      for (var i= metas.length; --i >= 0;) 
        if (metas[i].outerHTML.toLowerCase().indexOf("utf-8") >= 0) return true;
      return false;
    };
   
		features.supportsJava = function() {
			if (!Jmol.featureDetection._javaEnabled) {
				if (Jmol._isMsie) {
				  return true;
				  // sorry just can't deal with intentionally turning off Java in MSIE
				} else {
				  Jmol.featureDetection._javaEnabled = (navigator.javaEnabled() ? 1 : -1);
				}
			}
			return (Jmol.featureDetection._javaEnabled > 0);
    };
			
		features.compliantBrowser = function() {
			var a = !!document.getElementById;
			var os = features.os()
			// known exceptions (old browsers):
	  		if (features.browserName == "opera" && features.browserVersion <= 7.54 && os == "mac"
			      || features.browserName == "webkit" && features.browserVersion < 125.12
			      || features.browserName == "msie" && os == "mac"
			      || features.browserName == "konqueror" && features.browserVersion <= 3.3
			    ) a = false;
			return a;
		}
		
		features.isFullyCompliant = function() {
			return features.compliantBrowser() && features.supportsJava();
		}
	  	
	  features.useIEObject = (features.os() == "win" && features.browserName == "msie" && features.browserVersion >= 5.5);
	  features.useHtml4Object = (features.browserName == "mozilla" && features.browserVersion >= 5) ||
	   		(features.browserName == "opera" && features.browserVersion >= 8) ||
	   		(features.browserName == "webkit" && features.browserVersion >= 412.2);
        
		return features;
		
	})(document, window);

    
  	////////////// AJAX-related core functionality //////////////

	Jmol._ajax = function(info) {
	  if (!info.async) {
	  	return Jmol.$ajax(info).responseText;
	  }
		Jmol._ajaxQueue.push(info)
		if (Jmol._ajaxQueue.length == 1)
			Jmol._ajaxDone()
	}
	Jmol._ajaxDone = function() {
		var info = Jmol._ajaxQueue.shift();
		info && Jmol.$ajax(info);
	}
	
	Jmol._grabberOptions = [
	  ["$", "NCI(small molecules)"],
	  [":", "PubChem(small molecules)"],
	  ["=", "RCSB(macromolecules)"]
	];
	
	Jmol._getGrabberOptions = function(applet, note) {
		// feel free to adjust this look to anything you want
		if (Jmol._grabberOptions.length == 0)
			return ""
		var s = '<input type="text" id="ID_query" onkeypress="13==event.which&&Jmol._applets[\'ID\']._search()" size="32" value="" />';
		var b = '<button id="ID_submit" onclick="Jmol._applets[\'ID\']._search()">Search</button></nobr>'
		if (Jmol._grabberOptions.length == 1) {
			s = '<nobr>' + s + '<span style="display:none">';
			b = '</span>' + b;
		} else {
			s += '<br /><nobr>'
		}
		s += '<select id="ID_select">'
		for (var i = 0; i < Jmol._grabberOptions.length; i++) {
			var opt = Jmol._grabberOptions[i];
		 	s += '<option value="' + opt[0] + '" ' + (i == 0 ? 'selected' : '') + '>' + opt[1] + '</option>';
		}
		s = (s + '</select>' + b).replace(/ID/g, applet._id) + (note ? note : "");
		return '<br />' + s;
	}

	Jmol._saveFile = function(filename, mimetype, data, encoding) {
		var url = Jmol._serverUrl;
		if (!url) {
			// do something local here;
			return;
		}
		Jmol.$attr("__jsmolform__", "action", url + "?" + (new Date()).getMilliseconds());
		Jmol.$val("__jsmoldata__", data);
		Jmol.$val("__jsmolfilename__", filename);
		Jmol.$val("__jsmolmimetype__", mimetype);
		Jmol.$val("__jsmolencoding__", encoding);
		Jmol.$submit("__jsmolform__");
		Jmol.$val("__jsmoldata__", "");
	}
	
	Jmol._getScriptForDatabase = function(database) {
		return (database == "$" ? Jmol.db._nciLoadScript : database == ":" ? Jmol.db._pubChemLoadScript : Jmol.db._fileLoadScript);
	}
	
   //   <dataset><record><structureId>1BLU</structureId><structureTitle>STRUCTURE OF THE 2[4FE-4S] FERREDOXIN FROM CHROMATIUM VINOSUM</structureTitle></record><record><structureId>3EUN</structureId><structureTitle>Crystal structure of the 2[4Fe-4S] C57A ferredoxin variant from allochromatium vinosum</structureTitle></record></dataset>
      
	Jmol._setInfo = function(applet, database, data) {
		var info = [];
		var header = "";
		if (data.indexOf("ERROR") == 0)
			header = data;
		else
			switch (database) {
			case "=":
				var S = data.split("<dimStructure.structureId>");
				var info = ["<table>"];
				for (var i = 1; i < S.length; i++) {
					info.push("<tr><td valign=top><a href=\"javascript:Jmol.search(" + applet._id + ",'=" + S[i].substring(0, 4) + "')\">" + S[i].substring(0, 4) + "</a></td>");
					info.push("<td>" + S[i].split("Title>")[1].split("</")[0] + "</td></tr>");
				}
				info.push("</table>");
				header = (S.length - 1) + " matches";
				break;			
			case "$": // NCI
			case ":": // pubChem
			break;
			default:
				return;
		}
		applet._infoHeader = header;
		applet._info = info.join("");
		applet._showInfo(true);
	}
	
	Jmol._loadSuccess = function(a, fSuccess) {
	  if (!fSuccess)
	    return;
		Jmol._ajaxDone();
		fSuccess(a);
	}

	Jmol._loadError = function(fError){
		Jmol._ajaxDone();
		Jmol.say("Error connecting to server.");	
		null!=fError&&fError()
	}
	
	Jmol._isDatabaseCall = function(query) {
		return (Jmol.db._databasePrefixes.indexOf(query.substring(0, 1)) >= 0);
	}
	
	Jmol._getDirectDatabaseCall = function(query, checkXhr2) {
		if (checkXhr2 && !Jmol.featureDetection.supportsXhr2())
			return query;
		var pt = 2;
		var db;
		var call = Jmol.db._DirectDatabaseCalls[query.substring(0,pt)];
		if (!call)
			call = Jmol.db._DirectDatabaseCalls[db = query.substring(0,--pt)];
		if (call && db == ":") {
			var ql = query.toLowerCase();
			if (!isNaN(parseInt(query.substring(1)))) {
				query = ":cid/" + query.substring(1);
			} else if (ql.indexOf(":smiles:") == 0) {
				call += "?POST?smiles=" + query.substring(8);
				query = ":smiles";
			} else if (ql.indexOf(":cid:") == 0) {
				query = ":cid/" + query.substring(5);
			} else {
				if (ql.indexOf(":name:") == 0)
					query = query.substring(5);
				else if (ql.indexOf(":cas:") == 0)
					query = query.substring(4);
				query = ":name/" + encodeURIComponent(query.substring(1));
			}
		}
		query = (call ? call.replace(/\%FILE/, query.substring(pt)) : query);
		return query;
	}
	
	Jmol._getRawDataFromServer = function(database,query,fSuccess,fError,asBase64,noScript){
		var s = 
			"?call=getRawDataFromDatabase&database=" + database
				+ "&query=" + encodeURIComponent(query)
				+ (asBase64 ? "&encoding=base64" : "")
				+ (noScript ? "" : "&script=" + encodeURIComponent(Jmol._getScriptForDatabase(database)));
		return Jmol._contactServer(s, fSuccess, fError);
	}
	
	Jmol._getInfoFromDatabase = function(applet, database, query){
		if (database == "====") {
			var data = Jmol.db._restQueryXml.replace(/QUERY/,query);
			var info = {
				dataType: "text",
				type: "POST",
				contentType:"application/x-www-form-urlencoded",
				url: Jmol.db._restQueryUrl,
				data: encodeURIComponent(data) + "&req=browser",
				success: function(data) {Jmol._ajaxDone();Jmol._extractInfoFromRCSB(applet, database, query, data)},
				error: function() {Jmol._loadError(null)},
				async: Jmol._asynchronous
			}
			return Jmol._ajax(info);
		}		
		query = "?call=getInfoFromDatabase&database=" + database
				+ "&query=" + encodeURIComponent(query);
		return Jmol._contactServer(query, function(data) {Jmol._setInfo(applet, database, data)});
	}
	
	Jmol._extractInfoFromRCSB = function(applet, database, query, output) {
		var n = output.length/5;
		if (n == 0)
			return;	
		if (query.length == 4 && n != 1) {
			var QQQQ = query.toUpperCase();
			var pt = output.indexOf(QQQQ);
			if (pt > 0 && "123456789".indexOf(QQQQ.substring(0, 1)) >= 0)
				output = QQQQ + "," + output.substring(0, pt) + output.substring(pt + 5);
			if (n > 50)
				output = output.substring(0, 250);
			output = output.replace(/\n/g,",");
			var url = Jmol._restReportUrl.replace(/IDLIST/,output);
			Jmol._loadFileData(applet, url, function(data) {Jmol._setInfo(applet, database, data) });		
		}
	}

	Jmol._loadFileData = function(applet, fileName, fSuccess, fError){
		if (Jmol._isDatabaseCall(fileName)) {
			Jmol._setQueryTerm(applet, fileName);
			fileName = Jmol._getDirectDatabaseCall(fileName, true);
			
			if (Jmol._isDatabaseCall(fileName)) {
				// xhr2 not supported (MSIE)
				fileName = Jmol._getDirectDatabaseCall(fileName, false);
				Jmol._getRawDataFromServer("_",fileName,fSuccess,fError);		
				return;
			}
		}	
		var info = {
			dataType: "text",
			url: fileName,
			async: Jmol._asynchronous,
			success: function(a) {Jmol._loadSuccess(a, fSuccess)},
			error: function() {Jmol._loadError(fError)}
		}
		var pt = fileName.indexOf("?POST?");
		if (pt > 0) {
			info.url = fileName.substring(0, pt);
			info.data = fileName.substring(pt + 6);
			info.type = "POST";
			info.contentType = "application/x-www-form-urlencoded";
		}
		Jmol._ajax(info);
	}
	
	Jmol._contactServer = function(data,fSuccess,fError){
		var info = {
			dataType: "text",
			type: "GET",
			url: Jmol._serverUrl + data,
			success: function(a) {Jmol._loadSuccess(a, fSuccess)},
			error:function() { Jmol._loadError(fError) },
			async:fSuccess ? Jmol._asynchronous : false
		}
		return Jmol._ajax(info);
	}
	
	Jmol._setQueryTerm = function(applet, query) {
		if (!query || !applet._hasOptions || query.substring(0, 7) == "http://")
			return;
		if (Jmol._isDatabaseCall(query)) {
			var database = query.substring(0, 1);
			query = query.substring(1);
			if (database == "=" && query.length == 4 && query.substring(0, 1) == "=")
				query = query.substring(1);
			var d = Jmol._getElement(applet, "select");
			if (d.options)
				for (var i = 0; i < d.options.length; i++)
					if (d[i].value == database)
						d[i].selected = true;
		}
		Jmol._getElement(applet, "query").value = query;
	}

  Jmol._search = function(applet, query, script) {
  	arguments.length > 1 || (query = null);
  	Jmol._setQueryTerm(applet, query);
  	query || (query = Jmol._getElement(applet, "query").value);
    if (query.indexOf("!") == 0) {
    // command prompt in this box as well
      applet._script(query);
      return;
    } else if (query) {
  		query = query.replace(/\"/g, "");
    }
  	applet._showInfo(false);
  	var database;
  	if (Jmol._isDatabaseCall(query)) {
  		database = query.substring(0, 1);
  		query = query.substring(1);
  	} else {
  		database = (applet._hasOptions ? Jmol._getElement(applet, "select").value : "$");
  	}
  	if (database == "=" && query.length == 3)
  		query = "=" + query; // this is a ligand			
  	var dm = database + query;
  	if (!query || dm.indexOf("?") < 0 && dm == applet._thisJmolModel) {
  		return;    
  	}
  	applet._thisJmolModel = dm;
  	if (database == "$" || database == ":")
  		applet._jmolFileType = "MOL";
  	else if (database == "=")
  		applet._jmolFileType = "PDB";
  	applet._searchDatabase(query, database, script);
  }
  	
  Jmol._searchDatabase = function(applet, query, database, script) {
		applet._showInfo(false);
		if (query.indexOf("?") >= 0) {
			Jmol._getInfoFromDatabase(applet, database, query.split("?")[0]);
			return true;
		}
		if (Jmol.db._DirectDatabaseCalls[database]) {
			applet._loadFile(database + query, script);
			return true;
		}
		return false;
	}
  	
	Jmol._syncBinaryOK="?";
	
	Jmol._canSyncBinary = function() {
		if (self.VBArray) return (Jmol._syncBinaryOK = false);
	  if (Jmol._syncBinaryOK != "?") return Jmol._syncBinaryOK;
	  Jmol._syncBinaryOK = true;
		try {
			var xhr = new window.XMLHttpRequest();
		  xhr.open( "text", "http://google.com", false );
		  if (xhr.hasOwnProperty("responseType")) {
		    xhr.responseType = "arraybuffer";
		  } else if (xhr.overrideMimeType) {
		    xhr.overrideMimeType('text/plain; charset=x-user-defined');
		  }
		} catch( e ) {
      System.out.println("JmolCore.js: synchronous binary file transfer is not available");
			return Jmol._syncBinaryOK = false;
		}
		return true;	
	}

	Jmol._binaryTypes = [".gz",".jpg",".png",".zip",".jmol",".bin",".smol",".spartan",".mrc",".pse"]; // mrc? O? others?
	
  Jmol._isBinaryUrl = function(url) {
  	for (var i = Jmol._binaryTypes.length; --i >= 0;)
  		if (url.indexOf(Jmol._binaryTypes[i]) >= 0) return true;
  	return false;
  }
  
  Jmol._getFileData = function(fileName) {
  	// use host-server PHP relay if not from this host
    var type = (Jmol._isBinaryUrl(fileName) ? "binary" : "text");
    var asBase64 = ((type == "binary") && !Jmol._canSyncBinary());
    var isPost = (fileName.indexOf("?POST?") >= 0);
    if (fileName.indexOf("file:/") == 0 && fileName.indexOf("file:///") != 0)
      fileName = "file://" + fileName.substring(5);      /// fixes IE problem
    var isMyHost = (fileName.indexOf("://") < 0 || fileName.indexOf(document.location.protocol) == 0 && fileName.indexOf(document.location.host) >= 0);
    var isDirectCall = Jmol._isDirectCall(fileName);
    var cantDoSynchronousLoad = (!isMyHost && $.support.iecors);
  	if (cantDoSynchronousLoad || asBase64 || !isMyHost && !isDirectCall)
		  return Jmol._getRawDataFromServer("_",fileName, null, null, asBase64, true);
		
		var info = {dataType:type,async:false};
		if (isPost) {
			info.type = "POST";
			info.url = fileName.split("?POST?")[0]
			info.data = fileName.split("?POST?")[1]
		} else {
			info.url = fileName;
		}
		var xhr = Jmol.$ajax(info);
		if (self.Clazz && Clazz.instanceOf(xhr.response, self.ArrayBuffer)) {
		  // Safari
		  return xhr.response;
		} 
		return xhr.responseText;
	}
	
	Jmol._isDirectCall = function(url) {
		for (var key in Jmol.db._DirectDatabaseCalls) {
			if (key.indexOf(".") >= 0 && url.indexOf(key) >= 0)
				return true;
		}
		return false;
	}

	Jmol._cleanFileData = function(data) {
		if (data.indexOf("\r") >= 0 && data.indexOf("\n") >= 0) {
			return data.replace(/\r\n/g,"\n");
		}
		if (data.indexOf("\r") >= 0) {
			return data.replace(/\r/g,"\n");
		}
		return data;
	};

	Jmol._getFileType = function(name) {
		var database = name.substring(0, 1);
		if (database == "$" || database == ":")
			return "MOL";
		if (database == "=")
			return (name.substring(1,2) == "=" ? "LCIF" : "PDB");
		// just the extension, which must be PDB, XYZ..., CIF, or MOL
		name = name.split('.').pop().toUpperCase();
		return name.substring(0, Math.min(name.length, 3));
	};

  Jmol._scriptLoad = function(app, file, params, doload) {
    var doscript = (app._isJava || !app._noscript || params.length > 1);
    if (doscript)
  	  app._script("zap;set echo middle center;echo Retrieving data...");
  	if (!doload)
      return false;
    if (doscript)
  	  app._script("load \"" + file + "\"" + params);
    else
      app._applet.viewer.openFile(file);
    app._checkDeferred("");
    return true;
  }

	////////////// applet start-up functionality //////////////

  Jmol._setConsoleDiv = function (d) {
  	if (!self.Clazz)return;
  	Clazz.setConsoleDiv(d);
  }

  Jmol._setJmolParams = function(params, Info, isHashtable) {      
		var availableValues = "'progressbar','progresscolor','boxbgcolor','boxfgcolor','allowjavascript','boxmessage',\
									'messagecallback','pickcallback','animframecallback','appletreadycallback','atommovedcallback',\
									'echocallback','evalcallback','hovercallback','language','loadstructcallback','measurecallback',\
									'minimizationcallback','resizecallback','scriptcallback','statusform','statustext','statustextarea',\
									'synccallback','usecommandthread'";
		for (var i in Info)
			if(availableValues.indexOf("'" + i.toLowerCase() + "'") >= 0){
        if (i == "language" && !Jmol.featureDetection.supportsLocalization())continue;
        if (isHashtable)
          params.put(i, (Info[i] === true ? Boolean.TRUE: Info[i] === false ? Boolean.FALSE : Info[i]))
        else
				  params[i] = Info[i];
      }
	}			
   
	Jmol._registerApplet = function(id, applet) {
		return window[id] = Jmol._applets[id] = Jmol._applets[applet] = applet;
	}	

  Jmol._readyCallback = function (a,b,c,d) {
    var app = a.split("_object")[0];
		// necessary for MSIE in strict mode -- apparently, we can't call 
		// jmol._readyCallback, but we can call Jmol._readyCallback. Go figure...

		Jmol._applets[app]._readyCallback(a,b,c,d);
	}

	Jmol._getWrapper = function(applet, isHeader) {
		var height = applet._height;
		var width = applet._width;
		if (typeof height !== "string" || height.indexOf("%") < 0)
			height += "px";
		if (typeof width !== "string" || width.indexOf("%") < 0)
			width += "px";
			
			// id_appletinfotablediv
			//     id_appletdiv
			//     id_coverdiv
			//     id_infotablediv
			//       id_infoheaderdiv
			//          id_infoheaderspan
			//          id_infocheckboxspan
			//       id_infodiv
			
			
			// for whatever reason, without DOCTYPE, with MSIE, "height:auto" does not work, 
			// and the text scrolls off the page.
			// So I'm using height:95% here.
			// The table was a fix for MSIE with no DOCTYPE tag to fix the miscalculation
			// in height of the div when using 95% for height. 
			// But it turns out the table has problems with DOCTYPE tags, so that's out. 
			// The 95% is a compromise that we need until the no-DOCTYPE MSIE solution is found. 
			// (100% does not work with the JME linked applet)
			
      var img = "";  
      if (applet._coverImage){
        var more = " onclick=\"Jmol.coverApplet(ID, false)\" title=\"" + applet._coverTitle + "\"";
        var play = "<image id=\"ID_coverclickgo\" src=\"" + applet._j2sPath + "/img/play_make_live.jpg\" style=\"width:25px;height:25px;position:absolute;bottom:10px;left:10px;z-index:10001;opacity:0.5;\"" + more + " />"  
        img = "<div id=\"ID_coverdiv\" style=\"backgoround-color:red;z-index:10000;width:100%;height:100%;display:inline;position:absolute;top:0px;left:0px\"><image id=\"ID_coverimage\" src=\""
         + applet._coverImage + "\" style=\"width:100%;height:100%\"" + more + "/>" + play + "</div>";
      }
      var sform = (!isHeader && !Jmol._formdiv ? 
			 '<div id="__jsmolformdiv__" style="display:none">\
				<form id="__jsmolform__" method="post" target="_blank" action="">\
				<input name="call" value="saveFile"/>\
				<input id="__jsmolmimetype__" name="mimetype" value=""/>\
				<input id="__jsmolencoding__" name="encoding" value=""/>\
				<input id="__jsmolfilename__" name="filename" value=""/>\
				<input id="__jsmoldata__" name="data" value=""/>\
				</form>\
				</div>': ""); 
			if (sform)
				Jmol._formdiv = "__jsmolform__";
			var s = (isHeader ? "<div id=\"ID_appletinfotablediv\" style=\"width:Wpx;height:Hpx;position:relative\">IMG<div id=\"ID_appletdiv\" style=\"z-index:9999;width:100%;height:100%;position:absolute:top:0px;left:0px;\">"
				: "</div><div id=\"ID_infotablediv\" style=\"width:100%;height:100%;position:absolute;top:0px;left:0px\">\
			<div id=\"ID_infoheaderdiv\" style=\"height:20px;width:100%;background:yellow;display:none\"><span id=\"ID_infoheaderspan\"></span><span id=\"ID_infocheckboxspan\" style=\"position:absolute;text-align:right;right:1px;\"><a href=\"javascript:Jmol.showInfo(ID,false)\">[x]</a></span></div>\
			<div id=\"ID_infodiv\" style=\"position:absolute;top:20px;bottom:0;width:100%;height:95%;overflow:auto\"></div></div></div>"+sform);
		return s.replace(/IMG/, img).replace(/Hpx/g, height).replace(/Wpx/g, width).replace(/ID/g, applet._id);
	}

	Jmol._documentWrite = function(text) {
		if (Jmol._document) {
			if (Jmol._isXHTML && !Jmol._XhtmlElement) {
				var s = document.getElementsByTagName("script");
				Jmol._XhtmlElement = s.item(s.length - 1);
				Jmol._XhtmlAppendChild = false;
			}
			if (Jmol._XhtmlElement)
				Jmol._domWrite(text);
			else
				Jmol._document.write(text);
			return null;
		}
		return text;
	}

	Jmol._domWrite = function(data) {
	  var pt = 0
	  var Ptr = []
	  Ptr[0] = 0
	  while (Ptr[0] < data.length) {
	    var child = Jmol._getDomElement(data, Ptr);
	    if (!child)
				break;
	    if (Jmol._XhtmlAppendChild)
	      Jmol._XhtmlElement.appendChild(child);
	    else
	      Jmol._XhtmlElement.parentNode.insertBefore(child, _jmol.XhtmlElement);
	  }
	}
	
	Jmol._getDomElement = function(data, Ptr, closetag, lvel) {

		// there is no "document.write" in XHTML
	
		var e = document.createElement("span");
		e.innerHTML = data;
		Ptr[0] = data.length;

/*
	// unnecessary ?	

		closetag || (closetag = "");
		lvel || (lvel = 0);
		var pt0 = Ptr[0];
		var pt = pt0;
		while (pt < data.length && data.charAt(pt) != "<") 
			pt++
		if (pt != pt0) {
			var text = data.substring(pt0, pt);
			Ptr[0] = pt;
			return document.createTextNode(text);
		}
		pt0 = ++pt;
		var ch;
		while (pt < data.length && "\n\r\t >".indexOf(ch = data.charAt(pt)) < 0) 
			pt++;
		var tagname = data.substring(pt0, pt);
		var e = (tagname == closetag	|| tagname == "/" ? ""
			: document.createElementNS ? document.createElementNS('http://www.w3.org/1999/xhtml', tagname)
			: document.createElement(tagname));
		if (ch == ">") {
			Ptr[0] = ++pt;
			return e;
		}
		while (pt < data.length && (ch = data.charAt(pt)) != ">") {
			while (pt < data.length && "\n\r\t ".indexOf(ch = data.charAt(pt)) >= 0) 
				pt++;
			pt0 = pt;
			while (pt < data.length && "\n\r\t =/>".indexOf(ch = data.charAt(pt)) < 0) 
				pt++;
			var attrname = data.substring(pt0, pt).toLowerCase();
			if (attrname && ch != "=")
				e.setAttribute(attrname, "true");
			while (pt < data.length && "\n\r\t ".indexOf(ch = data.charAt(pt)) >= 0) 
				pt++;
			if (ch == "/") {
				Ptr[0] = pt + 2;
				return e;
			} else if (ch == "=") {
				var quote = data.charAt(++pt);
				pt0 = ++pt;
				while (pt < data.length && (ch = data.charAt(pt)) != quote) 
					pt++;
				var attrvalue = data.substring(pt0, pt);
				e.setAttribute(attrname, attrvalue);
				pt++;
			}
		}
		Ptr[0] = ++pt;
		while (Ptr[0] < data.length) {
			var child = Jmol._getDomElement(data, Ptr, "/" + tagname, lvel+1);
			if (!child)
				break;
			e.appendChild(child);
		}
*/
		return e;    
	}
	
	Jmol._setObject = function(obj, id, Info) {
  	obj._id = id;
    obj.__Info = {};  
    for (var i in Info)
      obj.__Info[i] = Info[i];
		obj._width = Info.width;
		obj._height = Info.height;
    obj._noscript = !obj._isJava && Info.noscript;
    obj._console = Info.console;


		if (!obj._console)
			obj._console = obj._id + "_infodiv";
		if (obj._console == "none")
			obj._console = null;
      
		obj._color = (Info.color ? Info.color.replace(/0x/,"#") : "#FFFFFF");
		obj._disableInitialConsole = Info.disableInitialConsole;
		obj._noMonitor = Info.disableJ2SLoadMonitor;
		obj._j2sPath = Info.j2sPath;
    obj._deferApplet = Info.deferApplet;
    obj._deferUncover = Info.deferUncover;
    obj._coverImage = !obj._isJava && Info.coverImage;

    obj._isCovered = !!obj._coverImage; 
    obj._coverScript = Info.coverScript;
    obj._coverTitle = Info.coverTitle;

    if (!obj._coverTitle)
      obj._coverTitle = (obj._deferApplet ? "activate 3D model" : "3D model is loading...")
    obj._containerWidth = obj._width + ((obj._width==parseFloat(obj._width))? "px":"");
		obj._containerHeight = obj._height + ((obj._height==parseFloat(obj._height))? "px":"");
		obj._info = "";
		obj._infoHeader = obj._jmolType + ' "' + obj._id + '"'
		obj._hasOptions = Info.addSelectionOptions;
		obj._defaultModel = Info.defaultModel;
		obj._readyScript = (Info.script ? Info.script : "");
		obj._readyFunction = Info.readyFunction;
    if (obj._coverImage && !obj._deferApplet)
      obj._readyScript += ";javascript " + id + "._displayCoverImage(false)";
		obj._src = Info.src;

	}

	Jmol._addDefaultInfo = function(Info, DefaultInfo) {
		for (var x in DefaultInfo)
		  if (typeof Info[x] == "undefined")
		  	Info[x] = DefaultInfo[x];
	}
	
	Jmol._syncedApplets = [];
	Jmol._syncedCommands = [];
	Jmol._syncedReady = [];
	Jmol._syncReady = false;
  Jmol._isJmolJSVSync = false;

  Jmol._setReady = function(applet) {
    Jmol._syncedReady[applet] = 1;
    var n = 0;
    for (var i = 0; i < Jmol._syncedApplets.length; i++) {
      if (Jmol._syncedApplets[i] == applet._id) {
        Jmol._syncedApplets[i] = applet;
        Jmol._syncedReady[i] = 1;
      } else if (!Jmol._syncedReady[i]) {
        continue;
      }
      n++;
		}
		if (n != Jmol._syncedApplets.length)
			return;
		Jmol._setSyncReady();
	}

  Jmol._setDestroy = function(applet) {
    //MSIE bug responds to any link click even if it is just a JavaScript call
    
    if (Jmol.featureDetection.allowDestroy)
      Jmol.$on('beforeunload', function () { Jmol._destroy(applet); } );
  }
  
  Jmol._destroy = function(applet) {
    try {
      if (applet._applet) applet._applet.destroy();
      applet._applet = null;
      Jmol._unsetMouse(applet._canvas)
      applet._canvas = null;
      var n = 0;
      for (var i = 0; i < Jmol._syncedApplets.length; i++) {
        if (Jmol._syncedApplets[i] == applet)
          Jmol._syncedApplets[i] = null;
        if (Jmol._syncedApplets[i])
          n++;
  		}
  		if (n > 0)
  			return;
  		Jmol._clearVars();
    } catch(e){}
	}

	////////////// misc core functionality //////////////

	Jmol._setSyncReady = function() {
	  Jmol._syncReady = true;
	  var s = ""
    for (var i = 0; i < Jmol._syncedApplets.length; i++)
    	if (Jmol._syncedCommands[i])
        s += "Jmol.script(Jmol._syncedApplets[" + i + "], Jmol._syncedCommands[" + i + "]);"
    setTimeout(s, 50);  
	}

	Jmol._mySyncCallback = function(app,msg) {
	  if (!Jmol._syncReady || !Jmol._isJmolJSVSync)
	  	return 1; // continue processing and ignore me
    for (var i = 0; i < Jmol._syncedApplets.length; i++) {
      if (msg.indexOf(Jmol._syncedApplets[i]._syncKeyword) >= 0) {
        Jmol._syncedApplets[i]._syncScript(msg);
      }
    }
	  return 0 // prevents further Jmol sync processing	
	}              

	Jmol._getElement = function(applet, what) {
		var d = document.getElementById(applet._id + "_" + what);
		return (d || {});
	}	
   
	Jmol._evalJSON = function(s,key){
		s = s + "";
		if(!s)
			return [];
		if(s.charAt(0) != "{") {
			if(s.indexOf(" | ") >= 0)
				s = s.replace(/\ \|\ /g, "\n");
			return s;
		}
		var A = (new Function( "return " + s ) )();
		return (!A ? null : key && A[key] != undefined ? A[key] : A);
	}

	Jmol._sortMessages = function(A){
		/*
		 * private function
		 */
		function _sortKey0(a,b){
			return (a[0]<b[0]?1:a[0]>b[0]?-1:0);
		}

		if(!A || typeof (A) != "object")
			return [];
		var B = [];
		for(var i = A.length - 1; i >= 0; i--)
			for(var j = 0, jj= A[i].length; j < jj; j++)
				B[B.length] = A[i][j];
		if(B.length == 0)
			return;
		B = B.sort(_sortKey0);
		return B;
	}

  //////////////////// mouse events //////////////////////
  
	Jmol._jsGetMouseModifiers = function(ev) {
		var modifiers = 0;
		switch (ev.button) {
		case 0:
		  modifiers = 16;//J.api.Event.MOUSE_LEFT;
		  break;
		case 1:
		  modifiers = 8;//J.api.Event.MOUSE_MIDDLE;
		  break;
		case 2:
		  modifiers = 4;//J.api.Event.MOUSE_RIGHT;
		  break;
		}
		if (ev.shiftKey)
		  modifiers += 1;//J.api.Event.SHIFT_MASK;
		if (ev.altKey)
		  modifiers += 8;//J.api.Event.ALT_MASK;
		if (ev.ctrlKey)
		  modifiers += 2;//J.api.Event.CTRL_MASK;
		return modifiers;
	}

	Jmol._jsGetXY = function(canvas, ev) {
    if (!canvas.applet._ready)
      return false;
		ev.preventDefault();
		var offsets = Jmol.$offset(canvas.id);
		var x, y;
		var oe = ev.originalEvent;
		Jmol._mousePageX = ev.pageX;
		Jmol._mousePageY = ev.pageY;
		if (oe.targetTouches && oe.targetTouches[0]) {
			x = oe.targetTouches[0].pageX - offsets.left;
			y = oe.targetTouches[0].pageY - offsets.top;
		} else if (oe.changedTouches) {
			x = oe.changedTouches[0].pageX - offsets.left;
			y = oe.changedTouches[0].pageY - offsets.top;
		} else {
      x = ev.pageX - offsets.left;
      y = ev.pageY - offsets.top;
		}
		return (x == undefined ? null : [Math.round(x), Math.round(y), Jmol._jsGetMouseModifiers(ev)]);
	}

  Jmol._gestureUpdate = function(canvas, ev) {
   	ev.stopPropagation();
  	ev.preventDefault();
    var oe = ev.originalEvent;
    
    switch (ev.type) {
    case "touchstart":
      Jmol._touching = true;
      break;
    case "touchend":
      Jmol._touching = false;
      break;
    }
    if (!oe.touches || oe.touches.length != 2) return false;
    switch (ev.type) {
    case "touchstart":
      canvas._touches = [[],[]];
      break;
    case "touchmove":
			var offsets = Jmol.$offset(canvas.id);
	    canvas._touches[0].push([oe.touches[0].pageX - offsets.left, oe.touches[0].pageY - offsets.top]);
	    canvas._touches[1].push([oe.touches[1].pageX - offsets.left, oe.touches[1].pageY - offsets.top]);
	    if (canvas._touches[0].length >= 2)
				canvas.applet._processGesture(canvas._touches);
      break;
    }
    return true;
  }
  
	Jmol._jsSetMouse = function(canvas) {
		Jmol.$bind(canvas, 'mousedown touchstart', function(ev) {
	   	ev.stopPropagation();
	  	ev.preventDefault();
		  canvas.isDragging = true;
      if ((ev.type == "touchstart") && Jmol._gestureUpdate(canvas, ev))
        return false;
			Jmol._setConsoleDiv(canvas.applet._console);
			var xym = Jmol._jsGetXY(canvas, ev);
			if(!xym)
        return false;
			if (ev.button != 2 && canvas.applet._popups)
				Jmol.Menu.hidePopups(canvas.applet._popups);

			canvas.applet._processEvent(501, xym); //J.api.Event.MOUSE_DOWN
			return false;
		});
		Jmol.$bind(canvas, 'mouseup touchend', function(ev) {
	   	ev.stopPropagation();
	  	ev.preventDefault();
		  canvas.isDragging = false;
      if (ev.type == "touchend" && Jmol._gestureUpdate(canvas, ev))
        return false;
			var xym = Jmol._jsGetXY(canvas, ev);
			if(!xym) return false;
			canvas.applet._processEvent(502, xym);//J.api.Event.MOUSE_UP
			return false;

		});
		Jmol.$bind(canvas, 'mousemove touchmove', function(ev) { // touchmove
     	ev.stopPropagation();
	  	ev.preventDefault();
      var isTouch = (ev.type == "touchmove") || Jmol._touching;
	    if (isTouch && Jmol._gestureUpdate(canvas, ev))
        return false;
			var xym = Jmol._jsGetXY(canvas, ev);
			if(!xym) return false;
      if (!canvas.isDragging)
        xym[2] = 0;
			canvas.applet._processEvent((canvas.isDragging ? 506 : 503), xym); // J.api.Event.MOUSE_DRAG : J.api.Event.MOUSE_MOVE
			return false;
		});
		Jmol.$bind(canvas, 'DOMMouseScroll mousewheel', function(ev) { // Zoom
	   	ev.stopPropagation();
	  	ev.preventDefault();
			// Webkit or Firefox
		  canvas.isDragging = false;
		  var oe = ev.originalEvent;
			var scroll = (oe.detail ? oe.detail : oe.wheelDelta);
			var modifiers = Jmol._jsGetMouseModifiers(ev);
			canvas.applet._processEvent(-1,[scroll < 0 ? -1 : 1,0,modifiers]);
			return false;
		});

		// context menu is fired on mouse down, not up, and it's handled already anyway.
				
		Jmol.$bind(canvas, "contextmenu", function() {return false;});
		
		Jmol.$bind(canvas, 'mouseout', function(ev) {
      if (canvas.applet._applet)
        canvas.applet._applet.viewer.startHoverWatcher(false);
      canvas.isDragging = false;
		});

		Jmol.$bind(canvas, 'mouseenter', function(ev) {
      if (canvas.applet._applet)
        canvas.applet._applet.viewer.startHoverWatcher(true);
  		if (ev.buttons === 0 || ev.which === 0) {
  		  canvas.isDragging = false;
  			var xym = Jmol._jsGetXY(canvas, ev);
        if (!xym) return false;
  			canvas.applet._processEvent(502, xym);//J.api.Event.MOUSE_UP
  		}
		});

    if (canvas.applet._is2D)
    	Jmol.$resize(function() {
        if (!canvas.applet)
          return;
        canvas.applet._resize();
    	});
 
		Jmol.$bind('body', 'mouseup touchend', function(ev) {
      if (canvas.applet)
			  canvas.isDragging = false;
		});

	}

	Jmol._jsUnsetMouse = function(canvas) {
    canvas.applet = null;
		Jmol.$bind(canvas, 'mousedown touchstart mousemove touchmove mouseup touchend DOMMouseScroll mousewheel contextmenu mouseout mouseenter', null);
	}

Jmol._setDraggable = function(Obj) {
	var proto = Obj.prototype;
	// for menus and console
	proto.setContainer = function(container) {
		this.container = container;
		this.isDragging = false;
		this.ignoreMouse = false;
		var me = this;
		container.bind('mousedown touchstart', function(ev) {
			if (me.ignoreMouse) {
				me.ignoreMouse = false;
				return true;
			}
		  me.isDragging = true;
		  me.pageX = ev.pageX;
		  me.pageY = ev.pageY;
		  return false;
		});
		container.bind('mousemove touchmove', function(ev) {
			if (me.isDragging) {
				me.mouseMove(ev);
				return false;
			}
		});
		container.bind('mouseup touchend', function(ev) {
			me.mouseUp(ev);
		});
	};

	proto.mouseUp = function(ev) {
		if (this.isDragging) {
			this.pageX0 += (ev.pageX - this.pageX);
			this.pageY0 += (ev.pageY - this.pageY);
		  this.isDragging = false;
		  return false;
		}
	}
	
	proto.setPosition = function() {
    if (Jmol._mousePageX === null) {
      var id = this.applet._id + "_" + (this.applet._is2D ? "canvas2d" : "canvas");
      var offsets = Jmol.$offset(id);
      Jmol._mousePageX = offsets.left;
      Jmol._mousePageY = offsets.top;
    }
		this.pageX0 = Jmol._mousePageX;
		this.pageY0 = Jmol._mousePageY;
		var pos = { top: Jmol._mousePageY + 'px', left: Jmol._mousePageX + 'px' };
		this.container.css(pos);
	};
	
	proto.mouseMove = function(ev) {
		if (!this.isDragging)
			return;
		var x = this.pageX0 + (ev.pageX - this.pageX);
		var y = this.pageY0 + (ev.pageY - this.pageY);
		this.container.css({ top: y + 'px', left: x + 'px' })
	};
		
	proto.dragBind = function(isBind) {
		this.container.unbind('mousemoveoutjsmol');
		this.container.unbind('touchmoveoutjsmol');
		this.container.unbind('mouseupoutjsmol');
		this.container.unbind('touchendoutjsmol');
		if (isBind) {
			var me = this;
			this.container.bind('mousemoveoutjsmol touchmoveoutjsmol', function(evspecial, target, ev) {
			  me.mouseMove(ev);
			});
			this.container.bind('mouseupoutjsmol touchendoutjsmol', function(evspecial, target, ev) {
				me.mouseUp(ev);
			});
		}
	};
}

})(Jmol, jQuery);

// BH 3/5/2013 9:54:16 PM added support for a cover image: Info.coverImage, coverScript, coverTitle, deferApplet, deferUncover
 
// BH 1/3/2013 4:54:01 AM mouse binding should return false -- see d.bind(...), and d.bind("contextmenu") is not necessary

// JSmol.js -- Jmol pure JavaScript version
// author: Bob Hanson, hansonr@stolaf.edu	4/16/2012
// author: Takanori Nakane biochem_fan 6/12/2012

// This library requires
//
//	JSmoljQuery.js
//	JSmolCore.js
//  JSmolApplet.js
//  JSmolApi.js
//  j2s/j2sjmol.js    (Clazz and associated classes)
// prior to JSmol.js

// these two:
//
//  JSmolThree.js
//  JSmolGLmol.js
//  
//  are optional 

;(function (Jmol) {

	Jmol._getCanvas = function(id, Info, checkOnly, checkWebGL, checkHTML5) {
		// overrides the function in JmolCore.js
		var canvas = null;
		if (checkWebGL && Jmol.featureDetection.supportsWebGL()) {
			Jmol._Canvas3D.prototype = Jmol._jsSetPrototype(new Jmol._Applet(id,Info, "", true));
			GLmol.setRefresh(Jmol._Canvas3D.prototype);
			canvas = new Jmol._Canvas3D(id, Info, null, checkOnly);
		}
		if (checkHTML5 && canvas == null) {
			Jmol._Canvas2D.prototype = Jmol._jsSetPrototype(new Jmol._Applet(id,Info, "", true));
			canvas = new Jmol._Canvas2D(id, Info, null, checkOnly);
		}
		return canvas;
	};

	Jmol._Canvas2D = function(id, Info, caption, checkOnly){
		this._syncId = ("" + Math.random()).substring(3);
		this._id = id;
		this._is2D = true;
    this._isJava = false;
		this._aaScale = 1; // antialias scaling
		this._jmolType = "Jmol._Canvas2D (JSmol)";
		this._platform = "J.awtjs2d.Platform";
		if (checkOnly)
			return this;
    window[id] = this;
		this._createCanvas(id, Info, caption, null);
    if (!Jmol._document || this._deferApplet)
      return this;
    this._init();
		return this;
	};


	Jmol._jsSetPrototype = function(proto) {
    proto._init = function() {
   		this._setupJS();
		  this._showInfo(true); 
		  if (this._disableInitialConsole)
			  this._showInfo(false);
    };
    
    proto._createCanvas = function(id, Info, caption, glmol) {
			Jmol._setObject(this, id, Info);
      if (glmol) {
  			this._GLmol = glmol;
	   		this._GLmol.applet = this;
        this._GLmol.id = this._id;
      }      
      var t = Jmol._getWrapper(this, true);
      if (this._deferApplet) {
      } else if (Jmol._document) {
				Jmol._documentWrite(t);
        this._getCanvas(false);				        
				t = "";
			} else {
        this._deferApplet = true;
				t += '<script type="text/javascript">' + id + '._cover(false)</script>';
			}
			t += Jmol._getWrapper(this, false);
      if (Info.addSelectionOptions)
				t += Jmol._getGrabberOptions(this, caption);
			if (Jmol._debugAlert && !Jmol._document)
				alert(t);
			this._code = Jmol._documentWrite(t);
		};
		                      
    proto._cover = function (doCover) {
      if (doCover || !this._deferApplet) {
        this._displayCoverImage(doCover);
        return;
      }
      // uncovering UNMADE applet upon clicking image
      var s = (this._coverScript ? this._coverScript : "");
      this._coverScript = "";
      if (this._deferUncover)
        s += ";refresh;javascript " + this._id + "._displayCoverImage(false)";
      this._script(s, true);
      if (this._deferUncover && this._coverTitle == "activate 3D model")
        Jmol._getElement(this, "coverimage").title = "3D model is loading...";
      this._start();
      if (!this._deferUncover)
        this._displayCoverImage(doCover);
      if (this._init)
        this._init();
    };
  
    proto._displayCoverImage = function(TF) {
      if (!this._coverImage || this._isCovered == TF) return;
      this._isCovered = TF;
      Jmol._getElement(this, "coverdiv").style.display = (TF ? "block" : "none");
    };

    proto._start = function() {
      if (this._deferApplet)
        this._getCanvas(false);      
      if (this._defaultModel)
        Jmol._search(this, this._defaultModel);
      //if (this._readyScript) {
        //this._script(this._readyScript);
      //}
      this._showInfo(false);
    };                      

    proto._getCanvas = function(doReplace) {
      if (this._is2D)
        this._createCanvas2d(doReplace);
      else
        this._GLmol.create();
    };
                      
		proto._createCanvas2d = function(doReplace) {
		  var canvas = document.createElement( 'canvas' );
      var container = Jmol.$(this, "appletdiv");
      if (doReplace) {
        container[0].removeChild(this._canvas);
        Jmol._jsUnsetMouse(this._canvas);
      }
      var w = Math.round(container.width());
	  	var h = Math.round(container.height());
  		canvas.applet = this;
      this._canvas = canvas;
  		canvas.style.width = "100%";
  		canvas.style.height = "100%";
  		canvas.width = w;
  		canvas.height = h; // w and h used in setScreenDimension
  		canvas.id = this._id + "_canvas2d";
  		container.append(canvas);
      Jmol._jsSetMouse(canvas);
		}
		
		proto._setupJS = function() {
			window["j2s.lib"] = {
				base : this._j2sPath + "/",
				alias : ".",
				console : this._console
			};
			
			var es = Jmol._execStack;
			var doStart = (es.length == 0);
			es.push([this, Jmol.__loadClazz, null, "loadClazz"])
			if (!this._is2D) {
	   		es.push([this, Jmol.__loadClass, "J.exportjs.JSExporter","load JSExporter"])
				es.push([this, this.__addExportHook, null, "addExportHook"])
			}			 			
			if (Jmol.debugCode) {
        //es.push([this.__checkLoadStatus, null,"checkLoadStatus"])
        es.push([this, Jmol.__loadClass, "J.appletjs.Jmol", "load Jmol"])
      }
			es.push([this, this.__createApplet, null,"createApplet"])

			this._isSigned = true; // access all files via URL hook
			this._ready = false; 
			this._applet = null;
			this._canScript = function(script) {return true;};
			this._savedOrientations = [];
			this._syncKeyword = "Select:";
			Jmol._execLog += ("execStack loaded by " + this._id + " len=" + Jmol._execStack.length + "\n")
			if (!doStart)return;
			Jmol.__nextExecution();
		};

//	  proto.__checkLoadStatus = function(applet) {
//	  return;
//		  if (J.appletjs && J.appletjs.Jmol) {
//		    Jmol.__nextExecution();
//		  	return;
//		}
//			// spin wheels until core.z.js is processed
//			setTimeout(applet._id + ".__checkLoadStatus(" + applet._id + ")",100);
//		}


		proto.__addExportHook = function(applet) {
		  GLmol.addExportHook(applet);
			Jmol.__nextExecution();
		};

		proto.__createApplet = function(applet) {
			var viewerOptions =  new java.util.Hashtable ();
      Jmol._setJmolParams(viewerOptions, applet.__Info, true);
			viewerOptions.put("appletReadyCallback","Jmol._readyCallback");
			viewerOptions.put("applet", true);
			viewerOptions.put("name", applet._id + "_object");
			viewerOptions.put("syncId", applet._syncId);      
      if (applet._color) 
        viewerOptions.put("bgcolor", applet._color);
      if (!applet._is2D)  
			  viewerOptions.put("script", "set multipleBondSpacing 0.35;");
			
			viewerOptions.put("signedApplet", "true");
			viewerOptions.put("platform", applet._platform);
			if (applet._is2D)
				viewerOptions.put("display",applet._id + "_canvas2d");
  			
			//viewerOptions.put("repaintManager", "J.render");
			viewerOptions.put("documentBase", document.location.href);
			var base = document.location.href.split("?")[0].split("#")[0].split("/")
			base[base.length - 1] = window["j2s.lib"].base
			viewerOptions.put ("codeBase", base.join("/"));
      
			Jmol._registerApplet(applet._id, applet);
      applet._applet = new J.appletjs.Jmol(viewerOptions);
      
      if (!applet._is2D)
				applet._GLmol.applet = applet;
			applet._jsSetScreenDimensions();
      
      
			if(applet.aaScale && applet.aaScale != 1)
				applet._applet.viewer.actionManager.setMouseDragFactor(applet.aaScale)
			Jmol.__nextExecution();
		};
		
		proto._jsSetScreenDimensions = function() {
				if (!this._applet)return
				// strangely, if CTRL+/CTRL- are used repeatedly, then the
        // applet div can be not the same size as the canvas if there
        // is a border in place.
				var d = Jmol._getElement(this, (this._is2D ? "canvas2d" : "canvas"));
				this._applet.viewer.setScreenDimension(
				d.width, d.height);
//				Math.floor(Jmol.$(this, "appletdiv").height()));

		};

		proto._loadModel = function(mol, params) {
			var script = 'load DATA "model"\n' + mol + '\nEND "model" ' + params;
			this._script(script);
			this._showInfo(false);
		};
	
/*		
		proto._showInfo = function(tf) {
				Jmol._getElement(this, "infoheaderspan").innerHTML = this._infoHeader;
			if (this._info)
				Jmol._getElement(this, "infodiv").innerHTML = this._info;
			if ((!this._isInfoVisible) == (!tf))
				return;
			this._isInfoVisible = tf;
			if (this._infoObject) {
				this._infoObject._showInfo(tf);
			} else {
				Jmol._getElement(this, "infotablediv").style.display = (tf ? "block" : "none");
  			Jmol._getElement(this, "infoheaderdiv").style.display = (tf ? "block" : "none");
			}
  		this._show(!tf);
		}
*/		
		proto._show = function(tf) {
			Jmol._getElement(this,"appletdiv").style.display = (tf ? "block" : "none");
			if (tf)
				Jmol._repaint(this, true);
		};
		
		proto._canScript = function(script) {return true};
		
		proto._delay = function(eval, sc, millis) {
		// does not take into account that scripts may be added after this and need to be cached.
			this._delayID = setTimeout(function(){eval.resumeEval(sc,false)}, millis);		
		}
		
		proto._loadFile = function(fileName, params){
			this._showInfo(false);
			params || (params = "");
			this._thisJmolModel = "" + Math.random();
			this._fileName = fileName;
      Jmol._scriptLoad(this, fileName, params, true);
		};
		
		proto._createDomNode = function(id, data) {
			id = this._id + "_" + id;
			var d = document.getElementById(id);
			if (d)
				document.body.removeChild(d);
			if (!data)
				return;
			if (data.indexOf("<?") == 0)
				data = data.substring(data.indexOf("<", 1));
			if (data.indexOf("/>") >= 0) {
				// no doubt there is a more efficient way to do this.
				// Firefox, at least, does not recognize "/>" in HTML blocks
				// that are added this way.
				var D = data.split("/>");
				for (var i = D.length - 1; --i >= 0;) {
					var s = D[i];
					var pt = s.lastIndexOf("<") + 1;
					var pt2 = pt;
					var len = s.length;
					var name = "";
					while (++pt2 < len) {
					  if (" \t\n\r".indexOf(s.charAt(pt2))>= 0) {
							var name = s.substring(pt, pt2);
							D[i] = s + "></"+name+">";
							break;
						}	  	
					}
				}
				data = D.join('');
			}
			d = document.createElement("_xml")
			d.id = id;
			d.innerHTML = data;
			d.style.display = "none";
			document.body.appendChild(d);
			return d;
		}		
	
		proto.equals = function(a) { return this == a };
		proto.clone = function() { return this };
		proto.hashCode = function() { return parseInt(this._syncId) };  
	
  
    proto._processGesture = function(touches) {
      return this._applet.viewer.mouse.processTwoPointGesture(touches);
    }
    
    proto._processEvent = function(type, xym) {
			this._applet.viewer.handleOldJvm10Event(type,xym[0],xym[1],xym[2],System.currentTimeMillis());
    }
    
    proto._resize = function() {
      var s = "__resizeTimeout_" + this._id;
      // only at end
      if (Jmol[s])
        clearTimeout(Jmol[s]);
      var self = this;
      Jmol[s] = setTimeout(function() {Jmol._repaint(self, true);Jmol[s]=null}, 100);
    }
    
    
    return proto;
	};
	
	
	
  Jmol._repaint = function(applet, asNewThread) {
    // asNewThread: true is from RepaintManager.repaintNow()
    //              false is from Repaintmanager.requestRepaintAndWait()
    //
		
    //alert("_repaint " + arguments.callee.caller.caller.exName)
		if (!applet._applet)return;

		//asNewThread = false;
		var container = Jmol.$(applet, "appletdiv");
		var w = Math.round(container.width());
		var h = Math.round(container.height());
    if (applet._is2D && (applet._canvas.width != w || applet._canvas.height != h)) {
      applet._getCanvas(true);
      applet._applet.viewer.setDisplay(applet._canvas);
    }
		applet._applet.viewer.setScreenDimension(w, h);

		if (asNewThread) {
      setTimeout(function(){ applet._applet.viewer.updateJS(0,0)});
  	} else {
  		applet._applet.viewer.updateJS(0,0);
  	}
  	//System.out.println(applet._applet.fullName)
	}

	Jmol._getHiddenCanvas = function(applet, id, width, height, forceNew) {
  	id = applet._id + "_" + id;
		var d = document.getElementById(id);
    if (d && forceNew) {
      //$("body").removeChild(d);
      d = null;
    }
		if (!d)
	    d = document.createElement( 'canvas' );
	    // for some reason both these need to be set, or maybe just d.width?
		d.width = d.style.width = width;
		d.height = d.style.height = height;
		//d.style.display = "none";
		if (d.id != id) {
			d.id = id;
	  	//$("body").append(d);
	  }
	  return d;
	}

	Jmol.__loadClass = function(applet, javaClass) {
	  ClazzLoader.loadClass(javaClass, function() {Jmol.__nextExecution()});
	};

	Jmol.__nextExecution = function(trigger) {
		var es = Jmol._execStack;
	  if (es.length == 0)
	  	return;
	  if (!trigger) {
			Jmol._execLog += ("settimeout for " + es[0][0]._id + " " + es[0][3] + " len=" + es.length + "\n")
		  setTimeout("Jmol.__nextExecution(true)",10)
	  	return;
	  }
	  var e = es.shift();
	  Jmol._execLog += "executing " + e[0]._id + " " + e[3] + "\n"
		e[1](e[0],e[2]);	
	};

	Jmol.__loadClazz = function(applet) {
		// problems with multiple applets?
	  if (!Jmol.__clazzLoaded) {
  		Jmol.__clazzLoaded = true;
			LoadClazz();
			if (applet._noMonitor)
				ClassLoaderProgressMonitor.showStatus = function() {}
			LoadClazz = null;

			ClazzLoader.globalLoaded = function (file) {
       // not really.... just nothing more yet to do yet
      	ClassLoaderProgressMonitor.showStatus ("Application loaded.", true);
  			if (!Jmol.debugCode || !Jmol.haveCore) {
  				Jmol.haveCore = true;
    			Jmol.__nextExecution();
    		}
      };
			ClazzLoader.packageClasspath ("java", null, true);
			ClazzLoader.setPrimaryFolder (applet._j2sPath); // where org.jsmol.test.Test is to be found
			ClazzLoader.packageClasspath (applet._j2sPath); // where the other files are to be found
  		//if (!Jmol.debugCode)
			  return;
		}
		Jmol.__nextExecution();
	};

	Jmol._doAjax = function(url, postOut, bytesOut) {
    url = url.toString();
    
    if (bytesOut != null) {
    	bytesOut = J.io.Base64.getBase64(bytesOut).toString();
    	var filename = url.substring(url.lastIndexOf("/") + 1);
    	var mimetype = (filename.indexOf(".png") >= 0 ? "image/png" : filename.indexOf(".jpg") >= 0 ? "image/jpg" : "");
    	Jmol._saveFile(filename, mimetype, bytesOut, "base64");
    	return "OK";
    }
    if (postOut)
      url += "?POST?" + postOut;
    var data = Jmol._getFileData(url)
    return Jmol._processData(data, Jmol._isBinaryUrl(url));
	}

  Jmol._processData = function(data, isBinary) {
    if (typeof data == "undefined") {
      data = "";
      isBinary = false;
    }
    isBinary &= Jmol._canSyncBinary();
    if (!isBinary)
  		return J.util.SB.newS(data);
  	var b;
		if (Clazz.instanceOf(data, self.ArrayBuffer)) {
			data = new Uint8Array(data);
    	b = Clazz.newByteArray(data.length, 0);
	    for (var i = data.length; --i >= 0;)
  	    b[i] = data[i];
    //alert("Jmol._processData len=" + b.length + " b[0-5]=" + b[0] + " " + b[1]+ " " + b[2] + " " + b[3]+ " " + b[4] + " " + b[5])
    	return b;
						
		}
    b = Clazz.newByteArray(data.length, 0);
    for (var i = data.length; --i >= 0;)
      b[i] = data.charCodeAt(i) & 0xFF;
    //alert("Jmol._processData len=" + b.length + " b[0-5]=" + b[0] + " " + b[1]+ " " + b[2] + " " + b[3]+ " " + b[4] + " " + b[5])
    return b;
  };


  Jmol._loadImage = function(platform, echoNameAndPath, bytes, fOnload, image) {
  // bytes would be from a ZIP file -- will have to reflect those back from server as an image after conversion to base64
  // ah, but that's a problem, because that image would be needed to be posted, but you can't post an image call. 
  // so we would have to go with <image data:> which does not work in all browsers. Hmm. 
  
    var path = echoNameAndPath[1];
    
    if (image == null) {
      var image = new Image();
      image.onload = function() {Jmol._loadImage(platform, echoNameAndPath, null, fOnload, image)};

      if (bytes != null) {      
        bytes = J.io.Base64.getBase64(bytes).toString();      
        var filename = path.substring(url.lastIndexOf("/") + 1);                                                           
        var mimetype = (filename.indexOf(".png") >= 0 ? "image/png" : filename.indexOf(".jpg") >= 0 ? "image/jpg" : "");
    	   // now what?
      }
      image.src = path;
      return true; // as far as we can tell!
    }
    var width = image.width;
    var height = image.height; 
    var id = "echo_" + echoNameAndPath[0];
    var canvas = Jmol._getHiddenCanvas(platform.viewer.applet, id, width, height, true);
    canvas.imageWidth = width;
    canvas.imageHeight = height;
    canvas.id = id;
    canvas.image=image;
    Jmol._setCanvasImage(canvas, width, height);
    // return a null canvas and the error in path if there is a problem
    fOnload(canvas,path);
  };

  Jmol._setCanvasImage = function(canvas, width, height) {
    canvas.buf32 = null;
    canvas.width = width;
    canvas.height = height;
    canvas.getContext("2d").drawImage(canvas.image, 0, 0, width, height);
  };

		
})(Jmol);
// JmolApplet.js -- Jmol._Applet and Jmol._Image

// BH 7/16/2012 1:50:03 PM adds server-side scripting for image
// BH 8/11/2012 11:00:01 AM adds Jmol._readyCallback for MSIE not in Quirks mode
// BH 8/12/2012 3:56:40 AM allows .min.png to be replaced by .all.png in Image file name
// BH 8/13/2012 6:16:55 PM fix for no-java message not displaying
// BH 11/18/2012 1:06:39 PM adds option ">" in database query box for quick command execution
// BH 12/17/2012 6:25:00 AM change ">" option to "!"

(function (Jmol, document) {


	// _Applet -- the main, full-featured, object
	
	Jmol._Applet = function(id, Info, caption, checkOnly){
    window[id] = this;
		this._jmolType = "Jmol._Applet" + (Info.isSigned ? " (signed)" : "");
    this._isJava = true;
		if (checkOnly)
			return this;
		this._isSigned = Info.isSigned;
		this._readyFunction = Info.readyFunction;
		this._ready = false;
    this._isJava = true; 
		this._isInfoVisible = false;
		this._applet = null;
		this._memoryLimit = Info.memoryLimit || 512;
		this._canScript = function(script) {return true;};
		this._savedOrientations = [];
		this._syncKeyword = "Select:";
		
		/*
		 * privileged methods
		 */
		this._initialize = function(jarPath, jarFile) {
			var doReport = false;
			if(this._jarFile) {
				var f = this._jarFile;
				if(f.indexOf("/") >= 0) {
					alert("This web page URL is requesting that the applet used be " + f + ". This is a possible security risk, particularly if the applet is signed, because signed applets can read and write files on your local machine or network.");
					var ok = prompt("Do you want to use applet " + f + "? ", "yes or no")
					if(ok == "yes") {
						jarPath = f.substring(0, f.lastIndexOf("/"));
						jarFile = f.substring(f.lastIndexOf("/") + 1);
					} else {
						doReport = true;
					}
				} else {
					jarFile = f;
				}
			}
 			this._jarPath = jarPath || ".";
			this._jarFile = (typeof(jarFile) == "string" ? jarFile : (jarFile ?  "JmolAppletSigned" : "JmolApplet") + "0.jar");
	    if (doReport)
				alert("The web page URL was ignored. Continuing using " + this._jarFile + ' in directory "' + this._jarPath + '"');
			Jmol.controls == undefined || Jmol.controls._onloadResetForms();		
		}		
		this._create(id, Info, caption);
		return this;
	}

  Jmol._Applet._getApplet = function(id, Info, checkOnly) {
	
	// note that the variable name the return is assigned to MUST match the first parameter in quotes
	// applet = Jmol.getApplet("applet", Info)

		checkOnly || (checkOnly = false);
		Info || (Info = {});
		var DefaultInfo = {
			color: "#FFFFFF", // applet object background color, as for older jmolSetBackgroundColor(s)
			width: 300,
			height: 300,
			addSelectionOptions: false,
			serverURL: "http://chemapps.stolaf.edu/jmol/jsmol/jsmol.php",
			defaultModel: "",
			script: null,
			src: null,
			readyFunction: null,
			use: "HTML5",//other options include JAVA, WEBGL, and IMAGE
			jarPath: "java",
			jarFile: "JmolApplet0.jar",
			isSigned: false,
			j2sPath: "j2s",
      coverImage: null,     // URL for image to display
      coverTitle: "",       // tip that is displayed before model starts to load
      coverCommand: "",     // Jmol command executed upon clicking image
      deferApplet: false,   // true == the model should not be loaded until the image is clicked
      deferUncover: false,  // true == the image should remain until command execution is complete 
			disableJ2SLoadMonitor: false,
			disableInitialConsole: false,
			debug: false
		};	 
		Jmol._addDefaultInfo(Info, DefaultInfo);

  	Jmol._debugAlert = Info.debug;
      if (!Jmol.featureDetection.allowHTML5)Info.use = "JAVA";
      
      //alert(Info.use)
		  var javaAllowed = false;
    
	
		Info.serverURL && (Jmol._serverUrl = Info.serverURL);
		var applet = null;
		
		if (Info.use) {
		
		// better idea....
		// order matters; must have JAVA, WEBGL, HTML5, or IMAGE in some order, separated by a single space

    	var List = Info.use.toUpperCase().split("#")[0].split(" ");
		
		  for (var i = 0; i < List.length; i++) {
		    switch (List[i]) {
		    case "JAVA":
		    	javaAllowed = true;
		    	if (Jmol.featureDetection.supportsJava())
						applet = new Jmol._Applet(id, Info, null, checkOnly);
					break;
		    case "WEBGL":
					applet = Jmol._getCanvas(id, Info, checkOnly, true, false);
		      break;
		    case "HTML5":
					applet = Jmol._getCanvas(id, Info, checkOnly, false, true);
		      break;
		    case "IMAGE":
					applet = new Jmol._Image(id, Info, null, checkOnly);
					break;
		    }
		    if (applet != null)
		    	break;		  
		  }
		  if (applet == null) {
		  	if (checkOnly || !javaAllowed)
		  		applet = {_jmolType : "none" };
		  	else if (javaAllowed)
	 		  	applet = new Jmol._Applet(id, Info, null, false);
			}
			
		} else {

			// early idea -- deprecated
			
			if (!Info.useNoApplet && !Info.useImageOnly 
				&& (navigator.javaEnabled() || Info.useJmolOnly)) {
			
			// Jmol applet, signed or unsigned
			
				applet = new Jmol._Applet(id, Info, null, checkOnly);
			} 
			if (applet == null) {
				if (!Info.useJmolOnly && !Info.useImageOnly) {
					applet = Jmol._getCanvas(id, Info, checkOnly, Info.useWebGlIfAvailable, !Info.useWebGlIfAvailable);
				} 
				if (applet == null)
					applet = new Jmol._Image(id, Info, null, checkOnly);
			}
		}

		// keyed to both its string id and itself
		return (checkOnly ? applet : Jmol._registerApplet(id, applet));  
  }
  /*  AngelH, mar2007:
    By (re)setting these variables in the webpage before calling Jmol.getApplet(),
    a custom message can be provided (e.g. localized for user's language) when no Java is installed.
  */
	Jmol._Applet._noJavaMsg =
      "You do not have Java applets enabled in your web browser, or your browser is blocking this applet.<br />\
      Check the warning message from your browser and/or enable Java applets in<br />\
      your web browser preferences, or install the Java Runtime Environment from <a href='http://www.java.com'>www.java.com</a>";
	Jmol._Applet._noJavaMsg2 =
      "You do not have the<br />\
      Java Runtime Environment<br />\
      installed for applet support.<br />\
      Visit <a href='http://www.java.com'>www.java.com</a>";

	Jmol._Applet._setCommonMethods = function(proto) {
		proto._showInfo = Jmol._Applet.prototype._showInfo;	
		proto._search = Jmol._Applet.prototype._search;
		proto._readyCallback = Jmol._Applet.prototype._readyCallback;
	}

	Jmol._Applet._createApplet = function(applet, Info, params, myClass, script, caption) {

		if (Jmol._syncedApplets.length)
		  params.synccallback = "Jmol._mySyncCallback";
		params.java_arguments = "-Xmx" + Math.round(Info.memoryLimit || applet._memoryLimit) + "m";

		applet._initialize(Info.jarPath, Info.jarFile);

		// size is set to 100% of containers' size, but only if resizable. 
		// Note that resizability in MSIE requires: 
		// <!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
		var w = (applet._containerWidth.indexOf("px") >= 0 ? applet._containerWidth : "100%");
		var h = (applet._containerHeight.indexOf("px") >= 0 ? applet._containerHeight : "100%");
		var widthAndHeight = " style=\"width:" + w + ";height:" + h + "\" ";
		var tHeader, tFooter;
		if (Jmol.featureDetection.useIEObject || Jmol.featureDetection.useHtml4Object) {
			params.archive = applet._jarFile;
			if (script)
  			params.script = script;
			params.mayscript = 'true';
			params.codebase = applet._jarPath;
			params.code = myClass + ".class";
			tHeader =
				"<object name='" + applet._id +
				"_object' id='" + applet._id + "_object' " + "\n" +
				widthAndHeight + "\n";
			tFooter = "</object>";
		}
		if (Jmol.featureDetection.useIEObject) { // use MSFT IE6 object tag with .cab file reference
			var _windowsClassId = "clsid:8AD9C840-044E-11D1-B3E9-00805F499D93";
			var _windowsCabUrl = "http://java.sun.com/update/1.6.0/jinstall-6u22-windows-i586.cab";
			tHeader += " classid='" + _windowsClassId + "'\n" + " codebase='" + _windowsCabUrl + "'\n>\n";
		} else if (Jmol.featureDetection.useHtml4Object) { // use HTML4 object tag
			tHeader += " type='application/x-java-applet'\n>\n";
		} else { // use applet tag
			if (script)
  			params.script = script;
			tHeader =
				"<applet name='" + applet._id +
				"_object' id='" + applet._id + "_object' \n" +
				widthAndHeight + "\n" +
				" code='" + myClass + "'" +
				" archive='" + applet._jarFile + "' codebase='" + applet._jarPath + "'\n" +
				" mayscript='true'>\n";
			tFooter = "</applet>";
		}
		var visitJava;
		if (Jmol.featureDetection.useIEObject || Jmol.featureDetection.useHtml4Object) {
			var szX = "width:" + applet._width;
			if ( szX.indexOf("%")==-1 ) 
				szX+="px";
			var szY = "height:" + applet._height;
			if ( szY.indexOf("%")==-1 )
				szY+="px";
			visitJava = "<p style='background-color:yellow; color:black; " + szX + ";" + szY + ";" +
					// why doesn't this vertical-align work?
				"text-align:center;vertical-align:middle;'>\n" +
				Jmol._Applet._noJavaMsg + "</p>";
		} else {
			visitJava = "<table bgcolor='yellow'><tr>" +
				"<td align='center' valign='middle' " + widthAndHeight + "><font color='black'>\n" +
				Jmol._Applet._noJavaMsg2 + "</font></td></tr></table>";
		}
	
		var t = Jmol._getWrapper(applet, true) + tHeader;
 		for (var i in params)
			if(params[i])
		 		t+="  <param name='"+i+"' value='"+params[i]+"' />\n";
		t += visitJava + tFooter 
			+ Jmol._getWrapper(applet, false) 
			+ (Info.addSelectionOptions ? Jmol._getGrabberOptions(applet, caption) : "");
		if (Jmol._debugAlert)
			alert(t);
		applet._code = Jmol._documentWrite(t);
	}

  var japroto = Jmol._Applet.prototype;
  
	japroto._create = function(id, Info, caption){
		Jmol._setObject(this, id, Info);
		var params = {
			syncId: ("" + Math.random()).substring(3),
			progressbar: "true",                      
			progresscolor: "blue",
			boxbgcolor: this._color || "black",
			boxfgcolor: "white",
			boxmessage: "Downloading JmolApplet ...",
			script: (!Info.color ? "" : "background " + (Info.color.indexOf("#") == 0 ? "[0x" + Info.color.substring(1) + "]" : Info.color))
		};
    Jmol._setJmolParams(params, Info, false);
		function sterilizeInline(model) {
			model = model.replace(/\r|\n|\r\n/g, (model.indexOf("|") >= 0 ? "\\/n" : "|")).replace(/'/g, "&#39;");
			if(Jmol._debugAlert)
				alert("inline model:\n" + model);
			return model;
		}

		params.loadInline = (Info.inlineModel ? sterilizeInline(Info.inlineModel) : "");
		params.appletReadyCallback = "Jmol._readyCallback";
		if (Jmol._syncedApplets.length)
		  params.synccallback = "Jmol._mySyncCallback";
		params.java_arguments = "-Xmx" + Math.round(Info.memoryLimit || this._memoryLimit) + "m";

		this._initialize(Info.jarPath, Info.jarFile);
		Jmol._Applet._createApplet(this, Info, params, "JmolApplet", null);
	}

	japroto._readyCallback = function(id, fullid, isReady, applet) {
		if (!isReady)
			return; // ignore -- page is closing
    var me = this;
    Jmol._setDestroy(me);
		me._ready = true;
		var script = me._readyScript;
		me._applet = applet;
		if (me._defaultModel)
			Jmol._search(me, me._defaultModel, (script ? ";" + script : ""));
		else if (script)
			me._script(script);
		else if (me._src)
			me._script('load "' + me._src + '"');
		me._showInfo(true);
		me._showInfo(false);
		me._readyFunction && me._readyFunction(me);
		Jmol._setReady(this);
	}
	
	japroto._showInfo = function(tf) {
		Jmol._getElement(this, "infoheaderspan").innerHTML = this._infoHeader;
		if (this._info)
			Jmol._getElement(this, "infodiv").innerHTML = this._info;
		if ((!this._isInfoVisible) == (!tf))
			return;
		this._isInfoVisible = tf;
		// 1px does not work for MSIE
	  if (this._isJava) {
  		var w = (tf ? "2px" : "100%");
  		var h = (tf ? "2px" : "100%");
  //		var w = (tf ? "2px" : this._containerWidth.indexOf("px") >= 0 ? this._containerWidth : "100%");
  //		var h = (tf ? "2px" : this._containerHeight.indexOf("px") >= 0 ? this._containerHeight : "100%");
  		Jmol._getElement(this, "appletdiv").style.width = w;
  		Jmol._getElement(this, "appletdiv").style.height = h;
    }
		if (this._infoObject) {
			this._infoObject._showInfo(tf);
		} else {
			Jmol._getElement(this, "infotablediv").style.display = (tf ? "block" : "none");
  		Jmol._getElement(this, "infoheaderdiv").style.display = (tf ? "block" : "none");
		}
		this._show(!tf);
	}

	japroto._show = function(tf) {
		var w = (!tf ? "2px" : "100%");
		var h = (!tf ? "2px" : "100%");
		//var w = (!tf ? "2px" : this._containerWidth.indexOf("px") >= 0 ? this._containerWidth : "100%");
		//var h = (!tf ? "2px" : this._containerHeight.indexOf("px") >= 0 ? this._containerHeight : "100%");
		var o = Jmol._getElement(this, "object");
		if (o && o.style) {
			o.style.width = w; 
			o.style.height = h;
		} 
	}

	japroto._search = function(query, script){
		Jmol._search(this, query, script);
	}
	
	japroto._loadModel = function(mol, params) {
    if (!params && this._noscript) {
      this._applet.viewer.loadInline(mol, '\0');
      return;
    }
		var script = 'load DATA "model"\n' + mol + '\nEND "model" ' + params;
		this._applet.script(script);
	}
	
	japroto._clearConsole = function () {
			if (this._console == this._id + "_infodiv")
				this.info = "";
			if (!self.Clazz)return;
			Jmol._setConsoleDiv(this._console);
			Clazz.Console.clear();
		}

	
  japroto._addScript = function(script) {      
		this._readyScript || (this._readyScript = ";");
		this._readyScript += ";" + script;
    return true;
  }

	japroto._script = function(script) {
		if (!this._ready)
        return this._addScript(script);
		Jmol._setConsoleDiv(this._console);
		this._applet.script(script);
	}
	
	japroto._syncScript = function(script) {
		this._applet.syncScript(script);
	}
	
	japroto._scriptWait = function(script) {
		var Ret = this._scriptWaitAsArray(script);
		var s = "";
		for(var i = Ret.length; --i >= 0; )
			for(var j = 0, jj = Ret[i].length; j < jj; j++)
				s += Ret[i][j] + "\n";
		return s;
	}
	
	japroto._scriptEcho = function(script) {
		// returns a newline-separated list of all echos from a script
		var Ret = this._scriptWaitAsArray(script);
		var s = "";
		for(var i = Ret.length; --i >= 0; )
			for(var j = Ret[i].length; --j >= 0; )
				if(Ret[i][j][1] == "scriptEcho")
					s += Ret[i][j][3] + "\n";
		return s.replace(/ \| /g, "\n");
	}
	
	japroto._scriptMessage = function(script) {
		// returns a newline-separated list of all messages from a script, ending with "script completed\n"
		var Ret = this._scriptWaitAsArray(script);
		var s = "";
		for(var i = Ret.length; --i >= 0; )
			for(var j = Ret[i].length; --j >= 0; )
				if(Ret[i][j][1] == "scriptStatus")
					s += Ret[i][j][3] + "\n";
		return s.replace(/ \| /g, "\n");
	}
	
	japroto._scriptWaitOutput = function(script) {
		var ret = "";
		try {
			if(script) {
				ret += this._applet.scriptWaitOutput(script);
			}
		} catch(e) {
		}
		return ret;
	}

	japroto._scriptWaitAsArray = function(script) {
		var ret = "";
		try {
			this._getStatus("scriptEcho,scriptMessage,scriptStatus,scriptError");
			if(script) {
				ret += this._applet.scriptWait(script);
				ret = Jmol._evalJSON(ret, "jmolStatus");
				if( typeof ret == "object")
					return ret;
			}
		} catch(e) {
		}
		return [[ret]];
	}
	
	japroto._getStatus = function(strStatus) {
		return Jmol._sortMessages(this._getPropertyAsArray("jmolStatus",strStatus));
	}
	
	japroto._getPropertyAsArray = function(sKey,sValue) {
		return Jmol._evalJSON(this._getPropertyAsJSON(sKey,sValue),sKey);
	}

	japroto._getPropertyAsString = function(sKey,sValue) {
		sValue == undefined && ( sValue = "");
		return this._applet.getPropertyAsString(sKey, sValue) + "";
	}

	japroto._getPropertyAsJSON = function(sKey,sValue) {
		sValue == undefined && ( sValue = "");
		try {
			return (this._applet.getPropertyAsJSON(sKey, sValue) + "");
		} catch(e) {
			return "";
		}
	}

	japroto._getPropertyAsJavaObject = function(sKey,sValue) {		
		sValue == undefined && ( sValue = "");
		return this._applet.getProperty(sKey,sValue);
	}

	
	japroto._evaluate = function(molecularMath) {
		//carries out molecular math on a model
	
		var result = "" + this._getPropertyAsJavaObject("evaluate", molecularMath);
		var s = result.replace(/\-*\d+/, "");
		if(s == "" && !isNaN(parseInt(result)))
			return parseInt(result);
		var s = result.replace(/\-*\d*\.\d*/, "")
		if(s == "" && !isNaN(parseFloat(result)))
			return parseFloat(result);
		return result;
	}

	
	japroto._saveOrientation = function(id) {	
		return this._savedOrientations[id] = this._getPropertyAsArray("orientationInfo","info").moveTo;
	}

	
	japroto._restoreOrientation = function(id) {
		var s = this._savedOrientations[id];
		if(!s || s == "")
			return s = s.replace(/1\.0/, "0");
		return this._scriptWait(s);
	}

	
	japroto._restoreOrientationDelayed = function(id,delay) {
		arguments.length < 1 && ( delay = 1);
		var s = this._savedOrientations[id];
		if(!s || s == "")
			return s = s.replace(/1\.0/, delay);
		return this._scriptWait(s);
	}

	japroto._resizeApplet = function(size) {
		// See _jmolGetAppletSize() for the formats accepted as size [same used by jmolApplet()]
		//  Special case: an empty value for width or height is accepted, meaning no change in that dimension.
		
		/*
		 * private functions
		 */
		function _getAppletSize(size, units) {
			/* Accepts single number, 2-value array, or object with width and height as mroperties, each one can be one of:
			 percent (text string ending %), decimal 0 to 1 (percent/100), number, or text string (interpreted as nr.)
			 [width, height] array of strings is returned, with units added if specified.
			 Percent is relative to container div or element (which should have explicitly set size).
			 */
			var width, height;
			if(( typeof size) == "object" && size != null) {
				width = size[0]||size.width;
				height = size[1]||size.height;
			} else {
				width = height = size;
			}
			return [_fixDim(width, units), _fixDim(height, units)];
		}

		function _fixDim(x, units) {
			var sx = "" + x;
			return (sx.length == 0 ? (units ? "" : Jmol._allowedJmolSize[2]) 
				: sx.indexOf("%") == sx.length - 1 ? sx 
				: (x = parseFloat(x)) <= 1 && x > 0 ? x * 100 + "%" 
				: (isNaN(x = Math.floor(x)) ? Jmol._allowedJmolSize[2] 
				: x < Jmol._allowedJmolSize[0] ? Jmol._allowedJmolSize[0] 
				: x > Jmol._allowedJmolSize[1] ? Jmol._allowedJmolSize[1] 
				: x)
				+ (units ? units : "")
			);
		}
		
		var sz = _getAppletSize(size, "px");
		var d = Jmol._getElement(this, "appletinfotablediv");
		d.style.width = sz[0];
		d.style.height = sz[1];
		this._containerWidth = sz[0];
		this._containerHeight = sz[1];
    if (this._is2D)
      Jmol._repaint(this, true);
	}
	      
	japroto._loadFile = function(fileName, params){
		this._showInfo(false);
		params || (params = "");
		this._thisJmolModel = "" + Math.random();
		this._fileName = fileName;
    if (!Jmol._scriptLoad(this, fileName, params, this._isSigned)) {
		  var self = this;
		  Jmol._loadFileData(this, fileName, function(data){self._loadModel(data, params)});
    }
	}
	         
	japroto._searchDatabase = function(query, database, script){
		this._showInfo(false);
		if (query.indexOf("?") >= 0) {
			Jmol._getInfoFromDatabase(this, database, query.split("?")[0]);
			return;
		}
		script || (script = Jmol._getScriptForDatabase(database));
		var dm = database + query;
    
 		if (Jmol.db._DirectDatabaseCalls[database]) {
 			this._loadFile(dm, script);
			return;
		}
    script = ";" + script;
    if (!Jmol._scriptLoad(this, dm, script, this._isSigned)) {
			// need to do the postLoad here as well
			var self = this;
			Jmol._getRawDataFromServer(
				database,
				query,
				function(data){self._loadModel(data, script)}
			);
		}
	}

  japroto._isDeferred = function () {
    return this._cover && this._isCovered && this._deferApplet
  }
  
  japroto._checkDeferred = function(script) {
    if (this._isDeferred()) {
      this._coverScript = script;
      this._cover(false);
      return true;
    }
    return false;
  }	

	// _Image -- an alternative to _Applet
	
	Jmol._Image = function(id, Info, caption, checkOnly){
		this._jmolType = "image";
		if (checkOnly)
			return this;
		this._create(id, Info, caption);
		return this;
	}

  Jmol._Image.prototype._create = function(id, Info, caption) {
  	Jmol._setObject(this, id, Info);
  	this._src || (this._src = "");
		var t = Jmol._getWrapper(this, true) 
			+ '<img id="'+id+'_image" width="' + Info.width + '" height="' + Info.height + '" src=""/>'
		 	+	Jmol._getWrapper(this, false)
			+ (Info.addSelectionOptions ? Jmol._getGrabberOptions(this, caption) : "");
		if (Jmol._debugAlert)
			alert(t);
		this._code = Jmol._documentWrite(t);
		this._ready = false;
		if (Jmol._document)
			this._readyCallback(id, null, this._ready = true, null);
  }

	Jmol._Applet._setCommonMethods(Jmol._Image.prototype);

	Jmol._Image.prototype._canScript = function(script) {
		var slc = script.toLowerCase().replace(/[\",\']/g, '');
		var ipt = slc.length;
		return (script.indexOf("#alt:LOAD") >= 0 || slc.indexOf(";") < 0 && slc.indexOf("\n") < 0
		  && (slc.indexOf("script ") == 0 || slc.indexOf("load ") == 0)
		  && (slc.indexOf(".png") == ipt - 4 || slc.indexOf(".jpg") == ipt - 4));
	}

	Jmol._Image.prototype._script = function(script) {
		var slc = script.toLowerCase().replace(/[\",\']/g, '');
		// single command only
		// "script ..." or "load ..." only
		// PNG or PNGJ or JPG only
		// automatically switches to .all.png(j) from .min.png(j)
		var ipt = slc.length;
		if (slc.indexOf(";") < 0 && slc.indexOf("\n") < 0
		  && (slc.indexOf("script ") == 0 || slc.indexOf("load ") == 0)
		  && (slc.indexOf(".png") == ipt - 4 || slc.indexOf(".pngj") == ipt - 5 || slc.indexOf(".jpg") == ipt - 4)) {
			var imageFile = script.substring(script.indexOf(" ") + 1);
			ipt = imageFile.length;
			for (var i = 0; i < ipt; i++) {
				switch (imageFile.charAt(i)) {
				case " ":
					continue;
				case '"':
					imageFile = imageFile.substring(i + 1, imageFile.indexOf('"', i + 1))
					i = ipt;
					continue;
				case "'":
					imageFile = imageFile.substring(i + 1, imageFile.indexOf("'", i + 1))
					i = ipt;
					continue;
				default:
					imageFile = imageFile.substring(i)
					i = ipt;
					continue;
				}
			}
			imageFile = imageFile.replace(/\.min\.png/,".all.png")
			document.getElementById(this._id + "_image").src = imageFile
		} else if (script.indexOf("#alt:LOAD ") >= 0) {
		  imageFile = script.split("#alt:LOAD ")[1]
			if (imageFile.indexOf("??") >= 0) {
				var db = imageFile.split("??")[0];
				imageFile = prompt(imageFile.split("??")[1], "");
				if (!imageFile)
					return;
				if (!Jmol.db._DirectDatabaseCalls[imageFile.substring(0,1)])
					imageFile = db + imageFile;
			}
			this._loadFile(imageFile);
    }
	}
	
	Jmol._Image.prototype._show = function(tf) {
		Jmol._getElement(this, "appletdiv").style.display = (tf ? "block" : "none");
	}
		
	Jmol._Image.prototype._loadFile = function(fileName, params){
		this._showInfo(false);
		this._thisJmolModel = "" + Math.random();
		params = (params ? params : "");
		var database = "";
		if (Jmol._isDatabaseCall(fileName)) {
			database = fileName.substring(0, 1); 
			fileName = Jmol._getDirectDatabaseCall(fileName, false);
		} else if (fileName.indexOf("://") < 0) {
			var ref = document.location.href
			var pt = ref.lastIndexOf("/");
			fileName = ref.substring(0, pt + 1) + fileName;
		}
		
		var src = Jmol._serverUrl 
				+ "?call=getImageForFileLoad"
				+ "&file=" + escape(fileName)
				+ "&width=" + this._width
				+ "&height=" + this._height
				+ "&params=" + encodeURIComponent(params + ";frank off;");
		Jmol._getElement(this, "image").src = src;
	}

	Jmol._Image.prototype._searchDatabase = function(query, database, script){
		if (query.indexOf("?") == query.length - 1) {
			Jmol._getInfoFromDatabase(this, database, query.split("?")[0]);
			return;
		}
		this._showInfo(false);
		script || (script = Jmol._getScriptForDatabase(database));
		var src = Jmol._serverUrl 
			+ "?call=getImageFromDatabase"
			+ "&database=" + database
			+ "&query=" + query
			+ "&width=" + this._width
			+ "&height=" + this._height
			+ "&script=" + encodeURIComponent(script + ";frank off;");
		Jmol._getElement(this, "image").src = src;
	}

})(Jmol, document);
// BH 5/16/2013 8:14:47 AM fix for checkbox groups and default radio names
// BH 8:36 AM 7/27/2012  adds name/id for cmd button 
// BH 8/12/2012 6:51:53 AM adds function() {...} option for all controls:
//    Jmol.jmolButton(jmol, function(jmol) {...}, "xxxx")

(function(Jmol) {

	// private
	
	var c = Jmol.controls = {

		_hasResetForms: false,	
		_scripts: [""],
		_checkboxMasters: {},
		_checkboxItems: {},
		_actions: {},
	
		_buttonCount: 0,
		_checkboxCount: 0,
		_radioGroupCount: 0,
		_radioCount: 0,
		_linkCount: 0,
		_cmdCount: 0,
		_menuCount: 0,
		
		_previousOnloadHandler: null,	
		_control: null,
		_element: null,
		
		_appletCssClass: null,
		_appletCssText: "",
		_buttonCssClass: null,
		_buttonCssText: "",
		_checkboxCssClass: null,
		_checkboxCssText: "",
		_radioCssClass: null,
		_radioCssText: "",
		_linkCssClass: null,
		_linkCssText: "",
		_menuCssClass: null,
		_menuCssText: ""
	};

	c._addScript = function(appId,script) {
		var index = c._scripts.length;
		c._scripts[index] = [appId, script];
		return index;
	}
	
	c._getIdForControl = function(appletOrId, script) {
  //alert(appletOrId + " " + typeof appletOrId + " " + script + appletOrId._canScript)
		return (typeof appletOrId == "string" ? appletOrId 
		  : !script || !appletOrId._canScript || appletOrId._canScript(script) ? appletOrId._id
			: null);
	}
		
	c._radio = function(appletOrId, script, labelHtml, isChecked, separatorHtml, groupName, id, title) {
		var appId = c._getIdForControl(appletOrId, script);
		if (appId == null)
			return null;
		++c._radioCount;
		groupName != undefined && groupName != null || (groupName = "jmolRadioGroup" + (c._radioGroupCount - 1));
		if (!script)
			return "";
		id != undefined && id != null || (id = "jmolRadio" + (c._radioCount - 1));
		labelHtml != undefined && labelHtml != null || (labelHtml = script.substring(0, 32));
		separatorHtml || (separatorHtml = "");
		var eospan = "</span>";
		c._actions[id] = c._addScript(appId, script);
		var t = "<span id=\"span_"+id+"\""+(title ? " title=\"" + title + "\"":"")+"><input name='"
		  + groupName + "' id='"+id+"' type='radio'"
      + " onclick='Jmol.controls._click(this);return true;'"
      + " onmouseover='Jmol.controls._mouseOver(this);return true;'"
      + " onmouseout='Jmol.controls._mouseOut()' " +
		 (isChecked ? "checked='true' " : "") + c._radioCssText + " />";
		if (labelHtml.toLowerCase().indexOf("<td>")>=0) {
			t += eospan;
			eospan = "";
		}
		t += "<label for=\"" + id + "\">" + labelHtml + "</label>" +eospan + separatorHtml;
		return t;
	}
	
/////////// events //////////

	c._scriptExecute = function(element, scriptInfo) {
		var applet = Jmol._applets[scriptInfo[0]];
		var script = scriptInfo[1];
		if (typeof(script) == "object")
			script[0](element, script, applet);
		else if (typeof(script) == "function")
		  script(applet);
		else
			Jmol.script(applet, script);
	}
	
	c._commandKeyPress = function(e, id, appId) {
		var keycode = (e == 13 ? 13 : window.event ? window.event.keyCode : e ? e.which : 0);
		if (keycode == 13) {
			var inputBox = document.getElementById(id)
			c._scriptExecute(inputBox, [appId, inputBox.value]);
		}
	}
	
	c._click = function(obj, scriptIndex) {
		c._element = obj;
    if (arguments.length == 1)
      scriptIndex = c._actions[obj.id];
		c._scriptExecute(obj, c._scripts[scriptIndex]);
	}
	
	c._menuSelected = function(menuObject) {
		var scriptIndex = menuObject.value;
		if (scriptIndex != undefined) {
			c._scriptExecute(menuObject, c._scripts[scriptIndex]);
			return;
		}
		var len = menuObject.length;
		if (typeof len == "number")
			for (var i = 0; i < len; ++i)
				if (menuObject[i].selected) {
					c._click(menuObject[i], menuObject[i].value);
					return;
				}
		alert("?Que? menu selected bug #8734");
	}
		
	c._cbNotifyMaster = function(m){
		//called when a group item is checked
		var allOn = true;
		var allOff = true;
		for (var chkBox in m.chkGroup){
			if(m.chkGroup[chkBox].checked)
				allOff = false;
			else
				allOn = false;
		}
		if (allOn)m.chkMaster.checked = true;
		if (allOff)m.chkMaster.checked = false;
		if ((allOn || allOff) && c._checkboxItems[m.chkMaster.id])
			c._cbNotifyMaster(c._checkboxItems[m.chkMaster.id])
	}
	
	c._cbNotifyGroup = function(m, isOn){
		//called when a master item is checked
		for (var chkBox in m.chkGroup){
			var item = m.chkGroup[chkBox]
      if (item.checked != isOn) {
  			item.checked = isOn;
        c._cbClick(item);
      }
			if (c._checkboxMasters[item.id])
				c._cbNotifyGroup(c._checkboxMasters[item.id], isOn)
		}
	}
	
	c._cbSetCheckboxGroup = function(chkMaster, chkboxes, args){
		var id = chkMaster;
		if(typeof(id)=="number")id = "jmolCheckbox" + id;
		chkMaster = document.getElementById(id);
		if (!chkMaster)alert("jmolSetCheckboxGroup: master checkbox not found: " + id);
		var m = c._checkboxMasters[id] = {};
		m.chkMaster = chkMaster;
		m.chkGroup = {};
    var i0;
    if (typeof(chkboxes)=="string") {
      chkboxes = args;
      i0 = 1;
    } else {
      i0 = 0;
    }
		for (var i = i0; i < chkboxes.length; i++){
			var id = chkboxes[i];
			if(typeof(id)=="number")id = "jmolCheckbox" + id;
			checkboxItem = document.getElementById(id);
			if (!checkboxItem)alert("jmolSetCheckboxGroup: group checkbox not found: " + id);
			m.chkGroup[id] = checkboxItem;
			c._checkboxItems[id] = m;
		}
	}
	
	c._cbClick = function(ckbox) {
		c._control = ckbox;
    var whenChecked = c._actions[ckbox.id][0];
    var whenUnchecked = c._actions[ckbox.id][1];
		c._click(ckbox, ckbox.checked ? whenChecked : whenUnchecked);
		if(c._checkboxMasters[ckbox.id])
			c._cbNotifyGroup(c._checkboxMasters[ckbox.id], ckbox.checked)
		if(c._checkboxItems[ckbox.id])
			c._cbNotifyMaster(c._checkboxItems[ckbox.id])
	}
	
	c._cbOver = function(ckbox) {
    var whenChecked = c._actions[ckbox.id][0];
    var whenUnchecked = c._actions[ckbox.id][1];
		window.status = c._scripts[ckbox.checked ? whenUnchecked : whenChecked];
	}
	
	c._mouseOver = function(obj, scriptIndex) {
    if (arguments.length == 1)
      scriptIndex = c._actions[obj.id];
		window.status = c._scripts[scriptIndex];
	}
	
	c._mouseOut = function() {
		window.status = " ";
		return true;
	}

// from JmolApplet

	c._onloadResetForms = function() {
		// must be evaluated ONLY once -- is this compatible with jQuery?
		if (c._hasResetForms)
			return;
		c._hasResetForms = true;
		c._previousOnloadHandler = window.onload;
		window.onload = function() {
//			var c = Jmol.controls;
			if (c._buttonCount+c._checkboxCount+c._menuCount+c._radioCount+c._radioGroupCount > 0) {
				var forms = document.forms;
				for (var i = forms.length; --i >= 0; )
					forms[i].reset();
			}
			if (c._previousOnloadHandler)
				c._previousOnloadHandler();
		}
	}

// from JmolApi

	c._getButton = function(appletOrId, script, label, id, title) {
		var appId = c._getIdForControl(appletOrId, script);
		if (appId == null)
			return "";
		//_jmolInitCheck();
		id != undefined && id != null || (id = "jmolButton" + c._buttonCount);
		label != undefined && label != null || (label = script.substring(0, 32));
		++c._buttonCount;
    c._actions[id] = c._addScript(appId, script);
		var t = "<span id=\"span_"+id+"\""+(title ? " title=\"" + title + "\"":"")+"><input type='button' name='" + id + "' id='" + id +
						"' value='" + label +
						"' onclick='Jmol.controls._click(this)' onmouseover='Jmol.controls._mouseOver(this);return true' onmouseout='Jmol.controls._mouseOut()' " +
						c._buttonCssText + " /></span>";
		if (Jmol._debugAlert)
			alert(t);
		return Jmol._documentWrite(t);
	}

	c._getCheckbox = function(appletOrId, scriptWhenChecked, scriptWhenUnchecked,
			labelHtml, isChecked, id, title) {

		var appId = c._getIdForControl(appletOrId, scriptWhenChecked);
		if (appId != null)
			appId = c._getIdForControl(appletOrId, scriptWhenUnchecked);
		if (appId == null)
			return "";

		//_jmolInitCheck();
		id != undefined && id != null || (id = "jmolCheckbox" + c._checkboxCount);
		++c._checkboxCount;
		if (scriptWhenChecked == undefined || scriptWhenChecked == null ||
				scriptWhenUnchecked == undefined || scriptWhenUnchecked == null) {
			alert("jmolCheckbox requires two scripts");
			return;
		}
		if (labelHtml == undefined || labelHtml == null) {
			alert("jmolCheckbox requires a label");
			return;
		}
    c._actions[id] = [c._addScript(appId, scriptWhenChecked),c._addScript(appId, scriptWhenUnchecked)];
		var eospan = "</span>"
		var t = "<span id=\"span_"+id+"\""+(title ? " title=\"" + title + "\"":"")+"><input type='checkbox' name='" + id + "' id='" + id +
						"' onclick='Jmol.controls._cbClick(this)" +
						"' onmouseover='Jmol.controls._cbOver(this)" +
						";return true' onmouseout='Jmol.controls._mouseOut()' " +
			(isChecked ? "checked='true' " : "")+ c._checkboxCssText + " />"
		if (labelHtml.toLowerCase().indexOf("<td>")>=0) {
			t += eospan
			eospan = "";
		}
		t += "<label for=\"" + id + "\">" + labelHtml + "</label>" +eospan;
		if (Jmol._debugAlert)
			alert(t);
		return Jmol._documentWrite(t);
	}

	c._getCommandInput = function(appletOrId, label, size, id, title) {
		var appId = c._getIdForControl(appletOrId, "x");
		if (appId == null)
			return "";
		//_jmolInitCheck();
		id != undefined && id != null || (id = "jmolCmd" + c._cmdCount);
		label != undefined && label != null || (label = "Execute");
		size != undefined && !isNaN(size) || (size = 60);
		++c._cmdCount;
		var t = "<span id=\"span_"+id+"\""+(title ? " title=\"" + title + "\"":"")+"><input name='" + id + "' id='" + id +
						"' size='"+size+"' onkeypress='Jmol.controls._commandKeyPress(event,\""+id+"\",\"" + appId + "\")' /><input " +
						" type='button' name='" + id + "Btn' id='" + id + "Btn' value = '"+label+"' onclick='Jmol.controls._commandKeyPress(13,\""+id+"\",\"" + appId + "\")' /></span>";
		if (Jmol._debugAlert)
			alert(t);
		return Jmol._documentWrite(t);
	}

	c._getLink = function(appletOrId, script, label, id, title) {
		var appId = c._getIdForControl(appletOrId, script);
		if (appId == null)
			return "";
		//_jmolInitCheck();
		id != undefined && id != null || (id = "jmolLink" + c._linkCount);
		label != undefined && label != null || (label = script.substring(0, 32));
		++c._linkCount;
		var scriptIndex = c._addScript(appId, script);
		var t = "<span id=\"span_"+id+"\""+(title ? " title=\"" + title + "\"":"")+"><a name='" + id + "' id='" + id +
						"' href='javascript:Jmol.controls._click(null,"+scriptIndex+");' onmouseover='Jmol.controls._mouseOver(null,"+scriptIndex+");return true;' onmouseout='Jmol.controls._mouseOut()' " +
						c._linkCssText + ">" + label + "</a></span>";
		if (Jmol._debugAlert)
			alert(t);
		return Jmol._documentWrite(t);
	}

	c._getMenu = function(appletOrId, arrayOfMenuItems, size, id, title) {
		var appId = c._getIdForControl(appletOrId, null);
		var optgroup = null;
		//_jmolInitCheck();
		id != undefined && id != null || (id = "jmolMenu" + c._menuCount);
		++c._menuCount;
		var type = typeof arrayOfMenuItems;
		if (type != null && type == "object" && arrayOfMenuItems.length) {
			var len = arrayOfMenuItems.length;
			if (typeof size != "number" || size == 1)
				size = null;
			else if (size < 0)
				size = len;
			var sizeText = size ? " size='" + size + "' " : "";
			var t = "<span id=\"span_"+id+"\""+(title ? " title=\"" + title + "\"":"")+"><select name='" + id + "' id='" + id +
							"' onChange='Jmol.controls._menuSelected(this)'" +
							sizeText + c._menuCssText + ">";
			for (var i = 0; i < len; ++i) {
				var menuItem = arrayOfMenuItems[i];
				type = typeof menuItem;
				var script = null;
				var text = null;
				var isSelected = null;
				if (type == "object" && menuItem != null) {
					script = menuItem[0];
					text = menuItem[1];
					isSelected = menuItem[2];
				} else {
					script = text = menuItem;
				}
				appId = c._getIdForControl(appletOrId, script);
				if (appId == null)
					return "";
				text == null && (text = script);
				if (script=="#optgroup") {
					t += "<optgroup label='" + text + "'>";
				} else if (script=="#optgroupEnd") {
					t += "</optgroup>";
				} else {
					var scriptIndex = c._addScript(appId, script);
					var selectedText = isSelected ? "' selected='true'>" : "'>";
					t += "<option value='" + scriptIndex + selectedText + text + "</option>";
				}
			}
			t += "</select></span>";
			if (Jmol._debugAlert)
				alert(t);
			return Jmol._documentWrite(t);
		}
	}
	
	c._getRadio = function(appletOrId, script, labelHtml, isChecked, separatorHtml, groupName, id, title) {
		//_jmolInitCheck();
		if (c._radioGroupCount == 0)
			++c._radioGroupCount;
		groupName || (groupName = "jmolRadioGroup" + (c._radioGroupCount - 1));
		var t = c._radio(appletOrId, script, labelHtml, isChecked, separatorHtml, groupName, (id ? id : groupName + "_" + c._radioCount), title ? title : 0);
		if (t == null)
			return "";
		if (Jmol._debugAlert)
			alert(t);
		return Jmol._documentWrite(t);
	}
	
	c._getRadioGroup = function(appletOrId, arrayOfRadioButtons, separatorHtml, groupName, id, title) {
		/*
	
			array: [radio1,radio2,radio3...]
			where radioN = ["script","label",isSelected,"id","title"]
	
		*/
	
		//_jmolInitCheck();
		var type = typeof arrayOfRadioButtons;
		if (type != "object" || type == null || ! arrayOfRadioButtons.length) {
			alert("invalid arrayOfRadioButtons");
			return;
		}
		separatorHtml != undefined && separatorHtml != null || (separatorHtml = "&#xa0; ");
		var len = arrayOfRadioButtons.length;
		++c._radioGroupCount;
		groupName || (groupName = "jmolRadioGroup" + (c._radioGroupCount - 1));
		var t = "<span id='"+(id ? id : groupName)+"'>";
		for (var i = 0; i < len; ++i) {
			if (i == len - 1)
				separatorHtml = "";
			var radio = arrayOfRadioButtons[i];
			type = typeof radio;
			var s = null;
			if (type == "object") {
				t += (s = c._radio(appletOrId, radio[0], radio[1], radio[2], separatorHtml, groupName, (radio.length > 3 ? radio[3]: (id ? id : groupName)+"_"+i), (radio.length > 4 ? radio[4] : 0), title));
			} else {
				t += (s = c._radio(appletOrId, radio, null, null, separatorHtml, groupName, (id ? id : groupName)+"_"+i, title));
			}
			if (s == null)
			  return "";
		}
		t+="</span>"
		if (Jmol._debugAlert)
			alert(t);
		return Jmol._documentWrite(t);
	}
	
	
})(Jmol);
// JmolApi.js -- Jmol user functions  Bob Hanson hansonr@stolaf.edu

// BH 5/16/2013 9:01:41 AM checkbox group fix
// BH 1/15/2013 10:55:06 AM updated to default to HTML5 not JAVA
 
// along with this file you need at least JmolCore.js and JmolApplet.js. Also, if you want buttons, JmolControls.js
// in that order. Then include JmolApi.js. 

// default settings are below. Generally you would do something like this:

// jmol = "jmol"
// Info = {.....your settings if not default....}
// Jmol.jmolButton(jmol,....)
// jmol = Jmol.getApplet(jmol, Info)
// Jmol.script(jmol,"....")
// Jmol.jmolLink(jmol,....)
// etc. 
// first parameter is always the applet id, either the string "jmol" or the object defined by Jmol.getApplet()
// no need for waiting to start giving script commands. You can also define a callback function as part of Info.

// see JmolCore.js for details

// BH 8/12/2012 5:15:11 PM added Jmol.getAppletHtml()

(function (Jmol) {

	Jmol.getVersion = function(){return Jmol._jmolInfo.version};

	Jmol.getApplet = function(id, Info, checkOnly) {
  	// requires JmolApplet.js and java/JmolApplet*.jar
    /*
		var DefaultInfo = {
			color: "#FFFFFF", // applet object background color, as for older jmolSetBackgroundColor(s)
			width: 300,
			height: 300,
			addSelectionOptions: false,
			serverURL: "http://chemapps.stolaf.edu/jmol/jsmol/jsmol.php",
			defaultModel: "",
			script: null,
			src: null,
			readyFunction: null,
			use: "HTML5",//other options include JAVA, WEBGL, and IMAGE
			jarPath: "java",
			jarFile: "JmolApplet0.jar",
			isSigned: false,
			j2sPath: "j2s",
      coverImage: null,     // URL for image to display
      coverTitle: "",       // tip that is displayed before model starts to load
      coverCommand: "",     // Jmol command executed upon clicking image
      deferApplet: false,   // true == the model should not be loaded until the image is clicked
      deferUncover: false,  // true == the image should remain until command execution is complete 
			disableJ2SLoadMonitor: false,
			disableInitialConsole: false,
			debug: false
		};	 
    
    */
    return Jmol._Applet._getApplet(id, Info, checkOnly);
	}

	Jmol.getJMEApplet = function(id, Info, linkedApplet, checkOnly) {
  	// requires JmolJME.js and jme/JME.jar
    /*
		var DefaultInfo = {
			width: 300,
			height: 300,
			jarPath: "jme",
			jarFile: "JME.jar",
			options: "autoez"
			// see http://www2.chemie.uni-erlangen.de/services/fragment/editor/jme_functions.html
			// rbutton, norbutton - show / hide R button
			// hydrogens, nohydrogens - display / hide hydrogens
			// query, noquery - enable / disable query features
			// autoez, noautoez - automatic generation of SMILES with E,Z stereochemistry
			// nocanonize - SMILES canonicalization and detection of aromaticity supressed
			// nostereo - stereochemistry not considered when creating SMILES
			// reaction, noreaction - enable / disable reaction input
			// multipart - possibility to enter multipart structures
			// number - possibility to number (mark) atoms
			// depict - the applet will appear without editing butons,this is used for structure display only
		};		    
    */
    return Jmol._JMEApplet._getApplet(id, Info, linkedApplet, checkOnly);
  }
  	
	Jmol.getJSVApplet = function(id, Info, checkOnly) {
	  // requires JmolJSV.js and either JSpecViewApplet.jar or JSpecViewAppletSigned.jar  
    /*
  	var DefaultInfo = {
			width: 500,
			height: 300,
			debug: false,
			jarPath: ".",
			jarFile: "JSpecViewApplet.jar",
			isSigned: false,
			initParams: null,
			readyFunction: null,
			script: null
		};
    */
    return Jmol._JSVApplet._getApplet(id, Info, checkOnly);
  }	


////////////////// scripting ///////////////////
  
	Jmol.loadFile = function(applet, fileName, params){
		applet._loadFile(fileName, params);
	}

	Jmol.script = function(applet, script) {
    if (applet._checkDeferred(script)) 
      return;
		applet._script(script);
	}
	
	Jmol.scriptWait = function(applet, script) {
		return applet._scriptWait(script);
	}
	
	Jmol.scriptEcho = function(applet, script) {
		return applet._scriptEcho(script);
	}
	
	Jmol.scriptMessage = function(applet, script) {
		return applet._scriptMessage(script);
	}
	
	Jmol.scriptWaitOutput = function(applet, script) {
		return applet._scriptWait(script);
	}
	
	Jmol.scriptWaitAsArray = function(applet, script) {
		return applet._scriptWaitAsArray(script);
	}
	
	Jmol.search = function(applet, query, script) {
		applet._search(query, script);
	}

////////////////// "get" methods ///////////////////
	
	Jmol.evaluate = function(applet,molecularMath) {
		return applet._evaluate(molecularMath);
	}
	
  Jmol.getAppletHtml = function(applet) {
    return applet._code;
	}
		
	Jmol.getPropertyAsArray = function(applet,sKey,sValue) {
		return applet._getPropertyAsArray(sKey,sValue);
	}

	Jmol.getPropertyAsJavaObject = function(applet,sKey,sValue) {
		return applet._getPropertyAsJavaObject(sKey,sValue);
	}
	
	Jmol.getPropertyAsJSON = function(applet,sKey,sValue) {
		return applet._getPropertyAsJSON(sKey,sValue);
	}

	Jmol.getPropertyAsString = function(applet,sKey,sValue) {
		return applet._getPropertyAsString(sKey,sValue);
	}

	Jmol.getStatus = function(applet,strStatus) {
		return applet._getStatus(strStatus);
	}

	
////////////////// general methods ///////////////////

	Jmol.resizeApplet = function(applet,size) {
		return applet._resizeApplet(size);
	}

	Jmol.restoreOrientation = function(applet,id) {
		return applet._restoreOrientation(id);
	}
	
	Jmol.restoreOrientationDelayed = function(applet,id,delay) {
		return applet._restoreOrientationDelayed(id,delay);
	}
	
	Jmol.saveOrientation = function(applet,id) {
		return applet._saveOrientation(id);
	}
	
	Jmol.say = function(msg) {
		alert(msg);
	}

//////////// console functions /////////////

	Jmol.clearConsole = function(applet) {
		applet._clearConsole();
	}

	Jmol.getInfo = function(applet) {
		return applet._info;
	}

	Jmol.setInfo = function(applet, info, isShown) {
		applet._info = info;
		if (arguments.length > 2)
			applet._showInfo(isShown);
	}

	Jmol.showInfo = function(applet, tf) {
		applet._showInfo(tf);
	}


//////////// controls and HTML /////////////


	Jmol.jmolBr = function() {
		return Jmol._documentWrite("<br />");
	}

	Jmol.jmolButton = function(appletOrId, script, label, id, title) {
		return Jmol.controls._getButton(appletOrId, script, label, id, title);
	}
	
	Jmol.jmolCheckbox = function(appletOrId, scriptWhenChecked, scriptWhenUnchecked,
			labelHtml, isChecked, id, title) {
		return Jmol.controls._getCheckbox(appletOrId, scriptWhenChecked, scriptWhenUnchecked,
			labelHtml, isChecked, id, title);
	}


	Jmol.jmolCommandInput = function(appletOrId, label, size, id, title) {
		return Jmol.controls._getCommandInput(appletOrId, label, size, id, title);
	}
		
	Jmol.jmolHtml = function(html) {
		return Jmol._documentWrite(html);
	}
	
	Jmol.jmolLink = function(appletOrId, script, label, id, title) {
		return Jmol.controls._getLink(appletOrId, script, label, id, title);
	}

	Jmol.jmolMenu = function(appletOrId, arrayOfMenuItems, size, id, title) {
		return Jmol.controls._getMenu(appletOrId, arrayOfMenuItems, size, id, title);
	}

	Jmol.jmolRadio = function(appletOrId, script, labelHtml, isChecked, separatorHtml, groupName, id, title) {
		return Jmol.controls._getRadio(appletOrId, script, labelHtml, isChecked, separatorHtml, groupName, id, title);
	}

	Jmol.jmolRadioGroup = function (appletOrId, arrayOfRadioButtons, separatorHtml, groupName, id, title) {
		return Jmol.controls._getRadioGroup(appletOrId, arrayOfRadioButtons, separatorHtml, groupName, id, title);
	}

	Jmol.setCheckboxGroup = function(chkMaster, chkBoxes) {
    // chkBoxes can be an array or any number of additional string arguments
		Jmol.controls._cbSetCheckboxGroup(chkMaster, chkBoxes, arguments);
	}
	
	Jmol.setDocument = function(doc) {
		
		// If doc is null or 0, Jmol.getApplet() will still return an Object, but the HTML will
		// put in applet._code and not written to the page. This can be nice, because then you 
		// can still refer to the applet, but place it on the page after the controls are made. 
		//
		// This really isn't necessary, though, because there is a simpler way: Just define the 
		// applet variable like this:
		//
		// jmolApplet0 = "jmolApplet0"
		//
		// and then, in the getApplet command, use
		//
		// jmolapplet0 = Jmol.getApplet(jmolApplet0,....)
		// 
		// prior to this, "jmolApplet0" will suffice, and after it, the Object will work as well
		// in any button creation 
		//		 
		//  Bob Hanson 25.04.2012
		
		Jmol._document = doc;
	}

	Jmol.setXHTML = function(id) {
		Jmol._isXHTML = true;
		Jmol._XhtmlElement = null;
		Jmol._XhtmlAppendChild = false;
		if (id){
			Jmol._XhtmlElement = document.getElementById(id);
			Jmol._XhtmlAppendChild = true;
		}
	}

	////////////////////////////////////////////////////////////////
	// Cascading Style Sheet Class support
	////////////////////////////////////////////////////////////////
	
	// BH 4/25 -- added text option. setAppletCss(null, "style=\"xxxx\"")
	// note that since you must add the style keyword, this can be used to add any attribute to these tags, not just css. 
	
	Jmol.setAppletCss = function(cssClass, text) {
		cssClass != null && (Jmol.controls._appletCssClass = cssClass);
		Jmol.controls._appletCssText = text ? text + " " : cssClass ? "class=\"" + cssClass + "\" " : "";
	}
	
	Jmol.setButtonCss = function(cssClass, text) {
		cssClass != null && (Jmol.controls._buttonCssClass = cssClass);
		Jmol.controls._buttonCssText = text ? text + " " : cssClass ? "class=\"" + cssClass + "\" " : "";
	}
	
	Jmol.setCheckboxCss = function(cssClass, text) {
		cssClass != null && (Jmol.controls._checkboxCssClass = cssClass);
		Jmol.controls._checkboxCssText = text ? text + " " : cssClass ? "class=\"" + cssClass + "\" " : "";
	}
	
	Jmol.setRadioCss = function(cssClass, text) {
		cssClass != null && (Jmol.controls._radioCssClass = cssClass);
		Jmol.controls._radioCssText = text ? text + " " : cssClass ? "class=\"" + cssClass + "\" " : "";
	}
	
	Jmol.setLinkCss = function(cssClass, text) {
		cssClass != null && (Jmol.controls._linkCssClass = cssClass);
		Jmol.controls._linkCssText = text ? text + " " : cssClass ? "class=\"" + cssClass + "\" " : "";
	}
	
	Jmol.setMenuCss = function(cssClass, text) {
		cssClass != null && (Jmol.controls._menuCssClass = cssClass);
		Jmol.controls._menuCssText = text ? text + " ": cssClass ? "class=\"" + cssClass + "\" " : "";
	}

  Jmol.setAppletSync = function(applets, commands, isJmolJSV) {
    Jmol._syncedApplets = applets;   // an array of appletIDs
    Jmol._syncedCommands = commands; // an array of commands; one or more may be null 
    Jmol._syncedReady = {};
    Jmol._isJmolJSVSync = isJmolJSV;
	}
	
	/*
	Jmol._grabberOptions = [
	  ["$", "NCI(small molecules)"],
	  [":", "PubChem(small molecules)"],
	  ["=", "RCSB(macromolecules)"]
	];
	*/
	
	Jmol.setGrabberOptions = function(options) {
	  Jmol._grabberOptions = options;
	}

  Jmol.setAppletHtml = function (applet, divid) {
    if (!applet._code) 
      return;
    Jmol.$html(divid, applet._code);
    if (applet._init && !applet._deferApplet)
      applet._init();
  }

  Jmol.coverApplet = function(applet, doCover) {
    if (applet._cover)
      applet._cover(doCover);
  }
  


})(Jmol);
// j2sjmol.js 

// Java programming notes:
//   
//   There are a few motifs to avoid when optimizing Java code to work smoothly
//   with the J2S compiler:
//   
//   arrays: 
//   
// 1. an array with null elements cannot be typed and must be avoided.
// 2. instances of Java "instance of" involving arrays must be found and convered to calls to Clazz.isA...
// 3. new int[n][] must not be used. Use instead J.util.ArrayUtil.newInt2(n);
// 4. new int[] { 1, 2, 3 } has problems because it creates simply [ ] and not IntArray32
//   
//   numbers:
//   
// 1. Remember that EVERY number in JavaScript is a double -- doesn't matter if it is in IntArray32 or not. 
// 2. You cannot reliably use Java long, because doubles consume bits for the exponent which cannot be tested.
// 3. Bit 31 of an integer is unreliable, since (int) -1 is now  , not just 0zFFFFFFFF, and 
//    FFFFFFFF + 1 = 100000000, not 0. In JavaScript, 0xFFFFFFFF is 4294967295, not -1.
//    This means that writeInt(b) will fail if b is negative. What you need is instead
//    writeInt((int)(b & 0xFFFFFFFFl) so that JavaScript knocks off the high bits explicitly. 
//
//   general:
//
// 1. j2sRequireImport xxxx is needed if xxxx is a method used in a static function

 // NOTES by Bob Hanson: 

  
 // J2S class changes:
 
 // BH 6/15/2013 8:02:07 AM corrections to Class.isAS to return true if first element is null
 // BH 6/14/2013 4:41:09 PM corrections to Clazz.isAI and related methods to include check for null object
 // BH 3/17/2013 11:54:28 AM adds stackTrace for ERROR 

 // BH 3/13/2013 6:58:26 PM adds Clazz.clone(me) for BS clone 
 // BH 3/12/2013 6:30:53 AM fixes Clazz.exceptionOf for ERROR condition trapping
 // BH 3/2/2013 9:09:53 AM delete globals c$ and $fz
 // BH 3/2/2013 9:10:45 AM optimizing defineMethod using "look no further" "@" parameter designation (see "\\@" below -- removed 3/23/13)
 // BH 2/27/2013 optimizing getParamsType for common cases () and (Number)
 // BH 2/27/2013 optimizing SAEM delegation for hashCode and equals -- disallows overloading of equals(Object)
 
 // BH 2/23/2013 found String.replaceAll does not work -- solution was to never call it.
 // BH 2/9/2013 9:18:03 PM Int32Array/Float64Array fixed for MSIE9
 // BH 1/25/2013 1:55:31 AM moved package.js from j2s/java to j2s/core 
 // BH 1/17/2013 4:37:17 PM String.compareTo() added
 // BH 1/17/2013 4:52:22 PM Int32Array and Float64Array may not have .prototype.sort method
 // BH 1/16/2013 6:20:34 PM Float64Array not available in Safari 5.1
 // BH 1/14/2013 11:28:58 PM  Going to all doubles in JavaScript (Float64Array, not Float32Array)
 //   so that (new float[] {13.48f})[0] == 13.48f, effectively

 // BH 1/14/2013 12:53:41 AM  Fix for Opera 10 not loading any files
 // BH 1/13/2013 11:50:11 PM  Fix for MSIE not loading (nonbinary) files locally
 
 // BH 12/1/2012 9:52:26 AM Compiler note: Thread.start() cannot be executed within the constructor;
 
 // BH 11/24/2012 11:08:39 AM removed unneeded sections
 // BH 11/24/2012 10:23:22 AM  all XHR uses sync loading (ClazzLoader.setLoadingMode)
 // BH 11/21/2012 7:30:06 PM 	if (base != null)	map["@" + pkg] = base;  critical for multiple applets

 // BH 10/8/2012 3:27:41 PM         if (clazzName.indexOf("Array") >= 0) return "Array"; in Clazz.getClassName for function
 // BH removed Clazz.ie$plit = "\\2".split (/\\/).length == 1; unnecessary; using RegEx slows process significantly in all browsers
 // BH 10/6/12 added Int32Array, Float32Array, newArrayBH, upgraded java.lang and java.io
 // BH added Integer.bitCount in core.z.js
 // BH changed alert to Clazz.alert in java.lang.Class.js *.ClassLoader.js, java.lang.thread.js
 // BH removed toString from innerFunctionNames due to infinite recursion
 // BH note: Logger.error(null, e) does not work -- get no constructor for (String) (TypeError)
 // BH added j2s.lib.console
 // BH allowed for alias="."
 // BH removed alert def --> Clazz.alert
 // BH added wrapper at line 2856 
 // BH newArray fix at line 2205
 // BH System.getProperty fix at line 6693
 // BH added Enum .value() method at line 2183
 // BH added System.getSecurityManager() at end
 // BH added String.contains() at end
 // BH added System.gc() at end
 // BH added Clazz.exceptionOf = updated
 // BH added String.getBytes() at end
 

LoadClazz = function() {

if (!window["j2s.clazzloaded"])
window["j2s.clazzloaded"] = false;

if (window["j2s.clazzloaded"])return;

window["j2s.clazzloaded"] = true;

// BH this just allows me to monitor exactly what is being delegated
// I run xxxShowParams from the developer console in the browser.
bhtest = false;
xxxbhtest=""
xxxbhparams = {};
xxxShowParams = function() {
  var s = "";
  for (var i in xxxbhparams) {
    s += ("#P " + xxxbhparams[i] + "\t" + i + "\n");
  }
  xxxbhparams= {};
  alert(s)
}


window["j2s.object.native"] = true;

 // Clazz changes:

 /* http://j2s.sf.net/ *//******************************************************************************
 * Copyright (c) 2007 java2script.org and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     Zhou Renjian - initial API and implementation
 *****************************************************************************/
/*******
 * @author zhou renjian
 * @create Nov 5, 2005
 *******/
 
//if (window["Clazz"] == null) {

/*
 * The following *-# are used to compress the JavaScript file into small file.
 * For more details, please read /net.sf.j2s.lib/build/build.xml
 */
/*-#
 # _x_CLASS_NAME__ -> C$N
 # _x_PKG_NAME__ -> P$N
 #
 # clazzThis -> Tz
 # objThis -> To
 # clazzHost -> Hz
 # hostThis -> Th
 # hostSuper -> Sh
 # clazzFun -> Fc
 # clazzName -> Nc
 # funName -> Nf
 # funBody -> Bf
 # objType -> oT
 #  
 # qClazzName ->Nq
 #-*/
/**
 * Class Clazz. All the methods are static in this class.
 */
/* static */
Class = Clazz = function () {};


//Clazz.dateToString = Date.prototype.toString

;(function(Clazz) {


NullObject = function () {};

JavaObject = Object;

/* protected */
Clazz.supportsNativeObject = window["j2s.object.native"];

JavaObject = (Clazz.supportsNativeObject ? function () {} : Object);

ClazzLoaderProgressMonitor = ClassLoaderProgressMonitor = {};
Clazz.Console = {};
Clazz.dateToString = Date.prototype.toString;

System = new JavaObject ();

Clazz.debuggingBH = false;

Clazz.getSignature = function(proto, name, func, isNew) {
  // BH pointing to signatures based on number of parameters
  // would only make SAEM somewhat faster; not worth it? 
  // better to just avoid SAEM altogether
  return (isNew ? proto[name] = func : proto[name]);
/*
  if (!isNew) return proto[name];
  if (proto[name])
    func.sigs = proto[name].sigs;
  proto[name] = func;
  if (!func.sigs) func.sigs = [];
  var n = (func.arguments ? func.arguments.length : 0);
  if (func.sigs[n])
    func.sigs[n] == -1; // overloaded for this number of parameters
  else                  // unique for this function
    func.sigs[n] = func;
  return func;
*/
};


Clazz.addProto = function(proto, name, func) {
  Clazz.getSignature(proto, name, func, true); // BH
};

;(function(proto) {
  Clazz.addProto(proto, "equals", function (obj) {
  	return this == obj;
  });

  Clazz.addProto(proto, "hashCode", function () {
  	try {
  		return this.toString ().hashCode ();
  	} catch (e) {
  		var str = ":";
  		for (var s in this) {
  			str += s + ":"
  		}
  		return str.hashCode ();
  	}
  });

  Clazz.addProto(proto, "getClass", function () {
	 return Clazz.getClass (this);
  });

  Clazz.addProto(proto, "clone", function () {
    return Clazz.clone(this);
  });

  Clazz.clone = function(me) {
    // BH allows @j2sNative access without super constructor
  	var o = new me.constructor ();
  	for (var i in me)
  		o[i] = me[i];
  	return o;
  }
/*
 * Methods for thread in Object
 */
  Clazz.addProto(proto, "finalize", function () {});
  Clazz.addProto(proto, "notify", function () {});
  Clazz.addProto(proto, "notifyAll", function () {});
  Clazz.addProto(proto, "wait", function () {});

  Clazz.addProto(proto, "to$tring", Object.prototype.toString);
  Clazz.addProto(proto, "toString", function () {
  	if (this.__CLASS_NAME__ != null) {
  		return "[" + this.__CLASS_NAME__ + " object]";
  	} else {
  		return this.to$tring.apply (this, arguments);
  	}
  });


  if (Clazz.supportsNativeObject) {
  	/* protected */
  	Clazz.extendedObjectMethods = [
  			"equals", "hashCode", "getClass", "clone", "finalize", "notify", "notifyAll", "wait", "to$tring", "toString"
  	];
  
  	for (var i = 0; i < Clazz.extendedObjectMethods.length; i++) {
  		var p = Clazz.extendedObjectMethods[i];
  		Array.prototype[p] = JavaObject.prototype[p];
  	}
  	JavaObject.__CLASS_NAME__ = "Object";
  	JavaObject["getClass"] = function () { return JavaObject; }; 
  }


})(JavaObject.prototype);

Clazz.extendJO = function(c, name) {  
  if (name)
    c.__CLASS_NAME__ = c.prototype.__CLASS_NAME__ = name;
  if (Clazz.supportsNativeObject) {
  	for (var i = 0; i < Clazz.extendedObjectMethods.length; i++) {
  		var p = Clazz.extendedObjectMethods[i];
  		Clazz.getSignature(c.prototype, p, JavaObject.prototype[p], true);
  	}
  }
};

/**
 * Try to fix bug on Safari
 */
//InternalFunction = Object;

Clazz.extractClassName = function(clazzStr) {
  // [object Int32Array]
	var clazzName = clazzStr.substring (1, clazzStr.length - 1);
	return (clazzName.indexOf("Array") >= 0 ? "Array" // BH -- for Float64Array and Int32Array
    : clazzName.indexOf ("object ") >= 0 ? clazzName.substring (7) // IE
    : clazzName);
}
/**
 * Return the class name of the given class or object.
 *
 * @param clazzHost given class or object
 * @return class name
 */
/* public */
Clazz.getClassName = function (obj) {
	if (obj == null) {
		/* 
		 * null is always treated as Object.
		 * But what about "undefined"?
		 */
		return "NullObject";
	}
	
  if (obj instanceof Clazz.CastedNull)
		return obj.clazzName;
	switch(typeof obj) {
	case "number":
		return "Number";
	case "boolean":
		return "Boolean";
	case "string":
		/* 
		 * Always treat the constant string as String object.
		 * This will be compatiable with Java String instance.
		 */
		return "String";
  case "function":
		if (obj.__CLASS_NAME__ != null)
      return (arguments[1] ? obj.__CLASS_NAME__ : "Class"); /* user defined class name */
		var s = obj.toString();
		var idx0 = s.indexOf("function");
		if (idx0 < 0)
			return (s.charAt(0) == '[' ? Clazz.extractClassName(s) : s.replace(/[^a-zA-Z0-9]/g, ''));
		var idx1 = idx0 + 8;
		var idx2 = s.indexOf ("(", idx1);
		if (idx2 < 0)
			return "Object";
		s = s.substring (idx1, idx2);
    if (s.indexOf("Array") >= 0)
      return "Array";  // BH -- for Float64Array and Int32Array
		s = s.replace (/^\s+/, "").replace (/\s+$/, ""); // .trim ()
		return (s == "anonymous" || s == "" ? "Function" : s);
     // BH -- for general functions, clazzName may be ""
	case "object":
		if (obj.__CLASS_NAME__ != null) // user defined class name
			return obj.__CLASS_NAME__;
    if (obj.constructor == null)
			return "Object"; // For HTML Element in IE
    if (obj.constructor.__CLASS_NAME__ == null) {
			if (obj instanceof Number)
				return "Number";
      if (obj instanceof Boolean)
				return "Boolean";
      if (obj instanceof Array)
				return "Array";
			var s = obj.toString();
			if (s.charAt(0) == '[')
        return Clazz.extractClassName(s);
		}
	}
	return Clazz.getClassName (obj.constructor, true);
};
/**
 * Return the class of the given class or object.
 *
 * @param clazzHost given class or object
 * @return class name
 */
/* public */
Clazz.getClass = function (clazzHost) {
	if (clazzHost == null) {
		/* 
		 * null is always treated as Object.
		 * But what about "undefined"?
		 */
		return JavaObject;
	}
	if (typeof clazzHost == "function") {
		return clazzHost;
	} else {
		var clazzName = null;
		var obj = clazzHost;
		if (obj instanceof Clazz.CastedNull) {
			clazzName = obj.clazzName;
		} else {
			var objType = typeof obj;
			if (objType == "string") {
				return String;
			} else if (typeof obj == "object") {
				/* user defined class name */
				if (obj.__CLASS_NAME__ != null) {
					clazzName = obj.__CLASS_NAME__;
				} else if (obj.constructor == null) {
					return JavaObject; // Is it safe?
				} else {
					return obj.constructor;
				}
			}
		}
		if (clazzName != null) {
			return Clazz.evalType (clazzName, true);
		} else {
			return obj.constructor;
		}
	}
};

/*
 * Be used to copy members of class
 */
/* protected */
/*-# extendsProperties -> eP #-*/
Clazz.extendsProperties = function (hostThis, hostSuper) {
	for (var o in hostSuper) {
		if (o != "b$" && o != "prototype" && o != "superClazz"
				&& o != "__CLASS_NAME__" && o != "implementz"
				&& !Clazz.checkInnerFunction (hostSuper, o)) {
			hostThis[o] = hostSuper[o];
		}
	}
};

/* private */
/*-# checkInnerFunction -> cIF #-*/
Clazz.checkInnerFunction = function (hostSuper, funName) {
	for (var k = 0; k < Clazz.innerFunctionNames.length; k++) {
		if (funName == Clazz.innerFunctionNames[k] && 
				Clazz.innerFunctions[funName] === hostSuper[funName]) {
			return true;
		}
	}
	return false;
};

/*
 * Be used to copy members of interface
 */
/* protected */
/*-# implementsProperties -> ip #-*/
Clazz.implementsProperties = function (hostThis, hostSuper) {
	for (var o in hostSuper) {
		if (o != "b$" && o != "prototype" && o != "superClazz"
				&& o != "__CLASS_NAME__" && o != "implementz") {
			if (typeof hostSuper[o] == "function") {
				/*
				 * static final member of interface may be a class, which may
				 * be function.
				 */
				if (Clazz.checkInnerFunction (hostSuper, o)) {
					continue;
				}
			}
			hostThis[o] = hostSuper[o];
			hostThis.prototype[o] = hostSuper[o];
		}
	}
};

/*-# args4InheritClass -> aIC #-*/
Clazz.args4InheritClass = function () {
};
Clazz.inheritArgs = new Clazz.args4InheritClass ();

/**
 * Inherit class with "extends" keyword and also copy those static members. 
 * Example, as in Java, if NAME is a static member of ClassA, and ClassB 
 * extends ClassA then ClassB.NAME can be accessed in some ways.
 *
 * @param clazzThis child class to be extended
 * @param clazzSuper super class which is inherited from
 * @param objSuper super class instance
 */
/* protected */
/*-#
 # inheritClass -> xic 
 #
 # objSuper -> oSp
 #-*/
Clazz.inheritClass = function (clazzThis, clazzSuper, objSuper) {
	//var thisClassName = Clazz.getClassName (clazzThis);
	Clazz.extendsProperties (clazzThis, clazzSuper);
	if (Clazz.isClassUnloaded (clazzThis)) {
		// Don't change clazzThis.protoype! Keep it!
	} else if (objSuper != null) {
		// ! Unsafe reference prototype to an instance!
		// Feb 19, 2006 --josson
		// OK for this reference to an instance, as this is anonymous instance,
		// which is not referenced elsewhere.
		// March 13, 2006
		clazzThis.prototype = objSuper; 
	} else if (clazzSuper !== Number) {
		clazzThis.prototype = new clazzSuper (Clazz.inheritArgs);
	} else { // Number
		clazzThis.prototype = new Number ();
	}
	clazzThis.superClazz = clazzSuper;
	/*
	 * Is it necessary to reassign the class name?
	 * Mar 10, 2006 --josson
	 */
	//clazzThis.__CLASS_NAME__ = thisClassName;
	clazzThis.prototype.__CLASS_NAME__ = clazzThis.__CLASS_NAME__;
};

/**
 * Implementation of Java's keyword "implements".
 * As in JavaScript there are on "implements" keyword implemented, a property
 * of "implementz" is added to the class to record the interfaces the class
 * is implemented.
 * 
 * @param clazzThis the class to implement
 * @param interfacez Array of interfaces
 */
/* public */
Clazz.implementOf = function (clazzThis, interfacez) {
	if (arguments.length >= 2) {
		if (clazzThis.implementz == null) {
			clazzThis.implementz = new Array ();
		}
		var impls = clazzThis.implementz;
		if (arguments.length == 2) {
			if (typeof interfacez == "function") {
				impls[impls.length] = interfacez;
				Clazz.implementsProperties (clazzThis, interfacez);
			} else if (interfacez instanceof Array) {
				for (var i = 0; i < interfacez.length; i++) {
					impls[impls.length] = interfacez[i];
					Clazz.implementsProperties (clazzThis, interfacez[i]);
				}
			}
		} else {
			for (var i = 1; i < arguments.length; i++) {
				impls[impls.length] = arguments[i];
				Clazz.implementsProperties (clazzThis, arguments[i]);
			}
		}
	}
};

/**
 * TODO: More should be done for interface's inheritance
 */
/* public */
Clazz.extendInterface = Clazz.implementOf;

/* protected */
/*-#
 # equalsOrExtendsLevel -> eOE 
 #
 # clazzAncestor -> anc
 #-*/
Clazz.equalsOrExtendsLevel = function (clazzThis, clazzAncestor) {
	if (clazzThis === clazzAncestor) {
		return 0;
	}
	if (clazzThis.implementz != null) {
		var impls = clazzThis.implementz;
		for (var i = 0; i < impls.length; i++) {
			var level = Clazz.equalsOrExtendsLevel (impls[i], clazzAncestor);
			if (level >= 0) {
				return level + 1;
			}
		}
	}
	return -1;
};

/* protected */
/*-#
 # getInheritedLevel -> gIL 
 #
 # clazzBase -> bs
 # clazzTarget -> tg
 #-*/
Clazz.getInheritedLevel = function (clazzTarget, clazzBase) {
	if (clazzTarget === clazzBase) {
		return 0;
	}
	var isTgtStr = (typeof clazzTarget == "string");
	var isBaseStr = (typeof clazzBase == "string");
	if ((isTgtStr && ("void" == clazzTarget || "unknown" == clazzTarget)) 
			|| (isBaseStr && ("void" == clazzBase 
					|| "unknown" == clazzBase))) {
		return -1;
	}
	/*
	 * ? The following lines are confusing
	 * March 10, 2006
	 */
	if ((isTgtStr && "NullObject" == clazzTarget) 
			|| NullObject === clazzTarget) {
		if (clazzBase !== Number && clazzBase !== Boolean
				&& clazzBase !== NullObject) {
			return 0;
		}
	}
	if (isTgtStr) {
		clazzTarget = Clazz.evalType (clazzTarget);
	}
	if (isBaseStr) {
		clazzBase = Clazz.evalType (clazzBase);
	}
	if (clazzBase == null || clazzTarget == null) {
		return -1;
	}
	var level = 0;
	var zzalc = clazzTarget; // zzalc <--> clazz
	while (zzalc !== clazzBase && level < 10) {
		/* maybe clazzBase is interface */
		if (zzalc.implementz != null) {
			var impls = zzalc.implementz;
			for (var i = 0; i < impls.length; i++) {
				var implsLevel = Clazz.equalsOrExtendsLevel (impls[i], 
						clazzBase);
				if (implsLevel >= 0) {
					return level + implsLevel + 1;
				}
			}
		}
		
		zzalc = zzalc.superClazz;
		if (zzalc == null) {
			if (clazzBase === Object || clazzBase === JavaObject) {
				/*
				 * getInheritedLevel(String, CharSequence) == 1
				 * getInheritedLevel(String, Object) == 1.5
				 * So if both #test(CharSequence) and #test(Object) existed,
				 * #test("hello") will correctly call #test(CharSequence)
				 * insted of #test(Object).
				 */
				return level + 1.5; // 1.5! Special!
			} else {
				return -1;
			}
		}
		level++;
	}
	return level;
};


/**
 * Implements Java's keyword "instanceof" in JavaScript's way.
 * As in JavaScript part of the object inheritance is implemented in only-
 * JavaScript way.
 *
 * @param obj the object to be tested
 * @param clazz the class to be checked
 * @return whether the object is an instance of the class
 */
/* public */
Clazz.instanceOf = function (obj, clazz) {
	if (obj == null) {
		return clazz == false; // should usually false
	}
	if (clazz == null) {
		return false;
	}
	if (obj instanceof clazz) {
		return true;
	} else {
		/*
		 * To check all the inherited interfaces.
		 */
		var clazzName = Clazz.getClassName (obj);
		return Clazz.getInheritedLevel (clazzName, clazz) >= 0;
	}
};

/**
 * Call super method of the class. 
 * The same effect as Java's expression:
 * <code> super.* () </code>
 * 
 * @param objThis host object
 * @param clazzThis class of declaring method scope. It's hard to determine 
 * which super class is right class for "super.*()" call when it's in runtime
 * environment. For example,
 * 1. ClasssA has method #run()
 * 2. ClassB extends ClassA overriding method #run() with "super.run()" call
 * 3. ClassC extends ClassB
 * 4. objC is an instance of ClassC
 * Now we have to decide which super #run() method is to be invoked. Without
 * explicit clazzThis parameter, we only know that objC.getClass() is ClassC 
 * and current method scope is #run(). We do not known we are in scope 
 * ClassA#run() or scope of ClassB#run(). if ClassB is given, Clazz can search
 * all super methods that are before ClassB and get the correct super method.
 * This is the reason why there must be an extra clazzThis parameter.
 * @param funName method name to be called
 * @param funParams Array of method parameters
 */
/* public */
Clazz.superCall = function (objThis, clazzThis, funName, funParams) {
	var fx = null;
	var i = -1;
	var clazzFun = objThis[funName];
  
	if (clazzFun != null) {
		if (clazzFun.claxxOwner != null) { 
			// claxxOwner is a mark for methods that is single.
			if (clazzFun.claxxOwner !== clazzThis) {
				// This is a single method, call directly!
				fx = clazzFun;
			}
		} else if (clazzFun.stacks == null && !(clazzFun.lastClaxxRef != null
					&& clazzFun.lastClaxxRef.prototype[funName] != null
					&& clazzFun.lastClaxxRef.prototype[funName].stacks != null)) { // super.toString
			fx = clazzFun;
		} else { // normal wrapped method
			var stacks = clazzFun.stacks;
			if (stacks == null) {
				stacks = clazzFun.lastClaxxRef.prototype[funName].stacks;
			}
			var length = stacks.length;
			for (i = length - 1; i >= 0; i--) {
				/*
				 * Once super call is computed precisely, there are no need 
				 * to calculate the inherited level but just an equals
				 * comparision
				 */
				//var level = Clazz.getInheritedLevel (clazzThis, stacks[i]);
				if (clazzThis === stacks[i]) { // level == 0
					if (i > 0) {
						i--;
						fx = stacks[i].prototype[funName];
					} else {
						/*
						 * Will this case be reachable?
						 * March 4, 2006
						 * Should never reach here if all things are converted
						 * by Java2Script
						 */
						fx = stacks[0].prototype[funName]["\\unknown"];



      


      
        //if (funName == "clone")alert("fx=" + fx) 
 




      

        
      




					}
					break;
				} else if (Clazz.getInheritedLevel (clazzThis, 
						stacks[i]) > 0) {
					fx = stacks[i].prototype[funName];

					break;
				}
			} // end of for loop
		} // end of normal wrapped method
	} // end of clazzFun != null
  
  
	if (fx != null) {
		/* there are members which are initialized out of the constructor */
		if (i == 0 && funName == "construct") {
			var ss = clazzFun.stacks;
			if (ss != null && ss[0].superClazz == null
					&& ss[0].con$truct != null) {
				ss[0].con$truct.apply (objThis, []);
			}
		}
		/*# {$no.debug.support} >>x #*/
		if (Clazz.tracingCalling) {
			var caller = arguments.callee.caller;
			if (caller === Clazz.superConstructor) {
				caller = caller.arguments.callee.caller;
			}
			Clazz.pu$hCalling (new Clazz.callingStack (caller, clazzThis));
			var ret = fx.apply (objThis, (funParams == null) ? [] : funParams);
			Clazz.p0pCalling ();
			return ret;
		}
		/*# x<< #*/

      
		return fx.apply (objThis, (funParams == null) ? [] : funParams);
	} else if (funName == "construct") {
		/* there are members which are initialized out of the constructor */
		/* No super constructor! */
		return ;
	}
	Clazz.alert(["j2slib","no class found",(funParams).typeString])
	throw new Clazz.MethodNotFoundException (objThis, clazzThis, funName, 
			Clazz.getParamsType (funParams).typeString);
};

/**
 * Call super constructor of the class. 
 * The same effect as Java's expression: 
 * <code> super () </code>
 */
/* public */
Clazz.superConstructor = function (objThis, clazzThis, funParams) {
	Clazz.superCall (objThis, clazzThis, "construct", funParams);
	/* If there are members which are initialized out of the constructor */
	if (clazzThis.con$truct != null) {
		clazzThis.con$truct.apply (objThis, []);
	}
};

/**
 * Class for null with a given class as to be casted.
 * This class will be used as an implementation of Java's casting way.
 * For example,
 * <code> this.call ((String) null); </code>
 */
/* protcted */
Clazz.CastedNull = function (asClazz) {
	if (asClazz != null) {
		if (asClazz instanceof String) {
			this.clazzName = asClazz;
		} else if (asClazz instanceof Function) {
			this.clazzName = Clazz.getClassName (asClazz, true);
		} else {
			this.clazzName = "" + asClazz;
		}
	} else {
		this.clazzName = "Object";
	}
	this.toString = function () {
		return null;
	};
	this.valueOf = function () {
		return null;
	};
};

/**
 * API for Java's casting null.
 * @see Clazz.CastedNull
 *
 * @param asClazz given class
 * @return an instance of class Clazz.CastedNull
 */
/* public */
Clazz.castNullAs = function (asClazz) {
	return new Clazz.CastedNull (asClazz);
};

/** 
 * MethodException will be used as a signal to notify that the method is
 * not found in the current clazz hierarchy.
 */
/* private */
Clazz.MethodException = function () {
};
/* protected */
Clazz.MethodNotFoundException = function () {
	this.toString = function () {
		return "MethodNotFoundException";
	};
};

/* private */
/*x-# getParamsType -> gPT #-x*/
Clazz.getParamsType = function (funParams) {
  // bh: optimization here for very common cases
  var n = funParams.length;
  switch (n) {
  case 0:
		var params = ["void"];
		params.typeString = "\\void";
    return params;
  case 1:
    // just so common
    var obj = funParams[0];
  	if (obj != null && typeof obj == "number") {
		  var params = ["Number"];
		  params.typeString = "\\Number";
      return params;
    }
	}
	
	var params = [];
	params.hasCastedNull = false;
	if (funParams != null) {
		for (var i = 0; i < n; i++) {
			params[i] = Clazz.getClassName (funParams[i]);
			if (funParams[i] instanceof Clazz.CastedNull) {
				params.hasCastedNull = true;
			}
		}
	}
	params.typeString = "\\" + params.join ('\\');
	return params;
};




/**
 * Search the given class prototype, find the method with the same
 * method name and the same parameter signatures by the given 
 * parameters, and then run the method with the given parameters.
 *
 * @param objThis the current host object
 * @param claxxRef the current host object's class
 * @param fxName the method name
 * @param funParams the given arguments
 * @return the result of the specified method of the host object,
 * the return maybe void.
 * @throws Clazz.MethodNotFoundException if no matched method is found
 */
/* protected */
/*-# searchAndExecuteMethod -> saem #-*/


Clazz.searchAndExecuteMethod = function (objThis, claxxRef, fxName, funParams) {
	var fx = objThis[fxName];
	var params = Clazz.getParamsType (funParams);

 if (bhtest && self.JSON) { 
  var s = claxxRef.__CLASS_NAME__ + " " + fxName + " " + JSON.stringify(params);
  if (!xxxbhparams[s]) {
    xxxbhparams[s] = 0;
  }
  xxxbhparams[s]++;
 }
  

	/*
	 * Cache last matched method
	 */
	if (fx.lastParams == params.typeString && fx.lastClaxxRef === claxxRef) {
		var methodParams = null;
		if (params.hasCastedNull) {
			methodParams = new Array ();
			for (var k = 0; k < funParams.length; k++) {
				if (funParams[k] instanceof Clazz.CastedNull) {
					/*
					 * For Clazz.CastedNull instances, the type name is
					 * already used to indentified the method in Clazz#
					 * searchMethod.
					 */
					methodParams[k] = null;
				} else {
					methodParams[k] = funParams[k];
				}
			}
		} else {
			methodParams = funParams;
		}
		if (fx.lastMethod != null) {
			return fx.lastMethod.apply (objThis, methodParams);
		} else { // missed default constructor ?
			return ;
		}
	}
	fx.lastParams = params.typeString;
	fx.lastClaxxRef = claxxRef;

	var stacks = fx.stacks;
	if (stacks == null) {
		stacks = claxxRef.prototype[fxName].stacks;
	}
	var length = stacks.length;

	/*
	 * Search the inheritance stacks to get the given class' function
	 */
	var began = false; // began to search its super classes
	for (var i = length - 1; i > -1; i--) {
		//if (Clazz.getInheritedLevel (claxxRef, stacks[i]) >= 0) {
		/*
		 * No need to calculate the inherited level as there always exist a 
		 * right claxxRef in the stacks, and the inherited level of stacks
		 * are in order.
		 */
		if (began || stacks[i] === claxxRef) {
			/*
			 * First try to search method within the same class scope
			 * with stacks[i] === claxxRef
			 */
			var clazzFun = stacks[i].prototype[fxName];

			var ret = Clazz.tryToSearchAndExecute (fxName, objThis, clazzFun, params,
					funParams/*, isSuper, clazzThis*/, fx);
			if (!(ret instanceof Clazz.MethodException)) {
				return ret;
			}
			/*
			 * As there are no such methods in current class, Clazz will try 
			 * to search its super class stacks. Here variable began indicates
			 * that super searchi is began, and there is no need checking
			 * <code>stacks[i] === claxxRef</code>
			 */
			began = true; 
		} // end of if
	} // end of for
	if ("construct" == fxName) {
		/*
		 * For non existed constructors, just return without throwing
		 * exceptions. In Java codes, extending Object can call super
		 * default Object#constructor, which is not defined in JS.
		 */
		return ;
	}
	// TODO: should be java.lang.NoSuchMethodException
	throw new Clazz.MethodNotFoundException (objThis, claxxRef, 
			fxName, params.typeString);
};


/*# {$no.debug.support} >>x #*/
Clazz.tracingCalling = false;
/*# x<< #*/

/* private */
/*-# tryToSearchAndExecute -> tsae #-*/
Clazz.tryToSearchAndExecute = function (fxName, objThis, clazzFun, params, funParams/*, 
		isSuper, clazzThis*/, fx) {
		//if (fxName != "construct")System.out.println("tsae " + fxName + " " + funParams);
	var methods = new Array ();
	//var xfparams = null;
	var generic = true;
	for (var fn in clazzFun) {
		//if (fn.indexOf ('\\') == 0) {
		if (fn.charCodeAt (0) == 92) { // 92 == '\\'.charCodeAt (0)
			var ps = fn.substring (1).split ("\\");
			if (ps.length == params.length) {
				methods[methods.length] = ps;
			}
			generic = false;
			continue;
		}
		/*
		 * When there are only one method in the class, use the funParams
		 * to identify the parameter type.
		 *
		 * AbstractCollection.remove (Object)
		 * AbstractList.remove (int)
		 * ArrayList.remove (int)
		 *
		 * Then calling #remove (Object) method on ArrayList instance will 
		 * need to search up to the AbstractCollection.remove (Object),
		 * which contains only one method.
		 */
		/*
		 * See Clazz#defineMethod --Mar 10, 2006, josson
		 */
		if (generic && fn == "funParams" && clazzFun.funParams != null) {
			//xfparams = clazzFun.funParams;
			fn = clazzFun.funParams;
			var ps = fn.substring (1).split ("\\");
			if (ps.length == params.length) {
				methods[0] = ps;
			}
			break;
		}
	}
	if (methods.length == 0) {
		//throw new Clazz.MethodException ();
		return new Clazz.MethodException ();
	}
	var method = Clazz.searchMethod (methods, params);
	if (method != null) {
		var f = null;
		if (generic) { /* Use the generic method */
			/*
			 * Will this case be reachable?
			 * March 4, 2006 josson
			 * 
			 * Reachable for calling #remove (Object) method on 
			 * ArrayList instance
			 * May 5, 2006 josson
			 */
			f = clazzFun; // call it directly
		} else {
			f = clazzFun["\\" + method];
		}
		//if (f != null) { // always not null
			var methodParams = null;
			if (params.hasCastedNull) {
				methodParams = new Array ();
				for (var k = 0; k < funParams.length; k++) {
					if (funParams[k] instanceof Clazz.CastedNull) {
						/*
						 * For Clazz.CastedNull instances, the type name is
						 * already used to indentified the method in Clazz#
						 * searchMethod.
						 */
						methodParams[k] = null;
					} else {
						methodParams[k] = funParams[k];
					}
				}
			} else {
				methodParams = funParams;
			}
			/*# {$no.debug.support} >>x #*/
			if (Clazz.tracingCalling) {
				var caller = arguments.callee.caller; // SAEM
				caller = caller.arguments.callee.caller; // Delegating
				caller = caller.arguments.callee.caller; 
				var xpushed = f.exName == "construct" 
						&& Clazz.getInheritedLevel (f.exClazz, Throwable) >= 0
						&& !Clazz.initializingException;
				if (xpushed) {
					Clazz.initializingException = true;
					// constructor is wrapped
					var xcaller = caller.arguments.callee.caller // Delegate
							.arguments.callee.caller; // last method
					var fun = xcaller.arguments.callee;
					var owner = fun.claxxReference;
					if (owner == null) {
						owner = fun.exClazz;
					}
					if (owner == null) {
						owner = fun.claxxOwner;
					}
					/*
					 * Keep the environment that Throwable instance is created
					 */
					Clazz.pu$hCalling (new Clazz.callingStack (xcaller, owner));
				}
				
				var noInnerWrapper = caller !== Clazz.instantialize 
						&& caller !== Clazz.superCall;
				if (noInnerWrapper) {
					var fun = caller.arguments.callee;
					var owner = fun.claxxReference;
					if (owner == null) {
						owner = fun.exClazz;
					}
					if (owner == null) {
						owner = fun.claxxOwner;
					}
					Clazz.pu$hCalling (new Clazz.callingStack (caller, owner));
				}
				fx.lastMethod = f;
				var ret = f.apply (objThis, methodParams);
				if (noInnerWrapper) {
					Clazz.p0pCalling ();
				}
				if (xpushed) {
					Clazz.p0pCalling ();
				}
				return ret;
			}
			/*# x<< #*/
			fx.lastMethod = f;
			return f.apply (objThis, methodParams);
		//}
	}
	//throw new Clazz.MethodException ();
	return new Clazz.MethodException ();
};

/*# {$no.debug.support} >>x #*/
Clazz.initializingException = false;
/*# x<< #*/

/**
 * Search the existed polymorphic methods to get the matched method with
 * the given parameter types.
 *
 * @param existedMethods Array of string which contains method parameters
 * @param paramTypes Array of string that is parameter type.
 * @return string of method parameters seperated by "\\"
 */
/* private */
/*-# 
 # searchMethod -> sM 
 #
 # roundOne -> rO
 # paramTypes -> pts
 #-*/
Clazz.searchMethod = function (roundOne, paramTypes) {
	/*
	 * Filter out all the fitted methods for the given parameters
	 */
	/*-# roundTwo -> rT #-*/
	var roundTwo = new Array ();
	for (var i = 0; i < roundOne.length; i++) {
		/*-# fittedLevel -> fL #-*/
		var fittedLevel = new Array ();
		var isFitted = true;
		for (var j = 0; j < roundOne[i].length; j++) {
			fittedLevel[j] = Clazz.getInheritedLevel (paramTypes[j], 
					roundOne[i][j]);
			if (fittedLevel[j] < 0) {
				isFitted = false;
				break;
			}
		}
		if (isFitted) {
			fittedLevel[paramTypes.length] = i; // Keep index for later use
			roundTwo[roundTwo.length] = fittedLevel;
		}
	}
	if (roundTwo.length == 0) {
		return null;
	}
	/*
	 * Find out the best method according to the inheritance.
	 */
	/*-# resultTwo -> rtT #-*/
	var resultTwo = roundTwo;
	var min = resultTwo[0];
	for (var i = 1; i < resultTwo.length; i++) {
		/*-# isVectorLesser -> vl #-*/
		var isVectorLesser = true;
		for (var j = 0; j < paramTypes.length; j++) {
			if (min[j] < resultTwo[i][j]) {
				isVectorLesser = false;;
				break;
			}
		}
		if (isVectorLesser) {
			min = resultTwo[i];
		}
	}
	var index = min[paramTypes.length]; // Get the previously stored index
	/*
	 * Return the method parameters' type string as indentifier of the
	 * choosen method.
	 */
	return roundOne[index].join ('\\');
};

/**
 * Generate delegating function for the given method name.
 *
 * @param claxxRef the specified class for the method
 * @funName method name of the specified method
 * @return the method delegate which will try to search the method
 * from the given class by the parameters
 */
/* private */
/*-# generateDelegatingMethod -> gDM #-*/

Clazz.generateDelegatingMethod = function (claxxRef, funName, fCall) {
	/*
	 * Delegating method.
	 * Each time the following expression will generate a new 
	 * function object.
	 */
	var delegating = function () {
			var r = arguments;
  		return SAEM (this, r.callee.claxxReference, r.callee.methodName, r);
	};
	delegating.methodName = funName;
	delegating.claxxReference = claxxRef;
	return delegating;
};



SAEM = Clazz.searchAndExecuteMethod;

/* private */
Clazz.expExpandParameters = function ($0, $1) {
	if ($1 == 'N') {
		return "Number";
	} else if ($1 == 'B') {
		return "Boolean"
	} else if ($1 == 'S') {
		return "String";
	} else if ($1 == 'O') {
		return "Object";
	} else if ($1 == 'A') {
		return "Array"
	}
	return "Unknown";
};

/*
 * Other developers may need to extend this formatParameters method
 * to deal complicated situation.
 */
/* protected */
Clazz.formatParameters = function (funParams) {
	if (funParams == null || funParams.length == 0) {
		return "\\void";
	}
	return funParams.replace (/~([NABSO])/g, Clazz.expExpandParameters)
		    .replace (/\s+/g, "").replace (/^|,/g, "\\")
				.replace (/\$/g, "org.eclipse.s");
	
};

/*
 * Override the existed methods which are in the same name.
 * Overriding methods is provided for the purpose that the JavaScript
 * does not need to search the whole hierarchied methods to find the
 * correct method to execute.
 * Be cautious about this method. Incorrectly using this method may
 * break the inheritance system.
 *
 * @param clazzThis host class in which the method to be defined
 * @param funName method name
 * @param funBody function object, e.g function () { ... }
 * @param funParams paramether signature, e.g ["string", "number"]
 */
/* public */
Clazz.overrideMethod = function (clazzThis, funName, funBody, funParams) {
	if (Clazz.assureInnerClass) Clazz.assureInnerClass (clazzThis, funBody);
	funBody.exName = funName;
	var fpName = Clazz.formatParameters (funParams);
	/*
	 * Replace old methods with new method. No super methods are kept.
	 */
	funBody.funParams = fpName; 
	funBody.claxxOwner = clazzThis;
  return Clazz.getSignature(clazzThis.prototype, funName, funBody, true);
};

/*
 * Define method for the class with the given method name and method
 * body and method parameter signature.
 *
 * @param clazzThis host class in which the method to be defined
 * @param funName method name
 * @param funBody function object, e.g function () { ... }
 * @param funParams paramether signature, e.g ["string", "number"]
 * @return method of the given name. The method may be funBody or a wrapper
 * of the given funBody.
 */
/* public */
Clazz.defineMethod = function (clazzThis, funName, funBody, funParams) {
  if (Clazz.assureInnerClass) Clazz.assureInnerClass (clazzThis, funBody);
	funBody.exName = funName;
	var fpName = Clazz.formatParameters (funParams);

  /*
	 * For method the first time is defined, just keep it rather than
	 * wrapping into deep hierarchies!
	 */
  
  // BH : signature based on nParams
  var proto = clazzThis.prototype;
	var f$ = Clazz.getSignature(proto, funName, funBody, false);
  if (f$ == null || (f$.claxxOwner === clazzThis && f$.funParams == fpName)) {
		// property "funParams" will be used as a mark of only-one method
		funBody.funParams = fpName; 
		funBody.claxxOwner = clazzThis;
		funBody.exClazz = clazzThis; // make it traceable
    delete $fz;           // BH -- delete global variables when no longer needed
		return Clazz.getSignature(proto, funName, funBody, true);
	}
	var oldFun = null;
	var oldStacks = new Array ();
		if (f$.stacks == null) {
			/* method is not defined by Clazz.defineMethod () */
			oldFun = f$;
			if (f$.claxxOwner != null) {
				oldStacks[0] = oldFun.claxxOwner;
			}
		} else {
			oldStacks = f$.stacks;
		}
		/*
	 * Method that is already defined in super class will be overridden
	 * with a new proxy method with class hierarchy stored in a stack.
	 * That is to say, the super methods are lost in this class' proxy
	 * method. 
	 * When method are being called, methods defined in the new proxy 
	 * method will be searched through first. And if no method fitted,
	 * it will then try to search method in the super class stacks.
	 */
	/* method has not been defined yet */
	/* method is not defined by Clazz.defineMethod () */
	/* method is defined in super class */
	if (f$.stacks == null 
			|| f$.claxxReference !== clazzThis) {
      //if (funName == "hashCode") {
        //alert(f$.claxxReference + " " + clazzThis)
      //}
		/*
		 * Generate a new delegating method for the class
		 */
		f$ = Clazz.getSignature(proto, funName, Clazz.generateDelegatingMethod (clazzThis, funName, f$), true);				
		//if (funName != "construct" && Clazz.debuggingBH)
	    // System.out.println("delegating " + clazzThis.__CLASS_NAME__ + " " + funName + " " + funParams);
		/*
		 * Keep the class inheritance stacks
		 */
		var arr = new Array ();
		for (var i = 0; i < oldStacks.length; i++) {
			arr[i] = oldStacks[i];
		}
		f$.stacks = arr;
	}
	var ss = f$.stacks;

	if (ss.length == 0) {
		ss[0] = clazzThis;
	} else {
		var existed = false;
		for (var i = ss.length - 1; i >= 0; i--) {
			if (ss[i] === clazzThis) {
				existed = true;
				break;
			}
		}
		if (!existed) {
			ss[ss.length] = clazzThis;
		}
	}

	if (oldFun != null) {
		if (oldFun.claxxOwner === clazzThis) {
			f$[oldFun.funParams] = oldFun;
			oldFun.claxxOwner = null;
			// property "funParams" will be used as a mark of only-one method
			oldFun.funParams = null; // null ? safe ? // safe for " != null"
		} else if (oldFun.claxxOwner == null) {
			/*
			 * The function is not defined Clazz.defineMethod ().
			 * Try to fixup the method ...
			 * As a matter of lost method information, I just suppose
			 * the method to be fixed is with void parameter!
			 */
			f$["\\unknown"] = oldFun;
		}
	}
	funBody.exClazz = clazzThis; // make it traceable
	f$[fpName] = funBody;
  delete $fz;           // BH -- delete global variables when no longer needed
	return f$;
};                                                

/**
 * Make constructor for the class with the given function body and parameters
 * signature.
 * 
 * @param clazzThis host class
 * @param funBody constructor body
 * @param funParams constructor parameters signature
 */
/* public */
Clazz.makeConstructor = function (clazzThis, funBody, funParams) {
	Clazz.defineMethod (clazzThis, "construct", funBody, funParams);
	if (clazzThis.con$truct != null) {
		clazzThis.con$truct.index = clazzThis.con$truct.stacks.length;
	}
	//clazzThis.con$truct = clazzThis.prototype.con$truct = null;
};

/**
 * Override constructor for the class with the given function body and
 * parameters signature.
 * 
 * @param clazzThis host class
 * @param funBody constructor body
 * @param funParams constructor parameters signature
 */
/* public */
Clazz.overrideConstructor = function (clazzThis, funBody, funParams) {
// $_k  @j2sOverrideConstructor
	Clazz.overrideMethod (clazzThis, "construct", funBody, funParams);
	if (clazzThis.con$truct != null) {
		clazzThis.con$truct.index = clazzThis.con$truct.stacks.length;
	}
	//clazzThis.con$truct = clazzThis.prototype.con$truct = null;
};

/*
 * all root packages. e.g. java.*, org.*, com.*
 */
/* protected */
Clazz.allPackage = {};

/**
 * Will be used to keep value of whether the class is defined or not.
 */
/* protected */
Clazz.allClasses = {};

Clazz.lastPackageName = null;
Clazz.lastPackage = null;

/* protected */
Clazz.unloadedClasses = new Array ();

/* public */
Clazz.isClassUnloaded = function (clzz) {
	var thisClassName = Clazz.getClassName (clzz, true);
	return Clazz.unloadedClasses[thisClassName] != null;
};

/* public */
Clazz.declarePackage = function (pkgName) {
	if (Clazz.lastPackageName == pkgName) {
		return Clazz.lastPackage;
	}
	if (pkgName != null && pkgName.length != 0) {
		var pkgFrags = pkgName.split (/\./);
		var pkg = Clazz.allPackage;
		for (var i = 0; i < pkgFrags.length; i++) {
			if (pkg[pkgFrags[i]] == null) {
				pkg[pkgFrags[i]] = { 
					__PKG_NAME__ : ((pkg.__PKG_NAME__ != null) ? 
						pkg.__PKG_NAME__ + "." + pkgFrags[i] : pkgFrags[i])
				}; 
				// pkg[pkgFrags[i]] = {};
				if (i == 0) {
					// eval ...
					window[pkgFrags[i]] = pkg[pkgFrags[i]];
				}
			}
			pkg = pkg[pkgFrags[i]]
		}
		Clazz.lastPackageName = pkgName;
		Clazz.lastPackage = pkg;
		return pkg;
	}
};

/* protected */
/*x-# evalType -> eT  #-x*/
Clazz.evalType = function (typeStr, isQualified) {
	var idx = typeStr.lastIndexOf (".");
	if (idx != -1) {
		var pkgName = typeStr.substring (0, idx);
		var pkg = Clazz.declarePackage (pkgName);
		var clazzName = typeStr.substring (idx + 1);
		return pkg[clazzName];
	} else if (isQualified) {
		return window[typeStr];
	} else if (typeStr == "number") {
		return Number;
	} else if (typeStr == "object") {
		return JavaObject;
	} else if (typeStr == "string") {
		return String;
	} else if (typeStr == "boolean") {
		return Boolean;
	} else if (typeStr == "function") {
		return Function;
	} else if (typeStr == "void" || typeStr == "undefined"
			|| typeStr == "unknown") {
		return typeStr;
	} else if (typeStr == "NullObject") {
		return NullObject;
	} else {
		return window[typeStr];
	}
};

/**
 * Define a class or interface.
 *
 * @param qClazzName String presents the qualified name of the class
 * @param clazzFun Function of the body
 * @param clazzParent Clazz to inherit from, may be null
 * @param interfacez Clazz may implement one or many interfaces
 *   interfacez can be Clazz object or Array of Clazz objects.
 * @return Ruturn the modified Clazz object
 */
/* public */
Clazz.defineType = function (qClazzName, clazzFun, clazzParent, interfacez) {
	var cf = Clazz.unloadedClasses[qClazzName];
	if (cf != null) {
		clazzFun = cf;
	}
	var idx = qClazzName.lastIndexOf (".");
	if (idx != -1) {
		var pkgName = qClazzName.substring (0, idx);
		var pkg = Clazz.declarePackage (pkgName);
		var clazzName = qClazzName.substring (idx + 1);
		if (pkg[clazzName] != null) {
			// already defined! Should throw exception!
			return pkg[clazzName];
		}
		pkg[clazzName] = clazzFun;
	} else {
		if (window[qClazzName] != null) {
			// already defined! Should throw exception!
			return window[qClazzName];
		}
		window[qClazzName] = clazzFun;
	}
	Clazz.decorateAsType (clazzFun, qClazzName, clazzParent, interfacez);
	/*# {$no.javascript.support} >>x #*/
	var iFun = Clazz.innerFunctions;
	clazzFun.defineMethod = iFun.defineMethod;
	clazzFun.defineStaticMethod = iFun.defineStaticMethod;
	clazzFun.makeConstructor = iFun.makeConstructor;
	/*# x<< #*/
	return clazzFun;
};

Clazz.isSafari = (navigator.userAgent.indexOf ("Safari") != -1);
Clazz.isSafari4Plus = false;
if (Clazz.isSafari) {
	var ua = navigator.userAgent;
	var verIdx = ua.indexOf ("Version/");
	if (verIdx  != -1) {
		var verStr = ua.substring (verIdx + 8);
		var verNumber = parseFloat (verStr);
		Clazz.isSafari4Plus = verNumber >= 4.0;
	}
}

/* protected */
Clazz.instantialize = function (objThis, args) {
	if (args != null && args.length == 1 && args[0] != null 
			&& args[0] instanceof Clazz.args4InheritClass) {
		return ;
	}
	/*
	if (objThis.con$truct != null) {
		objThis.con$truct.apply (objThis, args);
	}
	if (objThis.construct != null) {
		objThis.construct.apply (objThis, args);
	}
	*/
	if (objThis instanceof Number) {
		objThis.valueOf = function () {
			return this;
		};
	}
	if (Clazz.isSafari4Plus) { // Fix bug of Safari 4.0+'s over-optimization
		var argsClone = new Array ();
		for (var k = 0; k < args.length; k++) {
			argsClone[k] = args[k];
		}
		args = argsClone;
	}
	var c = objThis.construct;
	if (c != null) {
		if (objThis.con$truct == null) { // no need to init fields
			c.apply (objThis, args);
		} else if (objThis.getClass ().superClazz == null) { // the base class
			objThis.con$truct.apply (objThis, []);
			c.apply (objThis, args);
		} else if ((c.claxxOwner != null 
				&& c.claxxOwner === objThis.getClass ())
				|| (c.stacks != null 
				&& c.stacks[c.stacks.length - 1] == objThis.getClass ())) {
			/*
			 * This #construct is defined by this class itself.
			 * #construct will call Clazz.superConstructor, which will
			 * call #con$truct back
			 */
			c.apply (objThis, args);
		} else { // constructor is a super constructor
			if (c.claxxOwner != null && c.claxxOwner.superClazz == null 
						&& c.claxxOwner.con$truct != null) {
				c.claxxOwner.con$truct.apply (objThis, []);
			} else if (c.stacks != null && c.stacks.length == 1
					&& c.stacks[0].superClazz == null) {
				c.stacks[0].con$truct.apply (objThis, []);
			}
			c.apply (objThis, args);
			objThis.con$truct.apply (objThis, []);
		}
	} else if (objThis.con$truct != null) {
		objThis.con$truct.apply (objThis, []);
	}
};

/**
 * Once there are other methods registered to the Function.prototype, 
 * those method names should be add to the following Array.
 */
/*
 * static final member of interface may be a class, which may
 * be function.
 */
/* protected */
/*-# innerFunctionNames -> iFN #-*/
Clazz.innerFunctionNames = [
	"equals", "hashCode", /*"toString",*/ "getName", "getClassLoader", "getResourceAsStream" /*# {$no.javascript.support} >>x #*/, "defineMethod", "defineStaticMethod",
	"makeConstructor" /*# x<< #*/
];

/*
 * Static methods
 */
/*x-# innerFunctions -> inF #-x*/
Clazz.innerFunctions = {
	/*
	 * Similar to Object#equals
	 */
	equals : function (aFun) {
		return this === aFun;
	},

	hashCode : function () {
		return this.getName ().hashCode ();
	},

	toString : function () {
		return "class " + this.getName ();
	},

	/*
	 * Similar to Class#getName
	 */
	getName : function () {
		return Clazz.getClassName (this, true);
	},
	getClassLoader : function () {
		var clazzName = this.__CLASS_NAME__;
		var baseFolder = ClazzLoader.getClasspathFor (clazzName);
		var x = baseFolder.lastIndexOf (clazzName.replace (/\./g, "/"));
		if (x != -1) {
			baseFolder = baseFolder.substring (0, x);
		} else {
			baseFolder = ClazzLoader.getClasspathFor (clazzName, true);
		}
		var loader = ClassLoader.requireLoaderByBase (baseFolder);
		loader.getResourceAsStream = Clazz.innerFunctions.getResourceAsStream;
		return loader;
	},

	getResourceAsStream : function (name) {
		var is = null;
		if (name == null) {
			return is;
		}
		if (java.io.InputStream != null) {
			is = new java.io.InputStream ();
		} else {
			is = new JavaObject ();
			is.__CLASS_NAME__ = "java.io.InputStream";
			is.close = NullObject; // function () {};
		}
		is.read = function () { return 0; };
		name = name.replace (/\\/g, '/');
		/*-# baseFolder -> bFr #-*/
		var baseFolder = null;
		var clazzName = this.__CLASS_NAME__;
		if (arguments.length == 2 && name.indexOf ('/') != 0) { // additional argument
			name = "/" + name;
		}
		if (name.indexOf ('/') == 0) {
			//is.url = name.substring (1);
			if (arguments.length == 2) { // additional argument
				baseFolder = arguments[1];
				if (baseFolder == null) {
					baseFolder = ClazzLoader.binaryFolders[0];
				}
			} else if (window["ClazzLoader"] != null) {
				baseFolder = ClazzLoader.getClasspathFor (clazzName, true);
			}
			if (baseFolder == null || baseFolder.length == 0) {
				is.url = name.substring (1);
			} else {
				baseFolder = baseFolder.replace (/\\/g, '/');
				var length = baseFolder.length;
				var lastChar = baseFolder.charAt (length - 1);
				if (lastChar != '/') {
					baseFolder += "/";
				}
				is.url = baseFolder + name.substring (1);
			}
		} else {
			if (this.base != null) {
				baseFolder = this.base;
			} else if (window["ClazzLoader"] != null) {
				baseFolder = ClazzLoader.getClasspathFor (clazzName);
				var x = baseFolder.lastIndexOf (clazzName.replace (/\./g, "/"));
				if (x != -1) {
					baseFolder = baseFolder.substring (0, x);
				} else {
					//baseFolder = null;
					var y = -1;
					if (baseFolder.indexOf (".z.js") == baseFolder.length - 5
							&& (y = baseFolder.lastIndexOf ("/")) != -1) {
						baseFolder = baseFolder.substring (0, y + 1);
						var pkgs = clazzName.split (/\./);
						for (var k = 1; k < pkgs.length; k++) {
							var pkgURL = "/";
							for (var j = 0; j < k; j++) {
								pkgURL += pkgs[j] + "/";
							}
							if (pkgURL.length > baseFolder.length) {
								break;
							}
							if (baseFolder.indexOf (pkgURL) == baseFolder.length - pkgURL.length) {
								baseFolder = baseFolder.substring (0, baseFolder.length - pkgURL.length + 1);
								break;
							}
						}
					} else {
						baseFolder = ClazzLoader.getClasspathFor (clazzName, true);
					}
				}
			} else {
				var bins = Clazz.binaryFolders;
				if (bins != null && bins.length != 0) {
					baseFolder = bins[0];
				}
			}
			if (baseFolder == null || baseFolder.length == 0) {
				baseFolder = "j2s/";
			}
			baseFolder = baseFolder.replace (/\\/g, '/');
			var length = baseFolder.length;
			var lastChar = baseFolder.charAt (length - 1);
			if (lastChar != '/') {
				baseFolder += "/";
			}
			/*
			 * FIXME: bug here for "/"
			 */
			//if (baseFolder.indexOf ('/') == 0) {
			//	baseFolder = baseFolder.substring (1);
			//}
			if (this.base != null) {
				is.url = baseFolder + name;
			} else {
				var idx = clazzName.lastIndexOf ('.');
				if (idx == -1 || this.base != null) {
					is.url = baseFolder + name;
				} else {
					is.url = baseFolder + clazzName.substring (0, idx)
							.replace (/\./g, '/') +  "/" + name;
				}
			}
		}
		return is;
	}/*# {$no.javascript.support} >>x #*/,
	
	/*
	 * For JavaScript programmers
	 */
	defineMethod : function (methodName, funBody, paramTypes) {
		Clazz.defineMethod (this, methodName, funBody, paramTypes);
	},

	/*
	 * For JavaScript programmers
	 */
	defineStaticMethod : function (methodName, funBody, paramTypes) {
		Clazz.defineMethod (this, methodName, funBody, paramTypes);
		this[methodName] = this.prototype[methodName];
	},
	
	/*
	 * For JavaScript programmers
	 */
	makeConstructor : function (funBody, paramTypes) {
		Clazz.makeConstructor (this, funBody, paramTypes);
	}
	/*# x<< #*/
};

/* private */
/*-# decorateFunction -> dF #-*/
Clazz.decorateFunction = function (clazzFun, prefix, name) {
	if (window["ClazzLoader"] != null) {
		//alert ("decorate " + name);
		ClazzLoader.checkInteractive ();
	}
	var qName = null;
	if (prefix == null) {
		// e.g. Clazz.declareInterface (null, "ICorePlugin", 
		//		org.eclipse.ui.IPlugin);
		qName = name;
		window[name] = clazzFun;
	} else if (prefix.__PKG_NAME__ != null) {
		// e.g. Clazz.declareInterface (org.eclipse.ui, "ICorePlugin", 
		//		org.eclipse.ui.IPlugin);
		qName = prefix.__PKG_NAME__ + "." + name;
		prefix[name] = clazzFun;
		if (prefix === java.lang) {
			window[name] = clazzFun;
		}
	} else {
		// e.g. Clazz.declareInterface (org.eclipse.ui.Plugin, "ICorePlugin", 
		//		org.eclipse.ui.IPlugin);
		qName = prefix.__CLASS_NAME__ + "." + name;
		prefix[name] = clazzFun;
		//alert("j2slib.z Clazz.decorateFunction qname=" + qName)
	}
  Clazz.extendJO(clazzFun, qName);
	var inF = Clazz.innerFunctionNames;
	for (var i = 0; i < inF.length; i++) {
		clazzFun[inF[i]] = Clazz.innerFunctions[inF[i]];
	}

	if (window["ClazzLoader"] != null) {
		/*-# findClass -> fC #-*/
		var node = ClazzLoader.findClass (qName);
		/*-#
		 # ClazzNode.STATUS_KNOWN -> 1
		 #-*/
		if (node != null && node.status == ClazzNode.STATUS_KNOWN) {
			/*-# 
			 # updateNode -> uN 
			 #-*/
			window.setTimeout((function(nnn) {
				return function() {
					ClazzLoader.updateNode (nnn);
				};
			})(node), 1);
			/*
			 * #updateNode should be delayed! Or the class itself won't
			 * be initialized completely before marking itself as loaded.
			 */
			// ClazzLoader.updateNode (node);
		}
	}
};
Clazz.currentPath= "";
/* proected */
Clazz.declareInterface = function (prefix, name, interfacez) {
	var clazzFun = function () {};
	Clazz.decorateFunction (clazzFun, prefix, name, "Clazz.declareInterface");
	if (interfacez != null) {
		Clazz.implementOf (clazzFun, interfacez);
	}
	return clazzFun;
};

/* protected */
/*-# 
 # parentClazzInstance -> pi
 # clazzParent -> pc
 #-*/
Clazz.decorateAsClass = function (clazzFun, prefix, name, clazzParent, 
		interfacez, parentClazzInstance, fromWhere) {
	var prefixName = null;
	if (prefix != null) {
		prefixName = prefix.__PKG_NAME__;
		if (prefixName == null) {
			prefixName = prefix.__CLASS_NAME__;
		}
	}
	var qName = (prefixName == null ? "" : prefixName + ".") + name;
	var cf = Clazz.unloadedClasses[qName];
	if (cf != null) {
		clazzFun = cf;
	}
	var qName = null;
	Clazz.decorateFunction (clazzFun, prefix, name, fromWhere + "...Clazz.decorateAsClass");
	if (parentClazzInstance != null) {
		Clazz.inheritClass (clazzFun, clazzParent, parentClazzInstance);
	} else if (clazzParent != null) {
		Clazz.inheritClass (clazzFun, clazzParent);
	}
	if (interfacez != null) {
		Clazz.implementOf (clazzFun, interfacez);
	}
	return clazzFun;
};

/* public */
Clazz.declareType = function (prefix, name, clazzParent, interfacez, 
		parentClazzInstance) {
	var f = function () {
		Clazz.instantialize (this, arguments);
	};
	return Clazz.decorateAsClass (f, prefix, name, clazzParent, interfacez, 
			parentClazzInstance, "Clazz.declareType");
};

/* public */
Clazz.declareAnonymous = function (prefix, name, clazzParent, interfacez, 
		parentClazzInstance) {
	var f = function () {
		Clazz.prepareCallback (this, arguments);
		Clazz.instantialize (this, arguments);
	};
	return Clazz.decorateAsClass (f, prefix, name, clazzParent, interfacez, 
			parentClazzInstance, "Clazz.declareAnonymous");
};

/* protected */
Clazz.decorateAsType = function (clazzFun, qClazzName, clazzParent, 
		interfacez, parentClazzInstance, inheritClazzFuns) {
  Clazz.extendJO(clazzFun, qClazzName);
	clazzFun.equals = Clazz.innerFunctions.equals;
	clazzFun.getName = Clazz.innerFunctions.getName;
	if (inheritClazzFuns) {
		for (var i = 0; i < Clazz.innerFunctionNames.length; i++) {
			var methodName = Clazz.innerFunctionNames[i];
			clazzFun[methodName] = Clazz.innerFunctions[methodName];
		}
	}
	if (parentClazzInstance != null) {
		Clazz.inheritClass (clazzFun, clazzParent, parentClazzInstance);
	} else if (clazzParent != null) {
		Clazz.inheritClass (clazzFun, clazzParent);
	}
	if (interfacez != null) {
		Clazz.implementOf (clazzFun, interfacez);
	}
	return clazzFun;
};

/* sgurin: native exception detection mechanism. Only NullPointerException detected and wrapped to java excepions */
/** private utility method for creating a general regexp that can be used later  
 * for detecting a certain kind of native exceptions. use with error messages like "blabla IDENTIFIER blabla"
 * @param msg String - the error message
 * @param spliterName String, must be contained once in msg
 * spliterRegex String, a string with the regexp literal for identifying the spitter in exception further error messages.
 */
Clazz._ex_reg=function (msg, spliterName, spliterRegex) {
	if(!spliterRegex) 
		spliterRegex="[^\\s]+";	
	var idx = msg.indexOf (spliterName), 
		str = msg.substring (0, idx) + spliterRegex + msg.substring(idx + spliterName.length), 
		regexp = new RegExp("^"+str+"$");
	return regexp;
};
// reproduce NullPointerException for knowing how to detect them, and create detector function Clazz._isNPEExceptionPredicate
var $$o$$ = null;
try {
	$$o$$.hello ();
} catch (e) {
	if(/Opera[\/\s](\d+\.\d+)/.test(navigator.userAgent)) {// opera throws an exception with fixed messages like "Statement on line 23: Cannot convert undefined or null to Object Backtrace: Line....long text... " 
		var idx1 = e.message.indexOf(":"), idx2 = e.message.indexOf(":", idx1+2);
		Clazz._NPEMsgFragment = e.message.substr(idx1+1, idx2-idx1-20);
		Clazz._isNPEExceptionPredicate = function(e) {
			return e.message.indexOf(Clazz._NPEMsgFragment)!=-1;
		};
	}	
	else if(navigator.userAgent.toLowerCase().indexOf("webkit")!=-1) { //webkit, google chrome prints the property name accessed. 
		Clazz._exceptionNPERegExp = Clazz._ex_reg(e.message, "hello");
		Clazz._isNPEExceptionPredicate = function(e) {
			return Clazz._exceptionNPERegExp.test(e.message);
		};
	}
	else {// ie, firefox and others print the name of the object accessed: 
		Clazz._exceptionNPERegExp = Clazz._ex_reg(e.message, "$$o$$");
		Clazz._isNPEExceptionPredicate = function(e) {
			return Clazz._exceptionNPERegExp.test(e.message);
		};
	}		
};
/**sgurin
 * Implements Java's keyword "instanceof" in JavaScript's way **for exception objects**.
 * 
 * calls Clazz.instanceOf if e is a Java exception. If not, try to detect known native 
 * exceptions, like native NullPointerExceptions and wrap it into a Java exception and 
 * call Clazz.instanceOf again. if the native exception can't be wrapped, false is returned.
 * 
 * @param obj the object to be tested
 * @param clazz the class to be checked
 * @return whether the object is an instance of the class
 * @author: sgurin
 */
Clazz.exceptionOf=function(e, clazz) {
	if(e.__CLASS_NAME__) {
		return Clazz.instanceOf(e, clazz);
	}
  if(clazz == Error) {
    if (("" + e).indexOf("Error") >= 0) {
      var c = arguments.callee
      // attempt at a sort of stack trace
      for (var i = 0; i < 50; i++) {
        c = c.caller
        if (!c)break;
        System.out.println(i + " " + (c.exName ? (c.claxxOwner ? c.claxxOwner.__CLASS_NAME__ + "."  : "") + c.exName 
        : (c.toString ? c.toString().substring(0, c.toString().indexOf("{")) : "<native method>")))
      }
    }
    return (("" + e).indexOf("Error") >= 0);
    // everything here is a Java Exception, not a Java Error
  }
  return (clazz == Exception || clazz == Throwable
    || clazz == NullPointerException && Clazz._isNPEExceptionPredicate(e));
};

/* sgurin: preserve Number.prototype.toString */
Number.prototype._numberToString=Number.prototype.toString;


Clazz.declarePackage ("java.io");
//Clazz.declarePackage ("java.lang");
Clazz.declarePackage ("java.lang.annotation"); // java.lang
Clazz.declarePackage ("java.lang.instrument"); // java.lang
Clazz.declarePackage ("java.lang.management"); // java.lang
Clazz.declarePackage ("java.lang.reflect"); // java.lang
Clazz.declarePackage ("java.lang.ref");  // java.lang.ref
java.lang.ref.reflect = java.lang.reflect;
Clazz.declarePackage ("java.util");

/*
 * Consider these interfaces are basic!
 */
Clazz.declareInterface (java.io,"Closeable");
Clazz.declareInterface (java.io,"DataInput");
Clazz.declareInterface (java.io,"DataOutput");
Clazz.declareInterface (java.io,"Externalizable");
Clazz.declareInterface (java.io,"Flushable");
Clazz.declareInterface (java.io,"Serializable");
Clazz.declareInterface (java.lang,"Iterable");
Clazz.declareInterface (java.lang,"CharSequence");
Clazz.declareInterface (java.lang,"Cloneable");
Clazz.declareInterface (java.lang,"Appendable");
Clazz.declareInterface (java.lang,"Comparable");
Clazz.declareInterface (java.lang,"Runnable");
Clazz.declareInterface (java.util,"Comparator");

java.lang.ClassLoader = {
	__CLASS_NAME__ : "ClassLoader"
};

/******************************************************************************
 * Copyright (c) 2007 java2script.org and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     Zhou Renjian - initial API and implementation
 *****************************************************************************/
/*******
 * @author zhou renjian
 * @create March 10, 2006
 *******/

if (window["Clazz"] != null && window["Clazz"].unloadClass == null) {
/**
 * Once ClassExt.js is part of Class.js.
 * In order to make the Class.js as small as possible, part of its content
 * is moved into this ClassExt.js.
 *
 * See also http://j2s.sourceforge.net/j2sclazz/
 */
 
/**
 * Clazz.MethodNotFoundException is used to notify the developer about calling
 * methods with incorrect parameters.
 */
/* protected */
// Override the Clazz.MethodNotFoundException in Class.js to give details
Clazz.MethodNotFoundException = function (obj, clazz, method, params) {
	var paramStr = "";
	if (params != null) {
		paramStr = params.substring (1).replace (/\\/g, ",");
	}
	var leadingStr = "";
	if (method != null && method != "construct") {
		leadingStr = "Method";
	} else {
		leadingStr = "Constructor";
	}
	this.message = leadingStr + " " + Clazz.getClassName (clazz, true) + "." 
					+ method + "(" + paramStr + ") is not found!";
	this.toString = function () {
		return "MethodNotFoundException:" + this.message;
	}
};

/**
 * Prepare callback for instance of anonymous Class.
 * For example for the callback:
 *     this.callbacks.MyEditor.sayHello();
 *
 * @param objThis the host object for callback
 * @param args arguments object. args[0] will be classThisObj -- the "this"
 * object to be hooked
 * 
 * Attention: parameters should not be null!
 */
/* protected */
Clazz.prepareCallback = function (objThis, args) {
	var classThisObj = args[0];
	var cbName = "b$"; // "callbacks";
	if (objThis != null && classThisObj != null && classThisObj !== window) {
		var obs = new Array ();
		if (objThis[cbName] == null) {
			objThis[cbName] = obs;
		} else { // must make a copy!
			for (var s in objThis[cbName]) {
				if (s != "length") {
					obs[s] = objThis[cbName][s];
				}
			}
			objThis[cbName] = obs;
		}
		var className = Clazz.getClassName (classThisObj, true);
		//if (obs[className] == null) { /* == null make no sense! */
			//obs[className] = classThisObj;
			/*
			 * TODO: the following line is SWT-specific! Try to move it out!
			 */
			obs[className.replace (/org\.eclipse\.swt\./, "$wt.")] = classThisObj;
			var clazz = Clazz.getClass (classThisObj);
			while (clazz.superClazz != null) {
				clazz = clazz.superClazz;
				//obs[Clazz.getClassName (clazz)] = classThisObj;
				/*
				 * TODO: the following line is SWT-specific! Try to move it out!
				 */
				obs[Clazz.getClassName (clazz, true)
						.replace (/org\.eclipse\.swt\./, "$wt.")] = classThisObj;
			}
		//}
		var cbs = classThisObj[cbName];
		if (cbs != null && cbs instanceof Array) {
			for (var s in cbs) {
				if (s != "length") {
					obs[s] = cbs[s];
				}
			}
		}
	}
	// Shift the arguments
	for (var i = 0; i < args.length - 1; i++) {
		args[i] = args[i + 1];
	}
	args.length--;
	// arguments will be returned!
};

/**
 * Construct instance of the given inner class.
 *
 * @param classInner given inner class, alway with name like "*$*"
 * @param objThis this instance which can be used to call back.
 * @param finalVars final variables which the inner class may use
 * @return the constructed object
 *
 * @see Clazz#cloneFinals
 */
/* public */
Clazz.innerTypeInstance = function (clazzInner, objThis, finalVars) {
	if (clazzInner == null) {
		clazzInner = arguments.callee.caller;
	}
	var obj = null;
	if (finalVars == null && objThis.$finals == null) {
		/*if (arguments.length == 2) {
			obj = new clazzInner (objThis);
		} else */if (arguments.length == 3) {
			obj = new clazzInner (objThis);
		} else if (arguments.length == 4) {
			if (objThis.__CLASS_NAME__ == clazzInner.__CLASS_NAME__
					&& arguments[3] === Clazz.inheritArgs) {
				obj = objThis;
			} else {
				obj = new clazzInner (objThis, arguments[3]);
			}
		} else if (arguments.length == 5) {
			obj = new clazzInner (objThis, arguments[3], arguments[4]);
		} else if (arguments.length == 6) {
			obj = new clazzInner (objThis, arguments[3], arguments[4], 
					arguments[5]);
		} else if (arguments.length == 7) {
			obj = new clazzInner (objThis, arguments[3], arguments[4], 
					arguments[5], arguments[6]);
		} else if (arguments.length == 8) {
			obj = new clazzInner (objThis, arguments[3], arguments[4], 
					arguments[5], arguments[6], arguments[7]);
		} else if (arguments.length == 9) {
			obj = new clazzInner (objThis, arguments[3], arguments[4], 
					arguments[5], arguments[6], arguments[7], arguments[8]);
		} else if (arguments.length == 10) {
			obj = new clazzInner (objThis, arguments[3], arguments[4], 
					arguments[5], arguments[6], arguments[7], arguments[8],
					arguments[9]);
		} else {
			/*
			 * Should construct instance manually.
			 */
			obj = new clazzInner (objThis, Clazz.inheritArgs);
			//if (obj.construct == null) {
			//	throw new String ("No support anonymous class constructor with " 
			//			+ "more than 7 parameters.");
			//}
			var args = new Array ();
			for (var i = 3; i < arguments.length; i++) {
				args[i - 3] = arguments[i];
			}
			//obj.construct.apply (obj, args);
			Clazz.instantialize (obj, args);
		}
	} else {
		obj = new clazzInner (objThis, Clazz.inheritArgs);
		// f$ is short for the once choosen "$finals"
		if (finalVars != null && objThis.f$ == null) {
			obj.f$ = finalVars;
		} else if (finalVars == null && objThis.f$ != null) {
			obj.f$ = objThis.f$;
		} else if (finalVars != null && objThis.f$ != null) {
			var o = new Object ();
			for (var attr in objThis.f$) {
				o[attr] = objThis.f$[attr];
			}
			for (var attr in finalVars) {
				o[attr] = finalVars[attr];
			}
			obj.f$ = o;
		}
		
		var args = new Array ();
		for (var i = 3; i < arguments.length; i++) {
			args[i - 3] = arguments[i];
		}
		Clazz.instantialize (obj, args);
	}
	
	/*
	if (finalVars != null && objThis.$finals == null) {
		obj.$finals = finalVars;
	} else if (finalVars == null && objThis.$finals != null) {
		obj.$finals = objThis.$finals;
	} else if (finalVars != null && objThis.$finals != null) {
		var o = {};
		for (var attr in objThis.$finals) {
			o[attr] = objThis.$finals[attr];
		}
		for (var attr in finalVars) {
			o[attr] = finalVars[attr];
		}
		obj.$finals = o;
	}
	*/
	//Clazz.prepareCallback (obj, objThis);
	return obj;
};

/**
 * Clone variables whose modifier is "final".
 * Usage: var o = Clazz.cloneFinals ("name", name, "age", age);
 *
 * @return Object with all final variables
 */
/* protected */
Clazz.cloneFinals = function () {
	var o = {};
	var length = arguments.length / 2;
	for (var i = 0; i < length; i++) {
		o[arguments[i + i]] = arguments[i + i + 1];
	}
	return o;
};

/* public */
Clazz.isClassDefined = Clazz.isDefinedClass = function (clazzName) {
	if (clazzName != null && clazzName.length != 0) {
		if (Clazz.allClasses[clazzName]) {
			return true;
		}
		var pkgFrags = clazzName.split (/\./);
		var pkg = null;
		for (var i = 0; i < pkgFrags.length; i++) {
			if (pkg == null) {
				if (Clazz.allPackage[pkgFrags[0]] == null) {
					//error (clazzName + " / " + false);
					return false;
				}
				pkg = Clazz.allPackage[pkgFrags[0]];
			} else {
				if (pkg[pkgFrags[i]] == null) {
					//error (clazzName + " / " + false);
					return false;
				}
				pkg = pkg[pkgFrags[i]]
			}
		}
		//error (clazzName + " / " + (pkg != null));
		//return pkg != null;
		if (pkg != null) {
			Clazz.allClasses[clazzName] = true;
			return true;
		} else {
			return false;
		}
	} else {
		/* consider null or empty name as non-defined class */
		return false;
	}
};
/**
 * Define the enum constant.
 * @param classEnum enum type
 * @param enumName enum constant
 * @param enumOrdinal enum ordinal
 * @param initialParams enum constant constructor parameters
 * @return return defined enum constant
 */
/* public */
Clazz.defineEnumConstant = function (clazzEnum, enumName, enumOrdinal, initialParams, clazzEnumExt) {
	var o = null;
	if (clazzEnumExt != null) {
		o = new clazzEnumExt ();
	} else {
		o = new clazzEnum ();
	}
	Clazz.superConstructor (o, clazzEnum, [enumName, enumOrdinal]);
	if (initialParams != null && initialParams.length != 0) {
		o.construct.apply (o, initialParams);
	}
	clazzEnum[enumName] = o;
	clazzEnum.prototype[enumName] = o;
	if (clazzEnum["$ values"] == null) {  // BH added
	  clazzEnum["$ values"] = []          // BH added
	  clazzEnum.values = function() {     // BH added
	    return this["$ values"];          // BH added
	  };                                  // BH added
	}
	clazzEnum["$ values"].push(o);
	return o;
};

//////// (int) conversions //////////

Clazz.floatToInt = function (x) {
	return x < 0 ? Math.ceil(x) : Math.floor(x);
};

Clazz.floatToByte = Clazz.floatToShort = Clazz.floatToLong = Clazz.floatToInt;
Clazz.doubleToByte = Clazz.doubleToShort = Clazz.doubleToLong = Clazz.doubleToInt = Clazz.floatToInt;

Clazz.floatToChar = function (x) {
	return String.fromCharCode (x < 0 ? Math.ceil(x) : Math.floor(x));
};

Clazz.doubleToChar = Clazz.floatToChar;



//////// Array additions ////////////

if (self.Int32Array && self.Int32Array != Array) {
  Clazz.haveInt32 = true;
  if (!Int32Array.prototype.sort)
    Int32Array.prototype.sort = Array.prototype.sort
} else {
  Int32Array = function(n) {
    if (!n) n = 0;
    var b = new Array(n);
    b.toString = function(){return "[object Int32Array]"}
    for (var i = 0; i < n; i++)b[i] = 0
    return b;
  }
  Clazz.haveInt32 = false;
  Int32Array.prototype.sort = Array.prototype.sort
  Int32Array.prototype.int32Fake = function(){};
}

if (self.Float64Array && self.Float64Array != Array) {
  Clazz.haveFloat64 = true;
  if (!Float64Array.prototype.sort)
    Float64Array.prototype.sort = Array.prototype.sort
} else {
  Clazz.haveFloat64 = false;
	Float64Array = function(n) {
    if (!n) n = 0;
    var b = new Array(n);
    for (var i = 0; i < n; i++)b[i] = 0.0
    return b;
  };
  Float64Array.prototype.sort = Array.prototype.sort
  Float64Array.prototype.float64Fake = function() {}; // "present"
  Float64Array.prototype.toString = function() {return "[object Float64Array]"};
// Darn! Mozilla makes this a double, not a float. It's 64-bit.
// and Safari 5.1 doesn't have Float64Array 
}

/**
 * Make arrays.
 *
 * @return the created Array object
 */
/* public */
Clazz.newArray  = function () {
	if (arguments[0] instanceof Array) {
    // recursive, from newArray(n,m,value)
    // as newArray([m, value], newInt32Array)
		var args = arguments[0];
		var f = arguments[1];
	} else {
    var args = arguments;
    var f = Array;
  }
	if (args.length <= 1) return new Array(); // maybe never?
	var dim = args[0];
	if (typeof dim == "string") {
		dim = dim.charCodeAt (0); // char
	}
	var len = args.length - 1;
  var val = args[len];
  if (args.length == 2) {
    if (val == null)
      return new Array(dim);
    if (f === true && Clazz.haveInt32) return new Int32Array(dim);
    if (f === false && Clazz.haveFloat64) return new Float64Array(dim);
		if (f == Array && val == null) return new Array(dim);
    var arr = (f === true ? new Int32Array() : f === false ? new Float64Array() : new Array(dim));
  	for (var i = dim; --i >= 0;)
  	  arr[i] = val;
		return arr;
	}
	var xargs = new Array (len);
	for (var i = 0; i < len; i++) {
		xargs[i] = args[i + 1];
	}
	var arr = new Array (dim);
  if (val == null || val >= 0 || len > 2)
  	for (var i = 0; i < dim; i++) {
	 	// Call recursively!
  		arr[i] = Clazz.newArray (xargs, f);
  	}
	return arr;
};
                                
Clazz.newArray32 = function(args, isInt32) {
	var dim = args[0];
	if (typeof dim == "string") {
		dim = dim.charCodeAt (0); // char
	}
	var len = args.length - 1;
	var val = args[len];
	switch (args.length) {
	case 0:
	case 1:
    alert("ERROR IN newArray32 -- args length < 2");
    return new Array(0);
  case 2:
    if (val < 0)
      return new Array(dim);
  //  if (isInt32 ? !Clazz.haveInt32 : !Clazz.haveFloat64 ) {
      // no support for Int32Array in MSIE or Float64Array in Safari 5.1
      // so we must initialize ourselves
  //    var f = (isInt32 ? new Int32Array() : new Float64Array());
  //    for (var i = dim; --i >= 0;)
  //      f[i] = 0;
  //    return f;
  //  }
    try {
    	return (isInt32 ? new Int32Array(dim) : new Float64Array(dim));
  	}catch (e) {
    	alert(dim + " " + arguments.callee.caller.arguments.callee.caller + e)
    }
  }
	var xargs = new Array(len);
	for (var i = len; --i >= 0;) {
		xargs[i] = args[i + 1];
	}
	var arr = new Array (dim);
	for (var i = 0; i < dim; i++) {
		// Call newArray referencing this array type
		// only for the final iteration, and only if val === 0
		arr[i] = Clazz.newArray (xargs, isInt32);
	}
	return arr;
};


/**
 * Make arrays.
 *
 * @return the created Array object
 */
/* public */
Clazz.newInt32Array  = function () {
  return Clazz.newArray32(arguments, true);
}

/**
 * Make arrays.
 *
 * @return the created Array object
 */
/* public */
Clazz.newFloat64Array  = function () {
  return Clazz.newArray32(arguments, false);
}

Clazz.newFloatArray = Clazz.newDoubleArray = Clazz.newFloat64Array;
Clazz.newIntArray = Clazz.newLongArray = Clazz.newShortArray = Clazz.newByteArray = Clazz.newInt32Array;
Clazz.newCharArray = Clazz.newBooleanArray = Clazz.newArray;

$_AI=Clazz.newIntArray;
$_AF=Clazz.newFloatArray;
$_AD=Clazz.newDoubleArray;
$_AL=Clazz.newLongArray;
$_AS=Clazz.newShortArray;
$_AB=Clazz.newByteArray;
$_AC=Clazz.newCharArray;
$_Ab=Clazz.newBooleanArray;


Clazz.isAS = function(a) { // just checking first parameter
  return (a && typeof a == "object" && a.constructor && a.constructor.toString().indexOf(" Array") >= 0 && (typeof a[0] == "string" || typeof a[0] == "undefined"));
}

Clazz.isASS = function(a) {
  return (a && typeof a == "object" && Clazz.isAS(a[0]));
}

Clazz.isAP = function(a) {
  return (a && Clazz.getClassName(a[0]) == "J.util.Point3f");
}

Clazz.isAI = function(a) {
  return (a && typeof a == "object" && (Clazz.haveInt32 ? a.constructor && a.constructor.toString().indexOf("Int32Array") >= 0 : a.int32Fake ? true : false));
}

Clazz.isAII = function(a) { // assumes non-null a[0]
  return (a && typeof a == "object" && Clazz.isAI(a[0]));
}

Clazz.isAF = function(a) {
  return (a && typeof a == "object" && (Clazz.haveFloat64 ? a.constructor && a.constructor.toString().indexOf("Float64Array") >= 0 : a.float64Fake ? true : false));
}

Clazz.isAFF = function(a) { // assumes non-null a[0]
  return (a && typeof a == "object" && Clazz.isAF(a[0]));
}

Clazz.isAFFF = function(a) { // assumes non-null a[0]
  return (a && typeof a == "object" && Clazz.isAFF(a[0]));
}

Clazz.isAFloat = function(a) { // just checking first parameter
  return (a && typeof a == "object" && a.constructor && a.constructor.toString().indexOf(" Array") >= 0 && Clazz.instanceOf(a[0], Float));
}


/**
 * Make the RunnableCompatiability instance as a JavaScript function.
 *
 * @param jsr Instance of RunnableCompatiability
 * @return JavaScript function instance represents the method run of jsr.
 */
/* public */
Clazz.makeFunction = function (jsr) {
	return function (e) {
		if (e == null) {
			e = window.event;
		}
		if (jsr.setEvent != null) {
			jsr.setEvent (e);
		}
		jsr.run ();
		/*
		if (e != null && jsr.isReturned != null && jsr.isReturned()) {
			// Is it correct to stopPropagation here? --Feb 19, 2006
			e.cancelBubble = true;
			if (e.stopPropagation) {
				e.stopPropagation();
			}
		}
		*/
		if (jsr.returnSet == 1) {
			return jsr.returnNumber;
		} else if (jsr.returnSet == 2) {
			return jsr.returnBoolean;
		} else if (jsr.returnSet == 3) {
			return jsr.returnObject;
		}
	};
};

/* protected */
Clazz.defineStatics = function (clazz) {
	for (var i = 0; i < (arguments.length - 1) / 2; i++) {
		var name = arguments[i + i + 1];
		clazz[name] = clazz.prototype[name] = arguments[i + i + 2];
	}
};

/* protected */
Clazz.prepareFields = function (clazz, fieldsFun) {
	var stacks = new Array ();
 //System.out.println(fieldsFun)
	if (clazz.con$truct != null) {
		var ss = clazz.con$truct.stacks;
		var idx = 0;//clazz.con$truct.index;
		//System.out.println("clazz index = " + idx + " sslen=" + ss.length)
		for (var i = idx; i < ss.length; i++) {
			stacks[i] = ss[i];
		}
	}
	//System.out.println(JSON.stringify(stacks))
	Clazz.addProto(clazz.prototype, "con$truct", clazz.con$truct = function () {
		var stacks = arguments.callee.stacks;
		if (stacks != null) {
			for (var i = 0; i < stacks.length; i++) {
				stacks[i].apply (this, []);
			}
		}
	});
	stacks[stacks.length] = fieldsFun;
	clazz.con$truct.stacks = stacks;
	clazz.con$truct.index = 0;
};

/*
 * Serialize those public or protected fields in class 
 * net.sf.j2s.ajax.SimpleSerializable.
 */
/* protected */
Clazz.registerSerializableFields = function (clazz) {
	var args = arguments;
	var length = args.length;
	var newArr = new Array ();
	if (clazz.declared$Fields != null) {
		for (var i = 0; i < clazz.declared$Fields.length; i++) {
			newArr[i] = clazz.declared$Fields[i];
		}
	}
	clazz.declared$Fields = newArr;
	
	if (length > 0 && length % 2 == 1) {
		var fs = clazz.declared$Fields;
		for (var i = 1; i <= (length - 1) / 2; i++) {
			var o = { name : args[i + i - 1], type : args[i + i] };
			var existed = false;
			for (var j = 0; j < fs.length; j++) {
				if (fs[j].name == o.name) { // reloaded classes
					fs[j].type = o.type; // update type
					existed = true;
					break;
				}
			}
			if (!existed) {
				fs[fs.length] = o;
			}
		}
	}
};

/*
 * Get the caller method for those methods that are wrapped by 
 * Clazz.searchAndExecuteMethod.
 *
 * @param args caller method's arguments
 * @return caller method, null if there is not wrapped by 
 * Clazz.searchAndExecuteMethod or is called directly.
 */
/* protected */
/*-# getMixedCallerMethod -> gMCM  #-*/
Clazz.getMixedCallerMethod = function (args) {
	var o = {};
	var argc = args.callee.caller; // Clazz.tryToSearchAndExecute
	if (argc == null) return null;
	if (argc !== Clazz.tryToSearchAndExecute) { // inherited method's apply
		argc = argc.arguments.callee.caller;
		if (argc == null) return null;
	}
	if (argc !== Clazz.tryToSearchAndExecute) return null;
	argc = argc.arguments.callee.caller; // Clazz.searchAndExecuteMethod
	if (argc == null || argc !== Clazz.searchAndExecuteMethod) return null;
	o.claxxRef = argc.arguments[1];
	o.fxName = argc.arguments[2];
	o.paramTypes = Clazz.getParamsType (argc.arguments[3]);
	argc = argc.arguments.callee.caller; // Clazz.generateDelegatingMethod
	if (argc == null) return null;
	argc = argc.arguments.callee.caller; // the private method's caller
	if (argc == null) return null;
	o.caller = argc;
	return o;
};

/*
 * Check and return super private method.
 * In order make private methods be executed correctly, some extra javascript
 * must be inserted into the beggining of the method body of the non-private 
 * methods that with the same method signature as following:
 * <code>
 *			var $private = Clazz.checkPrivateMethod (arguments);
 *			if ($private != null) {
 *				return $private.apply (this, arguments);
 *			}
 * </code>
 * Be cautious about this. The above codes should be insert by Java2Script
 * compiler or with double checks to make sure things work correctly.
 *
 * @param args caller method's arguments
 * @return private method if there are private method fitted for the current 
 * calling environment
 */
/* public */
Clazz.checkPrivateMethod = function (args) {
	var m = Clazz.getMixedCallerMethod (args);
	if (m == null) return null;
	var callerFx = m.claxxRef.prototype[m.caller.exName];
	if (callerFx == null) return null; // may not be in the class hierarchies
	var ppFun = null;
	if (callerFx.claxxOwner != null) {
		ppFun = callerFx.claxxOwner.prototype[m.fxName];
	} else {
		var stacks = callerFx.stacks;
		for (var i = stacks.length - 1; i >= 0; i--) {
			var fx = stacks[i].prototype[m.caller.exName];
			if (fx === m.caller) {
				ppFun = stacks[i].prototype[m.fxName];
			} else if (fx != null) {
				for (var fn in fx) {
					if (fn.indexOf ('\\') == 0 && fx[fn] === m.caller) {
						ppFun = stacks[i].prototype[m.fxName];
						break;
					}
				}
			}
			if (ppFun != null) {
				break;
			}
		}
	}
	if (ppFun != null && ppFun.claxxOwner == null) {
		ppFun = ppFun["\\" + m.paramTypes];
	}
	if (ppFun != null && ppFun.isPrivate && ppFun !== args.callee) {
		return ppFun;
	}
	return null;
};
$fz = null; // for private method declaration
//var cla$$ = null;
c$ = null;
/*-# cla$$$tack -> cst  #-*/
Clazz.cla$$$tack = new Array ();
Clazz.pu$h = function () {
	if (c$ != null) { // if (cla$$ != null) {
		Clazz.cla$$$tack[Clazz.cla$$$tack.length] = c$; // cla$$;
	}
};
Clazz.p0p = function () {
	if (Clazz.cla$$$tack.length > 0) {
		var clazz = Clazz.cla$$$tack[Clazz.cla$$$tack.length - 1];
		Clazz.cla$$$tack.length--;
		return clazz;
	} else {
		return null;
	}
};

/*# {$no.debug.support} >>x #*/
/*
 * Option to switch on/off of stack traces.
 */
/* protect */
Clazz.tracingCalling = false;

/*
 * Use to mark that the Throwable instance is created or not.
 */
/* private */
Clazz.initializingException = false;

/* private */
Clazz.callingStack = function (caller, owner) {
	this.caller = caller;
	this.owner = owner;
};
Clazz.callingStackTraces = new Array ();
Clazz.pu$hCalling = function (stack) {
	Clazz.callingStackTraces[Clazz.callingStackTraces.length] = stack;
};
Clazz.p0pCalling = function () {
	var length = Clazz.callingStackTraces.length;
	if (length > 0) {
		var stack = Clazz.callingStackTraces[length - 1];
		Clazz.callingStackTraces.length--;
		return stack;
	} else {
		return null;
	}
};
/*# x<< #*/

/**
 * The first folder is considered as the primary folder.
 * And try to be compatiable with ClazzLoader system.
 */
/* private */
if (window["ClazzLoader"] != null && ClazzLoader.binaryFolders != null) {
	Clazz.binaryFolders = ClazzLoader.binaryFolders;
} else {
	Clazz.binaryFolders = ["j2s/", "", "j2slib/"];
}

Clazz.addBinaryFolder = function (bin) {
	if (bin != null) {
		var bins = Clazz.binaryFolders;
		for (var i = 0; i < bins.length; i++) {
			if (bins[i] == bin) {
				return ;
			}
		}
		bins[bins.length] = bin;
	}
};
Clazz.removeBinaryFolder = function (bin) {
	if (bin != null) {
		var bins = Clazz.binaryFolders;
		for (var i = 0; i < bins.length; i++) {
			if (bins[i] == bin) {
				for (var j = i; j < bins.length - 1; j++) {
					bins[j] = bins[j + 1];
				}
				bins.length--;
				return bin;
			}
		}
	}
	return null;
};
Clazz.setPrimaryFolder = function (bin) {
	if (bin != null) {
		Clazz.removeBinaryFolder (bin);
		var bins = Clazz.binaryFolders;
		for (var i = bins.length - 1; i >= 0; i--) {
			bins[i + 1] = bins[i];
		}
		bins[0] = bin;
	}
};

/**
 * This is a simple implementation for Clazz#load. It just ignore dependencies
 * of the class. This will be fine for jar *.z.js file.
 * It will be overriden by ClazzLoader#load.
 * For more details, see ClazzLoader.js
 */
/* protected */
Clazz.load = function (musts, clazz, optionals, declaration) {
	if (declaration != null) {
		declaration ();
	}
  delete c$;           // BH -- delete global variables when no longer needed?
};

/*
 * Invade the Object prototype!
 * TODO: make sure that invading Object prototype does not affect other
 * existed library, such as Dojo, YUI, Prototype, ...
 */
java.lang.Object = JavaObject;

JavaObject.getName = Clazz.innerFunctions.getName;


w$ = window; // Short for browser's window object
d$ = document; // Short for browser's document object
System = {
	currentTimeMillis : function () {
		return new Date ().getTime ();
	},
	props : null, //new java.util.Properties (),
	getProperties : function () {
		return System.props;
	},
	setProperties : function (props) {
		System.props = props;
	},
	getProperty : function (key, def) {
		if (System.props != null) {
			return System.props.getProperty (key, def);
		}
		if (def != null) {
			return def;
		}
		return key;
	},
	setProperty : function (key, val) {
		if (System.props == null) {
			return ;
		}
		System.props.setProperty (key, val);
	},
	currentTimeMillis : function () {
		return new Date ().getTime ();
	},
	arraycopy : function (src, srcPos, dest, destPos, length) {
		if (src !== dest) {
			for (var i = 0; i < length; i++) {
				dest[destPos + i] = src[srcPos + i];
			}
		} else {
			var swap = [];
			for (var i = 0; i < length; i++) {
				swap[i] = src[srcPos + i];
			}
			for (var i = 0; i < length; i++) {
				dest[destPos + i] = swap[i];
			}
		}
	}
};
System.out = new JavaObject ();
System.out.__CLASS_NAME__ = "java.io.PrintStream";
System.out.print = function () {};
System.out.printf = function () {};
System.out.println = function () {};

System.err = new JavaObject ();
System.err.__CLASS_NAME__ = "java.io.PrintStream";
System.err.print = function () {};
System.err.printf = function () {};
System.err.println = function () {};

Clazz.popup = Clazz.assert = Clazz.log = Clazz.error = window.alert;

Thread = function () {};
Thread.J2S_THREAD = Thread.prototype.J2S_THREAD = new Thread ();
Thread.currentThread = Thread.prototype.currentThread = function () {
	return this.J2S_THREAD;
};

/* public */
Clazz.intCast = function (n) { // 32bit
	var b1 = (n & 0xff000000) >> 24;
	var b2 = (n & 0xff0000) >> 16;
	var b3 = (n & 0xff00) >> 8;
	var b4 = n & 0xff;
	if ((b1 & 0x80) != 0) {
		return -(((b1 & 0x7f) << 24) + (b2 << 16) + (b3 << 8) + b4 + 1);
	} else {
		return (b1 << 24) + (b2 << 16) + (b3 << 8) + b4;
	}
};

/* public */
Clazz.shortCast = function (s) { // 16bit
	var b1 = (n & 0xff00) >> 8;
	var b2 = n & 0xff;
	if ((b1 & 0x80) != 0) {
		return -(((b1 & 0x7f) << 8) + b2 + 1);
	} else {
		return (b1 << 8) + b4;
	}
};

/* public */
Clazz.byteCast = function (b) { // 8bit
	if ((b & 0x80) != 0) {
		return -((b & 0x7f) + 1);
	} else {
		return b & 0xff;
	}
};

/* public */
Clazz.charCast = function (c) { // 8bit
	return String.fromCharCode (c & 0xff).charAt (0);
};

/**
 * Warning: Unsafe conversion!
 */
/* public */
Clazz.floatCast = function (f) { // 32bit
	return f;
};

/*
 * Try to fix JavaScript's shift operator defects on long type numbers.
 */

Clazz.longMasks = [];

Clazz.longReverseMasks = [];

Clazz.longBits = [];

(function () {
	var arr = [1];
	for (var i = 1; i < 53; i++) {
		arr[i] = arr[i - 1] + arr[i - 1]; // * 2 or << 1
	}
	Clazz.longBits = arr;
	Clazz.longMasks[52] = arr[52];
	for (var i = 51; i >= 0; i--) {
		Clazz.longMasks[i] = Clazz.longMasks[i + 1] + arr[i];
	}
	Clazz.longReverseMasks[0] = arr[0];
	for (var i = 1; i < 52; i++) {
		Clazz.longReverseMasks[i] = Clazz.longReverseMasks[i - 1] + arr[i];
	}
}) ();


/* public */
Clazz.longLeftShift = function (l, o) { // 64bit
	if (o == 0) return l;
	if (o >= 64) return 0;
	if (o > 52) {
		error ("[Java2Script] Error : JavaScript does not support long shift!");
		return l;
	}
	if ((l & Clazz.longMasks[o - 1]) != 0) {
		error ("[Java2Script] Error : Such shift operator results in wrong calculation!");
		return l;
	}
	var high = l & Clazz.longMasks[52 - 32 + o];
	if (high != 0) {
		return high * Clazz.longBits[o] + (l & Clazz.longReverseMasks[32 - o]) << 0;
	} else {
		return l << o;
	}
};

/* public */
Clazz.intLeftShift = function (n, o) { // 32bit
	return (n << o) & 0xffffffff;
};

/* public */
Clazz.longRightShift = function (l, o) { // 64bit
	if ((l & Clazz.longMasks[52 - 32]) != 0) {
		return Math.round((l & Clazz.longMasks[52 - 32]) / Clazz.longBits[32 - o]) + (l & Clazz.longReverseMasks[o]) >> o;
	} else {
		return l >> o;
	}
};

/* public */
Clazz.intRightShift = function (n, o) { // 32bit
	return n >> o; // no needs for this shifting wrapper
};

/* public */
Clazz.long0RightShift = function (l, o) { // 64bit
	return l >>> o;
};

/* public */
Clazz.int0RightShift = function (n, o) { // 64bit
	return n >>> o; // no needs for this shifting wrapper
};

// Compress the common public API method in shorter name
$_L=Clazz.load;
$_W=Clazz.declareAnonymous;$_T=Clazz.declareType;
$_J=Clazz.declarePackage;$_C=Clazz.decorateAsClass;
$_Z=Clazz.instantialize;$_I=Clazz.declareInterface;$_D=Clazz.isClassDefined;
$_H=Clazz.pu$h;$_P=Clazz.p0p;$_B=Clazz.prepareCallback;
$_N=Clazz.innerTypeInstance;$_K=Clazz.makeConstructor;$_U=Clazz.superCall;$_R=Clazz.superConstructor;
$_M=Clazz.defineMethod;$_V=Clazz.overrideMethod;$_S=Clazz.defineStatics;
$_E=Clazz.defineEnumConstant;
$_F=Clazz.cloneFinals;
$_Y=Clazz.prepareFields;$_A=Clazz.newArray;$_O=Clazz.instanceOf;
$_G=Clazz.inheritArgs;$_X=Clazz.checkPrivateMethod;$_Q=Clazz.makeFunction;
$_s=Clazz.registerSerializableFields;
$_k=Clazz.overrideConstructor;


var reflect = Clazz.declarePackage ("java.lang.reflect");
Clazz.declarePackage ("java.security");

Clazz.innerFunctionNames = Clazz.innerFunctionNames.concat (["getSuperclass",
		"isAssignableFrom", "getMethods", "getMethod", "getDeclaredMethods", 
		"getDeclaredMethod", "getConstructor", "getModifiers", "isArray", "newInstance"]);

Clazz.innerFunctions.getSuperclass = function () {
	return this.superClazz;	
};
Clazz.innerFunctions.isAssignableFrom = function (clazz) {
	return Clazz.getInheritedLevel (clazz, this) >= 0;	
};
Clazz.innerFunctions.getConstructor = function () {
	return new java.lang.reflect.Constructor (this, [], [], 
			java.lang.reflect.Modifier.PUBLIC);
};
/**
 * TODO: fix bug for polymorphic methods!
 */
Clazz.innerFunctions.getDeclaredMethods = Clazz.innerFunctions.getMethods = function () {
	var ms = new Array ();
	var p = this.prototype;
	for (var attr in p) {
		if (typeof p[attr] == "function" && p[attr].__CLASS_NAME__ == null) {
			/* there are polynormical methods. */
			ms[ms.length] = new java.lang.reflect.Method (this, attr,
					[], java.lang.Void, [], java.lang.reflect.Modifier.PUBLIC);
		}
	}
	p = this;
	for (var attr in p) {
		if (typeof p[attr] == "function" && p[attr].__CLASS_NAME__ == null) {
			ms[ms.length] = new java.lang.reflect.Method (this, attr,
					[], java.lang.Void, [], java.lang.reflect.Modifier.PUBLIC
					| java.lang.reflect.Modifier.STATIC);
		}
	}
	return ms;
};
Clazz.innerFunctions.getDeclaredMethod = Clazz.innerFunctions.getMethod = function (name, clazzes) {
	var p = this.prototype;
	for (var attr in p) {
		if (name == attr && typeof p[attr] == "function" 
				&& p[attr].__CLASS_NAME__ == null) {
			/* there are polynormical methods. */
			return new java.lang.reflect.Method (this, attr,
					[], java.lang.Void, [], java.lang.reflect.Modifier.PUBLIC);
		}
	}
	p = this;
	for (var attr in p) {
		if (name == attr && typeof p[attr] == "function" 
				&& p[attr].__CLASS_NAME__ == null) {
			return new java.lang.reflect.Method (this, attr,
					[], java.lang.Void, [], java.lang.reflect.Modifier.PUBLIC
					| java.lang.reflect.Modifier.STATIC);
		}
	}
	return null;
};
Clazz.innerFunctions.getModifiers = function () {
	return java.lang.reflect.Modifier.PUBLIC;
};
Clazz.innerFunctions.isArray = function () {
	return false;
};
Clazz.innerFunctions.newInstance = function () {
	var clz = this;
	return new clz ();
};

//Object.newInstance = Clazz.innerFunctions.newInstance;
;(function(){  // BH added wrapper here
	var inF = Clazz.innerFunctionNames;
	for (var i = 0; i < inF.length; i++) {
		JavaObject[inF[i]] = Clazz.innerFunctions[inF[i]];
		Array[inF[i]] = Clazz.innerFunctions[inF[i]];
	}
	Array["isArray"] = function () {
		return true;
	};
})();

/* public */
Clazz.forName = function (clazzName) {
	if (Clazz.isClassDefined (clazzName)) {
		return Clazz.evalType (clazzName);
	}
	if (window["ClazzLoader"] != null) {
		ClazzLoader.setLoadingMode ("xhr.sync");
		ClazzLoader.loadClass (clazzName);
		//alert("TESTING HERE in Clazz.forName")
		return Clazz.evalType (clazzName);
	} else {
		Clazz.alert ("[Java2Script] Error: No ClassLoader!");
	}
};

/* For hotspot and unloading */

/* private */
Clazz.cleanDelegateMethod = function (m) {
	if (m == null) return;
	if (typeof m == "function" && m.lastMethod != null
			&& m.lastParams != null && m.lastClaxxRef != null) {
		m.lastMethod = null;
		m.lastParams = null;
		m.lastClaxxRef = null;
	}
};

/* public */
Clazz.unloadClass = function (qClazzName) {
	var cc = Clazz.evalType (qClazzName);
	if (cc != null) {
		Clazz.unloadedClasses[qClazzName] = cc;
		var clazzName = qClazzName;
		var pkgFrags = clazzName.split (/\./);
		var pkg = null;
		for (var i = 0; i < pkgFrags.length - 1; i++) {
			if (pkg == null) {
				pkg = Clazz.allPackage[pkgFrags[0]];
			} else {
				pkg = pkg[pkgFrags[i]]
			}
		}
		if (pkg == null) {
			Clazz.allPackage[pkgFrags[0]] = null;
			window[pkgFrags[0]] = null;
			// also try to unload inner or anonymous classes
			for (var c in window) {
				if (c.indexOf (qClazzName + "$") == 0) {
					Clazz.unloadClass (c);
					window[c] = null;
				}
			}
		} else {
			pkg[pkgFrags[pkgFrags.length - 1]] = null;
			// also try to unload inner or anonymous classes
			for (var c in pkg) {
				if (c.indexOf (pkgFrags[pkgFrags.length - 1] + "$") == 0) {
					Clazz.unloadClass (pkg.__PKG_NAME__ + "." + c);
					pkg[c] = null;
				}
			}
		}

		if (Clazz.allClasses[qClazzName] == true) {
			Clazz.allClasses[qClazzName] = false;
			// also try to unload inner or anonymous classes
			for (var c in Clazz.allClasses) {
				if (c.indexOf (qClazzName + "$") == 0) {
					Clazz.allClasses[c] = false;
				}
			}
		}
		
		for (var m in cc) {
			Clazz.cleanDelegateMethod (cc[m]);
		}
		for (var m in cc.prototype) {
			Clazz.cleanDelegateMethod (cc.prototype[m]);
		}

		if (window["ClazzLoader"] != null) {
			ClazzLoader.unloadClassExt (qClazzName);
		}
		
		return true;
	}
	return false;
};

//written by Dean Edwards, 2005
//with input from Tino Zijdel, Matthias Miller, Diego Perini

//http://dean.edwards.name/weblog/2005/10/add-event/

// Merge Dean Edwards' addEvent for Java2Script
/* public */
Clazz.addEvent = function (element, type, handler) {
	if (element.addEventListener) {
		element.addEventListener(type, handler, false);
	} else {
		// assign each event handler a unique ID
		if (!handler.$$guid) handler.$$guid = Clazz.addEvent.guid++;
		// create a hash table of event types for the element
		if (!element.events) element.events = {};
		// create a hash table of event handlers for each element/event pair
		var handlers = element.events[type];
		if (!handlers) {
			handlers = element.events[type] = {};
			// store the existing event handler (if there is one)
			if (element["on" + type]) {
				handlers[0] = element["on" + type];
			}
		}
		// store the event handler in the hash table
		handlers[handler.$$guid] = handler;
		// assign a global event handler to do all the work
		element["on" + type] = Clazz.handleEvent;
	}
};
/* private */
//a counter used to create unique IDs
Clazz.addEvent.guid = 1;

/* public */
Clazz.removeEvent = function (element, type, handler) {
	if (element.removeEventListener) {
		element.removeEventListener(type, handler, false);
	} else {
		// delete the event handler from the hash table
		if (element.events && element.events[type]) {
			delete element.events[type][handler.$$guid];
		}
	}
};

/* private */
Clazz.isVeryOldIE = navigator.userAgent.indexOf("MSIE 6.0") != -1 || navigator.userAgent.indexOf("MSIE 5.5") != -1 || navigator.userAgent.indexOf("MSIE 5.0") != -1;

/* protected */
Clazz.handleEvent = function (event) {
	var returnValue = true;
	// grab the event object (IE uses a global event object)
	if (!Clazz.isVeryOldIE) {
		event = event || Clazz.fixEvent(((this.ownerDocument || this.document || this).parentWindow || window).event);
	} else { // The above line is buggy in IE 6.0
		if (event == null) {
			var evt = null;
			try {
				var pWindow = (this.ownerDocument || this.document || this).parentWindow;
				if (pWindow != null) {
					evt = pWindow.event;
				}
			} catch (e) {
				evt = window.event;
			}
			event = Clazz.fixEvent(evt);
		}
	}
	// get a reference to the hash table of event handlers
	var handlers = this.events[event.type];
	// execute each event handler
	for (var i in handlers) {
		if (isNaN (i)) {
			continue;
		}
		this.$$handleEvent = handlers[i];
		if (typeof this.$$handleEvent != "function") {
			continue;
		}
		if (this.$$handleEvent(event) === false) {
			returnValue = false;
		}
	}
	return returnValue;
};

/* private */
Clazz.fixEvent = function (event) {
	// add W3C standard event methods
	event.preventDefault = Clazz.fixEvent.preventDefault;
	event.stopPropagation = Clazz.fixEvent.stopPropagation;
	return event;
};
Clazz.fixEvent.preventDefault = function() {
	this.returnValue = false;
};
Clazz.fixEvent.stopPropagation = function() {
	this.cancelBubble = true;
};

}
/******************************************************************************
 * Copyright (c) 2007 java2script.org and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     Zhou Renjian - initial API and implementation
 *****************************************************************************/
/*******
 * @author zhou renjian
 * @create July 10, 2006
 *******/

//if (window["ClazzNode"] == null) {
/**
 * TODO:
 * Make optimization over class dependency tree.
 */

/*
 * ClassLoader Summary
 * 
 * ClassLoader creates SCRIPT elements and setup class path and onload 
 * callback to continue class loading.
 *
 * In the onload callbacks, ClazzLoader will try to calculate the next-to-be-
 * load *.js and load it. In *.js, it will contains some codes like
 * Clazz.load (..., "$wt.widgets.Control", ...);
 * to provide information to build up the class dependency tree.
 *
 * Some known problems of different browsers:
 * 1. In IE, loading *.js through SCRIPT will first triggers onreadstatechange 
 * event, and then executes inner *.js source.
 * 2. In Firefox, loading *.js will first executes *.js source and then 
 * triggers onload event.
 * 3. In Opera, similar to IE, but trigger onload event. (TODO: More details 
 * should be studied. Currently, Opera supports no multiple-thread-loading)
 * 
 * For class dependency tree, actually, it is not a tree. It is a reference
 * net with nodes have n parents and n children. There is a root, which 
 * ClassLoader knows where to start searching and loading classes, for such
 * a net. Each node is a class. Each class may require a set of must-classes, 
 * which must be loaded before itself getting initialized, and also need a set
 * of optional classes, which also be loaded before being called.
 *
 * The class loading status will be in 6 stages.
 * 1. Unknown, the class is newly introduced by other class.
 * 2. Known, the class is already mentioned by other class.
 * 3. Loaded, *.js source is in memory, but may not be initialized yet. It 
 * requires all its must-classes be intiailized, which is in the next stage.
 * 4. Musts loaded, all must classes is already loaded and declared.
 * 5. Delcared, the class is already declared (ClazzLoader#isClassDefined).
 * 6. Optionals loaded, all optional classes is loaded and declared.
 *
 * The ClassLoader tries to load all necessary classes in order, and intialize
 * them in order. For such job, it will traverse the dependency tree, and try 
 * to next class to-be-loaded. Sometime, the class dependencies may be in one
 * or more cycles, which must be broken down so classes is loaded in correct
 * order.
 *
 * Loading order and intializing order is very important for the ClassLoader.
 * The following technical options are considered:
 * 1. SCRIPT is loading asynchronously, which means controling order must use
 * callback methods to continue.
 * 2. Multiple loading threads are later introduced, which requires the 
 * ClassLoader should use variables to record the class status.
 * 3. Different browsers have different loading orders, which means extra tests
 * should be tested to make sure loading order won't be broken.
 * 4. Java2Script simulator itself have some loading orders that must be 
 * honored, which means it should be integrated seamlessly to Clazz system.
 * 5. Packed *.z.js is introduced to avoid lots of small *.js which requires 
 * lots of HTTP connections, which means that packed *.z.js should be treated
 * specially (There will be mappings for such packed classes).
 * 6. *.js or *.css loading may fail according to network status, which means
 * another loading try should be performed, so ClazzLoader is more robust.
 * 7. SWT lazy loading is later introduced, which means that class loading
 * process may be paused and should be resumed later.
 *
 * Some known bugs:
 * <code>$_L(["$wt.graphics.Drawable","$wt.widgets.Widget"],
 *  "$wt.widgets.Control", ...</code>
 * has errors while must classes in different order such as
 * <code>$_L(["$wt.widgets.Widget", "$wt.graphics.Drawable"],
 *  "$wt.widgets.Control", ...</code>
 * has no error.
 * 
 * Other maybe bug scenarios:
 * 1. In <code>ClazzLoader.maxLoadingThreads = 1;</code> single loading thread 
 * mode, there are no errors, but in default multiple thread loading mode, 
 * there are errors.
 * 2. No errors in one browser, but has errors on other browsers (Browser 
 * script loading order differences).
 * 3. First time loading has errors, but reloading it gets no errors (Maybe 
 * HTTP connections timeout, but should not accur in local file system, or it
 * is a loading bug by using JavaScript timeout thread).
 */

/*
 * The following comments with "#" are special configurations for a much
 * smaller *.js file size.
 *
 * @see net.sf.j2s.lib/src/net/sf/j2s/lib/build/SmartJSCompressor.java
 */
/*-#
 # ClazzNode -> $CN$
 # ClazzLoader -> $CL$
 # <<< ClazzLoader = $CL$;
 #-*/
 
/*-#
 # parents -> sp
 # musts -> sm
 # xxxoptionals -> so
 # declaration -> dcl
 # optionalsLoaded -> oled
 # qClazzName ->Nq
 #-*/
 
/**
 * Static class loader class
 */
ClazzLoader = function () {};

/**
 * Class dependency tree node
 */
/* private */
ClazzNode = function () {
	this.parents = new Array ();
	this.musts = new Array ();
	this.optionals = new Array ();
	this.declaration = null;
	this.name = null; // id
	this.path = null;
	this.status = 0;
	this.random = 0.13412;
	this.optionalsLoaded = null;
	this.toString = function () {
		if (this.name != null) {
			return this.name;
		} else if (this.path != null) {
			return this.path;
		} else {
			return "ClazzNode";
		}
	};
};

;(function(ClazzLoader, ClazzNode) {
/*-#
 # ClazzNode.STATUS_UNKNOWN = 0
 # ClazzNode.STATUS_KNOWN -> 1
 # ClazzNode.STATUS_CONTENT_LOADED -> 2
 # ClazzNode.STATUS_MUSTS_LOADED -> 3
 # ClazzNode.STATUS_DECLARED -> 4
 # ClazzNode.STATUS_OPTIONALS_LOADED -> 5
 #-*/
/*# >>x #*/
ClazzNode.STATUS_UNKNOWN = 0;
ClazzNode.STATUS_KNOWN = 1;
ClazzNode.STATUS_CONTENT_LOADED = 2;
ClazzNode.STATUS_MUSTS_LOADED = 3;
ClazzNode.STATUS_DECLARED = 4;
ClazzNode.STATUS_OPTIONALS_LOADED = 5;
/*# x<< #*/

             
ClazzLoader.loaders = [];

ClazzLoader.requireLoaderByBase = function (base) {
	for (var i = 0; i < ClazzLoader.loaders.length; i++) {
		if (ClazzLoader.loaders[i].base == base) {
			return ClazzLoader.loaders[i];
		}
	}
	var loader = new ClazzLoader ();
	loader.base = base; 
	ClazzLoader.loaders[ClazzLoader.loaders.length] = loader;
	return loader;
};

/**
 * Class dependency tree
 */
/*-# clazzTreeRoot -> tr #-*/
ClazzLoader.clazzTreeRoot = new ClazzNode ();

/**
 * Used to keep the status whether a given *.js path is loaded or not.
 */
/* private */
/*-# loadedScripts -> ls #-*/
ClazzLoader.loadedScripts = {};

/**
 * Multiple threads are used to speed up *.js loading.
 */
/* private */
/*-# inLoadingThreads -> ilt #-*/
ClazzLoader.inLoadingThreads = 0;

/**
 * Maximum of loading threads
 */
/* protected */
ClazzLoader.maxLoadingThreads = 6;

ClazzLoader.userAgent = navigator.userAgent.toLowerCase ();
ClazzLoader.isOpera = (ClazzLoader.userAgent.indexOf ("opera") != -1);
ClazzLoader.isIE = (ClazzLoader.userAgent.indexOf ("msie") != -1) && !ClazzLoader.isOpera;
ClazzLoader.isGecko = (ClazzLoader.userAgent.indexOf ("gecko") != -1);
//ClazzLoader.isChrome = (ClazzLoader.userAgent.indexOf ("chrome") != -1);

/*
 * Opera has different loading order which will result in performance degrade!
 * So just return to single thread loading in Opera!
 *
 * FIXME: This different loading order also causes bugs in single thread!
 */
if (ClazzLoader.isOpera) {
	ClazzLoader.maxLoadingThreads = 1;
	var index = ClazzLoader.userAgent.indexOf ("opera/");
	if (index != -1) {
		var verNumber = 9.0;
		try {
			verNumber = parseFloat(ClazzLoader.userAgent.subString (index + 6));
		} catch (e) {}
		if (verNumber >= 9.6) {
			ClazzLoader.maxLoadingThreads = 6;
		}
	} 
}

/**
 * Try to be compatiable with Clazz system.
 * In original design ClazzLoader and Clazz are independent!
 *  -- zhourenjian @ December 23, 2006
 */
if (window["Clazz"] != null && Clazz.isClassDefined) {
	ClazzLoader.isClassDefined = Clazz.isClassDefined;
} else {
	/*-# definedClasses -> dC #-*/
	ClazzLoader.definedClasses = {};
	ClazzLoader.isClassDefined = function (clazzName) {
		return ClazzLoader.definedClasses[clazzName] == true;
	};
}

/*
 * binaryFolders will be used for ResourceBundle to check *.properties
 * files. The default should always be "bin/"!
 */
if (window["Clazz"] != null && Clazz.binaryFolders != null) {
	ClazzLoader.binaryFolders = Clazz.binaryFolders;
} else {
	ClazzLoader.binaryFolders = ["bin/", "", "j2slib/"];
}

/*# {$clazz.existed} >>x #*/
/* public */
ClazzLoader.addBinaryFolder = function (bin) {
	if (bin != null) {
		var bins = ClazzLoader.binaryFolders;
		for (var i = 0; i < bins.length; i++) {
			if (bins[i] == bin) {
				return;
			}
		}
		bins[bins.length] = bin;
	}
};
/* public */
ClazzLoader.removeBinaryFolder = function (bin) {
	if (bin != null) {
		var bins = ClazzLoader.binaryFolders;
		for (var i = 0; i < bins.length; i++) {
			if (bins[i] == bin) {
				for (var j = i; j < bins.length - 1; j++) {
					bins[j] = bins[j + 1];
				}
				bins.length--;
				return bin;
			}
		}
	}
	return null;
};
/* public */
ClazzLoader.setPrimaryFolder = function (bin) {
	if (bin != null) {
		ClazzLoader.removeBinaryFolder (bin);
		var bins = ClazzLoader.binaryFolders;
		for (var i = bins.length - 1; i >= 0; i--) {
			bins[i + 1] = bins[i];
		}
		bins[0] = bin;
	}
};
/*# x<<
 # ClazzLoader.addBinaryFolder = Clazz.addBinaryFolder;
 # ClazzLoader.removeBinaryFolder = Clazz.removeBinaryFolder;
 # ClazzLoader.setPrimaryFolder = Clazz.setPrimaryFolder;
 #*/

/**
 * Indicate whether ClazzLoader is loading script synchronously or 
 * asynchronously.
 */
/* protected */
/*-# isAsynchronousLoading -> async #-*/
ClazzLoader.isAsynchronousLoading = true;

/* protected */
/*-# isUsingXMLHttpRequest -> xhr #-*/
ClazzLoader.isUsingXMLHttpRequest = false;

/* protected */
/*-# loadingTimeLag -> ltl #-*/
ClazzLoader.loadingTimeLag = -1;

/**
 * String mode:
 * asynchronous modes:
 * async(...).script, async(...).xhr, async(...).xmlhttprequest,
 * script.async(...), xhr.async(...), xmlhttprequest.async(...),
 * script
 * 
 * synchronous modes:
 * sync(...).xhr, sync(...).xmlhttprequest,
 * xhr.sync(...), xmlhttprequest.sync(...),
 * xmlhttprequest, xhr
 *                                                    
 * Integer mode:
 * Script/XHR bit: 1, 
 * 0: Script, 1: XHR
 * Asynchronous/Synchronous bit: 2
 * 0: Asynchronous, 2: Synchronous
 *
 * 0: Script & Asynchronous
 * 1: XHR & Asynchronous
 * 2: Script & Synchronous [Never]
 * 3: XHR & Synchronous
 */
/* public */
ClazzLoader.setLoadingMode = function (mode, timeLag) {
	if (mode == null) {
		if (ClazzLoader.isAsynchronousLoading && timeLag >= 0) {
			ClazzLoader.loadingTimeLag = timeLag;
		} else {
			ClazzLoader.loadingTimeLag = -1;
		}
		return;
	}
	if (typeof mode == "string") {
		mode = mode.toLowerCase ();
		if (mode.length == 0 || mode.indexOf ("script") != -1) {
			ClazzLoader.isUsingXMLHttpRequest = false;
			ClazzLoader.isAsynchronousLoading = true;
		} else {
			ClazzLoader.isUsingXMLHttpRequest = true;
			if (mode.indexOf ("async") != -1) {
				ClazzLoader.isAsynchronousLoading = true;
			} else {
				ClazzLoader.isAsynchronousLoading = false;
			}
		}
			ClazzLoader.isAsynchronousLoading = false; // BH
	/*# {$no.clazzloader.mode} >>x #*/
	} else {
		if (mode == ClazzLoader.MODE_SCRIPT) {
			ClazzLoader.isUsingXMLHttpRequest = false;
			ClazzLoader.isAsynchronousLoading = true;
		} else {
			ClazzLoader.isUsingXMLHttpRequest = true;
			if (mode == ClazzLoader.MODE_XHR_ASYNC) {
				ClazzLoader.isAsynchronousLoading = true;
			} else {
				ClazzLoader.isAsynchronousLoading = false;
			}
		}
	/*# x<< #*/
	}
	if (ClazzLoader.isAsynchronousLoading && timeLag >= 0) {
		ClazzLoader.loadingTimeLag = timeLag;
	} else {
		ClazzLoader.loadingTimeLag = -1;
	}
};

/*# {$no.clazzloader.mode} >>x #*/
ClazzLoader.MODE_SCRIPT = 0;
ClazzLoader.MODE_SCRIPT_ASYNC = 0;
ClazzLoader.MODE_XHR = 3;
ClazzLoader.MODE_XHR_SYNC = 3;
ClazzLoader.MODE_XHR_ASYNC = 1;


/*# x<< #*/
/**
 * Expand the shorten list of class names.
 * For example:
 * $wt.widgets.Shell, $.Display, $.Decorations
 * will be expanded to 
 * org.eclipse.swt.widgets.Shell, org.eclipse.swt.widgets.Display, 
 * org.eclipse.swt.widgets.Decorations ....
 * in which "$wt." stands for "org.eclipse.swt.", and "$." stands for
 * the previous class name's package.
 *
 * This method will be used to unwrap the required/optional classes list and 
 * the ignored classes list.
 */
/* private */
/*x-# unwrapArray -> uA #-x*/
ClazzLoader.unwrapArray = function (arr) {
	if (arr == null || arr.length == 0) {
		return arr;
	}
	var last = null;
	for (var i = 0; i < arr.length; i++) {
		if (arr[i] == null) {
			continue;
		}
		if (arr[i].charAt (0) == '$') {
			if (arr[i].charAt (1) == '.') {
				if (last == null) {
					continue;
				}
				var idx = last.lastIndexOf (".");
				if (idx != -1) {
					var prefix = last.substring (0, idx);
					arr[i] = prefix + arr[i].substring (1);
				}
			} else {
				arr[i] = "org.eclipse.s" + arr[i].substring (1);
			}
		}
		last = arr[i];
	}
	return arr;
};

/**
 * Used to keep to-be-loaded classes.
 */
/* private */
/*-# classQueue -> cq #-*/
ClazzLoader.classQueue = new Array ();

/* private */
/*-# classpathMap -> cm #-*/

//System.out.println("resetting classpathMap")
ClazzLoader.classpathMap = {};

/* public */
ClazzLoader.packageClasspath = function (pkg, base, index) {
	var map = ClazzLoader.classpathMap;
  
  /*
	 * In some situation, maybe,
	 * ClazzLoader.packageClasspath ("java", ..., true);
	 * is called after other ClazzLoader#packageClasspath, e.g.
	 * <code>
	 * ClazzLoader.packageClasspath ("org.eclipse.swt", "...", true);
	 * ClazzLoader.packageClasspath ("java", "...", true);
	 * </code>
	 * which is not recommended. But ClazzLoader should try to adjust orders
	 * which requires "java" to be declared before normal ClazzLoader
	 * #packageClasspath call before that line! And later that line
	 * should never initialize "java/package.js" again!
	 */
	var isPkgDeclared = (index == true && map["@" + pkg] != null);
	if (index && map["@java"] == null && pkg.indexOf ("java") != 0) {
		ClazzLoader.assurePackageClasspath ("java");
	}
	if (pkg instanceof Array) {
		ClazzLoader.unwrapArray (pkg);
		for (var i = 0; i < pkg.length; i++) {
			ClazzLoader.packageClasspath (pkg[i], base, index);
		}
		return;
	}
	if (pkg == "java" || pkg == "java.*") {
		// support ajax for default
		var key = "@net.sf.j2s.ajax";
		if (map[key] == null && base != null) {
			map[key] = base;
		}
		key = "@net.sf.j2s";
		if (map[key] == null && base != null) {
			map[key] = base;
		}
	} else if (pkg == "swt") { //abbrev
		pkg = "org.eclipse.swt";
	} else if (pkg == "ajax") { //abbrev
		pkg = "net.sf.j2s.ajax";
	} else if (pkg == "j2s") { //abbrev
		pkg = "net.sf.j2s";
	}
	if (pkg.lastIndexOf (".*") == pkg.length - 2) {
		pkg = pkg.substring (0, pkg.length - 2);
	}
	if (base != null) // critical for multiple applets
		map["@" + pkg] = base;
	if (index == true && window[pkg + ".registered"] != true && !isPkgDeclared) {
		ClazzLoader.pkgRefCount++;
    if (pkg == "java")pkg = "core" // JSmol -- moves java/package.js to core/package.js
		ClazzLoader.loadClass (pkg + ".package", function () {
					ClazzLoader.pkgRefCount--;
					if (ClazzLoader.pkgRefCount == 0) {
						ClazzLoader.runtimeLoaded ();
					}
				}, true);
	}
};

ClazzLoader.pkgRefCount = 0;

/**
 * Register classes to a given *.z.js path, so only a single *.z.js is loaded
 * for all those classes.
 */
/* public */
/*-# clazzes -> zs #-*/
ClazzLoader.jarClasspath = function (jar, clazzes) {
	if (!(clazzes instanceof Array))
    clazzes = [classes];
	ClazzLoader.unwrapArray (clazzes);
	for (var i = 0; i < clazzes.length; i++) {
		ClazzLoader.classpathMap["#" + clazzes[i]] = jar;
	}
	ClazzLoader.classpathMap["$" + jar] = clazzes;
};

/**
 * Usually be used in .../package.js. All given packages will be registered
 * to the same classpath of given prefix package.
 */
/* public */
ClazzLoader.registerPackages = function (prefix, pkgs) {
	//System.out.println("package " + prefix);
	ClazzLoader.checkInteractive ();
	var base = ClazzLoader.getClasspathFor (prefix + ".*", true);
	//System.out.println(base);
	for (var i = 0; i < pkgs.length; i++) {
		if (window["Clazz"] != null) {
			Clazz.declarePackage (prefix + "." + pkgs[i]);
		}
		ClazzLoader.packageClasspath (prefix + "." + pkgs[i], base);
	}
};

/**
 * Using multiple sites to load *.js in multiple threads. Using multiple
 * sites may avoid 2 HTTP 1.1 connections recommendation limit.
 * Here is a default implementation for http://archive.java2script.org.
 * In site archive.java2script.org, there are 6 sites:
 * 1. http://archive.java2script.org or http://a.java2script.org
 * 2. http://erchive.java2script.org or http://e.java2script.org
 * 3. http://irchive.java2script.org or http://i.java2script.org
 * 4. http://orchive.java2script.org or http://o.java2script.org
 * 5. http://urchive.java2script.org or http://u.java2script.org
 * 6. http://yrchive.java2script.org or http://y.java2script.org
 */
/* protected */
ClazzLoader.multipleSites = function (path) {
	var deltas = window["j2s.update.delta"];
	if (deltas != null && deltas instanceof Array && deltas.length >= 3) {
		var lastOldVersion = null;
		var lastNewVersion = null;
		for (var i = 0; i < deltas.length / 3; i++) {
			var oldVersion = deltas[i + i + i];
			if (oldVersion != "$") {
				lastOldVersion = oldVersion;
			}
			var newVersion = deltas[i + i + i + 1];
			if (newVersion != "$") {
				lastNewVersion = newVersion;
			}
			var relativePath = deltas[i + i + i + 2];
			var key = lastOldVersion + "/" + relativePath;
			var idx = path.indexOf (key);
			if (idx != -1 && idx == path.length - key.length) {
				path = path.substring (0, idx) + lastNewVersion + "/" + relativePath;
				break;
			}
		}
	}
	var length = path.length;
	if (ClazzLoader.maxLoadingThreads > 1 
			&& ((length > 15 && path.substring (0, 15) == "http://archive.")
			|| (length > 9 && path.substring (0, 9) == "http://a."))) {
		var index = path.lastIndexOf ("/");
		if (index < length - 3) {
			var arr = ['a', 'e', 'i', 'o', 'u', 'y'];
			var c1 = path.charCodeAt (index + 1);
			var c2 = path.charCodeAt (index + 2);
			var idx = (length - index) * 3 + c1 * 5 + c2 * 7; // Hash
			return path.substring (0, 7) + arr[idx % 6] + path.substring (8);
		}
	}
	return path;
};

/**
 * Return the *.js path of the given class. Maybe the class is contained
 * in a *.z.js jar file.
 * @param clazz Given class that the path is to be calculated for. May
 * be java.package, or java.lang.String
 * @param forRoot Optional argument, if true, the return path will be root
 * of the given classs' package root path.
 * @param ext Optional argument, if given, it will replace the default ".js"
 * extension.
 */
/* public */
ClazzLoader.getClasspathFor = function (clazz, forRoot, ext) {


	//System.out.println("check js path : " + arguments.callee.caller);
	//System.out.println("gcf " + clazz + " "  + forRoot + " " + ext);
	var path = ClazzLoader.classpathMap["#" + clazz];
	//System.out.println(path);
	var base = null;
	
	if (path != null) {
	//System.out.println("testing" + clazz + " " + forRoot + " " + ext)
		if (!forRoot && ext == null) { // return directly
			return ClazzLoader.multipleSites (path);
		} else {
			var idx = path.lastIndexOf (clazz.replace (/\./g, "/"));
			if (idx != -1) {
				base = path.substring (0, idx);
			} else {
				/*
				 * Check more: Maybe the same class' *.css is located
				 * in the same folder.
				 */
				idx = clazz.lastIndexOf (".");
				if (idx != -1) {
					idx = path.lastIndexOf (clazz.substring (0, idx)
							.replace (/\./g, "/"));
					if (idx != -1) {
						base = path.substring (0, idx);
					}
				}
			}
		}
	} else {
		/*
		path = ClazzLoader.classpathMap["@" + clazz]; // package
		if (path != null) {
			return ClazzLoader.assureBase (path) + clazz.replace (/\./g, "/") + "/";
		}
		*/
		//System.out.println("path was null")
		
		var idx = clazz.lastIndexOf (".");
		//*
		while (idx != -1) {
			var pkg = clazz.substring (0, idx);
			base = ClazzLoader.classpathMap["@" + pkg];
			//for (var jj in ClazzLoader.classpathMap)System.out.println("map[" + jj + "]=" + ClazzLoader.classpathMap[jj])
			//System.out.println("found " + base + " for @" + pkg)
			if (base != null) {
				break;
			}
			idx = clazz.lastIndexOf (".", idx - 2);
		}
		//*/
		/*
		if (idx != -1) {
			var pkg = clazz.substring (0, idx);
			base = ClazzLoader.classpathMap["@" + pkg];
		}
		//*/
	}
	
	
	
	//alert("gcf2 base=" + base);
	base = ClazzLoader.assureBase (base);
	//alert("gcf2za assurebase=" + base);
	if (forRoot) {
		return ClazzLoader.multipleSites (base);
	}
	if (clazz.lastIndexOf (".*") == clazz.length - 2) {
		var s = base + clazz.substring (0, idx + 1)
				.replace (/\./g, "/");
		//System.out.println("gcf2b " + s)
		return ClazzLoader.multipleSites (s);
	}
	if (ext == null) {
		ext = ".js";
	} else if (ext.charAt (0) != '.') {
		ext = "." + ext;
	}
	var jsPath = base + clazz.replace (/\./g, "/") + ext;
	
			//System.out.println("gcf2xx base=" + base + " clazz=" + clazz + " jsPath=" + jsPath)

	return ClazzLoader.multipleSites (jsPath);
};

/* private */
/*-# assureBase -> aB #-*/
ClazzLoader.assureBase = function (base) {

	if (base == null) {
		// Try to be compatiable with Clazz system.
		var bins = "binaryFolders";
		if (window["Clazz"] != null && Clazz[bins] != null
				&& Clazz[bins].length != 0) {
			base = Clazz[bins][0];
		} else if (ClazzLoader[bins] != null 
				&& ClazzLoader[bins].length != 0) {
			base = ClazzLoader[bins][0];
		} else {
			base = "j2s";
		}
	}
	if (base.lastIndexOf ("/") != base.length - 1) {
		base += "/";
	}
	return base;
};

/* Used to keep ignored classes */
/* private */
/*-# excludeClassMap -> exmap #-*/
ClazzLoader.excludeClassMap = {};

/**
 * To ignore some classes.
 */
/* public */
ClazzLoader.ignore = function () {
	var clazzes = null;
	if (arguments.length == 1) {
		if (arguments[0] instanceof Array) {
			clazzes = arguments[0];
		}
	}
	if (clazzes == null) {
		clazzes = new Array ();
		for (var i = 0; i < arguments.length; i++) {
			clazzes[clazzes.length] = arguments[i];
		}
	}
	ClazzLoader.unwrapArray (clazzes);
	for (var i = 0; i < clazzes.length; i++) {
		ClazzLoader.excludeClassMap["@" + clazzes[i]] = true;
	}
};

/* private */
/*-# isClassExcluded -> isEx #-*/
ClazzLoader.isClassExcluded = function (clazz) {
	return ClazzLoader.excludeClassMap["@" + clazz] == true;
};

/**
 * The following *.script* can be overriden to indicate the 
 * status of classes loading.
 *
 * TODO: There should be a Java interface with name like INativeLoaderStatus
 */
/* protected */
ClazzLoader.scriptLoading = function (file) {};

/* protected */
ClazzLoader.scriptLoaded = function (file) {};

/* protected */
ClazzLoader.scriptInited = function (file) {
};

/* protected */
ClazzLoader.scriptCompleted = function (file) {};

/* protected */
ClazzLoader.classUnloaded = function (clazz) {};

/* protected */
ClazzLoader.classReloaded = function (clazz) {};

/**
 * After all the classes are loaded, this method will be called.
 * Should be overriden to run *.main([]).
 */
/* protected */
ClazzLoader.globalLoaded = function () {};

/* protected */
ClazzLoader.keepOnLoading = true;

/* private */
/*-# mapPath2ClassNode -> p2node #-*/
ClazzLoader.mapPath2ClassNode = {};

/* private */
ClazzLoader.xhrOnload = function (transport, file) {

	if (transport.status >= 400 || transport.responseText == null
			|| transport.responseText.length == 0) { // error
			Clazz.alert("xhronload error" + transport.responseText);
		var fs = ClazzLoader.failedScripts;
		if (fs[file] == null) {
			// Silently take another try for bad network
			fs[file] = 1;
			ClazzLoader.loadedScripts[file] = false;
			ClazzLoader.loadScript (file, "xhrOnload 2nd try");
			return;
		} else {
			Clazz.alert ("[Java2Script] ClazzLoader.xhrOnload Error in loading " + file + "!");
		}
		ClazzLoader.tryToLoadNext (file);
	} else {
    ClazzLoader.evaluate(file, transport.responseText);
	}
};

ClazzLoader.evaluate = function(file, js) {
 		try {
			eval(js);
		} catch (e) {
    alert(e)
			Clazz.alert ("[Java2Script] Script error: " + e.message);
			throw e;
		}
		ClazzLoader.scriptLoaded (file);
		ClazzLoader.tryToLoadNext (file);
}
/**
 * Empty onreadystatechange for fixing IE's memeory leak on XMLHttpRequest
 */
/* private */
/*-# emptyOnRSC -> rsc #-*/
ClazzLoader.emptyOnRSC = function () {
};

/* protected */
/*-# failedScripts -> fss #-*/
ClazzLoader.failedScripts = {};

/* protected */
/*-# failedHandles -> fhs #-*/
ClazzLoader.failedHandles = {};

/* protected */
ClazzLoader.takeAnotherTry = true;

/* private */
/*-# generateRemovingFunction -> gRF #-*/
ClazzLoader.generateRemovingFunction = function (node) {
	return function () {
		if (node.readyState != "interactive") {
			try {
				if (node.parentNode != null) {
				  //alert("removing script " + node.src);
					node.parentNode.removeChild (node);
				}
			} catch (e) { }
			node = null;
		}
	};
};

/*
 * Dynamically SCRIPT elements are removed after they are parsed into memory.
 * And removed *.js may not be fetched again by "Refresh" action. And it will
 * save loading time for Java2Script applications.
 *
 * This may disturb debugging tools such as Firebug. Setting
 * window["j2s.script.debugging"] = true;
 * will ignore removing SCRIPT elements requests.
 */
/* private */
/*-# removeScriptNode -> RsN #-*/
ClazzLoader.removeScriptNode = function (n) {
	if (window["j2s.script.debugging"]) {
		return;
	}
	// lazily remove script nodes.
	window.setTimeout (ClazzLoader.generateRemovingFunction (n), 1);
};

/* private */
/*-# generatingXHROnload -> gXOd #-*/
ClazzLoader.generatingXHROnload = function (transport, file) {
	return function () {
		ClazzLoader.xhrOnload (transport, file);
		transport = null;
		file = null;
	};
};

/* private */
/*-# generatingXHRCallback -> gXcb #-*/
ClazzLoader.generatingXHRCallback = function (transport, file) {
	return function () {
		if (transport.readyState == 4) {
			if (ClazzLoader.inLoadingThreads > 0) {
				ClazzLoader.inLoadingThreads--;
			}
			var lazyFun = ClazzLoader.generatingXHROnload (transport, file);
			if (isActiveX) {
				transport.onreadystatechange = ClazzLoader.emptyOnRSC;
				// For IE, try to avoid stack overflow errors
				window.setTimeout (lazyFun,
						ClazzLoader.loadingTimeLag < 0 ? 0 : ClazzLoader.loadingTimeLag);
			} else {
				transport.onreadystatechange = null;
				if (ClazzLoader.loadingTimeLag >= 0) {
					window.setTimeout (lazyFun, ClazzLoader.loadingTimeLag);
				} else {
					ClazzLoader.xhrOnload (transport, file);
				}
			}
			transport = null;
			file = null;
		}
	};
};

/* private */
/*-# loadingNextByPath -> lNBP #-*/
ClazzLoader.loadingNextByPath = function (path) {
	if (ClazzLoader.loadingTimeLag >= 0) {
		window.setTimeout (function () {
					ClazzLoader.tryToLoadNext (path);
				}, ClazzLoader.loadingTimeLag);
	} else {
		ClazzLoader.tryToLoadNext (path);
	}
};

/* private */
/*-# ieToLoadScriptAgain -> iTLA #-*/
ClazzLoader.ieToLoadScriptAgain = function (path, local) {
	var fun = function () {
		if (!ClazzLoader.takeAnotherTry) {
			return;
		}
		// next time in "loading" state won't get waiting!
		ClazzLoader.failedScripts[path] = 0;
		ClazzLoader.loadedScripts[path] = false;
		// failed count down!
		if (ClazzLoader.inLoadingThreads > 0) {
			ClazzLoader.inLoadingThreads--;
		}
		// Take another try!
		// log ("re - loading ... " + path);
		ClazzLoader.loadScript (path, "ietoloadscriptAgain");
	};
	// consider 30 seconds available after failing!
	/*
	 * Set 1s waiting in local file system. Is it 1s enough?
	 * What about big *.z.js need more than 1s to initialize?
	 */
	var waitingTime = (local ? 500 : 15000); // 0.5s : 15s
	//alert ("waiting:" + waitingTime + " . " + path);
	return window.setTimeout (fun, waitingTime);
};

/* private */
/*-# w3cFailedLoadingTest -> wFLT #-*/
ClazzLoader.w3cFailedLoadingTest = function (script) {
	return window.setTimeout (function () {
				script.onerror ();
				script.timeoutHandle = null;
				script = null;
			}, 500); // 0.5s for loading a local file is considered enough long
};

/* private */
/*-# generatingW3CScriptOnCallback -> gWSC #-*/
ClazzLoader.generatingW3CScriptOnCallback = function (path, forError) {
	return function () {
	if (forError && Clazz.debuggingBH)Clazz.alert("############ forError=" + forError + " path=" + path + " ####" + (forError ? "NOT" : "") + "LOADED###");

		if (ClazzLoader.isGecko && this.timeoutHandle != null) {
			window.clearTimeout (this.timeoutHandle);
			this.timeoutHandle = null;
		}
		if (ClazzLoader.inLoadingThreads > 0) {
			ClazzLoader.inLoadingThreads--;
		}
		this.onload = null;
		this.onerror = null;
    var checkOpera = false;   // BH - disabled this for Opera 10
		if (!forError && checkOpera && ClazzLoader.isOpera // Opera only
				&& !ClazzLoader.innerLoadedScripts[this.src]) {
			ClazzLoader.checkInteractive();
		}
		if (forError || (checkOpera && !ClazzLoader.innerLoadedScripts[this.src]
					&& ClazzLoader.isOpera)) { 
			// Opera will not take another try.
			var fss = ClazzLoader.failedScripts;
			if (fss[path] == null && ClazzLoader.takeAnotherTry) {
				// silently take another try for bad network
				//alert ("re loading " + path + " ... ");
				fss[path] = 1;
				if (!forError) {
					ClazzLoader.innerLoadedScripts[this.src] = false;
				}
				ClazzLoader.loadedScripts[path] = false;
				ClazzLoader.loadScript (path, "w3c script failed");
				ClazzLoader.removeScriptNode (this);
				//alert("............[Java2Script] "  + " inLoadingThreads=" + ClazzLoader.inLoadingThreads + " ClazzLoader.generatingW3CScriptOnCallback loading " + path);
				return;
			} else {
				//alert("[Java2Script] ClazzLoader.generatingW3CScriptOnCallback Error in loading " + path + "! inLoadingThreads=" + ClazzLoader.inLoadingThreads);
			}
			if (forError) {
				ClazzLoader.scriptLoaded (path);
			}
		} else {
		  //System.out.println("path loaded: " + path)
			ClazzLoader.scriptLoaded (path);
		}
		ClazzLoader.loadingNextByPath (path);
		ClazzLoader.removeScriptNode (this);
	};
};

/* private */
/*-# generatingIEScriptOnCallback -> gISC #-*/
ClazzLoader.generatingIEScriptOnCallback = function (path) {
	return function () {
		var fhs = ClazzLoader.failedHandles;
		var fss = ClazzLoader.failedScripts;
		var state = "" + this.readyState;
		
    
		var local = state == "loading" 
				&& (this.src.indexOf ("file:") == 0 
				|| (window.location.protocol == "file:"
				&& this.src.indexOf ("http") != 0));

		// alert (state + "/" + this.src);
		if (state != "loaded" && state != "complete") {
			/*
			 * When no such *.js existed, IE will be
			 * stuck here without loaded event!
			 */
			/*
			if ((window.location.protocol != "file:" 
					&& this.src.indexOf ("file:") != 0)
					|| this.readyState != "loading") {
				return;
			}
			*/
			if (fss[path] == null) {
				fhs[path] = ClazzLoader.ieToLoadScriptAgain (path, local);
				return;
			}
			if (fss[path] == 1) { // above function will be executed?!
				return;
			}
		}
		if (fhs[path] != null) {
			window.clearTimeout (fhs[path]);
			fhs[path] = null;
		}
		if ((local || state == "loaded")
				&& !ClazzLoader.innerLoadedScripts[this.src]) {
			if (!local && (fss[path] == null || fss[path] == 0)
					&& ClazzLoader.takeAnotherTry) {
				// failed! count down
				if (ClazzLoader.inLoadingThreads > 0) {
					ClazzLoader.inLoadingThreads--;
				}
				// silently take another try for bad network
				fss[path] = 1;
				// log ("reloading ... " + path);
				ClazzLoader.loadedScripts[path] = false;
				ClazzLoader.loadScript (path, "local or loaded failed");
				ClazzLoader.removeScriptNode (this);
				return;
			} else {
				Clazz.alert ("[Java2Script] generatingIEScriptOnCallback Error in loading " + path + "!");
			}
		}
		if (ClazzLoader.inLoadingThreads > 0) {
			ClazzLoader.inLoadingThreads--;
		}
		ClazzLoader.scriptLoaded (path);
		// Unset onreadystatechange, leaks mem in IE
		this.onreadystatechange = null; 
		ClazzLoader.loadingNextByPath (path);
		ClazzLoader.removeScriptNode (this);
	};
};

/*
 * There is another thread trying to remove j2slib.z.js or similar SCRIPT.
 * See the end of this file.
 */


		Clazz.transport = null;

/**
 * Load *.js by adding script elements into head. Hook the onload event to
 * load the next class in dependency tree.
 */
/* protected */
/*-#
 # loadScript -> xrpt
 #
 # transport -> tt
 # isActiveX -> iX
 # ignoreOnload -> iol
 #-*/
ClazzLoader.loadScript = function (file, why) {
	  Clazz.currentPath = file;
    //alert("loadScript" + file)
  	//System.out.println(("call loadScript " + file.replace(/\//g,"\\") + " Z").replace(/j2s[\\\.]/g,""))
	// maybe some scripts are to be loaded without needs to know onload event.
  
  //alert(file  + " " + arguments.callee.caller.caller )
  
 
	var ignoreOnload = (arguments[2] == true);
	if (ClazzLoader.loadedScripts[file] && !ignoreOnload) {
		ClazzLoader.tryToLoadNext (file);
		return;
	}
	ClazzLoader.loadedScripts[file] = true;
	/* also remove from those queue */
	var cq = ClazzLoader.classQueue;
	for (var i = 0; i < cq.length; i++) {
		if (cq[i] == file) {
			for (var j = i; j < cq.length - 1; j++) {
				cq[i] = cq[i + 1];
			}
			cq.length--;
			break;
		}
	}
  
  System.out.println("call loadScript "
    + file.replace(/\//g,"\\")
      .replace(/j2s[\\\.]/g,"") + (why ? " -- required by " + why : ""))


	if (ClazzLoader.isUsingXMLHttpRequest) {
		ClazzLoader.scriptLoading (file);
		//var transport = null;

    if (!ClazzLoader.isAsynchronousLoading) {
      // works in MSIE locally :)
    	var info = {dataType:"text",async:false,url:file};
		  var xhr = Jmol.$ajax(info);
      var data = xhr.responseText;
      ClazzLoader.evaluate(file, data); 
      return;
    }
    
		var isActiveX = false;
		if (!Clazz.transport) {
		if (window.XMLHttpRequest) {
			Clazz.transport = new XMLHttpRequest();
		} else {
			isActiveX = true;
			try {
				Clazz.transport = new ActiveXObject("Msxml2.XMLHTTP");
			} catch (e) {
				Clazz.transport = new ActiveXObject("Microsoft.XMLHTTP");
			}
		}
    
		if (Clazz.transport == null) { // should never happen in modern browsers.
			Clazz.alert ("[Java2Script] XMLHttpRequest not supported!");
			return;
		}
		}
		var transport = Clazz.transport
  
  //alert("transport=" + transport + " " + file)
  
  
  try{
  
		transport.open ("GET", file, ClazzLoader.isAsynchronousLoading);

  } catch (e) { alert(e + " " + file)}


		// transport.setRequestHeader ("User-Agent",
		// 		"Java2Script-Pacemaker/1.0 (+http://j2s.sourceforge.net)");
//		if (ClazzLoader.isAsynchronousLoading) {
			transport.onreadystatechange = ClazzLoader.generatingXHRCallback (
					transport, file);
			ClazzLoader.inLoadingThreads++;
			try {
				transport.send (null);
			} catch (e) {
				Clazz.alert ("[Java2Script] Loading file error: " + e.message);
				ClazzLoader.xhrOnload (transport, file);
				//throw e;
			}
//		} else {
//			try {
//				transport.send (null);
//			} catch (e) {
//				Clazz.alert ("[Java2Script] Loading file error: " + e.message);
//				//throw e;
//			}
//			ClazzLoader.xhrOnload (transport, file);
//		}
		return;
	}
	// Create script DOM element
	var script = document.createElement ("SCRIPT");
	script.type = "text/javascript";

  
	if (ClazzLoader.isChrome && ClazzLoader.reloadingClasses[file]) {
		script.src = file + "?" + Math.floor (100000 * Math.random ());
	} else {
		script.src = file;
	}
  
	var head = document.getElementsByTagName ("HEAD")[0];
	if (ignoreOnload) {
		head.appendChild (script);
		// ignore onload event and no call of ClazzLoader.scriptLoading
		return;
	}
	script.defer = true;
	// Alert when the script is loaded
	if (typeof (script.onreadystatechange) == "undefined" || !ClazzLoader.isIE) { // W3C
		if (ClazzLoader.isGecko && (file.indexOf ("file:") == 0 
				|| (window.location.protocol == "file:" && file.indexOf ("http") != 0))) {
			script.timeoutHandle = ClazzLoader.w3cFailedLoadingTest (script);
		}

		/*
		 * What about Safari and Google Chrome?
		 */
		/*
		 * Opera will trigger onload event even there are no *.js existed
		 */
		script.onload = ClazzLoader.generatingW3CScriptOnCallback (file, false);
    
    /*
		 * For Firefox/Mozilla, unexisted *.js will result in errors.
		 */
		script.onerror = ClazzLoader.generatingW3CScriptOnCallback (file, true);
    if (ClazzLoader.isOpera) {
			ClazzLoader.needOnloadCheck = true;
		}
	} else { // IE
		ClazzLoader.needOnloadCheck = true;
		script.onreadystatechange = ClazzLoader.generatingIEScriptOnCallback (file);
	}
	ClazzLoader.inLoadingThreads++;
	//alert("threads:"+ClazzLoader.inLoadingThreads);
	// Add script DOM element to document tree
	head.appendChild (script);
	ClazzLoader.scriptLoading (file);
};

/* protected */
ClazzLoader.isResourceExisted = function (id, path, base) {
	if (id != null && document.getElementById (id) != null) {
		return true;
	}
	if (path != null) {
		var key = path;
		if (base != null) {
			if (path.indexOf (base) == 0) {
				key = path.substring (base.length);
			}
		}
		if (path.lastIndexOf (".css") == path.length - 4) {
			var resLinks = document.getElementsByTagName ("LINK");
			for (var i = 0; i < resLinks.length; i++) {
				var cssPath = resLinks[i].href;
				var idx = cssPath.lastIndexOf (key);
				if (idx != -1 && idx == cssPath.length - key.length) {
					return true;
				}
			}
			if (window["css." + id] == true) {
				return true;
			}
		} else if (path.lastIndexOf (".js") == path.length - 4) {
			var resScripts = document.getElementsByTagName ("SCRIPT");
			for (var i = 0; i < resScripts.length; i++) {
				var jsPath = resScripts[i].src;
				var idx = jsPath.lastIndexOf (key);
				if (idx != -1 && idx == jsPath.length - key.length) {
					return true;
				}
			}
		}
	}
	return false;
};

/* private */
/*-# queueBe4SWT -> q4T #-*/
ClazzLoader.queueBe4SWT = [];

/* private */
/*-# lockQueueBe4SWT -> l4T #-*/
ClazzLoader.lockQueueBe4SWT = true;

/* private */
/*-# isLoadingEntryClass -> lec #-*/
ClazzLoader.isLoadingEntryClass = true;

/* private */
/*-# besidesJavaPackage -> bJP #-*/
ClazzLoader.besidesJavaPackage = false;

/**
 * After class is loaded, this method will be executed to check whether there
 * are classes in the dependency tree that need to be loaded.
 */
/* private */
/*-# tryToLoadNext -> next #-*/
ClazzLoader.tryToLoadNext = function (file) {
	/*
	 * Try to check whether current status is in SWT lazy loading mode. If
	 * yes, try to keep ClazzLoader#tryToLoadNext in queue and wait until
	 * all SWT core is loaded.
	 */
	if (ClazzLoader.lockQueueBe4SWT && ClazzLoader.pkgRefCount != 0 
			&& file.lastIndexOf ("package.js") != file.length - 10
			&& !ClazzLoader.isOpera) { // No Opera! Opera is in single thread.
		var qbs = ClazzLoader.queueBe4SWT;
		qbs[qbs.length] = file;
		return;
	}

	var node = ClazzLoader.mapPath2ClassNode["@" + file];
	if (node == null) { // maybe class tree root
		//error (" null node ?" + file);
		return;
	}
	var clazzes = ClazzLoader.classpathMap["$" + file];
	if (clazzes != null) {
		for (var i = 0; i < clazzes.length; i++) {
			var nm = clazzes[i];
			if (nm != node.name) {
				var n = ClazzLoader.findClass (nm);
				if (n != null) {
				//*
					if (n.status < ClazzNode.STATUS_CONTENT_LOADED) {
						n.status = ClazzNode.STATUS_CONTENT_LOADED;
						ClazzLoader.updateNode (n);
					}
				//*/
				//ClazzLoader.tryToLoadNext (n.path);
				} else {
					n = new ClazzNode ();
					n.name = nm;
					var pp = ClazzLoader.classpathMap["#" + nm];
					if (pp == null) {
						alert (nm);
						error ("Java2Script implementation error! Please report this bug!");
					}
					//n.path = ClazzLoader.lastScriptPath;
					n.path = pp;
					//error ("..." + node.path + "//" + node.name);
					ClazzLoader.mappingPathNameNode (n.path, nm, n);
					n.status = ClazzNode.STATUS_CONTENT_LOADED;
					ClazzLoader.addChildClassNode(ClazzLoader.clazzTreeRoot, n, -1);
					ClazzLoader.updateNode (n);
				}
			}
		}
	}
	if (node instanceof Array) {
		//log ("array of node " + node.length + ">>>>" + file);
		/*
		for (var i = 0; i < node.length; i++) {
			//alert ("array of node " + node[i].name);
		}
		*/
		for (var i = 0; i < node.length; i++) {
			if (node[i].status < ClazzNode.STATUS_CONTENT_LOADED) {
				node[i].status = ClazzNode.STATUS_CONTENT_LOADED;
				//error ("updating array : " + node[i].name + "..");
				ClazzLoader.updateNode (node[i]);
			}
		}
	} else {
		if (node.status < ClazzNode.STATUS_CONTENT_LOADED) {
			var stillLoading = false;
			var ss = document.getElementsByTagName ("SCRIPT");
			for (var i = 0; i < ss.length; i++) {
				if (ClazzLoader.isIE) {
					if (ss[i].onreadystatechange != null && ss[i].onreadystatechange.path == node.path
							&& ss[i].readyState == "interactive") {
						stillLoading = true;
						break;
					}
				} else {
					if (ss[i].onload != null && ss[i].onload.path == node.path) {
						stillLoading = true;
						break;
					}
				}
			}
			if (!stillLoading) {
				node.status = ClazzNode.STATUS_CONTENT_LOADED;
				ClazzLoader.updateNode (node);
			}
		}
	}
	/*
	 * Maybe in #optinalLoaded inside above ClazzLoader#updateNode calls, 
	 * ClazzLoader.keepOnLoading is set false (Already loaded the wanted
	 * classes), so here check to stop.
	 */
	 
	if (!ClazzLoader.keepOnLoading) {
		return;
	}

	var loadFurther = false;
	var n = ClazzLoader.findNextMustClass (ClazzLoader.clazzTreeRoot, 
			ClazzNode.STATUS_KNOWN);
	//System.out.println(file + " next ..." + n) ;
	if (n != null) {
		//log ("next ..." + n.name);
		ClazzLoader.loadClassNode (n);
		while (ClazzLoader.inLoadingThreads < ClazzLoader.maxLoadingThreads) {
			var nn = ClazzLoader.findNextMustClass (ClazzLoader.clazzTreeRoot,
					ClazzNode.STATUS_KNOWN);
			if (nn == null) break;
			ClazzLoader.loadClassNode (nn); // will increase inLoadingThreads!
		}
	} else {
		var cq = ClazzLoader.classQueue;
		if (cq.length != 0) { 
			/* queue must be loaded in order! */
			n = cq[0]; // popup class from the queue
			//alert ("load from queue");
			//alert (cq.length + ":" + cq);
			for (var i = 0; i < cq.length - 1; i++) {
				cq[i] = cq[i + 1];
			}
			cq.length--;
			//log (cq.length + ":" + cq);
			if (!ClazzLoader.loadedScripts[n.path] || cq.length != 0 
					|| !ClazzLoader.isLoadingEntryClass
					|| (n.musts != null && n.musts.length != 0)
					|| (n.optionals != null && n.optionals.length != 0)/*
					|| window["org.eclipse.swt.registered"] != null*/) {
				ClazzLoader.addChildClassNode(ClazzLoader.clazzTreeRoot, n, 1);
				ClazzLoader.loadScript (n.path, n.requiredBy);
				//alert("part1")
			} else {
				if (ClazzLoader.isLoadingEntryClass) {
					/*
					 * The first time reaching here is the time when ClassLoader
					 * is trying to load entry class. Class with #main method and
					 * is to be executed is called Entry Class.
					 *
					 * Here when loading entry class, ClassLoader should not call
					 * the next following loading script. This is because, those
					 * scripts will try to mark the class as loaded directly and
					 * then continue to call #optionalsLoaded callback method,
					 * which results in an script error!
					 */
					ClazzLoader.isLoadingEntryClass = false;
				}
				//alert ("Continue loading by SCRIPT onload event!");
				//ClazzLoader.addChildClassNode(ClazzLoader.clazzTreeRoot, n, 1);
				//ClazzLoader.loadScript (n.path);
			}
		} else { // Optionals
			n = ClazzLoader.findNextOptionalClass (ClazzNode.STATUS_KNOWN);
			//log ("options " + file);
			if (n != null) {
				//System.out.println("in optionals unknown..." + file + " " + n.name + " " + ClazzLoader.inLoadingThreads + "/" + ClazzLoader.maxLoadingThreads);
				ClazzLoader.loadClassNode (n);
				while (ClazzLoader.inLoadingThreads < ClazzLoader.maxLoadingThreads) {
					var nn = ClazzLoader.findNextOptionalClass (ClazzNode.STATUS_KNOWN);
					//log ("in second loading " + nn);
					if (nn == null) break;
				//System.out.println("in optionals unknown2..." + file + " " + nn.name);
					ClazzLoader.loadClassNode (nn); // will increase inLoadingThreads!
				}
			} else {
        //System.out.println("no optionals");
				loadFurther = true;
			}
		}
	}
		//log("test3 " + loadFurther + " " + ClazzLoader.inLoadingThreads )
	/*
	 * The following codes still need more tests, e.g. cyclic tests.
	 * And they also need optimization.
	 */
	if (loadFurther && ClazzLoader.inLoadingThreads == 0) {
		while ((n = ClazzLoader.findNextMustClass (ClazzLoader.clazzTreeRoot, ClazzNode.STATUS_CONTENT_LOADED)) != null) {
			ClazzLoader.updateNode (n);
		}
		var lastNode = null;
		while ((n = ClazzLoader.findNextOptionalClass (ClazzNode.STATUS_CONTENT_LOADED)) != null) {
			if (lastNode === n) { // Already existed cycle ?
				n.status = ClazzNode.STATUS_OPTIONALS_LOADED;
			}
			ClazzLoader.updateNode (n);
			lastNode = n;
		}
		while (true) {
			ClazzLoader.tracks = new Array ();
			if (!ClazzLoader.checkOptionalCycle (ClazzLoader.clazzTreeRoot)) {
				break;
			}
		}
		lastNode = null;
		while ((n = ClazzLoader.findNextMustClass (ClazzLoader.clazzTreeRoot, ClazzNode.STATUS_DECLARED)) != null) {
			if (lastNode === n) break;
			ClazzLoader.updateNode (n);
			lastNode = n;
		}
		lastNode = null;
		while ((n = ClazzLoader.findNextOptionalClass (ClazzNode.STATUS_DECLARED)) != null) {
			if (lastNode === n) break;
			ClazzLoader.updateNode (n);
			lastNode = n;
		}
		var dList = [];
		while ((n = ClazzLoader.findNextMustClass (ClazzLoader.clazzTreeRoot, ClazzNode.STATUS_DECLARED)) != null) {
			dList[dList.length] = n;
			n.status = ClazzNode.STATUS_OPTIONALS_LOADED;
		}
		while ((n = ClazzLoader.findNextOptionalClass (ClazzNode.STATUS_DECLARED)) != null) {
			dList[dList.length] = n;
			n.status = ClazzNode.STATUS_OPTIONALS_LOADED;
		}
		for (var i = 0; i < dList.length; i++) {
			// ClazzLoader.updateNode (dList[i]);
			ClazzLoader.destroyClassNode (dList[i]); // Same as above
		}
		for (var i = 0; i < dList.length; i++) {
			var optLoaded = dList[i].optionalsLoaded;
			if (optLoaded != null) {
				dList[i].optionalsLoaded = null;
				//window.setTimeout (optLoaded, 25);
				optLoaded ();
			}
		}
		/*
		 * It seems ClazzLoader#globalLoaded is seldom overrided.
		 */
		ClazzLoader.globalLoaded ();
		//error ("end ?");
	}
};

ClazzLoader.tracks = new Array ();

/*
 * There are classes reference cycles. Try to detect and break those cycles.
 * TODO: Reference cycles should be broken down carefully. Or there will be 
 * bugs!
 */
/* protected */
ClazzLoader.checkOptionalCycle = function (node) {
	var ts = ClazzLoader.tracks;
	var length = ts.length;
	var cycleFound = -1;
	for (var i = 0; i < ts.length; i++) {
		if (ts[i] === node && ts[i].status >= ClazzNode.STATUS_DECLARED) { 
			// Cycle is found;
			cycleFound = i;
			break;
		}
	}
	ts[ts.length] = node;
	if (cycleFound != -1) {
		/*
		for (var i = cycleFound; i < ts.length; i++) {
			//alert (ts[i].name + ":::" + ts[i].status);
		}
		//alert ("===");
		*/
		for (var i = cycleFound; i < ts.length; i++) {
			ts[i].status = ClazzNode.STATUS_OPTIONALS_LOADED;
			//ClazzLoader.updateNode (ts[i]);
			ClazzLoader.destroyClassNode (ts[i]); // Same as above
			for (var k = 0; k < ts[i].parents.length; k++) {
				//log ("updating parent ::" + ts[i].parents[k].name);
				ClazzLoader.updateNode (ts[i].parents[k]);
			}
			ts[i].parents = new Array ();
			var optLoaded = ts[i].optionalsLoaded;
			if (optLoaded != null) {
				ts[i].optionalsLoaded = null;
				//alert ("check cycle.");
				//window.setTimeout (optLoaded, 25);
				optLoaded ();
			}
		}
		ts.length = 0;
		return true;
	}
	for (var i = 0; i < node.musts.length; i++) {
		if (node.musts[i].status == ClazzNode.STATUS_DECLARED) {
			if (ClazzLoader.checkOptionalCycle (node.musts[i])) {
				return true;
			}
		}
	}
	for (var i = 0; i < node.optionals.length; i++) {
		if (node.optionals[i].status == ClazzNode.STATUS_DECLARED) {
			if (ClazzLoader.checkOptionalCycle (node.optionals[i])) {
				return true;
			}
		}
	}
	ts.length = length;
	return false;
};


/**
 * Update the dependency tree nodes recursively.
 */
/* private */
/*-# 
 # updateNode -> uN 
 #
 # isMustsOK -> mOK
 # isOptionsOK -> oOK
 #-*/
ClazzLoader.updateNode = function (node) {
	if (node.name == null 
			|| node.status >= ClazzNode.STATUS_OPTIONALS_LOADED) {
    //System.out.println("destroying node " + node.name + " " + node.path)
		ClazzLoader.destroyClassNode (node);
		return;
	}
	var isMustsOK = false;
	if (node.musts == null || node.musts.length == 0 
			|| node.declaration == null) {
		isMustsOK = true;
	} else {
		isMustsOK = true;
		var mustLength = node.musts.length;
		for (var i = mustLength - 1; i >= 0; i--) {

			/*
			 * Soheil reported a strange bug:
The problem here is that, Widget functions and field are not added to
the Control !
Control is not an instance of Widget! I got an error that says
this.checkOrientation is not a function ( in Control's constructor).

When I changed the order of Drawable and Widget in the definition of
Control's constructor, the line bellow, it works fine!
$_L(["$wt.graphics.Drawable","$wt.widgets.Widget"],"$wt.widgets.Control",
... : has the error
$_L(["$wt.widgets.Widget","$wt.graphics.Drawable"],"$wt.widgets.Control", 
... : does not have the error 
			 *
			 * In the bug fix procedure, it's known that node.musts will
			 * be changed according to the later codes:
			 * ClazzLoader.updateNode (n); // (see about 20 lines below)
			 * 
			 * As node.musts may become smaller, node.musts should be 
			 * traversed in reverse order, so all musts are checked.
			 *
			 * TODO:
			 */
			var n = node.musts[i];
			n.requiredBy = node;
			if (n.status < ClazzNode.STATUS_DECLARED) {
				if (ClazzLoader.isClassDefined (n.name)) {
					var nns = new Array (); // for optional loaded events!

    //System.out.println("destroying node2 " + n.name + " " + n.path)

					n.status = ClazzNode.STATUS_OPTIONALS_LOADED;
					// ClazzLoader.updateNode (n); // musts may be changed
					ClazzLoader.destroyClassNode (n); // Same as above
					/*
					 * For those classes within one *.js file, update
					 * them synchronously.
					 */
					if (n.declaration != null 
							&& n.declaration.clazzList != null) {
						var list = n.declaration.clazzList;
						for (var j = 0; j < list.length; j++) {
							var nn = ClazzLoader.findClass (list[j]);
							if (nn.status != ClazzNode.STATUS_OPTIONALS_LOADED
									&& nn !== n) {
								nn.status = n.status;
								nn.declaration = null;
								// ClazzLoader.updateNode (nn);
								// Same as above
    //System.out.println("destroying node3 " + nn.name + " " + nn.path)

								ClazzLoader.destroyClassNode (nn);
								if (nn.optionalsLoaded != null) {
									nns[nns.length] = nn;
								}
							}
						}
						n.declaration = null;
					}
					if (n.optionalsLoaded != null) {
						nns[nns.length] = n;
					}
					for (var j = 0; j < nns.length; j++) {
						var optLoaded = nns[j].optionalsLoaded;
						if (optLoaded != null) {
							nns[j].optionalsLoaded = null;
							//window.setTimeout (optLoaded, 25);
							optLoaded ();
						}
					}
				} else { // why not break? -Zhou Renjian @ Nov 28, 2006
					if (n.status == ClazzNode.STATUS_CONTENT_LOADED) {
						// lazy loading script doesn't work! - 2/26/2007
						ClazzLoader.updateNode (n); // musts may be changed
					}
					if (n.status < ClazzNode.STATUS_DECLARED) {
						isMustsOK = false;
					}
				}
				// fix the above strange bug
				if (node.musts.length != mustLength) {
					// length changed!
					mustLength = node.musts.length;
					i = mustLength; // -1
					isMustsOK = true;
				}
			}
		}
	}
	/*
	var scripts = document.getElementsByTagName ("SCRIPT");
	var count = 0;
	for (var i = 0; i < scripts.length; i++) {
		var s = scripts[i];
		if (s.onload != null) {
			log ("---:---" + s.src);
			count++;
		}
	}
	//alert ("There are " + count + " script loading ...");
	*/
  //System.out.println("testing " + node.name + " " + isMustsOK + " " + node.status + " " +  ClazzNode.STATUS_DECLARED + "isnull?" + (node.declaration == null))

	if (isMustsOK) {
		if (node.status < ClazzNode.STATUS_DECLARED) {
			var decl = node.declaration;
			if (decl != null) {
				if (decl.executed == false) {
					decl ();
					decl.executed = true;
				} else {
          decl ();
				}
			}
			node.status = ClazzNode.STATUS_DECLARED;
			if (ClazzLoader.definedClasses != null) {
				ClazzLoader.definedClasses[node.name] = true;
			}
			ClazzLoader.scriptInited (node.path);
					/*
					 * For those classes within one *.js file, update
					 * them synchronously.
					 */
					if (node.declaration != null 
							&& node.declaration.clazzList != null) {
						var list = node.declaration.clazzList;
						for (var j = 0; j < list.length; j++) {
							var nn = ClazzLoader.findClass (list[j]);
							if (nn.status != ClazzNode.STATUS_DECLARED
									&& nn !== node) {
			nn.status = ClazzNode.STATUS_DECLARED;
			if (ClazzLoader.definedClasses != null) {
				ClazzLoader.definedClasses[nn.name] = true;
			}
			ClazzLoader.scriptInited (nn.path);
							}
						}
					}
		}
		var level = ClazzNode.STATUS_DECLARED;
		var isOptionsOK = false;
		
		if (((node.optionals == null || node.optionals.length == 0) 
				&& (node.musts == null || node.musts.length == 0))
				|| (node.status > ClazzNode.STATUS_KNOWN 
				&& node.declaration == null)) {
			isOptionsOK = true;
		} else {
			isOptionsOK = true;
			

			for (var i = 0; i < node.musts.length; i++) {
				var n = node.musts[i];
				if (n.status < ClazzNode.STATUS_OPTIONALS_LOADED) {
					isOptionsOK = false;
					break;
				}
			}
			if (isOptionsOK) {
				for (var i = 0; i < node.optionals.length; i++) {
					var n = node.optionals[i];
					if (n.status < ClazzNode.STATUS_OPTIONALS_LOADED) {
						isOptionsOK = false;
						break;
					}
				}
			}
		}
		if (isOptionsOK) {
			level = ClazzNode.STATUS_OPTIONALS_LOADED;
			node.status = level;
			ClazzLoader.scriptCompleted (node.path);
			var optLoaded = node.optionalsLoaded;
			if (optLoaded != null) {
				node.optionalsLoaded = null;
				//window.setTimeout (optLoaded, 25);
				optLoaded ();
				if (!ClazzLoader.keepOnLoading) {
					return false;
				}
			}
			ClazzLoader.destroyClassNode (node);
					/*
					 * For those classes within one *.js file, update
					 * them synchronously.
					 */
					if (node.declaration != null 
							&& node.declaration.clazzList != null) {
						var list = node.declaration.clazzList;
						for (var j = 0; j < list.length; j++) {
							var nn = ClazzLoader.findClass (list[j]);
							if (nn.status != level && nn !== node) {
			nn.status = level;
			nn.declaration = null;
			ClazzLoader.scriptCompleted (nn.path);
			var optLoaded = nn.optionalsLoaded;
			if (optLoaded != null) {
				nn.optionalsLoaded = null;
				//window.setTimeout (optLoaded, 25);
				optLoaded ();
				if (!ClazzLoader.keepOnLoading) {
					return false;
				}
			}
			ClazzLoader.destroyClassNode (node);
							}
						}
					}
		}
		ClazzLoader.updateParents (node, level);
	}
};

/* private */
/*-# updateParents -> uP #-*/
ClazzLoader.updateParents = function (node, level) {
	if (node.parents == null || node.parents.length == 0) {
		return;
	}
	for (var i = 0; i < node.parents.length; i++) {
		var p = node.parents[i];
		if (p.status >= level) {
			continue;
		}
		ClazzLoader.updateNode (p);
	}
	if (level == ClazzNode.STATUS_OPTIONALS_LOADED) {
		node.parents = new Array ();
	}
};

/* private */
/*-# findNextMustClass -> fNM #-*/
ClazzLoader.findNextMustClass = function (node, status) {
	if (node != null) {
		/*
		if (ClazzLoader.isClassDefined (node.name)) {
			node.status = ClazzNode.STATUS_OPTIONALS_LOADED;
		}
		*/
		if (node.musts != null && node.musts.length != 0) {
			for (var i = 0; i < node.musts.length; i++) {
				var n = node.musts[i];
				if (n.status == status && (status != ClazzNode.STATUS_KNOWN 
						|| ClazzLoader.loadedScripts[n.path] != true)
						&& (status == ClazzNode.STATUS_DECLARED
						|| !ClazzLoader.isClassDefined (n.name))) {
					return n;
				} else {
					var nn = ClazzLoader.findNextMustClass (n, status);
					if (nn != null) {
						return nn;
					}
				}
			}
		}
		if (node.status == status && (status != ClazzNode.STATUS_KNOWN 
				|| ClazzLoader.loadedScripts[node.path] != true)
				&& (status == ClazzNode.STATUS_DECLARED
				|| !ClazzLoader.isClassDefined (node.name))) {
			return node;
		}
	}
	return null;
};

/*
 * Be used to record already used random numbers. And next new random
 * number should not be in the property set.
 */
/* private */
/*-# usedRandoms -> Rms #-*/
ClazzLoader.usedRandoms = {};
ClazzLoader.usedRandoms["r" + 0.13412] = 0.13412;

/* private */
/*-# findNextOptionalClass -> fNO #-*/
ClazzLoader.findNextOptionalClass = function (status) {
	var rnd = 0;
	while (true) { // try to generate a never used random number
		rnd = Math.random ();
		var s = "r" + rnd;
		if (ClazzLoader.usedRandoms[s] != rnd) {
			ClazzLoader.usedRandoms[s] = rnd;
			break;
		}
	}
	ClazzLoader.clazzTreeRoot.random = rnd;
	var node = ClazzLoader.clazzTreeRoot;
	return ClazzLoader.findNodeNextOptionalClass (node, status);
};

/* private */
/*-# findNodeNextOptionalClass -> fNNO #-*/
ClazzLoader.findNodeNextOptionalClass = function (node, status) {
	var rnd = ClazzLoader.clazzTreeRoot.random;
	// search musts first
	if (node.musts != null && node.musts.length != 0) {
		var n = ClazzLoader.searchClassArray (node.musts, rnd, status);
		if (n != null && (status != ClazzNode.STATUS_KNOWN 
				|| ClazzLoader.loadedScripts[n.path] != true)
				&& (status == ClazzNode.STATUS_DECLARED
				|| !ClazzLoader.isClassDefined (n.name))) {
			return n;
		}
	}
	// search optionals second
	if (node.optionals != null && node.optionals.length != 0) {
		var n = ClazzLoader.searchClassArray (node.optionals, rnd, status);
		if (n != null && (status != ClazzNode.STATUS_KNOWN 
				|| ClazzLoader.loadedScripts[n.path] != true)
				&& (status == ClazzNode.STATUS_DECLARED
				|| !ClazzLoader.isClassDefined (n.name))) {
			return n;
		}
	}
	// search itself
	if (node.status == status && (status != ClazzNode.STATUS_KNOWN 
			|| ClazzLoader.loadedScripts[node.path] != true)
			&& (status == ClazzNode.STATUS_DECLARED
			|| !ClazzLoader.isClassDefined (node.name))) {
		return node;
	}
	return null;
};

/* private */
ClazzLoader.searchClassArray = function (arr, rnd, status) {
	for (var i = 0; i < arr.length; i++) {
		var n = arr[i];
		if (n.status == status && (status != ClazzNode.STATUS_KNOWN 
				|| ClazzLoader.loadedScripts[n.path] != true)
				&& (status == ClazzNode.STATUS_DECLARED
				|| !ClazzLoader.isClassDefined (n.name))) {
			return n;
		} else {
			if (n.random == rnd) {
				continue;
			}
			n.random = rnd; // mark as visited!
			var nn = ClazzLoader.findNodeNextOptionalClass (n, status);
			if (nn != null) {
				return nn;
			}
		}
	}
	return null;
};

/**
 * This map variable is used to mark that *.js is correctly loaded.
 * In IE, ClazzLoader has defects to detect whether a *.js is correctly
 * loaded or not, so inner loading mark is used for detecting.
 */
/* private */
/*-# innerLoadedScripts -> ilss #-*/
ClazzLoader.innerLoadedScripts = {};

/**
 * This variable is used to keep the latest interactive SCRIPT element.
 */
/* private */
/*-# interactiveScript -> itst #-*/
ClazzLoader.interactiveScript = null;

/**
 * IE and Firefox/Mozilla are different in using <SCRIPT> tag to load *.js.
 */
/* private */
ClazzLoader.needOnloadCheck = false;

/**
 * Check the interactive status of SCRIPT elements to determine whether a
 * *.js file is correctly loaded or not.
 *
 * Only make senses for IE.
 */
/* protected */
ClazzLoader.checkInteractive = function () {
	//alert ("checking...");
	if (!ClazzLoader.needOnloadCheck) {
		return;
	}
	var is = ClazzLoader.interactiveScript;
	if (is != null && is.readyState == "interactive") { // IE
		return;
	}
	ClazzLoader.interactiveScript = null;
	var ss = document.getElementsByTagName ("SCRIPT");
	for (var i = 0; i < ss.length; i++) {
		if (ss[i].readyState == "interactive"
				&& ss[i].onreadystatechange != null) { // IE
			ClazzLoader.interactiveScript = ss[i];
			ClazzLoader.innerLoadedScripts[ss[i].src] = true;
		} else if (ClazzLoader.isOpera) { // Opera
			if (ss[i].readyState == "loaded" 
					&& ss[i].src != null && ss[i].src.length != 0) {
				ClazzLoader.innerLoadedScripts[ss[i].src] = true;
			}
		}
	}
};

/**
 * This method will be called in almost every *.js generated by Java2Script
 * compiler.
 */
/* protected */
ClazzLoader.load = function (musts, clazz, optionals, declaration) {
	
	ClazzLoader.checkInteractive ();

	if (clazz instanceof Array) {
		ClazzLoader.unwrapArray (clazz);
		for (var i = 0; i < clazz.length; i++) {
			ClazzLoader.load (musts, clazz[i], optionals, declaration, clazz);
		}
		return;
	}
	if (clazz.charAt (0) == '$') {
		clazz = "org.eclipse.s" + clazz.substring (1);
	}
	
	var node = ClazzLoader.mapPath2ClassNode["#" + clazz];
	
		//System.out.println("Loading " + clazz + " ... " + node);

	if (node == null) { // ClazzLoader.load called inside *.z.js?
		var n = ClazzLoader.findClass (clazz);
		if (n != null) {
			node = n;
		} else {
      xxxbhtest += clazz + ";" + declaration + "\n"
			node = new ClazzNode ();
		}
		node.name = clazz;
		var pp = ClazzLoader.classpathMap["#" + clazz];
		if (pp == null) { // TODO: Remove this test in final release
			//alert ("error finding classpathMap for " + clazz + " " );
			//error ("Java2Script implementation error! Please report this bug!");
      pp = "unknown"
		}
		//node.path = ClazzLoader.lastScriptPath;
		node.path = pp;
		//error ("..." + node.path + "//" + node.name);
		ClazzLoader.mappingPathNameNode (node.path, clazz, node);
		node.status = ClazzNode.STATUS_KNOWN;
		ClazzLoader.addChildClassNode(ClazzLoader.clazzTreeRoot, node, -1);
		//log (clazz);
		//alert ("[Java2Script] ClazzLoader#load is not executed correctly!");
		//*/
		/*
		if (declaration != null) {
			declaration ();
		}
		//alert ("[Java2Script] ClazzLoader#load is not executed correctly!");
		return;
		//*/
	}
	var okToInit = true;
	if (musts != null && musts.length != 0) {
		ClazzLoader.unwrapArray (musts);
		for (var i = 0; i < musts.length; i++) {
			var name = musts[i];
			if (name == null || name.length == 0) {
				continue;
			}
			//System.out.println(node.name + " must have " + name)
			if (ClazzLoader.isClassDefined (name)
					|| ClazzLoader.isClassExcluded (name)) {
			//System.out.println("which it does")
				continue;
			}
			okToInit = false;
			var n = ClazzLoader.findClass (name);
			if (n == null) {
  			//System.out.println(clazz + " requires " + name)
				n = new ClazzNode ();
				n.name = musts[i];
				n.status = ClazzNode.STATUS_KNOWN;
			}
			n.requiredBy = node;
			ClazzLoader.addChildClassNode (node, n, 1);
		}
	}

	/*
	 * The following lines are commented intentionally.
	 * So lots of class is not declared until there is a must?
	 *
	 * TODO: Test the commented won't break up the dependency tree.
	 */
	/*
	if (okToInit) {
		declaration ();
		node.declaration = null;
		node.status = ClazzNode.STATUS_DECLARED;
	} else {
		node.declaration = declaration;
	}
	*/
	if (arguments.length == 5 && declaration != null) {
		declaration.status = node.status;
		declaration.clazzList = arguments[4];
	}
	node.declaration = declaration;
	if (declaration != null) {
    //System.out.println("declaration found for " + node.name + " " + node.path)
		node.status = ClazzNode.STATUS_CONTENT_LOADED;
	}

	var isOptionalsOK = true;
	if (optionals != null && optionals.length != 0) {
		ClazzLoader.unwrapArray (optionals);
		for (var i = 0; i < optionals.length; i++) {
			var name = optionals[i];
			if (name == null || name.length == 0) {
				continue;
			}
			if (ClazzLoader.isClassDefined (name) 
					|| ClazzLoader.isClassExcluded (name)) {
				continue;
			}
			isOptionalsOK = false;
			var n = ClazzLoader.findClass (name);
			if (n == null) {
				n = new ClazzNode ();
				n.name = optionals[i];
				n.status = ClazzNode.STATUS_KNOWN;
			}
			ClazzLoader.addChildClassNode (node, n, -1);
		}
	}
};

/*
 * Try to be compatiable of Clazz
 */
if (window["Clazz"] != null) {
	Clazz.load = ClazzLoader.load;
	if (window["$_L"] != null) {
		$_L = Clazz.load;
	}
}

/**
 *
 */
/* protected */
/*-# findClass -> fC #-*/
ClazzLoader.findClass = function (clazzName) {
	var rnd = 0;
	while (true) { // try to generate a never used random number
		rnd = Math.random ();
		var s = "r" + rnd;
		if (ClazzLoader.usedRandoms[s] != rnd) {
			ClazzLoader.usedRandoms[s] = rnd;
			break;
		}
	}
	ClazzLoader.clazzTreeRoot.random = rnd;
	return ClazzLoader.findClassUnderNode (clazzName, 
			ClazzLoader.clazzTreeRoot);
};

/* private */
/*-# findClassUnderNode -> fCU #-*/
ClazzLoader.findClassUnderNode = function (clazzName, node) {
	var rnd = ClazzLoader.clazzTreeRoot.random;
	if (node.name == clazzName) {
		return node;
	}
	// musts first
	for (var i = 0; i < node.musts.length; i++) {
		var n = node.musts[i];
		if (n.name == clazzName) {
			return n;
		}
		if (n.random == rnd) {
			continue;
		}
		n.random = rnd;
		var nn = ClazzLoader.findClassUnderNode (clazzName, n);
		if (nn != null) {
			return nn;
		}
	}
	// optionals last
	for (var i = 0; i < node.optionals.length; i++) {
		var n = node.optionals[i];
		if (n.name == clazzName) {
			return n;
		}
		if (n.random == rnd) {
			continue;
		}
		n.random = rnd;
		var nn = ClazzLoader.findClassUnderNode (clazzName, n);
		if (nn != null) {
			return nn;
		}
	}
	return null;
};

/**
 * Map different class to the same path! Many classes may be packed into
 * a *.z.js already.
 *
 * @path *.js path
 * @name class name
 * @node ClazzNode object
 */
/* private */
/*-# mappingPathNameNode -> mpp #-*/
ClazzLoader.mappingPathNameNode = function (path, name, node) {
	var map = ClazzLoader.mapPath2ClassNode;
	var keyPath = "@" + path;
	var v = map[keyPath];
	if (v != null) {
		if (v instanceof Array) {
			var existed = false;
			for (var i = 0; i < v.length; i++) {
				if (v[i].name == name) {
					existed = true;
					break;
				}
			}
			if (!existed) {
				v[v.length] = node;
			}
		} else {
			var arr = new Array ();
			arr[0] = v;
			arr[1] = node;
			map[keyPath] = arr;
		}
	} else {
		map[keyPath] = node;
	}
	map["#" + name] = node;
};

/* protected */
/*-# loadClassNode -> lCN #-*/
ClazzLoader.loadClassNode = function (node) {
	var name = node.name;
	if (!ClazzLoader.isClassDefined (name) 
			&& !ClazzLoader.isClassExcluded (name)) {
		var path = ClazzLoader.getClasspathFor (name/*, true*/);
		node.path = path;
		ClazzLoader.mappingPathNameNode (path, name, node);
		if (!ClazzLoader.loadedScripts[path]) {
			ClazzLoader.loadScript (path, node.requiredBy);
			return true;
		}
	}
	return false;
};


/* protected */
ClazzLoader.runtimeKeyClass = "java.lang.String";

/**
 * Queue used to store classes before key class is loaded.
 */
/* private */
ClazzLoader.queueBe4KeyClazz = new Array ();

/**
 * Return J2SLib base path from existed SCRIPT src attribute.
 */
/* private */
/*-# getJ2SLibBase -> gLB #-*/
ClazzLoader.getJ2SLibBase = function () {
	var o = window["j2s.lib"];
	if (o != null) {
		if (o.base == null) {
			o.base = "http://archive.java2script.org/";
		}
		return o.base + (o.alias == "." ? "" : (o.alias ? o.alias : (o.version ? o.version : "1.0.0")) + "/");
	}
	var ss = document.getElementsByTagName ("SCRIPT");
	for (var i = 0; i < ss.length; i++) {
		var src = ss[i].src;
		var idx = src.indexOf ("j2slib.z.js"); // consider it as key string!
		if (idx != -1) {
			return src.substring (0, idx);
		}
		idx = src.indexOf ("j2slibcore.z.js"); // consider it as key string!
		if (idx != -1) {
			return src.substring (0, idx);
		}
		var base = ClazzLoader.classpathMap["@java"];
		if (base != null) {
			return base;
		}
		idx = src.indexOf ("java/lang/ClassLoader.js"); // may be not packed yet
		if (idx != -1) {
			return src.substring (0, idx);
		}
	}
	return null;
};

/* private static */
/*-# J2SLibBase -> JLB #-*/
ClazzLoader.J2SLibBase = null;
/*-# fastGetJ2SLibBase -> fgLB #-*/
ClazzLoader.fastGetJ2SLibBase = function () {
	if (ClazzLoader.J2SLibBase == null) {
		ClazzLoader.J2SLibBase = ClazzLoader.getJ2SLibBase ();
	}
	return ClazzLoader.J2SLibBase;
};

/*
 * Check whether given package's classpath is setup or not.
 * Only "java" and "org.eclipse.swt" are accepted in argument.
 */
/* private */
/*-# assurePackageClasspath -> acp #-*/
ClazzLoader.assurePackageClasspath = function (pkg) {
	var r = window[pkg + ".registered"];
	if (r != false && r != true && ClazzLoader.classpathMap["@" + pkg] == null) {
		window[pkg + ".registered"] = false;
		var base = ClazzLoader.fastGetJ2SLibBase ();
		if (base == null) {
			base = "j2s"; 
		}
		ClazzLoader.packageClasspath (pkg, base, true);
	}
};

/**
 * Load the given class ant its related classes.
 */
/* public */
ClazzLoader.loadClass = function (name, optionalsLoaded, forced, async) {

 	if (typeof optionalsLoaded == "boolean") {
		return Clazz.evalType (name);
	}

	/*
	 * Make sure that
	 * ClazzLoader.packageClasspath ("java", base, true); 
	 * is called before any ClazzLoader#loadClass is called.
	 */
	ClazzLoader.assurePackageClasspath ("java");
	ClazzLoader.assurePackageClasspath ("core"); // JSmol uses j2s/core, not j2s/java for package.js

	var swtPkg = "org.eclipse.swt";
	if (name.indexOf (swtPkg) == 0 || name.indexOf ("$wt") == 0) {
		ClazzLoader.assurePackageClasspath (swtPkg);
	}
	if (name.indexOf ("junit") == 0) {
		ClazzLoader.assurePackageClasspath ("junit");
	}

	/*
	 * Any ClazzLoader#loadClass calls will be queued until java.* core classes
	 * is loaded.
	 */
	ClazzLoader.keepOnLoading = true;
	if (!forced && (
       ClazzLoader.pkgRefCount != 0 && name.lastIndexOf (".package") != name.length - 8
		|| !ClazzLoader.isClassDefined (ClazzLoader.runtimeKeyClass) && name.indexOf ("java.") != 0
  )) {
		var qbs = ClazzLoader.queueBe4KeyClazz;
		//alert("keep on loading " + name)
		qbs[qbs.length] = [name, optionalsLoaded];
		
		return;
	}
	if (!ClazzLoader.isClassDefined (name) 
			&& !ClazzLoader.isClassExcluded (name)) {
		var path = ClazzLoader.getClasspathFor (name/*, true*/);
		var existed = ClazzLoader.loadedScripts[path];
		var qq = ClazzLoader.classQueue;
		if (!existed) {
			for (var i = qq.length - 1; i >= 0; i--) {
				if (qq[i].path == path || qq[i].name == name) {
					existed = true;
				}
			}
		}
		//alert("@#@#@#@# " + name);
		if (!existed) {
			var n = null;
			if (Clazz.unloadedClasses[name] != null) {
				n = ClazzLoader.findClass (name);
			}
			if (n == null) {
				n = new ClazzNode ();
			}
			n.name = name;
			n.path = path;
			ClazzLoader.mappingPathNameNode (path, name, n);
			n.optionalsLoaded = optionalsLoaded;
			n.status = ClazzNode.STATUS_KNOWN;
			/*-# needBeingQueued -> nQ #-*/
			var needBeingQueued = false;
			//error (qq.length + ":" + qq);
			//error (path);
			for (var i = qq.length - 1; i >= 0; i--) {
				if (qq[i].status != ClazzNode.STATUS_OPTIONALS_LOADED) {
					needBeingQueued = true;
					break;
				}
			}
			if (path.lastIndexOf ("package.js") == path.length - 10) {//forced
				// push class to queue
				var inserted = false;
				for (var i = qq.length - 1; i >= 0; i--) {
					var name = qq[i].name;
					if (name.lastIndexOf ("package.js") == name.length - 10) {
						qq[i + 1] = n;
						inserted = true;
						break;
					}
					qq[i + 1] = qq[i];
				}
				if (!inserted) {
					qq[0] = n;
				}
			} else if (needBeingQueued) {
				qq[qq.length] = n;
			}
//alert(["-------------"]);
			if (!needBeingQueued) { // can be loaded directly
				/*-# bakEntryClassLoading -> bkECL #-*/
				var bakEntryClassLoading = false;
				if (optionalsLoaded != null) {	
					bakEntryClassLoading = ClazzLoader.isLoadingEntryClass;
					ClazzLoader.isLoadingEntryClass = true;
				}
				ClazzLoader.addChildClassNode(ClazzLoader.clazzTreeRoot, n, 1);
				//System.out.println("Clazz loading " + n.path)
				ClazzLoader.loadScript (n.path);
				if (optionalsLoaded != null) {
					ClazzLoader.isLoadingEntryClass = bakEntryClassLoading;
				}
			}
		} else if (optionalsLoaded != null) {
			var n = ClazzLoader.findClass (name);
			if (n != null) {
				if (n.optionalsLoaded == null) {
					n.optionalsLoaded = optionalsLoaded;
				} else if (optionalsLoaded != n.optionalsLoaded) {
					n.optionalsLoaded = (function (oF, nF) {
						return function () {
							oF();
							nF();
						};
					}) (n.optionalsLoaded, optionalsLoaded);
				}
			}
		}
	} else if (optionalsLoaded != null && ClazzLoader.isClassDefined (name)) {
	
		var nn = ClazzLoader.findClass (name);
		
		if (nn == null || nn.status >= ClazzNode.STATUS_OPTIONALS_LOADED) {
			/*if (nn != null) {
				ClazzLoader.destroyClassNode (nn);
			}*/
			if (async) {
				window.setTimeout (optionalsLoaded, 25);
			} else {
				optionalsLoaded ();
			}
		} // else ... should be called later
	}
};

/**
 * Load the application by the given class name and run its static main method.
 */
/* public */
ClazzLoader.loadJ2SApp = function (clazz, args, loaded) {
	if (clazz == null) {
		return;
	}
	var clazzStr = clazz;
	if (clazz.charAt (0) == '$') {
		clazzStr = "org.eclipse.s" + clazz.substring (1);
	}
	var idx = -1;
	if ((idx = clazzStr.indexOf ("@")) != -1) {
		var path = clazzStr.substring (idx + 1);
		ClazzLoader.setPrimaryFolder (path); // TODO: No primary folder?
		clazzStr = clazzStr.substring (0, idx);
		idx = clazzStr.lastIndexOf (".");
		if (idx != -1) {
			var pkgName = clazzStr.substring (0, idx);
			ClazzLoader.packageClasspath (pkgName, path);
		}
	}
	var agmts = args;
	if (agmts == null || !(agmts instanceof Array)) {
		agmts = [];
	}
	var afterLoaded = loaded;
	if (afterLoaded == null) {
		afterLoaded = (function (clazzName, argv) {
			return function () {
				Clazz.evalType (clazzName).main (argv);
			};
		}) (clazzStr, agmts);
	} else {
		afterLoaded = loaded (clazzStr, agmts);
	}
	ClazzLoader.loadClass (clazzStr, afterLoaded);
};
/**
 * Load JUnit tests by the given class name.
 */
/* public */
ClazzLoader.loadJUnit = function (clazz, args) {
	var afterLoaded = function (clazzName, argv) {
		return function () {
			ClazzLoader.loadClass ("junit.textui.TestRunner", function () {
				junit.textui.TestRunner.run (Clazz.evalType (clazzName));
			});
		};
	};
	ClazzLoader.loadJ2SApp (clazz, args, afterLoaded);
};

/* private */
ClazzLoader.runtimeLoaded = function () {
	if (ClazzLoader.pkgRefCount != 0 
			|| !ClazzLoader.isClassDefined (ClazzLoader.runtimeKeyClass)) {
		return;
	}
	var qbs = ClazzLoader.queueBe4KeyClazz;
	for (var i = 0; i < qbs.length; i++) {
		ClazzLoader.loadClass (qbs[i][0], qbs[i][1]);
	}
	ClazzLoader.queueBe4KeyClazz = [];
	/*
	 * Should not set to empty function! Some later package may need this
	 * runtimeLoaded function. For example, lazily loading SWT package may
	 * require this runtimeLoaded function. 
	 * -- zhou renjian @ Dec 17, 2006
	 */
	// ClazzLoader.runtimeLoaded = function () {};
};

/*
 * Load those key *.z.js. This *.z.js will be surely loaded before other 
 * queued *.js.
 */
/* public */
ClazzLoader.loadZJar = function (zjarPath, keyClazz) {
	var keyClass = keyClazz;
	var isArr = (keyClazz instanceof Array);
	if (isArr) {
		keyClass = keyClazz[keyClazz.length - 1];
	}
	ClazzLoader.jarClasspath (zjarPath, isArr ? keyClazz
			: [keyClazz]);
	if (keyClazz == ClazzLoader.runtimeKeyClass) {
		ClazzLoader.loadClass (keyClass, ClazzLoader.runtimeLoaded, true);
	} else {
		ClazzLoader.loadClass (keyClass, null, true);
	}
};

ClazzLoader._nodeMap = {};
ClazzLoader._allNodes = [];
/**
 * The method help constructing the multiple-binary class dependency tree.
 */
/* private */
/*-# addChildClassNode -> addCCN #-*/
ClazzLoader.addChildClassNode = function (parent, child, type) {
	var existed = false;
	var arr = null;
  if (type == 1) {
		arr = parent.musts;
		if (!child.requiredBy)child.requiredBy = parent;
		if (!parent.requires){parent.requires = [];parent.requiresMap = {};}
		if (!parent.requiresMap[child.name]) {
			parent.requiresMap[child.name] = child;
			parent.requires.push[child];
		}
	} else {
		arr = parent.optionals;
	}
	if (!ClazzLoader._nodeMap[child.name]) {
		ClazzLoader._allNodes.push(child)
		ClazzLoader._nodeMap[child.name]=child
	}
	for (var i = 0; i < arr.length; i++) {
		if (arr[i].name == child.name) {
			existed = true;
			break;
		}
	}
	if (!existed) {
		/*
		if (type != 1) { // test cyclic optionals
			existed = false;
			for (var j = 0; j < child.optionals.length; j++) {
				if (child.optionals[j].name == parent.name) {
					existed = true;
					break;
				}
			}
		}
		*/
		/* above existed tests are commented */
		//if (!existed) {
			arr[arr.length] = child;
			var swtPkg = "org.eclipse.swt";
			if (child.name.indexOf (swtPkg) == 0 
					|| child.name.indexOf ("$wt") == 0) {
				window["swt.lazy.loading.callback"] = ClazzLoader.swtLazyLoading;
				ClazzLoader.assurePackageClasspath (swtPkg);
			}
			if (ClazzLoader.isLoadingEntryClass 
					&& child.name.indexOf ("java") != 0 
					&& child.name.indexOf ("net.sf.j2s.ajax") != 0) {
				if (ClazzLoader.besidesJavaPackage) {
					ClazzLoader.isLoadingEntryClass = false;
				}
				ClazzLoader.besidesJavaPackage = true;
			}
		//}
	}
	existed = false;
	for (var i = 0; i < child.parents.length; i++) {
		if (child.parents[i].name == parent.name) {
			existed = true;
			break;
		}
	}
	if (!existed && parent.name != null && parent != ClazzLoader.clazzTreeRoot
			&& parent != child) {
		child.parents[child.parents.length] = parent;
	}
};

/*
 * Some SWT classes may already skip ClazzLoader#tryToLoadNext when it's
 * detected that SWT is in lazy loading mode. Here it will try to re-execute
 * those ClazzLoader#tryToLoadNext
 */
/* private */
ClazzLoader.swtLazyLoading = function () {
	ClazzLoader.lockQueueBe4SWT = false;
	var qbs = ClazzLoader.queueBe4SWT;
	for (var i = 0; i < qbs.length; i++) {
		ClazzLoader.tryToLoadNext (qbs[i]);
	}
	ClazzLoader.queueBe4SWT  = [];
};

/* private */
ClazzLoader.removeFromArray = function (node, arr) {
	if (arr == null || node == null) {
		return false;
	}
	/*var isPackedJS = (node.path != null
			&& node.path.indexOf (".z.js") == node.path.length - 5);
	log ("... remove " + node.path + " :: " + isPackedJS);*/
	var j = 0;
	for (var i = 0; i < arr.length; i++) {
		if (!(arr[i] === node/* || (isPackedJS && arr[i].path == node.path)*/)) {
			if (j < i) {
				arr[j] = arr[i];
			}
			j++;
		}
	}
	arr.length = j;
	return false;
};

/* private */
/*-# destroyClassNode -> dCN #-*/
ClazzLoader.destroyClassNode = function (node) {
	//log (node.name + " // " + node.path);
	var parents = node.parents;
	if (parents != null) {
		for (var k = 0; k < parents.length; k++) {
			if (!ClazzLoader.removeFromArray (node, parents[k].musts)) {
				ClazzLoader.removeFromArray (node, parents[k].optionals);
			}
		}
	}
	/*
	if (node.optionalsLoaded != null) {
		node.optionalsLoaded ();
	}
	if (!ClazzLoader.removeFromArray (node, ClazzLoader.clazzTreeRoot.musts)) {
		ClazzLoader.removeFromArray (node, ClazzLoader.clazzTreeRoot.optionals);
	}
	*/
};

/* For hotspot and unloading */

/* protected */
ClazzLoader.unloadClassExt = function (qClazzName) {
	if (ClazzLoader.definedClasses != null) {
		ClazzLoader.definedClasses[qClazzName] = false;
	}
	if (ClazzLoader.classpathMap["#" + qClazzName] != null) {
		var pp = ClazzLoader.classpathMap["#" + qClazzName];
		ClazzLoader.classpathMap["#" + qClazzName] = null;
		var arr = ClazzLoader.classpathMap["$" + pp];
		var removed = false;
		for (var i = 0; i < arr.length; i++) {
			if (arr[i] == qClazzName) {
				for (var j = i; j < arr.length - 1; j++) {
					arr[j] = arr[j + 1];
				}
				arr.length--;
				removed = true;
				break;
			}
		}
		if (removed) {
			ClazzLoader.classpathMap["$" + pp] = arr;
		}
	}
	var n = ClazzLoader.findClass (qClazzName);
	if (n != null) {
		n.status = ClazzNode.STATUS_KNOWN;
		ClazzLoader.loadedScripts[n.path] = false;
	}
	var path = ClazzLoader.getClasspathFor (qClazzName);
	ClazzLoader.loadedScripts[path] = false;
	if (ClazzLoader.innerLoadedScripts[path]) {
		ClazzLoader.innerLoadedScripts[path] = false;
	}

	ClazzLoader.classUnloaded (qClazzName);
};

/* protected */ /* Clazz#assureInnerClass */
ClazzLoader.assureInnerClass = function (clzz, fun) {
	var clzzName = clzz.__CLASS_NAME__;
	if (Clazz.unloadedClasses[clzzName]) {
		if (clzzName.indexOf ("$") != -1) return;
		var list = new Array ();
		var key = clzzName + "$";
		for (var s in Clazz.unloadedClasses) {
			if (Clazz.unloadedClasses[s] != null && s.indexOf (key) == 0) {
				list[list.length] = s;
			}
		}
		if (list.length == 0) return;
		var funStr = "" + fun;
		var idx1 = funStr.indexOf (key);
		if (idx1 == -1) return;
		var idx2 = funStr.indexOf ("\"", idx1 + key.length);
		if (idx2 == -1) return; // idx2 should never be -1;
		var anonyClazz = funStr.substring (idx1, idx2);
		if (Clazz.unloadedClasses[anonyClazz] == null) return;
		var idx3 = funStr.indexOf ("{", idx2);
		if (idx3 == -1) return;
		idx3++;
		var idx4 = funStr.indexOf ("(" + anonyClazz + ",", idx3 + 3);
		if (idx4 == -1) return; // idx3 should never be -1;
		var idx5 = funStr.lastIndexOf ("}", idx4 - 1);
		if (idx5 == -1) return;
		var innerClazzStr = funStr.substring (idx3, idx5);
		eval (innerClazzStr);
		Clazz.unloadedClasses[anonyClazz] = null;
		/*
		window.setTimeout ((function (clz, str) {
			return function () {
				eval (str);
				Clazz.unloadedClasses[clz] = null;
			};
		}) (anonyClazz, innerClazzStr), 10);
		*/
	}
};


ClassLoader = ClazzLoader;

})(ClazzLoader, ClazzNode);

//}
/******************************************************************************
 * Copyright (c) 2007 java2script.org and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     Zhou Renjian - initial API and implementation
 *****************************************************************************/
/*******
 * @author zhou renjian
 * @create Jan 11, 2007
 *******/

;(function(clpm) {
clpm.fadeOutTimer = null;
clpm.fadeAlpha = 0;
clpm.monitorEl = null;
clpm.lastScrollTop = 0;
clpm.bindingParent = null;
clpm.DEFAULT_OPACITY = 55;
/* private static */ clpm.clearChildren = function (el) {
	if (el == null) return;
	for (var i = el.childNodes.length - 1; i >= 0; i--) {
		var child = el.childNodes[i];
		if (child == null) continue;
		if (child.childNodes != null && child.childNodes.length != 0) {
			this.clearChildren (child);
		}
		try {
			el.removeChild (child);
		} catch (e) {};
	}
};
/* private */ clpm.setAlpha = function (alpha) {
	if (this.fadeOutTimer != null && alpha == this.DEFAULT_OPACITY) {
		window.clearTimeout (this.fadeOutTimer);
		this.fadeOutTimer = null;
	}
	this.fadeAlpha = alpha;
	var ua = navigator.userAgent.toLowerCase ();
	if (ua.indexOf ("msie") != -1 && ua.indexOf ("opera") == -1) {
		this.monitorEl.style.filter = "Alpha(Opacity=" + alpha + ")";
	} else {
		this.monitorEl.style.opacity = alpha / 100.0;
	}
};
/* private */ clpm.hiddingOnMouseOver = function () {
	this.style.display = "none";
};
/* private */ clpm.attached = false;
/* private */ clpm.cleanup = function () {
	var oThis = ClassLoaderProgressMonitor;
	if (oThis.monitorEl != null) {
		oThis.monitorEl.onmouseover = null;
	}
	oThis.monitorEl = null;
	oThis.bindingParent = null;
	Clazz.removeEvent (window, "unload", oThis.cleanup);
	//window.detachEvent ("onunload", oThis.cleanup);
	oThis.attached = false;
};
/* private */ clpm.createHandle = function () {
	var div = document.createElement ("DIV");
	div.id = "clazzloader-status";
	div.style.cssText = "position:absolute;bottom:4px;left:4px;padding:2px 8px;"
			+ "z-index:3333;background-color:#8e0000;color:yellow;" 
			+ "font-family:Arial, sans-serif;font-size:10pt;white-space:nowrap;";
	div.onmouseover = this.hiddingOnMouseOver;
	this.monitorEl = div;
	if (this.bindingParent == null) {
		document.body.appendChild (div);
	} else {
		this.bindingParent.appendChild (div);
	}
	return div;
};
/* private */ clpm.fadeOut = function () {
	if (this.monitorEl.style.display == "none") return;
	if (this.fadeAlpha == this.DEFAULT_OPACITY) {
		this.fadeOutTimer = window.setTimeout (function () {
					ClassLoaderProgressMonitor.fadeOut ();
				}, 750);
		this.fadeAlpha -= 5;
	} else if (this.fadeAlpha - 10 >= 0) {
		this.setAlpha (this.fadeAlpha - 10);
		this.fadeOutTimer = window.setTimeout (function () {
					ClassLoaderProgressMonitor.fadeOut ();
				}, 40);
	}
};
/* private */ clpm.getFixedOffsetTop = function (){
	if (this.bindingParent != null) {
		var b = this.bindingParent;
		return b.scrollTop;
	}
	var dua = navigator.userAgent;
	var b = document.body;
	var p = b.parentNode;
	var pcHeight = p.clientHeight;
	var bcScrollTop = b.scrollTop + b.offsetTop;
	var pcScrollTop = p.scrollTop + p.offsetTop;
	if (dua.indexOf("Opera") == -1 && document.all) {
		return (pcHeight == 0) ? bcScrollTop : pcScrollTop;
	} else if (dua.indexOf("Gecko") != -1) {
		return (pcHeight == p.offsetHeight 
				&& pcHeight == p.scrollHeight) ? bcScrollTop : pcScrollTop;
	}
	return bcScrollTop;
};
/* public */
clpm.initialize = function (parent) {
	this.bindingParent = parent;
	if (parent != null && !this.attached) {
		this.attached = true;
		Clazz.addEvent (window, "unload", this.cleanup);
		// window.attachEvent ("onunload", this.cleanup);
	}
};
/* public */
clpm.showStatus = function (msg, fading) {
	if (this.monitorEl == null) {
		this.createHandle ();
		if (!this.attached) {
			this.attached = true;
			Clazz.addEvent (window, "unload", this.cleanup);
			// window.attachEvent ("onunload", this.cleanup);
		}
	}
	this.clearChildren (this.monitorEl);
	this.monitorEl.appendChild (document.createTextNode ("" + msg));
	if (this.monitorEl.style.display == "none") {
		this.monitorEl.style.display = "";
	}
	this.setAlpha (this.DEFAULT_OPACITY);
	var offTop = this.getFixedOffsetTop ();
	if (this.lastScrollTop != offTop) {
		this.lastScrollTop = offTop;
		this.monitorEl.style.bottom = (this.lastScrollTop + 4) + "px";
	}
	if (fading) {
		this.fadeOut();
	}
};

if (window["ClazzLoader"] != null) {
	ClazzLoader.scriptLoading = function (file) {
		clpm.showStatus ("Loading " + file + "...");
	};
	ClazzLoader.scriptLoaded = function (file) {
		clpm.showStatus (file + " loaded.", true);
	};
	ClazzLoader.globalLoaded = function (file) {
		clpm.showStatus ("Application loaded.", true);
	};
	ClazzLoader.classUnloaded = function (clazz) {
		clpm.showStatus ("Class " + clazz + " is unloaded.", true);
	};
	ClazzLoader.classReloaded = function (clazz) {
		clpm.showStatus ("Class " + clazz + " is reloaded.", true);
	};

	var ua = navigator.userAgent.toLowerCase ();
	if (ua.indexOf ("msie") != -1 && ua.indexOf ("opera") == -1) {
		ClazzLoader.setLoadingMode ("script", 5);
	}
}
            
})(ClazzLoaderProgressMonitor);

//}
/******************************************************************************
 * Copyright (c) 2007 java2script.org and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     Zhou Renjian - initial API and implementation
 *****************************************************************************/
/*******
 * @author zhou renjian
 * @create Nov 5, 2005
 *******/

(function(Console) {
/**
 * Setting maxTotalLines to -1 will not limit the console result
 */
/* protected */
/*-# maxTotalLines -> mtl #-*/
Console.maxTotalLines =	10000;

/* protected */
Console.setMaxTotalLines = function (lines) {
	if (lines <= 0) {
		Console.maxTotalLines = 999999; // Won't reach before browser cracks
	} else {
		Console.maxTotalLines = lines;
	}
};

/*
 * The console window will be flicking badly in some situation. Try to use
 * double buffer to avoid flicking.
 */
/* protected */
/*-# buffering -> bi #-*/
//Console.buffering = false;

/* protected */
//Console.enableBuffering = function (enabled) {
//	Console.buffering = enabled;
//};

/* protected */
/*-# maxBufferedLines -> mbl #-*/
//Console.maxBufferedLines = 20;

/* protected */
//Console.setMaxBufferedLines = function (lines) {
//	if (lines <= 0) {
//		Console.maxBufferedLines = 20;
//	} else {
//		Console.maxBufferedLines = lines;
//	}
//};

/* protected */
/*-# maxLatency -> mlc #-*/
Console.maxLatency = 40;

/* protected */
Console.setMaxLatency = function (latency) {
	if (latency <= 0) {
		Console.maxLatency = 40;
	} else {
		Console.maxLatency = latency;
	}
};

/* protected */
/*-# pinning -> pi #-*/
Console.pinning  = false;

/* protected */
Console.enablePinning = function (enabled) {
	Console.pinning = enabled;
};

/* private */
/*-# linesCount -> lc #-*/
Console.linesCount = 0;

/* private */
/*-# metLineBreak -> mbr #-*/
Console.metLineBreak = false;

/* private */
Console.splitNeedFixed = "\n".split (/\n/).length != 2; // IE

/**
 * For IE to get a correct split result.
 */
/* private */
/*-# splitIntoLineByR -> slr #-*/
Console.splitIntoLineByR = function (s) {
	var arr = new Array ();
	var i = 0;
	var last = -1;
	while (true) {
		i = s.indexOf ('\r', last + 1);
		if (i != -1) {
			arr[arr.length] = s.substring (last + 1, i);
			last = i;
			if (last + 1 == s.length) {
				arr[arr.length] = "";
				break;
			}
		} else {
			arr[arr.length] = s.substring (last + 1);
			break;
		}
	}
	return arr;
};

/**
 * For IE to get a correct split result.
 */
/* private */
/*-# splitIntoLines -> sil #-*/
Console.splitIntoLines = function (s) {
	var arr = new Array ();
	if (s == null) {
		return arr;
	}
	var i = 0;
	var last = -1;
	while (true) {
		i = s.indexOf ('\n', last + 1);
		var str = null;
		if (i != -1) {
			if (i > 0 && s.charAt (i - 1) == '\r') {
				str = s.substring (last + 1, i - 1);
			} else {
				str = s.substring (last + 1, i);
			}
			last = i;
		} else {
			str = s.substring (last + 1);
		}
		var rArr = Console.splitIntoLineByR (str);
		for (var k = 0; k < rArr.length; k++) {
			arr[arr.length] = rArr[k];
		}
		if (i == -1) {
			break;
		} else if (last + 1 == s.length) {
			arr[arr.length] = "";
			break;
		}
	}
	return arr;
};

/**
 * Cache the console until the document.body is ready and console element
 * is created.
 */
/* private */
/*-# consoleBuffer -> cB #-*/
//Console.consoleBuffer = new Array ();

/* private */
/*-# lastOutputTime -> oT #-*/
//Console.lastOutputTime = new Date ().getTime ();

/* private */
/*-# checkingTimer -> lct #-*/
//Console.checkingTimer = 0;

/* private */
/*-# loopChecking -> li  #-*/
//Console.loopChecking = function () {
//	if (Console.consoleBuffer.length == 0) {
//		return ;
//	}
//	var console = document.getElementById ("_console_");;
//	if (console == null) {
//		if (document.body == null) {
//			if (Console.checkingTimer == 0) {
//				Console.checkingTimer = window.setTimeout (
//						"Console.loopChecking ();", Console.maxLatency);
//			}
//			return ;
//		}
//	}
//	Console.consoleOutput ();
//};

/*
 * Give an extension point so external script can create and bind the console
 * themself.
 *
 * TODO: provide more template of binding console window to browser.
 */
/* protected */
Console.createConsoleWindow = function (parentEl) {
	var console = document.createElement ("DIV");
	console.style.cssText = "font-family:monospace, Arial, sans-serif;";
	document.body.appendChild (console);
	return console;
};

/* protected */
/*-#
 # consoleOutput -> cot 
 #
 #-*/
Console.consoleOutput = function (s, color) {
	var console = window["j2s.lib"].console;
	if (console && typeof console == "string")
		console = document.getElementById(console)
		// console)
	if (console == null) {
	  return false; // BH this just means we have turned off all console action
	}
	if (Console.linesCount > Console.maxTotalLines) {
		for (var i = 0; i < Console.linesCount - Console.maxTotalLines; i++) {
			if (console != null && console.childNodes.length > 0) {
				console.removeChild (console.childNodes[0]);
			}
		}
		Console.linesCount = Console.maxTotalLines;
	}
	
	/*-# willMeetLineBreak -> wbr #-*/
	var willMeetLineBreak = false;
	if (typeof s == "undefined") {
		s = "";
	} else if (s == null) {
		s = "null";
	} else {
		s = "" + s;
	}
	if (s.length > 0) {
		/*-# lastChar -> lc #-*/
		var lastChar = s.charAt (s.length - 1);
		if (lastChar == '\n') {
			if (s.length > 1) {
				var preLastChar = s.charAt (s.length - 2);
				if (preLastChar == '\r') {
					s = s.substring (0, s.length - 2);
				} else {
					s = s.substring (0, s.length - 1);
				}
			} else {
				s = "";
			}
			willMeetLineBreak = true;
		} else if (lastChar == '\r') {
			s = s.substring (0, s.length - 1);
			willMeetLineBreak = true;
		}
	}

	var lines = null;
	s = s.replace (/\t/g, Console.c160);
	if (Console.splitNeedFixed) { // IE
		try {
			lines = Console.splitIntoLines (s);
		} catch (e) {
			window.popup (e.message);
		}
	} else { // Mozilla/Firefox, Opera
		lines = s.split (/\r\n|\r|\n/g);
	}
	for (var i = 0; i < lines.length; i++) {
		/*-# lastLineEl -> lE #-*/
		var lastLineEl = null;
		if (Console.metLineBreak || Console.linesCount == 0 
				|| console.childNodes.length < 1) {
			lastLineEl = document.createElement ("DIV");
			console.appendChild (lastLineEl);
			lastLineEl.style.whiteSpace = "nowrap";
			Console.linesCount++;
		} else {
			try {
				lastLineEl = console.childNodes[console.childNodes.length - 1];
			} catch (e) {
				lastLineEl = document.createElement ("DIV");
				console.appendChild (lastLineEl);
				lastLineEl.style.whiteSpace = "nowrap";
				Console.linesCount++;
			}
		}
		var el = document.createElement ("SPAN");
		lastLineEl.appendChild (el);
		el.style.whiteSpace = "nowrap";
		if (color != null) {
			el.style.color = color;
		}
		if (lines[i].length == 0) {
			lines[i] = String.fromCharCode (160);
			//el.style.height = "1em";
		}
		el.appendChild (document.createTextNode (lines[i]));
		if (!Console.pinning) {
			console.scrollTop += 100;
		}
		
		if (i != lines.length - 1) {
			Console.metLineBreak = true;
		} else {
			Console.metLineBreak = willMeetLineBreak;
		}
	}
	
	var cssClazzName = console.parentNode.className;
	if (!Console.pinning && cssClazzName != null
			&& cssClazzName.indexOf ("composite") != -1) {
		console.parentNode.scrollTop = console.parentNode.scrollHeight;
	}
	Console.lastOutputTime = new Date ().getTime ();
};

/*
 * Clear all contents inside the console.
 */
/* public */
Console.clear = function () {
	try {
	Console.metLineBreak = true;
	var console = window["j2s.lib"].console;
	if (console == null || (console = document.getElementById (console)) == null) {
		return;
	}
	var childNodes = console.childNodes;
	for (var i = childNodes.length - 1; i >= 0; i--) {
		console.removeChild (childNodes[i]);
	}
	Console.linesCount = 0;
	} catch(e){};
};

/* public */
Clazz.alert = function (s) {
	Console.consoleOutput (s + "\r\n");
};

	Console.c160 = String.fromCharCode (160); //nbsp;
	Console.c160 += Console.c160+Console.c160+Console.c160;
	
	System.currentTimeMillis = function () {
		return new Date ().getTime ();
	};

	/* public */
	System.arraycopy = function (src, srcPos, dest, destPos, length) {
		if (src != dest) {
			for (var i = 0; i < length; i++) {
				dest[destPos + i] = src[srcPos + i];
			}
		} else {
			var swap = [];
			for (var i = 0; i < length; i++) {
				swap[i] = src[srcPos + i];
			}
			for (var i = 0; i < length; i++) {
				dest[destPos + i] = swap[i];
			}
		}
	};
	
	System.props = null; //new java.util.Properties ();
	System.getProperties = function () {
		return System.props;
	};
	System.getProperty = function (key, def) {
		if (System.props != null) {
			return System.props.getProperty (key, def);
		}
		if (arguments.length == 1) return null;// BH
		if (def != null) {
			return def;
		}
		return key;
	};
	System.setProperties = function (props) {
		System.props = props;
	};
	System.setProperty = function (key, val) {
		if (System.props == null) {
			return ;
		}
		System.props.setProperty (key, val);
	};

	/* public */
	System.out = new JavaObject ();
	System.out.__CLASS_NAME__ = "java.io.PrintStream";
	
	/* public */
	System.out.print = function (s) { 
		Console.consoleOutput (s);
	};
	
	/* public */
	System.out.println = function (s) {
		if (typeof s == "undefined") {
			s = "\r\n";
		} else if (s == null) {
			s = "null\r\n";
		} else {
			s = s + "\r\n";
		}
		Console.consoleOutput (s);
	};
	
	/* public */
	System.err = new JavaObject ();
	System.err.__CLASS_NAME__ = "java.io.PrintStream";
	
	/* public */
	System.err.print = function (s) { 
		Console.consoleOutput (s, "red");
	};
	
	/* public */
	System.err.println = function (s) {
		if (typeof s == "undefined") {
			s = "\r\n";
		} else if (s == null) {
			s = "null\r\n";
		} else {
			s = s + "\r\n";
		}
		Console.consoleOutput (s, "red");
	};
	
  System.gc = function() {}; // bh added
  System.getSecurityManager = function() { return null };  // bh added
  String.prototype.contains = function(a) {return this.indexOf(a) >= 0}  // bh added
  String.prototype.compareTo = function(a){return this > a ? 1 : this < a ? -1 : 0} // bh added
})(Clazz.Console, System);

Clazz.setConsoleDiv = function(d) {
	window["j2s.lib"].console = d;
};

})(Clazz);

// moved here from package.js

	ClazzLoader.registerPackages ("java", [
			"io", "lang", 
			//"lang.annotation", 
			"lang.reflect",
			"util", 
			//"util.concurrent", "util.concurrent.atomic", "util.concurrent.locks",
			//"util.jar", "util.logging", "util.prefs", 
			"util.regex",
			"util.zip",
			"net", "text"]);
			
	window["reflect"] = java.lang.reflect;

	ClazzLoader.ignore([
		"net.sf.j2s.ajax.HttpRequest",
		"java.util.MapEntry.Type",
		"java.net.UnknownServiceException",
		"java.lang.Runtime",
		"java.security.AccessController",
		"java.security.PrivilegedExceptionAction",
		"java.io.File",
		"java.io.FileInputStream",
		"java.io.FileWriter",
		"java.io.OutputStreamWriter",
		"java.util.Calendar", // bypassed in ModelCollection
		"java.text.SimpleDateFormat", // not used
		"java.text.DateFormat", // not used
		"java.util.concurrent.Executors"
	])
	

};
