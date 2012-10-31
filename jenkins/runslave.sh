#!/bin/sh

set -e
set -x

host=$1
jardir=$2
ssh_options='-i /tmp/jenkins/.ssh/id_rsa -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null'

source=$jardir/slave.jar
target=$jardir/slave.jar
scp $ssh_options $source root@$host:$target
ssh $ssh_options root@$host java -jar $target
