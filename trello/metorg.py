#!/usr/bin/env python

import sys, os, re, urllib, json, sh
from trellosimple import TrelloSimple

# --- BEGIN Global classes ----------------------------------------------

class Command:
    def printOutput(self):
        if self.http_get:
            printJSONHeader()
        self.printOutputAsJSON()


# Lists the ID and name of all available boards.
class GetBoards(Command):
    def __init__(self, http_get):
        self.http_get = http_get

    def execute(self):
        self.board_id_and_names = getBoardIdAndNames()
        self.printOutput()

    def printOutputAsJSON(self):
        json.dump(self.board_id_and_names, sys.stdout, indent=2, ensure_ascii=True)
        sys.stdout.write('\n');


# Dumps all relevant info for a given board.
class DumpBoard(Command):
    def __init__(self, http_get, board_id):
        self.http_get = http_get
        self.board_id = board_id

    def execute(self):
        self.board = getFullBoard(self.board_id)
        self.printOutput()

    def printOutputAsJSON(self):
        json.dump(self.board, sys.stdout, indent=2, ensure_ascii=True)
        sys.stdout.write('\n');

# Dumps relevant info for all boards.
class DumpAllBoards(Command):
    def __init__(self, http_get):
        self.http_get = http_get

    def execute(self):
        self.boards = []
        for b in getBoardIdAndNames():
            sys.stderr.write('appending board {} (id:{}) ...\n'.format(b['name'].encode('utf-8'), b['id']))
            self.boards.append(getFullBoard(b['id']))
        self.printOutput()

    def printOutputAsJSON(self):
        json.dump(self.boards, sys.stdout, indent=2, ensure_ascii=True)
        sys.stdout.write('\n');


# Backs up given board to a git repo.
class BackupBoard(Command):
    def __init__(self, http_get, board_id):
        self.http_get = http_get
        self.board_id = board_id

    def execute(self):
        bname = getBoard(self.board_id)['name']
        sys.stderr.write('fetching board {} ({}) ... '.format(self.board_id, bname.encode('utf-8')))
        board = getFullBoard(self.board_id)
        sys.stderr.write('done\nbacking up ... ')
        self.status = backupToGitRepo(board, getEnv('METORGGITDIR'), 'metorg.json')
        sys.stderr.write('done\n')
        self.printOutput()

    def printOutputAsJSON(self):
        json.dump(self.status, sys.stdout, indent=2, ensure_ascii=True)
        sys.stdout.write('\n');


# Backs up all boards to a git repo.
class BackupAllBoards(Command):
    def __init__(self, http_get):
        self.http_get = http_get

    def execute(self):
        boards = []
        for b in getBoardIdAndNames():
            sys.stderr.write('fetching board {} ({}) ... '.format(b['id'], b['name'].encode('utf-8')))
            boards.append(getFullBoard(b['id']))
            sys.stderr.write('done\n')
        sys.stderr.write('backing up ... ')
        self.status = backupToGitRepo(boards, getEnv('METORGGITDIR'), 'metorg.json')
        sys.stderr.write('done\n')
        self.printOutput()

    def printOutputAsJSON(self):
        json.dump(self.status, sys.stdout, indent=2, ensure_ascii=True)
        sys.stdout.write('\n');

# --- END Global classes ----------------------------------------------


# --- BEGIN Global functions ----------------------------------------------

def getBoardIdAndNames():
    board_infos = trello.get(['organizations', org_name, 'boards'])
    return list({'id': board_info['id'], 'name': board_info['name']} for board_info in board_infos)

def getBoard(board_id):
    board = trello.get(['boards', board_id])
    return dict((k, board[k]) for k in ( # only keep certain attributes
            'id', 'name', 'prefs'
            ))

def getMembers(board_id):
    members = trello.get(['boards', board_id, 'members'])
    return members

def getActions(board_id):
    actions = trello.get(['boards', board_id, 'actions'])
    actions = list(x for x in actions if x['type'] == 'commentCard') # for now
    return actions

def getLists(board_id):
    lists = trello.get(['boards', board_id, 'lists'])
    lists2 = []
    for lst in lists:
        lists2.append(dict((k, lst[k]) for k in ( # only keep certain attributes
                    'id', 'name'
                    )))
    return lists2

def getCards(list_id):
    cards = trello.get(['lists', list_id, 'cards'])
    cards2 = []
    for card in cards:
        cards2.append(dict((k, card[k]) for k in ( # only keep certain attributes
                    'id', 'name', 'desc', 'labels', 'idLabels', 'idMembers'
                    )))
    return cards2

