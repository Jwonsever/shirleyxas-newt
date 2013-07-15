#!/usr/bin/env python
import json

'''
    TODO: if num_results=0, the list should be empty.
    Instead, it contains a single empty string.
    Fix this.
'''

class Cif:
    """ Represents a single CIF file. """
    
    # character used at beginning and end of structures
    sep = '#'

    def __init__(self, contents):
        # TODO:
        # make this intializable from both a string and a file.
        # Will have to differentiate between contents strings and
        # file path strings.
        self.contents = contents

    def __repr__(self):
        return 'Cif({0})'.format(self.contents)

    def __str__(self):
        return self.contents


class CifList(list):
    """ Represents a list of CIF files."""

    def __init__(self, *files):
        """
        Files are either Cif instances or strings.
        Each file can contain an arbitrary number of scructures,
        each of which will be added separately.
        """
        # store as strings
        def to_str(obj):
            if type(obj) is str:
                return obj
            else:
                return str(obj)

        map(self.extend,
            map(self.separate,
                map(to_str,
                    files)))

    def separate(self, cif):
        """
        Separate a single (string of a) CIF file containing multiple structure
        into multiple files.
        """
        sep = Cif.sep
        length = len(cif)
        start, end = 0, 0

        while 0 <= end < length:
            start = cif.find(sep, end)
            end = cif.find(sep, start + 1)
            end = cif.find(sep, end + 1)
            yield cif[start : end]

    @property
    def json(self):
        """ Return a JSON representation of this list. """
        return json.dumps(self)
            

def test():
    from argparse import ArgumentParser

    parser = ArgumentParser()
    parser.add_argument('cif',
                        help='try and parse a CIF file into a CifList')

    args = parser.parse_args()
    cif = open(args.cif)
    cl = CifList(cif.read())
    print cl
    print len(cl)

if __name__ == "__main__":
    test()
