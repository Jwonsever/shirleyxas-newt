#!/usr/bin/env python
from baseCrawler import BaseCrawler
from util import *

import argparse
import HTMLParser

class IcsdCrawler(BaseCrawler):
    """ Scraper for the ICSD. """

    # Startup resources

    # possible search-related parameters.
    # apparently CSS3 selectors need quotes for these.
    search_params = ParamList(
        SearchParam('--composition',
                    '"chemistrySearch.sumForm"',
                    help='space-separated chemical composition e.g. "Na Cl"'),
        SearchParam('--num_elements',
                    '"chemistrySearch.elCount"',
                    help='number of elements'),
        SearchParam('--struct_fmla',
                    '"chemistrySearch.structForm"',
                    help='space-separated structural formula e.g. "Pb (W 04)"'),
        SearchParam('--chem_name',
                    '"chemistrySearch.chemName"',
                    help='chemical name'),
        SearchParam('--mineral_name',
                    '"chemistrySearch.mineralName"',
                    help='mineral name'),
        SearchParam('--mineral_grp',
                    '"chemistrySearch.mineralGroup"',
                    help='mineral group'),
        SearchParam('--anx_fmla',
                    '"chemistrySearch.anxFormula"',
                    help='ANX formula crystal composition'),
        SearchParam('--ab_fmla',
                    '"chemistrySearch.abFormula"',
                    help='AB formula crystal composition'),
        SearchParam('--num_fmla_units',
                    '"chemistrySearch.z"',
                    help='number of formula units')
    )

    # possible parameters that are not search terms.
    non_search_params = ParamList(
        NonSearchParam('--debug',
                       param_debug,
                       action='store_true',
                       help='enables debug mode'),
        NonSearchParam('--num_results',
                       default=10,
                       type=int,
                       help='desired number of results to fetch')
    )
    
    # arguments to this scraper's parser.
    parser_params = {
        'description': 'A scraper for the ICSD at http://icsd.fiz-karlsruhe.de/'    
    }

    # Configuration Resources
    # (default)

    # Runtime Resources

    start_url = 'http://icsd.fiz-karlsruhe.de/'
    
    # CSS3 selectors
    selectors = {
        # for stage 1
        'chem_search_area_btn': 'li#dynNav li:nth-child(3) a',

        # for stage 2
        'search_form': 'form[name="Request"]',
        'run_query_btn': 'table#select_action_table td:nth-child(1) a.linkButtonClassBlock',

        # for stage 3.1
        'sorting_msg': 'div#SweetDevRia_WaitMessage',
        # for stage 3.2
        'num_hits': '#col3_innen div.contentElement div.title div:nth-child(3)',
        # for stage 3.3
        # for the result in the {0}th row
        'nth_row_box':  'table.ideo-ndg-body tr.ideo-ndg-row:nth-of-type({0}) input[type="checkbox"]',
        # for stage 3.4
        'export_data': '#navLinkExport',
        'single_cif_dl': '#singleCifFile',
        'big_dl_confirm': '#confirm div.yes',

        # for stage 3.error
        # requires different selectors because the alerts are inconsistently formatted
        'no_results': 'td#WzBoDyI>p:nth-child(2)>b',
        'too_many_results': 'td#WzBoDyI>p:first-child'
    }

    # various messages
    messages = {
        'default_search_error': 'An unknown error occurred with the search. Try narrowing or widening your search.'
    }

    # Runtime methods

    def crawl(self):
        """
        Make search and parse list of results.
        Return a JsonList of all the structures fetched.
        If an error occurs during search, this list will be empty.
        """
        # TODO: perhaps make this a little less hacky
        # (I mean the passing of the url)
        search_url = self.find_search_area()
        if self.do_search(search_url):
            return self.get_results()
        else:
            return self.handle_search_error()
    
    def find_search_area(self):
        """ Stage 1: navigate to Chemistry search area. """
        self.ghost.open(self.start_url)
        page, resources = self.ghost.click(self.selectors['chem_search_area_btn'], expect_loading=True)
        return page.url

    def do_search(self, search_url):
        """
        Stage 2: perform search query.
        Returns whether or not the search was performed successfully.
        """
        # populate search terms
        self.ghost.fill(self.selectors['search_form'], self.search_terms)
        # perform the search
        page, resources = self.ghost.click(self.selectors['run_query_btn'], expect_loading=True)
        # if search is successful, url will have changed
        # but ignore any cookie difference in url
        return self.strip_url(page.url) != self.strip_url(search_url)

    def get_results(self):
        """ Stage 3: download the desired number of search results. """
        self.wait_for_sorting()
        self.verify_num_results()
        self.select_results(self.num_results)
        # doesn't matter if sorting is done. Don't need to wait for it.
        return self.download_selected()

    def wait_for_sorting(self):
        """ Stage 3.1: wait for the table sorting to finish. """
        busy_msg = self.selectors['sorting_msg']

        def busy_msg_invisible():
            style = self.dom_prop(busy_msg, 'style')['cssText']
            return ('display: none' in style or     # <- lol in style
                    'display:none' in style)

        # wait for busy message to appear and disappear
        self.ghost.wait_for_selector(busy_msg)
        self.ghost.wait_for(busy_msg_invisible,
                            'table never finished sorting')

    def verify_num_results(self):
        """
        Stage 3.2: verify that the number of results requested is possible.
        If fewer results exist than were requested, select them all.
        """
        num_hits = self.dom_prop(self.selectors['num_hits'], 'innerHTML')
        # trim away label part
        num_hits = num_hits[num_hits.find(':')+1:].strip()
        num_hits = int(num_hits)
        self.num_results = min(self.num_results, num_hits)
        # TODO: deal with the popup that appears if num_results > 50
        self.num_results = min(self.num_results, 50)

    def select_results(self, num_results):
        """ Stage 3.3: select the desired results to download. """
        # prepare for jank: breaking two abstraction barriers.
        # 1) bypass checkboxes and go into SweetDevRia library.
        # 2) bypass library API and edit internal list of selected results.

        # expression to access SweetDevRia table properties
        onclick = self.get_attr(self.selectors['nth_row_box'].format(1),
                                'onclick')
        selector = onclick[: onclick.find(')') + 1]
        
        # selected results
        selections = []
        for nth_result in range(num_results):
            selections.append(str(nth_result))

        # edit SweetDevRia table property
        self.ghost.evaluate(selector + '.checkedRows = ' + str(selections))

    def download_selected(self):
        """ Stage 3.4: download the selected results. """
        # navigate to area where we can download concatenated .cif files.
        submitter = self.dom_prop(self.selectors['export_data'], 'href')
        colon_index = submitter.find(':')
        self.ghost.evaluate(submitter[colon_index+1:], expect_loading=True)

        # download all .cif files concatenated into one.
        _, resources = self.ghost.click(self.selectors['single_cif_dl'], expect_loading=True)
        # TODO: still getting duplication. Returning only the first resource hides this issue.
        # find out why duplication is happening.
        results = self.separate(resources[0].content)
        return JsonList(results)

    def handle_search_error(self):
        """ Stage 3.error: handle an error with the search. """
        error_msg = self.messages['default_search_error']
        # this ordering is important. Second is more generic and will pass either way.
        # this is because of the inconsistency in the alerts.
        for selector in ('no_results', 'too_many_results'):
            if self.ghost.exists(self.selectors[selector]):
                error_msg = self.dom_prop(self.selectors[selector], 'innerHTML')
                error_msg = HTMLParser.HTMLParser().unescape(error_msg)
                break

        print error_msg.strip()
        # empty JsonList
        return JsonList()

    # Utility methods

    def separate(self, cif):
        """
        Separate a single (string of a) CIF file containing multiple structures
        into multiple files.
        """
        sep = '#'
        length = len(cif)
        start, end = 0, 0

        while 0 <= end < length:
            start = cif.find(sep, end)
            end = cif.find(sep, start + 1)
            end = cif.find(sep, end + 1)
            yield cif[start : end]

    def strip_url(self, url, c=';'):
        """
        Return the url stripped of everything at and past
        the last instance of the given character.
        If the character is not found, the whole url is returned.
        """
        index = url.rfind(c)
        if index < 0:
            return url
        else:
            return url[:index]
        


