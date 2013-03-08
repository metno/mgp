#!/bin/sh -e

# This script copies all */config.xml files under /var/lib/jenkins/jobs on a remote (or local) Jenkins
# installation to a local directory timestamped with the current date.

USAGE=`cat <<EOF
Usage: \`basename $0\` <destination base directory (e.g. '~/jenkins_backup')>
<source user@host (e.g. 'ubuntu@ci.met.no:' or '')>
EOF
`

if [ $# -ne 2 ]; then
    echo $USAGE >&2
    exit 1
fi

destbasedir=$1
srcuserhost=$2
#destdir=$destbasedir/backup_`date +%Y-%m-%d_%H-%M-%S`

echo -n 'rsync ... '
rsync -a $srcuserhost/var/lib/jenkins/jobs --include='*/' --include='*/config.xml' --exclude='*' $destbasedir
echo 'done'
