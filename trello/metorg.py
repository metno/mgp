#!/usr/bin/env python

import sys, os, re, urllib, json, sh
from trellosimple import TrelloSimple

# --- BEGIN Global classes ----------------------------------------------

class Command:
    def printOutput(self):
        if self.http_get:
            printJSONHeader()
        self.printOutputAsJSON()


# Lists the ID and name of all available boards on the Trello server.
class GetBoards(Command):
    def __init__(self, http_get):
        self.http_get = http_get

    def execute(self):
        self.board_id_and_names = getBoardIdAndNames()
        self.printOutput()

    def printOutputAsJSON(self):
        json.dump(self.board_id_and_names, sys.stdout, indent=2, ensure_ascii=True)
        sys.stdout.write('\n');


# Lists the ID and name of all available boards in the local backup directory.
class GetBackedupBoards(Command):
    def __init__(self, http_get):
        self.http_get = http_get

    def execute(self):
        self.board_id_and_names = getBackedupBoardIdAndNames()
        self.printOutput()

    def printOutputAsJSON(self):
        json.dump(self.board_id_and_names, sys.stdout, indent=2, ensure_ascii=True)
        sys.stdout.write('\n');


# Prints relevant info for a given board on the Trello server.
class GetBoard(Command):
    def __init__(self, http_get, board_id):
        self.http_get = http_get
        self.board_id = board_id

    def execute(self):
        self.board = getFullBoard(self.board_id)
        self.printOutput()

    def printOutputAsJSON(self):
        json.dump(self.board, sys.stdout, indent=2, ensure_ascii=True)
        sys.stdout.write('\n');


# Prints relevant info for a given board in the local backup directory.
class GetBackedupBoard(Command):
    def __init__(self, http_get, board_id):
        self.http_get = http_get
        self.board_id = board_id

    def execute(self):
        self.board = getFullBackedupBoard(self.board_id)
        self.printOutput()

    def printOutputAsJSON(self):
        json.dump(self.board, sys.stdout, indent=2, ensure_ascii=True)
        sys.stdout.write('\n');


# Prints a HTML version of a given board in the local backup directory.
class GetBackedupBoardHtml(Command):
    def __init__(self, http_get, board_id):
        self.http_get = http_get
        self.board_id = board_id

    def execute(self):
        board = getFullBackedupBoard(self.board_id)

        html = (
            '<html>'
            '<head>'
            '<meta charset="UTF-8">'
            '<style>'
            'table { border-collapse:collapse; font-size:12px; }'
            'table, th, td { border: 1px solid #aaaaaa; padding:3; text-align:left; vertical-align:text-top; }'
            'td { white-space:pre; }'
            'th { background: #eee; font-size:120%; }'
            '</style>'
            '</head>'
            '<body>'
            )

        html += '<h1>{}</h1>'.format(board['board']['name'].encode('utf-8'))

        for lst in board['lists']:
            html += '<table>'
            html += '<tr><th colspan=2>{}</th></tr>'.format(lst['name'].encode('utf-8'))

            for card in board['cards'][lst['id']]:
                html += '<tr><td>{}</td><td>{}</td></tr>'.format(card['name'].encode('utf-8'), card['desc'].encode('utf-8'))

            html += '</table>'
            html += '<br/>'

        html += '</body>'
        html += '</html>'
        sys.stdout.write(html)


# Prints stats for a given board in the local backup directory.
class GetBackedupBoardStats(Command):
    def __init__(self, http_get, board_id):
        self.http_get = http_get
        self.board_id = board_id

    def execute(self):
        board = getFullBackedupBoard(self.board_id)
        self.stats = '<stats not implemented yet>'
        self.printOutput()

    def printOutputAsJSON(self):
        json.dump(self.stats, sys.stdout, indent=2, ensure_ascii=True)
        sys.stdout.write('\n');

# Backs up given board to a git repo (via the local backup directory).
class BackupBoard(Command):
    def __init__(self, http_get, board_id):
        self.http_get = http_get
        self.board_id = board_id

    def execute(self):
        bname = getBoard(self.board_id)['name']
        sys.stderr.write('fetching board {} ({}) ... '.format(self.board_id, bname.encode('utf-8')))
        board = getFullBoard(self.board_id)
        sys.stderr.write('done\nbacking up ... ')
        self.status = backupToGitRepo([board], getEnv('TRELLOBACKUPDIR'))
        sys.stderr.write('done\n')
        self.printOutput()

    def printOutputAsJSON(self):
        json.dump(self.status, sys.stdout, indent=2, ensure_ascii=True)
        sys.stdout.write('\n');