def parse_crawler_args():
    """ parse command-line arguments into a dictionary of web crawler arguments. """
    parser = argparse.ArgumentParser(description='A scraper for the ICSD at http://icsd.fiz-karlsruhe.de/')
    parser.add_argument('--num_results',
                        default=10,
                        type=int,
                        help='desired number of results to fetch')
    parser.add_argument('--debug',
                        action='store_true',
                        help='enables debug mode')
    parser.add_argument('--composition',
                        default='',
                        help='space-separated chemical composition e.g. "Na Cl"')
    parser.add_argument('--num_elements',
                        default='',
                        help='number of elements')
    parser.add_argument('--struct_fmla',
                        default='',
                        help='space-separated structural formula e.g. "Pb (W 04)"')
    parser.add_argument('--chem_name',
                        default='',
                        help='chemical name')
    parser.add_argument('--mineral_name',
                        default='',
                        help='mineral name')
    parser.add_argument('--mineral_grp',
                        default='',
                        help='mineral group')
    parser.add_argument('--anx_fmla',
                        default='',
                        help='ANX formula crystal composition')
    parser.add_argument('--ab_fmla',
                        default='',
                        help='AB formula crystal composition')
    parser.add_argument('--num_fmla_units',
                        default='',
                        help='number of formula units')

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
        args['composition'],
        args['num_elements'],
        args['struct_fmla'],
        args['chem_name'],
        args['mineral_name'],
        args['mineral_grp'],
        args['anx_fmla'],
        args['ab_fmla'],
        args['num_fmla_units']
    ]

    search_given = False
    for arg in search_args:
        if arg != '':
            search_given = True
    
    return search_given

def main():
    spider = IcsdCrawler(**parse_crawler_args())
    cl = spider.crawl()
    print cl.json
    print 'fetched {0} results.'.format(len(cl))

if __name__ == "__main__":
    main()
