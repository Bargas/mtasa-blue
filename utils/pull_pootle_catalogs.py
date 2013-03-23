##############################################################################
#
#  PROJECT:     Multi Theft Auto v1.0
#  LICENSE:     See LICENSE in the top level directory
#  FILE:        utils/build_gettext_catalog.py
#  PURPOSE:     Pull translation files from a Pootle web server into a directory
#  DEVELOPERS:  Dan Chowdhury <>
#
#  Multi Theft Auto is available from http://www.multitheftauto.com/
#
##############################################################################

import os
import json
import shutil
import urllib2
import urlparse
from optparse import OptionParser

parser = OptionParser()
parser.add_option("-u", "--url", dest="url",
            help="Pootle homepage URL", default="http://translate.mtasa.com/")
parser.add_option("-o", "--output", dest="output",
            help="Output directory", default="../output/MTA San Andreas/mta/locale")
parser.add_option("-p", "--project", dest="project",
            help="Internal name of the pootle project", default="client")
parser.add_option("-G", "--gnu",action="store_true", dest="gnu", default=False,
            help="Whether the project is a GNU directory layout")
parser.add_option("-L", "--languages",dest="languages", 
            help="A JSON list of languages to pull from Pootle", default='["nl", "ru"]')
parser.add_option("-r", "--rmdir",action="store_true", dest="rmdir", default=False,
            help="Clear the output directory before starting")

(options, args) = parser.parse_args()

# e.g http://translate.multitheftauto.com/export/client/nl/client.po
if options.rmdir:
    # Clear our output directory first
    if os.path.exists(options.output):
        shutil.rmtree(options.output)
    print ( "Cleared output directory: '%s'"%(options.output) )

# Let's decode our JSON list into a Python list, and loop it
for lang in json.loads(options.languages):
    # Let's create our full Pootle export URL depending on whether we're GNU or not
    url = ""
    output = ""
    if options.gnu:
        url = urlparse.urljoin(options.url, "export/%s/%s.po"%(options.project,lang))
        output = os.path.join(options.output,"%s.po"%(lang))
    else:
        url = urlparse.urljoin(options.url, "export/%s/%s/%s.po"%(options.project,lang,options.project))
        output = os.path.join(options.output,"%s/%s.po"%(lang,options.project))

    u = urllib2.urlopen(url)
    path,tail = os.path.split(output)
    if ( not os.path.exists(path) ):
        os.makedirs(path)
    
    localFile = open(output, 'w')
    localFile.write(u.read())
    localFile.close()
    print ( "Read '%s' and written to '%s'"%(url,output) )