# Backs up all boards to a git repo (via the local backup directory).
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
        self.status = backupToGitRepo(boards, getEnv('TRELLOBACKUPDIR'))
        sys.stderr.write('done\n')
        self.printOutput()

    def printOutputAsJSON(self):
        json.dump(self.status, sys.stdout, indent=2, ensure_ascii=True)
        sys.stdout.write('\n');


# Secure all boards on the Trello server so that only admins may invite new members.
class SecureAllBoards(Command):
    def __init__(self, http_get):
        self.http_get = http_get

    def execute(self):
        boards = []
        board_infos = getBoardIdAndNames()
        self.sec_count = 0
        self.tot_count = len(board_infos)
        for b in board_infos:
            sys.stderr.write('securing board {} ({}) ... '.format(b['id'], b['name'].encode('utf-8')))
            try:
                trello.put(
                    ['boards', b['id'], 'prefs', 'invitations'],
                    arguments = {
                        'value': 'admins'
                        }
                    )
                self.sec_count = self.sec_count + 1
                sys.stderr.write('done\n')
            except:
                sys.stderr.write('failed: {}\n'.format(str(sys.exc_info())))
        self.printOutput()

    def printOutputAsJSON(self):
        json.dump(
            'secured {} of {} boards; only admins may invite new members to those'.format(
                self.sec_count, self.tot_count), sys.stdout, indent=2, ensure_ascii=True)
        sys.stdout.write('\n');


# Initializes a board on the Trello server based on a source board in the local backup directory.
# - The board will be created if it doesn't already exist.
#
# - When merging the source board with an existing destination board, the contents of the latter will
#   take priority over the former to ensure that any updates made manually via the Trello web GUI are kept.
#   For example, if the destination board contains a card with the same name as a card in the source board, and the
#   two cards are in lists with the same name, the card in the source board will not overwrite the one in the
#   destination board (even if other card attributes, like the description, have changed).
#
# - Lists and cards that exist only in the destination board will be kept.
#
# - Labels will be reset to an sensible intial state.
# 
# - Cards will be moved to initial lists based on metadata in the card description.
#
class InitBoard(Command):
    def __init__(self, http_get, src_id, dst_name):
        self.http_get = http_get
        self.src_id = src_id
        self.dst_name = dst_name

    def execute(self):
        src_board = getFullBackedupBoard(self.src_id) # get source board

        board_infos = getBoardIdAndNames()
        if not self.dst_name in [item['name'] for item in board_infos]:
            # board did not exist, so create it
            sys.stderr.write('creating new board ... ')
            trello.post(
                ['boards'],
                arguments = {
                    'name': self.dst_name,
                    'desc': 'copied from {}'.format(src_board['board']['name']),
                    'idOrganization': org_name,
                    'prefs_permissionLevel': 'org',
                    'prefs_comments': 'org',
                    'prefs_invitations': 'admins', # ensure that only admins may invite new members to the board
                    'prefs_selfJoin': 'false'
                    }
                )
            sys.stderr.write('done\n')

            board_infos = getBoardIdAndNames() # recompute and expect the new board to be present this time
        else:
            sys.stderr.write('updating existing board\n')

        # get ID of destination board
        assert self.dst_name in [item['name'] for item in board_infos]
        dst_id = next((item['id'] for item in board_infos if item['name'] == self.dst_name), None)

        # save original state of destination board (so that we can merge in that information later)
        dst_board = getFullBoard(dst_id)

        # open destination board
        sys.stderr.write('open board ... ')
        trello.put(
            ['boards', dst_id, 'closed'],
            arguments = {
                'value': 'false'
                }
            )
        sys.stderr.write('done\n')

        # For each list in the source board that doesn't already exist (i.e. with the same name)
        # in the destination board, create an empty destination list with this name and
        # insert it at the last/rightmost position ... TBD
        sys.stderr.write('copy lists ... ')
        src_lnames = [item['name'] for item in src_board['lists']]
        dst_lnames = [item['name'] for item in dst_board['lists']]
        for src_lname in src_lnames:
            if src_lname not in dst_lnames:
                trello.post(
                    ['boards', dst_id, 'lists'],
                    arguments = {
                        'name': src_lname,
                        'pos': 'bottom'
                        }
                    )
        sys.stderr.write('done\n')


        # For each card SC in the source board (regardless of list), copy the card to the list DL in the
        # destination board with a name that matches the value of 'standardvakt' in the description of SC,
        # unless a card with this name doesn't already exist in DL.
        # Cards that don't have such a value, or where the value doesn't match any list name, are copied to a
        # special list 'ukjent'.
        # HM ... what if there are more than one corresponding destination list (i.e. having the same name)?
        #   -> only consider the first/leftmost one? (yup!)

        # ALSO handle labels (replace 'ferdig' with 'ikke paabegynt')
        # ALSO clear actions in each card


        pass ### TBD

