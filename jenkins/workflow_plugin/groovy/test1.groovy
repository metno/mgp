// Execute test_a.py twice sequentially.
node('metapps-cpp-slave1') {

    //echo 'checking out met-api.git in workspace on metapps-cpp-slave1 ...';
    //git url: 'https://github.com/metno/met-api.git';
    echo 'checking out joa.git in workspace on metapps-cpp-slave1 ...';
    git url: 'git://git.met.no/joa.git';

    echo 'done';

    //input 'ready to continue?'

    echo 'running test_a.py sequentially for 5 secs ...';
    sh 'jenkins/workflow_plugin/python/test_a.py 5 1.0'
    echo '... and then sequentially for 3 secs ...';
    sh 'jenkins/workflow_plugin/python/test_a.py 6 0.5'
}


// Execute test_a.py twice in parallel.
def branches = [:]
branches["branch >>> 0 <<<"] = {
    node('metapps-cpp-slave1') {
        echo 'running test_a.py in parallel for 5 secs ...';
        sh 'jenkins/workflow_plugin/python/test_a.py 5 1.0'
    }
}
branches["branch >>> 1 <<<"] = {
    node('metapps-cpp-slave1') {
        echo 'running test_a.py in parallel for 3 secs ...';
        sh 'jenkins/workflow_plugin/python/test_a.py 6 0.5'
    }
}

echo 'starting parallel branches ...';
parallel branches
