#!/usr/bin/env python

import sys, os, re, urllib, json, sh, fnmatch
from trellosimple import TrelloSimple

# --- BEGIN Global classes ----------------------------------------------

class Command:
    def printOutput(self):
        if self.http_get:
            printJSONHeader()
        self.printOutputAsJSON()


# Lists the ID and name of all open boards on the Trello server.
class GetLiveBoards(Command):
    def __init__(self, http_get, name_filter):
        self.http_get = http_get
        self.name_filter = name_filter.strip()
        if self.name_filter == '':
            self.name_filter = '*'

    def execute(self):
        self.board_id_and_names = getLiveBoardIdAndNames(self.name_filter.decode('utf-8'))
        self.printOutput()

    def printOutputAsJSON(self):
        json.dump({ 'boards': self.board_id_and_names }, sys.stdout, indent=2, ensure_ascii=True)
        sys.stdout.write('\n');


# Lists the ID and name of all available boards in the local backup directory.
class GetBackedupBoards(Command):
    def __init__(self, http_get, name_filter):
        self.http_get = http_get
        self.name_filter = name_filter.strip()
        if self.name_filter == '':
            self.name_filter = '*'

    def execute(self):
        self.board_id_and_names = getBackedupBoardIdAndNames(self.name_filter.decode('utf-8'))
        self.printOutput()

    def printOutputAsJSON(self):
        json.dump({ 'boards': self.board_id_and_names }, sys.stdout, indent=2, ensure_ascii=True)
        sys.stdout.write('\n');


# Prints relevant info for a given board on the Trello server.
class GetLiveBoard(Command):
    def __init__(self, http_get, board_id):
        self.http_get = http_get
        self.board_id = board_id

    def execute(self):
        self.board = getFullLiveBoard(self.board_id)
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


