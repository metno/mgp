import sys, re

# Extracts the option dictionary from the command line
def getOptDict():
    options = {}
    p = re.compile('^--(.+)$')
    key = None
    for arg in sys.argv[1:]:
        if key != None:
            options[key] = arg
        m = p.match(arg)
        if m:
            key = m.group(1)
            # Support '-help' as the only value-less option:
            if key == 'help':
                options[key] = 1
                key = None
        else:
            key = None
    return options