def getFullBoard(board_id):
    board = getBoard(board_id)
    members = getMembers(board_id)
    actions = getActions(board_id)
    lists = getLists(board_id)
    cards = {}
    for lst in lists:
        cards[lst['id']] = getCards(lst['id'])
    return {
        'board': board,
        'members': members,
        'actions': actions,
        'lists': lists,
        'cards': cards
        }

def backupToGitRepo(data, gitdir, fname):
    git = sh.git.bake(_cwd=gitdir)
    git.checkout('master')
    git.stash()
    fpath = '{}/{}'.format(gitdir, fname)
    json.dump(data, open(fpath, 'w'), indent=2, ensure_ascii=True)
    git.add(fname)

    diff = False
    try:
        git.commit('-a', '-m', 'updated metorg backup') # ignore empty diff
        diff = True
        git.push()
        git.stash('pop') # ignore empty stash
    except:
        pass

    if diff:
        sha1 = git('rev-parse', 'HEAD').strip()
        return 'updated backup {} with Commit {}'.format(fpath, sha1)
    return 'no difference since last backup'

def printJSONHeader():
    sys.stdout.write('Content-type: text/json\n')

def printErrorAsJSON(error):
    sys.stdout.write('{\"error\": \"' + error + '\"}\n')

# Returns a 2-tuple consisting of:
# 1: an option dictionary, and
# 2: a flag that is true iff the QUERY_STRING environment variable is
#    present (i.e. that the script is invoked as a CGI-script for a
#    HTTP GET request).
#
# The option dictionary is extracted from either the QUERY_STRING environment
# variable (first priority) or command-line arguments (second priority).
# In the latter case, the options must be of the form
# ... --<opt1> <val1> ... --<optN> <valN> ...
def getOptions():

    def getOptDictFromQueryString(qs):
        options = {}
        for sq in qs.split('&'):
            keyval = sq.split('=')
            options[keyval[0]] = urllib.unquote(keyval[1])
        return options

    def getOptDictFromCommandLine():
        options = {}
        p = re.compile('^--(.+)$')
        key = None
        for arg in sys.argv[1:]:
            if key != None:
                options[key] = arg
            m = p.match(arg)
            if m:
                key = m.group(1)
                if key in ['help', 'force']: # value-less options
                    options[key] = 1
                    key = None
            else:
                key = None
        return options

    qs = 'QUERY_STRING'
    if qs in os.environ:
        return (getOptDictFromQueryString(os.environ[qs]), True)
    else:
        return (getOptDictFromCommandLine(), False)


# Returns a command instance.
def createCommand(options, http_get):

    def printUsageError():
        error = (
            'usage: ' + sys.argv[0] +
            ' --cmd get_boards |'
            ' --cmd dump_board --id <board ID> |'
            ' --cmd dump_all_boards |'
            ' --cmd backup_board --id <board ID> |'
            ' --cmd backup_all_boards |'
            ' --cmd create_board --id <source board ID> --name <destination board name>')
        if http_get:
            printJSONHeader()
        printErrorAsJSON(error)

    # check for mandatory 'cmd' argument
    if 'cmd' in options:
        cmd = options['cmd']
    else:
        printUsageError()
        sys.exit(1)

    # return the command if possible
    if cmd == 'get_boards':
        return GetBoards(http_get)
    elif cmd == 'dump_board':
        if 'bid' in options:
            return DumpBoard(http_get, options['bid'])
    elif cmd == 'dump_all_boards':
        return DumpAllBoards(http_get)
    elif cmd == 'backup_board':
        if 'bid' in options:
            return BackupBoard(http_get, options['bid'])
    elif cmd == 'backup_all_boards':
        return BackupAllBoards(http_get)
    elif cmd == 'create_board':
        if ('src' in options) and ('dst' in options):
            return CreateBoard(http_get, options['src'], options['dst'])

    # no match
    printUsageError()
    sys.exit(1)

def getEnv(key):
    if not key in os.environ:
        sys.stderr.write('environment variable not found: {}\n'.format(key))
        sys.exit(1)
    return os.environ[key]

# --- END Global functions ----------------------------------------------


# --- BEGIN Main program ----------------------------------------------

apikey = getEnv('TRELLOAPIKEY')
token = getEnv('TRELLOTOKEN')
org_name = getEnv('TRELLOORG')

trello = TrelloSimple(apikey, token)

options, http_get = getOptions()
command = createCommand(options, http_get)
try:
    command.execute()
except:
    printErrorAsJSON(str(sys.exc_info()))
    sys.exit(1)

sys.exit(0)

# --- END Main program ----------------------------------------------
