#!/bin/sh

set -e
set -x

host=$1
ssh_options='-i /tmp/jenkins/.ssh/id_rsa -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null'

source=$JENKINS_SCRIPTS_PATH/slave.jar
target=$JENKINS_HOME/slave.jar
scp $ssh_options $source root@$host:$target
ssh $ssh_options root@$host java -jar $target
