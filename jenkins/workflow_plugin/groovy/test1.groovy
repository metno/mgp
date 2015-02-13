node('metapps-cpp-slave1') {

    //echo 'checking out met-api.git in workspace on metapps-cpp-slave1 ...';
    //git url: 'https://github.com/metno/met-api.git';
    echo 'checking out joa.git in workspace on metapps-cpp-slave1 ...';
    git url: 'git://git.met.no/joa.git';

    echo 'done';

    //input 'ready to continue?'

    echo 'continuing to run test_a.py ...';
    sh 'pwd'
    sh 'jenkins/workflow_plugin/python/test_a.py 5 1.0'
}
