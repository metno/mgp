# This file is taken from https://developers.kilnhg.com/Code/Trello/Group/TrelloSimple

try:
  import simplejson as json
except ImportError:
  import json
import requests
from urllib import quote_plus

class TrelloSimple(object):  
  def __init__(self, apikey, token=None):
    self._apikey = apikey
    self._token = token
    self._apiversion = 1
    self._proxies = None

  def set_token(self, token):
    self._token = token

  def set_proxy(self, proxies):
    '''e.g. proxies={'https':'127.0.0.1:8888'}'''
    self._proxies = proxies

  def get_token_url(self, app_name, expires='30days', write_access=True, scope_override = ''):
    scope = ''
    if scope_override != '':
      scope = scope_override
    else:
      scope = 'read,write' if write_access else 'read'
    return 'https://trello.com/1/authorize?key=%s&name=%s&expiration=%s&response_type=token&scope=%s' % (
      self._apikey, quote_plus(app_name), expires, scope)

  def get(self, urlPieces, arguments = None):
    return self._http_action('get',urlPieces, arguments)

  def put(self, urlPieces, arguments = None, files=None):
    return self._http_action('put',urlPieces, arguments,files)

  def post(self, urlPieces, arguments = None, files=None):
    return self._http_action('post',urlPieces, arguments,files)

  def delete(self, urlPieces, arguments = None):
    return self._http_action('delete',urlPieces, arguments)

  def _http_action(self, method, urlPieces, arguments = None, files=None):
    #If the user wants to pass in a formatted string for urlPieces, just use
    #the string. Otherwise, assume we have a list of strings and join with /.
    if not isinstance(urlPieces, basestring):
      urlPieces = '/'.join(urlPieces)
    baseUrl = 'https://trello.com/%s/%s' % (self._apiversion, urlPieces) 

    params = {'key':self._apikey,'token':self._token}

    if method in ['get','delete'] and arguments:
      params = dict(params.items() + arguments.items())
    if method == 'get':
      resp = requests.get(baseUrl,params=params, proxies=self._proxies)
    elif method == 'delete':
      resp = requests.delete(baseUrl,params=params, proxies=self._proxies)
    elif method == 'put':
      resp = requests.put(baseUrl,params=params,data=arguments, proxies=self._proxies,files=files)
    elif method == 'post':
      resp = requests.post(baseUrl,params=params,data=arguments, proxies=self._proxies, files=files)

    resp.raise_for_status()
    return json.loads(resp.content)
