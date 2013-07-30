#!/usr/bin/env python
from crawl import BaseCrawler
from jsonList import JsonList

import argparse
import time

class AmcsdCrawler(BaseCrawler):
    start_url = 'http://rruff.geo.arizona.edu/AMS/amcsd.php'

    # possible search-related arguments mapped to their names in the form.
    # apparently CSS3 selectors need quotes for these.
    search_params = {
        'mineral': '"Mineral"',
        'author': '"Author"',
        'chemistry': '"Periodic"',
        'cellParam': '"CellParam"',
        'diffraction': '"diff"',
        'general': '"Key"',
    }
    
    # CSS3 selectors
    selectors = {
        # for stage 1
        'search_form': 'form[name="myForm"]',
        'format_btn': 'form[name="myForm"] input[type="radio"][name="Download"]',
        'result_table': 'form[name="result_form"] table',

        # for stage 2.1
        'select_all_btn': 'form[name="result_form"] input[type="button"][onclick*="selectall()"]',
        # for stage 2.2
        'result_form': 'form[name="result_form"]',
        'view_btn': 'form[name="result_form"] input[name="viewSelected"]',
        'result_area': 'pre',

        # for stage 2.error
        'error_msg': 'form[name="result_form"]'
    }

    # various messages
    messages = {
        'no_matches': 'No matches were found for the search',
        'default_search_error': 'An unknown error occurred with the search. Try narrowing or widening your search.'
    }

    def crawl(self):
        """
        Make search and parse list of results.
        Return a JsonList of all the structures fetched.
        If an error occurs during search, this list will be empty.
        """
        self.ghost.open(self.start_url)
        if self.do_search():
            return self.get_results()
        else:
            return self.handle_search_error()

    def do_search(self):
        """
        Stage 1: perform search query.
        Returns whether or not the search was performed successfully.
        """
        form = self.selectors['search_form']
        # populate search terms.
        self.ghost.fill(form, self.search_terms)
        self.ghost.set_field_value(self.selectors['format_btn'], 'cif')
        # perform the search.
        self.get_attr(form, 'submit()', expect_loading=True)

        # test if the search worked.
        return self.ghost.exists(self.selectors['result_table'])
    
    def get_results(self):
        """
        Stage 2: download all the results on the page.
        TODO: limit the number of downloaded results to a specified maximum.
        """
        self.select_results()
        return self.download_selected()

    def select_results(self):
        """
        Stage 2.1: select the desired results to download.
        """
        self.get_attr(self.selectors['select_all_btn'], 'click()')


    def download_selected(self):
        """
        Stage 2.2: download the selected results.
        """
        # TODO: split into two steps: navigation and downloading

        # TODO: HUGE BUG:
        # if there is exactly one result, this will fail

        # get to results-viewing page
        self.get_attr(self.selectors['view_btn'], 'click()')
        self.ghost.fire_on(self.selectors['result_form'], 'submit', expect_loading=True)

        # get results
        results_string = self.get_attr(self.selectors['result_area'], 'innerHTML')
        results = results_string.split('\n\n')

        # filter out any empty string elements
        not_empty = lambda string: string != ''
        results = filter(not_empty, results)

        return JsonList(results)

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

        # empty JsonList
        return JsonList()
        


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
    spider = AmcsdCrawler(**parse_crawler_args())
    cl = spider.crawl()
    print cl.json
    print 'fetched {0} results.'.format(len(cl))

if __name__ == "__main__":
    main()
