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


# Returns a single-line, stripped version of s.
def singleLine(s):
    p = re.compile('\s+')
    return p.sub(' ', s).strip()


# Returns an elided version of s.
def elided(s, n):
    if len(s) <= n:
        return s
    r = ' ...'
    return s[:n-len(r)] + r

# Pretty-prints output from another process to stderr.
def printOutput(stdout, stderr):
    w = 40
    sys.stderr.write(
        '--- BEGIN standard output ' + '-'*w +
        '\n{}--- END standard output '.format(stdout) + '-'*w + '\n')
    sys.stderr.write(
        '--- BEGIN standard error ' + '-'*w +
        '\n{}--- END standard error '.format(stderr) + '-'*w + '\n')
