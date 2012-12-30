# -*- coding: utf-8 -*-
#
# Copyright (C) 2012 PedroHLC <pedro.laracampos@gmail.com>
#
# This file is part of OSD Lyrics.
#
# OSD Lyrics is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# OSD Lyrics is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with OSD Lyrics. If not, see <http://www.gnu.org/licenses/>.
#/

import re
import httplib
import urllib
import urlparse
import hashlib
from xml.dom.minidom import parseString
from osdlyrics.lyricsource import BaseLyricSourcePlugin, SearchResult
from osdlyrics.utils import ensure_utf8, http_download, get_proxy_settings

VIEWLYRICS_HOST = 'search.crintsoft.com'
VIEWLYRICS_SEARCH_URL = '/searchlyrics.htm'

VIEWLYRICS_QUERY_FORM = '<?xml version=\'1.0\' encoding=\'utf-8\' standalone=\'yes\' ?><search filetype=\"lyrics\" artist=\"%artist\" title=\"%title\"%etc />'
VIEWLYRICS_AGENT = 'MiniLyrics'
VIEWLYRICS_KEY = 'Mlv1clt4.0'

class ViewlyricsSource(BaseLyricSourcePlugin):
    def __init__(self):
        
        BaseLyricSourcePlugin.__init__(self, id='viewlyrics', name='viewlyrics')

    def do_search(self, metadata):
        keys = []
        query = VIEWLYRICS_QUERY_FORM
        if metadata.title:
            query =  query.replace('%title', metadata.title)
        else:
            query =  query.replace('%title', '')
        if metadata.artist:
            query =  query.replace('%artist', metadata.artist)
        else:
            query =  query.replace('%artist', '')
        
        query =  ensure_utf8(query.replace('%etc', ' client=\"MiniLyrics\" RequestPage=\'0\'')) #Needs real RequestPage
        
        queryhash = hashlib.md5()
        queryhash.update(query)
        queryhash.update(VIEWLYRICS_KEY)
        
        masterquery = '\2\0\4\0\0\0' + queryhash.digest() + query
        
        url = VIEWLYRICS_HOST + VIEWLYRICS_SEARCH_URL
        status, content = http_download(url=url,
                                        method='POST',
                                        params=masterquery
                                        #,headers={'User-Agent': VIEWLYRICS_AGENT}
                                        #,proxy=get_proxy_settings(self.config_proxy))
                                        )
        
        if status < 200 or status >= 400:
                raise httplib.HTTPException(status, '')
        
        contentbytes = map(ord, content)
        codekey = contentbytes[1]
        deccontent = ''
        for char in contentbytes[22:]:
                deccontent += unichr(char ^ codekey)
        
        result = []
        tagreturn = parseString(deccontent).getElementsByTagName('return')[0]
        if tagreturn:
                tagsfileinfo = tagreturn.getElementsByTagName('fileinfo')
                if tagsfileinfo:
                    for onefileinfo in tagsfileinfo:
                        #
                                title = onefileinfo.attributes['title'].value
                                artist = onefileinfo.attributes['artist'].value
                                album = onefileinfo.attributes['title'].value
                                url = onefileinfo.attributes['link'].value
                                if url is not None:
                                        result.append(SearchResult(title=title,
                                                artist=artist,
                                                album=album,
                                                sourceid=self.id,
                                                downloadinfo=url))
        return result

    def do_download(self, downloadinfo):
        # return a string
        # downloadinfo is what you set in SearchResult
        if not isinstance(downloadinfo, str) and \
                not isinstance(downloadinfo, unicode):
            raise TypeError('Expect the downloadinfo as a string of url, but got type ',
                            type(downloadinfo))
        status, content = http_download(url=downloadinfo,
                                        proxy=get_proxy_settings(self.config_proxy))
        if status < 200 or status >= 400:
            raise httplib.HTTPException(status, '')
        return content

if __name__ == '__main__':
    viewlyrics = ViewlyricsSource()
    viewlyrics._app.run()