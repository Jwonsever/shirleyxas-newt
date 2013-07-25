#!/usr/bin/env python
from ghost import Ghost

from cif import CifList

import argparse
import HTMLParser
import logging

logging.basicConfig()

class AmcsdCrawlerGhost:
    start_url = 'http://rruff.geo.arizona.edu/AMS/amcsd.php'

    # possible search-related arguments mapped to their names in the form
    # apparently CSS3 selectors need quotes for these
    search_params = {
        'mineral': '"Mineral"',
        'author': '"Author"',
        'chemistry': '"Periodic"',
        'cellParam': '"CellParam"',
        'diffraction': '"diff"',
        'general': '"key"',
    }

    # possible arguments that are not search terms, and their default values.
    non_search_params = {
        'debug': False
    }
    
    # CSS3 selectors
    selectors = {
        # for stage 1
        'search_form': 'form[name="myForm"]',
        'search_button': 'form[name="myForm"] input[type="submit"]',
        'result_table': 'form[name="myForm"] table',

        # for stage 2.1
        'select_all_btn': 'form[name="result_form"] input[type="button"][onclick*="selectall()"]',
        # for stage 2.2
        'format_select': 'form[name="result_form"] select[name="down"]',
        # for stage 2.3
        # "ownload" for improvised case-insensitivity of "[Dd]ownload"
        'download_btn': 'form[name="result_form"] input[type="Submit"][value*="ownload"]',

        # for stage 2.error
        'error_msg': 'form[name="result_form"]>p'
    }

    # javascript expressions
    js_exprs = {
        # {0} is a CSS3 selector for the element, {1} is the attribute to select.
        'get_attr': "document.querySelector('{0}').{1};" ,
        # {0} is a CSS3 selector for the element, {1} is the attribute to set, and
        # {2} is the value to assign it.
        'set_attr': "document.querySelector('{0}').{1} = {2};"
    }

    # various messages
    messages = {
        'no_matches': 'No matches were found for the search',
        'default_search_error': 'An unknown error occurred with the search. Try narrowing or widening your search.'
    }

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
        self.ghost = Ghost(download_images=False,
                           wait_timeout=20,
                           display=self.debug)

    def crawl(self):
        """
        Make search and parse list of results.
        Return a CifList of all the structures fetched.
        If an error occurs during search, this list will be empty.
        """
        if self.do_search():
            return self.get_results()
        else:
            self.handle_search_error()

    def do_search(self, search_url):
        """
        Stage 1: perform search query.
        Returns whether or not the search was performed successfully.
        """
        # populate search terms.
        self.ghost.fill(self.selectors['search_form'], self.search_terms)
        # perform the search.
        page, resources = self.ghost.click(self.selectors['run_query_btn'], expect_loading=True)
        # test if the search worked.
        return self.ghost.exists(self.selectors['result_table'])
    
    def get_results(self):
        """
        Stage 2: download all the results on the page.
        TODO: limit the number of downloaded results to a specified maximum.
        """
        self.select_results()
        self.select_format()
        return self.download_selected()

    def select_results(self):
        """
        Stage 2.1: select the desired results to download.
        """
        self.ghost.click(self.selectors['select_all_btn'])

    def select_format(self):
        """
        Stage 2.2: select the desired format of the results.
        """
        # select the 'cif' option from the format selection drop-down menu.
        self.ghost.evaluate("document.querySelector('{0}').setAttribute('{1}');" \
                            .format(self.selectors['format_select'],
                                    'cif'))

    def download_selected(self):
        """
        Stage 2.3: download the selected results.
        """
        _, resources = self.ghost.click(self.selectors['download_btn'])
        # TODO: return CifList of contents of resources.
        print resources

    def handle_search_error(self):
        """
        Stage 2.error: handle an error with the search.
        """
        # TODO: generalize for different messages.
        message = self.get_attr(self.selectors['error_msg'], 'innerHTML')
        if message.find('No matches') >= 0:
            print self.messages['no_matches']
        else: 
            print self.messages['default_search_error']

    def set_attr(self, selector, attr, new_value):
        """ Set an attribute of a DOM element. """
        self.ghost.evaluate(self.js_exprs['set_attr'] \
                            .format(selector,
                                    attr,
                                    new_value))

    def get_attr(self, selector, attr):
        """ Get an attribute from a DOM element. """
        return self.ghost.evaluate(self.js_exprs['get_attr'] \
                                   .format(selector,
                                           attr))[0]

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
        


def parse_crawler_args():
    """ parse command-line arguments into a dictionary of web crawler arguments. """
    parser = argparse.ArgumentParser(description='A scraper for the AMCSD at http://rruff.geo.arizona.edu/AMS/amcsd.php')
    parser.add_argument('--debug',
                        action='store_true',
                        help='enables debug mode')
    parser.add_argument('--mineral',
                        default='',
                        help='name of a mineral')
    parser.add_argument('--author',
                        default='',
                        help='name of an author')
    parser.add_argument('--chemistry',
                        default='',
                        help='comma-separated elements, all inside parentheses. See website for details.')
    parser.add_argument('--cellParam',
                        default='',
                        help='cell parameters and symmetry. See website for details')
    parser.add_argument('--diffraction',
                        default='',
                        help='diffraction pattern. See website for detials.')
    parser.add_argument('--general',
                        default='',
                        help='general, keyword-based search. See website for details.')

    args = vars(parser.parse_args())
    
    # at least one search term must be given.
    if not verify_search_made(args):
        parser.error('No search made. Must provide at least one search argument.')

    return args

def verify_search_made(args):
    """
    Verify that a dict of argments constitutes a valid search.
    """
    search_args = [
        args['mineral'],
        args['author'],
        args['chemistry'],
        args['cellParam'],
        args['diffraction'],
        args['general']
    ]

    search_given = False
    for arg in search_args:
        if arg != '':
            search_given = True
    
    return search_given

def main():
    spider = AmcsdCrawlerGhost(**parse_crawler_args())
    cl = spider.crawl()
    print cl.json
    print 'fetched {0} results.'.format(len(cl))

if __name__ == "__main__":
    main()
