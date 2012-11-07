#!/bin/sh

set -e
set -x

user=$1
host=$2
jenkins_home=$3
jenkins_scripts_path=$4
ssh_options='-i /tmp/jenkins/.ssh/id_rsa -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null'

source=$jenkins_scripts_path/slave.jar
target=$jenkins_home/slave.jar
scp $ssh_options $source root@$host:$target
ssh $ssh_options $user@$host java -jar $target
