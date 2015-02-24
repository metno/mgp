#!/usr/bin/env python

import sys, os, re, urllib, json
from trellosimple import TrelloSimple

# --- BEGIN Global classes ----------------------------------------------

class Command:
    def writeOutput(self):
        self.writeOutputAsJSON()


# Lists the names of all available boards.
class GetBoardnames(Command):
    def __init__(self):
        pass

    def execute(self):
        self.boardnames = getBoardnames()
        self.writeOutput()

    def writeOutputAsJSON(self):
        json.dump(self.boardnames, sys.stdout, indent=2, ensure_ascii=True)
        sys.stdout.write('\n');


# Dumps all relevant info for a given board.
class DumpBoard(Command):
    def __init__(self, boardname):
        self.boardname = boardname

    def execute(self):
        self.board = getBoard(self.boardname)
        self.writeOutput()

    def writeOutputAsJSON(self):
        json.dump(self.board, sys.stdout, indent=2, ensure_ascii=True)
        sys.stdout.write('\n');


# Dumps relevant info for all boards.
class DumpAllBoards(Command):
    def __init__(self):
        pass

    def execute(self):
        self.boards = []
        for bname in getBoardnames():
            sys.stderr.write('appending board {} ...\n'.format(bname.encode('utf-8')))
            self.boards.append(getBoard(bname))
        self.writeOutput()

    def writeOutputAsJSON(self):
        json.dump(self.boards, sys.stdout, indent=2, ensure_ascii=True)
        sys.stdout.write('\n');

# --- END Global classes ----------------------------------------------


# --- BEGIN Global functions ----------------------------------------------

def getBoardnames():
    board_infos = trello.get(['organizations', org_name, 'boards'])
    return list(board_info['name'] for board_info in board_infos)

def getBoard(boardname):
    # get the board info
    board_info = getBoardInfoFromName(boardname)

    # get the action infos
    action_infos = trello.get(['boards', board_info['id'], 'actions'])
    # ... but only keep the 'commentCard' actions for now
    action_infos = list(x for x in action_infos if x['type'] == 'commentCard')

    # get the list infos
    list_infos = trello.get(['boards', board_info['id'], 'lists'])

    # get the card infos of each list
    card_infos = {}
    for list_info in list_infos:
        card_infos[list_info['id']] = trello.get(['lists', list_info['id'], 'cards'])

    return {
        'board_info': board_info,
        'action_infos': action_infos,
        'list_infos': list_infos,
        'card_infos': card_infos
        }

def getBoardInfoFromName(boardname):
  board_infos = trello.get(['organizations', org_name, 'boards'])
  for board_info in board_infos:
#     if board_info['name'] == unicode(boardname, 'utf-8'): # [1]
      if board_info['name'] == boardname: # [2]
          return board_info

# ### Problem: [1] must be used to support this use case:
#
#         $ ./metorg.py --cmd dump_board --board LØRDAG
#
#     while [2] must be used to support this use case:
#
#         $ ./metorg.py --cmd dump_all_boards
#

  print 'error: no such board: >{}<'.format(boardname)
  sys.exit(1)


# Returns an option dictionary is extracted from command-line arguments
# of the form ... --<opt1> <val1> ... --<optN> <valN> ...
def getOptions():
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

# Returns a command instance.
def createCommand(options):

    def printUsageError():
        error = (
            'usage: ' + sys.argv[0] + ' \\\n'
            '  --cmd get_boardnames | \\\n'
            '  --cmd dump_board --board B | \\\n'
            '  --cmd dump_all_boards | \\\n'
            '  --cmd create_board --src S --dst D')
        print error

    # check for mandatory 'cmd' argument
    if 'cmd' in options:
        cmd = options['cmd']
    else:
        printUsageError()
        sys.exit(1)

    # return the command if possible
    if cmd == 'get_boardnames':
        return GetBoardnames()
    elif cmd == 'dump_board':
        if 'board' in options:
            return DumpBoard(options['board'])
    elif cmd == 'dump_all_boards':
        return DumpAllBoards()
    elif cmd == 'create_board':
        if ('src' in options) and ('dst' in options):
            return CreateBoard(options['src'], options['dst'])

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

options = getOptions()
command = createCommand(options)
command.execute()

# --- END Main program ----------------------------------------------
