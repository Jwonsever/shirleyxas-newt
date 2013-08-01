#!/usr/bin/env python
from util import *

from ghost import Ghost

class BaseCrawler(object):
    """ Base class for scrapers. """

    # Startup resources

    # possible search-related parameters.
    # apparently CSS3 selectors need quotes for these.
    # INHERITORS: override this with ParamList of desired SearchParam objects.
    search_params = ParamList()

    # possible parameters that are not search terms.
    # INHERITORS: recommend override this with ParamList of desired Param objects.
    non_search_params = ParamList(
        Param('--debug',
              action='store_true',
              help='enables debug mode')
    )

    # arguments to this scraper's parser.
    # INHERITORS: override this with:
    #   - key: argparse constructor parameter.
    #   - value: argument for parameter
    parser_params = {
        'description': 'An example web scraper'
    }

    # Configuration resources

    # Ghost configuration options.
    # will be unpacked in Ghost constructor;
    # see implementation of config_ghost().
    # INTERITORS: recommend override this with:
    #   - key: Ghost constructor parameter.
    #   - value: argument for parameter.
    ghost_params = {
            # TODO:
            # this breaks the --debug command-line option. fix it.
            # Possibly make non-search-parameters more flexible by
            # allowing for alternate handlers than the default setattr one.
        'display': False,
        'wait_timeout': 20,
        'download_images': False
    }

    # Runtime resources
    
    # CSS3 selectors
    # INHERITORS: recommend override this with:
    #   - key: meaningful identifier for a CSS3 selector.
    #   - value: CSS3 selector.
    selectors = {
    }

    # javascript expressions
    # INHERITORS: recommend override this with:
    #   - key: meaningful identifier for a javascript expression.
    #   - value: javascript expression.
    js_exprs = {
        # {0} is a CSS3 selector for the element, {1} is the property to access.
        'dom_prop': "document.querySelector('{0}').{1};"
    }

    # Configuration methods

    @classmethod
    def verify_args(cls, args):
        """
        Verify that the given command-line arguments are valid.
        They have already been syntactically validated;
        this is meant to check semantic validity.

        args: a dict of:
            - key: argument name.
            - value: argument value.

        return: Status object.
            - Success: no message required.
            - Failure: message required
        """
        # verify at least one search arg was given
        #print args
        for param_name, arg_value in args.iteritems():
            #print param_name, arg_value
            #print cls.search_params.hasParam(param_name)
            if (arg_value != '' and
                cls.search_params.hasParam(param_name)):
                    #print 'SUCCESS'
                    return Status(True)

        m = 'No search made. Must provide at least one search argument.'
        #print 'FAILURE'
        return Status(False, m)

    # Initialization methods

    def __init__(self, **terms):
        """
        Handle command-line arguments:
            - put search-related arguments in dict self.search_terms:
                - key: search form input element name
                - value: user-supplied search value, or blank by default.
            - assign non-search-related arguments as instance variables:
                - if not specified, uses default value, as defined by
                  self.non_search_params.

        Also configure Ghost.
        """
        '''
        # specified arguments will overwrite defaults
        self.write_defaults()
        '''
        # maps webpage form <input> names to user-supplied search terms
        self.search_terms = {}
        
        for param_name, arg_value in terms.iteritems():
            #print 'looking for', param_name
            if self.non_search_params.hasParam(param_name):
                #print 'found non_search_param'
                setattr(self, param_name, arg_value)
            else:
                # must be a search param
                #print 'must be a search_param'
                input_name = self.search_params.getParam(param_name).input_name
                self.search_terms[input_name] = arg_value

        self.config_ghost()

    '''
    def write_defaults(self):
        """
        Write default values to all instance variables:
            - search-related: defaults to blank.
            - non-search-related: defaults specified by self.non_search_params
        """
        # maps webpage form <input> names to user-supplied search terms
        self.search_terms = {}
        
        # if search term was not supplied, will be an empty string
        for searchParam in self.search_params:
            self.search_terms[searchParam.input_name] = ''

        for var, val in self.non_search_params.iteritems():
            setattr(self, var, val)
    '''

    def config_ghost(self):
        """
        Configure Ghost using self.ghost_params.
        """
        # TODO: make a more flexible way to specify these options.
        self.ghost = Ghost(**self.ghost_params)

    # Runtime methods

    def crawl(self):
        """
        Crawl the website.
        This is the "business method".
        Will be called automatically, do not call it yourself.
        INHERITORS: must override this.
        """
        # empty JsonList
        return JsonList()

    # Utility methods

    def dom_prop(self, selector, prop, **kwargs):
        """
        Access a property from a DOM node.
        Passing a property name will get its value.
        Assiging a property name, such as 'checked = true',
        will assign to the property.
        Note that member functions can be called like this as well;
        see the implementation of get_attr().
        """
        return self.ghost.evaluate(self.js_exprs['dom_prop'] \
                                   .format(selector,
                                           prop),
                                   **kwargs)[0]

    def get_attr(self, selector, attr, **kwargs):
        """
        Get an attribute from a DOM node.
        """
        return self.dom_prop(selector, 
                             'getAttribute("{0}")'.format(attr),
                             **kwargs)

    def get_attr_extract(self, selector, attr):
        """
        Get an attribute from a DOM node. Do so by parsing its tag.
        You shouldn't really be using this. Use get_attr instead,
        unless for some reason it doesn't work and this does.
        That propably shouldn't happen, but browsers are strange.
        """
        outerHtml = self.ghost.evaluate(self.js_exprs['dom_prop'] \
                                        .format(selector,
                                                'outerHTML'))[0]
        return self.extract_tag_attr(outerHtml, attr)

    def extract_tag_attr(self, tag, attr, quote='"'):
        """
        Given a tag, extract the value of a given attribute inside it,
        wrapped in the given type of quotes.
        """ 
        attr_assignment = tag.find(attr + '=')
        first = tag.find(quote, attr_assignment)
        last = tag.find(quote, first + 1)
        return tag[first + 1 : last]
