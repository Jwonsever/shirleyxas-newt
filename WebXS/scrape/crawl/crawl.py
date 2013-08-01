#!/usr/bin/env python
import argparse
import logging

# import all known crawlers
# if you make a new crawler, import it here.
from icsdCrawler import IcsdCrawler
from amcsdCrawler import AmcsdCrawler

logging.basicConfig()

# known crawler names, mapped to their corresponding class.
# if you make a new crawler, add it here.
crawlers = {
        # TODO:
        # change js to use caps
    'ICSD': IcsdCrawler,
    'AMCSD': AmcsdCrawler
}

def get_crawler():
    """
    Parse command-line arguments and return a crawler for the desired
    database, with the specified crawler arguments.
    """
    '''
    parser = Crawler.get_parser()

    # add required crawler parameter
    parser.add_argument('crawler',
                        choices=crawlers.iterkeys(),
                        help='which crawler to use')
    '''
    parser = argparse.ArgumentParser(description='Scrape an online database.')
    subparsers = parser.add_subparsers(help='database to scrape')

    # set up parsing for each crawler.
    for name, CrawlerClass in crawlers.iteritems():
        # add subparser for each crawler
        subparser = subparsers.add_parser(name, **CrawlerClass.parser_params)
        # register this CrawlerClass with its subparser
        subparser.set_defaults(Crawler=CrawlerClass)
        # add crawler-specific search-related and non-search-related parameters
        for params in (CrawlerClass.search_params, CrawlerClass.non_search_params):
            for param in params:
                subparser.add_argument(param.name, **param.config_pack)

    args = vars(parser.parse_args())
    Crawler = args['Crawler']

    # remove Crawler from args so constructor doesn't get confused.
    # it was only used for our convenience.
    del args['Crawler']

    '''
    # verify crawler argument validity
    if args.crawler not in crawlers:
        message = 'Not a valid crawler. Valid crawler arguments are: \n\n'
        for crawler in crawlers:
            message += crawler + ',\n'

        parser.error(message)
    '''

    # verify crawler-specific argument validity.
    status = Crawler.verify_args(args)
    if not status:
        parser.error(status.message)

    return Crawler(**args)

def main():
    crawler = get_crawler()
    results = crawler.crawl()
    print results.json
    print 'fetched {0} results.'.format(len(results))

if __name__ == '__main__':
    main()