# --- END Global classes ----------------------------------------------


# --- BEGIN Global functions ----------------------------------------------

def getOrgId():
    return trello.get(['organizations', org_name, 'id'])

def getBoardIdAndNames():
    board_infos = trello.get(['organizations', org_name, 'boards'])
    return [{'id': board_info['id'], 'name': board_info['name']} for board_info in board_infos]

def getBackedupBoardIdAndNames():
    budir = getEnv('TRELLOBACKUPDIR')
    result = []
    for fname in os.listdir(budir):
        if fname.endswith(".json"):
            try:
                data = json.load(open('{}/{}'.format(budir, fname)))
                result.append({'id': data['board']['id'], 'name': data['board']['name']})
            except:
                pass # ignore parsing errors
    return result

def getBoard(board_id):
    board = trello.get(['boards', board_id])
    return dict((k, board[k]) for k in ( # only keep certain attributes
            'id', 'name', 'prefs', 'closed'
            ))

def getMembers(board_id):
    members = trello.get(['boards', board_id, 'members'])
    return members

def getActions(board_id):
    actions = trello.get(['boards', board_id, 'actions'])
    actions = [x for x in actions if x['type'] == 'commentCard'] # for now
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

def getFullBackedupBoard(board_id):
    return json.load(open('{}/{}.json'.format(getEnv('TRELLOBACKUPDIR'), board_id)))

def backupToGitRepo(boards, gitdir):
    git = sh.git.bake(_cwd=gitdir)
    git.checkout('master')
    git.stash()
    for board in boards:
        fpath = '{}/{}.json'.format(gitdir, board['board']['id'])
        json.dump(board, open(fpath, 'w'), indent=2, ensure_ascii=True)
        git.add(fpath)

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

def printErrorAsJSON(error, http_get):
    if http_get:
        printJSONHeader()
    json.dump({ 'error': error }, sys.stdout if http_get else sys.stderr, indent=2, ensure_ascii=True)
    if not http_get:
        sys.stderr.write('\n')

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
        error = {
            'argv0': sys.argv[0],
            'commands': [
                '--cmd get_boards',
                '--cmd get_backedup_boards',
                '--cmd get_board --id <board ID>',
                '--cmd get_backedup_board --id <board ID>',
                '--cmd get_backedup_board_html --id <board ID>',
                '--cmd get_backedup_board_stats --id <board ID>',
                '--cmd backup_board --id <board ID>',
                '--cmd backup_all_boards',
                '--cmd init_board --src_id <source board ID> --dst_name <destination board name>'
                ]
            }
        printErrorAsJSON(error, http_get)

    # check for mandatory 'cmd' argument
    if 'cmd' in options:
        cmd = options['cmd']
    else:
        printUsageError()
        sys.exit(1)

    # return the command if possible
    if cmd == 'get_boards':
        return GetBoards(http_get)
    elif cmd == 'get_backedup_boards':
        return GetBackedupBoards(http_get)
    elif cmd == 'get_board':
        if 'bid' in options:
            return GetBoard(http_get, options['bid'])
    elif cmd == 'get_backedup_board':
        if 'bid' in options:
            return GetBackedupBoard(http_get, options['bid'])
    elif cmd == 'get_backedup_board_html':
        if 'bid' in options:
            return GetBackedupBoardHtml(http_get, options['bid'])
    elif cmd == 'get_backedup_board_stats':
        if 'bid' in options:
            return GetBackedupBoardStats(http_get, options['bid'])
    elif cmd == 'backup_board':
        if 'bid' in options:
            return BackupBoard(http_get, options['bid'])
    elif cmd == 'backup_all_boards':
        return BackupAllBoards(http_get)
    elif cmd == 'secure_all_boards':
        return SecureAllBoards(http_get)
    elif cmd == 'init_board':
        if ('src_id' in options) and ('dst_name' in options):
            return InitBoard(http_get, options['src_id'], options['dst_name'])

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
    printErrorAsJSON(str(sys.exc_info()), http_get)
    sys.exit(1)

sys.exit(0)

# --- END Main program ----------------------------------------------
