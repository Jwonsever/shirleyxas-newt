#!/usr/bin/env python
from ghost import Ghost

class BaseCrawler:
    # possible search-related parameters mapped to their names in the form.
    # apparently CSS3 selectors need quotes for these.
    # INHERITORS: override this with:
    #   - key: command-line search-related parameter name (with any dashes).
    #   - value: search form input element name.
    search_params = {
    }

    # possible parameters that are not search terms, and their specifications.
    # INHERITORS: recommend override this with:
    #   - key: command-line not-search-related parameter name (with any dashes).
    #   - value: dict containing argparse add_argument() arguments.
    non_search_params = {
        '--debug': {
            'action': 'store_true',
            'help': 'enables debug mode'
        }
    }

    # arguments to this scraper's parser.
    # INHERITORS: override this with:
    #   - key: argparse constructor parameter.
    #   - value: argument for parameter
    parser_params = {
        'description': 'An example web scraper'
    }

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

    # initialization methods

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
        # specified arguments will overwrite defaults
        self.write_defaults()

        for key, value in terms.iteritems():
            if key in self.non_search_params:
                setattr(self, key, value)
            else:
                self.search_terms[self.search_params[key]] = value

        self.config_ghost()

    def write_defaults(self):
        """
        Write default values to all instance variables:
            - search-related: defaults to blank.
            - non-search-related: defaults specified by self.non_search_params
        """
        # maps webpage form inputs to user-supplied search terms
        self.search_terms = {}
        # if search term was not supplied, will be an empty string
        for form_field in self.search_params.itervalues():
            self.search_terms[form_field] = ''

        for var, val in self.non_search_params.iteritems():
            setattr(self, var, val)

    def config_ghost(self):
        """
        Configure Ghost using self.ghost_params.
        """
        # TODO: make a more flexible way to specify these options.
        self.ghost = Ghost(**self.ghost_params)

    # utility methods

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
