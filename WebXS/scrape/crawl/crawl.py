#!/usr/bin/env python
from ghost import Ghost

import argparse
import logging

logging.basicConfig()

class BaseCrawler:
    # possible search-related arguments mapped to their names in the form.
    # apparently CSS3 selectors need quotes for these.
    search_params = {
    }

    # possible arguments that are not search terms, and their default values.
    non_search_params = {
        'debug': False,
        'timeout': 20,
        'dl_images': False
    }
    
    # CSS3 selectors
    selectors = {
    }

    # javascript expressions
    js_exprs = {
        # {0} is a CSS3 selector for the element, {1} is the attribute to select.
        'get_attr': "document.querySelector('{0}').{1};" ,
        # {0} is a CSS3 selector for the element, {1} is the attribute to set, and
        # {2} is the value to assign it.
        'set_attr': "document.querySelector('{0}').{1} = {2};"
    }

    # initialization methods

    def __init__(self, **terms):
        # specified arguments will overwrite defaults
        self.write_defaults()

        for key, value in terms.iteritems():
            if key in self.non_search_params:
                setattr(self, key, value)
            else:
                self.search_terms[self.search_params[key]] = value

        self.config_ghost()

    def write_defaults(self):
        """ Write default values to all instance variables. """
        # maps webpage form inputs to user-supplied search terms
        self.search_terms = {}
        # if search term was not supplied, will be an empty string
        for form_field in self.search_params.itervalues():
            self.search_terms[form_field] = ''

        for var, val in self.non_search_params.iteritems():
            setattr(self, var, val)

    def config_ghost(self):
        self.ghost = Ghost(download_images=self.dl_images,
                           wait_timeout=self.timeout,
                           display=self.debug)

    # utility methods

    def set_attr(self, selector, attr, new_value, **kwargs):
        """ Set an attribute of a DOM element. """
        self.ghost.evaluate(self.js_exprs['set_attr'] \
                            .format(selector,
                                    attr,
                                    new_value),
                            **kwargs)

    def get_attr(self, selector, attr, **kwargs):
        """ Get an attribute from a DOM element. """
        return self.ghost.evaluate(self.js_exprs['get_attr'] \
                                   .format(selector,
                                           attr),
                                   **kwargs)[0]

    def get_attr_extract(self, selector, attr):
        """ Get an attribute from a DOM element. Do so by parsing its tag. """
        outerHtml = self.ghost.evaluate(self.js_exprs['get_attr'] \
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
