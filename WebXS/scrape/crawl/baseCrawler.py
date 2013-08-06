#!/usr/bin/env python
from util import *

from ghost import Ghost

# this is defined outside the class so that it can be referenced
# while the class is being interpreted.
def param_debug(param, self, value):
    """
    Override the default NonSearchParam on_eval callback for --debug,
    so that it can change the Ghost configuration options.
    Note the order of parameters, especially where 'self' is. This should be
    attached as a method to the Param for --debug; it is not attached to
    the BaseCrawler. See the implementation of BaseCrawler.visit_param().

    param: the NonSearchParam this will be attached to. See implementation
    of util.Param for details of how this all works.

    self: this crawler

    value: the parameter value passed by the user.
    """
    self.debug = value
    self.ghost_params['display'] = value



class BaseCrawler(object):
    """ Base class for scrapers. """

    # Startup resources

    # possible search-related parameters.
    # apparently CSS3 selectors need quotes for these.
    # INHERITORS: override this with ParamList of desired SearchParam objects.
    search_params = ParamList()

    # possible parameters that are not search terms.
    # INHERITORS: recommend override this with ParamList of
    # desired NonSearchParam objects.
    non_search_params = ParamList(
        NonSearchParam('--debug',
                       on_eval=param_debug, # INHERITORS: use 'baseCrawler.param_debug'
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
    #
    # These should all be decorated @classmethod, as they will all be called
    # before an instance can be initialized.
    
    @classmethod
    def parser_preconfig(cls, parser):
        """
        Configure the parser corresponding to this crawler.
        This will be called before arguments have been automatically added.
        """
        pass

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
        for param_name, arg_value in args.iteritems():
            if (arg_value != '' and
                cls.search_params.hasParam(param_name)):
                    return Status(True)

        m = 'No search made. Must provide at least one search argument.'
        return Status(False, m)
    
    @classmethod
    def parser_postconfig(cls, parser):
        """
        Configure the parser corresponding to this crawler.
        This will be called after arguments have been automatically added.
        """
        pass

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
        # maps webpage form <input> names to user-supplied search terms
        self.search_terms = {}
        
        param = None
        for param_name, arg_value in terms.iteritems():
            # test whether SearchParam or NonSearchParam
            if self.non_search_params.hasParam(param_name):
                param = self.non_search_params.getParam(param_name)
                setattr(self, param_name, arg_value)
            else:
                # must be a search param
                param = self.search_params.getParam(param_name)
                input_name = self.search_params.getParam(param_name).input_name
                self.search_terms[input_name] = arg_value

            self.visit_param(param, arg_value)

        self.config_ghost()

    def visit_param(self, param, value):
        """
        Visit a param during evaluation.
        This fires its on_eval callback.
        """
        param.on_eval(self, value)

    def config_ghost(self):
        """
        Configure Ghost using self.ghost_params.
        """
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
