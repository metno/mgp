node('metapps-cpp-slave1') {
    echo 'checking out met-api.git in workspace on metapps-cpp-slave1 ...';
    git url: 'https://github.com/metno/met-api.git';
    echo 'done';

    input 'ready to continue?'
    echo 'continuing ...';
}