# Prints a HTML version of a board (based on template method pattern).
class GetBoardHtml(Command):
    def execute(self):
        board = self.getFullBoard()

        self.html = (
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

        self.html += (
            '<h1 style="display:inline">{}</h1> <span style="color:#444; font-size:120%">'
            '( {} )</span><br/><br/>').format(
            board['board']['name'].encode('utf-8'), self.board_id)

        cards = {}
        for lst in board['lists']:
            cards[lst['id']] = []
        for card in board['cards']:
            lid = card['idList']
            cards[lid].append(card)

        for lst in board['lists']:
            self.html += '<table>'
            self.html += '<tr><th colspan=2>{}</th></tr>'.format(lst['name'].encode('utf-8'))

            for card in cards[lst['id']]:
                self.html += '<tr><td>{}</td><td>{}</td></tr>'.format(card['name'].encode('utf-8'), card['desc'].encode('utf-8'))

            self.html += '</table>'
            self.html += '<br/>'

        self.html += '</body>'
        self.html += '</html>'

        self.printOutput()

    def printOutputAsJSON(self):
        json.dump({ 'html': self.html }, sys.stdout, indent=2, ensure_ascii=True)
        sys.stdout.write('\n');

# Prints a HTML version of a board.
class GetLiveBoardHtml(GetBoardHtml):
    def __init__(self, http_get, board_id):
        self.http_get = http_get
        self.board_id = board_id

    def getFullBoard(self):
        return getFullLiveBoard(self.board_id)

# Prints a HTML version of a board.
class GetBackedupBoardHtml(GetBoardHtml):
    def __init__(self, http_get, board_id):
        self.http_get = http_get
        self.board_id = board_id

    def getFullBoard(self):
        return getFullBackedupBoard(self.board_id)


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

# Backs up given board on the Trello server to a git repo (via the local backup directory).
class BackupLiveBoard(Command):
    def __init__(self, http_get, board_id):
        self.http_get = http_get
        self.board_id = board_id

    def execute(self):
        bname = getLiveBoard(self.board_id)['name']
        #sys.stderr.write('fetching board {} ({}) ... '.format(self.board_id, bname.encode('utf-8')))
        board = getFullLiveBoard(self.board_id)
        #sys.stderr.write('done\nbacking up ... ')
        self.commit = backupToGitRepo([board], getEnv('TRELLOBACKUPDIR'))
        #sys.stderr.write('done\n')
        self.printOutput()

    def printOutputAsJSON(self):
        json.dump({ 'commit': self.commit }, sys.stdout, indent=2, ensure_ascii=True)
        sys.stdout.write('\n');


# Backs up all open boards on the Trello server to a git repo (via the local backup directory).
class BackupAllLiveBoards(Command):
    def __init__(self, http_get):
        self.http_get = http_get

    def execute(self):
        boards = []
        for b in getLiveBoardIdAndNames():
            #sys.stderr.write('fetching board {} ({}) ... '.format(b['id'], b['name'].encode('utf-8')))
            boards.append(getFullLiveBoard(b['id']))
            #sys.stderr.write('done\n')
        #sys.stderr.write('backing up ... ')
        self.status = backupToGitRepo(boards, getEnv('TRELLOBACKUPDIR'))
        #sys.stderr.write('done\n')
        self.printOutput()

    def printOutputAsJSON(self):
        json.dump(self.status, sys.stdout, indent=2, ensure_ascii=True)
        sys.stdout.write('\n');


# Secures all open boards on the Trello server so that only admins may invite new members.
class SecureAllLiveBoards(Command):
    def __init__(self, http_get):
        self.http_get = http_get

    def execute(self):
        boards = []
        board_infos = getLiveBoardIdAndNames()
        self.sec_count = 0
        self.tot_count = len(board_infos)
        for b in board_infos:
            #sys.stderr.write('securing board {} ({}) ... '.format(b['id'], b['name'].encode('utf-8')))
            try:
                trello.put(
                    ['boards', b['id'], 'prefs', 'invitations'],
                    arguments = {
                        'value': 'admins'
                        }
                    )
                self.sec_count = self.sec_count + 1
                #sys.stderr.write('done\n')
            except:
                sys.stderr.write('failed: {}\n'.format(str(sys.exc_info())))
        self.printOutput()

    def printOutputAsJSON(self):
        json.dump(
            'secured {} of {} boards; only admins may invite new members to those'.format(
                self.sec_count, self.tot_count), sys.stdout, indent=2, ensure_ascii=True)
        sys.stdout.write('\n');


# Copies an existing open board on the Trello server to a new open board.
# The command fails if an open board with the same name already exists.
class CopyLiveBoard(Command):
    def __init__(self, http_get, src_id, dst_name):
        self.http_get = http_get
        self.src_id = src_id
        self.dst_name = dst_name.strip()
        self.status = None
        self.error = None

    def execute(self):
        board_infos = getLiveBoardIdAndNames()
        if self.dst_name in [item['name'] for item in board_infos]:
            # an open board with this name already exists
            self.error = 'an open board already exists with the name {}'.format(self.dst_name)
        elif self.dst_name == '':
            self.error = 'empty name'
        else:
            # copy board
            src_name = getBoardNameFromId(board_infos, self.src_id)
            trello.post(
                ['boards'],
                arguments = {
                    'name': self.dst_name,
                    'desc': 'copied from {} ({})'.format(src_name, self.src_id),
                    'idBoardSource': self.src_id,
                    'idOrganization': org_name,
                    'prefs_permissionLevel': 'org',
                    'prefs_comments': 'org',
                    'prefs_invitations': 'admins', # ensure that only admins may invite new members to the board
                    'prefs_selfJoin': 'false'
                    }
                )

            dst_id = getBoardIdFromName(getLiveBoardIdAndNames(), self.dst_name)

            addNonAdminOrgMembersToBoard(dst_id)

            self.status = 'successfully copied {} ({}) to {} ({})'.format(src_name, self.src_id, self.dst_name, dst_id)

        self.printOutput()

    def printOutputAsJSON(self):
        json.dump({ 'status': self.status, 'error': self.error }, sys.stdout, indent=2, ensure_ascii=True)
        sys.stdout.write('\n');


# Lists the ID and name of all members of the current organization on the Trello server.
class GetOrgMembers(Command):
    def __init__(self, http_get):
        self.http_get = http_get

    def execute(self):
        self.member_id_and_names = getOrgMemberIdAndNames()
        self.printOutput()

    def printOutputAsJSON(self):
        json.dump({ 'members': self.member_id_and_names }, sys.stdout, indent=2, ensure_ascii=True)
        sys.stdout.write('\n');


# Adds all non-admin members of the current organization to a given board on the Trello server.
class AddOrgMembersToBoard(Command):
    def __init__(self, http_get, board_id):
        self.http_get = http_get
        self.board_id = board_id

    def execute(self):
        self.status = addNonAdminOrgMembersToBoard(self.board_id)
        self.printOutput()

    def printOutputAsJSON(self):
        json.dump({ 'status': self.status }, sys.stdout, indent=2, ensure_ascii=True)
        sys.stdout.write('\n');

# --- END Global classes ----------------------------------------------


# --- BEGIN Global functions ----------------------------------------------

def getOrgId():
    return trello.get(['organizations', org_name, 'id'])

def getLiveBoardIdAndNames(name_filter = None):
    board_infos = trello.get(['organizations', org_name, 'boards'], arguments = { 'filter': 'open' })

    result = []
    for board_info in board_infos:
        name = board_info['name']
        if (name_filter == None) or fnmatch.fnmatch(name, name_filter):
            result.append({ 'id': board_info['id'], 'name': name })
    return result

    #return [{'id': board_info['id'], 'name': board_info['name']} for board_info in board_infos]

def getBoardNameFromId(bin_dicts, bid):
    result = filter(lambda d: d['id'] == bid, bin_dicts)
    if len(result) == 1:
        return result[0]['name']
    return None

def getBoardIdFromName(bin_dicts, bname):
    result = filter(lambda d: d['name'] == bname, bin_dicts)
    if len(result) == 1:
        return result[0]['id']
    return None

def getOrgMemberIdAndNames():
    return trello.get(['organizations', org_name, 'members'], arguments = { 'fields': 'username,fullName' })

def getLastCommitTime(gitdir, fname):
    git = sh.git.bake('--no-pager', _cwd=gitdir)
    try:
        last_ct = int(git.log('-1', '--pretty=format:%ct', fname))
    except:
        last_ct = -1
    return last_ct

def getBackedupBoardIdAndNames(name_filter = None):
    budir = getEnv('TRELLOBACKUPDIR')
    result = []
    for fname in os.listdir(budir):
        if fname.endswith(".json"):
            try:
                data = json.load(open('{}/{}'.format(budir, fname)))
                name = data['board']['name']
                if (name_filter == None) or fnmatch.fnmatch(name, name_filter):
                    last_ct = getLastCommitTime(budir, fname)
                    result.append({ 'id': data['board']['id'], 'name': name, 'last_ct': last_ct })
            except:
                pass # ignore parsing errors
    return result

def getLiveBoard(board_id):
    board = trello.get(['boards', board_id])
    return dict((k, board[k]) for k in ( # only keep certain attributes
            'id', 'name', 'prefs', 'closed'
            ))

def getLiveMembers(board_id):
    members = trello.get(['boards', board_id, 'members'])
    return members

def getLiveActions(board_id):
    actions = trello.get(['boards', board_id, 'actions'])
    actions = [x for x in actions if x['type'] == 'commentCard'] # for now
    return actions

def getLiveLists(board_id, filter='open'):
    lists = trello.get(['boards', board_id, 'lists'], arguments = { 'filter': filter })
    lists2 = []
    for lst in lists:
        lists2.append(dict((k, lst[k]) for k in ( # only keep certain attributes
                    'id', 'name'
                    )))
    return lists2

def getLiveListsAll(board_id):
    return getLiveLists(board_id, 'all')

def getLiveCards(board_id):
    cards = trello.get(['boards', board_id, 'cards'])
    cards2 = []
    for card in cards:
        cards2.append(dict((k, card[k]) for k in ( # only keep certain attributes
                    'id', 'name', 'desc', 'idList', 'labels', 'idLabels', 'idMembers'
                    )))
    return cards2

def getFullLiveBoard(board_id):
    board = getLiveBoard(board_id)
    members = getLiveMembers(board_id)
    actions = getLiveActions(board_id)
    lists = getLiveLists(board_id)
    cards = getLiveCards(board_id)
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
        git.commit('-a', '-m', 'updated backup') # ignore empty diff
        diff = True
        git.push()
        git.stash('pop') # ignore empty stash
    except:
        pass

    if diff:
        sha1 = git('rev-parse', 'HEAD').strip()
        return sha1
    return ''

def getLabelId(name, board_id, color):
    labels = trello.get(['boards', board_id, 'labels'], arguments = { 'fields': 'name,color' })
    for label in labels:
        if (label['name'] == name) and (label['color'] == color):
            return label['id'] # found

    # not found, so insert it
    trello.post(['boards', board_id, 'labels'], arguments = { 'name': name, 'color': color })
    # ... and find the ID
    labels = trello.get(['boards', board_id, 'labels'], arguments = { 'fields': 'name,color' })
    for label in labels:
        if (label['name'] == name) and (label['color'] == color):
            return label['id'] # found

    raise Exception('inserted label not found')

# Adds all non-admin members of the current organization to a board on the Trello server.
def addNonAdminOrgMembersToBoard(board_id):

    # get current board members
    board_member_ids = [item['id'] for item in getLiveMembers(board_id)]

    # add org
    nadded = 0
    for item in getOrgMemberIdAndNames():
        if item['username'] == 'metorg_adm':
            continue
        if item['id'] not in board_member_ids:
            trello.put(
                ['boards', board_id, 'members', item['id']],
                arguments = {
                    'idMember': item['id'],
                    'type': 'normal'
                    }
                )
            nadded = nadded + 1

    return 'added {} member{} of organization {} to board {} ({})'.format(
        nadded, '' if (nadded == 1) else 's', org_name, board_id, getBoardNameFromId(getLiveBoardIdAndNames(), board_id))

def printJSONHeader():
    sys.stdout.write('Content-type: text/json\n\n')

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
                '--cmd get_live_boards',
                '--cmd get_backedup_boards [--filter <board name filter>]',
                '--cmd get_live_board --id <board ID>',
                '--cmd get_live_board_html --id <board ID>',
                '--cmd get_backedup_board --id <board ID>',
                '--cmd get_backedup_board_html --id <board ID>',
                '--cmd get_backedup_board_stats --id <board ID>',
                '--cmd backup_live_board --id <board ID>',
                '--cmd backup_all_live_boards',
                '--cmd secure_all_live_boards',
                '--cmd copy_live_board --src_id <source board ID> --dst_name <destination board name>',
                '--cmd get_org_members',
                '--cmd add_org_members_to_board --id <board ID>'
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
    if cmd == 'get_live_boards':
        return GetLiveBoards(http_get, options['filter'] if 'filter' in options else '')
    elif cmd == 'get_backedup_boards':
        return GetBackedupBoards(http_get, options['filter'] if 'filter' in options else '')
    elif cmd == 'get_live_board':
        if 'id' in options:
            return GetLiveBoard(http_get, options['id'])
    elif cmd == 'get_live_board_html':
        if 'id' in options:
            return GetLiveBoardHtml(http_get, options['id'])
    elif cmd == 'get_backedup_board':
        if 'id' in options:
            return GetBackedupBoard(http_get, options['id'])
    elif cmd == 'get_backedup_board_html':
        if 'id' in options:
            return GetBackedupBoardHtml(http_get, options['id'])
    elif cmd == 'get_backedup_board_stats':
        if 'id' in options:
            return GetBackedupBoardStats(http_get, options['id'])
    elif cmd == 'backup_live_board':
        if 'id' in options:
            return BackupLiveBoard(http_get, options['id'])
    elif cmd == 'backup_all_live_boards':
        return BackupAllLiveBoards(http_get)
    elif cmd == 'secure_all_live_boards':
        return SecureAllLiveBoards(http_get)
    elif cmd == 'copy_live_board':
        if ('src_id' in options) and ('dst_name' in options):
            return CopyLiveBoard(http_get, options['src_id'], options['dst_name'])
    elif cmd == 'get_org_members':
        return GetOrgMembers(http_get)
    elif cmd == 'add_org_members_to_board':
        if 'id' in options:
            return AddOrgMembersToBoard(http_get, options['id'])

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
