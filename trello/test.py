#!/usr/bin/python

import sys
from trellosimple import TrelloSimple

# See README for documentation of the following two variables:
apikey = '8387ec96c89f2b594a7bbffac916fe82'
token = '1ce5c35ab2c4809799339367be4c53b083f99adbbca852e6e5eef332d0ed4323'

trello = TrelloSimple(apikey, token)

def printOverallInfo(org_name):
  print trello.get(['organizations', org_name])

def printNameAndDescr(org_name):
  print trello.get(['organizations', org_name, 'name'])
  print trello.get(['organizations', org_name, 'desc'])

def printBoards(org_name):
  print trello.get(['organizations', org_name, 'boards'])

def printBoardNames(org_name):
  boards = trello.get(['organizations', org_name, 'boards'])
  names = [board['name'] for board in boards]
  print names

def getBoardIdFromName(org_name, board_name):
  boards = trello.get(['organizations', org_name, 'boards'])
  board_id = None
  for board in boards:
     if board['name'] == board_name:
       board_id = board['id']
       break
  if board_id:
    return board_id
  else:
    print 'error: failed to get id of board >{}<'.format(board_name)
    sys.exit(1)

def printCardNamesFromBoardId(board_id):
  cards = trello.get(['boards', board_id, 'cards'])
  names = [card['name'] for card in cards]
  print names

def printCardNames(org_name, board_name):
    printCardNamesFromBoardId(getBoardIdFromName(org_name, board_name))

def printListNamesFromBoardId(board_id):
  lists = trello.get(['boards', board_id, 'lists'])
  names = [list['name'] for list in lists]
  print names

def printListNames(org_name, board_name):
  printListNamesFromBoardId(getBoardIdFromName(org_name, board_name))

# def getOrgIdFromName(org_name):
#   org = trello.get(['organizations', org_name])
#   print org
#   return org['id']

def addBoard(org_name, board_name):
  trello.post(
    ['boards'],
    arguments = {
      'name': board_name,
      'desc': 'description of the new board ... 1 2 3',
      'idOrganization': org_name,
      'prefs_permissionLevel': 'org'
      }
    )

def printMemberInfo(user_name):
  print trello.get(['members', user_name])

def setMemberFullName(user_name, full_name):
  trello.put(
    ['members', user_name, 'fullName'],
    arguments = {
      'value': full_name
      }
    )

#----------------------------------------------------------------

org_name = 'metorgtest' # same as the 'Short Name' of the organization in the Trello web app.
#printOverallInfo(org_name)
#printNameAndDescr(org_name)
#printBoards(org_name)
#printBoardNames(org_name)
board_name = 'Tasks - 21 January 2015'
#printCardNames(org_name, board_name)
#printListNames(org_name, board_name)
addBoard(org_name, 'my new board 6')
user_name = '_mt2' # same as the 'Username' of the member
#printMemberInfo(user_name)
#setMemberFullName(user_name, 'a_new_full_name_for__mt2')
