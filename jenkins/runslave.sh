#!/bin/sh

set -e
set -x

host=$1
jenkins_home=$2
jenkins_scripts_path=$3
ssh_options='-i /tmp/jenkins/.ssh/id_rsa -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null'

source=$jenkins_scripts_path/slave.jar
target=$jenkins_home/slave.jar
scp $ssh_options $source root@$host:$target
ssh $ssh_options root@$host java -jar $target
