import sys
import re
import json
import urllib.parse
from html.parser import HTMLParser
from pathlib import Path, PurePath
from argparse import ArgumentParser

class NppExecDocParser(HTMLParser):
    CHARSET = 'cp1252'

    def __init__(self):
        super().__init__()
        self.topics = {}
        self.doc_title = ''
        self.content_chunks = []
        self.is_doc_title, self.is_content = False, False

    def handle_endtag(self, tag):
        # re-parse the parent element after a child is encountered,
        # otherwise the rest will be discarded
        if tag.lower() in ['a', 'b', 'code', 'em', 'i', 'pre', 'span', 'strong']:
            self.handle_starttag('p', [])

    def handle_starttag(self, tag: str, attrs: str):
        _tag = tag.lower()
        self.is_doc_title = (_tag == 'title')
        self.is_content = \
            re.match(r'h[1-6]', _tag) is not None or \
            _tag in ['b', 'blockquote', 'body', 'code', 'div', 'em', 'i',
                     'li', 'ol', 'p', 'pre', 'span', 'strong', 'ul', 'br']

    def handle_data(self, data: str):
        if self.is_doc_title:
            self.doc_title = data.strip()
            self.topics[self.doc_title] = { 'content': '' }
            self.content_chunks.clear()
            self.is_doc_title = False
        elif self.is_content and bool(data.strip()):
            try:
                def erase_word(match):
                    word = match.group(0)
                    return '' \
                        if match.group(0).lower() not in ['if'] \
                        else word
                is_non_word = \
                    r'(?i)\b([chw]*a[ndsty]*(re|nnot|n\'t)?|b(e|ut)?|do(es|n\'t)?|' \
                    r'etc|fr?om?r?|i[fn\'t]*s?|no[rt]*|(ab)?o(ut)?[fnr]*f?|s(ee|o(me)?)|' \
                    r't(o|he)|you[\'re]*)\b'
                text = ' '.join(re.sub(is_non_word, erase_word, data).split()).strip()
                self.content_chunks.append(text)
            finally:
                self.is_content = False

    def collect_info(self, root: Path, doc: Path):
        try:
            with open (doc, 'r', encoding=self.CHARSET) as d:
                self.feed(d.read())

            path = doc.relative_to(root)
            self.topics[self.doc_title]['path'] = "./{0}".format(urllib.parse.quote(str(path), safe='./:'))
            self.topics[self.doc_title]['content'] = " ".join(self.content_chunks)
        except (KeyError, AttributeError, IOError, ValueError) as e:
            print(repr(e), file=sys.stderr)
            sys.exit(2)


def include_finder_script(doc: Path, name: str, rewrite: bool):
    """
    Copy the script with `name` to the same directory as `doc`
    """
    with open (doc, 'r+', encoding=NppExecDocParser.CHARSET) as html:
        content = html.read()
        old_script = re.search(r'(?is)<script.*</script>', content)
        header = """<script src="{0}" type="text/javascript"></script>""".format(name)

        if old_script is not None:
            if not rewrite:
                return
            start, end = old_script.span()
            new_script = header.strip()
            content = content.replace(content[start:end], new_script)
            html.truncate(0)
        else:
            head = re.search(r'(?i)</HEAD>', content)
            if head is not None:
                endtag = head.group()
                content = content.replace(endtag, "{0}\n{1}".format(header, endtag))

        html.seek(0)
        html.write(content)

    try:
        src = Path(PurePath(sys.path[0]) / 'docs' / name)
        out = doc.parent / src.name
        if rewrite or not out.exists():
            out.write_bytes(src.read_bytes())
    except (AttributeError, TypeError, FileNotFoundError) as e:
        print(str(e), file=sys.stderr)
        sys.exit(2)


def run(format_opts: dict, script_opts: dict):
    parser = NppExecDocParser()
    docdir = Path(PurePath(sys.path[0]) / 'docs' / 'NppExec_Manual')
    topics = Path(PurePath(sys.path[0]) / 'docs' / 'NppExec_Manual' / 'topics.js')

    if not docdir.exists():
        print('No such directory:', docdir)
        sys.exit(1)

    print('Parsing docs in', docdir, '...')

    for doc in docdir.glob('**/*.html'):
        if doc.stem in ['toc', 'index']:
            continue
        if script_opts.get('add_script', False):
            rewrite = script_opts.get('update', False)
            name = script_opts.get('script_name', './NppExec_Manual/show_matches.js')
            include_finder_script(doc, name, rewrite)

        parser.collect_info(docdir, doc)

    print('Found', len(parser.topics.keys()), 'topics')

    with open (topics, 'w', encoding=NppExecDocParser.CHARSET) as out:
        out.write("var NPPEXEC_HELP_TOPICS = {0}".format(json.dumps(parser.topics, **format_opts)))


def parse_options():
    arg_parser = ArgumentParser(description="Extract metadata from NppExec's HTML docs.")
    arg_parser.add_argument('-i', '--indent-width', action='store', type=int,
        dest='indent', metavar='n', default=None,
        help='use %(metavar)s number of spaces to indent JSON properties')
    arg_parser.add_argument('-s', '--sort-keys', action='store_true',
        dest='sort_keys', default=False,
        help='sort JSON properties in ascending order')
    arg_parser.add_argument('-a', '--add-finder-script', action='store_true',
        dest='add_script', default=False,
        # must be given if '-u' is given
        required=bool(set(sys.argv).intersection(['-u', '--update-script'])),
        help='rewrite docs with a script to highlight matching search strings')
    arg_parser.add_argument('-u', '--update-script', action='store_true',
        dest='update_script', default=False,
        help='rewrite docs even if the <head> already has a <script> element')
    arg_parser.add_argument('-n', '--script-name', action='store', type=str,
        dest='script_name', metavar='<name>.js',default='./NppExec_Manual/show_matches.js',
        help='name of script to include in docs')
    args = arg_parser.parse_args()
    opts = vars(args)
    add_script, name, update = map(opts.pop, ('add_script', 'script_name', 'update_script'))
    script_opts = { 'add_script': add_script, 'script_name': name, 'update': update }
    return opts, script_opts


if __name__ == '__main__':
    run(*parse_options())
