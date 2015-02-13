stage 'Stage1', concurrency: 1

// Execute test_a.py twice sequentially.
node('metapps-cpp-slave1') {

    //git url: 'https://github.com/metno/met-api.git';
    git url: 'git://git.met.no/joa.git';

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
        git url: 'git://git.met.no/joa.git';
        echo 'running test_a.py in parallel for 50 secs ...';
        sh 'jenkins/workflow_plugin/python/test_a.py 50 1.0'
    }
}
branches["branch >>> 1 <<<"] = {
//    node('ted') {
    node('metapps-cpp-slave1') {
        git url: 'git://git.met.no/joa.git';
        echo 'running test_a.py in parallel for 30 secs and failing after iteration 20 ...';
        sh 'jenkins/workflow_plugin/python/test_a.py 60 0.5 20'
    }
}

echo 'starting parallel branches ...';
parallel branches

echo 'after parallel execution';

// Execute test_a.py once sequentially.
node('metapps-cpp-slave1') {
    git url: 'git://git.met.no/joa.git';
    echo 'running test_a.py sequentially for 4 secs ...';
    sh 'jenkins/workflow_plugin/python/test_a.py 4 1.0'
}
